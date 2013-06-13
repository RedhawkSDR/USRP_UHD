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


#ifndef PORT_H
#define PORT_H

#include "ossie/Port_impl.h"
#include <queue>
#include <list>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>

class USRP_UHD_base;
class USRP_UHD_i;

#define CORBA_MAX_TRANSFER_BYTES omniORB::giopMaxMsgSize()

#include "ossie/CF/QueryablePort.h"
 
#include "BULKIO/bio_dataShort.h"
#include "FRONTEND/TunerControl.h"
#include "FRONTEND/RFInfo.h"
#include "BULKIO/bio_dataFloat.h"

class queueSemaphore
{
    private:
        unsigned int maxValue;
        unsigned int currValue;
        boost::mutex mutex;
        boost::condition_variable condition;

    public:
        queueSemaphore(unsigned int initialMaxValue):mutex(),condition() {
        	maxValue = initialMaxValue;
        }

        void setMaxValue(unsigned int newMaxValue) {
            boost::unique_lock<boost::mutex> lock(mutex);
            maxValue = newMaxValue;
        }

        unsigned int getMaxValue(void) {
			boost::unique_lock<boost::mutex> lock(mutex);
			return maxValue;
		}

        void setCurrValue(unsigned int newValue) {
        	boost::unique_lock<boost::mutex> lock(mutex);
        	if (newValue < maxValue) {
        		unsigned int oldValue = currValue;
        		currValue = newValue;

        		if (oldValue > newValue) {
        			condition.notify_one();
        		}
        	}
        }

        void incr() {
            boost::unique_lock<boost::mutex> lock(mutex);
            while (currValue >= maxValue) {
                condition.wait(lock);
            }
            ++currValue;
        }

        void decr() {
            boost::unique_lock<boost::mutex> lock(mutex);
            if (currValue > 0) {
            	--currValue;
            }
            condition.notify_one();
        }
};        
 
// ----------------------------------------------------------------------------------------
// BULKIO_dataShort_Out_i declaration
// ----------------------------------------------------------------------------------------
class BULKIO_dataShort_Out_i : public Port_Uses_base_impl, public virtual POA_BULKIO::UsesPortStatisticsProvider
{
    public:
        BULKIO_dataShort_Out_i(std::string port_name, USRP_UHD_base *_parent);
        ~BULKIO_dataShort_Out_i();

        void pushSRI(const BULKIO::StreamSRI& H);
        
        /*
         * pushPacket
         *     description: push data out of the port
         *
         *  data: structure containing the payload to send out
         *  T: constant of type BULKIO::PrecisionUTCTime containing the timestamp for the outgoing data.
         *    tcmode: timecode mode
         *    tcstatus: timecode status 
         *    toff: fractional sample offset
         *    twsec: J1970 GMT 
         *    tfsec: fractional seconds: 0.0 to 1.0
         *  EOS: end-of-stream flag
         *  streamID: stream identifier
         */
        template <typename ALLOCATOR>
        void pushPacket(std::vector<CORBA::Short, ALLOCATOR>& data, BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID) {
            if (refreshSRI) {
                if (currentSRIs.find(streamID) != currentSRIs.end()) {
                    pushSRI(currentSRIs[streamID]);
                }
            }
            boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
            // Magic is below, make a new sequence using the data from the Iterator
            // as the data for the sequence.  The 'false' at the end is whether or not
            // CORBA is allowed to delete the buffer when the sequence is destroyed.
            PortTypes::ShortSequence seq = PortTypes::ShortSequence(data.size(), data.size(), &(data[0]), false);
            if (active) {
                std::vector < std::pair < BULKIO::dataShort_var, std::string > >::iterator port;
                for (port = outConnections.begin(); port != outConnections.end(); port++) {
                    try {
                        ((*port).first)->pushPacket(seq, T, EOS, streamID.c_str());
                        stats[(*port).second].update(data.size(), 0, 0, streamID);
                    } catch(...) {
                        std::cout << "Call to pushPacket by BULKIO_dataShort_Out_i failed" << std::endl;
                    }
                }
            }
        };
        class linkStatistics
        {
            public:
                struct statPoint {
                    unsigned int elements;
                    float queueSize;
                    double secs;
                    double usecs;
                };
                
