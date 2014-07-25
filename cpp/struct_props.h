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
#ifndef STRUCTPROPS_H
#define STRUCTPROPS_H

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

*******************************************************************************************/

#include <ossie/CorbaUtils.h>
#include <bulkio/bulkio.h>
typedef bulkio::connection_descriptor_struct connection_descriptor_struct;

#include <frontend/fe_tuner_struct_props.h>

struct target_device_struct {
    target_device_struct ()
    {
        type = "";
        ip_address = "";
        name = "";
        serial = "";
    };

    static std::string getId() {
        return std::string("target_device");
    };

    std::string type;
    std::string ip_address;
    std::string name;
    std::string serial;
};

inline bool operator>>= (const CORBA::Any& a, target_device_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("target::type", props[idx].id)) {
            if (!(props[idx].value >>= s.type)) return false;
        }
        else if (!strcmp("target::ip_address", props[idx].id)) {
            if (!(props[idx].value >>= s.ip_address)) return false;
        }
        else if (!strcmp("target::name", props[idx].id)) {
            if (!(props[idx].value >>= s.name)) return false;
        }
        else if (!strcmp("target::serial", props[idx].id)) {
            if (!(props[idx].value >>= s.serial)) return false;
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const target_device_struct& s) {
    CF::Properties props;
    props.length(4);
    props[0].id = CORBA::string_dup("target::type");
    props[0].value <<= s.type;
    props[1].id = CORBA::string_dup("target::ip_address");
    props[1].value <<= s.ip_address;
    props[2].id = CORBA::string_dup("target::name");
    props[2].value <<= s.name;
    props[3].id = CORBA::string_dup("target::serial");
    props[3].value <<= s.serial;
    a <<= props;
};

inline bool operator== (const target_device_struct& s1, const target_device_struct& s2) {
    if (s1.type!=s2.type)
        return false;
    if (s1.ip_address!=s2.ip_address)
        return false;
    if (s1.name!=s2.name)
        return false;
    if (s1.serial!=s2.serial)
        return false;
    return true;
};

inline bool operator!= (const target_device_struct& s1, const target_device_struct& s2) {
    return !(s1==s2);
};

struct frontend_tuner_status_struct_struct : public frontend::default_frontend_tuner_status_struct_struct {
    frontend_tuner_status_struct_struct () : frontend::default_frontend_tuner_status_struct_struct()
    {
    };

    static std::string getId() {
        return std::string("FRONTEND::tuner_status_struct");
    };

    std::string available_bandwidth;
    std::string available_frequency;
    std::string available_gain;
    std::string available_sample_rate;
    double bandwidth_tolerance;
    bool complex;
    double gain;
    CORBA::Long reference_source;
    double sample_rate_tolerance;
    short tuner_number;
    bool valid;
    std::string stream_id;
};

