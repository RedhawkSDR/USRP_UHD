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


#include "USRP_UHD_base.h"

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY
    
 	Source: USRP_UHD.spd.xml
 	Generated on: Thu Aug 02 12:02:47 EDT 2012
 	Redhawk IDE
 	Version:M.1.8.1
 	Build id: v201207181831-r8893

*******************************************************************************************/

/******************************************************************************************

    The following class functions are for the base class for the component class. To
    customize any of these functions, do not modify them here. Instead, overload them
    on the child class

******************************************************************************************/

USRP_UHD_base::USRP_UHD_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl) :
          Device_impl (devMgr_ior, id, lbl, sftwrPrfl) , serviceThread(0){
    construct();
}

USRP_UHD_base::USRP_UHD_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev) :
          Device_impl (devMgr_ior, id, lbl, sftwrPrfl, compDev) , serviceThread(0){
    construct();
}

USRP_UHD_base::USRP_UHD_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities) :
          Device_impl (devMgr_ior, id, lbl, sftwrPrfl) , serviceThread(0){
    construct();
}

USRP_UHD_base::USRP_UHD_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev) :
          Device_impl (devMgr_ior, id, lbl, sftwrPrfl, compDev) , serviceThread(0){
    construct();
}

void USRP_UHD_base::construct()
{
    Resource_impl::_started = false;
    loadProperties();
    serviceThread = 0;
    
    PortableServer::ObjectId_var oid;
    DigitalTuner_in = new FRONTEND_DigitalTuner_In_i("DigitalTuner_in", this);
    oid = ossie::corba::RootPOA()->activate_object(DigitalTuner_in);
    RFInfo_in = new FRONTEND_RFInfo_In_i("RFInfo_in", this);
    oid = ossie::corba::RootPOA()->activate_object(RFInfo_in);
    dataShortComplex_in = new BULKIO_dataShort_In_i("dataShortComplex_in", this);
    oid = ossie::corba::RootPOA()->activate_object(dataShortComplex_in);
    dataFloatComplex_in = new BULKIO_dataFloat_In_i("dataFloatComplex_in", this);
    oid = ossie::corba::RootPOA()->activate_object(dataFloatComplex_in);
    dataShort_out = new BULKIO_dataShort_Out_i("dataShort_out", this);
    oid = ossie::corba::RootPOA()->activate_object(dataShort_out);

    registerInPort(DigitalTuner_in);
    registerInPort(RFInfo_in);
    registerInPort(dataShortComplex_in);
    registerInPort(dataFloatComplex_in);
    registerOutPort(dataShort_out, dataShort_out->_this());
}

/*******************************************************************************************
    Framework-level functions
    These functions are generally called by the framework to perform housekeeping.
*******************************************************************************************/
void USRP_UHD_base::initialize() throw (CF::LifeCycle::InitializeError, CORBA::SystemException)
{
}

void USRP_UHD_base::start() throw (CORBA::SystemException, CF::Resource::StartError)
{
    boost::mutex::scoped_lock lock(serviceThreadLock);
    if (serviceThread == 0) {
        dataShortComplex_in->unblock();
        dataFloatComplex_in->unblock();
        serviceThread = new ProcessThread<USRP_UHD_base>(this, 0.1);
        serviceThread->start();
    }
    
    if (!Resource_impl::started()) {
    	Resource_impl::start();
    }
}

void USRP_UHD_base::stop() throw (CORBA::SystemException, CF::Resource::StopError)
{
    boost::mutex::scoped_lock lock(serviceThreadLock);
    // release the child thread (if it exists)
    if (serviceThread != 0) {
        dataShortComplex_in->block();
        dataFloatComplex_in->block();
        if (!serviceThread->release(2)) {
            throw CF::Resource::StopError(CF::CF_NOTSET, "Processing thread did not die");
        }
        serviceThread = 0;
    }
    
    if (Resource_impl::started()) {
    	Resource_impl::stop();
    }
}

