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

#ifndef BULKIOTOSDDSPROCESSOR_H_
#define BULKIOTOSDDSPROCESSOR_H_

#include <bulkio/bulkio.h>
#include <boost/thread.hpp>
#include <endian.h>
#include <ossie/debug.h>
#include <Resource_impl.h>
#include "sddspacket.h"
#include "socketUtils/multicast.h"
#include "socketUtils/unicast.h"
#include <queue>

#include "BlockingReadFifo.h"
#include "CustomStructs.h"
#include "BoundedBuffer.h"

#define SDDS_DATA_SIZE 1024
#define SDDS_HEADER_SIZE 56
#define SSD_LENGTH 2
#define AAD_LENGTH 20
// decimal 43981 is 0xABCD in hexadecimal
#define DATA_REF_STR_LITTLE 43981
// decimal 52651 is 0xCDAB in hexadecimal
#define DATA_REF_STR_BIG 52651

#ifdef BYTE_ORDER
#if BYTE_ORDER==BIG_ENDIAN
#define DATA_REF_STR_NATIVE DATA_REF_STR_BIG
#else
#define DATA_REF_STR_NATIVE DATA_REF_STR_LITTLE
#endif
#endif

template <class DATA_TYPE>
class SddsProcessor {
    ENABLE_LOGGING

    typedef inputMetadata<short> METADATA_TYPE;

public:
    SddsProcessor(bulkio::OutSDDSPort * dataSddsOut, size_t bufSz=SDDS_DATA_SIZE/sizeof(DATA_TYPE),
    		size_t bufCnt=20000);
    ~SddsProcessor();

    bool isActive();
    std::string getStreamId();
    void join();
    void shutdown();
    void run();
    void pushSri(BULKIO::dataSDDS::_ptr_type sdds_input_port);
    bool setStream(std::string streamID, std::string iface, std::string ip, long port, uint16_t vlan,
            std::string attach_user_id, long ttv=-1, long endiance=-1, bool sri_has_priority=false);
    void removeStream(std::string streamID);
    void dataIn(const std::vector<DATA_TYPE>& data, const BULKIO::PrecisionUTCTime& T, bool EOS, const BULKIO::StreamSRI& sri);
    void dataIn(const std::vector<DATA_TYPE>& data, const BULKIO::PrecisionUTCTime& T, bool EOS);

private:
    void pushSri();
    void callDetach();
    void callAttach();
    void callAttach(BULKIO::dataSDDS::_ptr_type sdds_input_port);
    void callAttach(const BULKIO::StreamSRI& sri);
    void callAttach(BULKIO::dataSDDS::_ptr_type sdds_input_port, const BULKIO::StreamSRI& sri);
    int setupSocket(std::string iface, std::string ip, long port, uint16_t vlan=0);
    void _run();
    size_t getDataPointer();
    int sendPacket(char* dataBlock, size_t num_bytes);
    void initializeSDDSHeader();
    void setSddsHeaderFromSri();
    void setSddsTimestamp();
    double getSecondsSinceStartOfYear(double twsec);

    bulkio::OutSDDSPort *m_sdds_out_port;
    bool m_first_run, m_shutdown, m_running, m_active_stream, m_attached;
    boost::thread *m_processorThread;

    std::string m_user_id, m_streamID;
    long m_ttv_override;
    bool m_byte_swap;
    int32_t m_data_ref_str;
    bool m_give_sri_priority;
    connection_t m_connection;
    uint16_t m_vlan;

    METADATA_TYPE m_metadata;
    SDDSpacket m_sdds_template;
    char m_zero_pad_buffer[SDDS_DATA_SIZE]; // This buffer is only used if we do a non full read off of the stream API.
    iovec m_msg_iov[3];
    msghdr m_pkt_template;
    uint16_t m_seq;

    BoundedBuffer<DATA_TYPE> m_input_data_q;
    BlockingReadFifo<METADATA_TYPE> m_input_metadata_q;
    METADATA_TYPE m_input_metadata;
    boost::mutex m_input_mutex;

    time_t m_year_start_s;
    time_t m_year_end_s;

    template <typename CORBAXX>
        bool addModifyKeyword(BULKIO::StreamSRI *sri, CORBA::String_member id, CORBAXX myValue, bool addOnly = false) {
            CORBA::Any value;
            value <<= (CORBAXX) myValue;
            unsigned long keySize = sri->keywords.length();
            if (!addOnly) {
                for (unsigned int i = 0; i < keySize; i++) {
                    if (!strcmp(sri->keywords[i].id, id)) {
                        sri->keywords[i].value = value;
                        return true;
                    }
                }
            }
            sri->keywords.length(keySize + 1);
            if (sri->keywords.length() != keySize + 1)
                return false;
            sri->keywords[keySize].id = CORBA::string_dup(id);
            sri->keywords[keySize].value = value;
            return true;
        }
};

#endif /* BULKIOTOSDDSPROCESSOR_H_ */
