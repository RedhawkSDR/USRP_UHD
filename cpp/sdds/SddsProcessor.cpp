/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK USRP_UHD.
 *
 * REDHAWK USRP_UHD is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK USRP_UHD is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#include "SddsProcessor.h"

template <class DATA_TYPE>
PREPARE_LOGGING(SddsProcessor<DATA_TYPE>)

/**
 * Constructor for the templated SDDS processor class. Initializes the SDDS header with default values and
 * sets up the scatter / gather array for the UDP socket pushes.
 */
template <class DATA_TYPE>
SddsProcessor<DATA_TYPE>::SddsProcessor(bulkio::OutSDDSPort * dataSddsOut, size_t bufSz, size_t bufCnt):
        m_sdds_out_port(dataSddsOut), m_first_run(true), m_shutdown(false), m_running(false),
        m_active_stream(false), m_attached(false), m_processorThread(NULL), m_ttv_override(0),
        m_byte_swap(false), m_data_ref_str(DATA_REF_STR_NATIVE), m_give_sri_priority(false),
        m_vlan(0), m_seq(0), m_input_data_q(bufSz*bufCnt, bufSz+(SDDS_DATA_SIZE/sizeof(DATA_TYPE))-1),
		m_year_start_s(0), m_year_end_s(0) {
    /* The arguments to m_input_data_q are:
     *   1. Total number of samples to buffer = (size of expected pushPacket)*(number of packets to buffer)
     *   2. Total number of samples that might need to be read contiguously in memory = \
     *                       (one full pushPacket of data)+(one sample short of a full SDDS packet of data)
     *      -- This should represent the worst case. Scenario would be we have a partial SDDS packet
     *         missing a single sample (if it was full, it would be transmitted). To complete packet, we
     *         queue up the next block of data, which is a full pushPacket of data, and we need to have all
     *         this data in contiguous memory. The second argument satisfies this requirement.
     */

    m_pkt_template.msg_name = NULL;
    m_pkt_template.msg_namelen = 0;
    m_pkt_template.msg_iov = &m_msg_iov[0];
    m_pkt_template.msg_iovlen = 3; // We use three iov. One is the SDDS header and the second is the payload, the third for any zero padding if necessary.
    m_pkt_template.msg_control = NULL;
    m_pkt_template.msg_controllen = 0;
    m_pkt_template.msg_flags = NULL;

    m_msg_iov[0].iov_base = &m_sdds_template;
    m_msg_iov[0].iov_len = SDDS_HEADER_SIZE;
    m_msg_iov[1].iov_base = NULL; // This will be set later.
    m_msg_iov[1].iov_len = SDDS_DATA_SIZE;
    m_msg_iov[2].iov_base = m_zero_pad_buffer;
    m_msg_iov[2].iov_len = 0;

    memset(&m_zero_pad_buffer, 0, SDDS_DATA_SIZE);
    initializeSDDSHeader();
}

/**
 * Prior to destroying this instance, the parent class should be stopped to free up the
 * port locks. This destructor will never return unless the blocking port locks are lifted.
 */
template <class DATA_TYPE>
SddsProcessor<DATA_TYPE>::~SddsProcessor() {

    if (m_processorThread) {
        shutdown();
        join();
    }

    m_sdds_out_port->removeStream(getStreamId()); // TODO - does this detach as well? should we call detach ourselves?

    if (m_connection.sock > 0) {
        close(m_connection.sock);
        memset(&m_connection, 0, sizeof(m_connection));
    }
    // TODO - double check we're cleaning up everything
}

/**
 * Sets the active stream and starts the processing thread if the component has also started.
 */