CORBA::Object_ptr USRP_UHD_base::getPort(const char* _id) throw (CORBA::SystemException, CF::PortSupplier::UnknownPort)
{

    std::map<std::string, Port_Provides_base_impl *>::iterator p_in = inPorts.find(std::string(_id));
    if (p_in != inPorts.end()) {

        if (!strcmp(_id,"DigitalTuner_in")) {
            FRONTEND_DigitalTuner_In_i *ptr = dynamic_cast<FRONTEND_DigitalTuner_In_i *>(p_in->second);
            if (ptr) {
                return FRONTEND::DigitalTuner::_duplicate(ptr->_this());
            }
        }
        if (!strcmp(_id,"RFInfo_in")) {
            FRONTEND_RFInfo_In_i *ptr = dynamic_cast<FRONTEND_RFInfo_In_i *>(p_in->second);
            if (ptr) {
                return FRONTEND::RFInfo::_duplicate(ptr->_this());
            }
        }
        if (!strcmp(_id,"dataShortComplex_in")) {
            BULKIO_dataShort_In_i *ptr = dynamic_cast<BULKIO_dataShort_In_i *>(p_in->second);
            if (ptr) {
                return BULKIO::dataShort::_duplicate(ptr->_this());
            }
        }
        if (!strcmp(_id,"dataFloatComplex_in")) {
            BULKIO_dataFloat_In_i *ptr = dynamic_cast<BULKIO_dataFloat_In_i *>(p_in->second);
            if (ptr) {
                return BULKIO::dataFloat::_duplicate(ptr->_this());
            }
        }
    }

    std::map<std::string, CF::Port_var>::iterator p_out = outPorts_var.find(std::string(_id));
    if (p_out != outPorts_var.end()) {
        return CF::Port::_duplicate(p_out->second);
    }

    throw (CF::PortSupplier::UnknownPort());
}

void USRP_UHD_base::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
    // This function clears the component running condition so main shuts down everything
    try {
        stop();
    } catch (CF::Resource::StopError& ex) {
        // TODO - this should probably be logged instead of ignored
    }

    // deactivate ports
    releaseInPorts();
    releaseOutPorts();

    delete(DigitalTuner_in);
    delete(RFInfo_in);
    delete(dataShortComplex_in);
    delete(dataFloatComplex_in);
    delete(dataShort_out);

    Device_impl::releaseObject();
}

void USRP_UHD_base::loadProperties()
{
    addProperty(device_kind,
                "FRONTEND", 
               "DCE:cdc5ee18-7ceb-4ae6-bf4c-31f983179b4d",
               "device_kind",
               "readonly",
               "",
               "eq",
               "configure,allocation");

    addProperty(device_model,
                "USRP", 
               "DCE:0f99b2e4-9903-4631-9846-ff349d18ecfb",
               "device_model",
               "readonly",
               "",
               "eq",
               "configure,allocation");

    addProperty(USRP_ip_address,
               "USRP_ip_address",
               "USRP_ip_address",
               "readwrite",
               "",
               "external",
               "configure");

    addProperty(update_available_usrp_seq,
                false, 
               "update_available_usrp_seq",
               "update_available_usrp_seq",
               "readwrite",
               "",
               "external",
               "configure");

    addProperty(clock_ref,
                "INTERNAL", 
               "clock_ref",
               "clock_ref",
               "readwrite",
               "",
               "external",
               "configure");
            
        available_usrp_seq.resize(0);
    addProperty(available_usrp_seq,
               "available_usrp_seq",
               "available_usrp_seq",
               "readonly",
               "",
               "external",
               "configure");
            
        myDevice_channel_seq.resize(0);
    addProperty(myDevice_channel_seq,
               "myDevice_channel_seq",
               "myDevice_channel_seq",
               "readonly",
               "",
               "external",
               "configure");
            
        myDevice_motherboard_seq.resize(0);
    addProperty(myDevice_motherboard_seq,
               "myDevice_motherboard_seq",
               "myDevice_motherboard_seq",
               "readonly",
               "",
               "external",
               "configure");

    addProperty(frontend_tuner_allocation,
               "FRONTEND::tuner_allocation",
               "frontend_tuner_allocation",
               "readwrite",
               "",
               "external",
               "allocation");

    addProperty(frontend_listener_allocation,
                frontend_listener_allocation_struct(), 
               "FRONTEND::listener_allocation",
               "frontend_listener_allocation",
               "readwrite",
               "",
               "external",
               "allocation");
            
        frontend_tuner_status.resize(0);
    addProperty(frontend_tuner_status,
               "FRONTEND::tuner_status",
               "frontend_tuner_status",
               "readonly",
               "",
               "external",
               "configure");

    addProperty(gain_global,
                0, 
               "gain_global",
               "",
               "readwrite",
               "dB",
               "external",
               "configure");

    addProperty(group_id_global,
               "group_id_global",
               "",
               "readwrite",
               "",
               "external",
               "configure");

}
