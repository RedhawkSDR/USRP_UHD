# REDHAWK FEI Compliance Test Results

## Execution

To run [test\_USRP_UHD\_FEI.py](test_USRP_UHD_FEI.py), execute the following command from the `USRP_UHD/tests` directory:

```
python test_USRP_UHD_FEI.py
```

The test may take several minutes to perform the 268 checks when successful. It is common that fewer checks are made when unexpected failures occur, which prevent all checks from being made.

## Important Note

The unit test will fail to run if the USRP hardware is not loaded with firmware and fpga images that are compatible with the UHD driver installed on the host computer. Execute `uhd_usrp_probe` (optionally with `--args=addr=<IP address>`, if needed) to diagnose the issue. If the command executes successfully, the firmware and fpga images are compatible. Also note that `uhd_find_devices` will execute successfully even with incompatible firmware/fpga images, and that the `uhd_usrp_probe` command must be used instead.

## Warnings

The UHD driver prints several warnings and errors to the console that can be ignored for unit and compliance testing. Several examples of such warnings are given below.

The following warning typically occurs when the user is not a member of the `usrp` group:
```
UHD Warning:
    Unable to set the thread priority. Performance may be negatively affected.
    Please see the general application notes in the manual for instructions.
    EnvironmentError: OSError: error in pthread_setschedparam
```

The following warning indicates that 16-bit complex samples at 50 MSps exceeds the maximum capacity of the Ethernet connection. Overflow warnings can be ignored for unit testing since overflow is acceptable when checking command and control of the device. For applications that require 50 MSps, the USRP must be configured to use 8-bit complex samples to avoid overflow.
```
UHD Warning:
    The total sum of rates (50.000000 MSps on 1 channels) exceeds the maximum capacity of the connection.
    This can cause overflows (O).
```

The following warning indicates that the chosen sample rate is not ideal. The sample rate values are randomly generated during testing and it is common that some requested sample rates result in odd decimation. Odd decimation warnings can be ignored for unit testing since CIC rolloff is not of concern.
```
UHD Warning:
    The requested decimation is odd; the user should expect CIC rolloff.
    decimation = dsp_rate/samp_rate -> 13 = (100.000000 MHz)/(7.692308 MHz)
```



## Results

### Summary Report

```
Report Statistics:
   Checks that returned "WARN" .................. 1
   Checks that returned "info" .................. 1
   Checks that returned "no" .................... 6
   Checks that returned "ok" .................... 222
   Checks with silent results ................... 38
   Total checks made ............................ 268
```

* `FAIL` indicates the test failed. It may be acceptable to fail a test depending on the device/design. There should be no failures for rh.USRP_UHD.
* `WARN` CAN be fine, and may just be informational. See below.
* `info` is fine, just informational for developer to confirm the intended results. See below.
* `no` is fine, just informational for developer to confirm the intended results. Indicates an optional field was not used.
* `ok` is good, and indicates a check passed.
* `silent results` are checks that passed but do not appear anywhere in the output unless they fail.

### `info` Details

The `info` check is reporting that it is impossible to test full multi-out port capability with a single channel. The features that cannot be tested are not necessary for a single channel anyway.

```
dataShort_out: Cannot fully test multiport because only single
     RX_DIGITIZER tuner capability..........................................info
```

### `WARN` Details

The single `WARN` check is reporting that an unknown field was found in the tuner status, which is permitted. The reason is it a warning is to call extra attention in case the unknown (user defined) property could be modified to one of the many pre-defined optional fields also reported in the test.
```
tuner_status has UNKNOWN field FRONTEND::tuner_status::stream_id............WARN
```