template <class DATA_TYPE>
bool SddsProcessor<DATA_TYPE>::setStream(std::string streamID, std::string iface, std::string ip, long port,
        uint16_t vlan, std::string attach_user_id, long ttv, long endiance, bool sri_has_priority){
    LOG_INFO(SddsProcessor,"Received new streamID: " << streamID);

    if (m_active_stream) {
        LOG_ERROR(SddsProcessor,"There is already an active stream: " << m_streamID << ", cannot set new stream: " << streamID);
        return false;
    }

    m_streamID = streamID;
    m_user_id = attach_user_id;
    m_ttv_override = ttv;
    m_give_sri_priority = sri_has_priority;

    if (endiance==0) {
        m_data_ref_str = DATA_REF_STR_LITTLE;
    } else if (endiance==1) {
        m_data_ref_str = DATA_REF_STR_BIG;
    } else { // else native
        m_data_ref_str = DATA_REF_STR_NATIVE;
    }
    m_byte_swap = (DATA_REF_STR_NATIVE != m_data_ref_str);

    // set up socket and connection
    int sock;
    try {
        sock = setupSocket(iface, ip, port, vlan);
    } catch (...) {
        sock = -1;
    }
    if (sock < 0) {
        LOG_ERROR(SddsProcessor, "Could not setup the output socket, cannot start without successful socket connection.");
        return false;
    }
    m_vlan = vlan;
    m_pkt_template.msg_name = &m_connection.addr;
    m_pkt_template.msg_namelen = sizeof(m_connection.addr);

    initializeSDDSHeader();
    m_sdds_template.bps = (sizeof(DATA_TYPE) == sizeof(float)) ? (31) : 8*sizeof(DATA_TYPE);

    m_active_stream = true;
    return true;
}

/**
 * Attempts to setup either a multicast socket or unicast socket depending on the IP address provided.
 * Returns the socket file descriptor on success and -1 on failure. May also throw an exception in some
 * failure cases so both should be checked for.
 */
template <class DATA_TYPE>
int SddsProcessor<DATA_TYPE>::setupSocket(std::string iface, std::string ip, long port, uint16_t vlan) {
    int retVal = -1;
    std::string interface = iface;
    memset(&m_connection, 0, sizeof(m_connection));

    LOG_INFO(SddsProcessor, "Setting connection info to Interface: " << iface << " IP: " << ip<< " Port: " << port);
    if (ip.empty()) {
        LOG_ERROR(SddsProcessor, "IP or interface was empty when trying to setup the socket connection.")
        return retVal;
    }

    in_addr_t lowMulti = inet_network("224.0.0.0");
    in_addr_t highMulti = inet_network("239.255.255.250");

    if (vlan) {
        std::stringstream ss;
        ss << interface << "." << vlan;
        interface = ss.str();
    }

    if ((inet_network(ip.c_str()) > lowMulti) && (inet_addr(ip.c_str()) < highMulti)) {
        m_connection = multicast_server(interface.c_str(), ip.c_str(), port);
    } else {
        m_connection = unicast_server(interface.c_str(), ip.c_str(), port);
    }

    LOG_INFO(SddsProcessor, "Created socket (fd: " << m_connection.sock << ") connection on: " << interface << " IP: " << ip << " Port: " << port);

    return m_connection.sock;
}

/**
 * Removes the active stream and shuts down the processing thread. It is possible
 * that the processing thread can get stuck in a blocking BulkIO read if no data is being sent
 * and an EOS isn't received.
 */
template <class DATA_TYPE>
void SddsProcessor<DATA_TYPE>::removeStream(std::string streamID) {
    LOG_INFO(SddsProcessor,"Removing stream: " << streamID);
    if (m_active_stream && m_streamID == streamID) {
        if (m_running) {
            shutdown();
            join();
        }
    } else {
        LOG_WARN(SddsProcessor,"Was told to remove stream that was not already set.");
    }

    if (m_connection.sock > 0) {
        close(m_connection.sock);
        memset(&m_connection, 0, sizeof(m_connection));
    }

    m_sdds_out_port->removeStream(getStreamId());
}

/**
 * If there is an active stream to process, this will inform connections via the BulkIO SDDS attach call and kick off a new thread
 * to perform the processing. If there is no active stream to process this call will simply return.
 */
