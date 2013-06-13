/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file 
 * distributed with this source distribution.
 * 
 * This file is part of GNUHAWK.
 * 
 * GNUHAWK is free software: you can redistribute it and/or modify is under the 
 * terms of the GNU General Public License as published by the Free Software 
 * Foundation, either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * GNUHAWK is distributed in the hope that it will be useful, but WITHOUT ANY 
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more 
 * details.
 * 
 * You should have received a copy of the GNU General Public License along with 
 * this program.  If not, see http://www.gnu.org/licenses/.
 */


/**************************************************************************

    This is the component code. This file contains the child class where
    custom functionality can be added to the component. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

 	Source: USRP_UHD.spd.xml
 	Generated on: Wed Feb 29 10:32:42 EST 2012
 	Redhawk IDE
 	Version:T.1.8.0
 	Build id: v201202211631-r7419

 **************************************************************************/

#include "USRP_UHD.h"
PREPARE_LOGGING(USRP_UHD_i)


USRP_UHD_i::USRP_UHD_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl) :
USRP_UHD_base(devMgr_ior, id, lbl, sftwrPrfl)
{
	_constructor_();
}

USRP_UHD_i::USRP_UHD_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev) :
    				USRP_UHD_base(devMgr_ior, id, lbl, sftwrPrfl, compDev)
{
	_constructor_();
}

USRP_UHD_i::USRP_UHD_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities) :
    				USRP_UHD_base(devMgr_ior, id, lbl, sftwrPrfl, capacities)
{
	_constructor_();
}

USRP_UHD_i::USRP_UHD_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev) :
    				USRP_UHD_base(devMgr_ior, id, lbl, sftwrPrfl, capacities, compDev)
{
	_constructor_();
}
void USRP_UHD_i::_constructor_() {
	serviceThread_transmit = NULL;


	/**
	 * NOTE: The following code is an example of how one can de-register ports and re-instantiate with different classes. In this case
	 * 		 I have added port_impl_customized.* files that contain custom port code that is not overwritten during the REDHAWK code re-generation process.
	 * 		 In a majority of cases, the following is never done and the port_impl.* files are modified directly.
	 */


	// Releasing / De-registering all ports
	releaseInPorts();
	releaseOutPorts();

	// only have to delete the non-customized ports (due to the fact that when you re-register ports they delete any existing memory)
	delete USRP_UHD_base::dataShortComplex_in;
	delete USRP_UHD_base::dataFloatComplex_in;

	// Create new port instances
	DigitalTuner_in = new FRONTEND_DigitalTuner_In_i_customized("DigitalTuner_in", this);	/** CUSTOM PORT */
	RFInfo_in = new FRONTEND_RFInfo_In_i_customized("RFInfo_in", this);						/** CUSTOM PORT */
	dataShortComplex_in = new BULKIO_dataShort_In_i("dataShortComplex_in", this);
	dataFloatComplex_in = new BULKIO_dataFloat_In_i("dataFloatComplex_in", this);
	dataShort_out = new BULKIO_dataShort_Out_i_customized("dataShort_out", this);

	// Re-register all ports
	registerInPort(DigitalTuner_in);
	registerInPort(RFInfo_in);
	registerInPort(dataShortComplex_in);
	registerInPort(dataFloatComplex_in);
	registerOutPort(dataShort_out, dataShort_out->_this());

}

USRP_UHD_i::~USRP_UHD_i()
{
	for (size_t i = 0; i < tunerChannels.size(); i++) {
		if (tunerChannels[i].lock != NULL)
			delete tunerChannels[i].lock;
	}
	tunerChannels.clear();
}

void USRP_UHD_i::start() throw (CORBA::SystemException, CF::Resource::StartError) {
	USRP_UHD_base::start();

	// Create additional transmit thread
	try {
		boost::mutex::scoped_lock lock(serviceThreadTransmit_2);
		if (serviceThread_transmit == 0) {
			dataShortComplex_in->unblock();
			dataFloatComplex_in->unblock();
			serviceThread_transmit = new ProcessThread_2<USRP_UHD_i> (this, 0.1);
			serviceThread_transmit->start();
		}

		serviceThread->updateDelay(0.001);
		serviceThread_transmit->updateDelay(0.001);

	} catch (...) {
		stop();
		throw;
	}
}

void USRP_UHD_i::stop() throw (CORBA::SystemException, CF::Resource::StopError) {
	USRP_UHD_base::stop();

	boost::mutex::scoped_lock lock(serviceThreadTransmit_2);
	// release the child thread (if it exists)
	if (serviceThread_transmit != 0) {
		dataShortComplex_in->block();
		dataFloatComplex_in->block();
		if (!serviceThread_transmit->release(2)) {
			throw CF::Resource::StopError(CF::CF_NOTSET,"Transmit processing thread did not die");
		}
		delete serviceThread_transmit;
		serviceThread_transmit = 0;
	}
}

void USRP_UHD_i::initialize() throw (CF::LifeCycle::InitializeError, CORBA::SystemException)
{
	/** As of the REDHAWK 1.8.3 release, device are not started automatically by the node. Therefore
	 *  the device must start itself. */
	start();
}