                linkStatistics() {
                    bitSize = sizeof(CORBA::Short) * 8.0;
                    historyWindow = 10;
                    activeStreamIDs.resize(0);
                    receivedStatistics_idx = 0;
                    receivedStatistics.resize(historyWindow);
                    runningStats.elementsPerSecond = -1.0;
                    runningStats.bitsPerSecond = -1.0;
                    runningStats.callsPerSecond = -1.0;
                    runningStats.averageQueueDepth = -1.0;
                    runningStats.streamIDs.length(0);
                    runningStats.timeSinceLastCall = -1;
                    enabled = true;
                };

                void setEnabled(bool enableStats) {
                    enabled = enableStats;
                }

                void update(unsigned int elementsReceived, float queueSize, bool EOS, std::string streamID) {
                    if (!enabled) {
                        return;
                    }
                    struct timeval tv;
                    struct timezone tz;
                    gettimeofday(&tv, &tz);
                    receivedStatistics[receivedStatistics_idx].elements = elementsReceived;
                    receivedStatistics[receivedStatistics_idx].queueSize = queueSize;
                    receivedStatistics[receivedStatistics_idx].secs = tv.tv_sec;
                    receivedStatistics[receivedStatistics_idx++].usecs = tv.tv_usec;
                    receivedStatistics_idx = receivedStatistics_idx % historyWindow;
                    if (!EOS) {
                        std::list<std::string>::iterator p = activeStreamIDs.begin();
                        bool foundStreamID = false;
                        while (p != activeStreamIDs.end()) {
                            if (*p == streamID) {
                                foundStreamID = true;
                                break;
                            }
                            p++;
                        }
                        if (!foundStreamID) {
                            activeStreamIDs.push_back(streamID);
                        }
                    } else {
                        std::list<std::string>::iterator p = activeStreamIDs.begin();
                        while (p != activeStreamIDs.end()) {
                            if (*p == streamID) {
                                activeStreamIDs.erase(p);
                                break;
                            }
                            p++;
                        }
                    }
                };

                BULKIO::PortStatistics retrieve() {
                    if (!enabled) {
                        return runningStats;
                    }
                    struct timeval tv;
                    struct timezone tz;
                    gettimeofday(&tv, &tz);

                    int idx = (receivedStatistics_idx == 0) ? (historyWindow - 1) : (receivedStatistics_idx - 1);
                    double front_sec = receivedStatistics[idx].secs;
                    double front_usec = receivedStatistics[idx].usecs;
                    double secDiff = tv.tv_sec - receivedStatistics[receivedStatistics_idx].secs;
                    double usecDiff = (tv.tv_usec - receivedStatistics[receivedStatistics_idx].usecs) / ((double)1e6);

                    double totalTime = secDiff + usecDiff;
                    double totalData = 0;
                    float queueSize = 0;
                    int startIdx = (receivedStatistics_idx + 1) % historyWindow;
                    for (int i = startIdx; i != receivedStatistics_idx; ) {
                        totalData += receivedStatistics[i].elements;
                        queueSize += receivedStatistics[i].queueSize;
                        i = (i + 1) % historyWindow;
                    }
                    runningStats.bitsPerSecond = ((totalData * bitSize) / totalTime);
                    runningStats.elementsPerSecond = (totalData / totalTime);
                    runningStats.averageQueueDepth = (queueSize / historyWindow);
                    runningStats.callsPerSecond = (double(historyWindow - 1) / totalTime);
                    runningStats.timeSinceLastCall = (((double)tv.tv_sec) - front_sec) + (((double)tv.tv_usec - front_usec) / ((double)1e6));
                    unsigned int streamIDsize = activeStreamIDs.size();
                    std::list< std::string >::iterator p = activeStreamIDs.begin();
                    runningStats.streamIDs.length(streamIDsize);
                    for (unsigned int i = 0; i < streamIDsize; i++) {
                        if (p == activeStreamIDs.end()) {
                            break;
                        }
                        runningStats.streamIDs[i] = CORBA::string_dup((*p).c_str());
                        p++;
                    }
                    return runningStats;
                };

            protected:
                bool enabled;
                double bitSize;
                BULKIO::PortStatistics runningStats;
                std::vector<statPoint> receivedStatistics;
                std::list< std::string > activeStreamIDs;
                unsigned long historyWindow;
                int receivedStatistics_idx;
        };