template <class DATA_TYPE>
void SddsProcessor<DATA_TYPE>::run() {
    LOG_TRACE(SddsProcessor,"Entering the Run Method");
    if (m_processorThread) {
        if (m_running) {
            LOG_ERROR(SddsProcessor,"The BulkIO To SDDS Processor is already running, cannot start a new stream without first receiving an EOS!");
            return;
        } else {
            LOG_TRACE(SddsProcessor,"Told to run but there is already a thread instance, calling join first.");
            join();
        }
    }

    if (not m_active_stream) {
        return;
    }

    m_processorThread = new boost::thread(boost::bind(&SddsProcessor<DATA_TYPE>::_run, boost::ref(*this)));
    LOG_TRACE(SddsProcessor,"Leaving the Run Method");
}

/**
 * Simply sets the boolean shutdown field to true so that the run thread will stop on its next iteration.
 * It is possible that the run thread can get stuck in a blocking BulkIO read but a components stop() call will
 * force a return of the blocking read.
 */
template <class DATA_TYPE>
void SddsProcessor<DATA_TYPE>::shutdown() {
    if (m_running)
        m_shutdown = true;
}

/**
 * If the processing thread is running, this joins the thread, deletes the boost thread object
 * and sets the thread pointer to NULL. If the thread is not running, it does nothing.
 */
template <class DATA_TYPE>
void SddsProcessor<DATA_TYPE>::join() {
    if (m_processorThread) {
        m_input_metadata_q.interrupt();
        m_processorThread->join();
        delete(m_processorThread);
        m_processorThread = NULL;
    } else {
        LOG_DEBUG(SddsProcessor,"Join called but the thread object is null.");
    }
}

/**
 * The internal run method is the entry point for the processing thread. Will loop infinitely until there is an issue sending a packet,
 * it has received an EOS, or is told to shutdown.
 * Under normal operation, this loop will receive data from the BulkIO stream via a call to getDataPointer and send the received data payload
 * via the socket connection along with the appropriate SDDS header data. SRI is pushed across the SDDS port if needed as well.
 */
template <class DATA_TYPE>
void SddsProcessor<DATA_TYPE>::_run() {
    LOG_TRACE(SddsProcessor,"Entering the _run Method");
    m_running = true;
    int bytes_read = 0;

    while (not m_shutdown) {
        LOG_TRACE(SddsProcessor,"Sent     " << m_metadata.total_consumed()*sizeof(DATA_TYPE) << " of "<< bytes_read <<" bytes input data");
        bytes_read = getDataPointer();
        LOG_TRACE(SddsProcessor,"Received " << bytes_read << " bytes input data");
        if (m_metadata.sri_changed()) {
            LOG_INFO(SddsProcessor,"Stream SRI has changed, updating SDDS header.");
            m_metadata.sri_changed(false);
            pushSri();
            setSddsHeaderFromSri();
        }

        // It could be that we were stopped, it could also be we got an empty packet with an EOS.
        if (not bytes_read) {
            if (m_attached && m_metadata.eos()) {
                callDetach();
            }
            shutdown();
            continue;
        }

        // Loop through, pushing full packets
        bool error = false;
        char *sddsDataBlock = reinterpret_cast<char*>(m_metadata.data());
        while (m_metadata.size()*sizeof(DATA_TYPE) >= SDDS_DATA_SIZE && !error) {
            if ( (error = (sendPacket(sddsDataBlock, SDDS_DATA_SIZE) < 0)) ) {
                continue;
            }
            sddsDataBlock += SDDS_DATA_SIZE;
            m_metadata.consume(SDDS_DATA_SIZE/sizeof(DATA_TYPE));
        }
        if (error) {
            LOG_ERROR(SddsProcessor,"Failed to push packet over socket, SddsProcessor will shutdown.");
            //callDetach(); // XXX Add detach here even though it's not EOS? If we start back up, will another attach be sent?
            shutdown();
            continue;
        }

        /* There is less than a full packet of data remaining. Push partial if size > 0 and:
         *   1. current block has EOS (done, send partial packet), OR
         *   2. getDataPointer returned a partial packet (i.e. bytes_read < SDDS_DATA_SIZE)
         *     - this means either we were interrupted or the next data has new sri
         */
        if (m_metadata.size()>0 && (m_metadata.eos() || bytes_read < SDDS_DATA_SIZE) ) {
            LOG_TRACE(SddsProcessor,"Pushing partial packet with " << m_metadata.size()*sizeof(DATA_TYPE) << " bytes.");
            if ( (error = (sendPacket(sddsDataBlock, m_metadata.size()*sizeof(DATA_TYPE)) < 0)) ) {
                LOG_ERROR(SddsProcessor,"Failed to push partial packet over socket, SddsProcessor will shutdown.");
                //callDetach(); // XXX Add detach here even though it's not EOS? If we start back up, will another attach be sent?
                shutdown();
                continue;
            }
            sddsDataBlock = 0;
            m_metadata.consume();
        }

        if (m_first_run) {
            m_first_run = false;
        }

        // This is for the case where we received a full packet but it came with an EOS flag attached.
        if (m_metadata.eos()) {
            callDetach();
            shutdown();
            continue;
        }
    }

    m_first_run = true;
    m_shutdown = false;
    m_running = false;

    LOG_TRACE(SddsProcessor,"Exiting the _run Method");
}

