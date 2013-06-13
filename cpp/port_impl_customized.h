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

 
#ifndef PORTCUSTOM_H
#define PORTCUSTOM_H

#include "ossie/Port_impl.h"
#include <queue>
#include <list>
#include <iostream>
#include "port_impl.h"
#include "ossie/CF/QueryablePort.h"
#include "BULKIO/bio_dataShort.h"
#include "FRONTEND/TunerControl.h"

#define CORBA_MAX_TRANSFER_BYTES omniORB::giopMaxMsgSize()


class USRP_UHD_base;
class USRP_UHD_i;


// ----------------------------------------------------------------------------------------
// BULKIO_datashort_Out_i declaration
// ----------------------------------------------------------------------------------------


class BULKIO_dataShort_Out_i_customized : public BULKIO_dataShort_Out_i {
public:
	BULKIO_dataShort_Out_i_customized(std::string port_name, USRP_UHD_base *_parent);
    ~BULKIO_dataShort_Out_i_customized();

    void pushSRI_on_connection(const std::string& connection_name);
    void pushSRI(const BULKIO::StreamSRI& H);
    void pushPacket(short* data, size_t length, BULKIO::PrecisionUTCTime& T, bool EOS, std::string& streamID);

    template <typename ALLOCATOR>  void pushPacket(std::vector<CORBA::Short, ALLOCATOR>& data, BULKIO::PrecisionUTCTime& T, bool EOS, std::string& streamID) {
        pushPacket((short*) &(data[0]), data.size(), T, EOS, streamID);
    };


    void connectPort(CORBA::Object_ptr connection, const char* connectionId) {
        {
        boost::mutex::scoped_lock lock(updatingPortsLock); // don't want to process while command information is coming in
        BULKIO::dataShort_var port = BULKIO::dataShort::_narrow(connection);
        outConnections.push_back(std::make_pair(port, connectionId));
        active = true;
        refreshSRI = true;
        }
         pushSRI_on_connection(std::string(connectionId));
    };


private:

};


// ----------------------------------------------------------------------------------------
// FRONTEND_DigitalTuner_In_i_customized declaration
// ----------------------------------------------------------------------------------------
class FRONTEND_DigitalTuner_In_i_customized : public FRONTEND_DigitalTuner_In_i
{
	public:
        FRONTEND_DigitalTuner_In_i_customized(std::string port_name, USRP_UHD_base *_parent);
        ~FRONTEND_DigitalTuner_In_i_customized();

        char* getTunerType(const char* id);
        CORBA::Boolean getTunerDeviceControl(const char* id);
        char* getTunerGroupId(const char* id);
        char* getTunerRfFlowId(const char* id);
        CF::Properties* getTunerStatus(const char* id);
        void setTunerCenterFrequency(const char* id, CORBA::Double freq);
        CORBA::Double getTunerCenterFrequency(const char* id);
        void setTunerBandwidth(const char* id, CORBA::Double bw);
        CORBA::Double getTunerBandwidth(const char* id);
        void setTunerAgcEnable(const char* id, CORBA::Boolean enable);
        CORBA::Boolean getTunerAgcEnable(const char* id);
        void setTunerGain(const char* id, CORBA::Float gain);
        CORBA::Float getTunerGain(const char* id);
        void setTunerReferenceSource(const char* id, CORBA::Long source);
        CORBA::Long getTunerReferenceSource(const char* id);
        void setTunerEnable(const char* id, CORBA::Boolean enable);
        CORBA::Boolean getTunerEnable(const char* id);
        void setTunerOutputSampleRate(const char* id, CORBA::Double sr);
        CORBA::Double getTunerOutputSampleRate(const char* id);


    private:

};

// ----------------------------------------------------------------------------------------
// FRONTEND_RFInfo_In_i declaration
// ----------------------------------------------------------------------------------------
class FRONTEND_RFInfo_In_i_customized : public FRONTEND_RFInfo_In_i
{
    public:
		FRONTEND_RFInfo_In_i_customized(std::string port_name, USRP_UHD_base *_parent);
        ~FRONTEND_RFInfo_In_i_customized();

        char* rf_flow_id();
        void rf_flow_id(const char* data);

        FRONTEND::RFInfoPkt* rfinfo_pkt();
        void rfinfo_pkt(const FRONTEND::RFInfoPkt& data);
//
//
//
//        void sendRFInfoPacket(const FRONTEND::RFInfoPkt& packet, const BULKIO::PrecisionUTCTime& T);
//        void setRfFlowId(const char* RfFlowId);
//
//
//        std::string rf_flow_id;
};



#endif
