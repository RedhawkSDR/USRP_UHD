#!/usr/bin/env python

# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of USRP_UHD Device.
#
# USRP_UHD Device is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation, either version 3 of the License, or (at your
# option) any later version.
#
# USRP_UHD Device is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.

import sys, os
script_dir = os.path.dirname(os.path.abspath(__file__))
project_dir = os.path.abspath(os.path.join(script_dir, '..'))
lib_dir = os.path.join(script_dir, 'fei_base')
sys.path.append(lib_dir)
import frontend_tuner_unit_test_base as fe

''' TODO:
  1)  set the desired DEBUG_LEVEL (typical values include 0, 1, 2, 3, 4 and 5)
  2)  set DUT correctly to specify the USRP under test
  3)  set IMPL_ID to the implementation that should be tested.
  4)  Optional: set dut_execparams if it is necessary to specify a particular
      USRP. Default behavior is to target the first device of the type specified.
  5)  Optional: set dut_capabilities to reflect the hardware capabilities of
      the dut if different from what is provided below.
'''

DEBUG_LEVEL = 3

# DUT='USRP|<type>|<uhd driver version>'
# DUT='USRP|usrp2|uhd.3.5.3'
# DUT='USRP|usrp2|uhd.3.5.5'
# DUT='USRP|b200|uhd.3.7.1'
# DUT='USRP|usrp2|uhd.3.7.1'
# DUT='USRP|usrp3|uhd.3.7.1'
# DUT='USRP|b200|uhd.3.7.3'
DUT='USRP|usrp2|uhd.3.7.3'  # default
# DUT='USRP|usrp3|uhd.3.7.3'

IMPL_ID='cpp'

if 'USRP' in DUT:
    dut_name = 'USRP_UHD'
    dut_execparams = {}
    dut_configure = {'target_device':{'target::name':'',
                                       'target::serial':'',
                                       'target::ip_address':'',
                                       'target::type':DUT.split('|')[1]},
                      'device_group_id_global':'FEI_UNIT_TESTING',
                      'sdds_network_settings':[{'sdds_network_settings::interface':'lo',
                                                'sdds_network_settings::ip_address':'127.0.0.1',
                                                'sdds_network_settings::port':29495,
                                                'sdds_network_settings::vlan':0}
                                               ]
                     }

    # UHD 3.5.3
    if 'uhd.3.5.3' in DUT:
        dut_capabilities = {'RX_DIGITIZER':{'COMPLEX': True,
                                            'CF_MAX': 2240.000e6,
                                            'CF_MIN':   28.750e6,
                                            'BW_MAX':   40.0e6,
                                            'BW_MIN':   40.0e6,
                                            'SR_MAX':   50.0e6,
                                            'SR_MIN':    0.1953125e6,
                                            'GAIN_MIN' :0.0,
                                            'GAIN_MAX' :38.0}}
    
    # UHD 3.5.5 and newer - freq range reported by usrp2 is 20 MHz more restrictive on each limit (40 MHz total)
    #elif 'uhd.3.5.5' in DUT.split('|')[3]:
    else:
        if  DUT.split('|')[1] == 'b200':
            dut_capabilities = {'RX_DIGITIZER':{'COMPLEX': True,
                                                'CF_MAX': 6000.0e6,
                                                'CF_MIN':   70.0e6,
                                                'BW_MAX':   56.0e6,
                                                'BW_MIN':   56.0e6,
                                                'SR_MAX':   32.0e6,
                                                'SR_MIN':   62.5e3,
                                                'GAIN_MIN' :0.0,
                                                'GAIN_MAX' :73.0}}
        elif  DUT.split('|')[1] == 'usrp3':
            # TODO - update SR, CF, GAIN with correct capabilities
            dut_capabilities = {'RX_DIGITIZER':{'COMPLEX': True,
                                                'CF_MAX': 2220.0e6,
                                                'CF_MIN':   48.75e6,
                                                'BW_MAX':  120.0e6,
                                                'BW_MIN':  120.0e6,
                                                'SR_MAX':   50.0e6,
                                                'SR_MIN':  195.3125e3,
                                                'GAIN_MIN' :0.0,
                                                'GAIN_MAX' :73.0}}
        #elif  DUT.split('|')[1] == 'usrp2':
        # assume usrp2 - just fake it, we'll figure it out once we can
        #                query the USRP for it's daughterboard type.
        else:
            dut_capabilities = {'RX_DIGITIZER':{'COMPLEX': True,
                                                'CF_MAX': 0.0e6,
                                                'CF_MIN': 0.0e6,
                                                'BW_MAX': 0.0e6,
                                                'BW_MIN': 0.0e6,
                                                'SR_MAX': 0.0e6,
                                                'SR_MIN': 0.0e3,
                                                'GAIN_MIN' :0.0,
                                                'GAIN_MAX' :0.0}}

