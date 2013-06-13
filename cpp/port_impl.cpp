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

 
/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

 	Source: USRP_UHD.spd.xml
 	Generated on: Thu Aug 02 12:02:47 EDT 2012
 	Redhawk IDE
 	Version:M.1.8.1
 	Build id: v201207181831-r8893

*******************************************************************************************/

#include "USRP_UHD.h"
 
// ----------------------------------------------------------------------------------------
// BULKIO_dataShort_Out_i definition
// ----------------------------------------------------------------------------------------
BULKIO_dataShort_Out_i::BULKIO_dataShort_Out_i(std::string port_name, USRP_UHD_base *_parent) :
Port_Uses_base_impl(port_name)
{
    parent = static_cast<USRP_UHD_i *> (_parent);
    recConnectionsRefresh = false;
    recConnections.length(0);
}

BULKIO_dataShort_Out_i::~BULKIO_dataShort_Out_i()
{
}

/*
 * pushSRI
 *     description: send out SRI describing the data payload
 *
 *  H: structure of type BULKIO::StreamSRI with the SRI for this stream
 *    hversion
 *    xstart: start time of the stream
 *    xdelta: delta between two samples
 *    xunits: unit types from Platinum specification
 *    subsize: 0 if the data is one-dimensional
 *    ystart
 *    ydelta
 *    yunits: unit types from Platinum specification
 *    mode: 0-scalar, 1-complex
 *    streamID: stream identifier
 *    sequence<CF::DataType> keywords: unconstrained sequence of key-value pairs for additional description
 */
void BULKIO_dataShort_Out_i::pushSRI(const BULKIO::StreamSRI& H)
{
    std::vector < std::pair < BULKIO::dataShort_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                ((*i).first)->pushSRI(H);
            } catch(...) {
                std::cout << "Call to pushSRI by BULKIO_dataShort_Out_i failed" << std::endl;
            }
        }
    }

    currentSRIs[std::string(H.streamID)] = H;
    refreshSRI = false;

    return;
}



 
// ----------------------------------------------------------------------------------------
// BULKIO_dataShort_In_i definition
// ----------------------------------------------------------------------------------------
BULKIO_dataShort_In_i::BULKIO_dataShort_In_i(std::string port_name, USRP_UHD_base *_parent) : 
Port_Provides_base_impl(port_name)
{
    dataAvailable = new omni_condition(&dataAvailableMutex);
    queueSem = new queueSemaphore(100);
    blocking = false;
    parent = static_cast<USRP_UHD_i *> (_parent);
}

BULKIO_dataShort_In_i::~BULKIO_dataShort_In_i()
{
    block();
    while (workQueue.size() != 0) {
        dataTransfer *tmp = workQueue.front();
        workQueue.pop_front();
        delete tmp;
    }
}

BULKIO::PortStatistics * BULKIO_dataShort_In_i::statistics()
{
    boost::mutex::scoped_lock lock(dataBufferLock);
    BULKIO::PortStatistics_var recStat = new BULKIO::PortStatistics(stats.retrieve());
    // NOTE: You must delete the object that this function returns!
    return recStat._retn();
}

BULKIO::PortUsageType BULKIO_dataShort_In_i::state()
{
    boost::mutex::scoped_lock lock(dataBufferLock);
    if (workQueue.size() == queueSem->getMaxValue()) {
        return BULKIO::BUSY;
    } else if (workQueue.size() == 0) {
        return BULKIO::IDLE;
    } else {
        return BULKIO::ACTIVE;
    }

    return BULKIO::BUSY;
}

BULKIO::StreamSRISequence * BULKIO_dataShort_In_i::activeSRIs()
{
    boost::mutex::scoped_lock lock(sriUpdateLock);
    BULKIO::StreamSRISequence seq_rtn;
    std::map<std::string, std::pair<BULKIO::StreamSRI, bool> >::iterator currH;
    int i = 0;
    for (currH = currentHs.begin(); currH != currentHs.end(); currH++) {
    	i++;
    	seq_rtn.length(i);
    	seq_rtn[i] = currH->second.first;
    }
    BULKIO::StreamSRISequence_var retSRI = new BULKIO::StreamSRISequence(seq_rtn);

    // NOTE: You must delete the object that this function returns!
    return retSRI._retn();
}