/**
 * Attempts to read 1024 bytes of data from the BulkIO stream API and return a pointer to the data read.
 * Updates the provided sriChanged flag and sets the class variables for the current bulkIO time and current SRI.
 * If the user has requested byte swapping the received data is swapped in place here.
 */
template <class DATA_TYPE>
size_t SddsProcessor<DATA_TYPE>::getDataPointer() {
    LOG_TRACE(SddsProcessor,"Entering getDataPointer Method");

    // Make sure consumed data is removed (skipped) from m_input_data_q
    m_input_data_q.skip(m_metadata.total_consumed());

    size_t existing_samples = m_metadata.size(); // need to track this b/c they've already been swapped (if necessary)
    if (existing_samples == 0) {
        // The following call blocks until data (valid=true), or interrupted (valid=false)
        bool valid = m_input_metadata_q.pop(m_metadata);
        if( !valid){
            LOG_TRACE(SddsProcessor,"Leaving getDataPointer Method");
            //m_metadata.consume(); // maintains sri/eos, advances timestamp, clears data/sample count/sri_changed -- actually, this is unnecessary
            return 0;
        }
    }
    bool done = false;
    while (m_metadata.size()*sizeof(DATA_TYPE) < SDDS_DATA_SIZE && !done && !m_metadata.eos()) {
        /* Loop until one of the following is true (or interrupted):
         *   1. current block has EOS (done, send partial packet)
         *   2. next block has sri_changed (can't combine different sri's, send partial packet)
         *   3. Combined blocks reach desired size (send full packet)
         * Note: Pull blocks from m_input_metadata_q and update current
         *       m_metadata with additional sample count and EOS (if necessary).
         */

        // Peek ahead to see if next block can be combined with current block
        METADATA_TYPE tmp_meta;
        try {
            done = m_input_metadata_q.front().sri_changed();
        } catch (std::runtime_error& e) {
            LOG_TRACE(SddsProcessor,"Returning partial packet due to interrupt.");
            done = true;
            continue;
        }
        if (!done) {
            LOG_TRACE(SddsProcessor,"Combining two blocks with same SRI.");
            done = !m_input_metadata_q.pop(tmp_meta);
            if(done){
                LOG_TRACE(SddsProcessor,"Returning partial packet due to interrupt.");
            } else {
                // Combine blocks
                if(m_metadata.add(tmp_meta) != tmp_meta.size()) {
                    LOG_ERROR(SddsProcessor,"Returning partial packet due to problem combining blocks.");
                    done = true;
                }
            }
        } else {
            LOG_TRACE(SddsProcessor,"Returning partial packet due to updated SRI.");
        }
    }

    // This could be empty block w/ new sri only and we got here b/c of interrupt,
    // so only do following call if there's data since it's a blocking call
    if (m_metadata.size() > 0)
        m_metadata.set(&m_input_data_q.front(m_metadata.size()));
    size_t bytes_read = m_metadata.size()*sizeof(DATA_TYPE);

    size_t existing_bytes = existing_samples*sizeof(DATA_TYPE);
    if (bytes_read-existing_bytes > 0) {
        if (m_byte_swap) {
            //char *dataPointer = reinterpret_cast<char*>(m_metadata.data()+existing_samples);
            char *dataPointer = reinterpret_cast<char*>(m_metadata.data()) + existing_bytes;
            uint32_t *buf = reinterpret_cast<uint32_t*>(dataPointer);

            switch (sizeof(DATA_TYPE)) {
            case sizeof(short):
                swab(dataPointer, dataPointer, bytes_read-existing_bytes);
                break;
            case sizeof(float):
                for (size_t i = 0; i < m_metadata.size()-existing_samples; ++i) {
                    buf[i] = __builtin_bswap32(buf[i]);
                }
                break;
            case sizeof(char):
                // Do nothing
                break;
            default:
                break;
            }
        }
    }

    if (m_first_run) {
        m_metadata.sri_changed(true);
    }

    LOG_TRACE(SddsProcessor,"Leaving getDataPointer Method");
    return bytes_read;
}