inline bool operator>>= (const CORBA::Any& a, frontend_tuner_status_struct_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("FRONTEND::tuner_status::allocation_id_csv", props[idx].id)) {
            if (!(props[idx].value >>= s.allocation_id_csv)) return false;
        }
        else if (!strcmp("FRONTEND::tuner_status::available_bandwidth", props[idx].id)) {
            if (!(props[idx].value >>= s.available_bandwidth)) return false;
        }
        else if (!strcmp("FRONTEND::tuner_status::available_frequency", props[idx].id)) {
            if (!(props[idx].value >>= s.available_frequency)) return false;
        }
        else if (!strcmp("FRONTEND::tuner_status::available_gain", props[idx].id)) {
            if (!(props[idx].value >>= s.available_gain)) return false;
        }
        else if (!strcmp("FRONTEND::tuner_status::available_sample_rate", props[idx].id)) {
            if (!(props[idx].value >>= s.available_sample_rate)) return false;
        }
        else if (!strcmp("FRONTEND::tuner_status::bandwidth", props[idx].id)) {
            if (!(props[idx].value >>= s.bandwidth)) return false;
        }
        else if (!strcmp("FRONTEND::tuner_status::bandwidth_tolerance", props[idx].id)) {
            if (!(props[idx].value >>= s.bandwidth_tolerance)) return false;
        }
        else if (!strcmp("FRONTEND::tuner_status::center_frequency", props[idx].id)) {
            if (!(props[idx].value >>= s.center_frequency)) return false;
        }
        else if (!strcmp("FRONTEND::tuner_status::complex", props[idx].id)) {
            if (!(props[idx].value >>= s.complex)) return false;
        }
        else if (!strcmp("FRONTEND::tuner_status::enabled", props[idx].id)) {
            if (!(props[idx].value >>= s.enabled)) return false;
        }
        else if (!strcmp("FRONTEND::tuner_status::gain", props[idx].id)) {
            if (!(props[idx].value >>= s.gain)) return false;
        }
        else if (!strcmp("FRONTEND::tuner_status::group_id", props[idx].id)) {
            if (!(props[idx].value >>= s.group_id)) return false;
        }
        else if (!strcmp("FRONTEND::tuner_status::reference_source", props[idx].id)) {
            if (!(props[idx].value >>= s.reference_source)) return false;
        }
        else if (!strcmp("FRONTEND::tuner_status::rf_flow_id", props[idx].id)) {
            if (!(props[idx].value >>= s.rf_flow_id)) return false;
        }
        else if (!strcmp("FRONTEND::tuner_status::sample_rate", props[idx].id)) {
            if (!(props[idx].value >>= s.sample_rate)) return false;
        }
        else if (!strcmp("FRONTEND::tuner_status::sample_rate_tolerance", props[idx].id)) {
            if (!(props[idx].value >>= s.sample_rate_tolerance)) return false;
        }
        else if (!strcmp("FRONTEND::tuner_status::tuner_number", props[idx].id)) {
            if (!(props[idx].value >>= s.tuner_number)) return false;
        }
        else if (!strcmp("FRONTEND::tuner_status::tuner_type", props[idx].id)) {
            if (!(props[idx].value >>= s.tuner_type)) return false;
        }
        else if (!strcmp("FRONTEND::tuner_status::valid", props[idx].id)) {
            if (!(props[idx].value >>= s.valid)) return false;
        }
        else if (!strcmp("FRONTEND::tuner_status::stream_id", props[idx].id)) {
            if (!(props[idx].value >>= s.stream_id)) return false;
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const frontend_tuner_status_struct_struct& s) {
    CF::Properties props;
    props.length(20);
    props[0].id = CORBA::string_dup("FRONTEND::tuner_status::allocation_id_csv");
    props[0].value <<= s.allocation_id_csv;
    props[1].id = CORBA::string_dup("FRONTEND::tuner_status::available_bandwidth");
    props[1].value <<= s.available_bandwidth;
    props[2].id = CORBA::string_dup("FRONTEND::tuner_status::available_frequency");
    props[2].value <<= s.available_frequency;
    props[3].id = CORBA::string_dup("FRONTEND::tuner_status::available_gain");
    props[3].value <<= s.available_gain;
    props[4].id = CORBA::string_dup("FRONTEND::tuner_status::available_sample_rate");
    props[4].value <<= s.available_sample_rate;
    props[5].id = CORBA::string_dup("FRONTEND::tuner_status::bandwidth");
    props[5].value <<= s.bandwidth;
    props[6].id = CORBA::string_dup("FRONTEND::tuner_status::bandwidth_tolerance");
    props[6].value <<= s.bandwidth_tolerance;
    props[7].id = CORBA::string_dup("FRONTEND::tuner_status::center_frequency");
    props[7].value <<= s.center_frequency;
    props[8].id = CORBA::string_dup("FRONTEND::tuner_status::complex");
    props[8].value <<= s.complex;
    props[9].id = CORBA::string_dup("FRONTEND::tuner_status::enabled");
    props[9].value <<= s.enabled;
    props[10].id = CORBA::string_dup("FRONTEND::tuner_status::gain");
    props[10].value <<= s.gain;
    props[11].id = CORBA::string_dup("FRONTEND::tuner_status::group_id");
    props[11].value <<= s.group_id;
    props[12].id = CORBA::string_dup("FRONTEND::tuner_status::reference_source");
    props[12].value <<= s.reference_source;
    props[13].id = CORBA::string_dup("FRONTEND::tuner_status::rf_flow_id");
    props[13].value <<= s.rf_flow_id;
    props[14].id = CORBA::string_dup("FRONTEND::tuner_status::sample_rate");
    props[14].value <<= s.sample_rate;
    props[15].id = CORBA::string_dup("FRONTEND::tuner_status::sample_rate_tolerance");
    props[15].value <<= s.sample_rate_tolerance;
    props[16].id = CORBA::string_dup("FRONTEND::tuner_status::tuner_number");
    props[16].value <<= s.tuner_number;
    props[17].id = CORBA::string_dup("FRONTEND::tuner_status::tuner_type");
    props[17].value <<= s.tuner_type;
    props[18].id = CORBA::string_dup("FRONTEND::tuner_status::valid");
    props[18].value <<= s.valid;
    props[19].id = CORBA::string_dup("FRONTEND::tuner_status::stream_id");
    props[19].value <<= s.stream_id;
    a <<= props;
};

