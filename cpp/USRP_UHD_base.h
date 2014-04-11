
#ifndef USRP_UHD_IMPL_BASE_H
#define USRP_UHD_IMPL_BASE_H

#include <boost/thread.hpp>
#include <frontend/frontend.h>

#include <bulkio/bulkio.h>
#include "port_impl.h"
#include "struct_props.h"

#define NOOP 0
#define FINISH -1
#define NORMAL 1

#define BOOL_VALUE_HERE 0
#define DOUBLE_VALUE_HERE 0

class USRP_UHD_base;

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

class USRP_UHD_base : public frontend::FrontendTunerDevice<frontend_tuner_status_struct_struct>, public virtual frontend::digital_tuner_delegation, public virtual frontend::rfinfo_delegation
{
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

        void connectionTableChanged(const std::vector<connection_descriptor_struct>* oldValue, const std::vector<connection_descriptor_struct>* newValue);
        void matchAllocationIdToStreamId(const std::string allocation_id, const std::string stream_id, const std::string port_name="");
        void removeAllocationIdRouting(const size_t tuner_id);
        void removeStreamIdRouting(const std::string stream_id, const std::string allocation_id="");

 
        virtual std::string get_rf_flow_id(const std::string& port_name);
        virtual void set_rf_flow_id(const std::string& port_name, const std::string& id);
        virtual frontend::RFInfoPkt get_rfinfo_pkt(const std::string& port_name);
        virtual void set_rfinfo_pkt(const std::string& port_name, const frontend::RFInfoPkt& pkt);
 
        virtual std::string getTunerType(const std::string& allocation_id);
        virtual bool getTunerDeviceControl(const std::string& allocation_id);
        virtual std::string getTunerGroupId(const std::string& allocation_id);
        virtual std::string getTunerRfFlowId(const std::string& allocation_id);
        virtual CF::Properties* getTunerStatus(const std::string& allocation_id);
        virtual void assignListener(const std::string& listen_alloc_id, const std::string& allocation_id);
        virtual void removeListener(const std::string& listen_alloc_id);
 
        virtual double getTunerCenterFrequency(const std::string& allocation_id);
        virtual void setTunerCenterFrequency(const std::string& allocation_id, double freq);
        virtual double getTunerBandwidth(const std::string& allocation_id);
        virtual void setTunerBandwidth(const std::string& allocation_id, double bw);
        virtual bool getTunerAgcEnable(const std::string& allocation_id);
        virtual void setTunerAgcEnable(const std::string& allocation_id, bool enable);
        virtual float getTunerGain(const std::string& allocation_id);
        virtual void setTunerGain(const std::string& allocation_id, float gain);
        virtual long getTunerReferenceSource(const std::string& allocation_id);
        virtual void setTunerReferenceSource(const std::string& allocation_id, long source);
        virtual bool getTunerEnable(const std::string& allocation_id);
        virtual void setTunerEnable(const std::string& allocation_id, bool enable);
 
        virtual double getTunerOutputSampleRate(const std::string& allocation_id);
        virtual void setTunerOutputSampleRate(const std::string& allocation_id,double sr);
        void frontendTunerStatusChanged(const std::vector<frontend_tuner_status_struct_struct>* oldValue, const std::vector<frontend_tuner_status_struct_struct>* newValue);
        
    protected:
        ProcessThread<USRP_UHD_base> *serviceThread; 
        boost::mutex serviceThreadLock;
        std::map<std::string, std::string> listeners;

        // Member variables exposed as properties
        bool update_available_devices;
        std::string device_ip_address;
        std::string device_reference_source_global;
        float device_rx_gain_global;
        float device_tx_gain_global;
        std::string device_group_id_global;
        std::vector<connection_descriptor_struct> connectionTable;
        std::vector<usrp_device_struct> available_devices;
        std::vector<usrp_channel_struct> device_channels;
        std::vector<usrp_motherboard_struct> device_motherboards;

        // Ports
        frontend::InRFInfoPort *RFInfo_in;
        frontend::InDigitalTunerPort *DigitalTuner_in;
        bulkio::InShortPort *dataShortTX_in;
        bulkio::InFloatPort *dataFloatTX_in;
        bulkio::OutShortPort *dataShort_out;
        frontend::OutRFInfoPort *RFInfoTX_out;

        virtual void setNumChannels(size_t num);
        void construct();

};
#endif // USRP_UHD_IMPL_BASE_H
