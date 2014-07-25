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
#include "USRP_UHD_base.h"

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

    The following class functions are for the base class for the device class. To
    customize any of these functions, do not modify them here. Instead, overload them
    on the child class

******************************************************************************************/

USRP_UHD_base::USRP_UHD_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl) :
    frontend::FrontendTunerDevice<frontend_tuner_status_struct_struct>(devMgr_ior, id, lbl, sftwrPrfl),
    ThreadedComponent()
{
    construct();
}

USRP_UHD_base::USRP_UHD_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev) :
    frontend::FrontendTunerDevice<frontend_tuner_status_struct_struct>(devMgr_ior, id, lbl, sftwrPrfl, compDev),
    ThreadedComponent()
{
    construct();
}

USRP_UHD_base::USRP_UHD_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities) :
    frontend::FrontendTunerDevice<frontend_tuner_status_struct_struct>(devMgr_ior, id, lbl, sftwrPrfl, capacities),
    ThreadedComponent()
{
    construct();
}

USRP_UHD_base::USRP_UHD_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev) :
    frontend::FrontendTunerDevice<frontend_tuner_status_struct_struct>(devMgr_ior, id, lbl, sftwrPrfl, capacities, compDev),
    ThreadedComponent()
{
    construct();
}

USRP_UHD_base::~USRP_UHD_base()
{
    delete RFInfo_in;
    RFInfo_in = 0;
    delete DigitalTuner_in;
    DigitalTuner_in = 0;
    delete dataShortTX_in;
    dataShortTX_in = 0;
    delete dataFloatTX_in;
    dataFloatTX_in = 0;
    delete dataShort_out;
    dataShort_out = 0;
    delete RFInfoTX_out;
    RFInfoTX_out = 0;
}

void USRP_UHD_base::construct()
{
    loadProperties();

    RFInfo_in = new frontend::InRFInfoPort("RFInfo_in", this);
    addPort("RFInfo_in", RFInfo_in);
    DigitalTuner_in = new frontend::InDigitalTunerPort("DigitalTuner_in", this);
    addPort("DigitalTuner_in", DigitalTuner_in);
    dataShortTX_in = new bulkio::InShortPort("dataShortTX_in");
    addPort("dataShortTX_in", dataShortTX_in);
    dataFloatTX_in = new bulkio::InFloatPort("dataFloatTX_in");
    addPort("dataFloatTX_in", dataFloatTX_in);
    dataShort_out = new bulkio::OutShortPort("dataShort_out");
    addPort("dataShort_out", dataShort_out);
    RFInfoTX_out = new frontend::OutRFInfoPort("RFInfoTX_out");
    addPort("RFInfoTX_out", RFInfoTX_out);

    this->addPropertyChangeListener("connectionTable", this, &USRP_UHD_base::connectionTableChanged);

}

/*******************************************************************************************
    Framework-level functions
    These functions are generally called by the framework to perform housekeeping.
*******************************************************************************************/
void USRP_UHD_base::start() throw (CORBA::SystemException, CF::Resource::StartError)
{
    frontend::FrontendTunerDevice<frontend_tuner_status_struct_struct>::start();
    ThreadedComponent::startThread();
}

void USRP_UHD_base::stop() throw (CORBA::SystemException, CF::Resource::StopError)
{
    frontend::FrontendTunerDevice<frontend_tuner_status_struct_struct>::stop();
    if (!ThreadedComponent::stopThread()) {
        throw CF::Resource::StopError(CF::CF_NOTSET, "Processing thread did not die");
    }
}

void USRP_UHD_base::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
    // This function clears the device running condition so main shuts down everything
    try {
        stop();
    } catch (CF::Resource::StopError& ex) {
        // TODO - this should probably be logged instead of ignored
    }

    frontend::FrontendTunerDevice<frontend_tuner_status_struct_struct>::releaseObject();
}

void USRP_UHD_base::connectionTableChanged(const std::vector<connection_descriptor_struct>* oldValue, const std::vector<connection_descriptor_struct>* newValue)
{
    dataShort_out->updateConnectionFilter(*newValue);
}

