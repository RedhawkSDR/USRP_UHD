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

 
#ifndef STRUCTPROPS_H
#define STRUCTPROPS_H

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

*******************************************************************************************/

#include <ossie/CorbaUtils.h>

struct usrp_device_struct_struct {
	usrp_device_struct_struct ()
	{
	};

    std::string getId() {
        return std::string("usrp_device_struct");
    };
	
	std::string type;
	std::string ip_address;
	std::string name;
	std::string serial;
};

inline bool operator>>= (const CORBA::Any& a, usrp_device_struct_struct& s) {
	CF::Properties* temp;
	if (!(a >>= temp)) return false;
	CF::Properties& props = *temp;
	for (unsigned int idx = 0; idx < props.length(); idx++) {
		if (!strcmp("type", props[idx].id)) {
			if (!(props[idx].value >>= s.type)) return false;
		}
		if (!strcmp("ip_address", props[idx].id)) {
			if (!(props[idx].value >>= s.ip_address)) return false;
		}
		if (!strcmp("name", props[idx].id)) {
			if (!(props[idx].value >>= s.name)) return false;
		}
		if (!strcmp("serial", props[idx].id)) {
			if (!(props[idx].value >>= s.serial)) return false;
		}
	}
	return true;
};

inline void operator<<= (CORBA::Any& a, const usrp_device_struct_struct& s) {
	CF::Properties props;
	props.length(4);
	props[0].id = CORBA::string_dup("type");
	props[0].value <<= s.type;
	props[1].id = CORBA::string_dup("ip_address");
	props[1].value <<= s.ip_address;
	props[2].id = CORBA::string_dup("name");
	props[2].value <<= s.name;
	props[3].id = CORBA::string_dup("serial");
	props[3].value <<= s.serial;
	a <<= props;
};

