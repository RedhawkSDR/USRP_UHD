/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK rh.USRP_UHD.
 *
 * REDHAWK rh.USRP_UHD is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK rh.USRP_UHD is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#ifndef USRP_UHD_BASE_IMPL_BASE_H
#define USRP_UHD_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <frontend/frontend.h>
#include <ossie/ThreadedComponent.h>

#include <frontend/frontend.h>
#include <bulkio/bulkio.h>
#include "struct_props.h"

#define BOOL_VALUE_HERE 0

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
        /// Property: device_group_id_global
        std::string device_group_id_global;
        /// Property: device_reference_source_global
        std::string device_reference_source_global;
        /// Property: device_rx_gain_global
        float device_rx_gain_global;
        /// Property: device_rx_mode
        std::string device_rx_mode;
        /// Property: device_tx_gain_global
        float device_tx_gain_global;
        /// Property: device_tx_mode
        std::string device_tx_mode;
        /// Property: update_available_devices
        bool update_available_devices;
        /// Property: sdds_settings
        sdds_settings_struct sdds_settings;
        /// Property: target_device
        target_device_struct target_device;
        /// Property: sdds_network_settings
        std::vector<sdds_network_settings_struct_struct> sdds_network_settings;
        /// Property: available_devices
        std::vector<usrp_device_struct> available_devices;
        /// Property: device_motherboards
        std::vector<usrp_motherboard_struct> device_motherboards;
        /// Property: device_channels
        std::vector<usrp_channel_struct> device_channels;
        /// Property: connectionTable
        std::vector<connection_descriptor_struct> connectionTable;

        // Ports
        /// Port: RFInfo_in
        frontend::InRFInfoPort *RFInfo_in;
        /// Port: DigitalTuner_in
        frontend::InDigitalTunerPort *DigitalTuner_in;
        /// Port: dataShortTX_in
        bulkio::InShortPort *dataShortTX_in;
        /// Port: dataFloatTX_in
        bulkio::InFloatPort *dataFloatTX_in;
        /// Port: dataShort_out
        bulkio::OutShortPort *dataShort_out;
        /// Port: RFInfoTX_out
        frontend::OutRFInfoPort *RFInfoTX_out;
        /// Port: dataSDDS_out
        bulkio::OutSDDSPort *dataSDDS_out;

        std::map<std::string, std::string> listeners;

        virtual void setNumChannels(size_t num);
        virtual void setNumChannels(size_t num, std::string tuner_type);

    private:
        void construct();
};
#endif // USRP_UHD_BASE_IMPL_BASE_H
