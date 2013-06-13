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


#ifndef USRP_UHD_IMPL_H
#define USRP_UHD_IMPL_H

#include "USRP_UHD_base.h"
#include <uhd/usrp/multi_usrp.hpp>
#include <uuid/uuid.h>
#include <iostream>
#include <complex>
#include <exception>
#include <set>
#include <stdexcept>
#include "port_impl_customized.h"

class USRP_UHD_i;


/*********************************************************************************************/
/**************************             UUID_HELPER                 **************************/
/*********************************************************************************************/
namespace UUID_HELPER {
    inline std::string new_uuid() {
        uuid_t new_random_uuid;
        uuid_generate_random(new_random_uuid);
        char new_random_uuid_str[37];
        uuid_unparse(new_random_uuid, new_random_uuid_str);
        return std::string(new_random_uuid_str);
    }
}
using namespace UUID_HELPER;


/*********************************************************************************************/
/**************************             BULKIO HELPERS              **************************/
/*********************************************************************************************/
namespace BIO_HELPER{

	inline void zeroSRI(BULKIO::StreamSRI *sri) {
		sri->hversion = 1;
		sri->xstart = 0.0;
		sri->xdelta = 1.0;
		sri->xunits = 1;
		sri->subsize = 1;
		sri->ystart = 0.0;
		sri->ydelta = 1.0;
		sri->yunits = 1;
		sri->mode = 0;
		sri->streamID = "";
		sri->keywords.length(0);
	};


	inline std::string time_to_string(const BULKIO::PrecisionUTCTime& timeTag, bool include_fractional = true, bool compressed = false){
		time_t _fileStartTime;
		struct tm *local;
		char timeArray[30];
		_fileStartTime = (time_t) timeTag.twsec;
		local = gmtime(&_fileStartTime); //converts second since epoch to tm struct
		if(compressed)
			strftime(timeArray, 30, "%d%b%Y.%H%M%S", local); //prints out string from tm struct
		else
			strftime(timeArray, 30, "%d-%b-%Y %H:%M:%S", local); //prints out string from tm struct
		std::string time = std::string(timeArray);
		if(include_fractional){
			char fractSec[30];
			sprintf(fractSec,"%010.0f",timeTag.tfsec*1e10);
			time+="."+std::string(fractSec);
		}
		return time;
	}


	inline void zeroTime(BULKIO::PrecisionUTCTime *timeTag) {
		timeTag->tcmode = 1;
		timeTag->tcstatus = 0;
		timeTag->toff = 0.0;
		timeTag->twsec = 0.0;
		timeTag->tfsec = 0.0;
	};


	inline BULKIO::PrecisionUTCTime getSystemTimestamp(double additional_time = 0.0) {
		double whole;
		double fract = modf(additional_time,&whole);
		struct timeval tmp_time;
		struct timezone tmp_tz;
		gettimeofday(&tmp_time, &tmp_tz);
		double wsec = tmp_time.tv_sec;
		double fsec = tmp_time.tv_usec / 1e6;
		BULKIO::PrecisionUTCTime tstamp = BULKIO::PrecisionUTCTime();
		tstamp.tcmode = 1;
		tstamp.tcstatus = (short) 1;
		tstamp.toff = 0.0;
		tstamp.twsec = wsec + whole;
		tstamp.tfsec = fsec + fract;
		while(tstamp.tfsec < 0){
			tstamp.twsec -= 1.0;
			tstamp.tfsec += 1.0;
		}
		return tstamp;
	};

	inline std::string uptime_string(const BULKIO::PrecisionUTCTime& start_time){
		BULKIO::PrecisionUTCTime current_time= getSystemTimestamp();
		char timeArray[30];
		double seconds;
		double minutes;
		double hours;
		double days;
		double diff_seconds = current_time.twsec - start_time.twsec;
		modf(diff_seconds/86400.0, &days);
		diff_seconds -= days*86400.0;
		modf(diff_seconds/3600.0, &hours);
		diff_seconds -= hours*3600.0;
		modf(diff_seconds/60.0, &minutes);
		diff_seconds -= minutes*60.0;
		seconds = diff_seconds;
		sprintf(timeArray,"%03.0f:%02.0f:%02.0f:%02.0f",days,hours,minutes,seconds);
		std::string time = std::string(timeArray);
		return time;

	}