int BULKIO_dataShort_In_i::getMaxQueueDepth()
{
    boost::mutex::scoped_lock lock(dataBufferLock);
    return queueSem->getMaxValue();
}

int BULKIO_dataShort_In_i::getCurrentQueueDepth()
{
    boost::mutex::scoped_lock lock(dataBufferLock);
    return workQueue.size();
}

void BULKIO_dataShort_In_i::setMaxQueueDepth(int newDepth)
{
    boost::mutex::scoped_lock lock(dataBufferLock);
    queueSem->setMaxValue(newDepth);
}

void BULKIO_dataShort_In_i::pushSRI(const BULKIO::StreamSRI& H)
{
    boost::mutex::scoped_lock lock(sriUpdateLock);
    BULKIO::StreamSRI tmpH = H;
    std::map<std::string, std::pair<BULKIO::StreamSRI, bool> >::iterator currH = currentHs.find(std::string(H.streamID));
    if (currH == currentHs.end()) {
    	currentHs[std::string(H.streamID)] = std::make_pair(tmpH, true);
        if (H.blocking) {
            boost::mutex::scoped_lock lock(dataBufferLock);
            blocking = true;
            queueSem->setCurrValue(workQueue.size());
        }
    } else {
    	if (!parent->compareSRI(tmpH, currH->second.first)) {
            currentHs[std::string(H.streamID)] = std::make_pair(tmpH, true);
            if (H.blocking) {
                boost::mutex::scoped_lock lock(dataBufferLock);
                blocking = true;
                queueSem->setCurrValue(workQueue.size());
            }
    	}
    }
}

void BULKIO_dataShort_In_i::pushPacket(const PortTypes::ShortSequence& data, const BULKIO::PrecisionUTCTime& T, CORBA::Boolean EOS, const char* streamID)
{
    if (queueSem->getMaxValue() == 0) {
        return;
    }    
    BULKIO::StreamSRI tmpH = {1, 0.0, 1.0, 1, 0, 0.0, 0.0, 0, 0, streamID, false, 0};
    bool sriChanged = false;
    bool portBlocking = false;

    std::map<std::string, std::pair<BULKIO::StreamSRI, bool> >::iterator currH;
    {
        boost::mutex::scoped_lock lock(sriUpdateLock);

        currH = currentHs.find(std::string(streamID));
        if (currH != currentHs.end()) {
            tmpH = currH->second.first;
            sriChanged = currH->second.second;
            currentHs[streamID] = std::make_pair(currH->second.first, false);
        }
        portBlocking = blocking;
    }

    if(portBlocking) {
        queueSem->incr();
        boost::mutex::scoped_lock lock(dataBufferLock);
        stats.update(data.length(), workQueue.size()/queueSem->getMaxValue(), EOS, streamID, false);
        BULKIO_dataShort_In_i::dataTransfer *tmpIn = new BULKIO_dataShort_In_i::dataTransfer(data, T, EOS, streamID, tmpH, sriChanged, false);
        workQueue.push_back(tmpIn);
        dataAvailable->signal();
    } else {
        boost::mutex::scoped_lock lock(dataBufferLock);
        bool flushToReport = false;
        if (workQueue.size() == queueSem->getMaxValue()) { // reached maximum queue depth - flush the queue
            flushToReport = true;
            BULKIO_dataShort_In_i::dataTransfer *tmp;
            while (workQueue.size() != 0) {
                tmp = workQueue.front();
                workQueue.pop_front();
                delete tmp;
            }
        }
        stats.update(data.length(), workQueue.size()/queueSem->getMaxValue(), EOS, streamID, flushToReport);
        BULKIO_dataShort_In_i::dataTransfer *tmpIn = new BULKIO_dataShort_In_i::dataTransfer(data, T, EOS, streamID, tmpH, sriChanged, flushToReport);
        workQueue.push_back(tmpIn);
        dataAvailable->signal();
    }
}


void BULKIO_dataShort_In_i::block()
{
    breakBlock = true;
    dataAvailable->signal();
}

void BULKIO_dataShort_In_i::unblock()
{
    breakBlock = false;
}

/*
 * getPacket
 *     description: retrieve data from the provides (input) port
 *
 *  timeout: the amount of time to wait for data before a NULL is returned.
 *           Use 0.0 for non-blocking and -1 for blocking.
 */