/***********************************************************************************************

    Basic functionality:

        The service function is called by the serviceThread object (of type ProcessThread).
        This call happens immediately after the previous call if the return value for
        the previous call was NORMAL.
        If the return value for the previous call was NOOP, then the serviceThread waits
        an amount of time defined in the serviceThread's constructor.

    SRI:
        To create a StreamSRI object, use the following code:
        	stream_id = "";
	    	sri = BULKIO::StreamSRI();
	    	sri.hversion = 1;
	    	sri.xstart = 0.0;
	    	sri.xdelta = 0.0;
	    	sri.xunits = BULKIO::UNITS_TIME;
	    	sri.subsize = 0;
	    	sri.ystart = 0.0;
	    	sri.ydelta = 0.0;
	    	sri.yunits = BULKIO::UNITS_NONE;
	    	sri.mode = 0;
	    	sri.streamID = this->stream_id.c_str();

	Time:
	    To create a PrecisionUTCTime object, use the following code:
	        struct timeval tmp_time;
	        struct timezone tmp_tz;
	        gettimeofday(&tmp_time, &tmp_tz);
	        double wsec = tmp_time.tv_sec;
	        double fsec = tmp_time.tv_usec / 1e6;;
	        BULKIO::PrecisionUTCTime tstamp = BULKIO::PrecisionUTCTime();
	        tstamp.tcmode = BULKIO::TCM_CPU;
	        tstamp.tcstatus = (short)1;
	        tstamp.toff = 0.0;
	        tstamp.twsec = wsec;
	        tstamp.tfsec = fsec;

    Ports:

        Data is passed to the serviceFunction through the getPacket call (BULKIO only).
        The dataTransfer class is a port-specific class, so each port implementing the
        BULKIO interface will have its own type-specific dataTransfer.

        The argument to the getPacket function is a floating point number that specifies
        the time to wait in seconds. A zero value is non-blocking. A negative value
        is blocking.

        Each received dataTransfer is owned by serviceFunction and *MUST* be
        explicitly deallocated.

        To send data using a BULKIO interface, a convenience interface has been added 
        that takes a std::vector as the data input

        NOTE: If you have a BULKIO dataSDDS port, you must manually call 
              "port->updateStats()" to update the port statistics when appropriate.

        Example:
            // this example assumes that the component has two ports:
            //  A provides (input) port of type BULKIO::dataShort called short_in
            //  A uses (output) port of type BULKIO::dataFloat called float_out
            // The mapping between the port and the class is found
            // in the component base class header file

            BULKIO_dataShort_In_i::dataTransfer *tmp = short_in->getPacket(-1);
            if (not tmp) { // No data is available
                return NOOP;
            }

            std::vector<float> outputData;
            outputData.resize(tmp->dataBuffer.size());
            for (unsigned int i=0; i<tmp->dataBuffer.size(); i++) {
                outputData[i] = (float)tmp->dataBuffer[i];
            }

            // NOTE: You must make at least one valid pushSRI call
            if (tmp->sriChanged) {
                float_out->pushSRI(tmp->SRI);
            }
            float_out->pushPacket(outputData, tmp->T, tmp->EOS, tmp->streamID);

            delete tmp; // IMPORTANT: MUST RELEASE THE RECEIVED DATA BLOCK
            return NORMAL;

        Interactions with non-BULKIO ports are left up to the component developer's discretion

    Properties:

        Properties are accessed directly as member variables. For example, if the
        property name is "baudRate", it may be accessed within member functions as
        "baudRate". Unnamed properties are given a generated name of the form
        "prop_n", where "n" is the ordinal number of the property in the PRF file.
        Property types are mapped to the nearest C++ type, (e.g. "string" becomes
        "std::string"). All generated properties are declared in the base class
        (USRP_UHD_base).

        Simple sequence properties are mapped to "std::vector" of the simple type.
        Struct properties, if used, are mapped to C++ structs defined in the
        generated file "struct_props.h". Field names are taken from the name in
        the properties file; if no name is given, a generated name of the form
        "field_n" is used, where "n" is the ordinal number of the field.

        Example:
            // This example makes use of the following Properties:
            //  - A float value called scaleValue
            //  - A boolean called scaleInput

            if (scaleInput) {
                dataOut[i] = dataIn[i] * scaleValue;
            } else {
                dataOut[i] = dataIn[i];
            }


 ************************************************************************************************/

//*** serviceFunction = Receive thread *//
int USRP_UHD_i::serviceFunction() {
	bool rx_data = false;
	{
		exclusive_lock lock(propLock);
		for (size_t i = 0; i < tunerChannels.size(); i++) {
			//Check to see if channel is either not allocated, or the output is not enabled
			if (tunerChannels[i].control_allocation_id.empty() || tunerChannels[i].frontend_status->tuner_type != "RX_DIGITIZER")
				continue;

			exclusive_lock lock(*tunerChannels[i].lock);
			uhd::rx_metadata_t _metadata;
			size_t num_samps = usrpDevice->get_device()->recv(&tunerChannels[i].outputBuffer.front(), tunerChannels[i].outputBuffer.size() / 2, _metadata, uhd::io_type_t::COMPLEX_INT16, uhd::device::RECV_MODE_FULL_BUFF);
			//handle possible errors conditions
			switch (_metadata.error_code) {
				case uhd::rx_metadata_t::ERROR_CODE_NONE:
					break;
				case uhd::rx_metadata_t::ERROR_CODE_TIMEOUT:
					LOG_WARN(USRP_UHD_i,"WARNING: TIMEOUT OCCURED ON USRP RECEIVE!");
					continue;
				case uhd::rx_metadata_t::ERROR_CODE_OVERFLOW:
					LOG_WARN(USRP_UHD_i,"WARNING: USRP OVERFLOW DETECTED!");
					rx_data = true;
					break;
				default:
					LOG_WARN(USRP_UHD_i,"WARNING: UHD source block got error code 0x" << _metadata.error_code);
					continue;
			}

			// Ensure data was received
			if(num_samps > 0 )
				rx_data = true;
			else
				continue;

			// Updating Timestamps
			tunerChannels[i].outputBufferTime = BIO_HELPER::getSystemTimestamp();
			tunerChannels[i].outputBufferTime.twsec = _metadata.time_spec.get_real_secs();
			tunerChannels[i].outputBufferTime.tfsec = _metadata.time_spec.get_frac_secs();
			if (tunerChannels[i].timeUp.twsec <= 0)
				tunerChannels[i].timeUp = tunerChannels[i].outputBufferTime;
			tunerChannels[i].timeDown = tunerChannels[i].outputBufferTime;

			// Pushing Data
			std::string streamID = std::string(tunerChannels[i].sri.streamID);
			dataShort_out->pushPacket(& tunerChannels[i].outputBuffer[0], num_samps * 2, tunerChannels[i].outputBufferTime, false, streamID);

		}
	}

	if(rx_data)
		return NORMAL;
	return NOOP;

}

//*** serviceFunction_transmit = Transmit thread *//
int USRP_UHD_i::serviceFunction_transmit(){

	bool ret = singleService_transmit(dataShortComplex_in);
	ret = ret || singleService_transmit(dataFloatComplex_in);
	if(ret)
		return NORMAL;
	return NOOP;

}



