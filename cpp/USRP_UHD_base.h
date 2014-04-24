#ifndef USRP_UHD_IMPL_BASE_H
#define USRP_UHD_IMPL_BASE_H

#include <boost/thread.hpp>
#include <frontend/frontend.h>
#include <ossie/ThreadedComponent.h>

#include <frontend/frontend.h>
#include <bulkio/bulkio.h>
#include "struct_props.h"

#define BOOL_VALUE_HERE 0
#define DOUBLE_VALUE_HERE 0

class USRP_UHD_base : public frontend::FrontendTunerDevice<frontend_tuner_status_struct_struct>, public virtual frontend::digital_tuner_delegation, public virtual frontend::rfinfo_delegation, protected ThreadedComponent
{
    public:
        USRP_UHD_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        USRP_UHD_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        USRP_UHD_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        USRP_UHD_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~USRP_UHD_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();
        void matchAllocationIdToStreamId(const std::string allocation_id, const std::string stream_id, const std::string port_name="");
        void removeAllocationIdRouting(const size_t tuner_id);
        void removeStreamIdRouting(const std::string stream_id, const std::string allocation_id="");

        virtual CF::Properties* getTunerStatus(const std::string& allocation_id);
        virtual void assignListener(const std::string& listen_alloc_id, const std::string& allocation_id);
        virtual void removeListener(const std::string& listen_alloc_id);
        void frontendTunerStatusChanged(const std::vector<frontend_tuner_status_struct_struct>* oldValue, const std::vector<frontend_tuner_status_struct_struct>* newValue);

    protected:
        void connectionTableChanged(const std::vector<connection_descriptor_struct>* oldValue, const std::vector<connection_descriptor_struct>* newValue);

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

        std::map<std::string, std::string> listeners;

        virtual void setNumChannels(size_t num);

    private:
        void construct();
};
#endif // USRP_UHD_IMPL_BASE_H
