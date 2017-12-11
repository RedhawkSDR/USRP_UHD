#!/bin/env python

import os, sys, time, re
from pprint import pprint as pp
from ossie.utils import sb
from frontend import createTunerAllocation
from frontend.tuner_device import floatingPointCompare

# TODO - modify this with the correct IP address for the target USRP
USRP_IP = '192.168.20.2'
#USRP_IP = '192.168.10.2'

# TODO - modify this for the daughterboard being tested
dboard        = 'WBX40'   # dboard being tested (WBX40, CBX120, or SBX120)
dboard_ignore = None      # If two dboards installed, this is the one not being tested

#dboard        = 'CBX120'  # dboard being tested (WBX40, CBX120, or SBX120)
#dboard_ignore = 'SBX120'  # If two dboards installed, this is the one not being tested

#dboard        = 'SBX120'  # dboard being tested (WBX40, CBX120, or SBX120)
#dboard_ignore = 'CBX120'  # If two dboards installed, this is the one not being tested

print '=========================================================================='
print 'Testing dboard %s of USRP at IP %s'%(dboard,USRP_IP)
if dboard_ignore: print '  (ignoring second dboard %s)'%dboard_ignore
print 'Note: Overflow may occur if network interface cannot support full data rate'
print '=========================================================================='

DBOARDS = {}

# WBX-40 values
DBOARDS['WBX40'] = {
  'NAME'       : 'WBX',
  'CF_MIN'     : 48.75e6,
  'CF_MAX'     : 2.22e9,
  'RX_SR_MIN'  : 195312.5,
  'RX_SR_MAX'  : 50e6,
  'TX_SR_MIN'  : 195312.5,
  'TX_SR_MAX'  : 50e6,
  'BW_MIN'     : 40e6,
  'BW_MAX'     : 40e6,
  'RX_GAIN_MIN': 0.0,
  'RX_GAIN_MAX': 38.0,
  'TX_GAIN_MIN': 0.0,
  'TX_GAIN_MAX': 25.0,
  'DEFAULT_CF' : 100e6,
  'DEFAULT_SR' : 0.0,
  'DEFAULT_BW' : 0.0
}

# SBX-120 values
DBOARDS['SBX120'] = {
  'NAME'        :  'SBX',
  'CF_MIN'      : 340.0e6,
  'CF_MAX'      : 4.46e9,
  'RX_SR_MIN'   : 195312.5,
  'RX_SR_MAX'   : 200.0e6,
  'TX_SR_MIN'   : 390625.0,
  'TX_SR_MAX'   : 200.0e6,
  'BW_MIN'      : 120.0e6,
  'BW_MAX'      : 120.0e6,
  'RX_GAIN_MIN' : 0.0,
  'RX_GAIN_MAX' : 37.5,
  'TX_GAIN_MIN' : 0.0,
  'TX_GAIN_MAX' : 31.5,
  'DEFAULT_CF' : 2e9,
  'DEFAULT_SR' : 0.0,
  'DEFAULT_BW' : 0.0
}

# CBX-120 values
DBOARDS['CBX120'] = {
  'NAME'       : 'CBX',
  'CF_MIN'     : 1.14e9,
  'CF_MAX'     : 6.06e9,
  'RX_SR_MIN'  : 195312.5,
  'RX_SR_MAX'  : 200.0e6,
  'TX_SR_MIN'  : 390625.0,
  'TX_SR_MAX'  : 200.0e6,
  'BW_MIN'     : 120.0e6,
  'BW_MAX'     : 120.0e6,
  'RX_GAIN_MIN': 0.0,
  'RX_GAIN_MAX': 37.5,
  'TX_GAIN_MIN': 0.0,
  'TX_GAIN_MAX': 31.5,
  'DEFAULT_CF' : 2e9,
  'DEFAULT_SR' : 0.0,
  'DEFAULT_BW' : 0.0
}

# for easier access
DBOARD = DBOARDS[dboard]
DBOARD_IGNORE = None
if dboard_ignore in DBOARDS:
  DBOARD_IGNORE = DBOARDS[dboard_ignore]