template <class DATA_TYPE>
void SddsProcessor<DATA_TYPE>::dataIn(const std::vector<DATA_TYPE>& data, const BULKIO::PrecisionUTCTime& T, bool EOS, const BULKIO::StreamSRI& sri) {
    boost::mutex::scoped_lock lock(m_input_mutex); // only one writer at a time

    // Don't waste time with useless input
    if (!m_attached && data.empty() && EOS)
        return;

    // make attach call here if first dataIn w/o attach
    if (!m_attached)
        callAttach(sri);

    //size_t samples = m_input_data_q.write(&data[0], data.size()); // blocking, but can still write partial
    size_t samples = m_input_data_q.trywrite(&data[0], data.size()); // non-blocking

    m_input_metadata.set(samples, T, EOS, sri);
    m_input_metadata_q.push(m_input_metadata);

    if (samples < data.size()) {
        LOG_ERROR(SddsProcessor, "Failed to write full input data block, dropping " << (data.size()-samples)/(1+m_input_metadata.sri().mode) << "samples. Try increasing sdds buffer size.");
    }
}

template <class DATA_TYPE>
void SddsProcessor<DATA_TYPE>::dataIn(const std::vector<DATA_TYPE>& data, const BULKIO::PrecisionUTCTime& T, bool EOS) {
    boost::mutex::scoped_lock lock(m_input_mutex); // only one writer at a time

    // make sure attached w/ sri before accepting data w/o sri
    // also, ignore empty packets without any useful metadata (eos)
    if(!m_attached || (data.empty() && !EOS)) return;

    //size_t samples = m_input_data_q.write(&data[0], data.size()); // blocking, but can still write partial
    size_t samples = m_input_data_q.trywrite(&data[0], data.size()); // non-blocking

    m_input_metadata.update(samples, T, EOS);
    m_input_metadata_q.push(m_input_metadata);

    if (samples < data.size()) {
        LOG_ERROR(SddsProcessor, "Failed to write full input data block; wrote "
        		<< (samples)/(1+m_input_metadata.sri().mode) << " and dropping "
        		<< (data.size()-samples)/(1+m_input_metadata.sri().mode) << " of "
				<< (data.size())/(1+m_input_metadata.sri().mode) << " total samples");
    }
}

