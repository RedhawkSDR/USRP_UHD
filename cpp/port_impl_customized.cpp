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
#include "port_impl_customized.h"

// ----------------------------------------------------------------------------------------
// OutSDDSPort_customized
// ----------------------------------------------------------------------------------------

template <class DATA_TYPE>
PREPARE_LOGGING(OutSDDSPort_customized<DATA_TYPE>)

template <class DATA_TYPE>
OutSDDSPort_customized<DATA_TYPE>::OutSDDSPort_customized(std::string port_name):
bulkio::OutSDDSPort(port_name) {
    TRACE_ENTER(OutSDDSPort_customized);
    LOG_TRACE(OutSDDSPort_customized,__PRETTY_FUNCTION__<<"port_name: "<<port_name);
    setNewConnectListener(this, &OutSDDSPort_customized<DATA_TYPE>::pushSriOnConnect);
    TRACE_EXIT(OutSDDSPort_customized);
}

template <class DATA_TYPE>
OutSDDSPort_customized<DATA_TYPE>::~OutSDDSPort_customized() {
    TRACE_ENTER(OutSDDSPort_customized);
    typename streamid_to_proc_map_t::iterator it = streamid_to_processor.begin();
    while(it != streamid_to_processor.end()) {
        it->second->shutdown();
        it->second->join();
        ++it;
    }
    TRACE_EXIT(OutSDDSPort_customized);
}

template <class DATA_TYPE>
bool OutSDDSPort_customized<DATA_TYPE>::setStream(std::string streamID, std::string iface,
        std::string ip, long port, unsigned short vlan, std::string attach_user_id,
        long ttv, long endiance, bool sri_has_priority, size_t buffer_size, size_t buffer_cnt) {
    TRACE_ENTER(OutSDDSPort_customized);
    LOG_TRACE(OutSDDSPort_customized,__PRETTY_FUNCTION__<<"streamID: "<<streamID);

    // create proc, apply settings, setup socket, apply connection info
    typename streamid_to_proc_map_t::iterator proc_iter = streamid_to_processor.find(streamID);
    if (proc_iter != streamid_to_processor.end()) {
        if (proc_iter->second->isActive()) {
            LOG_ERROR(OutSDDSPort_customized,__PRETTY_FUNCTION__<<"Cannot create SDDS processor because there is already an active SDDS processor for same streamID ("<<streamID<<").");
            TRACE_EXIT(OutSDDSPort_customized);
            return false; // processor for streamID already exists!
        }
    	LOG_DEBUG(OutSDDSPort_customized,__PRETTY_FUNCTION__<<"Erasing existing inactive SDDS processor for same streamID ("<<streamID<<").");
    	streamid_to_processor.erase(proc_iter);
    }
	LOG_DEBUG(OutSDDSPort_customized,__PRETTY_FUNCTION__<<"Creating SDDS processor for streamID: "<<streamID<<".");
	proc_ptr_t proc = (proc_ptr_t) new SddsProcessor<DATA_TYPE>(this, buffer_size, buffer_cnt);
	if (proc->setStream(streamID, iface, ip, port, vlan, attach_user_id, ttv, endiance, sri_has_priority)) {
		LOG_DEBUG(OutSDDSPort_customized,__PRETTY_FUNCTION__<<"Successfully configured SDDS processor for streamID: "<<streamID<<".");
		streamid_to_processor.insert(std::make_pair(streamID, proc));
	} else {
		LOG_ERROR(OutSDDSPort_customized,__PRETTY_FUNCTION__<<"Failed to configure SDDS processor for streamID: "<<streamID<<".");
		TRACE_EXIT(OutSDDSPort_customized);
		return false;
	}
    TRACE_EXIT(OutSDDSPort_customized);
    return true;
}