/** A templated service function that is generic between data types. */
template <class IN_PORT_TYPE> bool USRP_UHD_i::singleService_transmit(IN_PORT_TYPE *dataIn) {

	typename IN_PORT_TYPE::dataTransfer *packet = dataIn->getPacket(0);
	if (packet == NULL)
		return false;

	if (packet->inputQueueFlushed){
		LOG_WARN(USRP_UHD_i,"Input Queue Flushed");
	}

	if (packet->SRI.mode != 1) {
		LOG_ERROR(USRP_UHD_i,"USRP device requires complex data.  Real data type received.");
		delete packet;
		return false;
	}

	//Sets basic data type. IE- float for float port, short for short port
	typedef typeof (packet->dataBuffer[0]) PACKET_ELEMENT_TYPE;

	for (size_t i = 0; i < tunerChannels.size(); i++) {
		//Check to see if wideband channel is either not allocated, or the output is not enabled
		if (tunerChannels[i].control_allocation_id.empty() || tunerChannels[i].frontend_status->tuner_type != "TX")
			continue;

		//Lock the tuner
		exclusive_lock lock(*tunerChannels[i].lock);
		uhd::tx_metadata_t _metadata;
		_metadata.start_of_burst = false;
		_metadata.end_of_burst = false;

		// Send in size/2 because it is complex
		if (sizeof (PACKET_ELEMENT_TYPE) == 2){
			if( usrpDevice->get_device()->send(&packet->dataBuffer.front(), packet->dataBuffer.size() / 2, _metadata, uhd::io_type_t::COMPLEX_INT16, uhd::device::SEND_MODE_FULL_BUFF) != packet->dataBuffer.size() / 2)
				LOG_WARN(USRP_UHD_i, "WARNING: THE USRP WAS UNABLE TO TRANSMIT " << packet->dataBuffer.size() / 2 << " NUMBER OF COMPLEX SHORT SAMPLES!");
		}
		else if (sizeof (PACKET_ELEMENT_TYPE) == 4){
			if( usrpDevice->get_device()->send(&packet->dataBuffer.front(), packet->dataBuffer.size() / 2, _metadata, uhd::io_type_t::COMPLEX_FLOAT32, uhd::device::SEND_MODE_FULL_BUFF) != packet->dataBuffer.size() / 2)
				LOG_WARN(USRP_UHD_i, "WARNING: THE USRP WAS UNABLE TO TRANSMIT " << packet->dataBuffer.size() / 2 << " NUMBER OF COMPLEX FLOAT SAMPLES!");
		}
	}

	//Delete Memory
	delete packet;
	return true;
}



std::string USRP_UHD_i::create_allocation_id_csv(size_t tunerNum){
	std::string alloc_id_csv = "";
	for(string_number_mapping::iterator it = allocationId_to_tunerNum.begin(); it != allocationId_to_tunerNum.end(); it++){
		if(it->second == tunerNum)
			alloc_id_csv += it->first + ",";
	}
	if(!alloc_id_csv.empty())
		alloc_id_csv.erase(alloc_id_csv.size()-1);
	return alloc_id_csv;
}



/** NOTE: This device has not been updated to register configure() call-back functions for specific properties. */
void USRP_UHD_i::configure(const CF::Properties& props) throw (CORBA::SystemException, CF::PropertySet::InvalidConfiguration, CF::PropertySet::PartialConfiguration) {
	LOG_TRACE(USRP_UHD_i,__PRETTY_FUNCTION__);
	exclusive_lock lock(propLock);

	USRP_UHD_base::configure(props);
	try{
		bool update_gain = false;
		bool reconnect_usrp = false;
		// Specific Events on Property Changes
		for (CORBA::ULong ii = 0; ii < props.length(); ++ii) {
			const std::string id = (const char*) props[ii].id;
			// Initialize the USRP if the IP has changed
			if (id == "USRP_ip_address") {
				reconnect_usrp = true;
			}
			else if (id == "gain_global") {
				update_gain = true;
			}
			else if (id == "group_id_global"){
				update_group_id(group_id_global);
			}
		}

		// Refresh Available USRP's Found on the Network
		if (update_available_usrp_seq) {
			update_available_usrp_seq = false;
			updateAvaiableUsrpSeq();
		}

		if(reconnect_usrp || usrpDevice.get() == NULL){
			reconnect_usrp = false;
			try{
				init_usrp(USRP_ip_address);
			}catch(...){
				LOG_WARN(USRP_UHD_i,"CAUGHT EXCEPTION WHEN INITIALIZING USRP. WAITING 1 SECOND AND TRYING AGAIN");
				sleep(1);
				init_usrp(USRP_ip_address);
			}
		}

		if(update_gain){
			for(size_t i = 0; i < tunerChannels.size(); i++){
				if(tunerChannels[i].frontend_status->tuner_type == "RX_DIGITIZER"){
					size_t usrp_channel_number = myDevice_channel_seq[i].chan_num;
					usrpDevice->set_rx_gain(gain_global,usrp_channel_number);
					tunerChannels[i].frontend_status->gain = usrpDevice->get_rx_gain(usrp_channel_number);
				}
			}
		}

		// Reference Clock
		if (clock_ref == "MIMO") {
			uhd::clock_config_t clock_config;
			clock_config.ref_source = uhd::clock_config_t::REF_MIMO;
			clock_config.pps_source = uhd::clock_config_t::PPS_MIMO;
			usrpDevice->set_clock_config(clock_config, 0);
		} else if (clock_ref == "EXTERNAL") {
			usrpDevice->set_clock_config(uhd::clock_config_t::external(), 0);
		} else if (clock_ref == "INTERNAL") {
			usrpDevice->set_clock_config(uhd::clock_config_t::internal(), 0);
		}
	}catch(...){
		LOG_ERROR(USRP_UHD_i,"Unable to initialize USRP!");
		throw CF::PropertySet::InvalidConfiguration("Unable to initialize USRP based on these properties",props);
	}

}

