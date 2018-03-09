#!/usr/bin/env python
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK.
#
# REDHAWK is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation, either version 3 of the License, or (at your
# option) any later version.
#
# REDHAWK is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.

import os, sys, copy, time
script_dir = os.path.dirname(os.path.abspath(__file__))
lib_dir = os.path.join(script_dir, 'fei_base')
sys.path.append(lib_dir)
import frontend_tuner_unit_test_base as fe
sdr_devices = os.path.join(os.environ.get('SDRROOT','/var/redhawk/sdr'),'dev/devices')
dut_config = {}

''' TODO:
  1)  set DUT to the key of one of the device configurations defined in the
      device_config dictionary below.
  2)  override spd, parent_dir, namespace, name, execparams, configure,
      or properties if necessary.
  3)  If 'custom', modify configuration to have accurate capabilities for the
      device under test.
  4)  Set DEBUG_LEVEL, DUT_*, or MCAST_* if desired and applicable.
  5)  Advanced: Override functions with device specific behavior.

  NOTE: When testing the USRP_UHD it is possible for the test to freeze due to
        saturating the network connection to the USRP hardware. This is
        is especially true when testing using a virtual machine. To avoid this,
        set the `sr_limit` key in the dut_config struct to the maximum sample
        rate the test should attempt. This must be a valid sample rate for the
        DUT.
'''

#******* MODIFY CONFIG BELOW **********#
DUT = 'USRP|SDDS'

# Optional:
DEBUG_LEVEL = 3    # typical values include 0, 1, 2, 3, 4 and 5
DUT_INDEX = None
DUT_IP_ADDR = None
DUT_PORT = None
DUT_IFACE = None
MCAST_GROUP = None
MCAST_PORT = None
MCAST_VLAN = None
MCAST_IFACE = None

# The following default values will result in a delay being added only when
# necessary. Setting to 0 or False will prevent a delay, setting to any other
# delay number will change the delay only when necessary, and setting ADD_* to
# True or False will force delay to be added or skipped. Delay is only known to
# be needed for USRP X310 when used with UHD 3.10 or greater due to the hardware
# not being available quick enough after a test for the next test.
DELAY_BT_TESTS = 10.0 # Seconds of delay between unit tests if ADD_DELAY_BT_TESTS
ADD_DELAY_BT_TESTS = None # None=added if needed; True=added; False=not added

# Your custom device
dut_config['custom'] = {
    # parent_dir/namespace/name are only used when spd = None
    'spd'         : None,       # path to SPD file for asset.
    'parent_dir'  : None,       # path to parent directory
    'namespace'   : None,       # leave as None if asset is not namespaced
    'name'        : 'myDevice', # exact project name, without namespace
    'impl_id'     : None,       # None for all (or only), 'cpp', 'python', 'java', etc.
    'execparams'  : {},         # {'prop_name': value}
    'configure'   : {},         # {'prop_name': value}
    'properties'  : {},         # {'prop_name': value}
    'capabilities': [           # entry for each tuner; single entry if all tuners are the same
        {                       # include entry for each tuner type (i.e. RX, TX, RX_DIGITIZER, etc.)
            'RX_DIGITIZER': {   # include ranges for CF, BW, SR, and GAIN
                'COMPLEX' : True,
                # To specify a range from 0 to 10 with a gap from 3 to 5: [0, 3, 5, 10]
                'CF'      : [100e6, 500e6], # enter center frequency range in Hz
                'BW'      : [0.0, 1e3],     # enter bandwidth range in Hz
                'SR'      : [0.0, 1e3],     # enter sample rate range in Hz or sps
                'GAIN'    : [-5.0, 10.0]    # enter gain range in dB
            }
        }
    ]
}

#******* MODIFY CONFIG ABOVE **********#

########################################
# Available Pre-defined Configurations #
########################################

# rh.FmRdsSimulator
dut_config['FmRdsSim'] = {
    'spd'         : None,
    'parent_dir'  : None,
    'namespace'   : 'rh',
    'name'        : 'FmRdsSimulator',
    'impl_id'     : 'cpp',
    'execparams'  : {},
    'configure'   : {},
    'properties'  : {},
    'capabilities': [
        {
            'RX_DIGITIZER': {
                'COMPLEX' : True,
                'CF'      : [88e6, 108e6],
                'BW'      : [2.28e6, 2.28e6],
                'SR'      : [2.28e3, 2.28e6],
                'GAIN'    : [-100.0, 100.0]
            }
        }
    ]
}