def assertEqual(a,b,msg=None):
  if msg:
    msg = '%s: %s %s %s ('+msg+')'
  else:
    msg = '%s: %s %s %s'
  if a == None or b == None or floatingPointCompare(a,b) != 0:
    print '    '+msg%('Fail',a,'does not equal',b)
    return False
  else:
    print '    '+msg%('PASS',a,'equals',b)
    return True

def runAllocTuneTest(msg, key, expected, dut, ttype, alloc_id='test_alloc',
                 cf=DBOARD['DEFAULT_CF'], sr=DBOARD['DEFAULT_SR'], bw=DBOARD['DEFAULT_BW'],
                 rx_gain=None, tx_gain=None, cf2=None, sr2=None, bw2=None, dealloc=True):
  alloc = createTunerAllocation( tuner_type = ttype, allocation_id = alloc_id,
                                 center_frequency = cf, bandwidth = bw, sample_rate = sr)
  actual = None
  try:
    if not dut.allocateCapacity(alloc):
      dealloc = True
    else:
      if cf2 != None:
        port = dut.getPort('DigitalTuner_in')
        port.setTunerCenterFrequency(alloc_id,cf2)
      elif sr2 != None:
        port = dut.getPort('DigitalTuner_in')
        port.setTunerOutputSampleRate(alloc_id,sr2)
      elif bw2 != None:
        port = dut.getPort('DigitalTuner_in')
        port.setTunerBandwidth(alloc_id,bw2)
      elif rx_gain != None:
        dut.device_rx_gain_global = rx_gain
      elif tx_gain != None:
        dut.device_tx_gain_global = tx_gain
      for tuner in dut.frontend_tuner_status:
        if alloc_id in tuner['FRONTEND::tuner_status::allocation_id_csv']:
          actual = tuner[key]
          break
  except Exception, e:
    print 'Exception', e
    dealloc = True
  if dealloc:
    dut.deallocateCapacity(alloc)
  return assertEqual(actual, expected, msg)

def runAvailableRangeTest(msg, key, expected_min, expected_max, dut, ttype):
  idx = None
  for idx, chan in enumerate(dut.device_channels):
    if DBOARD['NAME'] in chan['device_channels::ch_name'] and ttype == chan['device_channels::tuner_type']:
      break
  else:
    idx = None
  if idx != None:
    actual = dut.frontend_tuner_status[idx][key]
    actual = re.split(r'[,\-;]', actual)
    actual_min = float(actual[0])
    actual_max = float(actual[-1])
    passing = assertEqual(actual_min, expected_min, msg+' MIN')
    passing &= assertEqual(actual_max, expected_max, msg+' MAX')
    return passing
  else:
    return assertEqual(None, [expected_min,expected_max], msg)

# launch device
dut = sb.launch('rh.USRP_UHD')
dut.target_device.ip_address = USRP_IP
time.sleep(1)