/**
 * Takes the provided data buffer and sends it along with the SDDS header, increasing the SDDS sequence number and resetting the start
 * of stream flag when needed.
 *
 * Since the SDDS spec specifies the payload size to be exactly 1024 bytes there are two situations covered here.
 * 1. We've read exactly 1024 bytes. We get exactly one packets worth and this method runs only once.
 * 2. We've read less than 1024 bytes, occurs when the stream is changing. In this case the data is padded with zeros.
 *
 * dataBlocks sized less than 1024 are dealt with via the scatter gather concept. We keep 3 buffers, the SDDS header, the SDDS payload,
 * and a buffer of zeros sized at 1024. We tell the linux kernel to create a UDP packet consisting of each of these buffers and simply
 * vary the size of the latter two based on the provided buffer length.
 */
template <class DATA_TYPE>
int SddsProcessor<DATA_TYPE>::sendPacket(char* dataBlock, size_t num_bytes) {
    LOG_TRACE(SddsProcessor,"sendPacket called, told to send " << num_bytes <<  " bytes");
    if (num_bytes == 0)
        return 0;

    ssize_t numSent;
    // Reset the start of sequence flag if it is not the first packet sent, the sequence number has rolled over, and sos is currently set.
    if (not m_first_run && m_seq == 0 && m_sdds_template.sos) { m_sdds_template.sos = 0; }

    m_sdds_template.set_seq(m_seq);
    m_seq++;
    if (m_seq != 0 && m_seq % 32 == 31) { m_seq++; } // Account for the parity packet that we aren't sending

    m_msg_iov[1].iov_base = dataBlock;
    m_msg_iov[1].iov_len = num_bytes;
    m_msg_iov[2].iov_len = SDDS_DATA_SIZE - m_msg_iov[1].iov_len;

    setSddsTimestamp();
    numSent = sendmsg(m_connection.sock, &m_pkt_template, 0);
    if (numSent < 0) {return numSent;} // Error occurred

    LOG_TRACE(SddsProcessor,"Pushed " << numSent << " bytes out of socket.")
    return 0;
}

/**
 * Will call detach if there is an active stream, then unset the active stream flag.
 */
template <class DATA_TYPE>
void SddsProcessor<DATA_TYPE>::callDetach() {
    // There are multiple places were detach can be called during the shutdown process so just return if we've already cleared out the stream.
    if (m_active_stream) {
        LOG_TRACE(SddsProcessor, "Calling detach on current stream: " << m_streamID);
        m_active_stream = false; // Set this before the detach call in case it throws an exception
        try {
            m_sdds_out_port->detach(CORBA::string_dup(m_streamID.c_str()));
        } catch (...) {
            LOG_ERROR(SddsProcessor, "Error occurred while calling detach");
        }
    } else {
        LOG_TRACE(SddsProcessor, "Was told to call detach but also told there is no active stream!");
    }
    m_attached = false;
}

/**
 * Calls attach only on the provided BulkIO SDDS input port. If the provided input port is NULL,
 * it will call attach on all connections using the previously provided SDDS output port.
 */
template <class DATA_TYPE>
void SddsProcessor<DATA_TYPE>::callAttach(BULKIO::dataSDDS::_ptr_type sdds_input_port, const BULKIO::StreamSRI& sri) {
    LOG_DEBUG(SddsProcessor, "Calling attach");
    if (not m_active_stream) {
        LOG_DEBUG(SddsProcessor, "No active stream so attach will not go through");
        return;
    }
    m_attached = true;

    BULKIO::SDDSStreamDefinition sdef;

    sdef.id = CORBA::string_dup(m_streamID.c_str());

    switch(sizeof(DATA_TYPE)) {
    case sizeof(char):
        sdef.dataFormat = (sri.mode == 1) ? BULKIO::SDDS_CB : BULKIO::SDDS_SB;
        break;
    case sizeof(short):
        sdef.dataFormat = (sri.mode == 1) ? BULKIO::SDDS_CI : BULKIO::SDDS_SI;
        break;
    case sizeof(float):
        sdef.dataFormat = (sri.mode == 1) ? BULKIO::SDDS_CF : BULKIO::SDDS_SF;
        break;
    default:
        LOG_ERROR(SddsProcessor,"Native type size is not what we would expect.");
        break;

    }

    // SDDS StreamDef has sample rate as an unsigned long, so we round to nearest integer
    double tmp_sr = 1.0/sri.xdelta+0.5; // Needs to be 2 steps to prevent math errors on 32-bit systems
    sdef.sampleRate = (unsigned long) tmp_sr;
    sdef.timeTagValid = (m_metadata.timestamp().tcstatus == BULKIO::TCS_VALID);
    sdef.multicastAddress = CORBA::string_dup(inet_ntoa(m_connection.addr.sin_addr));
    sdef.vlan = m_vlan;
    sdef.port = ntohs(m_connection.addr.sin_port);

    if (sdds_input_port == NULL) {
        m_sdds_out_port->attach(sdef, m_user_id.c_str());
    } else {
        sdds_input_port->attach(sdef, m_user_id.c_str());
    }
}
template <class DATA_TYPE>
void SddsProcessor<DATA_TYPE>::callAttach(BULKIO::dataSDDS::_ptr_type sdds_input_port) {
    BULKIO::StreamSRI tmpSri = m_metadata.sri();
    callAttach(sdds_input_port, tmpSri);
}