        BULKIO::UsesPortStatisticsSequence * statistics()
        {
            boost::mutex::scoped_lock lock(updatingPortsLock);
            BULKIO::UsesPortStatisticsSequence_var recStat = new BULKIO::UsesPortStatisticsSequence();
            recStat->length(outConnections.size());
            for (unsigned int i = 0; i < outConnections.size(); i++) {
                recStat[i].connectionId = CORBA::string_dup(outConnections[i].second.c_str());
                recStat[i].statistics = stats[outConnections[i].second].retrieve();
            }
            return recStat._retn();
        };

        BULKIO::PortUsageType state()
        {
            boost::mutex::scoped_lock lock(updatingPortsLock);
            if (outConnections.size() > 0) {
                return BULKIO::ACTIVE;
            } else {
                return BULKIO::IDLE;
            }

            return BULKIO::BUSY;
        };
        
        void enableStats(bool enable)
        {
            for (unsigned int i = 0; i < outConnections.size(); i++) {
                stats[outConnections[i].second].setEnabled(enable);
            }
        };


        ExtendedCF::UsesConnectionSequence * connections() 
        {
            boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
            if (recConnectionsRefresh) {
                recConnections.length(outConnections.size());
                for (unsigned int i = 0; i < outConnections.size(); i++) {
                    recConnections[i].connectionId = CORBA::string_dup(outConnections[i].second.c_str());
                    recConnections[i].port = CORBA::Object::_duplicate(outConnections[i].first);
                }
                recConnectionsRefresh = false;
            }
            ExtendedCF::UsesConnectionSequence_var retVal = new ExtendedCF::UsesConnectionSequence(recConnections);
            // NOTE: You must delete the object that this function returns!
            return retVal._retn();
        };

        void connectPort(CORBA::Object_ptr connection, const char* connectionId)
        {
            boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
            BULKIO::dataShort_var port = BULKIO::dataShort::_narrow(connection);
            outConnections.push_back(std::make_pair(port, connectionId));
            active = true;
            refreshSRI = true;
        };

        void disconnectPort(const char* connectionId)
        {
            boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
            for (unsigned int i = 0; i < outConnections.size(); i++) {
                if (outConnections[i].second == connectionId) {
                    outConnections.erase(outConnections.begin() + i);
                    break;
                }
            }

            if (outConnections.size() == 0) {
                active = false;
            }
        };

        std::vector< std::pair<BULKIO::dataShort_var, std::string> > _getConnections()
        {
            return outConnections;
        };
        std::map<std::string, BULKIO::StreamSRI> currentSRIs;

    protected:
        USRP_UHD_i *parent;
        std::vector < std::pair<BULKIO::dataShort_var, std::string> > outConnections;
        ExtendedCF::UsesConnectionSequence recConnections;
        bool recConnectionsRefresh;
        std::map<std::string, linkStatistics> stats;
};
 

// ----------------------------------------------------------------------------------------
// BULKIO_dataShort_In_i declaration
// ----------------------------------------------------------------------------------------
class BULKIO_dataShort_In_i : public POA_BULKIO::dataShort, public Port_Provides_base_impl
{
    public:
        BULKIO_dataShort_In_i(std::string port_name, USRP_UHD_base *_parent);
        ~BULKIO_dataShort_In_i();

        void pushSRI(const BULKIO::StreamSRI& H);
        void pushPacket(const PortTypes::ShortSequence& data, const BULKIO::PrecisionUTCTime& T, CORBA::Boolean EOS, const char* streamID);

        BULKIO::PortUsageType state();
        BULKIO::PortStatistics* statistics();
        BULKIO::StreamSRISequence* activeSRIs();
        int getCurrentQueueDepth();
        int getMaxQueueDepth();
        void setMaxQueueDepth(int newDepth);

        class linkStatistics
        {
            public:
                struct statPoint {
                    unsigned int elements;
                    float queueSize;
                    double secs;
                    double usecs;
                };

                linkStatistics() {
                    bitSize = sizeof(CORBA::Short) * 8.0;
                    historyWindow = 10;
                    receivedStatistics_idx = 0;
                    receivedStatistics.resize(historyWindow);
                    activeStreamIDs.resize(0);
                    runningStats.elementsPerSecond = -1.0;
                    runningStats.bitsPerSecond = -1.0;
                    runningStats.callsPerSecond = -1.0;
                    runningStats.averageQueueDepth = -1.0;
                    runningStats.streamIDs.length(0);
                    runningStats.timeSinceLastCall = -1;
                    enabled = true;
                    flush_sec = 0;
                    flush_usec = 0;
                };