# verify values
print '=========================================================================='
print 'Verifying values reported by UHD match test values'
print '=========================================================================='
passing = True
for chan in xrange(len(dut.device_channels)):
  #pp(dut.device_channels[chan])
  if dut.device_channels[chan].tuner_type not in ['RX_DIGITIZER','TX']:
    print 'Skipping tuner type:', dut.device_channels[chan].tuner_type
    continue
  if DBOARD['NAME'] not in dut.device_channels[chan].ch_name:
    print 'Skipping channel that is not %s (%s)'%(DBOARD['NAME'],dut.device_channels[chan].ch_name)
    continue
  passing &= assertEqual(dut.device_channels[chan].freq_min,      DBOARD['CF_MIN'], 'cf min')
  passing &= assertEqual(dut.device_channels[chan].freq_max,      DBOARD['CF_MAX'], 'cf max')
  passing &= assertEqual(dut.device_channels[chan].bandwidth_min, DBOARD['BW_MIN'], 'bw min')
  passing &= assertEqual(dut.device_channels[chan].bandwidth_max, DBOARD['BW_MAX'], 'bw max')
  if dut.device_channels[chan].tuner_type == 'RX_DIGITIZER':
    passing &= assertEqual(dut.device_channels[chan].rate_min, DBOARD['RX_SR_MIN'],   'rx sr min')
    passing &= assertEqual(dut.device_channels[chan].rate_max, DBOARD['RX_SR_MAX'],   'rx sr max')
    passing &= assertEqual(dut.device_channels[chan].gain_min, DBOARD['RX_GAIN_MIN'], 'rx gain min')
    passing &= assertEqual(dut.device_channels[chan].gain_max, DBOARD['RX_GAIN_MAX'], 'rx gain max')
  elif dut.device_channels[chan].tuner_type == 'TX':
    passing &= assertEqual(dut.device_channels[chan].rate_min, DBOARD['TX_SR_MIN'],   'tx sr min')
    passing &= assertEqual(dut.device_channels[chan].rate_max, DBOARD['TX_SR_MAX'],   'tx sr max')
    passing &= assertEqual(dut.device_channels[chan].gain_min, DBOARD['TX_GAIN_MIN'], 'tx gain min')
    passing &= assertEqual(dut.device_channels[chan].gain_max, DBOARD['TX_GAIN_MAX'], 'tx gain max')

if passing:
  print '=========================================================================='
  print dboard, 'PASSED VALUE CHECK'
  print '=========================================================================='
else:
  print '=========================================================================='
  print dboard, 'FAILED VALUE CHECK'
  print '=========================================================================='
  sys.exit(1)

# run verification tests

# allocate dboard to ignore
if DBOARD_IGNORE:
  print '=========================================================================='
  print 'Ignoring other dboard tuners by allocating them with output disabled'
  print '=========================================================================='
  port = dut.getPort('DigitalTuner_in')
  for chan in xrange(len(dut.device_channels)):
    if dut.device_channels[chan].tuner_type not in ['RX_DIGITIZER','TX']:
      continue
    if DBOARD_IGNORE['NAME'] not in dut.device_channels[chan].ch_name:
      continue

    # allocate as many as will succeed
    cnt = ord('A')
    alloc_id = 'ignore_tuner_%s.%s'%(chan,chr(cnt))
    alloc = createTunerAllocation( tuner_type = str(dut.device_channels[chan].tuner_type),
                                   allocation_id = alloc_id,
                                   center_frequency = DBOARD_IGNORE['DEFAULT_CF'],
                                   bandwidth = DBOARD_IGNORE['DEFAULT_BW'],
                                   sample_rate = DBOARD_IGNORE['DEFAULT_SR'])
    success = True
    while success:
      try:
        success = dut.allocateCapacity(alloc)
      except:
        #print 'Exception when allocating a %s (%s)'%(dut.device_channels[chan].tuner_type,alloc_id)
        success = False
      else:
        if success:
          #print 'Allocated a %s (%s)'%(dut.device_channels[chan].tuner_type,alloc_id)
          cnt+=1
          port.setTunerEnable(alloc_id, False)
          alloc_id = 'ignore_tuner_%s.%s'%(chan,chr(cnt))
          alloc = createTunerAllocation( tuner_type = str(dut.device_channels[chan].tuner_type),
                                         allocation_id = alloc_id,
                                         center_frequency = DBOARD_IGNORE['DEFAULT_CF'],
                                         bandwidth = DBOARD_IGNORE['DEFAULT_BW'],
                                         sample_rate = DBOARD_IGNORE['DEFAULT_SR'])
        #else:
        #  print 'Failed to allocate a %s (%s)'%(dut.device_channels[chan].tuner_type,alloc_id)

  # Now deallocate any that happen to be for dboard we are testing
  for chan in xrange(len(dut.device_channels)):
    if dut.device_channels[chan].tuner_type not in ['RX_DIGITIZER','TX']:
      continue
    alloc_id = dut.frontend_tuner_status[chan].allocation_id_csv
    if not alloc_id:
      continue
    if DBOARD['NAME'] not in dut.device_channels[chan].ch_name:
      print 'Ignoring other dboard: %s #%s (%s)'%(dut.device_channels[chan].ch_name,chan,alloc_id)
      continue
    print 'Freeing for testing: %s #%s (%s)'%(dut.device_channels[chan].ch_name,chan,alloc_id)
    #tuner['FRONTEND::tuner_status::allocation_id_csv']:
    alloc = createTunerAllocation( tuner_type = str(dut.device_channels[chan].tuner_type),
                                   allocation_id = str(alloc_id),
                                   center_frequency = DBOARD_IGNORE['DEFAULT_CF'],
                                   bandwidth = DBOARD_IGNORE['DEFAULT_BW'],
                                   sample_rate = DBOARD_IGNORE['DEFAULT_SR'])
    dut.deallocateCapacity(alloc)

