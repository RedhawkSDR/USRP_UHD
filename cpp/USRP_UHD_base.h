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

#ifndef USRP_UHD_IMPL_BASE_H
#define USRP_UHD_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Resource_impl.h>

#include "ossie/Device_impl.h"
#include "port_impl.h"
#include "struct_props.h"

#define NOOP 0
#define FINISH -1
#define NORMAL 1

class USRP_UHD_base;

#include <ossie/prop_helpers.h>

template < typename TargetClass >
class ProcessThread
{
    public:
        ProcessThread(TargetClass *_target, float _delay) :
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
                _mythread = new boost::thread(&ProcessThread::run, this);
            }
        };

        // manage calls to target's service function
        void run() {
            int state = NORMAL;
            while (_thread_running and (state != FINISH)) {
                state = target->serviceFunction();
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

        virtual ~ProcessThread(){
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

class USRP_UHD_base : public Device_impl
{
    friend class BULKIO_dataShort_In_i;
    friend class FRONTEND_DigitalTuner_In_i;
    friend class BULKIO_dataFloat_In_i;
    friend class FRONTEND_RFInfo_In_i;
    friend class BULKIO_dataShort_Out_i;

    public:
        USRP_UHD_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        USRP_UHD_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        USRP_UHD_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        USRP_UHD_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        CORBA::Object_ptr getPort(const char* _id) throw (CF::PortSupplier::UnknownPort, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void initialize() throw (CF::LifeCycle::InitializeError, CORBA::SystemException);

        void loadProperties();

        virtual int serviceFunction() = 0;

        bool compareSRI(BULKIO::StreamSRI &SRI_1, BULKIO::StreamSRI &SRI_2){
            if (SRI_1.hversion != SRI_2.hversion)
                return false;
            if (SRI_1.xstart != SRI_2.xstart)
                return false;
            if (SRI_1.xdelta != SRI_2.xdelta)
                return false;
            if (SRI_1.xunits != SRI_2.xunits)
                return false;
            if (SRI_1.subsize != SRI_2.subsize)
                return false;
            if (SRI_1.ystart != SRI_2.ystart)
                return false;
            if (SRI_1.ydelta != SRI_2.ydelta)
                return false;
            if (SRI_1.yunits != SRI_2.yunits)
                return false;
            if (SRI_1.mode != SRI_2.mode)
                return false;
            if (strcmp(SRI_1.streamID, SRI_2.streamID) != 0)
                return false;
            if (SRI_1.keywords.length() != SRI_2.keywords.length())
                return false;
            std::string action = "eq";
            for (unsigned int i=0; i<SRI_1.keywords.length(); i++) {
                if (strcmp(SRI_1.keywords[i].id, SRI_2.keywords[i].id)) {
                    return false;
                }
                if (!SRI_1.keywords[i].value.type()->equivalent(SRI_2.keywords[i].value.type())) {
                    return false;
                }
                if (!ossie::compare_anys(SRI_1.keywords[i].value, SRI_2.keywords[i].value, action)) {
                    return false;
                }
            }
            return true;
        }
        
    protected:
        ProcessThread<USRP_UHD_base> *serviceThread; 
        boost::mutex serviceThreadLock;  

        // Member variables exposed as properties
        std::string device_kind;
        std::string device_model;
        std::string USRP_ip_address;
        bool update_available_usrp_seq;
        std::string clock_ref;
        std::vector<usrp_device_struct_struct> available_usrp_seq;
        std::vector<usrp_channel_struct_struct> myDevice_channel_seq;
        std::vector<usrp_motherboard_struct_struct> myDevice_motherboard_seq;
        frontend_tuner_allocation_struct frontend_tuner_allocation;
        frontend_listener_allocation_struct frontend_listener_allocation;
        std::vector<frontend_tuner_status_struct_struct> frontend_tuner_status;
        float gain_global;
        std::string group_id_global;

        // Ports
        FRONTEND_DigitalTuner_In_i *DigitalTuner_in;
        FRONTEND_RFInfo_In_i *RFInfo_in;
        BULKIO_dataShort_In_i *dataShortComplex_in;
        BULKIO_dataFloat_In_i *dataFloatComplex_in;
        BULKIO_dataShort_Out_i *dataShort_out;
    
    private:
        void construct();

};
#endif