BULKIO_dataShort_In_i::dataTransfer *BULKIO_dataShort_In_i::getPacket(float timeout)
{
    if (breakBlock) {
        return NULL;
    }
    if (workQueue.size() == 0) {
        if (timeout == 0.0) {
            return NULL;
        } else if (timeout > 0){
            secs = (unsigned long)(trunc(timeout));
            nsecs = (unsigned long)((timeout - secs) * 1e9);
            omni_thread::get_time(&timeout_secs, &timeout_nsecs, secs, nsecs);
            if (not dataAvailable->timedwait(timeout_secs, timeout_nsecs)) {
                return NULL;
            }
            if (breakBlock) {
                return NULL;
            }
        } else {
            dataAvailable->wait();
            if (breakBlock) {
                return NULL;
            }
        }
    }
    boost::mutex::scoped_lock lock1(dataBufferLock);
    BULKIO_dataShort_In_i::dataTransfer *tmp = workQueue.front();
    workQueue.pop_front();
    
    boost::mutex::scoped_lock lock2(sriUpdateLock);
    if (tmp->EOS) {
	    std::map<std::string, std::pair<BULKIO::StreamSRI, bool> >::iterator target = currentHs.find(std::string(tmp->streamID));
        if (target != currentHs.end()) {
            currentHs.erase(target);
            if (target->second.first.blocking) {
                std::map<std::string, std::pair<BULKIO::StreamSRI, bool> >::iterator currH;
                bool keepBlocking = false;
                for (currH = currentHs.begin(); currH != currentHs.end(); currH++) {
                if (currH->second.first.blocking) {
                        keepBlocking = true;
                        break;
                    }
                }

                if (!keepBlocking) {
                    queueSem->setCurrValue(0);
                    blocking = false;
                }
            }
        }
    }

    if (blocking) {
        queueSem->decr();
    }
    
    return tmp;
}

 

// ----------------------------------------------------------------------------------------
// FRONTEND_DigitalTuner_In_i definition
// ----------------------------------------------------------------------------------------
FRONTEND_DigitalTuner_In_i::FRONTEND_DigitalTuner_In_i(std::string port_name, USRP_UHD_base *_parent) : 
Port_Provides_base_impl(port_name)
{
    parent = static_cast<USRP_UHD_i *> (_parent);
}

FRONTEND_DigitalTuner_In_i::~FRONTEND_DigitalTuner_In_i()
{
}


char* FRONTEND_DigitalTuner_In_i::getTunerType(const char* id)
{
    boost::mutex::scoped_lock lock(portAccess);
    char* tmpVal = "";
    // TODO: Fill in this function

    return CORBA::string_dup(tmpVal);
}


CORBA::Boolean FRONTEND_DigitalTuner_In_i::getTunerDeviceControl(const char* id)
{
    boost::mutex::scoped_lock lock(portAccess);
    CORBA::Boolean tmpVal = false;
    // TODO: Fill in this function

    return tmpVal;
}


char* FRONTEND_DigitalTuner_In_i::getTunerGroupId(const char* id)
{
    boost::mutex::scoped_lock lock(portAccess);
    char* tmpVal = "";
    // TODO: Fill in this function

    return CORBA::string_dup(tmpVal);
}


char* FRONTEND_DigitalTuner_In_i::getTunerRfFlowId(const char* id)
{
    boost::mutex::scoped_lock lock(portAccess);
    char* tmpVal = "";
    // TODO: Fill in this function

    return CORBA::string_dup(tmpVal);
}


CF::Properties* FRONTEND_DigitalTuner_In_i::getTunerStatus(const char* id)
{
    boost::mutex::scoped_lock lock(portAccess);
    CF::Properties* tmpVal = NULL;
    // TODO: Fill in this function

    return tmpVal;
}


void FRONTEND_DigitalTuner_In_i::setTunerCenterFrequency(const char* id, CORBA::Double freq)
{
    boost::mutex::scoped_lock lock(portAccess);
    // TODO: Fill in this function
}


CORBA::Double FRONTEND_DigitalTuner_In_i::getTunerCenterFrequency(const char* id)
{
    boost::mutex::scoped_lock lock(portAccess);
    CORBA::Double tmpVal = 0.0;
    // TODO: Fill in this function

    return tmpVal;
}


