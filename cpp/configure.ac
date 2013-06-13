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
 
AC_INIT(USRP_UHD, 2.0.1)
AM_INIT_AUTOMAKE(nostdinc)

m4_ifdef([AC_USE_INTEL],[AC_USE_INTEL])
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL

AC_CORBA_ORB
OSSIE_CHECK_OSSIE
OSSIE_SDRROOT_AS_PREFIX

# Dependencies
export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig"
PKG_CHECK_MODULES([PROJECTDEPS], [ossie >= 1.8 omniORB4 >= 4.0.0])
PKG_CHECK_MODULES([INTERFACEDEPS], [bulkioInterfaces >= 1.8 frontendInterfaces])
OSSIE_ENABLE_LOG4CXX
AX_BOOST_BASE([1.41])
AX_BOOST_THREAD
CHECK_VECTOR_IMPL

AC_CONFIG_FILES(Makefile)
AC_OUTPUT