void USRP_UHD_i::init_usrp(std::string ip_addr) throw (CF::PropertySet::InvalidConfiguration) {
	LOG_TRACE(USRP_UHD_i,__PRETTY_FUNCTION__);
	try {
		device_addr = uhd::device_addr_t();
		device_addr["addr0"] = USRP_ip_address;
		usrpDevice = uhd::usrp::multi_usrp::make(device_addr);

		// get_rx/tx_freq will throw an exception if set_rx/tx_freq has not already been called
		for (size_t chan = 0; chan < usrpDevice->get_rx_num_channels(); chan++) {
			double setFreq = (usrpDevice->get_rx_freq_range(chan).start() + usrpDevice->get_rx_freq_range(chan).stop()) / 2;
			usrpDevice->set_rx_freq(setFreq, chan);
		}
		for (size_t chan = 0; chan < usrpDevice->get_tx_num_channels(); chan++) {
			double setFreq = (usrpDevice->get_tx_freq_range(chan).start() + usrpDevice->get_tx_freq_range(chan).stop()) / 2;
			usrpDevice->set_tx_freq(setFreq, chan);
		}

		struct timeval tmp_time;
		struct timezone tmp_tz;
		gettimeofday(&tmp_time, &tmp_tz);
		time_t wsec = tmp_time.tv_sec;
		double fsec = tmp_time.tv_usec / 1e6;
		usrpDevice->set_time_now(uhd::time_spec_t(wsec,fsec));

		// This will update property structures that describe the USRP device (motherboard + daughtercards)
		update_myDeviceSeq();

		// Initialize tasking and status vectors
		frontend_tuner_status.resize(myDevice_channel_seq.size());
		tunerChannels.resize(myDevice_channel_seq.size());

		//Initialize Data Members
		char tmp[128];
		for (size_t i = 0; i < tunerChannels.size(); i++) {
			tunerChannels[i].lock = new boost::mutex;
			tunerChannels[i].frontend_status = &frontend_tuner_status[i];
			tunerChannels[i].reset();
			tunerChannels[i].frontend_status->tuner_type = myDevice_channel_seq[i].tuner_type;

			sprintf(tmp,"%.2f-%.2f",myDevice_channel_seq[i].freq_min,myDevice_channel_seq[i].freq_max);
			tunerChannels[i].frontend_status->available_frequency = std::string(tmp);
			sprintf(tmp,"%.2f-%.2f",myDevice_channel_seq[i].gain_min,myDevice_channel_seq[i].gain_max);
			tunerChannels[i].frontend_status->available_gain = std::string(tmp);
			sprintf(tmp,"%.2f-%.2f",0.0,myDevice_channel_seq[i].rate_max);
			tunerChannels[i].frontend_status->available_sample_rate = std::string(tmp);
		}
	} catch (...) {
		LOG_ERROR(USRP_UHD_i,"USRP COULD NOT BE INITIALIZED!");
		throw CF::PropertySet::InvalidConfiguration();
	}

}


void USRP_UHD_i::update_rf_flow_id(std::string rfFlowId){
	rf_flow_id = rfFlowId;

	for(size_t i = 0; i < tunerChannels.size(); i++){
		if(tunerChannels[i].frontend_status->tuner_type == "RX_DIGITIZER"){
			tunerChannels[i].frontend_status->rf_flow_id = rf_flow_id;
		}
		else if(tunerChannels[i].frontend_status->tuner_type == "TX"){
			tunerChannels[i].frontend_status->rf_flow_id.clear();
		}
		else{
			LOG_WARN(USRP_UHD_i, std::string(__PRETTY_FUNCTION__) + ":: UNKNOWN TUNER TYPE: " + std::string(myDevice_channel_seq[i].tuner_type));
		}
	}

}

void USRP_UHD_i::update_group_id(std::string groupID){
	group_id_global = groupID;
	for(size_t i = 0; i < tunerChannels.size(); i++){
		tunerChannels[i].frontend_status->group_id = groupID;
	}

}


/*****************************************************************/
/* Allocation/Deallocation of Capacity                           */
/*****************************************************************/
CF::Device::UsageType USRP_UHD_i::updateUsageState() {
	size_t tunerAllocated = 0;
	for (size_t i = 0; i < tunerChannels.size(); i++) {
		if (!tunerChannels[i].control_allocation_id.empty())
			tunerAllocated++;
	}
	// If no tuners are allocated, device is idle
	if (tunerAllocated == 0)
		return CF::Device::IDLE;
	// If all tuners are allocated, device is busy
	if (tunerAllocated == tunerChannels.size())
		return CF::Device::BUSY;
	// Else, device is active
	return CF::Device::ACTIVE;
}


CORBA::Boolean USRP_UHD_i::allocateCapacity(const CF::Properties & capacities)
throw (CORBA::SystemException, CF::Device::InvalidCapacity, CF::Device::InvalidState) {
	LOG_TRACE(USRP_UHD_i,__PRETTY_FUNCTION__);
	CORBA::ULong ii;
	try{
		for (ii = 0; ii < capacities.length(); ++ii) {
			const std::string id = (const char*) capacities[ii].id;
			PropertyInterface* property = getPropertyFromId(id);
			if(!property)
				throw CF::Device::InvalidCapacity("UNKNOWN PROPERTY", capacities);
			property->setValue(capacities[ii].value);
			if (id == "FRONTEND::tuner_allocation"){
				LOG_TRACE(USRP_UHD_i,"ATTEMPTING ALLOCATION FOR TUNER TYPE: " << frontend_tuner_allocation.tuner_type);
				if(frontend_tuner_allocation.tuner_type == "RX_DIGITIZER" || frontend_tuner_allocation.tuner_type == "TX") {

					long tunerNum = addTunerMapping(frontend_tuner_allocation);
					if (tunerNum < 0)
						throw CF::Device::InvalidCapacity();

					// Initialize the tuner
					try{
						setupTuner(tunerNum, frontend_tuner_allocation);
					}catch(const std::logic_error &e) {
						LOG_TRACE(USRP_UHD_i, "SETUP TUNER FAILED WITH ERROR: " << e.what());
						throw e;
					};
				}
				else{
					throw CF::Device::InvalidCapacity("UNKNOWN FRONTEND TUNER TYPE", capacities);
				}
			}
			else if (id == "FRONTEND::listener_allocation") {
				if(addTunerMapping(frontend_listener_allocation) < 0)
					throw CF::Device::InvalidCapacity("UNKNOWN CONTROL ALLOCATION ID PROPERTY", capacities);
			}
			else {
				throw CF::Device::InvalidCapacity("UNKNOWN ALLOCATION PROPERTY", capacities);
			}
		}
	}
	catch(...){
		LOG_DEBUG(USRP_UHD_i,"ALLOCATION FAILED FOR USRP. DEALLOCATING...");
		deallocateCapacity(capacities);
		return false;
	};
	LOG_TRACE(USRP_UHD_i,"ALLOCATE CAPACITY RETURNING TRUE");

	return true;
}