template <class DATA_TYPE>
bool OutSDDSPort_customized<DATA_TYPE>::startStream(std::string streamID) {
    TRACE_ENTER(OutSDDSPort_customized);
    LOG_TRACE(OutSDDSPort_customized,__PRETTY_FUNCTION__<<"streamID: "<<streamID);
    typename streamid_to_proc_map_t::iterator proc_iter = streamid_to_processor.find(streamID);
    if (proc_iter == streamid_to_processor.end()) {
        LOG_ERROR(OutSDDSPort_customized,__PRETTY_FUNCTION__<<" Cannot start SDDS processor: no processor found with streamID: "<<streamID);
        TRACE_EXIT(OutSDDSPort_customized);
        return false; // processor for streamID does not exist!
    } else {
        proc_iter->second->run();
    }
    setActiveStatus(true);
    TRACE_EXIT(OutSDDSPort_customized);
    return true;
}
/*
template <class DATA_TYPE>
bool OutSDDSPort_customized<DATA_TYPE>::endStream(std::string streamID) {
    TRACE_ENTER(OutSDDSPort_customized);
    LOG_TRACE(OutSDDSPort_customized,__PRETTY_FUNCTION__<<"streamID: "<<streamID);
    typename streamid_to_proc_map_t::iterator proc_iter = streamid_to_processor.find(streamID);
    if (proc_iter == streamid_to_processor.end()) {
        LOG_ERROR(OutSDDSPort_customized,__PRETTY_FUNCTION__<<" Cannot shutdown SDDS processor: no processor found with streamID: "<<streamID);
        TRACE_EXIT(OutSDDSPort_customized);
        return false; // processor for streamID does not exist!
    } else {
        proc_iter->second->shutdown();
        LOG_DEBUG(OutSDDSPort_customized,__PRETTY_FUNCTION__<<" Shutting down SDDS processor with streamID: "<<streamID<<", waiting for join()...");
        proc_iter->second->join();
        LOG_DEBUG(OutSDDSPort_customized,__PRETTY_FUNCTION__<<" Joined SDDS processor with streamID: "<<streamID);
    }
    TRACE_EXIT(OutSDDSPort_customized);
    return true;
}

template <class DATA_TYPE>
bool OutSDDSPort_customized<DATA_TYPE>::removeStream(std::string streamID) {
    TRACE_ENTER(OutSDDSPort_customized);
    LOG_TRACE(OutSDDSPort_customized,__PRETTY_FUNCTION__<<"streamID: "<<streamID);
    typename streamid_to_proc_map_t::iterator proc_iter = streamid_to_processor.find(streamID);
    if (proc_iter == streamid_to_processor.end()) {
        LOG_ERROR(OutSDDSPort_customized,__PRETTY_FUNCTION__<<" Cannot shutdown SDDS processor: no processor found with streamID: "<<streamID);
        TRACE_EXIT(OutSDDSPort_customized);
        return false; // processor for streamID does not exist!
    } else {
        proc_iter->second->shutdown();
        LOG_DEBUG(OutSDDSPort_customized,__PRETTY_FUNCTION__<<" Shutting down SDDS processor with streamID: "<<streamID<<", waiting for join()...");
        proc_iter->second->join();
        LOG_DEBUG(OutSDDSPort_customized,__PRETTY_FUNCTION__<<" Joined SDDS processor with streamID: "<<streamID);
        streamid_to_processor.erase(proc_iter);
    }
    TRACE_EXIT(OutSDDSPort_customized);
    return true;
}
*/

/**
 * This is the callback method when a new connection from the SDDS output port is made.
 * Needed for any dynamic connections made while the component is running it will
 * determine the port on the other end of the connection and update the SRI.
 * This is normally done by BulkIO for the other BulkIO style ports but currently (2.0.1) this
 * is not done and has been logged as a bug against the framework. CF-1517
 * It seems as though the attach call is taken care of however.
 */
template <class DATA_TYPE>
void OutSDDSPort_customized<DATA_TYPE>::pushSriOnConnect(const char *connectionId) {
    TRACE_ENTER(OutSDDSPort_customized);
    LOG_TRACE(OutSDDSPort_customized,__PRETTY_FUNCTION__<<"connectionId: "<<connectionId);

    //boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    // Get SDDS Input Port
    BULKIO::dataSDDS_var sdds_input_port;
    ExtendedCF::UsesConnectionSequence_var currentConnections = connections();
    LOG_DEBUG(OutSDDSPort_customized,__PRETTY_FUNCTION__<<"There are "<<currentConnections->length()<<" current connections.");
    for (size_t i = 0; i < currentConnections->length(); ++i) {
        ExtendedCF::UsesConnection conn = currentConnections[i];
        if (strcmp(connectionId, conn.connectionId) == 0) {
            LOG_DEBUG(OutSDDSPort_customized,__PRETTY_FUNCTION__<<"Found connection entry with matching connectionId: "<<connectionId);
            sdds_input_port = BULKIO::dataSDDS::_narrow(conn.port);
        } else {
            LOG_WARN(OutSDDSPort_customized,__PRETTY_FUNCTION__<<"Could not find connection entry with matching connectionId: "<<connectionId);
        }
    }

    // Now push sri for each stream the connection is listening to
    std::string connId(connectionId);
    bool portListed = false;
    std::vector<bulkio::connection_descriptor_struct>::iterator ftPtr;
    for (ftPtr = filterTable.begin(); ftPtr != filterTable.end(); ftPtr++) {
        // Skip irrelevant port entries
        if (ftPtr->port_name != name)
            continue;
        // Process filterTable entry
        portListed = true;
        if (ftPtr->connection_id == connId) {
            LOG_DEBUG(OutSDDSPort_customized,__PRETTY_FUNCTION__<<"Port/connection found in connectionTable, pushing SRI for connectionId: "<<connectionId);
            try {
                streamid_to_processor.at(ftPtr->stream_id)->pushSri(sdds_input_port);
                LOG_DEBUG(OutSDDSPort_customized,__PRETTY_FUNCTION__<<"Pushed SRI for port: "<<name<<" and connectionId: "<<connectionId);
            } catch (std::out_of_range& e){
                LOG_WARN(OutSDDSPort_customized,__PRETTY_FUNCTION__<<"Failed to push SRI for connectionId: "<<connectionId);
                // pass
            }
        }
    }
    // If port wasn't listed, must not be multi-out. Push all.
    if (!portListed){
        LOG_DEBUG(OutSDDSPort_customized,__PRETTY_FUNCTION__<<"Port ("<<name<<") was NOT found in connectionTable for connectionId: "<<connectionId);
        typename streamid_to_proc_map_t::iterator procIter;
        for (procIter=streamid_to_processor.begin(); procIter!=streamid_to_processor.end(); procIter++) {
            LOG_DEBUG(OutSDDSPort_customized,__PRETTY_FUNCTION__<<"Calling pushSri for connectionID: "<<connectionId<<" on SDDS processor for streamID: "<<procIter->first);
            procIter->second->pushSri(sdds_input_port);
        }
    } else {
        LOG_DEBUG(OutSDDSPort_customized,__PRETTY_FUNCTION__<<"Port ("<<name<<") was found in connectionTable for connectionId: "<<connectionId);
    }
    TRACE_EXIT(OutSDDSPort_customized);
}