inline bool operator== (const frontend_tuner_status_struct_struct& s1, const frontend_tuner_status_struct_struct& s2) {
    if (s1.allocation_id_csv!=s2.allocation_id_csv)
        return false;
    if (s1.available_bandwidth!=s2.available_bandwidth)
        return false;
    if (s1.available_frequency!=s2.available_frequency)
        return false;
    if (s1.available_gain!=s2.available_gain)
        return false;
    if (s1.available_sample_rate!=s2.available_sample_rate)
        return false;
    if (s1.bandwidth!=s2.bandwidth)
        return false;
    if (s1.bandwidth_tolerance!=s2.bandwidth_tolerance)
        return false;
    if (s1.center_frequency!=s2.center_frequency)
        return false;
    if (s1.complex!=s2.complex)
        return false;
    if (s1.enabled!=s2.enabled)
        return false;
    if (s1.gain!=s2.gain)
        return false;
    if (s1.group_id!=s2.group_id)
        return false;
    if (s1.reference_source!=s2.reference_source)
        return false;
    if (s1.rf_flow_id!=s2.rf_flow_id)
        return false;
    if (s1.sample_rate!=s2.sample_rate)
        return false;
    if (s1.sample_rate_tolerance!=s2.sample_rate_tolerance)
        return false;
    if (s1.tuner_number!=s2.tuner_number)
        return false;
    if (s1.tuner_type!=s2.tuner_type)
        return false;
    if (s1.valid!=s2.valid)
        return false;
    if (s1.stream_id!=s2.stream_id)
        return false;
    return true;
};

inline bool operator!= (const frontend_tuner_status_struct_struct& s1, const frontend_tuner_status_struct_struct& s2) {
    return !(s1==s2);
};

struct usrp_device_struct {
    usrp_device_struct ()
    {
    };

    static std::string getId() {
        return std::string("available_devices::usrp_device");
    };

    std::string type;
    std::string ip_address;
    std::string name;
    std::string serial;
};

inline bool operator>>= (const CORBA::Any& a, usrp_device_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("available_devices::type", props[idx].id)) {
            if (!(props[idx].value >>= s.type)) return false;
        }
        else if (!strcmp("available_devices::ip_address", props[idx].id)) {
            if (!(props[idx].value >>= s.ip_address)) return false;
        }
        else if (!strcmp("available_devices::name", props[idx].id)) {
            if (!(props[idx].value >>= s.name)) return false;
        }
        else if (!strcmp("available_devices::serial", props[idx].id)) {
            if (!(props[idx].value >>= s.serial)) return false;
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const usrp_device_struct& s) {
    CF::Properties props;
    props.length(4);
    props[0].id = CORBA::string_dup("available_devices::type");
    props[0].value <<= s.type;
    props[1].id = CORBA::string_dup("available_devices::ip_address");
    props[1].value <<= s.ip_address;
    props[2].id = CORBA::string_dup("available_devices::name");
    props[2].value <<= s.name;
    props[3].id = CORBA::string_dup("available_devices::serial");
    props[3].value <<= s.serial;
    a <<= props;
};

inline bool operator== (const usrp_device_struct& s1, const usrp_device_struct& s2) {
    if (s1.type!=s2.type)
        return false;
    if (s1.ip_address!=s2.ip_address)
        return false;
    if (s1.name!=s2.name)
        return false;
    if (s1.serial!=s2.serial)
        return false;
    return true;
};