void USRP_UHD_i::deallocateCapacity(const CF::Properties & capacities)
throw (CORBA::SystemException, CF::Device::InvalidCapacity, CF::Device::InvalidState) {
	LOG_TRACE(USRP_UHD_i,__PRETTY_FUNCTION__);
	// Returns values for valid queries in the same order as requested
	for (CORBA::ULong ii = 0; ii < capacities.length(); ++ii) {
		try{
			const std::string id = (const char*) capacities[ii].id;
			PropertyInterface* property = getPropertyFromId(id);
			if(!property)
				throw CF::Device::InvalidCapacity("UNKNOWN PROPERTY", capacities);
			property->setValue(capacities[ii].value);
			if (id == "FRONTEND::tuner_allocation"){
				// Try to remove control of the device
				long tunerNum = getTunerMapping(frontend_tuner_allocation.allocation_id);
				if (tunerNum < 0)
					throw CF::Device::InvalidState();
				if(tunerChannels[tunerNum].control_allocation_id == frontend_tuner_allocation.allocation_id){
					removeTuner(tunerNum);
				}
				removeTunerMapping(frontend_tuner_allocation.allocation_id);
			}
			else if (id == "FRONTEND::listener_allocation") {
				removeTunerMapping(frontend_listener_allocation.listener_allocation_id);
			}
			else {
				LOG_TRACE(USRP_UHD_i,"WARNING: UNKNOWN ALLOCATION PROPERTY \""+ std::string(property->name) + "\". IGNORING!");
			}
		}
		catch(...){
			LOG_DEBUG(USRP_UHD_i,"ERROR WHEN DEALLOCATING. SKIPPING...");
		}
	}


}


/*****************************************************************/
/* Tuner Configurations - Wideband and Narrowband                */

/*****************************************************************/

bool USRP_UHD_i::setupTuner(size_t tuner_id, const frontend_tuner_allocation_struct& tuner_req) throw (std::logic_error) {
	LOG_TRACE(USRP_UHD_i,__PRETTY_FUNCTION__);
	if(tuner_req.allocation_id != tunerChannels[tuner_id].control_allocation_id)
		return false;


	// If the freq has changed (change in stream) or the tuner is disabled, then set it as disabled
	indivTuner* tuner_curr = &tunerChannels[tuner_id];
	bool isTunerEnabled = tuner_curr->frontend_status->enabled;
	if (!isTunerEnabled || tuner_curr->frontend_status->center_frequency != tuner_req.center_frequency)
		enableTuner(tuner_id, false);
	{
		exclusive_lock lock(* tuner_curr->lock);
		if(tuner_req.group_id != group_id_global )
			throw std::logic_error("setupTuner(): CAN NOT ALLOCATE A TUNER WITH THAT GROUP ID!");

		if(!tuner_req.rf_flow_id.empty() && tuner_req.rf_flow_id != tunerChannels[tuner_id].frontend_status->rf_flow_id )
			throw std::logic_error("setupTuner(): CAN NOT ALLOCATE A TUNER WITH RF FLOW ID = " + tuner_req.rf_flow_id + " !");

		//Check Validity
		if (tuner_req.center_frequency < myDevice_channel_seq[tuner_id].freq_min || tuner_req.center_frequency > myDevice_channel_seq[tuner_id].freq_max)
			throw std::logic_error("setupTuner(): INVALID FREQUENCY");
		if (tuner_curr->frontend_status->gain < myDevice_channel_seq[tuner_id].gain_min || tuner_curr->frontend_status->gain > myDevice_channel_seq[tuner_id].gain_max)
			throw std::logic_error("setupTuner(): INVALID GAIN");
		if (tuner_req.bandwidth <= 0)
			throw std::logic_error("setupTuner(): INVALID BANDWIDTH");
		if (tuner_req.sample_rate <= 0)
			throw std::logic_error("setupTuner(): INVALID RATE");

		size_t usrp_channel_number = myDevice_channel_seq[tuner_id].chan_num;
		if (tunerChannels[tuner_id].frontend_status->tuner_type == "RX_DIGITIZER") {
			usrpDevice->set_rx_bandwidth(tuner_req.bandwidth, usrp_channel_number);
			usrpDevice->set_rx_freq(tuner_req.center_frequency, usrp_channel_number);
			double opt_sr = optimize_rate(tuner_req.sample_rate,myDevice_channel_seq[tuner_id].rate_max,1/*myDevice_channel_seq[tuner_id].rate_min*/);	//min rate is not always published correctly
			usrpDevice->set_rx_rate(opt_sr, usrp_channel_number);
			usrpDevice->set_rx_gain(tuner_curr->frontend_status->gain, usrp_channel_number);
			update_myDeviceSeq();
			// Set the values - do not just set tuner_status[tuner_id] just to ensure allocation ID does not get overwritten
			tuner_curr->frontend_status->center_frequency = usrpDevice->get_rx_freq(usrp_channel_number);
			tuner_curr->frontend_status->bandwidth = usrpDevice->get_rx_bandwidth(usrp_channel_number);
			tuner_curr->frontend_status->sample_rate = usrpDevice->get_rx_rate(usrp_channel_number);
			tuner_curr->frontend_status->gain = usrpDevice->get_rx_gain(usrp_channel_number);

		} else if (tunerChannels[tuner_id].frontend_status->tuner_type== "TX") {
			usrpDevice->set_tx_bandwidth(tuner_req.bandwidth, usrp_channel_number);
			usrpDevice->set_tx_freq(tuner_req.center_frequency, usrp_channel_number);
			usrpDevice->set_tx_rate(tuner_req.sample_rate, usrp_channel_number);
			usrpDevice->set_tx_gain(tuner_curr->frontend_status->gain, usrp_channel_number);

			update_myDeviceSeq();

			// Set the values - do not just set tuner_status[tuner_id] just to ensure allocation ID does not get overwritten
			tuner_curr->frontend_status->center_frequency = usrpDevice->get_tx_freq(usrp_channel_number);
			tuner_curr->frontend_status->bandwidth = usrpDevice->get_tx_bandwidth(usrp_channel_number);
			tuner_curr->frontend_status->sample_rate = usrpDevice->get_tx_rate(usrp_channel_number);
			tuner_curr->frontend_status->gain = usrpDevice->get_tx_gain(usrp_channel_number);
		} else {
			throw std::logic_error("setupTuner(): INVALID TUNER TYPE. MUST BE RX OR TX!");
		}
		if(tuner_curr->frontend_status->tuner_type != "TX" && (tuner_curr->frontend_status->bandwidth < tuner_req.bandwidth ||
		   tuner_curr->frontend_status->bandwidth > tuner_req.bandwidth+tuner_req.bandwidth * tuner_req.bandwidth_tolerance/100.0 )){
			char msg[512];
			sprintf(msg,"setupTuner(%d): returned bw \"%f\" does not meet tolerance criteria of \"%f + %f percent\". ",tuner_id, tuner_curr->frontend_status->bandwidth,tuner_req.bandwidth,tuner_req.bandwidth_tolerance);
        	throw std::logic_error(msg);
        }
		if(tuner_curr->frontend_status->sample_rate < tuner_req.sample_rate ||
				tuner_curr->frontend_status->sample_rate > tuner_req.sample_rate+tuner_req.sample_rate * tuner_req.sample_rate_tolerance/100.0 ){
			char msg[512];
			sprintf(msg,"setupTuner(%d): returned sample rate \"%f\" does not meet tolerance criteria of \"%f + %f percent\". ",tuner_id, tuner_curr->frontend_status->sample_rate,tuner_req.sample_rate,tuner_req.sample_rate_tolerance);
        	throw std::logic_error(msg);
		}

	}

	if (isTunerEnabled)
		enableTuner(tuner_id, true);

	return true;

}