	inline bool operator<(const BULKIO::PrecisionUTCTime& t1, const BULKIO::PrecisionUTCTime& t2){
		if(t1.twsec != t2.twsec)
			return t1.twsec < t2.twsec;
		if(t1.tfsec != t2.tfsec)
			return t1.tfsec < t2.tfsec;
		return false;
	}
	inline bool operator>(const BULKIO::PrecisionUTCTime& t1, const BULKIO::PrecisionUTCTime& t2){
		if(t1.twsec != t2.twsec)
			return t1.twsec > t2.twsec;
		if(t1.tfsec != t2.tfsec)
			return t1.tfsec > t2.tfsec;
		return false;
	}
	inline BULKIO::PrecisionUTCTime operator+(const BULKIO::PrecisionUTCTime& t1, const BULKIO::PrecisionUTCTime& t2){
		BULKIO::PrecisionUTCTime tstamp;
		tstamp.tcmode = 1;
		tstamp.tcstatus = (short) 1;
		tstamp.toff = 0.0;
		tstamp.twsec = t1.twsec + t2.twsec;
		tstamp.tfsec = t1.tfsec + t2.tfsec;
		double whole;
		tstamp.tfsec = modf(tstamp.tfsec,&whole);
		tstamp.twsec += whole;
		return tstamp;
	}

};


/*********************************************************************************************/
/**************************       Additional Thread Class           **************************/
/*********************************************************************************************/
/** Note:: This additional process thread is based off of the process thread class in the 	 */
/** 			USRP_UHD_base.h file.														 */
template < typename TargetClass >
class ProcessThread_2
{
    public:
	ProcessThread_2(TargetClass *_target, float _delay) :
            target(_target)
        {
            _mythread = 0;
            _thread_running = false;
            _udelay = (__useconds_t)(_delay * 1000000);
        };

        // kick off the thread
        void start() {
            if (_mythread == 0) {
                _thread_running = true;
                _mythread = new boost::thread(&ProcessThread_2::run, this);
            }
        };

        // manage calls to target's service function
        void run() {
            int state = NORMAL;
            while (_thread_running and (state != FINISH)) {
                state = target->serviceFunction_transmit();
                if (state == NOOP) usleep(_udelay);
            }
        };

        // stop thread and wait for termination
        bool release(unsigned long secs = 0, unsigned long usecs = 0) {
            _thread_running = false;
            if (_mythread)  {
                if ((secs == 0) and (usecs == 0)){
                    _mythread->join();
                } else {
                    boost::system_time waitime= boost::get_system_time() + boost::posix_time::seconds(secs) +  boost::posix_time::microseconds(usecs) ;
                    if (!_mythread->timed_join(waitime)) {
                        return 0;
                    }
                }
                delete _mythread;
                _mythread = 0;
            }

            return 1;
        };

        virtual ~ProcessThread_2(){
            if (_mythread != 0) {
                release(0);
                _mythread = 0;
            }
        };

        void updateDelay(float _delay) { _udelay = (__useconds_t)(_delay * 1000000); };

    private:
        boost::thread *_mythread;
        bool _thread_running;
        TargetClass *target;
        __useconds_t _udelay;
        boost::condition_variable _end_of_run;
        boost::mutex _eor_mutex;
};


////////////////////////////
//        #DEFINES        //
////////////////////////////
#define TUNER_BUFFER_SIZE_BYTES 2048000


////////////////////////////
//  STRUCTURE DEFINITION  //
////////////////////////////

// Time Type Definition
enum timeTypes {
	J1970 = 1,
	JCY = 2
};


/** Individual Tuner. This structure contains stream specific data for channel/tuner to include:
 * 		- Data buffers
 * 		- Additional stream metadata (sri & timestamps)
 * 		- Control information (allocation id's)
 * 		- Reference to associated frontend_tuner_status property where additional information is held. Note: frontend_tuner_status structure is required by frontend interfaces v2.0
 */

struct indivTuner {
	indivTuner(){
		frontend_status = NULL;
	}

	std::vector<short> outputBuffer;
	BULKIO::StreamSRI sri;
	BULKIO::PrecisionUTCTime outputBufferTime;
	BULKIO::PrecisionUTCTime timeUp;
	BULKIO::PrecisionUTCTime timeDown;
	boost::mutex *lock;
	std::string control_allocation_id;

    frontend_tuner_status_struct_struct* frontend_status;


