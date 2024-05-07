#!/usr/bin/env bash 
# (c) Copyright 2024, Advanced Micro Devices, Inc.
# 
# Permission is hereby granted, free of charge, to any person obtaining a 
# copy of this software and associated documentation files (the "Software"), 
# to deal in the Software without restriction, including without limitation 
# the rights to use, copy, modify, merge, publish, distribute, sublicense, 
# and/or sell copies of the Software, and to permit persons to whom the 
# Software is furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in 
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
# DEALINGS IN THE SOFTWARE.
############################################################

DEST_DIR="/opt/amd/aved/amd_v80p_gen5x8_23.2_exdes_2"


if [ $(grep '^NAME' /etc/os-release | grep -c "Ubuntu") -eq 1 ]; then
    if [ $(grep '^VERSION_ID' /etc/os-release | grep -c "20.04") -eq 1 ]; then
       PKG="20.04.deb"
       PKG_MGR="apt"
    elif [ $(grep '^VERSION_ID' /etc/os-release | grep -c "22.04")  -eq 1 ]; then
       PKG="22.04.deb"
       PKG_MGR="apt"
    else
      echo "Unsupported version of Ubuntu"
    fi
elif [ $(grep '^NAME' /etc/os-release | grep -c "Red Hat") -eq 1 ]; then
   if [ $(grep '^VERSION_ID' /etc/os-release | grep -c "8.") -eq 1 ]; then
       PKG="rpm"
       PKG_MGR="yum"
    else
      echo "Unsupported version of Red Hat"; exit
    fi
else
   echo "Unsupported OS"
   echo "Only Ubuntu 20.04, Ubuntu 22.04 & Red Hat 8.* are supported"; exit
fi

echo " "
echo "=========================================================================="
echo "Removing previous AVED installations"
echo "=========================================================================="
echo " "

if [ $(grep '^NAME' /etc/os-release | grep -c "Ubuntu") -eq 1 ]; then
   if [ $(apt list | grep -c ^xbtest/now) -eq 1 ]; then
       echo " "
       echo "=========================================================================="
       echo "Removing previous installation of XBTEST"
       echo "=========================================================================="
       echo " "
       $PKG_MGR remove xbtest -y
    fi
   if [ $(apt list | grep -c ^ami/now) -eq 1 ]; then
       echo " "
       echo "=========================================================================="
       echo "Removing previous installation of AMI"
       echo "=========================================================================="
       echo " "
       $PKG_MGR remove ami -y
    fi
fi

if [ $(grep '^NAME' /etc/os-release | grep -c "Red Hat") -eq 1 ]; then
   if [ $(yum list | grep -c ^xbtest.x86) -eq 1 ]; then
       echo " "
       echo "=========================================================================="
       echo "Removing previous installation of XBTEST"
       echo "=========================================================================="
       echo " "
       $PKG_MGR remove xbtest -y
    fi
   if [ $(yum list | grep -c ^ami.x86) -eq 1 ]; then
       echo " "
       echo "=========================================================================="
       echo "Removing previous installation of AMI"
       echo "=========================================================================="
       echo " "
       $PKG_MGR remove ami -y
    fi
fi

echo " "
echo "=========================================================================="
echo "Removing previous installation of AVED HW "
echo "=========================================================================="
echo " "

rm -rf               $DEST_DIR

echo " "
echo "=========================================================================="
echo "Installing New AVED deployment"
echo "=========================================================================="
echo " "

mkdir -p             $DEST_DIR
cp -L    *.pdi       $DEST_DIR
cp -L    *.xsa       $DEST_DIR
cp -L    *.json      $DEST_DIR
cp -rL   flash_setup $DEST_DIR

echo " "
echo "=========================================================================="
echo "AVED Deployment files copied to $DEST_DIR"
echo "=========================================================================="
echo " "

echo " "
echo "=========================================================================="
echo "Installing AMI & XBTEST"
echo "=========================================================================="
echo " "

$PKG_MGR install ./sw_packages/*$PKG -y

echo " "
echo "=========================================================================="
echo "AVED Installation Complete"
echo "=========================================================================="
echo " "
echo " "
echo " "