bool USRP_UHD_i::removeTuner(size_t tuner_id) {
	LOG_TRACE(USRP_UHD_i,__PRETTY_FUNCTION__);
	enableTuner(tuner_id, false);
	tunerChannels[tuner_id].reset();
	return true;
}

bool USRP_UHD_i::enableTuner(size_t tuner_id, bool enable) {
	// Lock the tuner
	exclusive_lock lock(*tunerChannels[tuner_id].lock);

	bool prev_enabled = tunerChannels[tuner_id].frontend_status->enabled;
	tunerChannels[tuner_id].frontend_status->enabled = enable;

	// If going from disabled to enabled
	if (!prev_enabled && enable) {
		// Start Streaming Now
		if(tunerChannels[tuner_id].frontend_status->tuner_type != "TX"){
			usrpDevice->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS, tuner_id);
		}

		configureTunerSRI(& tunerChannels[tuner_id].sri, tunerChannels[tuner_id].frontend_status->center_frequency, tunerChannels[tuner_id].frontend_status->sample_rate, 1,tunerChannels[tuner_id].frontend_status->rf_flow_id);
		streamID_to_tunerNum.insert(std::make_pair(std::string(tunerChannels[tuner_id].sri.streamID), tuner_id));
		dataShort_out->pushSRI(tunerChannels[tuner_id].sri);

	}


	// If going from enabled to disabled
	if (prev_enabled && !enable && !std::string(tunerChannels[tuner_id].sri.streamID).empty()) {

		// Stop Streaming Now
		if(tunerChannels[tuner_id].frontend_status->tuner_type != "TX"){
			usrpDevice->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS,tuner_id);
		}

		std::string streamID = std::string(tunerChannels[tuner_id].sri.streamID);
		updateSriTimes(& tunerChannels[tuner_id].sri, tunerChannels[tuner_id].timeUp.twsec, tunerChannels[tuner_id].timeDown.twsec, J1970);
		dataShort_out->pushSRI(tunerChannels[tuner_id].sri);
		dataShort_out->pushPacket(tunerChannels[tuner_id].outputBuffer, tunerChannels[tuner_id].outputBufferTime, true, streamID);
		streamID_to_tunerNum.erase(streamID);

		BIO_HELPER::zeroSRI(& tunerChannels[tuner_id].sri);
		BIO_HELPER::zeroTime(& tunerChannels[tuner_id].outputBufferTime);
		BIO_HELPER::zeroTime(& tunerChannels[tuner_id].timeUp);
		BIO_HELPER::zeroTime(& tunerChannels[tuner_id].timeUp);
	}

	return true;

}


////////////////////////////
//        MAPPING         //
////////////////////////////