inline bool operator!= (const usrp_device_struct& s1, const usrp_device_struct& s2) {
    return !(s1==s2);
};

struct usrp_channel_struct {
    usrp_channel_struct ()
    {
    };

    static std::string getId() {
        return std::string("channels::usrp_channel");
    };

    std::string ch_name;
    std::string tuner_type;
    short chan_num;
    std::string antenna;
    double bandwidth_current;
    double bandwidth_min;
    double bandwidth_max;
    double rate_current;
    double rate_min;
    double rate_max;
    double freq_current;
    double freq_min;
    double freq_max;
    double gain_current;
    double gain_min;
    double gain_max;
    double clock_min;
    double clock_max;
};

inline bool operator>>= (const CORBA::Any& a, usrp_channel_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("device_channels::ch_name", props[idx].id)) {
            if (!(props[idx].value >>= s.ch_name)) return false;
        }
        else if (!strcmp("device_channels::tuner_type", props[idx].id)) {
            if (!(props[idx].value >>= s.tuner_type)) return false;
        }
        else if (!strcmp("device_channels::chan_num", props[idx].id)) {
            if (!(props[idx].value >>= s.chan_num)) return false;
        }
        else if (!strcmp("device_channels::antenna", props[idx].id)) {
            if (!(props[idx].value >>= s.antenna)) return false;
        }
        else if (!strcmp("device_channels::bandwidth_current", props[idx].id)) {
            if (!(props[idx].value >>= s.bandwidth_current)) return false;
        }
        else if (!strcmp("device_channels::bandwidth_min", props[idx].id)) {
            if (!(props[idx].value >>= s.bandwidth_min)) return false;
        }
        else if (!strcmp("device_channels::bandwidth_max", props[idx].id)) {
            if (!(props[idx].value >>= s.bandwidth_max)) return false;
        }
        else if (!strcmp("device_channels::rate_current", props[idx].id)) {
            if (!(props[idx].value >>= s.rate_current)) return false;
        }
        else if (!strcmp("device_channels::rate_min", props[idx].id)) {
            if (!(props[idx].value >>= s.rate_min)) return false;
        }
        else if (!strcmp("device_channels::rate_max", props[idx].id)) {
            if (!(props[idx].value >>= s.rate_max)) return false;
        }
        else if (!strcmp("device_channels::freq_current", props[idx].id)) {
            if (!(props[idx].value >>= s.freq_current)) return false;
        }
        else if (!strcmp("device_channels::freq_min", props[idx].id)) {
            if (!(props[idx].value >>= s.freq_min)) return false;
        }
        else if (!strcmp("device_channels::freq_max", props[idx].id)) {
            if (!(props[idx].value >>= s.freq_max)) return false;
        }
        else if (!strcmp("device_channels::gain_current", props[idx].id)) {
            if (!(props[idx].value >>= s.gain_current)) return false;
        }
        else if (!strcmp("device_channels::gain_min", props[idx].id)) {
            if (!(props[idx].value >>= s.gain_min)) return false;
        }
        else if (!strcmp("device_channels::gain_max", props[idx].id)) {
            if (!(props[idx].value >>= s.gain_max)) return false;
        }
        else if (!strcmp("device_channels::clock_min", props[idx].id)) {
            if (!(props[idx].value >>= s.clock_min)) return false;
        }
        else if (!strcmp("device_channels::clock_max", props[idx].id)) {
            if (!(props[idx].value >>= s.clock_max)) return false;
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const usrp_channel_struct& s) {
    CF::Properties props;
    props.length(18);
    props[0].id = CORBA::string_dup("device_channels::ch_name");
    props[0].value <<= s.ch_name;
    props[1].id = CORBA::string_dup("device_channels::tuner_type");
    props[1].value <<= s.tuner_type;
    props[2].id = CORBA::string_dup("device_channels::chan_num");
    props[2].value <<= s.chan_num;
    props[3].id = CORBA::string_dup("device_channels::antenna");
    props[3].value <<= s.antenna;
    props[4].id = CORBA::string_dup("device_channels::bandwidth_current");
    props[4].value <<= s.bandwidth_current;
    props[5].id = CORBA::string_dup("device_channels::bandwidth_min");
    props[5].value <<= s.bandwidth_min;
    props[6].id = CORBA::string_dup("device_channels::bandwidth_max");
    props[6].value <<= s.bandwidth_max;
    props[7].id = CORBA::string_dup("device_channels::rate_current");
    props[7].value <<= s.rate_current;
    props[8].id = CORBA::string_dup("device_channels::rate_min");
    props[8].value <<= s.rate_min;
    props[9].id = CORBA::string_dup("device_channels::rate_max");
    props[9].value <<= s.rate_max;
    props[10].id = CORBA::string_dup("device_channels::freq_current");
    props[10].value <<= s.freq_current;
    props[11].id = CORBA::string_dup("device_channels::freq_min");
    props[11].value <<= s.freq_min;
    props[12].id = CORBA::string_dup("device_channels::freq_max");
    props[12].value <<= s.freq_max;
    props[13].id = CORBA::string_dup("device_channels::gain_current");
    props[13].value <<= s.gain_current;
    props[14].id = CORBA::string_dup("device_channels::gain_min");
    props[14].value <<= s.gain_min;
    props[15].id = CORBA::string_dup("device_channels::gain_max");
    props[15].value <<= s.gain_max;
    props[16].id = CORBA::string_dup("device_channels::clock_min");
    props[16].value <<= s.clock_min;
    props[17].id = CORBA::string_dup("device_channels::clock_max");
    props[17].value <<= s.clock_max;
    a <<= props;
};