template<class DATA_TYPE>
void OutSDDSPort_customized<DATA_TYPE>::pushSRI(const BULKIO::StreamSRI& H) {
    TRACE_ENTER(OutSDDSPort_customized);
    LOG_TRACE(OutSDDSPort_customized,__PRETTY_FUNCTION__<<"streamID: "<<H.streamID);
    boost::mutex::scoped_lock lock(input_port_lock);

    std::string sid(H.streamID);
    streamid_to_sri_map_t::iterator sri_iter = streamid_to_sri.find(sid);
    if (sri_iter == streamid_to_sri.end()) {
        LOG_DEBUG(OutSDDSPort_customized,__PRETTY_FUNCTION__<<"Inserting new SRI for stream: "<<sid);
        streamid_to_sri.insert(std::make_pair(H.streamID, H));
    } else {
        // overwrite the SRI
        LOG_DEBUG(OutSDDSPort_customized,__PRETTY_FUNCTION__<<"Updating existing SRI for stream: "<<sid);
        sri_iter->second = H;
    }
    TRACE_EXIT(OutSDDSPort_customized);
}

template <class DATA_TYPE>
void OutSDDSPort_customized<DATA_TYPE>::pushPacket(std::vector<DATA_TYPE>& data, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID){
    TRACE_ENTER(OutSDDSPort_customized);
    LOG_TRACE(OutSDDSPort_customized,__PRETTY_FUNCTION__<<"streamID: "<<streamID<<" EOS:"<<EOS);
    boost::mutex::scoped_lock lock(input_port_lock);

    typename streamid_to_proc_map_t::iterator proc_iter = streamid_to_processor.find(streamID);
    if (proc_iter == streamid_to_processor.end()) {
        // no processor for this stream, SDDS must be disabled for this stream
        LOG_DEBUG(OutSDDSPort_customized,__PRETTY_FUNCTION__<<"No SDDS processor for stream "<<streamID<<", SDDS must be disabled for this stream.");
        TRACE_EXIT(OutSDDSPort_customized);
        return;
    }
    streamid_to_sri_map_t::iterator sri_iter = streamid_to_sri.find(streamID);
    if (sri_iter != streamid_to_sri.end()) {
        LOG_DEBUG(OutSDDSPort_customized,__PRETTY_FUNCTION__<<"Pushing packet with updated SRI for stream "<<streamID);
        proc_iter->second->dataIn( data, T, EOS, sri_iter->second);
        streamid_to_sri.erase(sri_iter);
    } else {
        LOG_DEBUG(OutSDDSPort_customized,__PRETTY_FUNCTION__<<"Pushing packet without SRI for stream "<<streamID);
        proc_iter->second->dataIn( data, T, EOS);
    }
    if (EOS) {
    	// Erase from streamID/processor map to make room for a potential new stream with same stream ID
    	streamid_to_processor.erase(proc_iter);
    }
    TRACE_EXIT(OutSDDSPort_customized);
}

/**
 * Since this is a templated class with a cpp and headerfile, the cpp
 * file must declare all the template types to generate. In this case it is
 * simply short port types.
 */
template class OutSDDSPort_customized<short>;