long USRP_UHD_i::addTunerMapping(const frontend_tuner_allocation_struct & frontend_alloc){
	long NO_VALID_TUNER = -1;

	// Do not allocate if allocation ID has already been used
	if(getTunerMapping(frontend_alloc.allocation_id) >= 0)
		return NO_VALID_TUNER;

	// Next, try to allocate a new tuner
	exclusive_lock lock(allocationID_MappingLock);
	for (long tunerNum = 0; tunerNum < long(frontend_tuner_status.size()); tunerNum++) {
		if(tunerChannels[tunerNum].frontend_status->tuner_type != frontend_alloc.tuner_type)
			continue;

		bool freq_valid = is_freq_valid(frontend_alloc.center_frequency, frontend_alloc.bandwidth, frontend_alloc.sample_rate,
				frontend_tuner_status[tunerNum].center_frequency, frontend_tuner_status[tunerNum].bandwidth, frontend_tuner_status[tunerNum].sample_rate);
		//listen
		if (!frontend_alloc.device_control && ! tunerChannels[tunerNum].control_allocation_id.empty() && freq_valid){
			allocationId_to_tunerNum.insert(std::pair<std::string, size_t > (frontend_alloc.allocation_id, tunerNum));
			tunerChannels[tunerNum].frontend_status->allocation_id_csv = create_allocation_id_csv(tunerNum);
			return tunerNum;
		}
		//control
		else if (frontend_alloc.device_control && tunerChannels[tunerNum].control_allocation_id.empty()){
			tunerChannels[tunerNum].control_allocation_id = frontend_alloc.allocation_id;
			allocationId_to_tunerNum.insert(std::pair<std::string, size_t > (frontend_alloc.allocation_id, tunerNum));
			tunerChannels[tunerNum].frontend_status->allocation_id_csv = create_allocation_id_csv(tunerNum);
			return tunerNum;
		}

	}
	return NO_VALID_TUNER;
}


long USRP_UHD_i::addTunerMapping(const frontend_listener_allocation_struct & frontend_listner_alloc){

	long NO_VALID_TUNER = -1;

	// Do not allocate if allocation ID has already been used
	int tuner_num = NO_VALID_TUNER;
	if ((tuner_num = getTunerMapping(frontend_listner_alloc.existing_allocation_id)) < 0)
		return NO_VALID_TUNER;

	allocationId_to_tunerNum.insert(std::pair<std::string, size_t > (frontend_listner_alloc.listener_allocation_id, tuner_num));
	tunerChannels[tuner_num].frontend_status->allocation_id_csv = create_allocation_id_csv(tuner_num);
	return tuner_num;
}


long USRP_UHD_i::getTunerMapping(std::string allocation_id) {
	long NO_VALID_TUNER = -1;

	exclusive_lock lock(allocationID_MappingLock);
	string_number_mapping::iterator iter = allocationId_to_tunerNum.find(allocation_id);
	if (iter != allocationId_to_tunerNum.end())
		return iter->second;

	return NO_VALID_TUNER;
}


bool USRP_UHD_i::removeTunerMapping(std::string allocation_id) {
	exclusive_lock lock(allocationID_MappingLock);
	if(allocationId_to_tunerNum.erase(allocation_id) > 0)
		return true;
	return false;

}

bool USRP_UHD_i::is_connectionID_valid_for_tunerNum(const size_t & tunerNum, const std::string & connectionID) {
	std::map<std::string, size_t>::iterator iter =  allocationId_to_tunerNum.find(connectionID);
	if(iter == allocationId_to_tunerNum.end())
		return false;
	if(iter->second != tunerNum)
		return false;
	return true;
}


bool USRP_UHD_i::is_connectionID_valid_for_streamID(const std::string & streamID, const std::string & connectionID) {
	string_number_mapping::iterator iter = streamID_to_tunerNum.find(streamID);
	if (iter == streamID_to_tunerNum.end())
		return false;
	return is_connectionID_valid_for_tunerNum(iter->second, connectionID);
}

bool USRP_UHD_i::is_freq_valid(double req_cf, double req_bw, double req_sr, double cf, double bw, double sr){
	double req_min_bw_sr = std::min(req_bw,req_sr);
	double min_bw_sr = std::min(bw,sr);
	if( (req_cf + req_min_bw_sr/2 <= cf + min_bw_sr/2) && (req_cf - req_min_bw_sr/2 >= cf - min_bw_sr/2) )
		return true;
	return false;
};




////////////////////////////
//          SRI           //
////////////////////////////

void USRP_UHD_i::configureTunerSRI(BULKIO::StreamSRI *sri, double frequency, double sample_rate, short mode, std::string rf_flow_id) {
	if (sri == NULL)
		return;

	// Create New UUID
	uuid_t new_random_uuid;
	uuid_generate_random(new_random_uuid);
	char new_random_uuid_str[16];
	uuid_unparse(new_random_uuid, new_random_uuid_str);

	//Convert CenterFreq into string
	long centerFreq = long(frequency);
	char centerFreq_str[16];
	sprintf(centerFreq_str, "%ld", centerFreq);

	//Create new streamID
	std::string streamID = "tuner_freq_" + std::string(centerFreq_str) + "_Hz_" + std::string(new_random_uuid_str);

	sri->mode = mode;
	sri->hversion = 0;
	sri->xstart = 0;
	sri->xdelta = (1.0 / (sample_rate));
	sri->subsize = long(sample_rate);
	sri->xunits = 1;
	sri->ystart = 0;
	sri->ydelta = 0.001;
	sri->yunits = 1;
	sri->streamID = CORBA::string_dup(streamID.c_str());
	sri->blocking = false;

	addModifyKeyword<CORBA::Double > (sri, "COL_RF", CORBA::Double(centerFreq));
	addModifyKeyword<CORBA::Double > (sri, "CHAN_RF", CORBA::Double(centerFreq));
	addModifyKeyword<std::string> (sri,"FRONTEND::RF_FLOW_ID",rf_flow_id);
	addModifyKeyword<CORBA::Double> (sri,"FRONTEND::BANDWIDTH", CORBA::Double(sample_rate)); // for now, just set bw = sample rate
	addModifyKeyword<std::string> (sri,"FRONTEND::DEVICE_ID",std::string(identifier()));
}