# rh.USRP_UHD by IP w/o SDDS
dut_config['USRP'] = {
    'spd'         : None,
    'parent_dir'  : None,
    'namespace'   : 'rh',
    'name'        : 'USRP_UHD',
    'impl_id'     : 'cpp',
    'execparams'  : {},
    'configure'   : {
        'target_device': {
            'target::name'      : '',
            'target::serial'    : '',
            'target::ip_address': DUT_IP_ADDR or '',
            'target::type'      : ''
        },
        'device_group_id_global':'FEI_UNIT_TESTING'
    },
    'properties'  : {},
    'capabilities': [],  # populate using query of device props
    'sr_limit'    : 25e6 # prevent network saturation on GigE link
}

# rh.USRP_UHD by IP w/ SDDS
dut_config['USRP|SDDS'] = copy.deepcopy(dut_config['USRP'])
dut_config['USRP|SDDS']['configure']['sdds_network_settings'] = [
    {
        'sdds_network_settings::interface' : MCAST_IFACE or 'lo',
        'sdds_network_settings::ip_address': MCAST_GROUP or '127.0.0.1',
        'sdds_network_settings::port'      : MCAST_PORT or 29495,
        'sdds_network_settings::vlan'      : MCAST_VLAN or 0
    }
]

# rh.USRP_UHD for USRP N2xx w/o SDDS
dut_config['USRP|usrp2'] = copy.deepcopy(dut_config['USRP'])
dut_config['USRP|usrp2']['configure']['target_device']['target::type'] = 'usrp2'

# rh.USRP_UHD for USRP N2xx w/ SDDS
dut_config['USRP|usrp2|SDDS'] = copy.deepcopy(dut_config['USRP|SDDS'])
dut_config['USRP|usrp2|SDDS']['configure']['target_device']['target::type'] = 'usrp2'

# rh.USRP_UHD for USRP X3xx w/o SDDS
dut_config['USRP|x300'] = copy.deepcopy(dut_config['USRP'])
dut_config['USRP|x300']['configure']['target_device']['target::type'] = 'x300'

# rh.USRP_UHD for USRP X3xx w/ SDDS
dut_config['USRP|x300|SDDS'] = copy.deepcopy(dut_config['USRP|SDDS'])
dut_config['USRP|x300|SDDS']['configure']['target_device']['target::type'] = 'x300'

# rh.USRP_UHD for USRP B2xx w/o SDDS
dut_config['USRP|b200'] = copy.deepcopy(dut_config['USRP'])
dut_config['USRP|b200']['configure']['target_device']['target::type'] = 'b200'
dut_config['USRP|b200']['configure']['target_device']['target::ip_address'] = ''

# rh.USRP_UHD for USRP B2xx w/ SDDS
dut_config['USRP|b200|SDDS'] = copy.deepcopy(dut_config['USRP|SDDS'])
dut_config['USRP|b200|SDDS']['configure']['target_device']['target::type'] = 'b200'
dut_config['USRP|b200|SDDS']['configure']['target_device']['target::ip_address'] = ''

# rh.RTL2832U with Rafael Micro R820T or R828D tuner chip
dut_config['RTL'] = {
    'spd'         : None,
    'parent_dir'  : None,
    'namespace'   : 'rh',
    'name'        : 'RTL2832U',
    'impl_id'     : 'cpp',
    'execparams'  : {},
    'configure'   : {
        'group_id': 'FEI_UNIT_TESTING',
        'target_device': {
            'target::name'      : '',
            'target::serial'    : '',
            'target::index'     : DUT_INDEX or '',
            'target::vendor'    : '',
            'target::product'   : ''
        },
    },
    'properties'  : {},
    'capabilities': [
        {
            'RX_DIGITIZER': {
                'COMPLEX' : True,
                'CF'      : [24e3, 1.766e9],
                'BW'      : [28.126e3, 2.56e3],
                'SR'      : [28.126e3, 2.56e3],
                'GAIN'    : [0.0, 49.6]
            }
        }
    ]
}