void USRP_UHD_base::loadProperties()
{
    device_kind = "FRONTEND::TUNER";
    device_model = "USRP";
    addProperty(update_available_devices,
                false,
                "update_available_devices",
                "update_available_devices",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(device_reference_source_global,
                "INTERNAL",
                "device_reference_source_global",
                "device_reference_source_global",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(device_rx_gain_global,
                0,
                "device_rx_gain_global",
                "device_rx_gain_global",
                "readwrite",
                "dB",
                "external",
                "configure");

    addProperty(device_tx_gain_global,
                0,
                "device_tx_gain_global",
                "device_tx_gain_global",
                "readwrite",
                "dB",
                "external",
                "configure");

    addProperty(device_group_id_global,
                "device_group_id_global",
                "device_group_id_global",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(device_rx_mode,
                "16bit",
                "device_rx_mode",
                "device_rx_mode",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(device_tx_mode,
                "16bit",
                "device_tx_mode",
                "device_tx_mode",
                "readwrite",
                "",
                "external",
                "configure");

    frontend_listener_allocation = frontend::frontend_listener_allocation_struct();
    frontend_tuner_allocation = frontend::frontend_tuner_allocation_struct();
    addProperty(target_device,
                target_device_struct(),
                "target_device",
                "target_device",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(connectionTable,
                "connectionTable",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(available_devices,
                "available_devices",
                "available_devices",
                "readonly",
                "",
                "external",
                "configure");

    addProperty(device_channels,
                "device_channels",
                "device_channels",
                "readonly",
                "",
                "external",
                "configure");

    addProperty(device_motherboards,
                "device_motherboards",
                "device_motherboards",
                "readonly",
                "",
                "external",
                "configure");

}

/* This sets the number of entries in the frontend_tuner_status struct sequence property
 *  * as well as the tuner_allocation_ids vector. Call this function during initialization
 *   */
void USRP_UHD_base::setNumChannels(size_t num)
{
    frontend_tuner_status.clear();
    frontend_tuner_status.resize(num);
    tuner_allocation_ids.clear();
    tuner_allocation_ids.resize(num);
}

void USRP_UHD_base::frontendTunerStatusChanged(const std::vector<frontend_tuner_status_struct_struct>* oldValue, const std::vector<frontend_tuner_status_struct_struct>* newValue)
{
    this->tuner_allocation_ids.resize(this->frontend_tuner_status.size());
}

CF::Properties* USRP_UHD_base::getTunerStatus(const std::string& allocation_id)
{
    CF::Properties* tmpVal = new CF::Properties();
    long tuner_id = getTunerMapping(allocation_id);
    if (tuner_id < 0)
        throw FRONTEND::FrontendException(("ERROR: ID: " + std::string(allocation_id) + " IS NOT ASSOCIATED WITH ANY TUNER!").c_str());
    CORBA::Any prop;
    prop <<= *(static_cast<frontend_tuner_status_struct_struct*>(&this->frontend_tuner_status[tuner_id]));
    prop >>= tmpVal;

    CF::Properties_var tmp = new CF::Properties(*tmpVal);
    return tmp._retn();
}

void USRP_UHD_base::assignListener(const std::string& listen_alloc_id, const std::string& allocation_id)
{
    // find control allocation_id
    std::string existing_alloc_id = allocation_id;
    std::map<std::string,std::string>::iterator existing_listener;
    while ((existing_listener=listeners.find(existing_alloc_id)) != listeners.end())
        existing_alloc_id = existing_listener->second;
    listeners[listen_alloc_id] = existing_alloc_id;

    std::vector<connection_descriptor_struct> old_table = connectionTable;
    std::vector<connection_descriptor_struct> new_entries;
    for (std::vector<connection_descriptor_struct>::iterator entry=connectionTable.begin();entry!=connectionTable.end();entry++) {
        if (entry->connection_id == existing_alloc_id) {
            connection_descriptor_struct tmp;
            tmp.connection_id = listen_alloc_id;
            tmp.stream_id = entry->stream_id;
            tmp.port_name = entry->port_name;
            new_entries.push_back(tmp);
        }
    }
    bool foundEntry = false;
    for (std::vector<connection_descriptor_struct>::iterator new_entry=new_entries.begin();new_entry!=new_entries.end();new_entry++) {
        for (std::vector<connection_descriptor_struct>::iterator entry=connectionTable.begin();entry!=connectionTable.end();entry++) {
            if (entry == new_entry) {
                foundEntry = true;
                break;
            }
        }
        if (!foundEntry) {
            connectionTable.push_back(*new_entry);
        }
    }
    connectionTableChanged(&old_table, &connectionTable);
}

void USRP_UHD_base::removeListener(const std::string& listen_alloc_id)
{
    if (listeners.find(listen_alloc_id) != listeners.end()) {
        listeners.erase(listen_alloc_id);
    }
    std::vector<connection_descriptor_struct> old_table = this->connectionTable;
    std::vector<connection_descriptor_struct>::iterator entry = this->connectionTable.begin();
    while (entry != this->connectionTable.end()) {
        if (entry->connection_id == listen_alloc_id) {
            entry = this->connectionTable.erase(entry);
        } else {
            entry++;
        }
    }
    ExtendedCF::UsesConnectionSequence_var tmp;
    // Check to see if port "dataShort_out" has a connection for this listener
    tmp = this->dataShort_out->connections();
    for (unsigned int i=0; i<this->dataShort_out->connections()->length(); i++) {
        std::string connection_id = ossie::corba::returnString(tmp[i].connectionId);
        if (connection_id == listen_alloc_id) {
            this->dataShort_out->disconnectPort(connection_id.c_str());
        }
    }
    this->connectionTableChanged(&old_table, &this->connectionTable);
}

void USRP_UHD_base::removeAllocationIdRouting(const size_t tuner_id) {
    std::string allocation_id = getControlAllocationId(tuner_id);
    std::vector<connection_descriptor_struct> old_table = this->connectionTable;
    std::vector<connection_descriptor_struct>::iterator itr = this->connectionTable.begin();
    while (itr != this->connectionTable.end()) {
        if (itr->connection_id == allocation_id) {
            itr = this->connectionTable.erase(itr);
            continue;
        }
        itr++;
    }
    for (std::map<std::string, std::string>::iterator listener=listeners.begin();listener!=listeners.end();listener++) {
        if (listener->second == allocation_id) {
            std::vector<connection_descriptor_struct>::iterator itr = this->connectionTable.begin();
            while (itr != this->connectionTable.end()) {
                if (itr->connection_id == listener->first) {
                    itr = this->connectionTable.erase(itr);
                    continue;
                }
                itr++;
            }
        }
    }
    this->connectionTableChanged(&old_table, &this->connectionTable);
}

void USRP_UHD_base::removeStreamIdRouting(const std::string stream_id, const std::string allocation_id) {
    std::vector<connection_descriptor_struct> old_table = this->connectionTable;
    std::vector<connection_descriptor_struct>::iterator itr = this->connectionTable.begin();
    while (itr != this->connectionTable.end()) {
        if (allocation_id == "") {
            if (itr->stream_id == stream_id) {
                itr = this->connectionTable.erase(itr);
                continue;
            }
        } else {
            if ((itr->stream_id == stream_id) and (itr->connection_id == allocation_id)) {
                itr = this->connectionTable.erase(itr);
                continue;
            }
        }
        itr++;
    }
    for (std::map<std::string, std::string>::iterator listener=listeners.begin();listener!=listeners.end();listener++) {
        if (listener->second == allocation_id) {
            std::vector<connection_descriptor_struct>::iterator itr = this->connectionTable.begin();
            while (itr != this->connectionTable.end()) {
                if ((itr->connection_id == listener->first) and (itr->stream_id == stream_id)) {
                    itr = this->connectionTable.erase(itr);
                    continue;
                }
                itr++;
            }
        }
    }
    this->connectionTableChanged(&old_table, &this->connectionTable);
}

void USRP_UHD_base::matchAllocationIdToStreamId(const std::string allocation_id, const std::string stream_id, const std::string port_name) {
    if (port_name != "") {
        for (std::vector<connection_descriptor_struct>::iterator prop_itr = this->connectionTable.begin(); prop_itr!=this->connectionTable.end(); prop_itr++) {
            if ((*prop_itr).port_name != port_name)
                continue;
            if ((*prop_itr).stream_id != stream_id)
                continue;
            if ((*prop_itr).connection_id != allocation_id)
                continue;
            // all three match. This is a repeat
            return;
        }
        std::vector<connection_descriptor_struct> old_table = this->connectionTable;
        connection_descriptor_struct tmp;
        tmp.connection_id = allocation_id;
        tmp.port_name = port_name;
        tmp.stream_id = stream_id;
        this->connectionTable.push_back(tmp);
        this->connectionTableChanged(&old_table, &this->connectionTable);
        return;
    }
    std::vector<connection_descriptor_struct> old_table = this->connectionTable;
    connection_descriptor_struct tmp;
    tmp.connection_id = allocation_id;
    tmp.port_name = "dataShort_out";
    tmp.stream_id = stream_id;
    this->connectionTable.push_back(tmp);
    this->connectionTableChanged(&old_table, &this->connectionTable);
}