   void reset(){
	    outputBuffer.resize(TUNER_BUFFER_SIZE_BYTES / sizeof (outputBuffer[0]) / 2);
	    BIO_HELPER::zeroSRI(&sri);
	    BIO_HELPER::zeroTime(&outputBufferTime);
	    BIO_HELPER::zeroTime(&timeUp);
	    BIO_HELPER::zeroTime(&timeDown);
	    control_allocation_id.clear();

	    if(frontend_status != NULL){
	    	frontend_status->allocation_id_csv.clear();
	    	frontend_status->center_frequency = 0.0;
	    	frontend_status->bandwidth = 0.0;
	    	frontend_status->sample_rate = 0.0;
	    	frontend_status->device_control = true;
	    	frontend_status->complex = true;
	    	frontend_status->enabled = true;
	    }

   };

};


////////////////////////////
//    CLASS DEFINITION    //
////////////////////////////

class USRP_UHD_i : public USRP_UHD_base
{
	ENABLE_LOGGING
	typedef std::vector<std::string> stringVector;
	typedef std::map<std::string, size_t> string_number_mapping;
	typedef std::map<std::string, std::string> string_string_mapping;
	typedef std::map<std::string, stringVector > string_stringVector_mapping;
	typedef boost::mutex::scoped_lock exclusive_lock;
	friend class FRONTEND_RFInfo_In_i_customized;
	friend class BULKIO_dataShort_In_i;
	friend class BULKIO_dataShort_In_i_customized;
	friend class FRONTEND_DigitalTuner_In_i_customized;
public:
	USRP_UHD_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
	USRP_UHD_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
	USRP_UHD_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
	USRP_UHD_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
	~USRP_UHD_i();

	// Overloaded base class functions for lifecycle control
	void initialize() throw (CF::LifeCycle::InitializeError, CORBA::SystemException);
	void start() throw (CF::Resource::StartError, CORBA::SystemException);
	void stop() throw (CF::Resource::StopError, CORBA::SystemException);
	void configure(const CF::Properties&) throw (CORBA::SystemException, CF::PropertySet::InvalidConfiguration, CF::PropertySet::PartialConfiguration);

	// Device specific allocation handling
	CF::Device::UsageType updateUsageState();
	CORBA::Boolean allocateCapacity(const CF::Properties & capacities) throw (CORBA::SystemException, CF::Device::InvalidCapacity, CF::Device::InvalidState);
	void deallocateCapacity(const CF::Properties & capacities)throw (CORBA::SystemException, CF::Device::InvalidCapacity, CF::Device::InvalidState);


	// Service functions. Essentially two threads where one (serviceFunction) handles receiving data and the other (serviceFunction_transmit) handles transmitting
	int serviceFunction();
	int serviceFunction_transmit();


	// Mapping and translation helpers. External string identifiers to internal numerical identifiers
	long addTunerMapping(const frontend_tuner_allocation_struct & frontend_alloc);
	long addTunerMapping(const frontend_listener_allocation_struct & frontend_listner_alloc);
	bool removeTunerMapping(std::string allocation_id);
	long getTunerMapping(std::string allocation_id);
	bool is_connectionID_valid_for_tunerNum( const size_t & tunerNum, const std::string & connectionID);
	bool is_connectionID_valid_for_streamID( const std::string & streamID, const std::string & connectionID);
	bool is_freq_valid(double req_cf, double req_bw, double req_sr, double cf, double bw, double sr);

	// Configure wideband wideband tuner - gets called during wideband wideband allocation
	bool setupTuner(size_t tuner_id, const frontend_tuner_allocation_struct& tuner_req) throw (std::logic_error);
	bool enableTuner(size_t tuner_id, bool enable);
	bool removeTuner(size_t tuner_id);

private:
	void _constructor_();
	void init_usrp(std::string ip_addr) throw (CF::PropertySet::InvalidConfiguration);
	void configureTunerSRI(BULKIO::StreamSRI *sri, double frequency, double sample_rate, short mode, std::string rf_flow_id = "");
	void updateSriTimes(BULKIO::StreamSRI *sri, double timeUp, double timeDown, timeTypes timeType);
	void updateAvaiableUsrpSeq();
	void update_myDeviceSeq();
	void update_rf_flow_id(std::string rfFlowId);
	void update_group_id(std::string groupID);
	std::string rf_flow_id;
	template <class IN_PORT_TYPE> bool singleService_transmit(IN_PORT_TYPE *dataIn);
	std::string create_allocation_id_csv(size_t tunerNum);