# put USRP in 8-bit mode for max sr tests
dut.device_rx_mode = '8bit'
dut.device_tx_mode = '8bit'

print '=========================================================================='
print 'Testing RX capabilities'
print '=========================================================================='
# allocate an RX_Digitizer with CF_MIN 
msg = 'RX alloc cf min'
key = 'FRONTEND::tuner_status::center_frequency'
expected = DBOARD['CF_MIN']
passing &= runAllocTuneTest(msg, key, expected, dut, 'RX_DIGITIZER', cf=expected)

# allocate an RX_Digitizer with CF_MAX 
msg = 'RX alloc cf max'
key = 'FRONTEND::tuner_status::center_frequency'
expected = DBOARD['CF_MAX']
passing &= runAllocTuneTest(msg, key, expected, dut, 'RX_DIGITIZER', cf=expected)

# allocate an RX_Digitizer then tune to CF_MIN 
msg = 'RX tune cf min'
key = 'FRONTEND::tuner_status::center_frequency'
expected = DBOARD['CF_MIN']
passing &= runAllocTuneTest(msg, key, expected, dut, 'RX_DIGITIZER', cf2=expected)

# allocate an RX_Digitizer then tune to CF_MAX 
msg = 'RX tune cf max'
key = 'FRONTEND::tuner_status::center_frequency'
expected = DBOARD['CF_MAX']
passing &= runAllocTuneTest(msg, key, expected, dut, 'RX_DIGITIZER', cf2=expected)

# allocate an RX_Digitizer with RX_SR_MIN 
msg = 'RX alloc sr min'
key = 'FRONTEND::tuner_status::sample_rate'
expected = DBOARD['RX_SR_MIN']
passing &= runAllocTuneTest(msg, key, expected, dut, 'RX_DIGITIZER', sr=expected)

# allocate an RX_Digitizer with RX_SR_MAX (note: device_rx_mode should be 8-bit to avoid overflow) 
msg = 'RX alloc sr max'
key = 'FRONTEND::tuner_status::sample_rate'
expected = DBOARD['RX_SR_MAX']
passing &= runAllocTuneTest(msg, key, expected, dut, 'RX_DIGITIZER', sr=expected)

# allocate an RX_Digitizer with SR 1 MHz, then tune to RX_SR_MIN 
msg = 'RX tune sr min'
key = 'FRONTEND::tuner_status::sample_rate'
expected = DBOARD['RX_SR_MIN']
passing &= runAllocTuneTest(msg, key, expected, dut, 'RX_DIGITIZER', sr=1e6, sr2=expected)

# allocate an RX_Digitizer with SR 1 MHz, then tune to RX_SR_MAX (note: device_rx_mode should be 8-bit to avoid overflow) 
msg = 'RX tune sr max'
key = 'FRONTEND::tuner_status::sample_rate'
expected = DBOARD['RX_SR_MAX']
passing &= runAllocTuneTest(msg, key, expected, dut, 'RX_DIGITIZER', sr=1e6, sr2=expected)

