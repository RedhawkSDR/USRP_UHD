/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK USRP_UHD.
 *
 * REDHAWK USRP_UHD is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK USRP_UHD is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
AC_INIT(rh.USRP_UHD, 6.0.0)
AM_INIT_AUTOMAKE([nostdinc foreign])
AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL

AC_CORBA_ORB
OSSIE_CHECK_OSSIE
OSSIE_SDRROOT_AS_PREFIX

AC_ARG_WITH(domainname,
        AS_HELP_STRING([--with-domainname=[domain name]], [Provide a domain name to the generated node, defaults to REDHAWK_DEV]),
        usrp_uhd_node_domain=$withval,
        usrp_uhd_node_domain=REDHAWK_DEV)
AC_SUBST(DOMAINNAME, $usrp_uhd_node_domain)

AC_ARG_WITH(usrptype,
        AS_HELP_STRING([--with-usrptype=[usrp type]], [Provide a USRP type to the generated node, defaults to usrp2]),
        usrp_uhd_node_type=$withval,
        usrp_uhd_node_type=usrp2)
AC_SUBST(USRPTYPE, $usrp_uhd_node_type)

AC_ARG_WITH(usrpip,
        AS_HELP_STRING([--with-usrpip=[usrp ip address]], [Provide a USRP ip address to the generated node, defaults to '']),
        usrp_uhd_node_ip=$withval,
        usrp_uhd_node_ip='')
AC_SUBST(USRPIP, $usrp_uhd_node_ip)

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Dependencies
export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig"
PKG_CHECK_MODULES([PROJECTDEPS], [ossie >= 2.0 omniORB4 >= 4.1.0])
PKG_CHECK_MODULES([INTERFACEDEPS], [frontend >= 2.2, bulkio >= 2.0])
PKG_CHECK_MODULES([LIBUHD], [uhd >= 3.5.3])
PKG_CHECK_MODULES([LIBUUID], [uuid])
OSSIE_ENABLE_LOG4CXX
AX_BOOST_BASE([1.41])
AX_BOOST_SYSTEM
AX_BOOST_THREAD
AX_BOOST_REGEX

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

