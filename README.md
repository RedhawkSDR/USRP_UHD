# REDHAWK Basic Devices rh.USRP_UHD
 
## Description

Contains the source and build script for the REDHAWK Basic Devices rh.USRP_UHD
device. This device is a FRONTEND Interfaces compliant device for the USRP that
requires the UHD host code and supporting libraries to be installed.

## Branches and Tags

All REDHAWK core assets use the same branching and tagging policy. Upon release,
the `master` branch is rebased with the specific commit released, and that
commit is tagged with the asset version number. For example, the commit released
as version 1.0.0 is tagged with `1.0.0`.

Development branches (i.e. `develop` or `develop-X.X`, where *X.X* is an asset
version number) contain the latest unreleased development code for the specified
version. If no version is specified (i.e. `develop`), the branch contains the
latest unreleased development code for the latest released version.

## REDHAWK Version Compatibility

| Asset Version | Minimum REDHAWK Version Required |
| ------------- | -------------------------------- |
| 5.x           | 2.0                              |
| 4.x           | 2.0                              |
| 3.x           | 1.10                             |
| 2.x           | 1.8                              |

## Installation Instructions

This asset requires the uhd library, which must be installed in order to build
and run this asset. To build from source, run the `build.sh` script found at the
top level directory. To install to $SDRROOT, run `build.sh install`. Note: root
privileges (`sudo`) may be required to install.

## Troubleshooting

The UHD software will raise a `RuntimeError` exception when the firmware and/or
fpga image flashed to the USRP hardware is incompatible with the UHD software
installed on the host system. To ensure that the firmware and fpga image are
compatible with the UHD software, run the `uhd_usrp_probe` command and check
for a `RuntimeError` to occur. If no error occurs, everything is compatible.

It may be necessary to identify the specific USRP target hardware when running
`uhd_usrp_probe`. For example, `uhd_usrp_probe --args="addr=192.168.10.2"` may
be used to target a network-attached USRP where `192.168.10.2` is the IP address
of the USRP hardware.

If there is an error, the message should indicate how to go about fixing the
error. Typically, the solution is to flash the USRP hardware with compatible
firmware and/or a compatible fpga image. The firmware and fpga images are
available by installing the `uhd-firmware` package using *yum* or *rpm*. This
package is included with REDHAWK yum repository, but must be installed
explicitly.

The `RuntimeError` should be similar to one of the following examples:

**USRP X3xx Series**
```
RuntimeError: Expected FPGA compatibility number 19, but got 13:
The FPGA image on your device is not compatible with this host code build.
Download the appropriate FPGA images for this version of UHD.
Please run:

"/usr/lib64/uhd/utils/uhd_images_downloader.py"

Then burn a new image to the on-board flash storage of your
USRP X3xx device using the image loader utility. Use this command:

"/usr/bin/uhd_image_loader" --args="type=x300,addr=192.168.10.2"

For more information, refer to the UHD manual:

 http://files.ettus.com/manual/page_usrp_x3x0.html#x3x0_flash
```

**USRP N2xx Series**
```
RuntimeError: Please update the firmware and FPGA images for your device.
See the application notes for USRP2/N-Series for instructions.
Expected FPGA compatibility number 11, but got 10:
The FPGA build is not compatible with the host code build.
Please run:

"/usr/local/lib/uhd/utils/uhd_images_downloader.py"
"/usr/local/lib/uhd/utils/usrp_n2xx_simple_net_burner" \
    --addr="192.168.10.2"
```

**Note:** The path to the UHD utilities mentioned in the messages above may be
different on your system. The path will typically begin with `/usr` or
`/usr/local`, and the library may either be in `lib` or `lib64`.

## Hardware Compatibility

UHD versions prior to 3.9.0 only support a subset of the X300/X310 hardware and
firmware. X3xx devices with a hardware revision number from 0 to 6 and a
firmware version of 3.x or lower will work with UHD prior to 3.9.0. Firmware
version 4.0 or later, as well as hardware revision number 7 or later, require
UHD version 3.9.0 or later.

## FEI Compliance Test Results

See the [FEI Compliance Results](tests/FEI_Compliance_Results.md) document.

## Copyrights

This work is protected by Copyright. Please refer to the
[Copyright File](COPYRIGHT) for updated copyright information.

## License

REDHAWK Basic Devices rh.USRP_UHD is licensed under the GNU Lesser General
Public License (LGPL).