void FRONTEND_DigitalTuner_In_i::setTunerBandwidth(const char* id, CORBA::Double bw)
{
    boost::mutex::scoped_lock lock(portAccess);
    // TODO: Fill in this function
}


CORBA::Double FRONTEND_DigitalTuner_In_i::getTunerBandwidth(const char* id)
{
    boost::mutex::scoped_lock lock(portAccess);
    CORBA::Double tmpVal = 0.0;
    // TODO: Fill in this function

    return tmpVal;
}


void FRONTEND_DigitalTuner_In_i::setTunerAgcEnable(const char* id, CORBA::Boolean enable)
{
    boost::mutex::scoped_lock lock(portAccess);
    // TODO: Fill in this function
}


CORBA::Boolean FRONTEND_DigitalTuner_In_i::getTunerAgcEnable(const char* id)
{
    boost::mutex::scoped_lock lock(portAccess);
    CORBA::Boolean tmpVal = false;
    // TODO: Fill in this function

    return tmpVal;
}


void FRONTEND_DigitalTuner_In_i::setTunerGain(const char* id, CORBA::Float gain)
{
    boost::mutex::scoped_lock lock(portAccess);
    // TODO: Fill in this function
}


CORBA::Float FRONTEND_DigitalTuner_In_i::getTunerGain(const char* id)
{
    boost::mutex::scoped_lock lock(portAccess);
    CORBA::Float tmpVal = 0.0;
    // TODO: Fill in this function

    return tmpVal;
}


void FRONTEND_DigitalTuner_In_i::setTunerReferenceSource(const char* id, CORBA::Long source)
{
    boost::mutex::scoped_lock lock(portAccess);
    // TODO: Fill in this function
}


CORBA::Long FRONTEND_DigitalTuner_In_i::getTunerReferenceSource(const char* id)
{
    boost::mutex::scoped_lock lock(portAccess);
    CORBA::Long tmpVal = 0;
    // TODO: Fill in this function

    return tmpVal;
}


void FRONTEND_DigitalTuner_In_i::setTunerEnable(const char* id, CORBA::Boolean enable)
{
    boost::mutex::scoped_lock lock(portAccess);
    // TODO: Fill in this function
}


CORBA::Boolean FRONTEND_DigitalTuner_In_i::getTunerEnable(const char* id)
{
    boost::mutex::scoped_lock lock(portAccess);
    CORBA::Boolean tmpVal = false;
    // TODO: Fill in this function

    return tmpVal;
}


void FRONTEND_DigitalTuner_In_i::setTunerOutputSampleRate(const char* id, CORBA::Double sr)
{
    boost::mutex::scoped_lock lock(portAccess);
    // TODO: Fill in this function
}


CORBA::Double FRONTEND_DigitalTuner_In_i::getTunerOutputSampleRate(const char* id)
{
    boost::mutex::scoped_lock lock(portAccess);
    CORBA::Double tmpVal = 0.0;
    // TODO: Fill in this function

    return tmpVal;
}


 
// ----------------------------------------------------------------------------------------
// BULKIO_dataFloat_In_i definition
// ----------------------------------------------------------------------------------------
BULKIO_dataFloat_In_i::BULKIO_dataFloat_In_i(std::string port_name, USRP_UHD_base *_parent) : 
Port_Provides_base_impl(port_name)
{
    dataAvailable = new omni_condition(&dataAvailableMutex);
    queueSem = new queueSemaphore(100);
    blocking = false;
    parent = static_cast<USRP_UHD_i *> (_parent);
}

BULKIO_dataFloat_In_i::~BULKIO_dataFloat_In_i()
{
    block();
    while (workQueue.size() != 0) {
        dataTransfer *tmp = workQueue.front();
        workQueue.pop_front();
        delete tmp;
    }
}

BULKIO::PortStatistics * BULKIO_dataFloat_In_i::statistics()
{
    boost::mutex::scoped_lock lock(dataBufferLock);
    BULKIO::PortStatistics_var recStat = new BULKIO::PortStatistics(stats.retrieve());
    // NOTE: You must delete the object that this function returns!
    return recStat._retn();
}

BULKIO::PortUsageType BULKIO_dataFloat_In_i::state()
{
    boost::mutex::scoped_lock lock(dataBufferLock);
    if (workQueue.size() == queueSem->getMaxValue()) {
        return BULKIO::BUSY;
    } else if (workQueue.size() == 0) {
        return BULKIO::IDLE;
    } else {
        return BULKIO::ACTIVE;
    }

    return BULKIO::BUSY;
}