	// tunerChannels is exclusively paired with property tuner_status. tunerChannels provide stream information for the channel while tuner_status provides the tuner information.
	std::vector<indivTuner> tunerChannels;

	// Ensures configure() and serviceFunction() are thread safe
	boost::mutex propLock;

	// Provides mapping from unique allocation ID to internal tuner (channel) number
	string_number_mapping allocationId_to_tunerNum;
	string_number_mapping streamID_to_tunerNum;
	boost::mutex allocationID_MappingLock;

	ProcessThread_2<USRP_UHD_i> *serviceThread_transmit;
	boost::mutex serviceThreadTransmit_2;

	//UHD Specific
	uhd::usrp::multi_usrp::sptr usrpDevice;
	uhd::device_addr_t device_addr;

    BULKIO_dataShort_Out_i_customized *dataShort_out;
    FRONTEND_DigitalTuner_In_i_customized *DigitalTuner_in;
    FRONTEND_RFInfo_In_i_customized *RFInfo_in;



	////////////////////////////
	// Other helper functions //
	////////////////////////////

    inline frontend_tuner_allocation_struct feAlloc_from_feStatus(size_t tuner_id){
    		frontend_tuner_allocation_struct newStruct;
    		newStruct.tuner_type= tunerChannels[tuner_id].frontend_status->tuner_type;
    		newStruct.allocation_id= tunerChannels[tuner_id].control_allocation_id;
    		newStruct.center_frequency= tunerChannels[tuner_id].frontend_status->center_frequency;
    		newStruct.bandwidth= tunerChannels[tuner_id].frontend_status->bandwidth;
    		newStruct.sample_rate= tunerChannels[tuner_id].frontend_status->sample_rate;
    		newStruct.device_control= tunerChannels[tuner_id].frontend_status->device_control;
    		newStruct.group_id = tunerChannels[tuner_id].frontend_status->group_id;
    		newStruct.rf_flow_id= tunerChannels[tuner_id].frontend_status->rf_flow_id;
    		return newStruct;
    	}


    double optimize_rate(const double& req_rate, const double& max_rate, const double& min_rate){
        for(size_t dec = size_t(max_rate/min_rate); dec >= 1; dec--){
            if(max_rate/double(dec) >= req_rate)
                return max_rate/double(dec);
        }
        return req_rate;

    }

	template <typename CORBAXX> bool addModifyKeyword(BULKIO::StreamSRI *sri, CORBA::String_member id, CORBAXX myValue, bool addOnly = false) {
		CORBA::Any value;
		value <<= myValue;
		unsigned long keySize = sri->keywords.length();
		if (!addOnly) {
			for (unsigned int i = 0; i < keySize; i++) {
				if (!strcmp(sri->keywords[i].id, id)) {
					sri->keywords[i].value = value;
					return true;
				}
			}
		}
		sri->keywords.length(keySize + 1);
		if (sri->keywords.length() != keySize + 1)
			return false;
		sri->keywords[keySize].id = CORBA::string_dup(id);
		sri->keywords[keySize].value = value;
		return true;
	}

	// This is not currently used but is available as a debugging tool
	void printSRI(BULKIO::StreamSRI *sri, std::string strHeader = "DEBUG SRI"){
		std::cout << strHeader << ":\n";
		std::cout << "\thversion: " << sri->hversion<< std::endl;
		std::cout << "\txstart: " << sri->xstart<< std::endl;
		std::cout << "\txdelta: " << sri->xdelta<< std::endl;
		std::cout << "\txunits: " << sri->xunits<< std::endl;
		std::cout << "\tsubsize: " <<sri->subsize<< std::endl;
		std::cout << "\tystart: " << sri->ystart<< std::endl;
		std::cout << "\tydelta: " << sri->ydelta<< std::endl;
		std::cout << "\tyunits: " << sri->yunits<< std::endl;
		std::cout << "\tmode: " << sri->mode<< std::endl;
		std::cout << "\tstreamID: " << sri->streamID<< std::endl;
		unsigned long keySize = sri->keywords.length();
		for (unsigned int i = 0; i < keySize; i++) {
			std::cout << "\t KEYWORD KEY/VAL :: " << sri->keywords[i].id << ": " << ossie::any_to_string(sri->keywords[i].value) << std::endl;
		}
		std::cout << std::endl;
	}


};




#endif
