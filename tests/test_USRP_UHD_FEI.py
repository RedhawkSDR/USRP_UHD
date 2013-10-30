#!/usr/bin/env python

# taken from frontendUnitTests repository
#   tag: "rel3"

import frontend_tuner_unit_test_base as fe

DEBUG_LEVEL = 0

# Define device under test below
DEVICE_INFO = {'SPD':'../USRP_UHD.spd.xml'}

# TODO: add execparams for your device here. {'prop_name':'value'}
DEVICE_INFO['execparams'] = {'USRP_ip_address':'127.0.0.1'}

# TODO: provide information about the basic capabilities of your device.
# Include a key entry for each tuner type: RX, TX, RX_DIGITIZER, CHANNELIZER, RX_DIGITIZER_CHANNELIZER, DDC
# And include max/min values for CF, BW, SR, and GAIN

# values for a USRP
DEVICE_INFO['RX_DIGITIZER'] = {'CF_MAX': 2.24e9,
                               'CF_MIN': 0.02875e9,
                               'BW_MAX': 0.04e9,
                               'BW_MIN': 0.04e9,
                               'SR_MAX': 0.1e9/2.0, # complex samples
                               'SR_MIN': 0.003125e9/2.0, # complex samples
                               'GAIN_MIN' :0.0,
                               'GAIN_MAX' :38.0}

class USRP_UHDFrontendTunerTests(fe.FrontendTunerTests):
    
    # Use functions below to add pre-/post-launch commands if your device has special startup requirements
    def devicePreLaunch(self):
        pass
    def devicePostLaunch(self):
        pass
    
    # Use functions below to add pre-/post-release commands if your device has special shutdown requirements
    def devicePreRelease(self):
        pass
    def devicePostRelease(self):
        pass
    
    
if __name__ == '__main__':
    import ossie.utils.testing
    fe.set_debug_level(DEBUG_LEVEL)
    fe.set_device_info(DEVICE_INFO)
    ossie.utils.testing.main(DEVICE_INFO['SPD'])
    
    # to search parent dir for SPD file...
    #import os
    #cname = os.path.basename(os.path.abspath(os.pardir))
    #spdfile = os.path.join(os.pardir,cname+'.spd.xml')
    #if os.path.exists(spdfile):
    #    print 'Using SPD file %s'%os.path.abspath(spdfile)
    #    fe.DEVICE_INFO['SPD'] = spdfile
    #    ossie.utils.testing.main(spdfile)
    #else:
    #    print 'ERROR - SPD file (%s) not found in parent directory. Executing in %s'%(spdfile,os.path.abspath(os.curdir))