BULKIO::StreamSRISequence * BULKIO_dataFloat_In_i::activeSRIs()
{
    boost::mutex::scoped_lock lock(sriUpdateLock);
    BULKIO::StreamSRISequence seq_rtn;
    std::map<std::string, std::pair<BULKIO::StreamSRI, bool> >::iterator currH;
    int i = 0;
    for (currH = currentHs.begin(); currH != currentHs.end(); currH++) {
    	i++;
    	seq_rtn.length(i);
    	seq_rtn[i] = currH->second.first;
    }
    BULKIO::StreamSRISequence_var retSRI = new BULKIO::StreamSRISequence(seq_rtn);

    // NOTE: You must delete the object that this function returns!
    return retSRI._retn();
}

int BULKIO_dataFloat_In_i::getMaxQueueDepth()
{
    boost::mutex::scoped_lock lock(dataBufferLock);
    return queueSem->getMaxValue();
}

int BULKIO_dataFloat_In_i::getCurrentQueueDepth()
{
    boost::mutex::scoped_lock lock(dataBufferLock);
    return workQueue.size();
}

void BULKIO_dataFloat_In_i::setMaxQueueDepth(int newDepth)
{
    boost::mutex::scoped_lock lock(dataBufferLock);
    queueSem->setMaxValue(newDepth);
}

void BULKIO_dataFloat_In_i::pushSRI(const BULKIO::StreamSRI& H)
{
    boost::mutex::scoped_lock lock(sriUpdateLock);
    BULKIO::StreamSRI tmpH = H;
    std::map<std::string, std::pair<BULKIO::StreamSRI, bool> >::iterator currH = currentHs.find(std::string(H.streamID));
    if (currH == currentHs.end()) {
    	currentHs[std::string(H.streamID)] = std::make_pair(tmpH, true);
        if (H.blocking) {
            boost::mutex::scoped_lock lock(dataBufferLock);
            blocking = true;
            queueSem->setCurrValue(workQueue.size());
        }
    } else {
    	if (!parent->compareSRI(tmpH, currH->second.first)) {
            currentHs[std::string(H.streamID)] = std::make_pair(tmpH, true);
            if (H.blocking) {
                boost::mutex::scoped_lock lock(dataBufferLock);
                blocking = true;
                queueSem->setCurrValue(workQueue.size());
            }
    	}
    }
}

void BULKIO_dataFloat_In_i::pushPacket(const PortTypes::FloatSequence& data, const BULKIO::PrecisionUTCTime& T, CORBA::Boolean EOS, const char* streamID)
{
    if (queueSem->getMaxValue() == 0) {
        return;
    }    
    BULKIO::StreamSRI tmpH = {1, 0.0, 1.0, 1, 0, 0.0, 0.0, 0, 0, streamID, false, 0};
    bool sriChanged = false;
    bool portBlocking = false;

    std::map<std::string, std::pair<BULKIO::StreamSRI, bool> >::iterator currH;
    {
        boost::mutex::scoped_lock lock(sriUpdateLock);

        currH = currentHs.find(std::string(streamID));
        if (currH != currentHs.end()) {
            tmpH = currH->second.first;
            sriChanged = currH->second.second;
            currentHs[streamID] = std::make_pair(currH->second.first, false);
        }
        portBlocking = blocking;
    }

    if(portBlocking) {
        queueSem->incr();
        boost::mutex::scoped_lock lock(dataBufferLock);
        stats.update(data.length(), workQueue.size()/queueSem->getMaxValue(), EOS, streamID, false);
        BULKIO_dataFloat_In_i::dataTransfer *tmpIn = new BULKIO_dataFloat_In_i::dataTransfer(data, T, EOS, streamID, tmpH, sriChanged, false);
        workQueue.push_back(tmpIn);
        dataAvailable->signal();
    } else {
        boost::mutex::scoped_lock lock(dataBufferLock);
        bool flushToReport = false;
        if (workQueue.size() == queueSem->getMaxValue()) { // reached maximum queue depth - flush the queue
            flushToReport = true;
            BULKIO_dataFloat_In_i::dataTransfer *tmp;
            while (workQueue.size() != 0) {
                tmp = workQueue.front();
                workQueue.pop_front();
                delete tmp;
            }
        }
        stats.update(data.length(), workQueue.size()/queueSem->getMaxValue(), EOS, streamID, flushToReport);
        BULKIO_dataFloat_In_i::dataTransfer *tmpIn = new BULKIO_dataFloat_In_i::dataTransfer(data, T, EOS, streamID, tmpH, sriChanged, flushToReport);
        workQueue.push_back(tmpIn);
        dataAvailable->signal();
    }
}