/**
 * Calls attach(NULL) which will in turn call attach on the sdds output port provided previously.
 */
template <class DATA_TYPE>
void SddsProcessor<DATA_TYPE>::callAttach(const BULKIO::StreamSRI& sri) {
    callAttach((BULKIO::dataSDDS::_ptr_type)NULL, sri);
}
template <class DATA_TYPE>
void SddsProcessor<DATA_TYPE>::callAttach() {
    BULKIO::StreamSRI tmpSri = m_metadata.sri();
    callAttach((BULKIO::dataSDDS::_ptr_type)NULL, tmpSri);
}

/**
 * Calls pushSri(NULL) which in turn will call pushSRI on all connected ports after adding any requested keywords.
 */
template <class DATA_TYPE>
void SddsProcessor<DATA_TYPE>::pushSri() {
    pushSri((BULKIO::dataSDDS::_ptr_type)NULL);
}

/**
 * Calls pushSRI on the port specified, or all connected ports if NULL is provided. Will add the
 * BULKIO_SRI_PRIORITY keyword if downstream_give_sri_priority property set to true and the
 * DATA_REF_STR to the appropriate endianness type.
 */
template <class DATA_TYPE>
void SddsProcessor<DATA_TYPE>::pushSri(BULKIO::dataSDDS::_ptr_type sdds_input_port) {
    LOG_DEBUG(SddsProcessor, "Pushing SRI to downstream components");
    if (not m_active_stream || not m_attached) {
        LOG_DEBUG(SddsProcessor, "No active stream so pushSRI will not go through");
        return;
    }
    BULKIO::StreamSRI tmpSri = m_metadata.sri(); // this could get updated at any moment, so just grab a copy to use
    BULKIO::PrecisionUTCTime tmpTime = m_metadata.timestamp(); // same as above

    if (m_give_sri_priority) {
        addModifyKeyword<long>(&tmpSri,"BULKIO_SRI_PRIORITY",CORBA::Long(1));
    }

    addModifyKeyword<long>(&tmpSri,"DATA_REF_STR",CORBA::Long(m_data_ref_str));

    if (sdds_input_port == NULL) {
        m_sdds_out_port->pushSRI(tmpSri, tmpTime);
    } else {
        sdds_input_port->pushSRI(tmpSri, tmpTime);
    }
}

/**
 * Returns true if an active stream has been set on this instance, false otherwise.
 */
template <class DATA_TYPE>
bool SddsProcessor<DATA_TYPE>::isActive() {
    return m_active_stream;
}

/**
 * Initializes the SDDS header with the default values and sets the dmode.
 */