# allocate an RX_Digitizer then tune to RX_GAIN_MIN (must use Device Property device_rx_gain_global) 
msg = 'RX gain min'
key = 'FRONTEND::tuner_status::gain'
expected = DBOARD['RX_GAIN_MIN']
passing &= runAllocTuneTest(msg, key, expected, dut, 'RX_DIGITIZER', rx_gain=expected)

# allocate an RX_Digitizer then tune to RX_GAIN_MAX (must use Device Property device_rx_gain_global) 
msg = 'RX gain max'
key = 'FRONTEND::tuner_status::gain'
expected = DBOARD['RX_GAIN_MAX']
passing &= runAllocTuneTest(msg, key, expected, dut, 'RX_DIGITIZER', rx_gain=expected)

# view the frontend_tuner_status.available_center_frequency Property associated with an RX_Digitizer, confirm range CF_MIN to CF_MAX 
msg = 'RX avail cf range'
key = 'FRONTEND::tuner_status::available_frequency'
expected_min = DBOARD['CF_MIN']
expected_max = DBOARD['CF_MAX']
passing &= runAvailableRangeTest(msg, key, expected_min, expected_max, dut, 'RX_DIGITIZER')

# view the frontend_tuner_status.available_sample_rate Property associated with an RX_Digitizer, confirm range RX_SR_MIN to RX_SR_MAX 
msg = 'RX avail sr range'
key = 'FRONTEND::tuner_status::available_sample_rate'
expected_min = DBOARD['RX_SR_MIN']
expected_max = DBOARD['RX_SR_MAX']
passing &= runAvailableRangeTest(msg, key, expected_min, expected_max, dut, 'RX_DIGITIZER')

# view the frontend_tuner_status.available_bandwidth Property associated with an RX_Digitizer, confirm range min(BW_MIN,RX_SR_MIN) to min(BW_MAX,RX_SR_MAX) 
msg = 'RX avail bw range'
key = 'FRONTEND::tuner_status::available_bandwidth'
expected_min = min(DBOARD['BW_MIN'],DBOARD['RX_SR_MIN'])
expected_max = min(DBOARD['BW_MAX'],DBOARD['RX_SR_MAX'])
passing &= runAvailableRangeTest(msg, key, expected_min, expected_max, dut, 'RX_DIGITIZER')

# view the frontend_tuner_status.available_gain Property associated with an RX_Digitizer, confirm range RX_GAIN_MIN to RX_GAIN_MAX 
msg = 'RX avail gain range'
key = 'FRONTEND::tuner_status::available_gain'
expected_min = DBOARD['RX_GAIN_MIN']
expected_max = DBOARD['RX_GAIN_MAX']
passing &= runAvailableRangeTest(msg, key, expected_min, expected_max, dut, 'RX_DIGITIZER')

print '=========================================================================='
print 'Testing TX capabilities'
print '=========================================================================='
# allocate a TX with CF_MIN 
msg = 'TX alloc cf min'
key = 'FRONTEND::tuner_status::center_frequency'
expected = DBOARD['CF_MIN']
passing &= runAllocTuneTest(msg, key, expected, dut, 'TX', cf=expected)

# allocate a TX with CF_MAX 
msg = 'TX alloc cf max'
key = 'FRONTEND::tuner_status::center_frequency'
expected = DBOARD['CF_MAX']
passing &= runAllocTuneTest(msg, key, expected, dut, 'TX', cf=expected)

# allocate a TX then tune to CF_MIN 
msg = 'TX tune cf min'
key = 'FRONTEND::tuner_status::center_frequency'
expected = DBOARD['CF_MIN']
passing &= runAllocTuneTest(msg, key, expected, dut, 'TX', cf2=expected)

# allocate a TX then tune to CF_MAX 
msg = 'TX tune cf max'
key = 'FRONTEND::tuner_status::center_frequency'
expected = DBOARD['CF_MAX']
passing &= runAllocTuneTest(msg, key, expected, dut, 'TX', cf2=expected)

# allocate a TX with TX_SR_MIN 
msg = 'TX alloc sr min'
key = 'FRONTEND::tuner_status::sample_rate'
expected = DBOARD['TX_SR_MIN']
passing &= runAllocTuneTest(msg, key, expected, dut, 'TX', sr=expected)