void BULKIO_dataFloat_In_i::block()
{
    breakBlock = true;
    dataAvailable->signal();
}

void BULKIO_dataFloat_In_i::unblock()
{
    breakBlock = false;
}

/*
 * getPacket
 *     description: retrieve data from the provides (input) port
 *
 *  timeout: the amount of time to wait for data before a NULL is returned.
 *           Use 0.0 for non-blocking and -1 for blocking.
 */
BULKIO_dataFloat_In_i::dataTransfer *BULKIO_dataFloat_In_i::getPacket(float timeout)
{
    if (breakBlock) {
        return NULL;
    }
    if (workQueue.size() == 0) {
        if (timeout == 0.0) {
            return NULL;
        } else if (timeout > 0){
            secs = (unsigned long)(trunc(timeout));
            nsecs = (unsigned long)((timeout - secs) * 1e9);
            omni_thread::get_time(&timeout_secs, &timeout_nsecs, secs, nsecs);
            if (not dataAvailable->timedwait(timeout_secs, timeout_nsecs)) {
                return NULL;
            }
            if (breakBlock) {
                return NULL;
            }
        } else {
            dataAvailable->wait();
            if (breakBlock) {
                return NULL;
            }
        }
    }
    boost::mutex::scoped_lock lock1(dataBufferLock);
    BULKIO_dataFloat_In_i::dataTransfer *tmp = workQueue.front();
    workQueue.pop_front();
    
    boost::mutex::scoped_lock lock2(sriUpdateLock);
    if (tmp->EOS) {
	    std::map<std::string, std::pair<BULKIO::StreamSRI, bool> >::iterator target = currentHs.find(std::string(tmp->streamID));
        if (target != currentHs.end()) {
            currentHs.erase(target);
            if (target->second.first.blocking) {
                std::map<std::string, std::pair<BULKIO::StreamSRI, bool> >::iterator currH;
                bool keepBlocking = false;
                for (currH = currentHs.begin(); currH != currentHs.end(); currH++) {
                if (currH->second.first.blocking) {
                        keepBlocking = true;
                        break;
                    }
                }

                if (!keepBlocking) {
                    queueSem->setCurrValue(0);
                    blocking = false;
                }
            }
        }
    }

    if (blocking) {
        queueSem->decr();
    }
    
    return tmp;
}

 

// ----------------------------------------------------------------------------------------
// FRONTEND_RFInfo_In_i definition
// ----------------------------------------------------------------------------------------
FRONTEND_RFInfo_In_i::FRONTEND_RFInfo_In_i(std::string port_name, USRP_UHD_base *_parent) : 
Port_Provides_base_impl(port_name)
{
    parent = static_cast<USRP_UHD_i *> (_parent);
}

FRONTEND_RFInfo_In_i::~FRONTEND_RFInfo_In_i()
{
}

char* FRONTEND_RFInfo_In_i::rf_flow_id()
{
    boost::mutex::scoped_lock lock(portAccess);
    
    char tmpVal[255] = {0};;
    // TODO: Fill in this function

    return CORBA::string_dup(tmpVal);
}

void FRONTEND_RFInfo_In_i::rf_flow_id(const char* data)
{
    boost::mutex::scoped_lock lock(portAccess);
    
    // TODO: Fill in this function
}

FRONTEND::RFInfoPkt* FRONTEND_RFInfo_In_i::rfinfo_pkt()
{
    boost::mutex::scoped_lock lock(portAccess);
    
    FRONTEND::RFInfoPkt* tmpVal = 0;
    // TODO: Fill in this function

    return tmpVal;
}

void FRONTEND_RFInfo_In_i::rfinfo_pkt(const FRONTEND::RFInfoPkt& data)
{
    boost::mutex::scoped_lock lock(portAccess);
    
    // TODO: Fill in this function
}