inline bool operator== (const usrp_channel_struct& s1, const usrp_channel_struct& s2) {
    if (s1.ch_name!=s2.ch_name)
        return false;
    if (s1.tuner_type!=s2.tuner_type)
        return false;
    if (s1.chan_num!=s2.chan_num)
        return false;
    if (s1.antenna!=s2.antenna)
        return false;
    if (s1.bandwidth_current!=s2.bandwidth_current)
        return false;
    if (s1.bandwidth_min!=s2.bandwidth_min)
        return false;
    if (s1.bandwidth_max!=s2.bandwidth_max)
        return false;
    if (s1.rate_current!=s2.rate_current)
        return false;
    if (s1.rate_min!=s2.rate_min)
        return false;
    if (s1.rate_max!=s2.rate_max)
        return false;
    if (s1.freq_current!=s2.freq_current)
        return false;
    if (s1.freq_min!=s2.freq_min)
        return false;
    if (s1.freq_max!=s2.freq_max)
        return false;
    if (s1.gain_current!=s2.gain_current)
        return false;
    if (s1.gain_min!=s2.gain_min)
        return false;
    if (s1.gain_max!=s2.gain_max)
        return false;
    if (s1.clock_min!=s2.clock_min)
        return false;
    if (s1.clock_max!=s2.clock_max)
        return false;
    return true;
};

inline bool operator!= (const usrp_channel_struct& s1, const usrp_channel_struct& s2) {
    return !(s1==s2);
};

struct usrp_motherboard_struct {
    usrp_motherboard_struct ()
    {
    };

    static std::string getId() {
        return std::string("motherboards::usrp_motherboard");
    };

    std::string mb_name;
    std::string mb_ip;
};

inline bool operator>>= (const CORBA::Any& a, usrp_motherboard_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("device_motherboards::mb_name", props[idx].id)) {
            if (!(props[idx].value >>= s.mb_name)) return false;
        }
        else if (!strcmp("device_motherboards::mb_ip", props[idx].id)) {
            if (!(props[idx].value >>= s.mb_ip)) return false;
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const usrp_motherboard_struct& s) {
    CF::Properties props;
    props.length(2);
    props[0].id = CORBA::string_dup("device_motherboards::mb_name");
    props[0].value <<= s.mb_name;
    props[1].id = CORBA::string_dup("device_motherboards::mb_ip");
    props[1].value <<= s.mb_ip;
    a <<= props;
};

inline bool operator== (const usrp_motherboard_struct& s1, const usrp_motherboard_struct& s2) {
    if (s1.mb_name!=s2.mb_name)
        return false;
    if (s1.mb_ip!=s2.mb_ip)
        return false;
    return true;
};

inline bool operator!= (const usrp_motherboard_struct& s1, const usrp_motherboard_struct& s2) {
    return !(s1==s2);
};

#endif // STRUCTPROPS_H
