# taken from frontendUnitTests repository
#   tag: "rel3"

import ossie.utils.testing
import os, sys, time, inspect, random, copy
from pprint import pprint as pp
from pprint import pformat as pf

from omniORB import any
from omniORB import CORBA

from ossie import properties
from ossie.cf import CF, CF__POA
#from ossie.cf import ExtendedCF
from ossie.utils import sb
from ossie.utils import uuid
#from ossie.resource import usesport, providesport

from redhawk.frontendInterfaces import FRONTEND, FRONTEND__POA, TunerControl_idl
from bulkio.bulkioInterfaces import BULKIO, BULKIO__POA
from ossie.utils.bulkio import bulkio_data_helpers

DEBUG_LEVEL = 0
def set_debug_level(lvl=0):
    global DEBUG_LEVEL
    DEBUG_LEVEL = lvl
def get_debug_level():
    return DEBUG_LEVEL

# Define device under test below
DEVICE_INFO = {'SPD':None}
def set_device_info(dev_info):
    global DEVICE_INFO
    DEVICE_INFO = dev_info
def get_device_info():
    return DEVICE_INFO

# execparams {'prop_name':'value',...}
DEVICE_INFO['execparams'] = {}

# information about the basic capabilities of the device.
# Must include a key entry for each tuner type applicable for the device:
#     RX, TX, RX_DIGITIZER, CHANNELIZER, RX_DIGITIZER_CHANNELIZER, DDC
# And include max/min values for CF, BW, SR, and GAIN
# FUTURE - require these to be defined in frontend_tuner_status for each tuner. currently this is optional.
'''
DEVICE_INFO['RX'] = {'CF_MAX': 1.0e9,
                     'CF_MIN': 0.1e9,
                     'BW_MAX': 0.025e9,
                     'BW_MIN': 0.001e9,
                     'GAIN_MIN' :0.0,
                     'GAIN_MAX' :1.0}
# Sample values for a USRP
DEVICE_INFO['RX_DIGITIZER'] = {'CF_MAX': 2.24e9,
                               'CF_MIN': 0.02875e9,
                               'BW_MAX': 0.04e9,
                               'BW_MIN': 0.04e9,
                               'SR_MAX': 0.1e9/2.0, # complex samples
                               'SR_MIN': 0.003125e9/2.0, # complex samples
                               'GAIN_MIN' :0.0,
                               'GAIN_MAX' :38.0}
# Sample values for an MSDD6000 WB
DEVICE_INFO['RX_DIGITIZER'] = {'CF_MAX': 6.00e9,
                               'CF_MIN': 0.03e9,
                               'BW_MAX': 0.020e9,
                               'BW_MIN': 0.010e9,
                               'SR_MAX': 0.0250e9, # complex samples
                               'SR_MIN': 0.0125e9, # complex samples
                               'GAIN_MIN' :-60.0,
                               'GAIN_MAX' :0.0}
DEVICE_INFO['CHANNELIZER'] = {'CF_MAX': 1.0e9,
                              'CF_MIN': 0.1e9,
                              'BW_MAX': 0.025e9,
                              'BW_MIN': 0.001e9,
                              'SR_MAX': 0.050e9/2.0, # complex samples
                              'SR_MIN': 0.002e9/2.0, # complex samples
                              'GAIN_MIN' :0.0,
                              'GAIN_MAX' :1.0,
                              'MAX_DDC_PER_CHAN':0} # 0 means DDCs are not mapped to a specific channel, see global DDC max
# Sample values for an MSDD6000 W8N
DEVICE_INFO['RX_DIGITIZER_CHANNELIZER'] = {'CF_MAX': 6.00e9,
                                           'CF_MIN': 0.03e9,
                                           'BW_MAX': 0.01e9,
                                           'BW_MIN': 0.01e9,
                                           'SR_MAX': 0.0125e9, # complex samples
                                           'SR_MIN': 0.0125e9, # complex samples
                                           'GAIN_MIN' :-60.0,
                                           'GAIN_MAX' :0.0,
                                           'MAX_DDC_PER_CHAN':8} # 0 means DDCs are not mapped to a specific channel, see global DDC max
# Sample values for an MSDDX000 W8N
DEVICE_INFO['DDC'] = {'BW_MAX': 0.0391e6,
                      'BW_MIN': 0.0391e6,
                      'SR_MAX': 0.0488e6, # complex samples
                      'SR_MIN': 0.0488e6, # complex samples
                      'GAIN_MIN' :0.0, # TBD
                      'GAIN_MAX' :0.0, # TBD
                      'MAX_DDC':0} # 0 means that DDCs are mapped to each channel, max DDCs must be specified in the channel dict
'''

