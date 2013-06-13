#!/bin/sh
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
 

if [ "$1" == "rpm" ]; then
    # A very simplistic RPM build scenario
    if [ -e USRP_UHD.spec ]; then
        mydir=`dirname $0`
        tmpdir=`mktemp -d`
        cp -r ${mydir} ${tmpdir}/USRP_UHD-2.0.1
        tar czf ${tmpdir}/USRP_UHD-2.0.1.tar.gz --exclude=".svn" -C ${tmpdir} USRP_UHD-2.0.1
        rpmbuild -ta ${tmpdir}/USRP_UHD-2.0.1.tar.gz
        rm -rf $tmpdir
    else
        echo "Missing RPM spec file in" `pwd`
        exit 1
    fi
else
    for impl in cpp ; do
        pushd $impl &> /dev/null
        if [ -e build.sh ]; then
            ./build.sh $*
        else
            echo "No build.sh found for $impl"
        fi
        popd &> /dev/null
    done
fi