                ~linkStatistics() {
                }

                void setEnabled(bool enableStats) {
                    enabled = enableStats;
                }

                void update(unsigned int elementsReceived, float queueSize, bool EOS, std::string streamID, bool flush) {
                    if (!enabled) {
                        return;
                    }
                    struct timeval tv;
                    struct timezone tz;
                    gettimeofday(&tv, &tz);
                    receivedStatistics[receivedStatistics_idx].elements = elementsReceived;
                    receivedStatistics[receivedStatistics_idx].queueSize = queueSize;
                    receivedStatistics[receivedStatistics_idx].secs = tv.tv_sec;
                    receivedStatistics[receivedStatistics_idx++].usecs = tv.tv_usec;
                    receivedStatistics_idx = receivedStatistics_idx % historyWindow;
                    if (flush) {
                        flush_sec = tv.tv_sec;
                        flush_usec = tv.tv_usec;
                    }
                    if (!EOS) {
                        std::list<std::string>::iterator p = activeStreamIDs.begin();
                        bool foundStreamID = false;
                        while (p != activeStreamIDs.end()) {
                            if (*p == streamID) {
                                foundStreamID = true;
                                break;
                            }
                            p++;
                        }
                        if (!foundStreamID) {
                            activeStreamIDs.push_back(streamID);
                        }
                    } else {
                        std::list<std::string>::iterator p = activeStreamIDs.begin();
                        while (p != activeStreamIDs.end()) {
                            if (*p == streamID) {
                                activeStreamIDs.erase(p);
                                break;
                            }
                            p++;
                        }
                    }
                }

                BULKIO::PortStatistics retrieve() {
                    if (!enabled) {
                        return runningStats;
                    }
                    struct timeval tv;
                    struct timezone tz;
                    gettimeofday(&tv, &tz);

                    int idx = (receivedStatistics_idx == 0) ? (historyWindow - 1) : (receivedStatistics_idx - 1);
                    double front_sec = receivedStatistics[idx].secs;
                    double front_usec = receivedStatistics[idx].usecs;
                    double secDiff = tv.tv_sec - receivedStatistics[receivedStatistics_idx].secs;
                    double usecDiff = (tv.tv_usec - receivedStatistics[receivedStatistics_idx].usecs) / ((double)1e6);
                    double totalTime = secDiff + usecDiff;
                    double totalData = 0;
                    float queueSize = 0;
                    int startIdx = (receivedStatistics_idx + 1) % historyWindow;
                    for (int i = startIdx; i != receivedStatistics_idx; ) {
                        totalData += receivedStatistics[i].elements;
                        queueSize += receivedStatistics[i].queueSize;
                        i = (i + 1) % historyWindow;
                    }
                    runningStats.bitsPerSecond = ((totalData * bitSize) / totalTime);
                    runningStats.elementsPerSecond = (totalData / totalTime);
                    runningStats.averageQueueDepth = (queueSize / historyWindow);
                    runningStats.callsPerSecond = (double(historyWindow - 1) / totalTime);
                    runningStats.timeSinceLastCall = (((double)tv.tv_sec) - front_sec) + (((double)tv.tv_usec - front_usec) / ((double)1e6));
                    unsigned int streamIDsize = activeStreamIDs.size();
                    std::list< std::string >::iterator p = activeStreamIDs.begin();
                    runningStats.streamIDs.length(streamIDsize);
                    for (unsigned int i = 0; i < streamIDsize; i++) {
                        if (p == activeStreamIDs.end()) {
                            break;
                        }
                        runningStats.streamIDs[i] = CORBA::string_dup((*p).c_str());
                        p++;
                    }
                    if ((flush_sec != 0) && (flush_usec != 0)) {
                        double flushTotalTime = (((double)tv.tv_sec) - flush_sec) + (((double)tv.tv_usec - flush_usec) / ((double)1e6));
                        runningStats.keywords.length(1);
                        runningStats.keywords[0].id = CORBA::string_dup("timeSinceLastFlush");
                        runningStats.keywords[0].value <<= CORBA::Double(flushTotalTime);
                    }
                    return runningStats;
                }

