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
#ifndef STRUCTPROPS_H
#define STRUCTPROPS_H

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

*******************************************************************************************/

#include <ossie/CorbaUtils.h>
#include <CF/cf.h>
#include <ossie/PropertyMap.h>
#include <bulkio/bulkio.h>
typedef bulkio::connection_descriptor_struct connection_descriptor_struct;

#include <frontend/fe_tuner_struct_props.h>

struct sdds_settings_struct {
    sdds_settings_struct ()
    {
        attach_user_id = "anonymous";
        downstream_give_sri_priority = false;
        sdds_endian_representation = 1;
        ttv_override = -1;
        buffer_size = 20;
    };

    static std::string getId() {
        return std::string("sdds_settings");
    };

    std::string attach_user_id;
    bool downstream_give_sri_priority;
    CORBA::Long sdds_endian_representation;
    CORBA::Long ttv_override;
    CORBA::ULong buffer_size;
};

inline bool operator>>= (const CORBA::Any& a, sdds_settings_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("sdds_settings::attach_user_id")) {
        if (!(props["sdds_settings::attach_user_id"] >>= s.attach_user_id)) return false;
    }
    if (props.contains("sdds_settings::downstream_should_use_sri")) {
        if (!(props["sdds_settings::downstream_should_use_sri"] >>= s.downstream_give_sri_priority)) return false;
    }
    if (props.contains("sdds_settings::sdds_endian_representation")) {
        if (!(props["sdds_settings::sdds_endian_representation"] >>= s.sdds_endian_representation)) return false;
    }
    if (props.contains("sdds_settings::ttv_override")) {
        if (!(props["sdds_settings::ttv_override"] >>= s.ttv_override)) return false;
    }
    if (props.contains("sdds_settings::buffer_size")) {
        if (!(props["sdds_settings::buffer_size"] >>= s.buffer_size)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const sdds_settings_struct& s) {
    redhawk::PropertyMap props;
 
    props["sdds_settings::attach_user_id"] = s.attach_user_id;
 
    props["sdds_settings::downstream_should_use_sri"] = s.downstream_give_sri_priority;
 
    props["sdds_settings::sdds_endian_representation"] = s.sdds_endian_representation;
 
    props["sdds_settings::ttv_override"] = s.ttv_override;
 
    props["sdds_settings::buffer_size"] = s.buffer_size;
    a <<= props;
}

inline bool operator== (const sdds_settings_struct& s1, const sdds_settings_struct& s2) {
    if (s1.attach_user_id!=s2.attach_user_id)
        return false;
    if (s1.downstream_give_sri_priority!=s2.downstream_give_sri_priority)
        return false;
    if (s1.sdds_endian_representation!=s2.sdds_endian_representation)
        return false;
    if (s1.ttv_override!=s2.ttv_override)
        return false;
    if (s1.buffer_size!=s2.buffer_size)
        return false;
    return true;
}

inline bool operator!= (const sdds_settings_struct& s1, const sdds_settings_struct& s2) {
    return !(s1==s2);
}

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
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("target::type")) {
        if (!(props["target::type"] >>= s.type)) return false;
    }
    if (props.contains("target::ip_address")) {
        if (!(props["target::ip_address"] >>= s.ip_address)) return false;
    }
    if (props.contains("target::name")) {
        if (!(props["target::name"] >>= s.name)) return false;
    }
    if (props.contains("target::serial")) {
        if (!(props["target::serial"] >>= s.serial)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const target_device_struct& s) {
    redhawk::PropertyMap props;
 
    props["target::type"] = s.type;
 
    props["target::ip_address"] = s.ip_address;
 
    props["target::name"] = s.name;
 
    props["target::serial"] = s.serial;
    a <<= props;
}

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
}

inline bool operator!= (const target_device_struct& s1, const target_device_struct& s2) {
    return !(s1==s2);
}

struct device_antenna_mapping_struct {
    device_antenna_mapping_struct ()
    {
        RFInfo_in = "";
        RFInfo_in2 = "";
        RFInfo_in3 = "";
        RFInfo_in4 = "";
        RFInfoTX_out = "";
        RFInfoTX_out2 = "";
    };