void USRP_UHD_i::updateSriTimes(BULKIO::StreamSRI *sri, double timeUp, double timeDown, timeTypes timeType) {


	std::stringstream DOIU;
	std::stringstream TUOI;
	std::stringstream DOID;
	std::stringstream TDOI;

	time_t time_up = (time_t) timeUp;
	time_t time_down = (time_t) timeDown;
	time_t now;
	struct tm curr_time;
	curr_time.tm_year = 0;
	struct tm gmt_up;
	struct tm gmt_down;

	gmt_up = *gmtime(&time_up);
	gmt_down = *gmtime(&time_down);
	if (timeType == JCY) {
		now = time(0);
		curr_time = *gmtime(&now);
	}

	DOIU.width(4);
	DOIU.fill('0');
	if (timeType == JCY)
		DOIU << (curr_time.tm_year + 1900);
	else
		DOIU << (gmt_up.tm_year + 1900);
	DOIU.width(2);
	DOIU.fill('0');
	DOIU << (gmt_up.tm_mon + 1);
	DOIU.width(2);
	DOIU.fill('0');
	DOIU << gmt_up.tm_mday;

	TUOI.width(2);
	TUOI.fill('0');
	TUOI << gmt_up.tm_hour;
	TUOI.width(2);
	TUOI.fill('0');
	TUOI << gmt_up.tm_min;
	TUOI.width(2);
	TUOI.fill('0');
	TUOI << gmt_up.tm_sec;

	DOID.width(4);
	DOID.fill('0');
	DOID << (gmt_down.tm_year + 1900);
	DOID.width(2);
	DOID.fill('0');
	DOID << (gmt_down.tm_mon + 1);
	DOID.width(2);
	DOID.fill('0');
	DOID << gmt_down.tm_mday;

	TDOI.width(2);
	TDOI.fill('0');
	TDOI << gmt_down.tm_hour;
	TDOI.width(2);
	TDOI.fill('0');
	TDOI << gmt_down.tm_min;
	TDOI.width(2);
	TDOI.fill('0');
	TDOI << gmt_down.tm_sec;


	addModifyKeyword<std::string > (sri, "DOIU", DOIU.str());
	addModifyKeyword<std::string > (sri, "TUOI", TUOI.str());
	addModifyKeyword<std::string > (sri, "DOID", DOID.str());
	addModifyKeyword<std::string > (sri, "TDOI", TDOI.str());

}


////////////////////////////
//   UPDATE SEQUENCES     //
////////////////////////////

void USRP_UHD_i::updateAvaiableUsrpSeq() {
	// EMPTY SEQ
	available_usrp_seq.clear();
	uhd::device_addr_t himt;
	uhd::device_addrs_t device_addrs = uhd::device::find(himt);
	if (device_addrs.empty())
		LOG_WARN(USRP_UHD_i, "WARNING: NO UHD (USRP) DEVICES FOUND!\n");
	for (size_t i = 0; i < device_addrs.size(); i++) {
		usrp_device_struct_struct availDev;

		BOOST_FOREACH(std::string key, device_addrs[i].keys()) {
			if (key == "type")
				availDev.type = device_addrs[i].get(key);
			else if (key == "addr")
				availDev.ip_address = device_addrs[i].get(key);
			else if (key == "name")
				availDev.name = device_addrs[i].get(key);
			else if (key == "serial")
				availDev.serial = device_addrs[i].get(key);
		}
		available_usrp_seq.push_back(availDev);
	}

}

void USRP_UHD_i::update_myDeviceSeq() {
	myDevice_channel_seq.clear();
	myDevice_motherboard_seq.clear();
	for (size_t mthr = 0; mthr < usrpDevice->get_num_mboards(); mthr++) {
		usrp_motherboard_struct_struct availMotherboard;
		availMotherboard.mb_name = usrpDevice->get_mboard_name(mthr);
		availMotherboard.mb_ip = "[NEED TO IMPLEMENT]";
		myDevice_motherboard_seq.push_back(availMotherboard);

	}
	for (size_t chan = 0; chan < usrpDevice->get_rx_num_channels(); chan++) {
		usrp_channel_struct_struct availChan;
		availChan.ch_name = usrpDevice->get_rx_subdev_name(chan);
		availChan.tuner_type = "RX_DIGITIZER";
		if(availChan.ch_name.find("unknown") != std::string::npos)
			availChan.tuner_type = "UNKNOWN";
		availChan.antenna = usrpDevice->get_rx_antenna(chan);
		availChan.bandwidth = usrpDevice->get_rx_bandwidth(chan);
		availChan.ch_name = usrpDevice->get_rx_subdev_name(chan);
		availChan.rate_current = usrpDevice->get_rx_rate(chan);
		std::vector<double> rates = usrpDevice->get_rx_dboard_iface(chan)->get_clock_rates(uhd::usrp::dboard_iface::UNIT_RX);
		availChan.rate_min = rates.back();
		availChan.rate_max = rates.front();

		availChan.freq_current = usrpDevice->get_rx_freq(chan);
		availChan.freq_min = usrpDevice->get_rx_freq_range(chan).start();
		availChan.freq_max = usrpDevice->get_rx_freq_range(chan).stop();
		availChan.gain_current = usrpDevice->get_rx_gain(chan);
		availChan.gain_min = usrpDevice->get_rx_gain_range(chan).start();
		availChan.gain_max = usrpDevice->get_rx_gain_range(chan).stop();
		availChan.chan_num = chan;
		myDevice_channel_seq.push_back(availChan);
	}
	for (size_t chan = 0; chan < usrpDevice->get_tx_num_channels(); chan++) {
		usrp_channel_struct_struct availChan;
		availChan.ch_name = usrpDevice->get_tx_subdev_name(chan);
		availChan.tuner_type = "TX";
		if(availChan.ch_name.find("unknown") != std::string::npos)
			availChan.tuner_type = "UNKNOWN";
		availChan.antenna = usrpDevice->get_tx_antenna(chan);
		availChan.bandwidth = usrpDevice->get_tx_bandwidth(chan);
		availChan.rate_current = usrpDevice->get_tx_rate(chan);
		std::vector<double> rates = usrpDevice->get_tx_dboard_iface(chan)->get_clock_rates(uhd::usrp::dboard_iface::UNIT_TX);
		availChan.rate_min = rates.back();
		availChan.rate_max = rates.front();
		availChan.freq_current = usrpDevice->get_tx_freq(chan);
		availChan.freq_min = usrpDevice->get_tx_freq_range(chan).start();
		availChan.freq_max = usrpDevice->get_tx_freq_range(chan).stop();
		availChan.gain_current = usrpDevice->get_tx_gain(chan);
		availChan.gain_min = usrpDevice->get_tx_gain_range(chan).start();
		availChan.gain_max = usrpDevice->get_tx_gain_range(chan).stop();
		availChan.chan_num = chan;
		myDevice_channel_seq.push_back(availChan);
	}

}