template <class DATA_TYPE>
void SddsProcessor<DATA_TYPE>::initializeSDDSHeader(){
    m_sdds_template.pp = 0;
    m_sdds_template.sf = 1;
    m_sdds_template.of = 0;
    m_sdds_template.ss = 0;
    m_sdds_template.vw = 0;
    m_sdds_template.snp = 0;
    m_sdds_template.clear_msptr(); // also clears m_sdds_template.msdel
    m_sdds_template.set_sscv(1);
    m_sdds_template.set_dfdt(0.0);
    m_sdds_template.set_ttv(m_ttv_override==1);

    switch (sizeof(DATA_TYPE)) {
    case sizeof(char):
        m_sdds_template.dmode = 1;
        break;
    case sizeof(short):
        m_sdds_template.dmode = 2;
        break;
    default:
        m_sdds_template.dmode = 0;
        break;
    }

    for (size_t i = 0; i < SSD_LENGTH; ++i) { m_sdds_template.ssd[i] = 0; }
    for (size_t i = 0; i < AAD_LENGTH; ++i) { m_sdds_template.aad[i] = 0; }
}

/**
 * Sets the complex flag, frequency field, and sos flag based on the SRI unless otherwise overridden by the user.
 */
template <class DATA_TYPE>
void SddsProcessor<DATA_TYPE>::setSddsHeaderFromSri() {
    BULKIO::StreamSRI tmpSri = m_metadata.sri();
    m_sdds_template.cx = tmpSri.mode;
    // This is the frequency of the digitizer clock which is twice the sample frequency if complex.
    double freq = (tmpSri.mode == 1) ? (2.0 / tmpSri.xdelta) : (1.0 / tmpSri.xdelta);
    m_sdds_template.set_freq(freq);
    if (m_first_run) {
        m_sdds_template.sos = 1;
    }
}

/**
 * Sets the SDDS timestamp field and the time tag valid field based on the received bulkIO time stamp
 * and the bulkio tcstatus flag.
 */
template <class DATA_TYPE>
void SddsProcessor<DATA_TYPE>::setSddsTimestamp() {
    BULKIO::PrecisionUTCTime tmpTs = m_metadata.timestamp();
    double seconds_since_new_year = getSecondsSinceStartOfYear(tmpTs.twsec);
    if (seconds_since_new_year < 0) {
        LOG_WARN(SddsProcessor, "Cannot properly convert BulkIOTime to SDDS, the BulkIO timestamp is not from this year.");
    }
    SDDSTime sdds_time(seconds_since_new_year, tmpTs.tfsec);
    m_sdds_template.set_SDDSTime(sdds_time);
    if (m_ttv_override < 0) { // if NOT overriding, use timestamp tcstatus
        m_sdds_template.set_ttv((tmpTs.tcstatus==BULKIO::TCS_VALID));
    }
}

/**
 * Returns the number of seconds since start of year provided now is current timestamp
 * Needed for the SDDS timestamp field
 */
template <class DATA_TYPE>
double SddsProcessor<DATA_TYPE>::getSecondsSinceStartOfYear(double twsec) {
	if (twsec < (double)m_year_end_s) {
		return twsec-m_year_start_s;
	}
    time_t now_utc = time_t(twsec) + timezone; // make UTC/GMT
    tm *now_struct_utc;

    /* Timestamp in a struct of day, month, year */
    now_struct_utc = gmtime(&now_utc);

    // TODO: Does this logic work on new years day?
    /* Find time from EPOCH to Jan 1st of current year */
    now_struct_utc->tm_sec=0;
    now_struct_utc->tm_min=0;
    now_struct_utc->tm_hour=0;
    now_struct_utc->tm_mday=1;
    now_struct_utc->tm_mon=0;

    m_year_start_s = (mktime(now_struct_utc) - timezone); // make local
    now_struct_utc->tm_year++;
    m_year_end_s = (mktime(now_struct_utc) - timezone); // make local

    return twsec-m_year_start_s;
}

/**
 * Returns the current streams stream ID.
 */
template <class DATA_TYPE>
std::string SddsProcessor<DATA_TYPE>::getStreamId() {
    return m_streamID;
}

/**
 * Since this is a templated class with a cpp and headerfile, the cpp
 * file must declare all the template types to generate. In this case it is
 * simply short port types.
 */
template class SddsProcessor<short>;
