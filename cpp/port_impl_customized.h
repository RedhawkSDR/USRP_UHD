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
#ifndef USRP_UHD_PORTCUSTOM_H
#define USRP_UHD_PORTCUSTOM_H

#include<bulkio/bulkio.h>
#include "sdds/SddsProcessor.h"

//#define CORBA_MAX_TRANSFER_BYTES omniORB::giopMaxMsgSize()

// ----------------------------------------------------------------------------------------
// OutSDDSPort_customized declaration
// ----------------------------------------------------------------------------------------

template <class DATA_TYPE>
class OutSDDSPort_customized : public bulkio::OutSDDSPort {
    ENABLE_LOGGING
    typedef boost::shared_ptr<SddsProcessor<DATA_TYPE> > proc_ptr_t;
    typedef std::map<std::string, proc_ptr_t > streamid_to_proc_map_t;
    typedef std::map<std::string, BULKIO::StreamSRI> streamid_to_sri_map_t;

public:
    OutSDDSPort_customized(std::string port_name);
    ~OutSDDSPort_customized();

    bool setStream(std::string streamID, std::string iface, std::string ip, long port, unsigned short vlan,
            std::string attach_user_id, long ttv=-1, long endiance=-1, bool sri_has_priority=false,
            size_t buffer_size=SDDS_DATA_SIZE/sizeof(DATA_TYPE), size_t buffer_cnt=20000);
    bool startStream(std::string streamID);
    //bool endStream(std::string streamID);
    //bool removeStream(std::string streamID)
    //  - should call setActiveStatus(false) if previous function(s) are implemented/overridden

    /* NOTE: Parent class shall call pushSRI without a timestamp. SRI and data are kept in sync and the timestamp
     *       from the data will be used to send SRI when necessary. Using pushSRI call with timestamp will cause
     *       SRI to be pushed without all necessary keywords for SDDS.
     */

    // these functions are used by parent class to send SRI and data to SDDS port
    // they interface with SddsProcessor, which handles sending sri and data to connections
    // SddsProcessor calls non-virtual bulkio::OutSDDSPort::pushSRI(H,T) rather than the overridden one here
    void pushSRI(const BULKIO::StreamSRI& H, const BULKIO::PrecisionUTCTime& T) {
        TRACE_ENTER(OutSDDSPort_customized);
        LOG_DEBUG(OutSDDSPort_customized,__PRETTY_FUNCTION__<<"OutSDDSPort_customized::pushSRI(H,T) called, NOT bulkio::OutSDDSPort::pushSRI(H,T)");
        pushSRI(H);
        TRACE_EXIT(OutSDDSPort_customized);
    }
    void pushSRI(const BULKIO::StreamSRI& H); // sri is in sync with data still, so use timestamp from data
    void pushPacket(std::vector<DATA_TYPE>& data, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID);

private:
    void pushSriOnConnect(const char *connectionId);
    streamid_to_proc_map_t streamid_to_processor;
    streamid_to_sri_map_t streamid_to_sri; // updated SRIs to be sent to SddsProcessor on next pushPacket.
    boost::mutex input_port_lock; // used for input to SddsProcessor. Base class mutex used for all else.

};

#endif