# allocate a TX with TX_SR_MAX (note: device_tx_mode should be 8-bit to avoid overflow) 
msg = 'TX alloc sr max'
key = 'FRONTEND::tuner_status::sample_rate'
expected = DBOARD['TX_SR_MAX']
passing &= runAllocTuneTest(msg, key, expected, dut, 'TX', sr=expected)

# allocate a TX with SR 1 MHz, then tune to TX_SR_MIN 
msg = 'TX tune sr min'
key = 'FRONTEND::tuner_status::sample_rate'
expected = DBOARD['TX_SR_MIN']
passing &= runAllocTuneTest(msg, key, expected, dut, 'TX', sr=1e6, sr2=expected)

# allocate a TX with SR 1 MHz, then tune to TX_SR_MAX (note: device_tx_mode should be 8-bit to avoid overflow) 
msg = 'TX tune sr max'
key = 'FRONTEND::tuner_status::sample_rate'
expected = DBOARD['TX_SR_MAX']
passing &= runAllocTuneTest(msg, key, expected, dut, 'TX', sr=1e6, sr2=expected)

# allocate a TX then tune to TX_GAIN_MIN (must use Device Property device_tx_gain_global) 
msg = 'TX gain min'
key = 'FRONTEND::tuner_status::gain'
expected = DBOARD['TX_GAIN_MIN']
passing &= runAllocTuneTest(msg, key, expected, dut, 'TX', tx_gain=expected)

# allocate a TX then tune to TX_GAIN_MAX (must use Device Property device_tx_gain_global) 
msg = 'TX gain max'
key = 'FRONTEND::tuner_status::gain'
expected = DBOARD['TX_GAIN_MAX']
passing &= runAllocTuneTest(msg, key, expected, dut, 'TX', tx_gain=expected)

# view the frontend_tuner_status.available_center_frequency Property associated with a TX, confirm range CF_MIN to CF_MAX 
msg = 'TX avail cf range'
key = 'FRONTEND::tuner_status::available_frequency'
expected_min = DBOARD['CF_MIN']
expected_max = DBOARD['CF_MAX']
passing &= runAvailableRangeTest(msg, key, expected_min, expected_max, dut, 'TX')

# view the frontend_tuner_status.available_sample_rate Property associated with a TX, confirm range TX_SR_MIN to TX_SR_MAX 
msg = 'TX avail sr range'
key = 'FRONTEND::tuner_status::available_sample_rate'
expected_min = DBOARD['TX_SR_MIN']
expected_max = DBOARD['TX_SR_MAX']
passing &= runAvailableRangeTest(msg, key, expected_min, expected_max, dut, 'TX')

# view the frontend_tuner_status.available_bandwidth Property associated with a TX, confirm range min(BW_MIN,TX_SR_MIN) to min(BW_MAX,TX_SR_MAX) 
msg = 'TX avail bw range'
key = 'FRONTEND::tuner_status::available_bandwidth'
expected_min = min(DBOARD['BW_MIN'],DBOARD['TX_SR_MIN'])
expected_max = min(DBOARD['BW_MAX'],DBOARD['TX_SR_MAX'])
passing &= runAvailableRangeTest(msg, key, expected_min, expected_max, dut, 'TX')

# view the frontend_tuner_status.available_gain Property associated with a TX, confirm range TX_GAIN_MIN to TX_GAIN_MAX 
msg = 'TX avail gain range'
key = 'FRONTEND::tuner_status::available_gain'
expected_min = DBOARD['TX_GAIN_MIN']
expected_max = DBOARD['TX_GAIN_MAX']
passing &= runAvailableRangeTest(msg, key, expected_min, expected_max, dut, 'TX')


print '=========================================================================='
if passing:
  print dboard, 'PASSED ALL TESTS'
else:
  print dboard, 'FAILED ONE OR MORE TESTS'
print '=========================================================================='