# rh.RTL2832U with Elonics E4000 tuner chip
dut_config['RTL|E4000'] = copy.deepcopy(dut_config['RTL'])
dut_config['RTL|E4000']['capabilities'][0]['RX_DIGITIZER']['CF'] = [52e6, 1.1e9, 1.25e9, 2.2e9] # gap from 1100 to 1250 MHz
dut_config['RTL|E4000']['capabilities'][0]['RX_DIGITIZER']['GAIN'] = [-1.0, 49.0]

# rh.RTL2832U with FC0013 tuner chip
dut_config['RTL|FC13'] = copy.deepcopy(dut_config['RTL'])
dut_config['RTL|FC13']['capabilities'][0]['RX_DIGITIZER']['CF'] = [22e6, 1.1e9]
dut_config['RTL|FC13']['capabilities'][0]['RX_DIGITIZER']['GAIN'] = [-9.9, 19.7]

# rh.RTL2832U with FC0012 tuner chip
dut_config['RTL|FC13']['capabilities'][0]['RX_DIGITIZER']['CF'] = [22e6, 948.6e6]

# rh.RTL2832U with FC2580 tuner chip
dut_config['RTL|FC2580'] = copy.deepcopy(dut_config['RTL|FC13'])
dut_config['RTL|FC2580']['capabilities'][0]['RX_DIGITIZER']['CF'] = [146e6, 308e6, 438e6, 924e6] # gap from 308 to 438 MHz

# rh.MSDD
dut_config['MSDD'] = {
    'spd'         : None,
    'parent_dir'  : None,
    'namespace'   : 'rh',
    'name'        : 'MSDD',
    'impl_id'     : 'python',
    'execparams'  : {},
    'configure'   : {
        'msdd_configuration': {
            'msdd_configuration::msdd_ip_address': DUT_IP_ADDR or '192.168.100.250',
            'msdd_configuration::msdd_port':       '%s'%(DUT_PORT) if DUT_PORT else '23'
        },
        'msdd_output_configuration': [
            {
                'msdd_output_configuration::tuner_number'    : 0,
                'msdd_output_configuration::protocol'        : 'UDP_SDDS',
                'msdd_output_configuration::ip_address'      : MCAST_GROUP or '234.0.0.100',
                'msdd_output_configuration::port'            : MCAST_PORT or 0,
                'msdd_output_configuration::vlan'            : MCAST_VLAN or 0,
                'msdd_output_configuration::enabled'         : True,
                'msdd_output_configuration::timestamp_offset': 0,
                'msdd_output_configuration::endianess'       : 1,
                'msdd_output_configuration::mfp_flush'       : 63,
                'msdd_output_configuration::vlan_enable'     : False                        
            }
        ]
    },
    'properties'  : {},
    'capabilities': [
        {
            'RX_DIGITIZER': {
                'COMPLEX' : True,
                'CF'      : [30e6, 6e9],
                'BW'      : [20e6, 20e6],
                'SR'      : [25e6, 25e6],
                'GAIN'    : [-48.0, 12.0]
            }
        }
    ]
}

# rh.MSDD_RX_Device
dut_config['MSDD|Dreamin'] = {
    'spd'         : None,
    'parent_dir'  : None,
    'namespace'   : 'rh',
    'name'        : 'MSDD_RX_Device_v2',
    'impl_id'     : 'DCE:3f0ac7cb-1601-456d-97fa-2dcc5be684ee',
    'execparams'  : {},
    'configure'   : {
        'NetworkConfiguration': {
             'status_destination_address'               : MCAST_GROUP or '234.0.0.100',
             'master_destination_address'               : MCAST_GROUP or '234.0.0.100',
             'DCE:d91f918d-5cdf-44d4-a2e1-6f4e5f3128bf' : MCAST_PORT or 8887,
             'master_destination_vlan'                  : MCAST_VLAN or 0,
             'msdd_address'                             : DUT_IP_ADDR or '192.168.100.250',
             'msdd_interface'                           : DUT_IFACE or 'em2',
             'msdd_port'                                : DUT_PORT or 8267
         }
    },
    'properties'  : {},
    'capabilities': [
        {
            'RX_DIGITIZER': {
                'COMPLEX' : True,
                'CF'      : [30e6, 3e9],
                'BW'      : [20e6, 20e6],
                'SR'      : [25e6, 25e6],
                'GAIN'    : [-60.0, 0.0]
            }
        }
    ]
}

# TODO - add configs for MSDD3000 vs MSDD6000