#******* DO NOT MODIFY BELOW **********#
DEVICE_INFO = {}
DEVICE_INFO[dut_name]               = dut_capabilities
DEVICE_INFO[dut_name]['SPD']        = os.path.join(project_dir, 'USRP_UHD.spd.xml')
DEVICE_INFO[dut_name]['execparams'] = dut_execparams
DEVICE_INFO[dut_name]['configure']  = dut_configure
#******* DO NOT MODIFY ABOVE **********#


class FrontendTunerTests(fe.FrontendTunerTests):
    
    def __init__(self,*args,**kwargs):
        import ossie.utils.testing
        super(FrontendTunerTests,self).__init__(*args,**kwargs)
        fe.set_debug_level(DEBUG_LEVEL)
        fe.set_device_info(DEVICE_INFO[dut_name])
        fe.set_impl_id(IMPL_ID)
  
    # Use functions below to add pre-/post-launch commands if your device has special startup requirements
    @classmethod
    def devicePreLaunch(cls):
        pass

    @classmethod
    def devicePostLaunch(cls):
        if 'usrp2' in DUT:
            #print "device channel: ", cls._query( ("device_channels",) )
            device_channels = cls._query( ("device_channels",) )['device_channels']

            #print "daughterboard :: ", device_channels[0]['device_channels::ch_name']
            ch_name = device_channels[0]['device_channels::ch_name']
            if 'SBX' in ch_name:
                print "Using capabilities for an SBX"
                DEVICE_INFO[dut_name]['RX_DIGITIZER'] = {'COMPLEX': True,
                                                         'CF_MAX': 4420.0e6,
                                                         'CF_MIN':  380.0e6,
                                                         'BW_MAX':   40.0e6,
                                                         'BW_MIN':   40.0e6,
                                                         'SR_MAX':   50.0e6,
                                                         'SR_MIN':  195.3125e3,
                                                         'GAIN_MIN' :0.0,
                                                         'GAIN_MAX' :38.0}
            elif 'WBX' in ch_name:
                print "Using capabilities for an WBX"
                DEVICE_INFO[dut_name]['RX_DIGITIZER'] = {'COMPLEX': True,
                                                         'CF_MAX': 2220.0e6,
                                                         'CF_MIN':   48.75e6,
                                                         'BW_MAX':   40.0e6,
                                                         'BW_MIN':   40.0e6,
                                                         'SR_MAX':   50.0e6,
                                                         'SR_MIN':  195.3125e3,
                                                         'GAIN_MIN' :0.0,
                                                         'GAIN_MAX' :38.0}
            else:
                print >> sys.stderr, "unable to determine daughterboard for this USRP2 device. Aborting." 
                sys.exit()
                
    
    # Use functions below to add pre-/post-release commands if your device has special shutdown requirements
    @classmethod
    def devicePreRelease(self):
        pass
    @classmethod
    def devicePostRelease(self):
        pass
    
    
if __name__ == '__main__':
    fe.set_debug_level(DEBUG_LEVEL)
    fe.set_device_info(DEVICE_INFO[dut_name])
    fe.set_impl_id(IMPL_ID)
    
    # run using nose
    import nose
    nose.main(defaultTest=__name__)
