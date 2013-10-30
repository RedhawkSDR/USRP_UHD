#
# This file is protected by Copyright. Please refer to the COPYRIGHT file 
# distributed with this source distribution.
# 
# This file is part of GNUHAWK.
# 
# GNUHAWK is free software: you can redistribute it and/or modify is under the 
# terms of the GNU General Public License as published by the Free Software 
# Foundation, either version 3 of the License, or (at your option) any later 
# version.
# 
# GNUHAWK is distributed in the hope that it will be useful, but WITHOUT ANY 
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR 
# A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

# You should have received a copy of the GNU General Public License along with 
# this program.  If not, see http://www.gnu.org/licenses/.
#
 
%bcond_with intel
# By default, the RPM will install to the standard REDHAWK SDR root location (/var/redhawk/sdr)
# You can override this at install time using --prefix /new/sdr/root when invoking rpm (preferred method, if you must)
%define _sdrroot /var/redhawk/sdr
%define _prefix %{_sdrroot}
Prefix: %{_prefix}

# Point install paths to locations within our target SDR root
%define _sysconfdir    %{_prefix}/etc
%define _localstatedir %{_prefix}/var
%define _mandir        %{_prefix}/man
%define _infodir       %{_prefix}/info

Name: USRP_UHD
Summary: Device %{name}
Version: 2.0.2
Release: 1%{?dist}
License: None
Group: REDHAWK/Devices
Source: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-root

Requires: redhawk >= 1.8
BuildRequires: redhawk-devel >= 1.8
BuildRequires: redhawk-sdrroot-dev-mgr >= 1.8

# Interface requirements
Requires: frontendInterfaces bulkioInterfaces
BuildRequires: frontendInterfaces bulkioInterfaces

# C++ requirements
Requires: libomniORB4.1
Requires: boost >= 1.41
Requires: apache-log4cxx >= 0.10
BuildRequires: boost-devel >= 1.41
BuildRequires: libomniORB4.1-devel

%if "%{?rhel}" == "6"
Requires: libuuid-devel
BuildRequires: libuuid-devel
%else
Requires: e2fsprogs-devel
BuildRequires: e2fsprogs-devel
%endif

BuildRequires: uhd
BuildRequires: uhd-devel

%if %{with intel}
BuildRequires: compat-libstdc++-33
Requires: compat-libstdc++-33
%endif

%description
Device %{name}

%prep
%setup

%build
# Implementation cpp
pushd cpp
./reconf
%define _bindir %{_prefix}/dev/devices/USRP_UHD/cpp
./configure %{?_with_intel} --with-intelopts

make
popd

%install
rm -rf $RPM_BUILD_ROOT
# Implementation cpp
pushd cpp
%define _bindir %{_prefix}/dev/devices/USRP_UHD/cpp 
make install DESTDIR=$RPM_BUILD_ROOT
popd

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,redhawk,redhawk)
%dir %{_prefix}/dev/devices/%{name}
%{_prefix}/dev/devices/%{name}/USRP_UHD.spd.xml
%{_prefix}/dev/devices/%{name}/USRP_UHD.prf.xml
%{_prefix}/dev/devices/%{name}/USRP_UHD.scd.xml
%{_prefix}/dev/devices/%{name}/cpp