########################################
#    END Pre-defined Configurations    #
########################################

########################################
# BEGIN Custom Override Functions      #
########################################

# If special logic must be used when generating tuner requests, modify the
# function below. and add 'tuner_gen' key to the dut_config struct with the
# function as the associated value. Add as many custom functions as needed for
# all target devices in dut_config.
def customGenerateTunerRequest(idx=0):
    #Pick a random set for CF,BW,SR and return
    value = {}
    value['ALLOC_ID'] = str(uuid.uuid4())
    value['TYPE'] = 'RX_DIGITIZER'
    value['BW_TOLERANCE'] = 100.0
    value['SR_TOLERANCE'] = 100.0
    value['RF_FLOW_ID'] = ''
    value['GROUP_ID'] = ''
    value['CONTROL'] = True

    value['CF'] = getValueInRange(DEVICE_INFO['capabilities'][idx]['RX_DIGITIZER']['CF'])
    sr_subrange = DEVICE_INFO['capabilities'][idx]['RX_DIGITIZER']['SR']
    if 'sr_limit' in DEVICE_INFO and DEVICE_INFO['sr_limit'] > 0:
        sr_subrange = getSubranges([0,DEVICE_INFO['sr_limit']], DEVICE_INFO['capabilities'][idx]['RX_DIGITIZER']['SR'])
    value['SR'] = getValueInRange(sr_subrange)
    # Usable BW is typically equal to SR if complex samples, otherwise half of SR
    BW_MULT = 1.0 if DEVICE_INFO['capabilities'][idx]['RX_DIGITIZER']['COMPLEX'] else 0.5
    value['BW'] = 0.8*value['SR']*BW_MULT # Try 80% of SR
    if isValueInRange(value['BW'], DEVICE_INFO['capabilities'][idx]['RX_DIGITIZER']['BW']):
        # success! all done, return value
        return value

    # Can't use 80% of SR as BW, try to find a BW value within SR tolerances
    bw_min = value['SR']*BW_MULT
    bw_max = value['SR']*(1.0+(value['SR_TOLERANCE']/100.0))*BW_MULT
    tolerance_range = [bw_min,bw_max]
    bw_subrange = getSubranges(tolerance_range, DEVICE_INFO['capabilities'][idx]['RX_DIGITIZER']['BW'])
    if len(bw_subrange) > 0:
        # success! get BW value and return
        value['BW'] = getValueInRange(bw_subrange)
        return value

    # last resort
    value['BW'] = getValueInRange(DEVICE_INFO['capabilities'][idx]['RX_DIGITIZER']['BW'])
    return value
#dut_config['custom']['tuner_gen']=customGenerateTunerRequest # TODO - uncomment this line to use custom
                                                              #        function for custom DUT

# UHD 3.10 introduced an issue where the usrp is slow to become available after release
# which causes unit tests to fail. Introducing a sleep 10 second sleep here solves that
# issue, but makes the test take about 12 minutes longer. It's already ridiculously
# slow with UHD 3.10 because the init of the X310 is also extremely slow. Total time
# may be about 45 minutes.
UHD_VERSION_REQUIRES_DELAY=True # default to True since latest version requires delay
USRP_MODEL_REQUIRES_DELAY=None # To be determined later
if ADD_DELAY_BT_TESTS==None: # No need to check if being forced
    # pkg-config is simple, but only works if uhd-devel is installed
    #from subprocess import call
    #if call(['pkg-config','uhd','--atleast-version=3.10'])==0:
    # Instead, we'll use rpm
    from rpm import TransactionSet, RPMTAG_NAME, RPMTAG_VERSION
    from distutils.version import LooseVersion
    UHD_3_10 = LooseVersion('3.10')
    UHD_INSTALLED = LooseVersion('3.10') # assume 3.10 unless discover otherwise
    ts = TransactionSet()
    mi = ts.dbMatch(RPMTAG_NAME,'uhd')
    for h in mi:
	UHD_INSTALLED = LooseVersion(h[RPMTAG_VERSION])
	break
    if UHD_INSTALLED >= UHD_3_10:
	UHD_VERSION_REQUIRES_DELAY = True
    else:
	UHD_VERSION_REQUIRES_DELAY = False

########################################
#   END Custom Override Functions      #
########################################