inline bool operator== (usrp_device_struct_struct& s1, const usrp_device_struct_struct& s2) {
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

inline bool operator!= (usrp_device_struct_struct& s1, const usrp_device_struct_struct& s2) {
    return !(s1==s2);
};

template<> inline short StructProperty<usrp_device_struct_struct>::compare (const CORBA::Any& a) {
    if (super::isNil_) {
        if (a.type()->kind() == (CORBA::tk_null)) {
            return 0;
        }
        return 1;
    }

    usrp_device_struct_struct tmp;
    if (fromAny(a, tmp)) {
        if (tmp != this->value_) {
            return 1;
        }

        return 0;
    } else {
        return 1;
    }
}

struct usrp_channel_struct_struct {
	usrp_channel_struct_struct ()
	{
	};

    std::string getId() {
        return std::string("usrp_channel_struct");
    };
	
	std::string ch_name;
	std::string tuner_type;
	short chan_num;
	std::string antenna;
	double bandwidth;
	double rate_current;
	double rate_min;
	double rate_max;
	double freq_current;
	double freq_min;
	double freq_max;
	double gain_current;
	double gain_min;
	double gain_max;
};

inline bool operator>>= (const CORBA::Any& a, usrp_channel_struct_struct& s) {
	CF::Properties* temp;
	if (!(a >>= temp)) return false;
	CF::Properties& props = *temp;
	for (unsigned int idx = 0; idx < props.length(); idx++) {
		if (!strcmp("ch_name", props[idx].id)) {
			if (!(props[idx].value >>= s.ch_name)) return false;
		}
		if (!strcmp("tuner_type", props[idx].id)) {
			if (!(props[idx].value >>= s.tuner_type)) return false;
		}
		if (!strcmp("chan_num", props[idx].id)) {
			if (!(props[idx].value >>= s.chan_num)) return false;
		}
		if (!strcmp("antenna", props[idx].id)) {
			if (!(props[idx].value >>= s.antenna)) return false;
		}
		if (!strcmp("bandwidth", props[idx].id)) {
			if (!(props[idx].value >>= s.bandwidth)) return false;
		}
		if (!strcmp("rate_current", props[idx].id)) {
			if (!(props[idx].value >>= s.rate_current)) return false;
		}
		if (!strcmp("rate_min", props[idx].id)) {
			if (!(props[idx].value >>= s.rate_min)) return false;
		}
		if (!strcmp("rate_max", props[idx].id)) {
			if (!(props[idx].value >>= s.rate_max)) return false;
		}
		if (!strcmp("freq_current", props[idx].id)) {
			if (!(props[idx].value >>= s.freq_current)) return false;
		}
		if (!strcmp("freq_min", props[idx].id)) {
			if (!(props[idx].value >>= s.freq_min)) return false;
		}
		if (!strcmp("freq_max", props[idx].id)) {
			if (!(props[idx].value >>= s.freq_max)) return false;
		}
		if (!strcmp("gain_current", props[idx].id)) {
			if (!(props[idx].value >>= s.gain_current)) return false;
		}
		if (!strcmp("gain_min", props[idx].id)) {
			if (!(props[idx].value >>= s.gain_min)) return false;
		}
		if (!strcmp("gain_max", props[idx].id)) {
			if (!(props[idx].value >>= s.gain_max)) return false;
		}
	}
	return true;
};

inline void operator<<= (CORBA::Any& a, const usrp_channel_struct_struct& s) {
	CF::Properties props;
	props.length(14);
	props[0].id = CORBA::string_dup("ch_name");
	props[0].value <<= s.ch_name;
	props[1].id = CORBA::string_dup("tuner_type");
	props[1].value <<= s.tuner_type;
	props[2].id = CORBA::string_dup("chan_num");
	props[2].value <<= s.chan_num;
	props[3].id = CORBA::string_dup("antenna");
	props[3].value <<= s.antenna;
	props[4].id = CORBA::string_dup("bandwidth");
	props[4].value <<= s.bandwidth;
	props[5].id = CORBA::string_dup("rate_current");
	props[5].value <<= s.rate_current;
	props[6].id = CORBA::string_dup("rate_min");
	props[6].value <<= s.rate_min;
	props[7].id = CORBA::string_dup("rate_max");
	props[7].value <<= s.rate_max;
	props[8].id = CORBA::string_dup("freq_current");
	props[8].value <<= s.freq_current;
	props[9].id = CORBA::string_dup("freq_min");
	props[9].value <<= s.freq_min;
	props[10].id = CORBA::string_dup("freq_max");
	props[10].value <<= s.freq_max;
	props[11].id = CORBA::string_dup("gain_current");
	props[11].value <<= s.gain_current;
	props[12].id = CORBA::string_dup("gain_min");
	props[12].value <<= s.gain_min;
	props[13].id = CORBA::string_dup("gain_max");
	props[13].value <<= s.gain_max;
	a <<= props;
};

inline bool operator== (usrp_channel_struct_struct& s1, const usrp_channel_struct_struct& s2) {
    if (s1.ch_name!=s2.ch_name)
        return false;
    if (s1.tuner_type!=s2.tuner_type)
        return false;
    if (s1.chan_num!=s2.chan_num)
        return false;
    if (s1.antenna!=s2.antenna)
        return false;
    if (s1.bandwidth!=s2.bandwidth)
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
    return true;
};

inline bool operator!= (usrp_channel_struct_struct& s1, const usrp_channel_struct_struct& s2) {
    return !(s1==s2);
};

template<> inline short StructProperty<usrp_channel_struct_struct>::compare (const CORBA::Any& a) {
    if (super::isNil_) {
        if (a.type()->kind() == (CORBA::tk_null)) {
            return 0;
        }
        return 1;
    }

    usrp_channel_struct_struct tmp;
    if (fromAny(a, tmp)) {
        if (tmp != this->value_) {
            return 1;
        }

        return 0;
    } else {
        return 1;
    }
}

struct usrp_motherboard_struct_struct {
	usrp_motherboard_struct_struct ()
	{
	};

    std::string getId() {
        return std::string("usrp_motherboard_struct");
    };
	
	std::string mb_name;
	std::string mb_ip;
};

inline bool operator>>= (const CORBA::Any& a, usrp_motherboard_struct_struct& s) {
	CF::Properties* temp;
	if (!(a >>= temp)) return false;
	CF::Properties& props = *temp;
	for (unsigned int idx = 0; idx < props.length(); idx++) {
		if (!strcmp("mb_name", props[idx].id)) {
			if (!(props[idx].value >>= s.mb_name)) return false;
		}
		if (!strcmp("mb_ip", props[idx].id)) {
			if (!(props[idx].value >>= s.mb_ip)) return false;
		}
	}
	return true;
};

inline void operator<<= (CORBA::Any& a, const usrp_motherboard_struct_struct& s) {
	CF::Properties props;
	props.length(2);
	props[0].id = CORBA::string_dup("mb_name");
	props[0].value <<= s.mb_name;
	props[1].id = CORBA::string_dup("mb_ip");
	props[1].value <<= s.mb_ip;
	a <<= props;
};

inline bool operator== (usrp_motherboard_struct_struct& s1, const usrp_motherboard_struct_struct& s2) {
    if (s1.mb_name!=s2.mb_name)
        return false;
    if (s1.mb_ip!=s2.mb_ip)
        return false;
    return true;
};

inline bool operator!= (usrp_motherboard_struct_struct& s1, const usrp_motherboard_struct_struct& s2) {
    return !(s1==s2);
};

template<> inline short StructProperty<usrp_motherboard_struct_struct>::compare (const CORBA::Any& a) {
    if (super::isNil_) {
        if (a.type()->kind() == (CORBA::tk_null)) {
            return 0;
        }
        return 1;
    }

    usrp_motherboard_struct_struct tmp;
    if (fromAny(a, tmp)) {
        if (tmp != this->value_) {
            return 1;
        }

        return 0;
    } else {
        return 1;
    }
}

struct frontend_tuner_allocation_struct {
	frontend_tuner_allocation_struct ()
	{
		tuner_type = "";
		allocation_id = "";
		center_frequency = 0.0;
		bandwidth = 0.0;
		bandwidth_tolerance = 10.0;
		sample_rate = 0.0;
		sample_rate_tolerance = 10.0;
		device_control = true;
		group_id = "";
		rf_flow_id = "";
	};

    std::string getId() {
        return std::string("FRONTEND::tuner_allocation");
    };
	
	std::string tuner_type;
	std::string allocation_id;
	double center_frequency;
	double bandwidth;
	double bandwidth_tolerance;
	double sample_rate;
	double sample_rate_tolerance;
	bool device_control;
	std::string group_id;
	std::string rf_flow_id;
};

inline bool operator>>= (const CORBA::Any& a, frontend_tuner_allocation_struct& s) {
	CF::Properties* temp;
	if (!(a >>= temp)) return false;
	CF::Properties& props = *temp;
	for (unsigned int idx = 0; idx < props.length(); idx++) {
		if (!strcmp("FRONTEND::tuner_allocation::tuner_type", props[idx].id)) {
			if (!(props[idx].value >>= s.tuner_type)) return false;
		}
		if (!strcmp("FRONTEND::tuner_allocation::allocation_id", props[idx].id)) {
			if (!(props[idx].value >>= s.allocation_id)) return false;
		}
		if (!strcmp("FRONTEND::tuner_allocation::center_frequency", props[idx].id)) {
			if (!(props[idx].value >>= s.center_frequency)) return false;
		}
		if (!strcmp("FRONTEND::tuner_allocation::bandwidth", props[idx].id)) {
			if (!(props[idx].value >>= s.bandwidth)) return false;
		}
		if (!strcmp("FRONTEND::tuner_allocation::bandwidth_tolerance", props[idx].id)) {
			if (!(props[idx].value >>= s.bandwidth_tolerance)) return false;
		}
		if (!strcmp("FRONTEND::tuner_allocation::sample_rate", props[idx].id)) {
			if (!(props[idx].value >>= s.sample_rate)) return false;
		}
		if (!strcmp("FRONTEND::tuner_allocation::sample_rate_tolerance", props[idx].id)) {
			if (!(props[idx].value >>= s.sample_rate_tolerance)) return false;
		}
		if (!strcmp("FRONTEND::tuner_allocation::device_control", props[idx].id)) {
			if (!(props[idx].value >>= s.device_control)) return false;
		}
		if (!strcmp("FRONTEND::tuner_allocation::group_id", props[idx].id)) {
			if (!(props[idx].value >>= s.group_id)) return false;
		}
		if (!strcmp("FRONTEND::tuner_allocation::rf_flow_id", props[idx].id)) {
			if (!(props[idx].value >>= s.rf_flow_id)) return false;
		}
	}
	return true;
};

inline void operator<<= (CORBA::Any& a, const frontend_tuner_allocation_struct& s) {
	CF::Properties props;
	props.length(10);
	props[0].id = CORBA::string_dup("FRONTEND::tuner_allocation::tuner_type");
	props[0].value <<= s.tuner_type;
	props[1].id = CORBA::string_dup("FRONTEND::tuner_allocation::allocation_id");
	props[1].value <<= s.allocation_id;
	props[2].id = CORBA::string_dup("FRONTEND::tuner_allocation::center_frequency");
	props[2].value <<= s.center_frequency;
	props[3].id = CORBA::string_dup("FRONTEND::tuner_allocation::bandwidth");
	props[3].value <<= s.bandwidth;
	props[4].id = CORBA::string_dup("FRONTEND::tuner_allocation::bandwidth_tolerance");
	props[4].value <<= s.bandwidth_tolerance;
	props[5].id = CORBA::string_dup("FRONTEND::tuner_allocation::sample_rate");
	props[5].value <<= s.sample_rate;
	props[6].id = CORBA::string_dup("FRONTEND::tuner_allocation::sample_rate_tolerance");
	props[6].value <<= s.sample_rate_tolerance;
	props[7].id = CORBA::string_dup("FRONTEND::tuner_allocation::device_control");
	props[7].value <<= s.device_control;
	props[8].id = CORBA::string_dup("FRONTEND::tuner_allocation::group_id");
	props[8].value <<= s.group_id;
	props[9].id = CORBA::string_dup("FRONTEND::tuner_allocation::rf_flow_id");
	props[9].value <<= s.rf_flow_id;
	a <<= props;
};

inline bool operator== (frontend_tuner_allocation_struct& s1, const frontend_tuner_allocation_struct& s2) {
    if (s1.tuner_type!=s2.tuner_type)
        return false;
    if (s1.allocation_id!=s2.allocation_id)
        return false;
    if (s1.center_frequency!=s2.center_frequency)
        return false;
    if (s1.bandwidth!=s2.bandwidth)
        return false;
    if (s1.bandwidth_tolerance!=s2.bandwidth_tolerance)
        return false;
    if (s1.sample_rate!=s2.sample_rate)
        return false;
    if (s1.sample_rate_tolerance!=s2.sample_rate_tolerance)
        return false;
    if (s1.device_control!=s2.device_control)
        return false;
    if (s1.group_id!=s2.group_id)
        return false;
    if (s1.rf_flow_id!=s2.rf_flow_id)
        return false;
    return true;
};

inline bool operator!= (frontend_tuner_allocation_struct& s1, const frontend_tuner_allocation_struct& s2) {
    return !(s1==s2);
};

template<> inline short StructProperty<frontend_tuner_allocation_struct>::compare (const CORBA::Any& a) {
    if (super::isNil_) {
        if (a.type()->kind() == (CORBA::tk_null)) {
            return 0;
        }
        return 1;
    }

    frontend_tuner_allocation_struct tmp;
    if (fromAny(a, tmp)) {
        if (tmp != this->value_) {
            return 1;
        }

        return 0;
    } else {
        return 1;
    }
}

struct frontend_listener_allocation_struct {
	frontend_listener_allocation_struct ()
	{
	};

    std::string getId() {
        return std::string("FRONTEND::listener_allocation");
    };
	
	std::string existing_allocation_id;
	std::string listener_allocation_id;
};

inline bool operator>>= (const CORBA::Any& a, frontend_listener_allocation_struct& s) {
	CF::Properties* temp;
	if (!(a >>= temp)) return false;
	CF::Properties& props = *temp;
	for (unsigned int idx = 0; idx < props.length(); idx++) {
		if (!strcmp("FRONTEND::listener_allocation::existing_allocation_id", props[idx].id)) {
			if (!(props[idx].value >>= s.existing_allocation_id)) return false;
		}
		if (!strcmp("FRONTEND::listener_allocation::listener_allocation_id", props[idx].id)) {
			if (!(props[idx].value >>= s.listener_allocation_id)) return false;
		}
	}
	return true;
};

inline void operator<<= (CORBA::Any& a, const frontend_listener_allocation_struct& s) {
	CF::Properties props;
	props.length(2);
	props[0].id = CORBA::string_dup("FRONTEND::listener_allocation::existing_allocation_id");
	props[0].value <<= s.existing_allocation_id;
	props[1].id = CORBA::string_dup("FRONTEND::listener_allocation::listener_allocation_id");
	props[1].value <<= s.listener_allocation_id;
	a <<= props;
};

inline bool operator== (frontend_listener_allocation_struct& s1, const frontend_listener_allocation_struct& s2) {
    if (s1.existing_allocation_id!=s2.existing_allocation_id)
        return false;
    if (s1.listener_allocation_id!=s2.listener_allocation_id)
        return false;
    return true;
};

inline bool operator!= (frontend_listener_allocation_struct& s1, const frontend_listener_allocation_struct& s2) {
    return !(s1==s2);
};

template<> inline short StructProperty<frontend_listener_allocation_struct>::compare (const CORBA::Any& a) {
    if (super::isNil_) {
        if (a.type()->kind() == (CORBA::tk_null)) {
            return 0;
        }
        return 1;
    }

    frontend_listener_allocation_struct tmp;
    if (fromAny(a, tmp)) {
        if (tmp != this->value_) {
            return 1;
        }

        return 0;
    } else {
        return 1;
    }
}

struct frontend_tuner_status_struct_struct {
	frontend_tuner_status_struct_struct ()
	{
		tuner_type = "";
		allocation_id_csv = "";
		center_frequency = 0.0;
		bandwidth = 0.0;
		sample_rate = 0.0;
		device_control = true;
		group_id = "";
		rf_flow_id = "";
		enabled = false;
		complex = false;
		gain = 0.0;
		available_frequency = "";
		available_gain = "";
		available_sample_rate = "";
	};

    std::string getId() {
        return std::string("frontend_tuner_status_struct");
    };
	
	std::string tuner_type;
	std::string allocation_id_csv;
	double center_frequency;
	double bandwidth;
	double sample_rate;
	bool device_control;
	std::string group_id;
	std::string rf_flow_id;
	bool enabled;
	bool complex;
	double gain;
	std::string available_frequency;
	std::string available_gain;
	std::string available_sample_rate;
};

inline bool operator>>= (const CORBA::Any& a, frontend_tuner_status_struct_struct& s) {
	CF::Properties* temp;
	if (!(a >>= temp)) return false;
	CF::Properties& props = *temp;
	for (unsigned int idx = 0; idx < props.length(); idx++) {
		if (!strcmp("FRONTEND::tuner_status::tuner_type", props[idx].id)) {
			if (!(props[idx].value >>= s.tuner_type)) return false;
		}
		if (!strcmp("FRONTEND::tuner_status::allocation_id_csv", props[idx].id)) {
			if (!(props[idx].value >>= s.allocation_id_csv)) return false;
		}
		if (!strcmp("FRONTEND::tuner_status::center_frequency", props[idx].id)) {
			if (!(props[idx].value >>= s.center_frequency)) return false;
		}
		if (!strcmp("FRONTEND::tuner_status::bandwidth", props[idx].id)) {
			if (!(props[idx].value >>= s.bandwidth)) return false;
		}
		if (!strcmp("FRONTEND::tuner_status::sample_rate", props[idx].id)) {
			if (!(props[idx].value >>= s.sample_rate)) return false;
		}
		if (!strcmp("FRONTEND::tuner_status::device_control", props[idx].id)) {
			if (!(props[idx].value >>= s.device_control)) return false;
		}
		if (!strcmp("FRONTEND::tuner_status::group_id", props[idx].id)) {
			if (!(props[idx].value >>= s.group_id)) return false;
		}
		if (!strcmp("FRONTEND::tuner_status::rf_flow_id", props[idx].id)) {
			if (!(props[idx].value >>= s.rf_flow_id)) return false;
		}
		if (!strcmp("FRONTEND::tuner_status::enabled", props[idx].id)) {
			if (!(props[idx].value >>= s.enabled)) return false;
		}
		if (!strcmp("FRONTEND::tuner_status::complex", props[idx].id)) {
			if (!(props[idx].value >>= s.complex)) return false;
		}
		if (!strcmp("FRONTEND::tuner_status::gain", props[idx].id)) {
			if (!(props[idx].value >>= s.gain)) return false;
		}
		if (!strcmp("FRONTEND::tuner_status::available_frequency", props[idx].id)) {
			if (!(props[idx].value >>= s.available_frequency)) return false;
		}
		if (!strcmp("FRONTEND::tuner_status::available_gain", props[idx].id)) {
			if (!(props[idx].value >>= s.available_gain)) return false;
		}
		if (!strcmp("FRONTEND::tuner_status::available_sample_rate", props[idx].id)) {
			if (!(props[idx].value >>= s.available_sample_rate)) return false;
		}
	}
	return true;
};

inline void operator<<= (CORBA::Any& a, const frontend_tuner_status_struct_struct& s) {
	CF::Properties props;
	props.length(14);
	props[0].id = CORBA::string_dup("FRONTEND::tuner_status::tuner_type");
	props[0].value <<= s.tuner_type;
	props[1].id = CORBA::string_dup("FRONTEND::tuner_status::allocation_id_csv");
	props[1].value <<= s.allocation_id_csv;
	props[2].id = CORBA::string_dup("FRONTEND::tuner_status::center_frequency");
	props[2].value <<= s.center_frequency;
	props[3].id = CORBA::string_dup("FRONTEND::tuner_status::bandwidth");
	props[3].value <<= s.bandwidth;
	props[4].id = CORBA::string_dup("FRONTEND::tuner_status::sample_rate");
	props[4].value <<= s.sample_rate;
	props[5].id = CORBA::string_dup("FRONTEND::tuner_status::device_control");
	props[5].value <<= s.device_control;
	props[6].id = CORBA::string_dup("FRONTEND::tuner_status::group_id");
	props[6].value <<= s.group_id;
	props[7].id = CORBA::string_dup("FRONTEND::tuner_status::rf_flow_id");
	props[7].value <<= s.rf_flow_id;
	props[8].id = CORBA::string_dup("FRONTEND::tuner_status::enabled");
	props[8].value <<= s.enabled;
	props[9].id = CORBA::string_dup("FRONTEND::tuner_status::complex");
	props[9].value <<= s.complex;
	props[10].id = CORBA::string_dup("FRONTEND::tuner_status::gain");
	props[10].value <<= s.gain;
	props[11].id = CORBA::string_dup("FRONTEND::tuner_status::available_frequency");
	props[11].value <<= s.available_frequency;
	props[12].id = CORBA::string_dup("FRONTEND::tuner_status::available_gain");
	props[12].value <<= s.available_gain;
	props[13].id = CORBA::string_dup("FRONTEND::tuner_status::available_sample_rate");
	props[13].value <<= s.available_sample_rate;
	a <<= props;
};

inline bool operator== (frontend_tuner_status_struct_struct& s1, const frontend_tuner_status_struct_struct& s2) {
    if (s1.tuner_type!=s2.tuner_type)
        return false;
    if (s1.allocation_id_csv!=s2.allocation_id_csv)
        return false;
    if (s1.center_frequency!=s2.center_frequency)
        return false;
    if (s1.bandwidth!=s2.bandwidth)
        return false;
    if (s1.sample_rate!=s2.sample_rate)
        return false;
    if (s1.device_control!=s2.device_control)
        return false;
    if (s1.group_id!=s2.group_id)
        return false;
    if (s1.rf_flow_id!=s2.rf_flow_id)
        return false;
    if (s1.enabled!=s2.enabled)
        return false;
    if (s1.complex!=s2.complex)
        return false;
    if (s1.gain!=s2.gain)
        return false;
    if (s1.available_frequency!=s2.available_frequency)
        return false;
    if (s1.available_gain!=s2.available_gain)
        return false;
    if (s1.available_sample_rate!=s2.available_sample_rate)
        return false;
    return true;
};

inline bool operator!= (frontend_tuner_status_struct_struct& s1, const frontend_tuner_status_struct_struct& s2) {
    return !(s1==s2);
};

template<> inline short StructProperty<frontend_tuner_status_struct_struct>::compare (const CORBA::Any& a) {
    if (super::isNil_) {
        if (a.type()->kind() == (CORBA::tk_null)) {
            return 0;
        }
        return 1;
    }

    frontend_tuner_status_struct_struct tmp;
    if (fromAny(a, tmp)) {
        if (tmp != this->value_) {
            return 1;
        }

        return 0;
    } else {
        return 1;
    }
}

inline bool operator== (std::vector<usrp_device_struct_struct>& s1, const std::vector<usrp_device_struct_struct>& s2) {
    if (s1.size() != s2.size()) {
        return false;
    }
    for (unsigned int i=0; i<s1.size(); i++) {
        if (s1[i] != s2[i]) {
            return false;
        }
    }
    return true;
};

inline bool operator!= (std::vector<usrp_device_struct_struct>& s1, const std::vector<usrp_device_struct_struct>& s2) {
    return !(s1==s2);
};

template<> inline short StructSequenceProperty<usrp_device_struct_struct>::compare (const CORBA::Any& a) {
    if (super::isNil_) {
        if (a.type()->kind() == (CORBA::tk_null)) {
            return 0;
        }
        return 1;
    }

    std::vector<usrp_device_struct_struct> tmp;
    if (fromAny(a, tmp)) {
        if (tmp != this->value_) {
            return 1;
        }

        return 0;
    } else {
        return 1;
    }
}
inline bool operator== (std::vector<usrp_channel_struct_struct>& s1, const std::vector<usrp_channel_struct_struct>& s2) {
    if (s1.size() != s2.size()) {
        return false;
    }
    for (unsigned int i=0; i<s1.size(); i++) {
        if (s1[i] != s2[i]) {
            return false;
        }
    }
    return true;
};

inline bool operator!= (std::vector<usrp_channel_struct_struct>& s1, const std::vector<usrp_channel_struct_struct>& s2) {
    return !(s1==s2);
};

template<> inline short StructSequenceProperty<usrp_channel_struct_struct>::compare (const CORBA::Any& a) {
    if (super::isNil_) {
        if (a.type()->kind() == (CORBA::tk_null)) {
            return 0;
        }
        return 1;
    }

    std::vector<usrp_channel_struct_struct> tmp;
    if (fromAny(a, tmp)) {
        if (tmp != this->value_) {
            return 1;
        }

        return 0;
    } else {
        return 1;
    }
}
inline bool operator== (std::vector<usrp_motherboard_struct_struct>& s1, const std::vector<usrp_motherboard_struct_struct>& s2) {
    if (s1.size() != s2.size()) {
        return false;
    }
    for (unsigned int i=0; i<s1.size(); i++) {
        if (s1[i] != s2[i]) {
            return false;
        }
    }
    return true;
};

inline bool operator!= (std::vector<usrp_motherboard_struct_struct>& s1, const std::vector<usrp_motherboard_struct_struct>& s2) {
    return !(s1==s2);
};

template<> inline short StructSequenceProperty<usrp_motherboard_struct_struct>::compare (const CORBA::Any& a) {
    if (super::isNil_) {
        if (a.type()->kind() == (CORBA::tk_null)) {
            return 0;
        }
        return 1;
    }

    std::vector<usrp_motherboard_struct_struct> tmp;
    if (fromAny(a, tmp)) {
        if (tmp != this->value_) {
            return 1;
        }

        return 0;
    } else {
        return 1;
    }
}
inline bool operator== (std::vector<frontend_tuner_status_struct_struct>& s1, const std::vector<frontend_tuner_status_struct_struct>& s2) {
    if (s1.size() != s2.size()) {
        return false;
    }
    for (unsigned int i=0; i<s1.size(); i++) {
        if (s1[i] != s2[i]) {
            return false;
        }
    }
    return true;
};

inline bool operator!= (std::vector<frontend_tuner_status_struct_struct>& s1, const std::vector<frontend_tuner_status_struct_struct>& s2) {
    return !(s1==s2);
};

template<> inline short StructSequenceProperty<frontend_tuner_status_struct_struct>::compare (const CORBA::Any& a) {
    if (super::isNil_) {
        if (a.type()->kind() == (CORBA::tk_null)) {
            return 0;
        }
        return 1;
    }

    std::vector<frontend_tuner_status_struct_struct> tmp;
    if (fromAny(a, tmp)) {
        if (tmp != this->value_) {
            return 1;
        }

        return 0;
    } else {
        return 1;
    }
}

#endif
