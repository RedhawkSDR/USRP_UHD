#ifndef USRP_UHD_IMPL_H
#define USRP_UHD_IMPL_H

#include "USRP_UHD_base.h"

#include <uhd/usrp/multi_usrp.hpp>


/*********************************************************************************************/
/**************************        Multi Process Thread Class       **************************/
/*********************************************************************************************/
/** Note:: This class is based off of the process thread class in the USRP_base.h file.      */
/**             Changed to accept serviceFunction as argument, rather than hard coded        */
/**             Added interrupt() member function to interrupt underlying boost::thread      */
/*********************************************************************************************/
template < typename TargetClass >
class MultiProcessThread
{
public:
    MultiProcessThread(TargetClass *_target, int (TargetClass::*_func)(),float _delay)
    {
        service_function = boost::bind(_func, _target);
        _mythread = 0;
        _thread_running = false;
        _udelay = (__useconds_t)(_delay * 1000000);
    };

    void interrupt(){
        _mythread->interrupt();
    }

    // kick off the thread
    void start() {
        if (_mythread == 0) {
            _thread_running = true;
            _mythread = new boost::thread(&MultiProcessThread::run, this);
        }
    };

    // manage calls to target's service function
    void run() {
        int state = NORMAL;
        while (_thread_running and (state != FINISH)) {
            // yield to other threads if necessary
            try{
                boost::this_thread::interruption_point();
            } catch(const boost::thread_interrupted& ){
                usleep(_udelay);
            }
            state = service_function();
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

    virtual ~MultiProcessThread(){
        if (_mythread != 0) {
            release(0);
            _mythread = 0;
        }
    };

    void updateDelay(float _delay) { _udelay = (__useconds_t)(_delay * 1000000); };

private:
    boost::thread *_mythread;
    bool _thread_running;
    boost::function<int ()> service_function;
    __useconds_t _udelay;
    boost::condition_variable _end_of_run;
    boost::mutex _eor_mutex;
};


/** Device Individual Tuner. This structure contains stream specific data for channel/tuner to include:
 *      - Data buffer
 *      - Additional stream metadata (timestamps)
 */
struct usrpTunerStruct {
    usrpTunerStruct(){
        lock = NULL;

        // size buffer within CORBA transfer limits
        // Multiply by some number < 1 to leave some margin for the CORBA header
        // fyi: the bulkio pushPacket call does this same calculation as of 1.10,
        //      so we'll only require a single pushPacket call per buffer
        // Also, since data is complex, ensure number of samples is even
        const size_t max_payload_size    = (size_t) (bulkio::Const::MaxTransferBytes() * .9);
        const size_t max_samples_per_push = size_t((max_payload_size/sizeof(output_buffer[0]))/2)*2;

        buffer_capacity = max_samples_per_push;
        output_buffer.resize( buffer_capacity );

        reset();
    }

    std::vector<short> output_buffer;
    size_t buffer_capacity; // num samps buffer can hold
    size_t buffer_size; // num samps in buffer
    BULKIO::PrecisionUTCTime output_buffer_time;
    BULKIO::PrecisionUTCTime time_up;
    BULKIO::PrecisionUTCTime time_down;
    bool update_sri;
    boost::mutex *lock;

    void reset(){
        buffer_size = 0;
        bulkio::sri::zeroTime(output_buffer_time);
        bulkio::sri::zeroTime(time_up);
        bulkio::sri::zeroTime(time_down);
        update_sri = false;
    };
};
struct usrpRangesStruct {
    usrpRangesStruct(){
        reset();
    }

    uhd::meta_range_t frequency;
    uhd::meta_range_t bandwidth;
    uhd::meta_range_t sample_rate;
    uhd::meta_range_t gain;

    void reset(){
        frequency.clear();
        bandwidth.clear();
        sample_rate.clear();
        gain.clear();
    };
};

class USRP_UHD_i : public USRP_UHD_base
{
    ENABLE_LOGGING
    public:
        USRP_UHD_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        USRP_UHD_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        USRP_UHD_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        USRP_UHD_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~USRP_UHD_i();
        int serviceFunction(){return FINISH;} // unused
        int serviceFunctionReceive();
        int serviceFunctionTransmit();
        void start() throw (CF::Resource::StartError, CORBA::SystemException);
        void stop() throw (CF::Resource::StopError, CORBA::SystemException);
    protected:
        std::string get_rf_flow_id(const std::string& port_name);
        void set_rf_flow_id(const std::string& port_name, const std::string& id);
        frontend::RFInfoPkt get_rfinfo_pkt(const std::string& port_name);
        void set_rfinfo_pkt(const std::string& port_name, const frontend::RFInfoPkt& pkt);
        std::string getTunerType(const std::string& allocation_id);
        bool getTunerDeviceControl(const std::string& allocation_id);
        std::string getTunerGroupId(const std::string& allocation_id);
        std::string getTunerRfFlowId(const std::string& allocation_id);
        double getTunerCenterFrequency(const std::string& allocation_id);
        void setTunerCenterFrequency(const std::string& allocation_id, double freq);
        double getTunerBandwidth(const std::string& allocation_id);
        void setTunerBandwidth(const std::string& allocation_id, double bw);
        bool getTunerAgcEnable(const std::string& allocation_id);
        void setTunerAgcEnable(const std::string& allocation_id, bool enable);
        float getTunerGain(const std::string& allocation_id);
        void setTunerGain(const std::string& allocation_id, float gain);
        long getTunerReferenceSource(const std::string& allocation_id);
        void setTunerReferenceSource(const std::string& allocation_id, long source);
        bool getTunerEnable(const std::string& allocation_id);
        void setTunerEnable(const std::string& allocation_id, bool enable);
        double getTunerOutputSampleRate(const std::string& allocation_id);
        void setTunerOutputSampleRate(const std::string& allocation_id, double sr);

    private:
        ////////////////////////////////////////
        // Required device specific functions // -- to be implemented by device developer
        ////////////////////////////////////////

        // these are pure virtual, must be implemented here
        void deviceEnable(frontend_tuner_status_struct_struct &fts, size_t tuner_id);
        void deviceDisable(frontend_tuner_status_struct_struct &fts, size_t tuner_id);
        bool deviceSetTuning(const frontend::frontend_tuner_allocation_struct &request, frontend_tuner_status_struct_struct &fts, size_t tuner_id);
        bool deviceDeleteTuning(frontend_tuner_status_struct_struct &fts, size_t tuner_id);

        /////////////////////////
        // Developer additions //
        /////////////////////////

        // develop added helper functions for the 4 functions above
        void deviceEnable(size_t tuner_id){deviceEnable(frontend_tuner_status[tuner_id],tuner_id);}
        void deviceDisable(size_t tuner_id){deviceDisable(frontend_tuner_status[tuner_id],tuner_id);}
        bool deviceSetTuning(const frontend::frontend_tuner_allocation_struct &request, size_t tuner_id){return deviceSetTuning(request,frontend_tuner_status[tuner_id],tuner_id);}
        bool deviceDeleteTuning(size_t tuner_id){return deviceDeleteTuning(frontend_tuner_status[tuner_id],tuner_id);}

        // serviceFunctionTransmit thread
        void interrupt(size_t tuner_id);
        MultiProcessThread<USRP_UHD_i> *receive_service_thread;
        MultiProcessThread<USRP_UHD_i> *transmit_service_thread;
        boost::mutex receive_service_thread_lock;
        boost::mutex transmit_service_thread_lock;
        template <class IN_PORT_TYPE> bool transmitHelper(IN_PORT_TYPE *dataIn);

        // Ensures access to properties is thread safe
        // excludes access to frontend_tuner_status, which is covered tuner-by-tuner via usrp_tuners[tuner_id].lock
        boost::mutex prop_lock;

        // global properties for all channels
        frontend::RFInfoPkt rx_rfinfo_pkt;
        frontend::RFInfoPkt tx_rfinfo_pkt;
        void updateRxRfFlowId(std::string rf_flow_id);
        void updateTxRfFlowId(std::string rf_flow_id);
        void updateGroupId(std::string group);

        // override base class functions
        void setNumChannels(size_t num_rx, size_t num_tx);

        // configure callbacks
        void updateAvailableDevicesChanged(const bool* old_value, const bool* new_value);
        void deviceIpAddressChanged(const std::string* old_value, const std::string* new_value) throw (CF::PropertySet::InvalidConfiguration);
        void deviceRxGainChanged(const float* old_value, const float* new_value);
        void deviceTxGainChanged(const float* old_value, const float* new_value);
        void deviceReferenceSourceChanged(const std::string* old_value, const std::string* new_value);
        void deviceGroupIdChanged(const std::string* old_value, const std::string* new_value);

        // additional bookkeeping for each channel
        std::vector<usrpRangesStruct> usrp_ranges; // freq/bw/sr/gain ranges supported by each tuner channel
                                                   // indices map to tuner_id
                                                   // protected by prop_lock
        std::vector<usrpTunerStruct> usrp_tuners; // data buffer/timestamps, lock
                                                  // indices map to tuner_id
                                                  // each element protected by corresponding usrp_tuners[tuner_id].lock
        std::vector<uhd::rx_streamer::sptr> usrp_rx_streamers; // indices map to usrp_tuners[tuner_id].tuner_number
                                                               // each element protected by corresponding usrp_tuners[tuner_id].lock
        std::vector<uhd::tx_streamer::sptr> usrp_tx_streamers; // indices map to usrp_tuners[tuner_id].tuner_number
                                                               // each element protected by corresponding usrp_tuners[tuner_id].lock
        std::vector<size_t> usrp_tx_streamer_typesize; // indices map to usrp_tuners[tuner_id].tuner_number
                                                       // each element protected by corresponding usrp_tuners[tuner_id].lock

        // usrp helper functions/etc.
        std::string getStreamId(size_t tuner_id);
        double optimizeRate(const double& req_rate, const size_t tuner_id);
        double optimizeBandwidth(const double& req_bw, const size_t tuner_id);
        void updateSriTimes(BULKIO::StreamSRI *sri, double timeUp, double timeDown, frontend::timeTypes timeType);

        // interface with usrp device
        void updateAvailableDevices();
        void initUsrp() throw (CF::PropertySet::InvalidConfiguration);
        void updateDeviceInfo();
        void updateDeviceRxGain(double gain);
        void updateDeviceTxGain(double gain);
        void updateDeviceReferenceSource(std::string source);
        long usrpReceive(size_t tuner_id, double timeout = 0.0);
        template <class PACKET_TYPE> bool usrpTransmit(size_t tuner_id, PACKET_TYPE *packet);
        bool usrpEnable(size_t tuner_id);
        bool usrpDisable(size_t tuner_id);
        bool usrpCreateRxStream(size_t tuner_id);
        template <class PACKET_ELEMENT_TYPE> bool usrpCreateTxStream(size_t tuner_id);

        // UHD driver specific
        uhd::usrp::multi_usrp::sptr usrp_device_ptr;
        uhd::device_addr_t usrp_device_addr;

    protected:
        void construct();
};

#endif // USRP_UHD_IMPL_H