            protected:
                bool enabled;
                double bitSize;
                BULKIO::PortStatistics runningStats;
                std::vector<statPoint> receivedStatistics;
                std::list< std::string > activeStreamIDs;
                unsigned long historyWindow;
                long receivedStatistics_idx;
                double flush_sec;
                double flush_usec;
        };
        
        void enableStats(bool enable) {
            stats.setEnabled(enable);
        };


        class dataTransfer
        {
            public:
                dataTransfer(const PortTypes::ShortSequence& data, const BULKIO::PrecisionUTCTime &_T, bool _EOS, const char* _streamID, BULKIO::StreamSRI &_H, bool _sriChanged, bool _inputQueueFlushed)
                {
                    int dataLength = data.length();

#ifdef EXPECTED_VECTOR_IMPL
                    std::_Vector_base<CORBA::Short, _seqVector::seqVectorAllocator<CORBA::Short> >::_Vector_impl *vectorPointer = (std::_Vector_base<CORBA::Short, _seqVector::seqVectorAllocator<CORBA::Short> >::_Vector_impl *) ((void*) & dataBuffer);
                    vectorPointer->_M_start = const_cast<PortTypes::ShortSequence*>(&data)->get_buffer(1);
                    vectorPointer->_M_finish = vectorPointer->_M_start + dataLength;
                    vectorPointer->_M_end_of_storage = vectorPointer->_M_finish;

#else
                    dataBuffer.resize(dataLength);
                    if (dataLength > 0) {
                        memcpy(&dataBuffer[0], &data[0], dataLength * sizeof(data[0]));
                    }

#endif
                    T = _T;
                    EOS = _EOS;
                    streamID = _streamID;
                    SRI = _H;
                    sriChanged = _sriChanged;
                    inputQueueFlushed = _inputQueueFlushed;
                };

#ifdef EXPECTED_VECTOR_IMPL
                std::vector< CORBA::Short, _seqVector::seqVectorAllocator<CORBA::Short> > dataBuffer;
#else
                std::vector<CORBA::Short> dataBuffer;
#endif
                BULKIO::PrecisionUTCTime T;
                bool EOS;
                std::string streamID;
                BULKIO::StreamSRI SRI;
                bool sriChanged;
                bool inputQueueFlushed;
        };

        dataTransfer *getPacket(float timeout);
        void block();
        void unblock();

    protected:
        USRP_UHD_i *parent;
        std::deque<dataTransfer *> workQueue;
        std::map<std::string, std::pair<BULKIO::StreamSRI, bool> > currentHs;
        boost::mutex dataBufferLock;
        boost::mutex sriUpdateLock;
        omni_mutex dataAvailableMutex;
        omni_condition* dataAvailable;
        unsigned long secs, nsecs, timeout_secs, timeout_nsecs;
        bool breakBlock;
        bool blocking;
        queueSemaphore* queueSem;

        // statistics
        linkStatistics stats;

};

 

// ----------------------------------------------------------------------------------------
// FRONTEND_DigitalTuner_In_i declaration
// ----------------------------------------------------------------------------------------
class FRONTEND_DigitalTuner_In_i : public POA_FRONTEND::DigitalTuner, public Port_Provides_base_impl
{
    public:
        FRONTEND_DigitalTuner_In_i(std::string port_name, USRP_UHD_base *_parent);
        ~FRONTEND_DigitalTuner_In_i();

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


    protected:
        USRP_UHD_i *parent;
        boost::mutex portAccess;

};

 

// ----------------------------------------------------------------------------------------
// BULKIO_dataFloat_In_i declaration
// ----------------------------------------------------------------------------------------
class BULKIO_dataFloat_In_i : public POA_BULKIO::dataFloat, public Port_Provides_base_impl
{
    public:
        BULKIO_dataFloat_In_i(std::string port_name, USRP_UHD_base *_parent);
        ~BULKIO_dataFloat_In_i();

        void pushSRI(const BULKIO::StreamSRI& H);
        void pushPacket(const PortTypes::FloatSequence& data, const BULKIO::PrecisionUTCTime& T, CORBA::Boolean EOS, const char* streamID);

        BULKIO::PortUsageType state();
        BULKIO::PortStatistics* statistics();
        BULKIO::StreamSRISequence* activeSRIs();
        int getCurrentQueueDepth();
        int getMaxQueueDepth();
        void setMaxQueueDepth(int newDepth);