class FrontendTunerTests(ossie.utils.testing.ScaComponentTestCase):
    ''' FrontEnd device compatibility tests
        Define DUT using the global DEVICE_INFO dict
        Customize deviceStartup function if your device has special start up requirements
        Customize deviceShutdown function if your device has special shut down requirements
    '''
    
    dut = None
    dut_ref = None
    device_discovery = {'TX':0, 'RX':0, 'CHANNELIZER':0, 'DDC':0, 'RX_DIGITIZER':0,
                        'RX_DIGITIZER_CHANNELIZER':0, 'UNKNOWN':0}
    testReport = []
    
    # mapping of required/optional frontend tuner status elements and the allowable data types
    FE_tuner_status_fields_req = {'FRONTEND::tuner_status::tuner_type':[str],
                                  'FRONTEND::tuner_status::allocation_id_csv':[str],
                                  'FRONTEND::tuner_status::center_frequency':[float],
                                  'FRONTEND::tuner_status::bandwidth':[float],
                                  'FRONTEND::tuner_status::sample_rate':[float],
                                  'FRONTEND::tuner_status::group_id':[str],
                                  'FRONTEND::tuner_status::rf_flow_id':[str],
                                  'FRONTEND::tuner_status::enabled':[bool]}
    FE_tuner_status_fields_opt = {'FRONTEND::tuner_status::bandwidth_tolerance':[float],
                                  'FRONTEND::tuner_status::sample_rate_tolerance':[float],
                                  'FRONTEND::tuner_status::complex':[bool],
                                  'FRONTEND::tuner_status::gain':[float],
                                  'FRONTEND::tuner_status::agc':[bool],
                                  'FRONTEND::tuner_status::valid':[bool],
                                  'FRONTEND::tuner_status::available_frequency':[str],
                                  'FRONTEND::tuner_status::available_bandwidth':[str],
                                  'FRONTEND::tuner_status::available_gain':[str],
                                  'FRONTEND::tuner_status::available_sample_rate':[str],
                                  'FRONTEND::tuner_status::reference_source':[int,long],
                                  'FRONTEND::tuner_status::output_format':str,
                                  'FRONTEND::tuner_status::output_multicast':str,
                                  'FRONTEND::tuner_status::output_vlan':[int,long],
                                  'FRONTEND::tuner_status::output_port':[int,long],
                                  'FRONTEND::tuner_status::decimation':[int,long],
                                  'FRONTEND::tuner_status::tuner_number':[int,long]}
    
    # get lists of all methods/functions defined in analog/digital tuner idl
    analog_tuner_idl = filter(lambda x: x[0]!='_', TunerControl_idl._0_FRONTEND._objref_AnalogTuner.__methods__)
    digital_tuner_idl = filter(lambda x: x[0]!='_', TunerControl_idl._0_FRONTEND._objref_DigitalTuner.__methods__)
    
    # map data types to DataSink port names
    port_map = {'dataShort':'shortIn',
                'dataFloat':'floatIn',
                'dataUlong':'uLongIn',
                'dataDouble':'doubleIn',
                'dataUshort':'ushortIn',
                'dataLong':'longIn',
                'dataUlongLong':'ulonglongIn',
                'dataLongLong':'longlongIn',
                'dataOctet':'octetIn',
                'dataXML':'xmlIn',
                'dataChar':'charIn',
                'dataFile':'fileIn'}
    
    def devicePreLaunch(self):
        pass
    def devicePostLaunch(self):
        pass
    
    def devicePreRelease(self):
        pass
    def devicePostRelease(self):
        pass
    
    def getToBasicState(self, execparams={}, configure={}, initialize=True):
        ''' Function used to launch device before each test case
            With no arguments, uses execparams defined in global DEVICE_INFO['execparams'] dict,
            configures props with values from prf, and initializes device.
            If specified, execparams overrides those specified in DEVICE_INFO dict, and configure
            overrides those specified in the prf.
            Add special start-up commands for your device to deviceStartup() function
        '''
        if not execparams:
            execparams = self.getPropertySet(kinds=('execparam',), modes=('readwrite', 'writeonly'), includeNil=False)
            execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])
            execparams['DEBUG_LEVEL'] = DEBUG_LEVEL
            #Add custom execparams here
            for param,val in DEVICE_INFO['execparams'].items():
                execparams[param] = val
                
        ### device-specific pre-launch commands
        self.devicePreLaunch()
        
        print 'Launching device --',DEVICE_INFO['SPD']
        print '\texecparams:',str(execparams)
        print '\tconfigure:',str(configure)
        print '\tinitialize:',str(initialize)

        try:
            # new method, use in versions >= 1.9
            self.dut = sb.launch(DEVICE_INFO['SPD'],execparams=execparams,configure=configure,initialize=initialize)
        except:
            # deprecated, use in 1.8.x versions
            self.dut = sb.Component(DEVICE_INFO['SPD'],execparams=execparams,configure=configure,initialize=initialize) 
        
        self.dut_ref = self.dut.ref._narrow(CF.Device)
        
        ### device-specific post-launch commands
        self.devicePostLaunch()

    def getToShutdownState(self):
        ''' Function used to release device after each test case
            Add special shut-down commands for your device to deviceShutdown() function
        '''
        
        ### device-specific pre-release commands
        self.devicePreRelease()
        
        if self.dut_ref:
            self.dut_ref = None
        if self.dut:
            self.dut.releaseObject()
            self.dut = None
            
        ### device-specific post-release commands
        self.devicePostRelease()

    def test_FRONTEND(self):
        self.testReport.append('Running FRONTEND Test')
        try:
            self.FRONTEND_1() # Generic
            if self.device_discovery['RX'] > 0: 
                self.FRONTEND_2() # RX
            if self.device_discovery['RX_DIGITIZER'] > 0: 
                self.FRONTEND_3() # RX_DIGITIZER
            if self.device_discovery['CHANNELIZER'] > 0: 
                self.FRONTEND_4() # CHANNELIZER
            if self.device_discovery['RX_DIGITIZER_CHANNELIZER'] > 0: 
                self.FRONTEND_5() # RX_DIGITIZER_CHANNELIZER
            if self.device_discovery['TX'] > 0: 
                self.FRONTEND_6() # TX
        except Exception, e:
            self.testReport.append('FRONTEND Test - terminated due to exception: %s - %s'%(e.__class__.__name__,e))
            self.printTestReport()
            self.getToShutdownState()
            raise
        self.testReport.append('\nFRONTEND Test - Completed')
        self.printTestReport()     

    def FRONTEND_1(self):
        self.testReport.append('\nFRONTEND Test 1 - General')
        self.FRONTEND_1_1_ExternalsTest()
        self.FRONTEND_1_2_DiscoveryTest()
        self.testReport.append('\nFRONTEND Test 1 - Completed')

    def FRONTEND_2(self):
        self.testReport.append('\nFRONTEND Test 2 - RX')
        self.FRONTEND_2_1_BasicAllocation()
        self.FRONTEND_2_2_AdvancedAllocation()
        self.FRONTEND_2_3_TunerControl()
        self.FRONTEND_2_4_DataFlow()
        self.FRONTEND_2_5_TunerStatusProperties()
        self.testReport.append('\nFRONTEND Test 2 - Completed')

    def FRONTEND_3(self):
        self.testReport.append('\nFRONTEND Test 3 - RX_DIGITIZER')
        self.FRONTEND_3_1_BasicAllocation()
        self.FRONTEND_3_2_AdvancedAllocation()
        self.FRONTEND_3_3_TunerControl()
        self.FRONTEND_3_4_DataFlow()
        self.FRONTEND_3_5_TunerStatusProperties()
        self.testReport.append('\nFRONTEND Test 3 - Completed')

    def FRONTEND_4(self):
        # TODO - need to create data source to connect to input of CHANNELIZER for all tests
        self.testReport.append('\nFRONTEND Test 4 - CHANNELIZER - Not Implemented')
        #self.testReport.append('\nFRONTEND Test 4 - CHANNELIZER')
        #self.FRONTEND_4_1_BasicAllocation()
        #self.FRONTEND_4_2_AdvancedAllocation()
        #self.FRONTEND_4_3_TunerControl()
        #self.FRONTEND_4_4_DataFlow()
        #self.FRONTEND_4_5_TunerStatusProperties()
        #self.testReport.append('\nFRONTEND Test 4 - Completed')

    def FRONTEND_5(self):
        self.testReport.append('\nFRONTEND Test 5 - RX_DIGITIZER_CHANNELIZER')
        self.FRONTEND_5_1_BasicAllocation()
        self.FRONTEND_5_2_AdvancedAllocation()
        self.FRONTEND_5_3_TunerControl()
        self.FRONTEND_5_4_DataFlow()
        self.FRONTEND_5_5_TunerStatusProperties()
        self.testReport.append('\nFRONTEND Test 5 - Completed')

    def FRONTEND_6(self):
        self.testReport.append('\nFRONTEND Test 6 - TX - Not implemented!')
        #self.testReport.append('\nFRONTEND Test 6 - TX')
        #self.testReport.append('\nFRONTEND Test 6 - Completed')

    def FRONTEND_1_1_ExternalsTest(self):
        self.testReport.append('\nTest 1.1 - External Information')
        self.getToBasicState()
        
        #Test 1.1.1 - Verify device_kind property
        props = self.dut.query([])
        props = properties.props_to_dict(props)
        pp(props)
        self.check(props.has_key('DCE:cdc5ee18-7ceb-4ae6-bf4c-31f983179b4d'), True, 'Has device_kind property')
        self.check(props['DCE:cdc5ee18-7ceb-4ae6-bf4c-31f983179b4d'], 'FRONTEND::TUNER', 'device_kind = FRONTEND::TUNER')
        
        #Test 1.1.2 - Verify that there is a device_model property
        self.check(props.has_key('DCE:0f99b2e4-9903-4631-9846-ff349d18ecfb'), True, 'Has device_model property')
        
        #Test 1.1.3 - Verify that there is a FRONTEND Status property
        self.check(props.has_key('FRONTEND::tuner_status'), True, 'Has tuner_status property')
        # check for required fields
        #pp(props['FRONTEND::tuner_status'])
        if (len(props['FRONTEND::tuner_status']) == 0):
                print '\nERROR - tuner_status is empty. Check that the unit test is configured to reach the target device hardware.\n'
                self.check(False,True,'\nERROR - tuner_status is empty. Check that the unit test is configured to reach the target device hardware.',throwOnFailure=True)
  
        for field in self.FE_tuner_status_fields_req:
            self.check(props['FRONTEND::tuner_status'][-1].has_key(field), True, 'tuner_status has %s required field'%field)
        
        #Test 1.1.4 - Verify there is a digitalTuner (or analogTuner) port
        #Attempt to get both ports and compare if None, then xor (^) the boolean result
        reason = 'both'
        try:
            DigitalTuner = self.dut.getPort('DigitalTuner_in')
        except:
            DigitalTuner= None
            reason = 'analog'
        try:
            AnalogTuner = self.dut.getPort('AnalogTuner_in')
        except:
            AnalogTuner = None
            reason = 'digital'
        if (DigitalTuner==None) and (AnalogTuner==None):
            reason = 'none'
        self.check( (DigitalTuner== None)^(AnalogTuner== None), True, 'Has an analog or digital tuner input port (%s)'%reason)
             
        self.getToShutdownState()
        self.testReport.append('Completed Test 1.1')

    def FRONTEND_1_2_DiscoveryTest(self):
        self.testReport.append('\nTest 1.2 - Discover Capabilities')
        self.getToBasicState()
        
        #Test 1.2 - Count # of each tuner type
        props = self.dut.query([])
        props = properties.props_to_dict(props)
        for tuner in props['FRONTEND::tuner_status']:
            if tuner['FRONTEND::tuner_status::tuner_type'] in self.device_discovery.keys():
                self.device_discovery[tuner['FRONTEND::tuner_status::tuner_type']] += 1
            else:
                self.device_discovery['UNKNOWN'] += 1
                
        #print '------------------------------------------------------'
        #pp(self.device_discovery)
        #print '------------------------------------------------------'
        
        for k,v in self.device_discovery.items():
            if v > 0:
                self.check(True, True, 'Found %s %s'%(v,k))
                
        self.getToShutdownState()
        self.testReport.append('Completed Test 1.2')    
        
    def FRONTEND_2_1_BasicAllocation(self):
        self.testReport.append('\nTest 2.1 - Basic RX Allocation')
        self.getToBasicState()
        
        self._checkBasicAllocation(tuner_type='RX')
        
        self.getToShutdownState()
        self.testReport.append('Completed Test 2.1')

    def FRONTEND_2_2_AdvancedAllocation(self):
        self.testReport.append('\nTest 2.2 - Advanced RX Allocation')
        self.getToBasicState()
        
        #2.2 Advanced Allocation Test
        rx = self._generateRX()
        self._checkInvalidAllocations(tuner=rx,ttype='RX')
        
        rx = self._generateRX()
        self._checkListenerAllocation(tuner=rx, ttype='RX')
        
        rx = self._generateRX()
        self._checkFrequencyBoundsAllocation(tuner=rx, ttype='RX', low=DEVICE_INFO['RX']['CF_MIN'], high=DEVICE_INFO['RX']['CF_MAX'])
        
        rx = self._generateRX()
        self._checkBandwidthBoundsAllocation(tuner=rx, ttype='RX', low=DEVICE_INFO['RX']['BW_MIN'], high=DEVICE_INFO['RX']['BW_MAX'])
        
        self.getToShutdownState()
        self.testReport.append('Completed Test 2.2')

    def FRONTEND_2_3_TunerControl(self):
        self.testReport.append('\nTest 2.3 - DigitalTuner port Tuner Control')
        self.getToBasicState()
        
        AnalogTuner = self.dut.getPort('AnalogTuner_in')
        AnalogTuner._narrow(FRONTEND.FrontendTuner)

        rx = self._generateRX()
        rxListener = self._generateListener(rx)
        
        self._checkTunerPortIDL(ttype='RX',tuner_info=DEVICE_INFO['RX'], controller=rx, listener=rxListener, analog_tuner=AnalogTuner)
        
        self.getToShutdownState()
        self.testReport.append('Completed Test 2.3')
        
    def FRONTEND_2_4_DataFlow(self):
        self.testReport.append('\nTest 2.4 - Data Flow')
        self.getToBasicState()
        
        rx = self._generateRX()
        rx['CF'] = DEVICE_INFO['RX']['CF_MIN'] + DEVICE_INFO['RX']['BW_MIN']
        rx['BW'] = DEVICE_INFO['RX']['BW_MIN']
        rxListener = self._generateListener(rx)
        self._checkAnalogDataFlow(ttype='RX', controller=rx, listener=rxListener)
        
        self.getToShutdownState()
        self.testReport.append('Completed Test 2.4')
        
    def FRONTEND_2_5_TunerStatusProperties(self):
        self.testReport.append('\nTest 2.5 - Tuner Status Properties')
        self.getToBasicState()
        
        AnalogTuner = self.dut.getPort('AnalogTuner_in')
        AnalogTuner._narrow(FRONTEND.FrontendTuner)
        
        rx = self._generateRX()
        rxListener1 = self._generateListener(rx)
        rxListener2 = self._generateListener(rx)
        self._checkFrontEndTunerStatus(controller=rx,
                                       listener1=rxListener1,
                                       listener2=rxListener2,
                                       analog_tuner=AnalogTuner)
            
        self.getToShutdownState()
        self.testReport.append('Completed Test 2.5')

    def FRONTEND_3_1_BasicAllocation(self):
        self.testReport.append('\nTest 3.1 - Basic RX_DIG Allocation')
        self.getToBasicState()
        
        self._checkBasicAllocation(tuner_type='RX_DIGITIZER')
        
        self.getToShutdownState()
        self.testReport.append('Completed Test 3.1')
        
    def FRONTEND_3_2_AdvancedAllocation(self):
        self.testReport.append('\nTest 3.2 - Advanced RX_DIGITIZER Allocation')
        self.getToBasicState()
        
        #3.2 Advanced Allocation Test
        rd = self._generateRD()
        self._checkInvalidAllocations(tuner=rd,ttype='RX_DIGITIZER')
        
        rd = self._generateRD()
        self._checkListenerAllocation(tuner=rd, ttype='RX_DIGITIZER')
        
        rd = self._generateRD()
        self._checkFrequencyBoundsAllocation(tuner=rd, ttype='RX_DIGITIZER', low=DEVICE_INFO['RX_DIGITIZER']['CF_MIN'], high=DEVICE_INFO['RX_DIGITIZER']['CF_MAX'])
        
        rd = self._generateRD()
        self._checkBandwidthBoundsAllocation(tuner=rd, ttype='RX_DIGITIZER', low=DEVICE_INFO['RX_DIGITIZER']['BW_MIN'], high=DEVICE_INFO['RX_DIGITIZER']['BW_MAX'])
        
        rd = self._generateRD()
        self._checkSampleRateBoundsAllocation(tuner=rd, ttype='RX_DIGITIZER', low=DEVICE_INFO['RX_DIGITIZER']['SR_MIN'], high=DEVICE_INFO['RX_DIGITIZER']['SR_MAX'])
        
        self.getToShutdownState()
        self.testReport.append('Completed Test 3.2')

    def FRONTEND_3_3_TunerControl(self):
        self.testReport.append('\nTest 3.3 - DigitalTuner port Tuner Control')
        self.getToBasicState()
        
        DigitalTuner = self.dut.getPort('DigitalTuner_in')
        DigitalTuner._narrow(FRONTEND.FrontendTuner)

        rd = self._generateRD()
        rdListener = self._generateListener(rd)
        
        self._checkTunerPortIDL(ttype='RX_DIGITIZER',tuner_info=DEVICE_INFO['RX_DIGITIZER'], controller=rd, listener=rdListener, digital_tuner=DigitalTuner)
        
        self.getToShutdownState()
        self.testReport.append('Completed Test 3.3')
        
    def FRONTEND_3_4_DataFlow(self):
        self.testReport.append('\nTest 3.4 - Data Flow')
        self.getToBasicState()
        
        rd = self._generateRD()
        rd['CF'] = DEVICE_INFO['RX_DIGITIZER']['CF_MIN'] + DEVICE_INFO['RX_DIGITIZER']['BW_MIN']
        rd['BW'] = DEVICE_INFO['RX_DIGITIZER']['BW_MIN']
        rd['SR'] = DEVICE_INFO['RX_DIGITIZER']['SR_MIN']
        rdListener1 = self._generateListener(rd)
        rdListener2 = self._generateListener(rd)
        self._checkDigitalDataFlow(ttype='RX_DIGITIZER', controller=rd, listener1=rdListener1, listener2=rdListener2)
        
        self.getToShutdownState()
        self.testReport.append('Completed Test 3.4')
        
    def FRONTEND_3_5_TunerStatusProperties(self):
        self.testReport.append('\nTest 3.5 - Tuner Status Properties')
        self.getToBasicState()
        
        DigitalTuner = self.dut.getPort('DigitalTuner_in')
        DigitalTuner._narrow(FRONTEND.FrontendTuner)
        
        rd = self._generateRD()
        rdListener1 = self._generateListener(rd)
        rdListener2 = self._generateListener(rd)
        self._checkFrontEndTunerStatus(controller=rd,
                                       listener1=rdListener1,
                                       listener2=rdListener2,
                                       digital_tuner=DigitalTuner)
            
        self.getToShutdownState()
        self.testReport.append('Completed Test 3.5')
        
    def FRONTEND_4_1_BasicAllocation(self):
        self.testReport.append('\nTest 4.1 - Basic CHANNELIZER Allocation')
        self.getToBasicState()
        
        # TODO - need to connect to input of CHAN
        
        self._checkBasicAllocation(tuner_type='CHANNELIZER')
        
        self.getToShutdownState()
        self.testReport.append('Completed Test 4.1')

    def FRONTEND_4_2_AdvancedAllocation(self):
        self.testReport.append('\nTest 4.2 - Advanced CHANNELIZER + DDC Allocation')
        self.getToBasicState()
        
        # TODO - need to connect to input of CHAN
                
        #4.2 Advanced Allocation Test
        
        # Check bad CHAN allocations
        chan = self._generateCHAN()
        self._checkInvalidAllocations(tuner=chan, ttype='CHANNELIZER')
        
        # Check bad DDC allocations
        chan = self._generateCHAN()
        chan_alloc_id = chan['ALLOC_ID']
        chanAlloc = self._generateAlloc(chan)
        self.dut_ref.allocateCapacity(chanAlloc)
        ddc = self._generateDDC(chan)
        self._checkInvalidAllocations(tuner=ddc, ttype='DDC',parent_alloc_id=chan_alloc_id)
        self.dut_ref.deallocateCapacity(chanAlloc)
        
        # Verify failure on alloc DDC prior to CHAN
        chan = self._generateCHAN()
        ddc = self._generateDDC(chan)
        ddcAlloc = self._generateAlloc(ddc)
        self.check(self.dut_ref.allocateCapacity(ddcAlloc), False, 'Allocate DDC prior to CHANNELIZER allocation check')
        self.dut_ref.deallocateCapacity(ddcAlloc)
        
        # Verify DDC is deallocated when CAHNNELIZER is deallocated
        chan = self._generateCHAN()
        chanAlloc = self._generateAlloc(chan)
        self.dut_ref.allocateCapacity(chanAlloc)
        ddc = self._generateDDC(chan)
        ddcAlloc = self._generateAlloc(ddc)
        self.dut_ref.allocateCapacity(ddcAlloc)
        self.check(self.dut_ref.deallocateCapacity(chanAlloc), True, 'Deallocated controller CHANNELIZER prior to DDC')
        has_ddc = self._tunerStatusHasAllocId(ddc['ALLOC_ID'])
        self.check(has_ddc, False, 'DDC deallocated  as result of CHANNELIZER deallocation (based on tuner status properties)')
        self.dut_ref.deallocateCapacity(ddcAlloc)
        
        # Verify failure on attempts to allocate listener CHANNELIZER
        # first via listener allocation
        chan = self._generateCHAN()
        chanAlloc = self._generateAlloc(chan)
        self.dut_ref.allocateCapacity(chanAlloc)
        chanListener = self._generateListener(chan)
        chanListenerAlloc = self._generateListenerAlloc(chanListener)
        # alloc and check for exception
        try:
            retval = self.dut_ref.allocateCapacity(chanListenerAlloc)
        except CF.Device.InvalidCapacity:
            self.check(True, True, 'Verify listener CHANNELIZER allocation attempt via listener alloc produces InvalidCapacity exception')
        except Exception, e:
            self.check(False, True, 'Verify listener CHANNELIZER allocation attempt via listener alloc produces InvalidCapacity exception (produces %s exception instead)'%(e.__class__.__name__))
        else:
            self.check(False, True, 'Verify listener CHANNELIZER allocation attempt via listener alloc produces InvalidCapacity exception (returns %s instead)'%(retval))
        self.dut_ref.deallocateCapacity(chanListenerAlloc)
        # second via tuner allocation
        chanListener = copy.deepcopy(chan)
        chan['CONTROL'] = False
        chanListenerAlloc = self._generateListenerAlloc(chanListener)
        # alloc and check for exception
        try:
            retval = self.dut_ref.allocateCapacity(chanListenerAlloc)
        except CF.Device.InvalidCapacity:
            self.check(True, True, 'Verify listener CHANNELIZER allocation attempt via tuner alloc produces InvalidCapacity exception')
        except Exception, e:
            self.check(False, True, 'Verify listener CHANNELIZER allocation attempt via tuner alloc produces InvalidCapacity exception (produces %s exception instead)'%(e.__class__.__name__))
        else:
            self.check(False, True, 'Verify listener CHANNELIZER allocation attempt via tuner alloc produces InvalidCapacity exception (returns %s instead)'%(retval))
        self.dut_ref.deallocateCapacity(chanListenerAlloc)
        self.dut_ref.deallocateCapacity(chanAlloc)
        
        # Check DDC listener allocation
        chan = self._generateCHAN()
        chanAlloc = self._generateAlloc(chan)
        self.dut_ref.allocateCapacity(chanAlloc)
        ddc = self._generateDDC(chan)
        self._checkListenerAllocation(tuner=ddc, ttype='DDC')
        self.dut_ref.deallocateCapacity(chanAlloc)
        
        # Check CHANNELIZER frequency bounds allocation
        chan = self._generateCHAN()
        self._checkFrequencyBoundsAllocation(tuner=chan, ttype='CHANNELIZER', low=DEVICE_INFO['CHANNELIZER']['CF_MIN'], high=DEVICE_INFO['CHANNELIZER']['CF_MAX'])
        
        # Check DDC frequency bounds allocation
        chan = self._generateCHAN()
        chanAlloc = self._generateAlloc(chan)
        self.dut_ref.allocateCapacity(chanAlloc)
        ddc = self._generateDDC(chan)
        low=max(chan['CF']-chan['BW']/2.0,DEVICE_INFO['CHANNELIZER']['CF_MIN'])
        high=min(chan['CF']+chan['BW']/2.0,DEVICE_INFO['CHANNELIZER']['CF_MAX'])
        self._checkFrequencyBoundsAllocation(tuner=ddc, ttype='DDC', low=low, high=high)
        self.dut_ref.deallocateCapacity(chanAlloc)
        
        # Check RDC bandwidth bounds allocation
        chan = self._generateCHAN()
        self._checkBandwidthBoundsAllocation(tuner=chan, ttype='CHANNELIZER', low=DEVICE_INFO['CHANNELIZER']['BW_MIN'], high=DEVICE_INFO['CHANNELIZER']['BW_MAX'])
        
        # Check DDC bandwidth bounds allocation
        chan = self._generateCHAN()
        chanAlloc = self._generateAlloc(chan)
        self.dut_ref.allocateCapacity(chanAlloc)
        ddc = self._generateDDC(chan)
        self._checkBandwidthBoundsAllocation(tuner=ddc, ttype='DDC', low=DEVICE_INFO['DDC']['BW_MIN'], high=DEVICE_INFO['DDC']['BW_MAX'])
        self.dut_ref.deallocateCapacity(chanAlloc)
        
        # Check RDC sample rate bounds allocation
        chan = self._generateCHAN()
        self._checkSampleRateBoundsAllocation(tuner=chan, ttype='CHANNELIZER', low=DEVICE_INFO['CHANNELIZER']['SR_MIN'], high=DEVICE_INFO['CHANNELIZER']['SR_MAX'])
        
        # Check DDC sample rate bounds allocation
        chan = self._generateCHAN()
        chanAlloc = self._generateAlloc(chan)
        self.dut_ref.allocateCapacity(chanAlloc)
        ddc = self._generateDDC(chan)
        self._checkSampleRateBoundsAllocation(tuner=ddc, ttype='DDC', low=DEVICE_INFO['DDC']['SR_MIN'], high=DEVICE_INFO['DDC']['SR_MAX'])
        self.dut_ref.deallocateCapacity(chanAlloc)
        
        self.getToShutdownState()
        self.testReport.append('Completed Test 5.2')

    def FRONTEND_4_3_TunerControl(self):
        self.testReport.append('\nTest 4.3 - DigitalTuner port Tuner Control')
        self.getToBasicState()
        
        # use this stuff for all tests
        DigitalTuner = self.dut.getPort('DigitalTuner_in')
        DigitalTuner._narrow(FRONTEND.FrontendTuner)
        chan = self._generateCHAN()
        #TODO - get input data port reference
        #TODO - create dataSource

        # first, test for channelizer
        #TODO - connection should be made after allocation, which means within the checkTunerPortIDL function...
        #TODO - connect dataSource to input data port
        #dataSource.connectPort(dataXXXX_in_port_obj, uuid.uuid4())
        self._checkTunerPortIDL(ttype='CHANNELIZER',tuner_info=DEVICE_INFO['CHANNELIZER'], controller=chan, digital_tuner=DigitalTuner)
        
        # now for ddc
        chanAlloc = self._generateAlloc(chan)
        self.dut_ref.allocateCapacity(chanAlloc)
        #TODO - connect dataSource to input data port
        #dataSource.connectPort(dataXXXX_in_port_obj, uuid.uuid4())
        ddc = self._generateDDC(chan)
        ddcListener = self._generateListener(ddc)
        ddc_info = copy.deepcopy(DEVICE_INFO['DDC'])
        ddc_info['CF_MIN'] = max(chan['CF']-chan['BW']/2.0,DEVICE_INFO['CHANNELIZER']['CF_MIN'])
        ddc_info['CF_MAX'] = min(chan['CF']+chan['BW']/2.0,DEVICE_INFO['CHANNELIZER']['CF_MAX'])
        self._checkTunerPortIDL(ttype='DDC',tuner_info=ddc_info, controller=ddc, listener=ddcListener, digital_tuner=DigitalTuner)
        self.dut_ref.deallocateCapacity(chanAlloc)
        
        self.getToShutdownState()
        self.testReport.append('Completed Test 4.3')
        
    def FRONTEND_4_4_DataFlow(self):
        self.testReport.append('\nTest 4.4 - Data Flow')
        self.getToBasicState()
        
        # use this stuff for all tests
        chan = self._generateCHAN()
        chan['CF'] = DEVICE_INFO['CHANNELIZER']['CF_MIN'] + DEVICE_INFO['CHANNELIZER']['BW_MIN']
        chan['BW'] = DEVICE_INFO['CHANNELIZER']['BW_MIN']
        chan['SR'] = DEVICE_INFO['CHANNELIZER']['SR_MIN']
        #TODO - get input data port reference
        #TODO - create dataSource

        # first, test for channelizer
        #TODO - connection should be made after allocation, which means within the checkTunerPortIDL function...
        #TODO - connect dataSource to input data port
        #dataSource.connectPort(dataXXXX_in_port_obj, uuid.uuid4())
        self._checkDigitalDataFlow(ttype='CHANNELIZER', controller=rdc, listener1=rdcListener1, listener2=rdcListener2)
        
        # now do DDC
        chanAlloc = self._generateAlloc(chan)
        self.dut_ref.allocateCapacity(chanAlloc)
        #TODO - connect dataSource to input data port
        #dataSource.connectPort(dataXXXX_in_port_obj, uuid.uuid4())
        
        ddc = self._generateDDC(chan)
        ddc['CF'] = chan['CF']
        ddc['BW'] = DEVICE_INFO['DDC']['BW_MIN']
        ddc['SR'] = DEVICE_INFO['DDC']['SR_MIN']
        ddcListener1 = self._generateListener(ddc)
        ddcListener2 = self._generateListener(ddc)
        self._checkDigitalDataFlow(ttype='DDC', controller=ddc, listener1=ddcListener1, listener2=ddcListener2)
        self.dut_ref.deallocateCapacity(chanAlloc)
        
        self.getToShutdownState()
        self.testReport.append('Completed Test 4.4')

    def FRONTEND_4_5_TunerStatusProperties(self):
        self.testReport.append('\nTest 4.5 - Tuner Status Properties')
        self.getToBasicState()
        
        # use this stuff for all tests
        DigitalTuner = self.dut.getPort('DigitalTuner_in')
        DigitalTuner._narrow(FRONTEND.FrontendTuner)
        chan = self._generateCHAN()
        #TODO - get input data port reference
        #TODO - create dataSource

        # first, test for channelizer
        #TODO - connection should be made after allocation, which means within the checkTunerPortIDL function...
        #TODO - connect dataSource to input data port
        #dataSource.connectPort(dataXXXX_in_port_obj, uuid.uuid4())
        
        # now run the tests for CHANNELIZER
        self._checkFrontEndTunerStatus(controller=chan,
                                       digital_tuner=DigitalTuner)
        
        # Second, do DDC (we need a CHAN to piggy-back on)
        chanAlloc = self._generateAlloc(chan)
        self.dut_ref.allocateCapacity(chanAlloc)
        #TODO - connect dataSource to input data port
        #dataSource.connectPort(dataXXXX_in_port_obj, uuid.uuid4())
        
        # now create alloc structs for a controller and two listeners (DDCs)
        ddc = self._generateDDC(chan)
        ddcListener1 = self._generateListener(ddc)
        ddcListener2 = self._generateListener(ddc)
        
        # now run the tests for DDC
        self._checkFrontEndTunerStatus(controller=ddc,
                                       listener1=ddcListener1,
                                       listener2=ddcListener2,
                                       digital_tuner=DigitalTuner)
        
        # cleanup after test    
        self.dut_ref.deallocateCapacity(chanAlloc)
        
        self.getToShutdownState()
        self.testReport.append('Completed Test 4.5')
    
    def FRONTEND_5_1_BasicAllocation(self):
        self.testReport.append('\nTest 5.1 - Basic RDC + DDC Allocation')
        self.getToBasicState()
        
        self._checkBasicAllocation(tuner_type='RX_DIGITIZER_CHANNELIZER')
        
        self.getToShutdownState()
        self.testReport.append('Completed Test 5.1')

    def FRONTEND_5_2_AdvancedAllocation(self):
        self.testReport.append('\nTest 5.2 - Advanced RDC + DDC Allocation')
        self.getToBasicState()
        
        #5.2 Advanced Allocation Test
        
        # Check bad RDC allocations
        rdc = self._generateRDC()
        self._checkInvalidAllocations(tuner=rdc, ttype='RX_DIGITIZER_CHANNELIZER')
        
        # Check bad DDC allocations
        rdc = self._generateRDC()
        rdc_alloc_id = rdc['ALLOC_ID']
        rdcAlloc = self._generateAlloc(rdc)
        self.dut_ref.allocateCapacity(rdcAlloc)
        ddc = self._generateDDC(rdc)
        self._checkInvalidAllocations(tuner=ddc, ttype='DDC',parent_alloc_id=rdc_alloc_id)
        self.dut_ref.deallocateCapacity(rdcAlloc)
        
        # Verify failure on alloc DDC prior to RDC
        rdc = self._generateRDC()
        ddc = self._generateDDC(rdc)
        ddcAlloc = self._generateAlloc(ddc)
        self.check(self.dut_ref.allocateCapacity(ddcAlloc), False, 'Allocate DDC prior to RX_DIGITIZER_CHANNELIZER allocation check')
        self.dut_ref.deallocateCapacity(ddcAlloc)
        
        # Verify DDC is deallocated when RDC is deallocated
        rdc = self._generateRDC()
        rdcAlloc = self._generateAlloc(rdc)
        self.dut_ref.allocateCapacity(rdcAlloc)
        ddc = self._generateDDC(rdc)
        ddcAlloc = self._generateAlloc(ddc)
        self.dut_ref.allocateCapacity(ddcAlloc)
        self.check(self.dut_ref.deallocateCapacity(rdcAlloc), True, 'Deallocated controller RX_DIGITIZER_CHANNELIZER prior to DDC')
        has_ddc = self._tunerStatusHasAllocId(ddc['ALLOC_ID'])
        self.check(has_ddc, False, 'DDC deallocated as result of RX_DIGITIZER_CHANNELIZER deallocation (based on tuner status properties)')
        self.dut_ref.deallocateCapacity(ddcAlloc)
        
        # Check RDC listener allocation
        rdc = self._generateRDC()
        self._checkListenerAllocation(tuner=rdc, ttype='RX_DIGITIZER_CHANNELIZER')
        
        # Check DDC listener allocation
        rdc = self._generateRDC()
        rdcAlloc = self._generateAlloc(rdc)
        self.dut_ref.allocateCapacity(rdcAlloc)
        ddc = self._generateDDC(rdc)
        self._checkListenerAllocation(tuner=ddc, ttype='DDC')
        self.dut_ref.deallocateCapacity(rdcAlloc)
        
        # Check RDC frequency bounds allocation
        rdc = self._generateRDC()
        self._checkFrequencyBoundsAllocation(tuner=rdc, ttype='RX_DIGITIZER_CHANNELIZER', low=DEVICE_INFO['RX_DIGITIZER_CHANNELIZER']['CF_MIN'], high=DEVICE_INFO['RX_DIGITIZER_CHANNELIZER']['CF_MAX'])
        
        # Check DDC frequency bounds allocation
        rdc = self._generateRDC()
        rdcAlloc = self._generateAlloc(rdc)
        self.dut_ref.allocateCapacity(rdcAlloc)
        ddc = self._generateDDC(rdc)
        low=max(rdc['CF']-rdc['BW']/2.0,DEVICE_INFO['RX_DIGITIZER_CHANNELIZER']['CF_MIN'])
        high=min(rdc['CF']+rdc['BW']/2.0,DEVICE_INFO['RX_DIGITIZER_CHANNELIZER']['CF_MAX'])
        self._checkFrequencyBoundsAllocation(tuner=ddc, ttype='DDC', low=low, high=high)
        self.dut_ref.deallocateCapacity(rdcAlloc)
        
        # Check RDC bandwidth bounds allocation
        rdc = self._generateRDC()
        self._checkBandwidthBoundsAllocation(tuner=rdc, ttype='RX_DIGITIZER_CHANNELIZER', low=DEVICE_INFO['RX_DIGITIZER_CHANNELIZER']['BW_MIN'], high=DEVICE_INFO['RX_DIGITIZER_CHANNELIZER']['BW_MAX'])
        
        # Check DDC bandwidth bounds allocation
        rdc = self._generateRDC()
        rdcAlloc = self._generateAlloc(rdc)
        self.dut_ref.allocateCapacity(rdcAlloc)
        ddc = self._generateDDC(rdc)
        self._checkBandwidthBoundsAllocation(tuner=ddc, ttype='DDC', low=DEVICE_INFO['DDC']['BW_MIN'], high=DEVICE_INFO['DDC']['BW_MAX'])
        self.dut_ref.deallocateCapacity(rdcAlloc)
        
        # Check RDC sample rate bounds allocation
        rdc = self._generateRDC()
        self._checkSampleRateBoundsAllocation(tuner=rdc, ttype='RX_DIGITIZER_CHANNELIZER', low=DEVICE_INFO['RX_DIGITIZER_CHANNELIZER']['SR_MIN'], high=DEVICE_INFO['RX_DIGITIZER_CHANNELIZER']['SR_MAX'])
        
        # Check DDC sample rate bounds allocation
        rdc = self._generateRDC()
        rdcAlloc = self._generateAlloc(rdc)
        self.dut_ref.allocateCapacity(rdcAlloc)
        ddc = self._generateDDC(rdc)
        self._checkSampleRateBoundsAllocation(tuner=ddc, ttype='DDC', low=DEVICE_INFO['DDC']['SR_MIN'], high=DEVICE_INFO['DDC']['SR_MAX'])
        self.dut_ref.deallocateCapacity(rdcAlloc)
        
        self.getToShutdownState()
        self.testReport.append('Completed Test 5.2')

    def FRONTEND_5_3_TunerControl(self):
        self.testReport.append('\nTest 5.3 - DigitalTuner port Tuner Control')
        self.getToBasicState()
        
        DigitalTuner = self.dut.getPort('DigitalTuner_in')
        DigitalTuner._narrow(FRONTEND.FrontendTuner)

        # first, test for channelizer
        rdc = self._generateRDC()
        rdcListener = self._generateListener(rdc)
        self._checkTunerPortIDL(ttype='RX_DIGITIZER_CHANNELIZER',tuner_info=DEVICE_INFO['RX_DIGITIZER_CHANNELIZER'], controller=rdc, listener=rdcListener, digital_tuner=DigitalTuner)
        
        # now for ddc
        rdc = self._generateRDC()
        rdcAlloc = self._generateAlloc(rdc)
        self.dut_ref.allocateCapacity(rdcAlloc)
        ddc = self._generateDDC(rdc)
        ddcListener = self._generateListener(ddc)
        ddc_info = copy.deepcopy(DEVICE_INFO['DDC'])
        ddc_info['CF_MIN'] = max(rdc['CF']-rdc['BW']/2.0,DEVICE_INFO['RX_DIGITIZER_CHANNELIZER']['CF_MIN'])
        ddc_info['CF_MAX'] = min(rdc['CF']+rdc['BW']/2.0,DEVICE_INFO['RX_DIGITIZER_CHANNELIZER']['CF_MAX'])
        self._checkTunerPortIDL(ttype='DDC',tuner_info=ddc_info, controller=ddc, listener=ddcListener, digital_tuner=DigitalTuner)
        self.dut_ref.deallocateCapacity(rdcAlloc)
        
        self.getToShutdownState()
        self.testReport.append('Completed Test 5.3')
        
    def FRONTEND_5_4_DataFlow(self):
        self.testReport.append('\nTest 5.4 - Data Flow')
        self.getToBasicState()
        
        # first, do RDC
        rdc = self._generateRDC()
        rdc['CF'] = DEVICE_INFO['RX_DIGITIZER_CHANNELIZER']['CF_MIN'] + DEVICE_INFO['RX_DIGITIZER_CHANNELIZER']['BW_MIN']
        rdc['BW'] = DEVICE_INFO['RX_DIGITIZER_CHANNELIZER']['BW_MIN']
        rdc['SR'] = DEVICE_INFO['RX_DIGITIZER_CHANNELIZER']['SR_MIN']
        rdcListener1 = self._generateListener(rdc)
        rdcListener2 = self._generateListener(rdc)
        self._checkDigitalDataFlow(ttype='RX_DIGITIZER_CHANNELIZER', controller=rdc, listener1=rdcListener1, listener2=rdcListener2)
        
        # now do DDC
        rdc = self._generateRDC()
        rdcAlloc = self._generateAlloc(rdc)
        self.dut_ref.allocateCapacity(rdcAlloc)
        ddc = self._generateDDC(rdc)
        ddc['CF'] = rdc['CF']
        ddc['BW'] = DEVICE_INFO['DDC']['BW_MIN']
        ddc['SR'] = DEVICE_INFO['DDC']['SR_MIN']
        ddcListener1 = self._generateListener(ddc)
        ddcListener2 = self._generateListener(ddc)
        self._checkDigitalDataFlow(ttype='DDC', controller=ddc, listener1=ddcListener1, listener2=ddcListener2)
        self.dut_ref.deallocateCapacity(rdcAlloc)
        
        self.getToShutdownState()
        self.testReport.append('Completed Test 5.4')
        
    def FRONTEND_5_5_TunerStatusProperties(self):
        self.testReport.append('\nTest 5.5 - Tuner Status Properties')
        self.getToBasicState()
        
        DigitalTuner = self.dut.getPort('DigitalTuner_in')
        DigitalTuner._narrow(FRONTEND.FrontendTuner)
        
        # first, do RDC - create alloc structs for a controller and two listeners
        rdc = self._generateRDC()
        rdcListener1 = self._generateListener(rdc)
        rdcListener2 = self._generateListener(rdc)
        self._checkFrontEndTunerStatus(controller=rdc,
                                       listener1=rdcListener1,
                                       listener2=rdcListener2,
                                       digital_tuner=DigitalTuner)
        
        # Second, do DDC (we need an RDC to piggy-back on)
        rdc = self._generateRDC()
        rdcAlloc = self._generateAlloc(rdc)
        self.dut_ref.allocateCapacity(rdcAlloc)
        
        # now create alloc structs for a controller and two listeners (DDCs)
        ddc = self._generateDDC(rdc)
        ddcListener1 = self._generateListener(ddc)
        ddcListener2 = self._generateListener(ddc)
        self._checkFrontEndTunerStatus(controller=ddc,
                                       listener1=ddcListener1,
                                       listener2=ddcListener2,
                                       digital_tuner=DigitalTuner)
        
        # cleanup after test    
        self.dut_ref.deallocateCapacity(rdcAlloc)
        
        self.getToShutdownState()
        self.testReport.append('Completed Test 5.5')

    def printTestReport(self):
        print 'TEST REPORT:'
        print '-----------------------------------------------------------------------------'
        for line in self.testReport:
            print line

    #Helpers
    def check(self, A, B, message, throwOnFailure=False, silentFailure=False, silentSuccess=False, indent_width=0, failureMsg='FAILURE', successMsg=u'\u2714'):
        # successMsg suggestions: PASS, YES, u'\u2714' (check mark)
        # failureMsg suggestions: FAIL, NO, u'\u2718' (x mark)
        if A == B:
            if silentSuccess == False:
                self.testReport.append(self._buildRow(message,successMsg,indent_width))
            return True # success!
        else:
            if silentFailure == False:
                self.testReport.append(self._buildRow(message,failureMsg,indent_width))
            if throwOnFailure:
                self.testReport.append('Terminal error, stopping current test...')
                self.getToShutdownState()
                self.assertFalse('Ending test')
            return False # failure!
        
    def checkAlmostEqual(self, A, B, message, throwOnFailure=False, silentFailure=False, silentSuccess=False, indent_width=0, failureMsg='FAILURE', successMsg=u'\u2714', places=7):
        # successMsg suggestions: PASS, YES, u'\u2714' (check mark)
        # failureMsg suggestions: FAIL, NO, u'\u2718' (x mark)
        if round(B-A, places) == 0:
            if silentSuccess == False:
                self.testReport.append(self._buildRow(message,successMsg,indent_width))
            return True # success!
        else:
            if silentFailure == False:
                self.testReport.append(self._buildRow(message,failureMsg,indent_width))
            if throwOnFailure:
                self.testReport.append('Terminal error, stopping current test...')
                self.getToShutdownState()
                self.assertFalse('Ending test')
            return False # failure!

    def _buildRow(self, lhs, rhs, indent_width=0, filler='.', len_total=80, rhs_width=4, firstLine=True):
        ''' builds a row (or multiple rows, if required) that fit within len_total columns
            format: <indent><lhs><at least 3 filler characters><rhs>
            -will be split over multiple lines if necessary
            -pads rhs text to number of characters specified by rhs_width using spaces
            -truncates rhs text to number of characters specified by rhs_width
        '''
        min_filler = 3
        filler_width = len_total - (indent_width + len(lhs) + rhs_width)
        if filler_width >= min_filler:
            return (' '*indent_width + lhs + filler*filler_width + rhs)[0:len_total]
        else:
            lhs1_width = len_total - (indent_width + min_filler + rhs_width)
            idx = lhs.rfind(' ',0,lhs1_width) # try to split on a space
            if idx == -1:
                idx = (lhs+' ').find(' ') # split at first space, if any, or take the whole string
            line1 = ' '*indent_width + lhs[:idx]
            if firstLine:
                indent_width += 4
            line2 = self._buildRow(lhs[idx:], rhs, indent_width, filler, len_total, rhs_width, firstLine=False)
            return line1 + '\n' + line2
        
    def _tunerStatusHasAllocId(self,alloc_id):
        props = self.dut.query([])
        props = properties.props_to_dict(props)
        for tuner in props['FRONTEND::tuner_status']:
            if alloc_id in tuner['FRONTEND::tuner_status::allocation_id_csv'].split(','):
                return True
        return False
    
    def _findTunerStatusProps(match={},notmatch={}):
        ''' query latest props, find tuner status associated with key/value pairs
            in "match" dict where the key/value pairs of "notmatch" dict don't match
            return a list of tuner status prop dicts
            return empty list no tuner status satisfies the criteria
            if FRONTEND::tuner_status prop not found, raises KeyError
            if any key in match or notmatch not found, raises KeyError
        '''
        props = self.dut.query([])
        props = properties.props_to_dict(props)
        tuners = copy.deepcopy(props['FRONTEND::tuner_status'])
        for k,v in match.items():
            bad = []
            for tuner in tuners:
                if tuner[k] != v:
                    bad.append(tuner)
            tuners = [x for x in tuners if x not in bad]
            #tuners = filter(lambda x: x not in bad, tuners)
        for k,v in notmatch.items():
            bad = []
            for tuner in tuners:
                if tuner[k] == v:
                    bad.append(tuner)
            tuners = [x for x in tuners if x not in bad]
            #tuners = filter(lambda x: x not in bad, tuners)
        return tuners
    
    def _getTunerStatusProp(self,alloc_id,name=None):
        ''' query latest props, find tuner status associated with alloc_id
            if name arg is specified, return the tuner status property of that name
            otherwise, return the tuner status prop as a dict
            return None if either alloc_id or name not found
            if FRONTEND::tuner_status prop not found, raises KeyError
        '''
        props = self.dut.query([])
        props = properties.props_to_dict(props)
        for tuner in props['FRONTEND::tuner_status']:
            if alloc_id in tuner['FRONTEND::tuner_status::allocation_id_csv'].split(','):
                break
        else:
            return None
        
        if name!=None:
            try:
                return tuner[name]
            except KeyError:
                return None
        else:
            return tuner
        
    def _checkBasicAllocation(self,tuner_type):
        ''' tuner_type is RX, TX, RX_DIGITIZER, CHANNELIZER, or RX_DIGITIZER_CHANNELIZER
        '''
        if tuner_type == 'RX':
            tuner_gen = self._generateRX
            ddc_gen = None
        elif tuner_type == 'RX_DIGITIZER':
            tuner_gen = self._generateRD
            ddc_gen = None
        elif tuner_type == 'CHANNELIZER':
            tuner_gen = self._generateCHAN
            ddc_gen = self._generateDDC
        elif tuner_type == 'RX_DIGITIZER_CHANNELIZER':
            tuner_gen = self._generateRDC
            ddc_gen = self._generateDDC
        else:
            print 'ERROR: Basic Allocation tests for tuner_type of %s not supported'%tuner_type
            return
        
        # Allocate a single tuner
        t1 = tuner_gen()
        t1Alloc = self._generateAlloc(t1)
        self.check(self.dut_ref.allocateCapacity(t1Alloc), True, 'Can allocate single %s'%tuner_type, throwOnFailure=True)
        
        if ddc_gen:
            # Allocate a single DDC on that channel
            ddc = ddc_gen(t1)
            ddcAlloc = self._generateAlloc(ddc)
            self.check(self.dut_ref.allocateCapacity(ddcAlloc), True, 'Can allocate single DDC', throwOnFailure=True)
    
            # Deallocate the DDC
            self.dut_ref.deallocateCapacity(ddcAlloc)
            self.check(True, True, 'Deallocated DDC')
        
        # Deallocate the tuner
        self.dut_ref.deallocateCapacity(t1Alloc)
        self.check(True, True, 'Deallocated %s without error'%tuner_type)
        
        # Allocate to max tuners
        ts = []
        for t in range(0,self.device_discovery[tuner_type]):
            ts.append(tuner_gen())
            tAlloc = self._generateAlloc(ts[-1])
            self.check(self.dut_ref.allocateCapacity(tAlloc), True, 'Allocating %s number: %s'%(tuner_type,t), throwOnFailure=True, silentSuccess=True)
        self.check(True, True, 'Allocated to max %ss'%tuner_type)
        
        if ddc_gen:
            # Allocate to max DDCs for each tuner
            if DEVICE_INFO[tuner_type]['MAX_DDC_PER_CHAN'] > 0:
                # DDCs are mapped to specific channels
                ddcs_per_chan = [DEVICE_INFO[tuner_type]['MAX_DDC_PER_CHAN']]*len(ts)  # list of # DDCs for each channel
            else:
                # DDCs are shared among channels
                ddcs_per_chan = [int(DEVICE_INFO['DDC']['MAX_DDC'] / len(ts))]*len(ts) # evenly distribute the DDCs among channels
                ddcs_per_chan[0] += DEVICE_INFO['DDC']['MAX_DDC'] - sum(ddcs_per_chan) # if leftover DDCs, add them to first channel
            ddcs = []
            for num,t in enumerate(ts):
                for d in range(0,ddcs_per_chan[num]):
                    ddcs.append(ddc_gen(t))
                    ddcAlloc = self._generateAlloc(ddcs[-1])
                    self.check(self.dut_ref.allocateCapacity(ddcAlloc), True, 'Allocating DDC number: %s.%s'%(num,d), throwOnFailure=True, silentSuccess=True)
            self.check(True, True, 'Allocated to max DDC on each %s'%tuner_type)
        
        # Verify over-allocation failure
        over_t = tuner_gen()
        over_tAlloc = self._generateAlloc(over_t)
        self.check(self.dut_ref.allocateCapacity(over_tAlloc), False, 'Over-allocate %s check'%tuner_type)
        self.dut_ref.deallocateCapacity(over_tAlloc)
        
        if ddc_gen:
            # Verify over-allocation failure of DDC
            over_ddc = ddc_gen(ts[-1]) #attempt allocation on last valid tuner
            over_ddcAlloc = self._generateAlloc(over_ddc)
            self.check(self.dut_ref.allocateCapacity(over_ddcAlloc), False, 'Overallocate DDC check')
            self.dut_ref.deallocateCapacity(over_ddcAlloc)
        
        # deallocate everything
        if ddc_gen:
            for ddc in ddcs:
                ddcAlloc = self._generateAlloc(ddc)
                self.dut_ref.deallocateCapacity(ddcAlloc)
        
        for t in ts:
            tAlloc = self._generateAlloc(t)
            self.dut_ref.deallocateCapacity(tAlloc)
        self.check(True, True, 'Deallocated all %s tuners'%tuner_type)
        
    def _checkInvalidAllocations(self,tuner,ttype,parent_alloc_id=None):
        #3.2.1 Verify InvalidCapacityException on repeat Alloc ID
        alloc_id = tuner['ALLOC_ID']
        tAlloc = self._generateAlloc(tuner)
        self.check(self.dut_ref.allocateCapacity(tAlloc), True, 'Allocate single %s with alloc id: %s'%(ttype,alloc_id))
        try:
            retval = self.dut_ref.allocateCapacity(tAlloc)
        except CF.Device.InvalidCapacity:
            self.check(True, True, 'Allocate second %s with same alloc id check (produces InvalidCapacity exception)'%(ttype))
        except Exception, e:
            self.check(False, True, 'Allocate second %s with same alloc id check (produces %s exception, should produce InvalidCapacity exception)'%(ttype,e.__class__.__name__))
        else:
            self.check(False, True, 'Allocate second %s with same alloc id check (returns %s, should produce InvalidCapacity exception)'%(ttype,retval))
        self.dut_ref.deallocateCapacity(tAlloc) # this will deallocate the original successful allocation
        
        if parent_alloc_id:
            # try to alloc using parent id
            alloc_id = tuner['ALLOC_ID']
            tAlloc1 = self._generateAlloc(tuner)
            self.check(self.dut_ref.allocateCapacity(tAlloc1), True, 'Allocate single %s with alloc id: %s'%(ttype,alloc_id))
            
            tmp_tuner = copy.deepcopy(tuner)
            tmp_tuner['ALLOC_ID'] = parent_alloc_id
            tAlloc2 = self._generateAlloc(tmp_tuner)
            try:
                retval = self.dut_ref.allocateCapacity(tAlloc2)
            except CF.Device.InvalidCapacity:
                self.check(True, True, 'Allocate second %s with alloc id of parent check (produces InvalidCapacity exception)'%(ttype))
            except Exception, e:
                self.check(False, True, 'Allocate second %s with alloc id of parent check (produces %s exception, should produce InvalidCapacity exception)'%(ttype,e.__class__.__name__))
            else:
                self.check(False, True, 'Allocate second %s with alloc id of parent check (returns %s, should produce InvalidCapacity exception)'%(ttype,retval))
            self.dut_ref.deallocateCapacity(tAlloc1)
            self.dut_ref.deallocateCapacity(tAlloc2)
            
        #Verify InvalidCapacityException on malformed request (missing alloc ID)
        # First, check empty string
        tmp_tuner = copy.deepcopy(tuner)
        tmp_tuner['ALLOC_ID'] = ''
        tAlloc = self._generateAlloc(tmp_tuner)
        try:
            retval = self.dut_ref.allocateCapacity(tAlloc)
        except CF.Device.InvalidCapacity:
            self.check(True, True, 'Allocate %s with malformed request (alloc_id="") check (produces InvalidCapcity exception)'%(ttype))
        except Exception, e:
            self.check(False, True, 'Allocate %s with malformed request (alloc_id="") check (produces %s exception, should produce InvalidCapacity exception)'%(ttype,e.__class__.__name__))
        else:
            self.check(False, True, 'Allocate %s with malformed request (alloc_id="") check (returns %s, should produce InvalidCapacity exception)'%(ttype,retval))
        self.dut_ref.deallocateCapacity(tAlloc)
        # now try None
        tmp_tuner['ALLOC_ID'] = None
        tAlloc = self._generateAlloc(tmp_tuner)
        try:
            retval = self.dut_ref.allocateCapacity(tAlloc)
        except CF.Device.InvalidCapacity:
            self.check(True, True, 'Allocate %s with malformed request (alloc_id=None) check (produces InvalidCapcity exception)'%(ttype))
        except Exception, e:
            self.check(False, True, 'Allocate %s with malformed request (alloc_id=None) check (produces %s exception, should produce InvalidCapacity exception)'%(ttype,e.__class__.__name__))
        else:
            self.check(False, True, 'Allocate %s with malformed request (alloc_id=None) check (returns %s, should produce InvalidCapacity exception)'%(ttype,retval))
        self.dut_ref.deallocateCapacity(tAlloc)
        
        # Verify failure on alloc with invalid group id (generate new uuid)
        tmp_tuner = copy.deepcopy(tuner)
        tmp_tuner['GROUP_ID'] = str(uuid.uuid4())
        tAlloc = self._generateAlloc(tmp_tuner)
        try:
            retval = self.dut_ref.allocateCapacity(tAlloc)
        except Exception, e:
            self.check(False, True, 'Allocate %s with invalid GROUP_ID check (produces %s exception, should return False)'%(ttype,e.__class__.__name__))
        else:
            self.check(False, retval, 'Allocate %s with invalid GROUP_ID check'%(ttype))
        self.dut_ref.deallocateCapacity(tAlloc)
        
        # Verify failure on alloc with invalid rf flow id (generate new uuid)
        tmp_tuner = copy.deepcopy(tuner)
        tmp_tuner['RF_FLOW_ID'] = str(uuid.uuid4())
        tAlloc = self._generateAlloc(tmp_tuner)
        try:
            retval = self.dut_ref.allocateCapacity(tAlloc)
        except Exception, e:
            self.check(False, True, 'Allocate %s with invalid RF_FLOW_ID check (produces %s exception, should return False)'%(ttype,e.__class__.__name__))
        else:
            self.check(False, retval, 'Allocate %s with invalid RF_FLOW_ID check'%(ttype))
        self.dut_ref.deallocateCapacity(tAlloc)
        
    def _checkListenerAllocation(self,tuner,ttype):
        # Allocate Listener via listener struct
        tAlloc = self._generateAlloc(tuner)
        self.dut_ref.allocateCapacity(tAlloc)
        #self.check(self.dut_ref.allocateCapacity(tAlloc), True, 'Allocate controller %s'%(ttype))
        
        tListener = self._generateListener(tuner)
        tListenerAlloc = self._generateListenerAlloc(tListener)
        self.check(self.dut_ref.allocateCapacity(tListenerAlloc), True, 'Allocate listener %s using listener allocation struct'%(ttype))
        
        # Deallocate listener using listener allocation struct
        try:
            self.dut_ref.deallocateCapacity(tListenerAlloc)
        except Exception,e:
            self.check(False, True, 'Deallocated listener %s using listener allocation struct without error'%(ttype))
        else:
            self.check(True, True, 'Deallocated listener %s using listener allocation struct without error'%(ttype))
        self.dut_ref.deallocateCapacity(tAlloc)
        
        # Allocate Listener via tuner allocation struct
        tAlloc = self._generateAlloc(tuner)
        self.dut_ref.allocateCapacity(tAlloc)
        
        tListener = copy.deepcopy(tuner)
        tListener['ALLOC_ID'] = str(uuid.uuid4())
        tListener['CONTROL'] = False
        tListenerAlloc = self._generateAlloc(tListener)
        self.check(self.dut_ref.allocateCapacity(tListenerAlloc), True, 'Allocate listener %s using tuner allocation struct'%(ttype))
        
        # Deallocate listener using tuner allocation struct
        try:
            self.dut_ref.deallocateCapacity(tListenerAlloc)
        except Exception,e:
            self.check(False, True, 'Deallocated listener %s using tuner allocation struct without error'%(ttype))
        else:
            self.check(True, True, 'Deallocated listener %s using tuner allocation struct without error'%(ttype))
        self.dut_ref.deallocateCapacity(tAlloc)
        
        # Verify failure on listener alloc w/o matching existing alloc id
        tAlloc = self._generateAlloc(tuner)
        self.dut_ref.allocateCapacity(tAlloc)
        tListener = self._generateListener(tuner)
        tListener['ALLOC_ID'] = str(uuid.uuid4())
        tListenerAlloc = self._generateListenerAlloc(tListener)
        self.check(self.dut_ref.allocateCapacity(tListenerAlloc), False, 'Allocate listener %s using listener allocation struct with bad allocation id check'%(ttype))
        self.dut_ref.deallocateCapacity(tListenerAlloc)
        self.dut_ref.deallocateCapacity(tAlloc)
        
        # Verify failure on listener alloc w/o suitable existing channel (bad freq)
        tAlloc = self._generateAlloc(tuner)
        self.dut_ref.allocateCapacity(tAlloc)
        tListener = copy.deepcopy(tuner)
        tListener['ALLOC_ID'] = str(uuid.uuid4())
        tListener['CF'] = tuner['CF'] * 2.0
        tListener['BW'] = tuner['BW'] * 2.0
        tListener['SR'] = tuner['SR'] * 2.0
        #rdListener = self._generateRD()
        tListener['CONTROL'] = False
        tListenerAlloc = self._generateAlloc(tListener)
        self.check(self.dut_ref.allocateCapacity(tListenerAlloc), False, 'Allocate listener %s using tuner allocation struct without suitable controller %s check'%(ttype,ttype))
        self.dut_ref.deallocateCapacity(tListenerAlloc)
        self.dut_ref.deallocateCapacity(tAlloc)
        
        # Verify listener allocations are deallocated following deallocation of controlling allocation
        tAlloc = self._generateAlloc(tuner)
        self.dut_ref.allocateCapacity(tAlloc)
        tListener = self._generateListener(tuner)
        tListenerAlloc = self._generateListenerAlloc(tListener)
        self.check(self.dut_ref.allocateCapacity(tListenerAlloc), True, 'Allocate listener %s using listener allocation struct'%(ttype))
        try:
            self.dut_ref.deallocateCapacity(tAlloc)
        except Exception, e:
            self.check(False, True, 'Deallocated controller %s which has a listener allocation'%(ttype))
        else:
            self.check(True, True, 'Deallocated controller %s which has a listener allocation'%(ttype))
        has_listener = self._tunerStatusHasAllocId(tListener['LISTENER_ID'])
        self.check(has_listener, False, 'Listener %s deallocated  as result of controller %s deallocation'%(ttype,ttype))
        self.dut_ref.deallocateCapacity(tListenerAlloc)
        
    def _checkFrequencyBoundsAllocation(self,tuner,ttype,low,high):
        #frequency bounds testing
        # allocate below minimum frequency
        tmp_tuner = copy.deepcopy(tuner)
        tmp_tuner['CF'] = float(int(low / 2.0))
        tAlloc = self._generateAlloc(tmp_tuner)
        self.check(self.dut_ref.allocateCapacity(tAlloc), False, 'Allocate %s below lowest frequency in range(%s < %s)'%(ttype,tmp_tuner['CF'],low))
        self.dut_ref.deallocateCapacity(tAlloc)
        
        # allocate above maximum frequency
        tmp_tuner = copy.deepcopy(tuner)
        tmp_tuner['CF'] = float(high * 2.0)
        tAlloc = self._generateAlloc(tmp_tuner)
        self.check(self.dut_ref.allocateCapacity(tAlloc), False, 'Allocate %s above highest frequency in range(%s > %s)'%(ttype,tmp_tuner['CF'],high))
        self.dut_ref.deallocateCapacity(tAlloc)
        
        # allocate at minimum frequency
        tmp_tuner = copy.deepcopy(tuner)
        tmp_tuner['CF'] = float(low)
        tAlloc = self._generateAlloc(tmp_tuner)
        self.check(self.dut_ref.allocateCapacity(tAlloc), True, 'Allocate %s at lowest frequency in range (%s)'%(ttype,tmp_tuner['CF']))
        self.dut_ref.deallocateCapacity(tAlloc)
        
        # allocate at maximum frequency
        tmp_tuner = copy.deepcopy(tuner)
        tmp_tuner['CF'] = float(high)
        tAlloc = self._generateAlloc(tmp_tuner)
        self.check(self.dut_ref.allocateCapacity(tAlloc), True, 'Allocate %s at highest frequency in range(%s)'%(ttype,tmp_tuner['CF']))
        self.dut_ref.deallocateCapacity(tAlloc)
        
    def _checkBandwidthBoundsAllocation(self,tuner,ttype,low,high):
        # bandwidth bounds testing
        # allocate below minimum bandwidth capable (succeed)
        tmp_tuner = copy.deepcopy(tuner)
        tmp_tuner['BW'] = float(int(low / 1.333333333))
        tAlloc = self._generateAlloc(tmp_tuner)
        self.check(self.dut_ref.allocateCapacity(tAlloc), True, 'Allocate %s below lowest bandwidth in range(%s < %s)'%(ttype,tmp_tuner['BW'],low))
        self.dut_ref.deallocateCapacity(tAlloc)
        
        # allocate above maximum bandwidth capable (fail)
        tmp_tuner = copy.deepcopy(tuner)
        tmp_tuner['BW'] = float(high * 2.0)
        tAlloc = self._generateAlloc(tmp_tuner)
        failed = not self.check(self.dut_ref.allocateCapacity(tAlloc), False, 'Allocate %s above highest bandwidth in range(%s > %s)'%(ttype,tmp_tuner['BW'],high))
        # DEBUG
        '''
        if failed:
            print 'DEBUG - failed max bw alloc test'
            print 'alloc request:'
            pp(tmp_tuner)
            print 'tuner status:'
            pp(self._getTunerStatusProp(tmp_tuner['ALLOC_ID']))
            print 'END DEBUG - failed max bw alloc test'
        '''
        self.dut_ref.deallocateCapacity(tAlloc)
        
        # allocate outside of bandwidth tolerance (fail)
        tmp_tuner = copy.deepcopy(tuner)
        tmp_tuner['BW'] = float(int(low / 2.0))
        tmp_tuner['BW_TOLERANCE'] = float(10.0)
        tAlloc = self._generateAlloc(tmp_tuner)
        failed = not self.check(self.dut_ref.allocateCapacity(tAlloc), False, 'Allocate %s outside of bandwidth tolerance (%s + %s%% < %s'%(ttype,tmp_tuner['BW'],tmp_tuner['BW_TOLERANCE'],low))
        # DEBUG
        '''
        if failed:
            print 'DEBUG - failed outside bw tolerance test'
            print 'alloc request:'
            pp(tmp_tuner)
            print 'tuner status:'
            pp(self._getTunerStatusProp(tmp_tuner['ALLOC_ID']))
            print 'END DEBUG - failed outside bw tolerance test'
        '''
        self.dut_ref.deallocateCapacity(tAlloc)
        
        # allocate with bandwidth = 0 (succeed)
        tmp_tuner = copy.deepcopy(tuner)
        tmp_tuner['BW'] = float(0.0)
        tAlloc = self._generateAlloc(tmp_tuner)
        self.check(self.dut_ref.allocateCapacity(tAlloc), True, 'Allocate %s without specifying bandwidth (BW=0)'%(ttype))
        self.dut_ref.deallocateCapacity(tAlloc)
        
    def _checkSampleRateBoundsAllocation(self,tuner,ttype,low,high):
        # sample rate bounds testing
        # allocate below minimum sample rate capable (succeed)
        tmp_tuner = copy.deepcopy(tuner)
        tmp_tuner['SR'] = float(int(low / 1.333333333))
        tAlloc = self._generateAlloc(tmp_tuner)
        self.check(self.dut_ref.allocateCapacity(tAlloc), True, 'Allocate %s below lowest sample rate in range(%s < %s)'%(ttype,tmp_tuner['SR'],low))
        self.dut_ref.deallocateCapacity(tAlloc)
        
        # allocate above maximum sample rate capable (fail)
        tmp_tuner = copy.deepcopy(tuner)
        tmp_tuner['SR'] = float(high * 2.0)
        tAlloc = self._generateAlloc(tmp_tuner)
        self.check(self.dut_ref.allocateCapacity(tAlloc), False, 'Allocate %s above highest sample rate in range(%s > %s)'%(ttype,tmp_tuner['SR'],high))
        self.dut_ref.deallocateCapacity(tAlloc)
        
        # allocate outside of sample rate tolerance (fail)
        tmp_tuner = copy.deepcopy(tuner)
        tmp_tuner['SR'] = float(int(low / 2.0))
        tmp_tuner['SR_TOLERANCE'] = float(10.0)
        tAlloc = self._generateAlloc(tmp_tuner)
        failed = not self.check(self.dut_ref.allocateCapacity(tAlloc), False, 'Allocate %s outside of sample rate tolerance (%s + %s%% < %s'%(ttype,tmp_tuner['SR'],tmp_tuner['SR_TOLERANCE'],low))
        # DEBUG
        '''
        if failed:
            print 'DEBUG - failed outside sr tolerance test'
            print 'alloc request:'
            pp(tmp_tuner)
            print 'tuner status:'
            pp(self._getTunerStatusProp(tmp_tuner['ALLOC_ID']))
            print 'END DEBUG - failed outside sr tolerance test'
        '''
        self.dut_ref.deallocateCapacity(tAlloc)
        
        # allocate with sample rate = 0 (succeed)
        tmp_tuner = copy.deepcopy(tuner)
        tmp_tuner['SR'] = float(0.0)
        tAlloc = self._generateAlloc(tmp_tuner)
        self.check(self.dut_ref.allocateCapacity(tAlloc), True, 'Allocate %s without specifying sample rate (SR=0)'%(ttype))
        self.dut_ref.deallocateCapacity(tAlloc)

    def _checkTunerPortIDL(self,ttype,tuner_info,controller,listener=None,analog_tuner=None,digital_tuner=None):
        if digital_tuner:
            tuner_control = digital_tuner
            port_name = 'DigitalTuner_in'
            function_list = self.digital_tuner_idl
        elif analog_tuner:
            tuner_control = analog_tuner
            port_name = 'AnalogTuner_in'
            function_list = self.analog_tuner_idl
        else:
            print 'ERROR - must pass in either analog or digital tuner port references'
            return
        
        # make allocations
        controller_id = controller['ALLOC_ID']
        controller_alloc = self._generateAlloc(controller)
        self.dut_ref.allocateCapacity(controller_alloc)
        if listener:
            listener_id = listener['LISTENER_ID']
            listener_alloc = self._generateListenerAlloc(listener)
            self.dut_ref.allocateCapacity(listener_alloc)
        
        # Verify connection to Tuner port
        self.check(tuner_control != None, True, 'Can get %s port'%(port_name), throwOnFailure=True)
        self.check(CORBA.is_nil(tuner_control), False, 'Port reference is not nil', throwOnFailure=True)
        
        #Test 3.3.2 Verify helper functions exist
        for attr in function_list:
            try:
                self.check(callable(getattr(tuner_control,attr)), True, '%s port has function %s'%(port_name,attr))
            except AttributeError, e:
                self.check(False, True, '%s port has function %s'%(port_name,attr))
                
        #Test 3.3.3 Verify getter functions
        props = self.dut.query([])
        props_type = type(props) # need this later to check getTunerStatus return type
        props = properties.props_to_dict(props)
        #pp(props)
        for tuner_prop in props['FRONTEND::tuner_status']:
            if controller_id in tuner_prop['FRONTEND::tuner_status::allocation_id_csv'].split(','):
                break
        else:
            print 'ERROR - could not get frontend tuner status property for allocation id %s'%controller_id
            tuner_prop = {}
        
        # getTunerType
        # string: ttype
        try:
            resp = tuner_control.getTunerType(controller_id)
        except FRONTEND.NotSupportedException:
            self.check(True,True,'%s.getTunerType produces NotSupportedException'%(port_name))
        except Exception, e:
            self.check(True,False,'%s.getTunerType produces exception %s'%(port_name,e))
        else:
            self.check(type(resp), str, '%s.getTunerType has correct return type'%(port_name))
            self.check(resp in ['RX','TX','RX_DIGITIZER','CHANNELIZER','RX_DIGITIZER_CHANNELIZER','DDC'], True, '%s.getTunerType return value is within expected results'%(port_name))
            self.check(resp, ttype, '%s.getTunerType return value is correct for %s'%(port_name,ttype))
            if 'FRONTEND::tuner_status::tuner_type' in tuner_prop:
                self.check(resp, tuner_prop['FRONTEND::tuner_status::tuner_type'], '%s.getTunerType matches frontend tuner status prop'%(port_name))
            
        # getTunerDeviceControl
        # boolean: True or False
        # controller
        try:
            resp = tuner_control.getTunerDeviceControl(controller_id)
        except FRONTEND.NotSupportedException:
            self.check(True,True,'%s.getTunerDeviceControl produces NotSupportedException'%(port_name))
        except Exception, e:
            self.check(True,False,'%s.getTunerDeviceControl produces exception %s'%(port_name,e))
        else:
            self.check(type(resp), bool, '%s.getTunerDeviceControl has correct return type'%(port_name))
            self.check(resp in [True,False], True, '%s.getTunerDeviceControl return value is within expected results'%(port_name))
            self.check(resp, True, '%s.getTunerDeviceControl return True for controller alloc_id'%(port_name))
        if listener:
            #listener
            try:
                resp = tuner_control.getTunerDeviceControl(listener_id)
            except FRONTEND.NotSupportedException:
                self.check(True,True,'%s.getTunerDeviceControl produces NotSupportedException'%(port_name))
            except Exception, e:
                self.check(True,False,'%s.getTunerDeviceControl produces exception %s'%(port_name,e))
            else:
                self.check(type(resp), bool, '%s.getTunerDeviceControl has correct return type'%(port_name))
                self.check(resp in [True,False], True, '%s.getTunerDeviceControl return value is within expected results'%(port_name))
                self.check(resp, False, '%s.getTunerDeviceControl returns False for listener alloc_id'%(port_name))
            
        # getTunerGroupId
        # string: *
        try:
            resp = tuner_control.getTunerGroupId(controller_id)
        except FRONTEND.NotSupportedException:
            self.check(True,True,'%s.getTunerGroupId produces NotSupportedException'%(port_name))
        except Exception, e:
            self.check(True,False,'%s.getTunerGroupId produces exception %s'%(port_name,e))
        else:
            self.check(type(resp), str, '%s.getTunerGroupId has correct return type'%(port_name))
            self.check(type(resp), str, '%s.getTunerGroupId return value is within expected results'%(port_name))
            if 'FRONTEND::tuner_status::group_id' in tuner_prop:
                self.check(resp, tuner_prop['FRONTEND::tuner_status::group_id'], '%s.getTunerGroupId matches frontend tuner status prop'%(port_name))
            
        # getTunerRfFlowId
        # string: *
        try:
            resp = tuner_control.getTunerRfFlowId(controller_id)
        except FRONTEND.NotSupportedException:
            self.check(True,True,'%s.getTunerRfFlowId produces NotSupportedException'%(port_name))
        except Exception, e:
            self.check(True,False,'%s.getTunerRfFlowId produces exception %s'%(port_name,e))
        else:
            self.check(type(resp), str, '%s.getTunerRfFlowId has correct return type'%(port_name))
            self.check(type(resp), str, '%s.getTunerRfFlowId return value is within expected results'%(port_name))
            if 'FRONTEND::tuner_status::rf_flow_id' in tuner_prop:
                self.check(resp, tuner_prop['FRONTEND::tuner_status::rf_flow_id'], '%s.getTunerRfFlowId matches frontend tuner status prop'%(port_name))
            
        # getTunerCenterFrequency
        # double: >= 0?
        try:
            resp = tuner_control.getTunerCenterFrequency(controller_id)
        except FRONTEND.NotSupportedException:
            self.check(True,True,'%s.getTunerCenterFrequency produces NotSupportedException'%(port_name))
        except Exception, e:
            self.check(True,False,'%s.getTunerCenterFrequency produces exception %s'%(port_name,e))
        else:
            self.check(type(resp), float, '%s.getTunerCenterFrequency has correct return type'%(port_name))
            self.check(resp >= 0.0, True, '%s.getTunerCenterFrequency return value is within expected results'%(port_name))
            if 'FRONTEND::tuner_status::center_frequency' in tuner_prop:
                self.checkAlmostEqual(resp, tuner_prop['FRONTEND::tuner_status::center_frequency'], '%s.getTunerCenterFrequency matches frontend tuner status prop'%(port_name),places=0)
            
        # getTunerBandwidth
        # double: >= 0?
        try:
            resp = tuner_control.getTunerBandwidth(controller_id)
        except FRONTEND.NotSupportedException:
            self.check(True,True,'%s.getTunerBandwidth produces NotSupportedException'%(port_name))
        except Exception, e:
            self.check(True,False,'%s.getTunerBandwidth produces exception %s'%(port_name,e))
        else:
            self.check(type(resp), float, '%s.getTunerBandwidth has correct return type'%(port_name))
            self.check(resp >= 0.0, True, '%s.getTunerBandwidth return value is within expected results'%(port_name))
            if 'FRONTEND::tuner_status::bandwidth' in tuner_prop:
                self.checkAlmostEqual(resp, tuner_prop['FRONTEND::tuner_status::bandwidth'], '%s.getTunerBandwidth matches frontend tuner status prop'%(port_name),places=0)
        
        if digital_tuner:
            # getTunerOutputSampleRate
            # double: >= 0?
            try:
                resp = tuner_control.getTunerOutputSampleRate(controller_id)
            except FRONTEND.NotSupportedException:
                self.check(True,True,'%s.getTunerOutputSampleRate produces NotSupportedException'%(port_name))
            except Exception, e:
                self.check(True,False,'%s.getTunerOutputSampleRate produces exception %s'%(port_name,e))
            else:
                self.check(type(resp), float, '%s.getTunerOutputSampleRate has correct return type'%(port_name))
                self.check(resp >= 0.0, True, '%s.getTunerOutputSampleRate return value is within expected results'%(port_name))
                if 'FRONTEND::tuner_status::sample_rate' in tuner_prop:
                    self.checkAlmostEqual(resp, tuner_prop['FRONTEND::tuner_status::sample_rate'], '%s.getTunerOutputSampleRate matches frontend tuner status prop'%(port_name),places=0)
            
        # getTunerAgcEnable
        # boolean: True or False
        try:
            resp = tuner_control.getTunerAgcEnable(controller_id)
        except FRONTEND.NotSupportedException:
            self.check(True,True,'%s.getTunerAgcEnable produces NotSupportedException'%(port_name))
        except Exception, e:
            self.check(True,False,'%s.getTunerAgcEnable produces exception %s'%(port_name,e))
        else:
            self.check(type(resp), bool, '%s.getTunerAgcEnable has correct return type'%(port_name))
            self.check(resp in [True,False], True, '%s.getTunerAgcEnable return value is within expected results'%(port_name))
            if 'FRONTEND::tuner_status::agc' in tuner_prop:
                self.check(resp, tuner_prop['FRONTEND::tuner_status::agc'], '%s.getTunerAgcEnable matches frontend tuner status prop'%(port_name))
            
        # getTunerGain
        # float: ?
        try:
            resp = tuner_control.getTunerGain(controller_id)
        except FRONTEND.NotSupportedException:
            self.check(True,True,'%s.getTunerGain produces NotSupportedException'%(port_name))
        except Exception, e:
            self.check(True,False,'%s.getTunerGain produces exception %s'%(port_name,e))
        else:
            self.check(type(resp), float, '%s.getTunerGain has correct return type'%(port_name))
            self.check(type(resp), float, '%s.getTunerGain return value is within expected results'%(port_name))
            if 'FRONTEND::tuner_status::gain' in tuner_prop:
                self.checkAlmostEqual(resp, tuner_prop['FRONTEND::tuner_status::gain'], '%s.getTunerGain matches frontend tuner status prop'%(port_name),places=2)
            
        # getTunerReferenceSource
        # long: 0 (internal) or 1 (external)
        try:
            resp = tuner_control.getTunerReferenceSource(controller_id)
        except FRONTEND.NotSupportedException:
            self.check(True,True,'%s.getTunerReferenceSource produces NotSupportedException'%(port_name))
        except Exception, e:
            self.check(True,False,'%s.getTunerReferenceSource produces exception %s'%(port_name,e))
        else:
            self.check(type(resp) in [int,long], True, '%s.getTunerReferenceSource returns correct type'%(port_name))
            self.check(resp in [0,1], True, '%s.getTunerReferenceSource return value within expected results'%(port_name))
            if 'FRONTEND::tuner_status::reference_source' in tuner_prop:
                self.check(resp, tuner_prop['FRONTEND::tuner_status::reference_source'], '%s.getTunerReferenceSource matches frontend tuner status prop'%(port_name))

        # getTunerEnable
        # boolean: True or False
        try:
            resp = tuner_control.getTunerEnable(controller_id)
        except FRONTEND.NotSupportedException:
            self.check(True,True,'%s.getTunerEnable produces NotSupportedException'%(port_name))
        except Exception, e:
            self.check(True,False,'%s.getTunerEnable produces exception %s'%(port_name,e))
        else:
            self.check(type(resp), bool, '%s.getTunerEnable has correct return type'%(port_name))
            self.check(resp in [True,False], True, '%s.getTunerEnable return value is within expected results'%(port_name))
            if 'FRONTEND::tuner_status::enabled' in tuner_prop:
                self.check(resp, tuner_prop['FRONTEND::tuner_status::enabled'], '%s.getTunerEnable matches frontend tuner status prop'%(port_name))
            
        # getTunerStatus
        # CF::Properties: has 'FRONTEND::tuner_status::allocation_id_csv' with alloc_id requested
        try:
            resp = tuner_control.getTunerStatus(controller_id)
        except FRONTEND.NotSupportedException:
            self.check(True,True,'%s.getTunerStatus produces NotSupportedException'%(port_name))
        except Exception, e:
            self.check(True,False,'%s.getTunerStatus produces exception %s'%(port_name,e))
        else:
            self.check(type(resp), props_type, '%s.getTunerStatus has correct return type'%(port_name))
            self.check(type(resp), props_type, '%s.getTunerStatus return value is within expected results'%(port_name))
            resp = properties.props_to_dict(resp)
            #pp(resp)
            self.check(controller_id in resp['FRONTEND::tuner_status::allocation_id_csv'].split(','), True, '%s.getTunerStatus return value has correct tuner status for allocation ID requested'%(port_name))
            self.check(resp, tuner_prop, '%s.getTunerStatus matches frontend tuner status prop'%(port_name))
        
        # Verify setter functions
        # for each of the following, do bounds checking in addition to simple setter checking
        # setTunerCenterFrequency
        # setTunerBandwidth
        # setTunerOutputSampleRate
        # setTunerGain
          
        # Verify in-bounds retune
        #check Center Freq: tune to min, max, then orig
        try:
            cf = tuner_control.getTunerCenterFrequency(controller_id)
        except FRONTEND.NotSupportedException:
            self.check(True,True,'%s.getTunerCenterFrequency produces NotSupportedException -- cannot verify setTunerFrequency function'%(port_name))
            try:
                tuner_control.setTunerCenterFrequency(controller_id, tuner_info['CF_MIN'])
            except FRONTEND.NotSupportedException:
                self.check(True,True,'%s.setTunerCenterFrequency produces NotSupportedException'%(port_name))
            else:
                self.check(True,True,'%s.setTunerCenterFrequency executes without throwing exception'%(port_name))
        else:
            try:
                tuner_control.setTunerCenterFrequency(controller_id, tuner_info['CF_MIN'])
            except FRONTEND.NotSupportedException:
                self.check(True,True,'%s.setTunerCenterFrequency produces NotSupportedException'%(port_name))
            else:
                self.checkAlmostEqual(tuner_info['CF_MIN'],tuner_control.getTunerCenterFrequency(controller_id),'In-bounds re-tune of frequency - tuned to minimum CF (%s)'%(tuner_info['CF_MIN']),places=0)
                tuner_control.setTunerCenterFrequency(controller_id, tuner_info['CF_MAX'])
                self.checkAlmostEqual(tuner_info['CF_MAX'],tuner_control.getTunerCenterFrequency(controller_id),'In-bounds re-tune of frequency - tuned to maximum CF (%s)'%(tuner_info['CF_MAX']),places=0)
                tuner_control.setTunerCenterFrequency(controller_id, cf)
                self.checkAlmostEqual(cf,tuner_control.getTunerCenterFrequency(controller_id),'In-bounds re-tune of frequency - tuned back to original CF (%s)'%(cf),places=0)
                
        #Check Bandwidth: tune to min, max, then orig
        try:
            bw = tuner_control.getTunerBandwidth(controller_id)
        except FRONTEND.NotSupportedException:
            self.check(True,True,'%s.getTunerBandwidth produces NotSupportedException -- cannot verify setTunerBandwidth function'%(port_name))
            try:
                tuner_control.setTunerBandwidth(controller_id, tuner_info['BW_MIN'])
            except FRONTEND.NotSupportedException:
                self.check(True,True,'%s.setTunerBandwidth produces NotSupportedException'%(port_name))
            else:
                self.check(True,True,'%s.setTunerBandwidth executes without throwing exception'%(port_name))
        else:
            try:
                tuner_control.setTunerBandwidth(controller_id, tuner_info['BW_MIN'])
            except FRONTEND.NotSupportedException:
                self.check(True,True,'%s.setTunerBandwidth produces NotSupportedException'%(port_name))
            else:
                self.checkAlmostEqual(tuner_info['BW_MIN'],tuner_control.getTunerBandwidth(controller_id),'In-bounds re-tune of bandwidth - set to minimum BW (%s)'%tuner_info['BW_MIN'],places=0)
                tuner_control.setTunerBandwidth(controller_id, tuner_info['BW_MAX'])
                self.checkAlmostEqual(tuner_info['BW_MAX'],tuner_control.getTunerBandwidth(controller_id),'In-bounds re-tune of bandwidth - set to maximum BW (%s)'%tuner_info['BW_MAX'],places=0)
                tuner_control.setTunerBandwidth(controller_id, bw)
                self.checkAlmostEqual(bw,tuner_control.getTunerBandwidth(controller_id),'In-bounds re-tune of bandwidth - set to original BW (%s)'%bw,places=0)
        
        if digital_tuner:
            #Check SR: tune to min, max, then orig
            try:
                sr = tuner_control.getTunerOutputSampleRate(controller_id)
            except FRONTEND.NotSupportedException:
                self.check(True,True,'%s.getTunerOutputSampleRate produces NotSupportedException -- cannot verify setTunerOutputSampleRate function'%(port_name))
                try:
                    tuner_control.setTunerOutputSampleRate(controller_id, tuner_info['SR_MIN'])
                except FRONTEND.NotSupportedException:
                    self.check(True,True,'%s.setTunerOutputSampleRate produces NotSupportedException'%(port_name))
                else:
                    self.check(True,True,'%s.setTunerOutputSampleRate executes without throwing exception'%(port_name))
            else:
                try:
                    tuner_control.setTunerOutputSampleRate(controller_id, tuner_info['SR_MIN'])
                except FRONTEND.NotSupportedException:
                    self.check(True,True,'%s.setTunerOutputSampleRate produces NotSupportedException'%(port_name))
                else:
                    self.checkAlmostEqual(tuner_info['SR_MIN'],tuner_control.getTunerOutputSampleRate(controller_id),'In-bounds re-tune of sample rate - set to minimum SR (%s)'%tuner_info['SR_MIN'],places=0)
                    tuner_control.setTunerOutputSampleRate(controller_id, tuner_info['SR_MAX'])   
                    self.checkAlmostEqual(tuner_info['SR_MAX'],tuner_control.getTunerOutputSampleRate(controller_id),'In-bounds re-tune of sample rate - set to maximum SR (%s)'%tuner_info['SR_MAX'],places=0)
                    tuner_control.setTunerOutputSampleRate(controller_id, sr)   
                    self.checkAlmostEqual(sr,tuner_control.getTunerOutputSampleRate(controller_id),'In-bounds re-tune of sample rate - set to original SR (%s)'%sr,places=0)
        
        # check gain: set to min, max, then orig
        try:
            gain = tuner_control.getTunerGain(controller_id)
        except FRONTEND.NotSupportedException:
            self.check(True,True,'%s.getTunerGain produces NotSupportedException -- cannot verify setTunerGain function'%(port_name))
            try:
                tuner_control.setTunerGain(controller_id, tuner_info['GAIN_MIN'])
            except FRONTEND.NotSupportedException:
                self.check(True,True,'%s.setTunerGain produces NotSupportedException'%(port_name))
            else:
                self.check(True,True,'%s.setTunerGain executes without throwing exception'%(port_name))
        else:
            try:
                tuner_control.setTunerGain(controller_id, tuner_info['GAIN_MIN'])
            except FRONTEND.NotSupportedException:
                self.check(True,True,'%s.setTunerGain produces NotSupportedException'%(port_name))
            else:
                self.checkAlmostEqual(tuner_info['GAIN_MIN'],tuner_control.getTunerGain(controller_id),'In-bounds setting of gain - gain set to minimum gain (%s)'%tuner_info['GAIN_MIN'],places=2)
                tuner_control.setTunerGain(controller_id, tuner_info['GAIN_MAX'])
                self.checkAlmostEqual(tuner_info['GAIN_MAX'],tuner_control.getTunerGain(controller_id),'In-bounds setting of gain - gain set to maximum gain (%s)'%tuner_info['GAIN_MAX'],places=2)
                tuner_control.setTunerGain(controller_id, gain)
                self.checkAlmostEqual(gain,tuner_control.getTunerGain(controller_id),'In-bounds setting of gain - set to original gain (%s)'%gain,places=2)
                    
        #Verify outside-bounds retune
        #check Center Freq:
        try:
            cf = tuner_control.getTunerCenterFrequency(controller_id)
        except FRONTEND.NotSupportedException:
            self.check(True,True,'%s.getTunerCenterFrequency produces NotSupportedException -- cannot verify out-of-bounds frequency tuning'%(port_name))
        else:
            try:
                tuner_control.setTunerCenterFrequency(controller_id, tuner_info['CF_MAX'] + cf)
            except FRONTEND.NotSupportedException:
                self.check(True,True,'%s.setTunerCenterFrequency produces NotSupportedException -- cannot verify out-of-bounds frequency tuning'%(port_name))
            except FRONTEND.BadParameterException, e:
                self.check(True, True,'Out-of-bounds re-tune of frequency produces BadParameterException')
            except FRONTEND.FrontendException, e:
                self.check(False, True,'Out-of-bounds re-tune of frequency produces BadParameterException (produces FrontendException instead)')
            else:
                self.check(False, True,'Out-of-bounds re-tune of frequency produces BadParameterException')
            if not self.checkAlmostEqual(cf, tuner_control.getTunerCenterFrequency(controller_id),'Out-of-bounds re-tune of frequency - CF unchanged',places=0):
                try:
                    tuner_control.setTunerCenterFrequency(controller_id, cf)
                except:
                    pass
        
        #Check Bandwidth      
        try:
            bw = tuner_control.getTunerBandwidth(controller_id)
        except FRONTEND.NotSupportedException:
            self.check(True,True,'%s.getTunerBandwidth produces NotSupportedException -- cannot verify out-of-bounds bandwidth tuning'%(port_name))
        else:
            try:
                tuner_control.setTunerBandwidth(controller_id, tuner_info['BW_MAX'] + bw)
            except FRONTEND.NotSupportedException:
                self.check(True,True,'%s.setTunerBandwidth produces NotSupportedException -- cannot verify out-of-bounds bandwidth tuning'%(port_name))
            except FRONTEND.BadParameterException, e:
                self.check(True, True,'Out-of-bounds re-tune of bandwidth produces BadParameterException')
            except FRONTEND.FrontendException, e:
                self.check(False, True,'Out-of-bounds re-tune of bandwidth produces BadParameterException (produces FrontendException instead)')
            else:
                self.check(False, True,'Out-of-bounds re-tune of bandwidth produces BadParameterException')
                # DEBUG
                '''
                print 'DEBUG - out of bounds retune of bw did not produce exception'
                print 'DEBUG - tuned bw: %s'%(tuner_info['BW_MAX'] + bw)
                print 'DEBUG - tuner status:'
                pp(self._getTunerStatusProp(controller_id))
                '''
            new_bw = tuner_control.getTunerBandwidth(controller_id)
            if not self.checkAlmostEqual(bw, new_bw,'Out-of-bounds re-tune of bandwidth - BW unchanged',places=0):
                # DEBUG
                '''
                print 'DEBUG - out of bounds retune of bw incorrectly caused change in bw'
                print 'DEBUG - orig bw: %s  new bw: %s  tuned bw: %s'%(bw,new_bw,tuner_info['BW_MAX'] + bw)
                # end DEBUG
                '''
                try:
                    tuner_control.setTunerBandwidth(controller_id, bw)
                except:
                    pass
        
        if digital_tuner:
            #Check SR
            try:
                sr = tuner_control.getTunerOutputSampleRate(controller_id)
            except FRONTEND.NotSupportedException:
                self.check(True,True,'%s.getTunerOutputSampleRate produces NotSupportedException -- cannot verify out-of-bounds sample rate tuning'%(port_name))
            else:
                try:
                    tuner_control.setTunerOutputSampleRate(controller_id, tuner_info['SR_MAX'] + sr)
                except FRONTEND.NotSupportedException:
                    self.check(True,True,'%s.setTunerOutputSampleRate produces NotSupportedException -- cannot verify out-of-bounds sample rate tuning'%(port_name))
                except FRONTEND.BadParameterException, e:
                    self.check(True, True,'Out-of-bounds re-tune of sample rate produces BadParameterException')
                except FRONTEND.FrontendException, e:
                    self.check(False, True,'Out-of-bounds re-tune of sample rate produces BadParameterException (produces FrontendException instead)')
                else:
                    self.check(False, True,'Out-of-bounds re-tune of sample rate produces BadParameterException')
                new_sr = tuner_control.getTunerOutputSampleRate(controller_id)
                if not self.checkAlmostEqual(sr, new_sr,'Out-of-bounds re-tune of sample rate - SR unchanged',places=0):
                    # DEBUG
                    '''
                    print 'DEBUG - out of bounds retune of sr incorrectly caused change in sr'
                    print 'DEBUG - orig sr: %s  new sr: %s  tuned sr: %s'%(sr,new_sr,DEVICE_INFO['RX_DIGITIZER']['SR_MAX'] + sr)
                    # end DEBUG
                    '''
                    try:
                        tuner_control.setTunerOutputSampleRate(controller_id, sr)
                    except:
                        pass
        
        #Check gain
        try:
            gain = tuner_control.getTunerGain(controller_id)
        except FRONTEND.NotSupportedException:
            self.check(True,True,'%s.getTunerGain produces NotSupportedException -- cannot verify out-of-bounds gain setting'%(port_name))
        else:
            try:
                tuner_control.setTunerGain(controller_id, tuner_info['GAIN_MAX'] + abs(tuner_info['GAIN_MAX']-tuner_info['GAIN_MIN']) + 1)
            except FRONTEND.NotSupportedException:
                self.check(True,True,'%s.setTunerGain produces NotSupportedException -- cannot verify out-of-bounds gain setting'%(port_name))
            except FRONTEND.BadParameterException, e:
                self.check(True, True,'Out-of-bounds setting of gain produces BadParameterException')
            except FRONTEND.FrontendException, e:
                self.check(False, True,'Out-of-bounds setting of gain produces BadParameterException (produces FrontendException instead)')
            else:
                self.check(False, True,'Out-of-bounds setting of gain produces BadParameterException')
            new_gain = tuner_control.getTunerGain(controller_id)
            if not self.checkAlmostEqual(gain, new_gain,'Out-of-bounds setting of gain - gain unchanged',places=2):
                '''
                # DEBUG
                print 'DEBUG - out of bounds retune of gain incorrectly caused change in gain'
                print 'DEBUG - orig gain: %s  new gain: %s  tuned gain: %s'%(gain,new_gain,tuner_info['GAIN_MAX'] + abs(tuner_info['GAIN_MAX']-tuner_info['GAIN_MIN']) + 1)
                # end DEBUG
                '''
                try:
                    tuner_control.setTunerGain(controller_id, gain)
                except:
                    pass
        
        # test changing values for the rest
        # setTunerAgcEnable
        try:
            orig = tuner_control.getTunerAgcEnable(controller_id)
        except FRONTEND.NotSupportedException:
            self.check(True,True,'%s.getTunerAgcEnable produces NotSupportedException -- cannot test setTunerAgcEnable function'%(port_name))
            try:
                tuner_control.setTunerAgcEnable(controller_id, False)
            except FRONTEND.NotSupportedException:
                self.check(True,True,'%s.setTunerAgcEnable produces NotSupportedException'%(port_name))
            else:
                self.check(True,True,'%s.setTunerAgcEnable executes without throwing exception'%(port_name))
        else:
            try:
                tuner_control.setTunerAgcEnable(controller_id, not orig)
            except FRONTEND.NotSupportedException:
                self.check(True,True,'%s.setTunerAgcEnable produces NotSupportedException'%(port_name))
            else:
                self.check(not orig,tuner_control.getTunerAgcEnable(controller_id),'setting agc enable -- set to new value')
                tuner_control.setTunerAgcEnable(controller_id, orig)
                self.check(orig,tuner_control.getTunerAgcEnable(controller_id),'setting agc enable -- set back to original value')
            
        # setTunerReferenceSource
        try:
            orig = tuner_control.getTunerReferenceSource(controller_id)
        except FRONTEND.NotSupportedException:
            self.check(True,True,'%s.getTunerReferenceSource produces NotSupportedException -- cannot test setTunerReferenceSource function'%(port_name))
            try:
                tuner_control.setTunerReferenceSource(controller_id, False)
            except FRONTEND.NotSupportedException:
                self.check(True,True,'%s.setTunerReferenceSource produces NotSupportedException'%(port_name))
            else:
                self.check(True,True,'%s.setTunerReferenceSource executes without throwing exception'%(port_name))
        else:
            try:
                tuner_control.setTunerReferenceSource(controller_id, int(not orig))
            except FRONTEND.NotSupportedException:
                self.check(True,True,'%s.setTunerReferenceSource produces NotSupportedException'%(port_name))
            else:
                self.check(int(not orig),tuner_control.getTunerReferenceSource(controller_id),'setting tuner reference source -- set to new value')
                tuner_control.setTunerReferenceSource(controller_id, orig)
                self.check(orig,tuner_control.getTunerReferenceSource(controller_id),'setting tuner reference source -- set back to original value')
            
        # setTunerEnable
        try:
            orig = tuner_control.getTunerEnable(controller_id)
        except FRONTEND.NotSupportedException:
            self.check(True,True,'%s.getTunerEnable produces NotSupportedException -- cannot test setTunerEnable function'%(port_name))
            try:
                tuner_control.setTunerEnable(controller_id, True)
            except FRONTEND.NotSupportedException:
                self.check(True,True,'%s.setTunerEnable produces NotSupportedException'%(port_name))
            else:
                self.check(True,True,'%s.setTunerEnable executes without throwing exception'%(port_name))
        else:
            try:
                tuner_control.setTunerEnable(controller_id, not orig)
            except FRONTEND.NotSupportedException:
                self.check(True,True,'%s.setTunerEnable produces NotSupportedException'%(port_name))
            else:
                self.check(not orig,tuner_control.getTunerEnable(controller_id),'setting tuner enable -- set to new value')
                tuner_control.setTunerEnable(controller_id, orig)
                self.check(orig,tuner_control.getTunerEnable(controller_id),'setting tuner enable -- set back to original value')
            
        # verify invalid alloc_id -> FrontendException... for each?
        bad_id = str(uuid.uuid4())
        for attr in filter(lambda x: x.startswith('get'),function_list):
            f = getattr(tuner_control,attr)
            try:
                resp = f(bad_id)
            except FRONTEND.NotSupportedException:
                self.check(True,True,'%s.%s called with bad alloc_id produces NotSupportedException'%(port_name,attr))
            except FRONTEND.FrontendException:
                self.check(True,True,'%s.%s called with bad alloc_id produces FrontendException'%(port_name,attr))
            except Exception, e:
                self.check(False,True,'%s.%s called with bad alloc_id produces FrontendException'%(port_name,attr))
            else:
                self.check(False,True,'%s.%s called with bad alloc_id produces FrontendException (no exception)'%(port_name,attr))
                
        # setTunerCenterFrequency
        try:
            tuner_control.setTunerCenterFrequency(bad_id, float(tuner_info['CF_MIN']))
        except FRONTEND.NotSupportedException:
            self.check(True,True,'%s.setTunerCenterFrequency called with bad alloc_id produces NotSupportedException'%(port_name))
        except FRONTEND.FrontendException:
            self.check(True,True,'%s.setTunerCenterFrequency called with bad alloc_id produces FrontendException'%(port_name))
        except Exception, e:
            self.check(False,True,'%s.setTunerCenterFrequency called with bad alloc_id produces FrontendException'%(port_name))
        else:
            self.check(False,True,'%s.setTunerCenterFrequency called with bad alloc_id produces FrontendException (no exception)'%(port_name))
            
        # setTunerBandwidth
        try:
            tuner_control.setTunerBandwidth(bad_id, float(tuner_info['BW_MIN']))
        except FRONTEND.NotSupportedException:
            self.check(True,True,'%s.setTunerBandwidth called with bad alloc_id produces NotSupportedException'%(port_name))
        except FRONTEND.FrontendException:
            self.check(True,True,'%s.setTunerBandwidth called with bad alloc_id produces FrontendException'%(port_name))
        except Exception, e:
            self.check(False,True,'%s.setTunerBandwidth called with bad alloc_id produces FrontendException'%(port_name))
        else:
            self.check(False,True,'%s.setTunerBandwidth called with bad alloc_id produces FrontendException (no exception)'%(port_name))
            
        if digital_tuner:
            # setTunerOutputSampleRate
            try:
                tuner_control.setTunerOutputSampleRate(bad_id, float(tuner_info['SR_MIN']))
            except FRONTEND.NotSupportedException:
                self.check(True,True,'%s.setTunerOutputSampleRate called with bad alloc_id produces NotSupportedException'%(port_name))
            except FRONTEND.FrontendException:
                self.check(True,True,'%s.setTunerOutputSampleRate called with bad alloc_id produces FrontendException'%(port_name))
            except Exception, e:
                self.check(False,True,'%s.setTunerOutputSampleRate called with bad alloc_id produces FrontendException'%(port_name))
            else:
                self.check(False,True,'%s.setTunerOutputSampleRate called with bad alloc_id produces FrontendException (no exception)'%(port_name))
            
        # setTunerGain
        try:
            tuner_control.setTunerGain(bad_id, float(tuner_info['GAIN_MIN']))
        except FRONTEND.NotSupportedException:
            self.check(True,True,'%s.setTunerGain called with bad alloc_id produces NotSupportedException'%(port_name))
        except FRONTEND.FrontendException:
            self.check(True,True,'%s.setTunerGain called with bad alloc_id produces FrontendException'%(port_name))
        except Exception, e:
            self.check(False,True,'%s.setTunerGain called with bad alloc_id produces FrontendException'%(port_name))
        else:
            self.check(False,True,'%s.setTunerGain called with bad alloc_id produces FrontendException (no exception)'%(port_name))
            
        # setTunerAgcEnable
        try:
            tuner_control.setTunerAgcEnable(bad_id, False)
        except FRONTEND.NotSupportedException:
            self.check(True,True,'%s.setTunerAgcEnable called with bad alloc_id produces NotSupportedException'%(port_name))
        except FRONTEND.FrontendException:
            self.check(True,True,'%s.setTunerAgcEnable called with bad alloc_id produces FrontendException'%(port_name))
        except Exception, e:
            self.check(False,True,'%s.setTunerAgcEnable called with bad alloc_id produces FrontendException'%(port_name))
        else:
            self.check(False,True,'%s.setTunerAgcEnable called with bad alloc_id produces FrontendException (no exception)'%(port_name))
            
        # setTunerReferenceSource
        try:
            tuner_control.setTunerReferenceSource(bad_id, 0)
        except FRONTEND.NotSupportedException:
            self.check(True,True,'%s.setTunerReferenceSource called with bad alloc_id produces NotSupportedException'%(port_name))
        except FRONTEND.FrontendException:
            self.check(True,True,'%s.setTunerReferenceSource called with bad alloc_id produces FrontendException'%(port_name))
        except Exception, e:
            self.check(False,True,'%s.setTunerReferenceSource called with bad alloc_id produces FrontendException'%(port_name))
        else:
            self.check(False,True,'%s.setTunerReferenceSource called with bad alloc_id produces FrontendException (no exception)'%(port_name))
            
        # setTunerEnable
        try:
            tuner_control.setTunerEnable(bad_id, False)
        except FRONTEND.NotSupportedException:
            self.check(True,True,'%s.setTunerEnable called with bad alloc_id produces NotSupportedException'%(port_name))
        except FRONTEND.FrontendException:
            self.check(True,True,'%s.setTunerEnable called with bad alloc_id produces FrontendException'%(port_name))
        except Exception, e:
            self.check(False,True,'%s.setTunerEnable called with bad alloc_id produces FrontendException'%(port_name))
        else:
            self.check(False,True,'%s.setTunerEnable called with bad alloc_id produces FrontendException (no exception)'%(port_name))
        
        self.dut_ref.deallocateCapacity(listener_alloc)
        self.dut_ref.deallocateCapacity(controller_alloc)
        
    def _checkAnalogDataFlow(self,ttype,controller,listener=None):
        tuner_control = self.dut.getPort('AnalogTuner_in')
        comp_port_obj = self.dut.getPort('RFInfo_out')
        
        # alloc a tuner
        tAlloc = self._generateAlloc(controller)
        self.dut_ref.allocateCapacity(tAlloc)
        rfinfo_in1_port_obj = self.PortFRONTENDRFInfoIn_i()
        rfinfo_in2_port_obj = self.PortFRONTENDRFInfoIn_i()
        rfinfo_in3_port_obj = self.PortFRONTENDRFInfoIn_i() 
        comp_port_obj.connectPort(rfinfo_in1_port_obj, controller['ALLOC_ID'])
        
        # verify RFInfoPkt
        self.check(rfinfo_in1_port_obj.rfinfo_pkt.rf_flow_id,rfinfo_in1_port_obj.rf_flow_id,'RFInfo: rfinfo_pkt.rf_flow_id matches RFInfo port rf_flow_id')
        self.check(rfinfo_in1_port_obj.rfinfo_pkt.rf_center_freq,controller['CF'],'RFInfo: rfinfo packet rf center frequency matches allocation center frequency')
        self.check(rfinfo_in1_port_obj.rfinfo_pkt.rf_bandwidth,controller['BW'],'RFInfo: rfinfo packet rf bandwidth matches allocation bandwidth')
        
        #verify multi-out port
        conn2_id = uuid.uuid4()
        comp_port_obj.connectPort(rfinfo_in2_port_obj, conn2_id)
        self.check(rfinfo_in1_port_obj.rf_flow_id==rfinfo_in2_port_obj.rf_flow_id,False,'RFInfo: Did not receive correct rf_flow_id from tuner allocation with wrong alloc_id (multiport test)')
        
        if self.device_discovery[ttype] < 2:
            self.check(True,True,'RFInfo: Cannot fully test multiport because only single %s tuner capability'%(ttype),successMsg='info')
        else:
            pass # TODO - additional multiport tests here
        
        if listener:
            # verify listener
            listenerAlloc = self._generateListenerAlloc(listener)
            self.dut_ref.allocateCapacity(listenerAlloc)
            comp_port_obj.connectPort(rfinfo_in3_port_obj, listener['LISTENER_ID'])
            self.check(rfinfo_in1_port_obj.rf_flow_id,rfinfo_in3_port_obj.rf_flow_id,'RFInfo: listener port received same rf_flow_id (multiport test)')
            self.check(rfinfo_in1_port_obj.rfinfo_pkt,rfinfo_in3_port_obj.rfinfo_pkt,'RFInfo: listener port received same rfinfo_pkt (multiport test)')
            self.dut_ref.deallocateCapacity(listenerAlloc)
            
        # cleanup
        comp_port_obj.disconnectPort(controller['ALLOC_ID'])
        comp_port_obj.disconnectPort(conn2_id)
        comp_port_obj.disconnectPort(listener['LISTENER_ID'])
        self.dut_ref.deallocateCapacity(tAlloc)
        
    def _checkDigitalDataFlow(self,ttype,controller,listener1=None,listener2=None):
        tuner_control = self.dut.getPort('DigitalTuner_in')
        for port in self.scd.get_componentfeatures().get_ports().get_uses():
            comp_port_type = port.get_repid().split(':')[1].split('/')[-1]
            comp_port_name = port.get_usesname()
            if comp_port_type not in self.port_map:
                print 'WARNING - skipping %s port named %s, not supported BULKIO port type'%(comp_port_type,comp_port_name)
                continue
            self._testBULKIO(tuner_control,comp_port_name,comp_port_type,ttype,controller,listener1,listener2)
            
    def _testBULKIO(self,tuner_control,comp_port_name,comp_port_type,ttype,controller,listener1=None,listener2=None):
        if comp_port_type == 'dataSDDS':
            print 'WARNING - dataSDDS output port testing not supported'
            return
        comp_port_obj = self.dut.getPort(str(comp_port_name))
        dataSink1 = sb.DataSink()
        dataSink2 = sb.DataSink()
        dataSink3 = sb.DataSink()
        dataSink4 = sb.DataSink()
        dataSink1_port_obj = dataSink1.getPort(self.port_map[comp_port_type])
        dataSink2_port_obj = dataSink2.getPort(self.port_map[comp_port_type])
        dataSink3_port_obj = dataSink3.getPort(self.port_map[comp_port_type])
        dataSink4_port_obj = dataSink4.getPort(self.port_map[comp_port_type])
        
        sb.start()
        
        # alloc a tuner
        controller['ALLOC_ID'] = uuid.uuid4() # unique for each loop
        tAlloc = self._generateAlloc(controller)
        self.dut_ref.allocateCapacity(tAlloc)
        comp_port_obj.connectPort(dataSink1_port_obj, controller['ALLOC_ID'])
        
        # verify basic data flow
        time.sleep(1.0)
        data1 = dataSink1.getData()
        #print 'data1',len(data1)
        self.check(len(data1)>0,True,'%s: Received data from tuner allocation'%(comp_port_name))
        
        # verify SRI
        try:
            status = properties.props_to_dict(tuner_control.getTunerStatus(controller['ALLOC_ID']))
        except FRONTEND.NotSupportedException, e:
            status = self._getTunerStatusProp(controller['ALLOC_ID'])
        #pp(status)
        if ttype=='DDC':
            # get tuner status of parent CHAN/RDC... may be ambiguous
            chan_props = {'FRONTEND::tuner_status::group_id':status['FRONTEND::tuner_status::group_id'],
                          'FRONTEND::tuner_status::rf_flow_id':status['FRONTEND::tuner_status::rf_flow_id']}
            ddc_props = {'FRONTEND::tuner_status::tuner_type':'DDC'}
            try:
                chan_status = self._findTunerStatusProps(match=chan_props,notmatch=ddc_props)
            except KeyError:
                chan_status = None
            else:
                if len(chan_status) != 1:
                    # ambiguous or no match found, can't be sure we're checking correct COL_RF
                    chan_status = None
                else:
                    chan_status = chan_status[0]
        
        sri1 = dataSink1.sri()
        #print 'sri1',sri1
        self.checkAlmostEqual(status['FRONTEND::tuner_status::sample_rate'], 1.0/sri1.xdelta, '%s: SRI xdelta has correct value'%(comp_port_name),places=0)
        self.check(status['FRONTEND::tuner_status::complex'],sri1.mode,'%s: SRI mode has correct value'%(comp_port_name))
        
        # verify SRI keywords
        keywords = properties.props_to_dict(sri1.keywords)
        if 'COL_RF' in keywords:
            self.check(True,True,'%s: SRI has COL_RF keyword'%(comp_port_name))
            if ttype == 'DDC':
                if chan_status != None:
                    self.checkAlmostEqual(chan_status['FRONTEND::tuner_status::center_frequency'],keywords['COL_RF'],'%s: SRI keyword COL_RF has correct value'%(comp_port_name),places=0)
                else:
                    print 'WARNING - could not determine center frequency of collector to compare with COL_RF keyword'
            else:
                self.checkAlmostEqual(status['FRONTEND::tuner_status::center_frequency'],keywords['COL_RF'],'%s: SRI keyword COL_RF has correct value'%(comp_port_name),places=0)
        else:
            self.check(False,True,'%s: SRI has COL_RF keyword'%(comp_port_name))
            
        if 'CHAN_RF' in keywords:
            self.check(True,True,'%s: SRI has CHAN_RF keyword'%(comp_port_name))
            self.checkAlmostEqual(status['FRONTEND::tuner_status::center_frequency'],keywords['CHAN_RF'],'%s: SRI keyword CHAN_RF has correct value'%(comp_port_name),places=0)
        else:
            self.check(False,True,'%s: SRI has CHAN_RF keyword'%(comp_port_name))
            
        if 'FRONTEND::BANDWIDTH' in keywords:
            self.check(True,True,'%s: SRI has FRONTEND::BANDWIDTH keyword'%(comp_port_name))
            if not self.checkAlmostEqual(status['FRONTEND::tuner_status::bandwidth'],keywords['FRONTEND::BANDWIDTH'],'%s: SRI keyword FRONTEND::BANDWIDTH has correct value'%(comp_port_name),places=0):
                self.checkAlmostEqual(status['FRONTEND::tuner_status::sample_rate'],keywords['FRONTEND::BANDWIDTH'],'%s: SRI keyword FRONTEND::BANDWIDTH has sample rate value'%(comp_port_name),places=0, silentFailure=True, successMsg='WARN')
        else:
            self.check(False,True,'%s: SRI has FRONTEND::BANDWIDTH keyword'%(comp_port_name))
            
        if 'FRONTEND::RF_FLOW_ID' in keywords:
            self.check(True,True,'%s: SRI has FRONTEND::RF_FLOW_ID keyword'%(comp_port_name))
            self.check(status['FRONTEND::tuner_status::rf_flow_id'],keywords['FRONTEND::RF_FLOW_ID'],'%s: SRI keyword FRONTEND::RF_FLOW_ID has correct value'%(comp_port_name))
        else:
            self.check(False,True,'%s: SRI has FRONTEND::RF_FLOW_ID keyword'%(comp_port_name))
            
        if 'FRONTEND::DEVICE_ID' in keywords:
            self.check(True,True,'%s: SRI has FRONTEND::DEVICE_ID keyword'%(comp_port_name))
            #self.check(1,keywords['FRONTEND::DEVICE_ID'],'SRI keyword FRONTEND::DEVICE_ID has correct value')
        else:
            self.check(False,True,'%s: SRI has FRONTEND::DEVICE_ID keyword'%(comp_port_name))
    
        # verify multi-out port
        bad_conn_id = uuid.uuid4()
        comp_port_obj.connectPort(dataSink2_port_obj, bad_conn_id)
        time.sleep(1.0)
        data2 = dataSink2.getData()
        #print 'data2',len(data2)
        self.check(len(data2)>0,False,'%s: Did not receive data from tuner allocation with wrong alloc_id (multiport test)'%(comp_port_name))
        sri1 = dataSink1.sri()
        sri2 = dataSink2.sri()
        #print 'sri2',sri2
        self.check(sri1.streamID==sri2.streamID,False,'%s: Did not receive correct SRI from tuner allocation with wrong alloc_id (multiport test)'%(comp_port_name))
        
        if self.device_discovery[ttype] < 2:
            self.check(True,True,'%s: Cannot fully test multiport because only single %s tuner capability'%(comp_port_name,ttype),successMsg='info')
        else:
            pass # TODO - additional multiport tests here
    
        if listener1:
            # verify listener
            listener1 = self._generateListener(controller) # unique for each loop
            listenerAlloc1 = self._generateListenerAlloc(listener1)
            self.dut_ref.allocateCapacity(listenerAlloc1)
            comp_port_obj.connectPort(dataSink3_port_obj, listener1['LISTENER_ID'])
            time.sleep(1.0)
            data3 = dataSink3.getData()
            #print 'data3',len(data3)
            self.check(len(data3)>0,True,'%s: Received data from listener allocation'%(comp_port_name))
            sri1 = dataSink1.sri()
            sri3 = dataSink3.sri()
            #print 'sri3',sri3
            self.check(sri1.streamID==sri3.streamID,True,'%s: Received correct SRI from listener allocation'%(comp_port_name))
            
            # verify EOS
            if listener2:
                listener2 = self._generateListener(controller) # unique for each loop
                listenerAlloc2 = self._generateListenerAlloc(listener2)
                self.dut_ref.allocateCapacity(listenerAlloc2)               
                comp_port_obj.connectPort(dataSink4_port_obj, listener2['LISTENER_ID'])
                time.sleep(1.0)
                #for port_dict in port_list:       
                    #data4 = dataSink4.getData()
            self.dut_ref.deallocateCapacity(listenerAlloc1)
            self.check(dataSink3.eos(),True,'%s: Listener received EOS after deallocation of listener'%(comp_port_name))
            self.check(dataSink1.eos(),False,'%s: Controller did not receive EOS after deallocation of listener'%(comp_port_name))
            self.dut_ref.deallocateCapacity(tAlloc)
            self.check(dataSink1.eos(),True,'%s: Controller did receive EOS after deallocation of tuner'%(comp_port_name))
            if listener2:
                self.check(dataSink4.eos(),True,'%s: Listener received EOS after deallocation of tuner'%(comp_port_name))
                # cleanup listener2
                comp_port_obj.disconnectPort(listener2['LISTENER_ID'])
                self.dut_ref.deallocateCapacity(listenerAlloc2)
            # cleanup listener1
            comp_port_obj.disconnectPort(listener1['LISTENER_ID'])
        # cleanup controller
        comp_port_obj.disconnectPort(controller['ALLOC_ID'])
        comp_port_obj.disconnectPort(bad_conn_id)
            
    def _checkFrontEndTunerStatus(self,controller,listener1=None,listener2=None,analog_tuner=None,digital_tuner=None):
        tuner_control = digital_tuner or analog_tuner
        
        # make allocations
        controller_id = controller['ALLOC_ID']
        controller_alloc = self._generateAlloc(controller)
        retval = self.dut_ref.allocateCapacity(controller_alloc)
        if not retval:
            self.testReport.append('Could not allocate controller -- terminating test')
            return False
        if listener1:
            listener1_id = listener1['LISTENER_ID']
            listener1_alloc = self._generateListenerAlloc(listener1)
            retval = self.dut_ref.allocateCapacity(listener1_alloc)
            if not retval:
                self.testReport.append('Could not allocate listener1 -- limited test')
                listener1 = None
            elif listener2:
                listener2_id = listener2['LISTENER_ID']
                listener2_alloc = self._generateListenerAlloc(listener2)
                retval = self.dut_ref.allocateCapacity(listener2_alloc)
                if not retval:
                    self.testReport.append('Could not allocate listener2 -- limited test')
                    listener2 = None
        else:
            listener2 = None # if no 1st listener, ignore 2nd listener even if specified
        
        # Verify correct tuner status structure (fields, types)
        # check presence of tuner status property
        # check that it contains the required fields of correct data type
        # check which optional fields it contains, and that they are of correct data type
        # check for unknown/undefined fields
        try:
            status = self._getTunerStatusProp(controller_id)
        except KeyError:
            self.check(False, True, 'Device has FRONTEND::tuner_status property (failure, cannot complete test)')
        else:
            if status == None:
                self.check(False, True, 'Device has FRONTEND::tuner_status property (failure, cannot complete test)')
            else:
                self.check(True, True, 'Device has FRONTEND::tuner_status property')
                for name,dtype in self.FE_tuner_status_fields_req.items():
                    if status.has_key(name):
                        self.check(True, True, 'tuner_status has required field %s'%name)
                        self.check(type(status[name]) in dtype, True, 'value has correct data type for %s'%(name))
                    else:
                        self.check(False, True, 'tuner_status has required field %s'%name)
                for name,dtype in self.FE_tuner_status_fields_opt.items():
                    if status.has_key(name):
                        self.check(True, True, 'tuner_status has OPTIONAL field %s'%name)#, successMsg='yes')
                        self.check(type(status[name]) in dtype, True, 'value has correct data type for %s'%(name))
                    else:
                        self.check(False, True, 'tuner_status has OPTIONAL field %s'%name, failureMsg='no')
                all_names = self.FE_tuner_status_fields_req.keys()+self.FE_tuner_status_fields_opt.keys()
                for name in filter(lambda x: x not in all_names,status.keys()):
                    self.check(False, True, 'tuner_status has UNKNOWN field %s'%name, failureMsg='WARN')
    
        # Verify alloc_id_csv is populated after controller allocation
        try:
            status_val = self._getTunerStatusProp(controller_id,'FRONTEND::tuner_status::allocation_id_csv')
        except KeyError:
            pass
        else:
            if status_val == None:
                self.check(True, False, 'controller allocation id added to tuner status after allocation of controller (could not get tuner status prop)')
            else:
                self.check(controller_id, status_val.split(',')[0], 'controller allocation id added to tuner status after allocation of controller (must be first in CSV list)')
        
        # Verify tuner is enabled following allocation
        try:
            status_val = self._getTunerStatusProp(controller_id,'FRONTEND::tuner_status::enabled')
        except KeyError:
            pass
        else:
            if status_val == None:
                self.check(True, False, 'Tuner is enabled in tuner status after tuner allocation (could not get tuner status prop)')
            else:
                self.check(True, status_val, 'Tuner is enabled in tuner status after tuner allocation')

        if listener1:
            # Verify listener allocation id is added after allocation of listener        
            try:
                status_val = self._getTunerStatusProp(controller_id,'FRONTEND::tuner_status::allocation_id_csv')
            except KeyError:
                pass
            else:
                if status_val == None:
                    self.check(True, False, 'listener allocation id added to tuner status after allocation of listener (could not get tuner status prop)')
                else:
                    self.check(listener1_id in status_val.split(',')[1:], True, 'listener allocation id added to tuner status after allocation of listener (must not be first in CSV list)')
        
        if tuner_control:
            # Verify frequency prop
            try:
                val = tuner_control.getTunerCenterFrequency(controller_id)
            except FRONTEND.NotSupportedException, e:
                 pass
            else:
                try:
                    status_val = self._getTunerStatusProp(controller_id,'FRONTEND::tuner_status::center_frequency')
                except KeyError:
                    pass
                else:
                    self.checkAlmostEqual(status_val, val, 'correct value for FRONTEND::tuner_status::center_frequency property',places=0)
            #setTunerCenterFrequency
                
            # Verify bandwidth prop
            try:
                val = tuner_control.getTunerBandwidth(controller_id)
            except FRONTEND.NotSupportedException, e:
                 pass
            else:
                try:
                    status_val = self._getTunerStatusProp(controller_id,'FRONTEND::tuner_status::bandwidth')
                except KeyError:
                    pass
                else:
                    self.checkAlmostEqual(status_val, val, 'correct value for FRONTEND::tuner_status::bandwidth property',places=0)
            #setTunerBandwidth
            
            # Verify sample rate prop
            if digital_tuner:
                try:
                    val = tuner_control.getTunerOutputSampleRate(controller_id)
                except FRONTEND.NotSupportedException, e:
                     pass
                else:
                    try:
                        status_val = self._getTunerStatusProp(controller_id,'FRONTEND::tuner_status::sample_rate')
                    except KeyError:
                        pass
                    else:
                        self.checkAlmostEqual(status_val, val, 'correct value for FRONTEND::tuner_status::sample_rate property',places=0)
                #setTunerOutputSampleRate
            
            # Verify group id prop
            try:
                val = tuner_control.getTunerGroupId(controller_id)
            except FRONTEND.NotSupportedException, e:
                 pass
            else:
                try:
                    status_val = self._getTunerStatusProp(controller_id,'FRONTEND::tuner_status::group_id')
                except KeyError:
                    pass
                else:
                    self.check(status_val, val, 'correct value for FRONTEND::tuner_status::group_id property')
            
            # Verify rf flow id prop
            try:
                val = tuner_control.getTunerRfFlowId(controller_id)
            except FRONTEND.NotSupportedException, e:
                 pass
            else:
                try:
                    status_val = self._getTunerStatusProp(controller_id,'FRONTEND::tuner_status::rf_flow_id')
                except KeyError:
                    pass
                else:
                    self.check(status_val, val, 'correct value for FRONTEND::tuner_status::rf_flow_id property')
        
        if listener1:
            # Verify listener allocation id is removed after deallocation of listener
            self.dut_ref.deallocateCapacity(listener1_alloc)
            try:
                status_val = self._getTunerStatusProp(controller_id,'FRONTEND::tuner_status::allocation_id_csv')
            except KeyError:
                pass
            else:
                self.check(listener1_id in status_val, False, 'listener allocation id removed from tuner status after deallocation of listener')
            
        # Verify controller allocation id is removed after deallocation of controller
        self.dut_ref.deallocateCapacity(controller_alloc)
        try:
            status = self._getTunerStatusProp(controller_id)
        except KeyError:
            pass
        else:
            self.check(None, status, 'controller allocation id removed from tuner status after deallocation of controller')
        
        if listener2:
            # Verify listener allocation id is removed after deallocation of controller
            #self.dut_ref.deallocateCapacity(controllerAlloc)
            try:
                status = self._getTunerStatusProp(listener2_id)
            except KeyError:
                pass
            else:
                self.check(None, status, 'listener allocation id removed from tuner status after deallocation of controller')
                
    def _generateRX(self):
        raise NotImplementedError
    
    def _generateCHAN(self):
        raise NotImplementedError
            
    def _generateRD(self):
        #Pick a random set for CF,BW,SR and return
        value = {}
        value['ALLOC_ID'] = str(uuid.uuid4())
        value['TYPE'] = 'RX_DIGITIZER'
        value['BW_TOLERANCE'] = 100.0
        value['SR_TOLERANCE'] = 100.0
        value['RF_FLOW_ID'] = ''
        value['GROUP_ID'] = ''
        value['CONTROL'] = True
        if (DEVICE_INFO['RX_DIGITIZER']['CF_MIN'] != DEVICE_INFO['RX_DIGITIZER']['CF_MAX']):
            value['CF'] = float(random.randrange(DEVICE_INFO['RX_DIGITIZER']['CF_MIN'], DEVICE_INFO['RX_DIGITIZER']['CF_MAX'], 1.0e6))
        else:
            value['CF'] = float(DEVICE_INFO['RX_DIGITIZER']['CF_MIN'])
        
        if (DEVICE_INFO['RX_DIGITIZER']['BW_MIN'] != DEVICE_INFO['RX_DIGITIZER']['BW_MAX']):
            value['BW'] = float(random.randrange(DEVICE_INFO['RX_DIGITIZER']['BW_MIN'], DEVICE_INFO['RX_DIGITIZER']['BW_MAX'], 1.0e3))
        else:
            value['BW'] = float(DEVICE_INFO['RX_DIGITIZER']['BW_MIN'])
        
        if (DEVICE_INFO['RX_DIGITIZER']['SR_MIN'] != DEVICE_INFO['RX_DIGITIZER']['SR_MAX']):
            value['SR'] = float(random.randrange(DEVICE_INFO['RX_DIGITIZER']['SR_MIN'], DEVICE_INFO['RX_DIGITIZER']['SR_MAX'], 1.0e3))
        else:
            value['SR'] = float(DEVICE_INFO['RX_DIGITIZER']['SR_MIN'])
        return value
        
    def _generateRDC(self):
        #Pick a random set for CF,BW,SR and return
        value = {}
        value['ALLOC_ID'] = str(uuid.uuid4())
        value['TYPE'] = 'RX_DIGITIZER_CHANNELIZER'
        value['BW_TOLERANCE'] = 1.0
        value['SR_TOLERANCE'] = 1.0
        value['RF_FLOW_ID'] = ''
        value['GROUP_ID'] = ''
        value['CONTROL'] = True
        if (DEVICE_INFO['RX_DIGITIZER_CHANNELIZER']['CF_MIN'] != DEVICE_INFO['RX_DIGITIZER_CHANNELIZER']['CF_MAX']):
            value['CF'] = float(random.randrange(DEVICE_INFO['RX_DIGITIZER_CHANNELIZER']['CF_MIN'], DEVICE_INFO['RX_DIGITIZER_CHANNELIZER']['CF_MAX'], 1.0e6))
        else:
            value['CF'] = float(DEVICE_INFO['RX_DIGITIZER_CHANNELIZER']['CF_MIN'])
        
        if (DEVICE_INFO['RX_DIGITIZER_CHANNELIZER']['BW_MIN'] != DEVICE_INFO['RX_DIGITIZER_CHANNELIZER']['BW_MAX']):
            value['BW'] = float(random.randrange(DEVICE_INFO['RX_DIGITIZER_CHANNELIZER']['BW_MIN'], DEVICE_INFO['RX_DIGITIZER_CHANNELIZER']['BW_MAX'], 1.0e3))
        else:
            value['BW'] = float(DEVICE_INFO['RX_DIGITIZER_CHANNELIZER']['BW_MIN'])
        
        if (DEVICE_INFO['RX_DIGITIZER_CHANNELIZER']['SR_MIN'] != DEVICE_INFO['RX_DIGITIZER_CHANNELIZER']['SR_MAX']):
            value['SR'] = float(random.randrange(DEVICE_INFO['RX_DIGITIZER_CHANNELIZER']['SR_MIN'], DEVICE_INFO['RX_DIGITIZER_CHANNELIZER']['SR_MAX'], 1.0e3))
        else:
            value['SR'] = float(DEVICE_INFO['RX_DIGITIZER_CHANNELIZER']['SR_MIN'])
        return value

    def _generateDDC(self, chan):
        #Pick a random set for CF,BW,SR and return
        value = {}
        value['ALLOC_ID'] = str(uuid.uuid4())
        value['TYPE'] = 'DDC'
        value['BW_TOLERANCE'] = 100.0
        value['SR_TOLERANCE'] = 100.0
        value['RF_FLOW_ID'] = chan['RF_FLOW_ID']
        value['GROUP_ID'] = chan['GROUP_ID']
        value['CONTROL'] = True
        if (DEVICE_INFO['DDC']['CF_MIN'] != DEVICE_INFO['DDC']['CF_MAX']):
            value['CF'] = float(random.randrange(DEVICE_INFO['DDC']['CF_MIN'], DEVICE_INFO['DDC']['CF_MAX'], 1.0e3))
        else:
            value['CF'] = float(DEVICE_INFO['DDC']['CF_MIN'])
        value['CF'] = float(value['CF']+chan['CF'])
        
        if (DEVICE_INFO['DDC']['BW_MIN'] != DEVICE_INFO['DDC']['BW_MAX']):
            value['BW'] = float(random.randrange(DEVICE_INFO['DDC']['BW_MIN'], DEVICE_INFO['DDC']['BW_MAX'], 1.0e3))
        else:
            value['BW'] = float(DEVICE_INFO['DDC']['BW_MIN'])
        
        if (DEVICE_INFO['DDC']['SR_MIN'] != DEVICE_INFO['DDC']['SR_MAX']):
            value['SR'] = float(random.randrange(DEVICE_INFO['DDC']['SR_MIN'], DEVICE_INFO['DDC']['SR_MAX'], 1.0e3))
        else:
            value['SR'] = float(DEVICE_INFO['DDC']['SR_MIN'])
        return value

    def _generateListener(self, c):
        value = {}
        value['LISTENER_ID'] = str(uuid.uuid4()) 
        value['ALLOC_ID'] = c['ALLOC_ID']
        return value

    def _generateListenerAlloc(self, value):
        allocationPropDict = {'FRONTEND::listener_allocation':{
                    'FRONTEND::listener_allocation::existing_allocation_id': value['ALLOC_ID'],
                    'FRONTEND::listener_allocation::listener_allocation_id': value['LISTENER_ID'],
                    }}
        return properties.props_from_dict(allocationPropDict)

    def _generateAlloc(self, value):
        #generate the allocation
        allocationPropDict = {'FRONTEND::tuner_allocation':{
                    'FRONTEND::tuner_allocation::tuner_type': value['TYPE'],
                    'FRONTEND::tuner_allocation::allocation_id': value['ALLOC_ID'],
                    'FRONTEND::tuner_allocation::center_frequency': value['CF'],
                    'FRONTEND::tuner_allocation::bandwidth': value['BW'],
                    'FRONTEND::tuner_allocation::bandwidth_tolerance': value['BW_TOLERANCE'],
                    'FRONTEND::tuner_allocation::sample_rate': value['SR'],
                    'FRONTEND::tuner_allocation::sample_rate_tolerance': value['SR_TOLERANCE'],
                    'FRONTEND::tuner_allocation::device_control': value['CONTROL'],
                    'FRONTEND::tuner_allocation::group_id': value['GROUP_ID'],
                    'FRONTEND::tuner_allocation::rf_flow_id': value['RF_FLOW_ID'],
                    }}
        return properties.props_from_dict(allocationPropDict)
        
    def _generateRFInfoPkt(self,rf_flow_id,chan):
        rf_center_freq=float(chan['CF'])
        rf_bandwidth=float(chan['BW'])
        if_center_freq = 70.0e6
        spectrum_inverted=False
        sensor=FRONTEND.SensorInfo(mission='TEST_MSSN', collector='TEST_COLLECTOR', rx='TEST_RX', 
                                   antenna='TEST_ANT', feed='TEST_FEED')
        ext_path_delays=[FRONTEND.PathDelay(freq=float(chan['CF']), delay_ns=10.0)]
        freq_range=FRONTEND.FreqRange(max_val=1.0e9,min_val=0.001e9,values=[0.001e9,float(chan['CF']),1.0e9])
        bw_range=FRONTEND.FreqRange(max_val=1.0e6,min_val=0.001e6,values=[0.001e6,float(chan['BW']),1.0e6])
        capabilities=FRONTEND.RFCapabilities(freq_range, bw_range)
        additional_info=properties.props_from_dict({'test_info':'test_value'})
        rfinfo_pkt = FRONTEND.RFInfoPkt(rf_flow_id=rf_flow_id, rf_center_freq=rf_center_freq, rf_bandwidth=rf_bandwidth, 
                                        if_center_freq=if_center_freq, spectrum_inverted=spectrum_inverted, 
                                        sensor=sensor, ext_path_delays=ext_path_delays, capabilities=capabilities, 
                                        additional_info=additional_info)
        return rfinfo_pkt
    
    class PortFRONTENDRFInfoIn_i(FRONTEND__POA.RFInfo):
        rf_flow_id = None
        rfinfo_pkt = None
        def __init__(self, parent, name):
            self.parent = parent
            self.name = name
            self.sri = None
            self.queue = Queue.Queue()
            self.port_lock = threading.Lock()
            self.rf_flow_id = ''
            self.rfinfo_pkt = None
            
        def _get_rf_flow_id(self):
            retVal = ''
            self.port_lock.acquire()
            retVal = self.rf_flow_id
            self.port_lock.release()
            return retVal
        
        def _set_rf_flow_id(self, data):
            self.port_lock.acquire()
            self.rf_flow_id=data
            self.port_lock.release()
        
        def _get_rfinfo_pkt(self):
            retVal = ''
            self.port_lock.acquire()
            retVal = self.rfinfo_pkt
            self.port_lock.release()
            return retVal
        
        def _set_rfinfo_pkt(self, data):
            self.port_lock.acquire()
            self.rfinfo_pkt=data
            self.port_lock.release()
    
    