    static std::string getId() {
        return std::string("device_antenna_mapping");
    };

    std::string RFInfo_in;
    std::string RFInfo_in2;
    std::string RFInfo_in3;
    std::string RFInfo_in4;
    std::string RFInfoTX_out;
    std::string RFInfoTX_out2;
};

inline bool operator>>= (const CORBA::Any& a, device_antenna_mapping_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("device_antenna_mapping::RFInfo_in")) {
        if (!(props["device_antenna_mapping::RFInfo_in"] >>= s.RFInfo_in)) return false;
    }
    if (props.contains("device_antenna_mapping::RFInfo_in2")) {
        if (!(props["device_antenna_mapping::RFInfo_in2"] >>= s.RFInfo_in2)) return false;
    }
    if (props.contains("device_antenna_mapping::RFInfo_in3")) {
        if (!(props["device_antenna_mapping::RFInfo_in3"] >>= s.RFInfo_in3)) return false;
    }
    if (props.contains("device_antenna_mapping::RFInfo_in4")) {
        if (!(props["device_antenna_mapping::RFInfo_in4"] >>= s.RFInfo_in4)) return false;
    }
    if (props.contains("device_antenna_mapping::RFInfoTX_out")) {
        if (!(props["device_antenna_mapping::RFInfoTX_out"] >>= s.RFInfoTX_out)) return false;
    }
    if (props.contains("device_antenna_mapping::RFInfoTX_out2")) {
        if (!(props["device_antenna_mapping::RFInfoTX_out2"] >>= s.RFInfoTX_out2)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const device_antenna_mapping_struct& s) {
    redhawk::PropertyMap props;
 
    props["device_antenna_mapping::RFInfo_in"] = s.RFInfo_in;
 
    props["device_antenna_mapping::RFInfo_in2"] = s.RFInfo_in2;
 
    props["device_antenna_mapping::RFInfo_in3"] = s.RFInfo_in3;
 
    props["device_antenna_mapping::RFInfo_in4"] = s.RFInfo_in4;
 
    props["device_antenna_mapping::RFInfoTX_out"] = s.RFInfoTX_out;
 
    props["device_antenna_mapping::RFInfoTX_out2"] = s.RFInfoTX_out2;
    a <<= props;
}

inline bool operator== (const device_antenna_mapping_struct& s1, const device_antenna_mapping_struct& s2) {
    if (s1.RFInfo_in!=s2.RFInfo_in)
        return false;
    if (s1.RFInfo_in2!=s2.RFInfo_in2)
        return false;
    if (s1.RFInfo_in3!=s2.RFInfo_in3)
        return false;
    if (s1.RFInfo_in4!=s2.RFInfo_in4)
        return false;
    if (s1.RFInfoTX_out!=s2.RFInfoTX_out)
        return false;
    if (s1.RFInfoTX_out2!=s2.RFInfoTX_out2)
        return false;
    return true;
}

inline bool operator!= (const device_antenna_mapping_struct& s1, const device_antenna_mapping_struct& s2) {
    return !(s1==s2);
}

struct configure_tuner_antenna_struct {
    configure_tuner_antenna_struct ()
    {
        tuner_index = 0;
        antenna = "";
    };

    static std::string getId() {
        return std::string("configure_tuner_antenna");
    };

    CORBA::ULong tuner_index;
    std::string antenna;
};

inline bool operator>>= (const CORBA::Any& a, configure_tuner_antenna_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("configure_tuner_antenna::tuner_id")) {
        if (!(props["configure_tuner_antenna::tuner_id"] >>= s.tuner_index)) return false;
    }
    if (props.contains("configure_tuner_antenna::antenna")) {
        if (!(props["configure_tuner_antenna::antenna"] >>= s.antenna)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const configure_tuner_antenna_struct& s) {
    redhawk::PropertyMap props;
 
    props["configure_tuner_antenna::tuner_id"] = s.tuner_index;
 
    props["configure_tuner_antenna::antenna"] = s.antenna;
    a <<= props;
}

inline bool operator== (const configure_tuner_antenna_struct& s1, const configure_tuner_antenna_struct& s2) {
    if (s1.tuner_index!=s2.tuner_index)
        return false;
    if (s1.antenna!=s2.antenna)
        return false;
    return true;
}

inline bool operator!= (const configure_tuner_antenna_struct& s1, const configure_tuner_antenna_struct& s2) {
    return !(s1==s2);
}

struct frontend_tuner_status_struct_struct : public frontend::default_frontend_tuner_status_struct_struct {
    frontend_tuner_status_struct_struct () : frontend::default_frontend_tuner_status_struct_struct()
    {
    };

    static std::string getId() {
        return std::string("FRONTEND::tuner_status_struct");
    };

    std::string antenna;
    std::string available_bandwidth;
    std::string available_frequency;
    std::string available_gain;
    std::string available_sample_rate;
    double bandwidth_tolerance;
    bool complex;
    double gain;
    std::string output_format;
    std::string output_multicast;
    CORBA::Long output_port;
    CORBA::Long output_vlan;
    CORBA::Long reference_source;
    double sample_rate_tolerance;
    std::string stream_id;
    CORBA::ULong tuner_index;
    short tuner_number;
    bool valid;
    std::vector<std::string> available_antennas;
};

inline bool operator>>= (const CORBA::Any& a, frontend_tuner_status_struct_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("FRONTEND::tuner_status::allocation_id_csv")) {
        if (!(props["FRONTEND::tuner_status::allocation_id_csv"] >>= s.allocation_id_csv)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::antenna")) {
        if (!(props["FRONTEND::tuner_status::antenna"] >>= s.antenna)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::available_bandwidth")) {
        if (!(props["FRONTEND::tuner_status::available_bandwidth"] >>= s.available_bandwidth)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::available_frequency")) {
        if (!(props["FRONTEND::tuner_status::available_frequency"] >>= s.available_frequency)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::available_gain")) {
        if (!(props["FRONTEND::tuner_status::available_gain"] >>= s.available_gain)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::available_sample_rate")) {
        if (!(props["FRONTEND::tuner_status::available_sample_rate"] >>= s.available_sample_rate)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::bandwidth")) {
        if (!(props["FRONTEND::tuner_status::bandwidth"] >>= s.bandwidth)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::bandwidth_tolerance")) {
        if (!(props["FRONTEND::tuner_status::bandwidth_tolerance"] >>= s.bandwidth_tolerance)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::center_frequency")) {
        if (!(props["FRONTEND::tuner_status::center_frequency"] >>= s.center_frequency)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::complex")) {
        if (!(props["FRONTEND::tuner_status::complex"] >>= s.complex)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::enabled")) {
        if (!(props["FRONTEND::tuner_status::enabled"] >>= s.enabled)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::gain")) {
        if (!(props["FRONTEND::tuner_status::gain"] >>= s.gain)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::group_id")) {
        if (!(props["FRONTEND::tuner_status::group_id"] >>= s.group_id)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::output_format")) {
        if (!(props["FRONTEND::tuner_status::output_format"] >>= s.output_format)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::output_multicast")) {
        if (!(props["FRONTEND::tuner_status::output_multicast"] >>= s.output_multicast)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::output_port")) {
        if (!(props["FRONTEND::tuner_status::output_port"] >>= s.output_port)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::output_vlan")) {
        if (!(props["FRONTEND::tuner_status::output_vlan"] >>= s.output_vlan)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::reference_source")) {
        if (!(props["FRONTEND::tuner_status::reference_source"] >>= s.reference_source)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::rf_flow_id")) {
        if (!(props["FRONTEND::tuner_status::rf_flow_id"] >>= s.rf_flow_id)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::sample_rate")) {
        if (!(props["FRONTEND::tuner_status::sample_rate"] >>= s.sample_rate)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::sample_rate_tolerance")) {
        if (!(props["FRONTEND::tuner_status::sample_rate_tolerance"] >>= s.sample_rate_tolerance)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::stream_id")) {
        if (!(props["FRONTEND::tuner_status::stream_id"] >>= s.stream_id)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::tuner_index")) {
        if (!(props["FRONTEND::tuner_status::tuner_index"] >>= s.tuner_index)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::tuner_number")) {
        if (!(props["FRONTEND::tuner_status::tuner_number"] >>= s.tuner_number)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::tuner_type")) {
        if (!(props["FRONTEND::tuner_status::tuner_type"] >>= s.tuner_type)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::valid")) {
        if (!(props["FRONTEND::tuner_status::valid"] >>= s.valid)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::available_antennas")) {
        if (!(props["FRONTEND::tuner_status::available_antennas"] >>= s.available_antennas)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const frontend_tuner_status_struct_struct& s) {
    redhawk::PropertyMap props;
 
    props["FRONTEND::tuner_status::allocation_id_csv"] = s.allocation_id_csv;
 
    props["FRONTEND::tuner_status::antenna"] = s.antenna;
 
    props["FRONTEND::tuner_status::available_bandwidth"] = s.available_bandwidth;
 
    props["FRONTEND::tuner_status::available_frequency"] = s.available_frequency;
 
    props["FRONTEND::tuner_status::available_gain"] = s.available_gain;
 
    props["FRONTEND::tuner_status::available_sample_rate"] = s.available_sample_rate;
 
    props["FRONTEND::tuner_status::bandwidth"] = s.bandwidth;
 
    props["FRONTEND::tuner_status::bandwidth_tolerance"] = s.bandwidth_tolerance;
 
    props["FRONTEND::tuner_status::center_frequency"] = s.center_frequency;
 
    props["FRONTEND::tuner_status::complex"] = s.complex;
 
    props["FRONTEND::tuner_status::enabled"] = s.enabled;
 
    props["FRONTEND::tuner_status::gain"] = s.gain;
 
    props["FRONTEND::tuner_status::group_id"] = s.group_id;
 
    props["FRONTEND::tuner_status::output_format"] = s.output_format;
 
    props["FRONTEND::tuner_status::output_multicast"] = s.output_multicast;
 
    props["FRONTEND::tuner_status::output_port"] = s.output_port;
 
    props["FRONTEND::tuner_status::output_vlan"] = s.output_vlan;
 
    props["FRONTEND::tuner_status::reference_source"] = s.reference_source;
 
    props["FRONTEND::tuner_status::rf_flow_id"] = s.rf_flow_id;
 
    props["FRONTEND::tuner_status::sample_rate"] = s.sample_rate;
 
    props["FRONTEND::tuner_status::sample_rate_tolerance"] = s.sample_rate_tolerance;
 
    props["FRONTEND::tuner_status::stream_id"] = s.stream_id;
 
    props["FRONTEND::tuner_status::tuner_index"] = s.tuner_index;
 
    props["FRONTEND::tuner_status::tuner_number"] = s.tuner_number;
 
    props["FRONTEND::tuner_status::tuner_type"] = s.tuner_type;
 
    props["FRONTEND::tuner_status::valid"] = s.valid;
 
    props["FRONTEND::tuner_status::available_antennas"] = s.available_antennas;
    a <<= props;
}

inline bool operator== (const frontend_tuner_status_struct_struct& s1, const frontend_tuner_status_struct_struct& s2) {
    if (s1.allocation_id_csv!=s2.allocation_id_csv)
        return false;
    if (s1.antenna!=s2.antenna)
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
    if (s1.output_format!=s2.output_format)
        return false;
    if (s1.output_multicast!=s2.output_multicast)
        return false;
    if (s1.output_port!=s2.output_port)
        return false;
    if (s1.output_vlan!=s2.output_vlan)
        return false;
    if (s1.reference_source!=s2.reference_source)
        return false;
    if (s1.rf_flow_id!=s2.rf_flow_id)
        return false;
    if (s1.sample_rate!=s2.sample_rate)
        return false;
    if (s1.sample_rate_tolerance!=s2.sample_rate_tolerance)
        return false;
    if (s1.stream_id!=s2.stream_id)
        return false;
    if (s1.tuner_index!=s2.tuner_index)
        return false;
    if (s1.tuner_number!=s2.tuner_number)
        return false;
    if (s1.tuner_type!=s2.tuner_type)
        return false;
    if (s1.valid!=s2.valid)
        return false;
    if (s1.available_antennas!=s2.available_antennas)
        return false;
    return true;
}

inline bool operator!= (const frontend_tuner_status_struct_struct& s1, const frontend_tuner_status_struct_struct& s2) {
    return !(s1==s2);
}

struct sdds_network_settings_struct_struct {
    sdds_network_settings_struct_struct ()
    {
        interface = "eth0";
        ip_address = "127.0.0.1";
        port = 29495;
        vlan = 0;
    };

    static std::string getId() {
        return std::string("sdds_network_settings_struct");
    };

    std::string interface;
    std::string ip_address;
    CORBA::Long port;
    unsigned short vlan;
};

inline bool operator>>= (const CORBA::Any& a, sdds_network_settings_struct_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("sdds_network_settings::interface")) {
        if (!(props["sdds_network_settings::interface"] >>= s.interface)) return false;
    }
    if (props.contains("sdds_network_settings::ip_address")) {
        if (!(props["sdds_network_settings::ip_address"] >>= s.ip_address)) return false;
    }
    if (props.contains("sdds_network_settings::port")) {
        if (!(props["sdds_network_settings::port"] >>= s.port)) return false;
    }
    if (props.contains("sdds_network_settings::vlan")) {
        if (!(props["sdds_network_settings::vlan"] >>= s.vlan)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const sdds_network_settings_struct_struct& s) {
    redhawk::PropertyMap props;
 
    props["sdds_network_settings::interface"] = s.interface;
 
    props["sdds_network_settings::ip_address"] = s.ip_address;
 
    props["sdds_network_settings::port"] = s.port;
 
    props["sdds_network_settings::vlan"] = s.vlan;
    a <<= props;
}

inline bool operator== (const sdds_network_settings_struct_struct& s1, const sdds_network_settings_struct_struct& s2) {
    if (s1.interface!=s2.interface)
        return false;
    if (s1.ip_address!=s2.ip_address)
        return false;
    if (s1.port!=s2.port)
        return false;
    if (s1.vlan!=s2.vlan)
        return false;
    return true;
}

inline bool operator!= (const sdds_network_settings_struct_struct& s1, const sdds_network_settings_struct_struct& s2) {
    return !(s1==s2);
}

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
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("available_devices::type")) {
        if (!(props["available_devices::type"] >>= s.type)) return false;
    }
    if (props.contains("available_devices::ip_address")) {
        if (!(props["available_devices::ip_address"] >>= s.ip_address)) return false;
    }
    if (props.contains("available_devices::name")) {
        if (!(props["available_devices::name"] >>= s.name)) return false;
    }
    if (props.contains("available_devices::serial")) {
        if (!(props["available_devices::serial"] >>= s.serial)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const usrp_device_struct& s) {
    redhawk::PropertyMap props;
 
    props["available_devices::type"] = s.type;
 
    props["available_devices::ip_address"] = s.ip_address;
 
    props["available_devices::name"] = s.name;
 
    props["available_devices::serial"] = s.serial;
    a <<= props;
}

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
}

inline bool operator!= (const usrp_device_struct& s1, const usrp_device_struct& s2) {
    return !(s1==s2);
}

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
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("device_motherboards::mb_name")) {
        if (!(props["device_motherboards::mb_name"] >>= s.mb_name)) return false;
    }
    if (props.contains("device_motherboards::mb_ip")) {
        if (!(props["device_motherboards::mb_ip"] >>= s.mb_ip)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const usrp_motherboard_struct& s) {
    redhawk::PropertyMap props;
 
    props["device_motherboards::mb_name"] = s.mb_name;
 
    props["device_motherboards::mb_ip"] = s.mb_ip;
    a <<= props;
}

inline bool operator== (const usrp_motherboard_struct& s1, const usrp_motherboard_struct& s2) {
    if (s1.mb_name!=s2.mb_name)
        return false;
    if (s1.mb_ip!=s2.mb_ip)
        return false;
    return true;
}

inline bool operator!= (const usrp_motherboard_struct& s1, const usrp_motherboard_struct& s2) {
    return !(s1==s2);
}

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
    std::vector<std::string> available_antennas;
};

inline bool operator>>= (const CORBA::Any& a, usrp_channel_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("device_channels::ch_name")) {
        if (!(props["device_channels::ch_name"] >>= s.ch_name)) return false;
    }
    if (props.contains("device_channels::tuner_type")) {
        if (!(props["device_channels::tuner_type"] >>= s.tuner_type)) return false;
    }
    if (props.contains("device_channels::chan_num")) {
        if (!(props["device_channels::chan_num"] >>= s.chan_num)) return false;
    }
    if (props.contains("device_channels::antenna")) {
        if (!(props["device_channels::antenna"] >>= s.antenna)) return false;
    }
    if (props.contains("device_channels::bandwidth_current")) {
        if (!(props["device_channels::bandwidth_current"] >>= s.bandwidth_current)) return false;
    }
    if (props.contains("device_channels::bandwidth_min")) {
        if (!(props["device_channels::bandwidth_min"] >>= s.bandwidth_min)) return false;
    }
    if (props.contains("device_channels::bandwidth_max")) {
        if (!(props["device_channels::bandwidth_max"] >>= s.bandwidth_max)) return false;
    }
    if (props.contains("device_channels::rate_current")) {
        if (!(props["device_channels::rate_current"] >>= s.rate_current)) return false;
    }
    if (props.contains("device_channels::rate_min")) {
        if (!(props["device_channels::rate_min"] >>= s.rate_min)) return false;
    }
    if (props.contains("device_channels::rate_max")) {
        if (!(props["device_channels::rate_max"] >>= s.rate_max)) return false;
    }
    if (props.contains("device_channels::freq_current")) {
        if (!(props["device_channels::freq_current"] >>= s.freq_current)) return false;
    }
    if (props.contains("device_channels::freq_min")) {
        if (!(props["device_channels::freq_min"] >>= s.freq_min)) return false;
    }
    if (props.contains("device_channels::freq_max")) {
        if (!(props["device_channels::freq_max"] >>= s.freq_max)) return false;
    }
    if (props.contains("device_channels::gain_current")) {
        if (!(props["device_channels::gain_current"] >>= s.gain_current)) return false;
    }
    if (props.contains("device_channels::gain_min")) {
        if (!(props["device_channels::gain_min"] >>= s.gain_min)) return false;
    }
    if (props.contains("device_channels::gain_max")) {
        if (!(props["device_channels::gain_max"] >>= s.gain_max)) return false;
    }
    if (props.contains("device_channels::clock_min")) {
        if (!(props["device_channels::clock_min"] >>= s.clock_min)) return false;
    }
    if (props.contains("device_channels::clock_max")) {
        if (!(props["device_channels::clock_max"] >>= s.clock_max)) return false;
    }
    if (props.contains("device_channels::available_antennas")) {
        if (!(props["device_channels::available_antennas"] >>= s.available_antennas)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const usrp_channel_struct& s) {
    redhawk::PropertyMap props;
 
    props["device_channels::ch_name"] = s.ch_name;
 
    props["device_channels::tuner_type"] = s.tuner_type;
 
    props["device_channels::chan_num"] = s.chan_num;
 
    props["device_channels::antenna"] = s.antenna;
 
    props["device_channels::bandwidth_current"] = s.bandwidth_current;
 
    props["device_channels::bandwidth_min"] = s.bandwidth_min;
 
    props["device_channels::bandwidth_max"] = s.bandwidth_max;
 
    props["device_channels::rate_current"] = s.rate_current;
 
    props["device_channels::rate_min"] = s.rate_min;
 
    props["device_channels::rate_max"] = s.rate_max;
 
    props["device_channels::freq_current"] = s.freq_current;
 
    props["device_channels::freq_min"] = s.freq_min;
 
    props["device_channels::freq_max"] = s.freq_max;
 
    props["device_channels::gain_current"] = s.gain_current;
 
    props["device_channels::gain_min"] = s.gain_min;
 
    props["device_channels::gain_max"] = s.gain_max;
 
    props["device_channels::clock_min"] = s.clock_min;
 
    props["device_channels::clock_max"] = s.clock_max;
 
    props["device_channels::available_antennas"] = s.available_antennas;
    a <<= props;
}

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
    if (s1.available_antennas!=s2.available_antennas)
        return false;
    return true;
}

inline bool operator!= (const usrp_channel_struct& s1, const usrp_channel_struct& s2) {
    return !(s1==s2);
}

#endif // STRUCTPROPS_H