        class linkStatistics
        {
            public:
                struct statPoint {
                    unsigned int elements;
                    float queueSize;
                    double secs;
                    double usecs;
                };

                linkStatistics() {
                    bitSize = sizeof(CORBA::Float) * 8.0;
                    historyWindow = 10;
                    receivedStatistics_idx = 0;
                    receivedStatistics.resize(historyWindow);
                    activeStreamIDs.resize(0);
                    runningStats.elementsPerSecond = -1.0;
                    runningStats.bitsPerSecond = -1.0;
                    runningStats.callsPerSecond = -1.0;
                    runningStats.averageQueueDepth = -1.0;
                    runningStats.streamIDs.length(0);
                    runningStats.timeSinceLastCall = -1;
                    enabled = true;
                    flush_sec = 0;
                    flush_usec = 0;
                };

                ~linkStatistics() {
                }

                void setEnabled(bool enableStats) {
                    enabled = enableStats;
                }

                void update(unsigned int elementsReceived, float queueSize, bool EOS, std::string streamID, bool flush) {
                    if (!enabled) {
                        return;
                    }
                    struct timeval tv;
                    struct timezone tz;
                    gettimeofday(&tv, &tz);
                    receivedStatistics[receivedStatistics_idx].elements = elementsReceived;
                    receivedStatistics[receivedStatistics_idx].queueSize = queueSize;
                    receivedStatistics[receivedStatistics_idx].secs = tv.tv_sec;
                    receivedStatistics[receivedStatistics_idx++].usecs = tv.tv_usec;
                    receivedStatistics_idx = receivedStatistics_idx % historyWindow;
                    if (flush) {
                        flush_sec = tv.tv_sec;
                        flush_usec = tv.tv_usec;
                    }
                    if (!EOS) {
                        std::list<std::string>::iterator p = activeStreamIDs.begin();
                        bool foundStreamID = false;
                        while (p != activeStreamIDs.end()) {
                            if (*p == streamID) {
                                foundStreamID = true;
                                break;
                            }
                            p++;
                        }
                        if (!foundStreamID) {
                            activeStreamIDs.push_back(streamID);
                        }
                    } else {
                        std::list<std::string>::iterator p = activeStreamIDs.begin();
                        while (p != activeStreamIDs.end()) {
                            if (*p == streamID) {
                                activeStreamIDs.erase(p);
                                break;
                            }
                            p++;
                        }
                    }
                }

                BULKIO::PortStatistics retrieve() {
                    if (!enabled) {
                        return runningStats;
                    }
                    struct timeval tv;
                    struct timezone tz;
                    gettimeofday(&tv, &tz);

                    int idx = (receivedStatistics_idx == 0) ? (historyWindow - 1) : (receivedStatistics_idx - 1);
                    double front_sec = receivedStatistics[idx].secs;
                    double front_usec = receivedStatistics[idx].usecs;
                    double secDiff = tv.tv_sec - receivedStatistics[receivedStatistics_idx].secs;
                    double usecDiff = (tv.tv_usec - receivedStatistics[receivedStatistics_idx].usecs) / ((double)1e6);
                    double totalTime = secDiff + usecDiff;
                    double totalData = 0;
                    float queueSize = 0;
                    int startIdx = (receivedStatistics_idx + 1) % historyWindow;
                    for (int i = startIdx; i != receivedStatistics_idx; ) {
                        totalData += receivedStatistics[i].elements;
                        queueSize += receivedStatistics[i].queueSize;
                        i = (i + 1) % historyWindow;
                    }
                    runningStats.bitsPerSecond = ((totalData * bitSize) / totalTime);
                    runningStats.elementsPerSecond = (totalData / totalTime);
                    runningStats.averageQueueDepth = (queueSize / historyWindow);
                    runningStats.callsPerSecond = (double(historyWindow - 1) / totalTime);
                    runningStats.timeSinceLastCall = (((double)tv.tv_sec) - front_sec) + (((double)tv.tv_usec - front_usec) / ((double)1e6));
                    unsigned int streamIDsize = activeStreamIDs.size();
                    std::list< std::string >::iterator p = activeStreamIDs.begin();
                    runningStats.streamIDs.length(streamIDsize);
                    for (unsigned int i = 0; i < streamIDsize; i++) {
                        if (p == activeStreamIDs.end()) {
                            break;
                        }
                        runningStats.streamIDs[i] = CORBA::string_dup((*p).c_str());
                        p++;
                    }
                    if ((flush_sec != 0) && (flush_usec != 0)) {
                        double flushTotalTime = (((double)tv.tv_sec) - flush_sec) + (((double)tv.tv_usec - flush_usec) / ((double)1e6));
                        runningStats.keywords.length(1);
                        runningStats.keywords[0].id = CORBA::string_dup("timeSinceLastFlush");
                        runningStats.keywords[0].value <<= CORBA::Double(flushTotalTime);
                    }
                    return runningStats;
                }