###############################################
## BACKUP CODE FOR POSSIBLE FUTURE ADDITIONS ##
###############################################

'''

        # Verify Basic Data Flow
        rdc = self._generateRDC()
        rdcAlloc = self._generateAlloc(rdc)  
        ddc = self._generateDDC(rdc)
        ddcAlloc = self._generateAlloc(ddc)
        #Confirm data port
        DataShortPort = self.dut.getPort('dataShort_out')     
        DataShortPort._narrow(BULKIO.dataShort)
        self.assertTrue(DataShortPort != None)
        self.assertFalse(CORBA.is_nil(DataShortPort))  
        #make connection to DDC
        basicPortObj = bulkio_data_helpers.ArraySink(BULKIO__POA.dataShort)
        portRef = basicPortObj.getPort()
        DataShortPort.connectPort(portRef, ddc['ALLOC_ID'])
        time.sleep(2)
        #verify no data
        self.assertTrue(len(basicPortObj.data) == 0)
        #turn on data then verify that there is data
        self.assertTrue(self.dut_ref.allocateCapacity(rdcAlloc))
        self.assertTrue(self.dut_ref.allocateCapacity(ddcAlloc)) 
        time.sleep(2)
        self.assertTrue(len(basicPortObj.data) > 0)
        
        # Verify multi-out
        #generate another ddc to make connection with
        ddc2 = self._generateDDC(rdc)
        ddc2Alloc = self._generateAlloc(ddc2)
        DataShortPort2 = self.dut.getPort('dataShort_out')     
        DataShortPort2._narrow(BULKIO.dataShort)
        self.assertTrue(DataShortPort2 != None)
        self.assertFalse(CORBA.is_nil(DataShortPort2))  
        #make connection to DDC
        basicPortObj2 = bulkio_data_helpers.ArraySink(BULKIO__POA.dataShort)
        portRef = basicPortObj2.getPort()
        DataShortPort.connectPort(portRef, ddc2['ALLOC_ID'])
        time.sleep(2)
        #verify no data, then data
        self.assertTrue(len(basicPortObj2.data) == 0)        
        self.assertTrue(self.dut_ref.allocateCapacity(ddc2Alloc)) 
        time.sleep(2)
        self.assertTrue(len(basicPortObj2.data) > 0)
'''
