#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK USRP_UHD.
#
# REDHAWK USRP_UHD is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK USRP_UHD is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
# By default, the RPM will install to the standard REDHAWK SDR root location (/var/redhawk/sdr)
# You can override this at install time using --prefix /new/sdr/root when invoking rpm (preferred method, if you must)
%{!?_sdrroot: %global _sdrroot /var/redhawk/sdr}
%define _prefix %{_sdrroot}
Prefix:         %{_prefix}

# Point install paths to locations within our target SDR root
%define _sysconfdir    %{_prefix}/etc
%define _localstatedir %{_prefix}/var
%define _mandir        %{_prefix}/man
%define _infodir       %{_prefix}/info

Name:           rh.USRP_UHD
Version:        5.0.0
Release:        6%{?dist}
Summary:        Device %{name}

Group:          REDHAWK/Devices
License:        LGPLv3+
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  redhawk-devel >= 2.0
Requires:       redhawk >= 2.0

Requires:       libuuid-devel
BuildRequires:  libuuid-devel

# Interface requirements
BuildRequires:  frontendInterfaces >= 2.2 bulkioInterfaces >= 2.0
Requires:       frontendInterfaces >= 2.2 bulkioInterfaces >= 2.0

BuildRequires:  uhd-devel

Obsoletes:      USRP_UHD < 4.0.0

%description
Device %{name}
 * Commit: __REVISION__
 * Source Date/Time: __DATETIME__


%prep
%setup -q


%build
# Implementation cpp
pushd cpp
./reconf
%define _bindir %{_prefix}/dev/devices/rh/USRP_UHD/cpp
%configure
make %{?_smp_mflags}
popd


%install
rm -rf $RPM_BUILD_ROOT
# Implementation cpp
pushd cpp
%define _bindir %{_prefix}/dev/devices/rh/USRP_UHD/cpp
make install DESTDIR=$RPM_BUILD_ROOT
popd


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,redhawk,redhawk,-)
%dir %{_sdrroot}/dev/devices/rh
%dir %{_sdrroot}/dev/devices/rh/USRP_UHD
%{_prefix}/dev/devices/rh/USRP_UHD/nodeconfig.py
%{_prefix}/dev/devices/rh/USRP_UHD/nodeconfig.pyc
%{_prefix}/dev/devices/rh/USRP_UHD/nodeconfig.pyo
%{_prefix}/dev/devices/rh/USRP_UHD/USRP_UHD.scd.xml
%{_prefix}/dev/devices/rh/USRP_UHD/USRP_UHD.prf.xml
%{_prefix}/dev/devices/rh/USRP_UHD/USRP_UHD.spd.xml
%{_prefix}/dev/devices/rh/USRP_UHD/cpp