# Find SPD if not specified
if not dut_config[DUT]['spd']:
    if dut_config[DUT]['parent_dir']:
        ns = ''
        if dut_config[DUT]['namespace']:
            ns = dut_config[DUT]['namespace']+'.'
        dut_config[DUT]['spd'] = os.path.join(dut_config[DUT]['parent_dir'], ns+dut_config[DUT]['name'], dut_config[DUT]['name']+'.spd.xml')
    else: # Assume it must be in SDR
        dut_config[DUT]['spd'] = os.path.join(sdr_devices, dut_config[DUT]['namespace'] or '', dut_config[DUT]['name'], dut_config[DUT]['name']+'.spd.xml')

# TODO - update to use `properties` instead of `configure` and `execparam`

class FrontendTunerTests(fe.FrontendTunerTests):
    
    def __init__(self,*args,**kwargs):
        import ossie.utils.testing
        super(FrontendTunerTests,self).__init__(*args,**kwargs)
        fe.set_debug_level(DEBUG_LEVEL)
        fe.set_device_info(dut_config[DUT])

    # Use functions below to add pre-/post-launch commands if your device has special startup requirements
    @classmethod
    def devicePreLaunch(cls):
        pass

    @classmethod
    def devicePostLaunch(cls):
        # This will only evaluate True once at most since it will populate capabilities list
        if 'USRP' == DUT[:4] and len(dut_config[DUT]['capabilities'])==0:
            device_channels = cls._query( ('device_channels',) )['device_channels']
            if DEBUG_LEVEL >= 4:
                from pprint import pprint as pp
                print 'device channel: '
                pp(device_channels)

            for chan in device_channels:
                if chan['device_channels::tuner_type'] != 'RX_DIGITIZER':
                    continue
                minbw = min([chan['device_channels::rate_min'],chan['device_channels::bandwidth_min']])
                maxbw = min([chan['device_channels::rate_max'],chan['device_channels::bandwidth_max']])
                chan_capabilities = {
                    chan['device_channels::tuner_type']: {
                        'COMPLEX' : True,
                        'CF'      : [chan['device_channels::freq_min'], chan['device_channels::freq_max']],
                        'BW'      : [minbw, maxbw],
                        'SR'      : [chan['device_channels::rate_min'], chan['device_channels::rate_max']],
                        'GAIN'    : [chan['device_channels::gain_min'], chan['device_channels::gain_max']]
                    }
                }
                dut_config[DUT]['capabilities'].append(chan_capabilities)

        global USRP_MODEL_REQUIRES_DELAY
        if 'USRP' == DUT[:4] and USRP_MODEL_REQUIRES_DELAY == None:
            # get prop from usrp that has type = x300, etc
            try:
                device_mbs = cls._query( ('device_motherboards',) )['device_motherboards']
                if DEBUG_LEVEL >= 4:
                    from pprint import pprint as pp
                    print 'device motherboards: '
                    pp(device_mbs)

                for mb in device_mbs:
                    if 'X3' == mb['device_motherboards::mb_name'][:2].upper():
                        USRP_MODEL_REQUIRES_DELAY = True
                    else:
                        USRP_MODEL_REQUIRES_DELAY = False
            except:
                USRP_MODEL_REQUIRES_DELAY=True

    # Use functions below to add pre-/post-release commands if your device has special shutdown requirements
    @classmethod
    def devicePreRelease(self):
        pass
    @classmethod
    def devicePostRelease(self):
        global DELAY_BT_TESTS, ADD_DELAY_BT_TESTS, UHD_VERSION_REQUIRES_DELAY, USRP_MODEL_REQUIRES_DELAY
        if DELAY_BT_TESTS > 0: #No need to add delay if set to 0
            if ADD_DELAY_BT_TESTS==True:
                print 'DELAY: adding user forced delay'
                time.sleep(DELAY_BT_TESTS)
            elif ADD_DELAY_BT_TESTS==False:
                print 'DELAY: user forced no delay'
            elif UHD_VERSION_REQUIRES_DELAY and USRP_MODEL_REQUIRES_DELAY:
                print 'DELAY: adding uhd+usrp required delay'
                time.sleep(DELAY_BT_TESTS)


if __name__ == '__main__':
    fe.set_debug_level(DEBUG_LEVEL)
    fe.set_device_info(dut_config[DUT])
    
    # run using nose
    import nose
    nose.main(defaultTest=__name__)