            protected:
                bool enabled;
                double bitSize;
                BULKIO::PortStatistics runningStats;
                std::vector<statPoint> receivedStatistics;
                std::list< std::string > activeStreamIDs;
                unsigned long historyWindow;
                long receivedStatistics_idx;
                double flush_sec;
                double flush_usec;
        };
        
        void enableStats(bool enable) {
            stats.setEnabled(enable);
        };


        class dataTransfer
        {
            public:
                dataTransfer(const PortTypes::FloatSequence& data, const BULKIO::PrecisionUTCTime &_T, bool _EOS, const char* _streamID, BULKIO::StreamSRI &_H, bool _sriChanged, bool _inputQueueFlushed)
                {
                    int dataLength = data.length();

#ifdef EXPECTED_VECTOR_IMPL
                    std::_Vector_base<CORBA::Float, _seqVector::seqVectorAllocator<CORBA::Float> >::_Vector_impl *vectorPointer = (std::_Vector_base<CORBA::Float, _seqVector::seqVectorAllocator<CORBA::Float> >::_Vector_impl *) ((void*) & dataBuffer);
                    vectorPointer->_M_start = const_cast<PortTypes::FloatSequence*>(&data)->get_buffer(1);
                    vectorPointer->_M_finish = vectorPointer->_M_start + dataLength;
                    vectorPointer->_M_end_of_storage = vectorPointer->_M_finish;

#else
                    dataBuffer.resize(dataLength);
                    if (dataLength > 0) {
                        memcpy(&dataBuffer[0], &data[0], dataLength * sizeof(data[0]));
                    }

#endif
                    T = _T;
                    EOS = _EOS;
                    streamID = _streamID;
                    SRI = _H;
                    sriChanged = _sriChanged;
                    inputQueueFlushed = _inputQueueFlushed;
                };

#ifdef EXPECTED_VECTOR_IMPL
                std::vector< CORBA::Float, _seqVector::seqVectorAllocator<CORBA::Float> > dataBuffer;
#else
                std::vector<CORBA::Float> dataBuffer;
#endif
                BULKIO::PrecisionUTCTime T;
                bool EOS;
                std::string streamID;
                BULKIO::StreamSRI SRI;
                bool sriChanged;
                bool inputQueueFlushed;
        };

        dataTransfer *getPacket(float timeout);
        void block();
        void unblock();

    protected:
        USRP_UHD_i *parent;
        std::deque<dataTransfer *> workQueue;
        std::map<std::string, std::pair<BULKIO::StreamSRI, bool> > currentHs;
        boost::mutex dataBufferLock;
        boost::mutex sriUpdateLock;
        omni_mutex dataAvailableMutex;
        omni_condition* dataAvailable;
        unsigned long secs, nsecs, timeout_secs, timeout_nsecs;
        bool breakBlock;
        bool blocking;
        queueSemaphore* queueSem;

        // statistics
        linkStatistics stats;

};

 

// ----------------------------------------------------------------------------------------
// FRONTEND_RFInfo_In_i declaration
// ----------------------------------------------------------------------------------------
class FRONTEND_RFInfo_In_i : public POA_FRONTEND::RFInfo, public Port_Provides_base_impl
{
    public:
        FRONTEND_RFInfo_In_i(std::string port_name, USRP_UHD_base *_parent);
        ~FRONTEND_RFInfo_In_i();


        char* rf_flow_id();
        void rf_flow_id(const char* data);

        FRONTEND::RFInfoPkt* rfinfo_pkt();
        void rfinfo_pkt(const FRONTEND::RFInfoPkt& data);


    protected:
        USRP_UHD_i *parent;
        boost::mutex portAccess;

};

#endif
