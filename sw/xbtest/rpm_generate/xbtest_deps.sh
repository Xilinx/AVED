#!/bin/bash

# Copyright (C) 2022 Xilinx, Inc.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


FLAVOR=`grep '^ID=' /etc/os-release | awk -F= '{print $2}' | tr -d '"'`
VERSION=`grep '^VERSION_ID=' /etc/os-release | awk -F= '{print $2}' | tr -d '"'`
MAJOR=${VERSION%.*}
ARCH=`uname -m`

usage()
{
    echo "Usage: xbtest_deps.sh [options]"
    echo
    echo "[-help]                    List this help"
    echo "[-validate]                Validate that required packages are installed"
    exit 1
}

validate=0

while [ $# -gt 0 ]; do
    case "$1" in
        -help)
            usage
            ;;
        -validate)
            validate=1
            shift
            ;;
        *)
            echo "unknown option"
            usage
            ;;
    esac
done

rh_package_list()
{
    RH_LIST=(\
     json-glib-devel \
     ncurses-devel \
     python3 \
     python3-pip \
     redhat-lsb-core \
     rpm-build \
    )
}

ub_package_list()
{
    UB_LIST=(\
     libjson-glib-dev \
     libncurses5-dev \
     python3 \
     python3-pip \
     lsb-core \
     dpkg \
    )
}

suse_package_list()
{
    SUSE_LIST=(\
     json-glib-devel \
     ncurses-devel \
     python3 \
     python3-pip \
     lsb-release \
     rpm-build \
    )
}


update_package_list()
{
    if [ $FLAVOR == "ubuntu" ] || [ $FLAVOR == "debian" ]; then
        ub_package_list
    elif [ $FLAVOR == "centos" ] || [ $FLAVOR == "rhel" ]; then
        rh_package_list
    elif [ $FLAVOR == "sles" ]; then
        suse_package_list
    else
        echo "unknown OS flavor $FLAVOR"
        exit 1
    fi
}

validate()
{
    if [ $FLAVOR == "ubuntu" ] || [ $FLAVOR == "debian" ]; then
        #apt -qq list "${UB_LIST[@]}"
        dpkg -l "${UB_LIST[@]}" > /dev/null
    fi

    if [ $FLAVOR == "centos" ] || [ $FLAVOR == "rhel" ]; then
        rpm -q "${RH_LIST[@]}"
    fi

    if [ $FLAVOR == "sles" ]; then
        rpm -q "${SUSE_LIST[@]}"
    fi
}

prep_ubuntu()
{
    echo "Preparing ubuntu ..."
}

prep_centos7()
{
    echo "Enabling EPEL repository..."
    rpm -q --quiet epel-release
    if [ $? != 0 ]; then
    	yum install -y https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm
	    yum check-update
    fi
    echo "Installing cmake3 from EPEL repository..."
    yum install -y cmake3

    echo "Enabling CentOS SCL repository..."
    yum --enablerepo=extras install -y centos-release-scl
}

prep_rhel7()
{
    echo "Enabling EPEL repository..."
    rpm -q --quiet epel-release
    if [ $? != 0 ]; then
    	 yum install -y https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm
	 yum check-update
    fi

    echo "Enabling RHEL SCL repository..."
    yum-config-manager --enable rhel-server-rhscl-7-rpms

    echo "Enabling repository 'rhel-7-server-e4s-optional-rpms"
    subscription-manager repos --enable "rhel-7-server-e4s-optional-rpms"

    echo "Enabling repository 'rhel-7-server-optional-rpms'"
    subscription-manager repos --enable "rhel-7-server-optional-rpms"
}

prep_rhel8()
{
    echo "Enabling EPEL repository..."
    rpm -q --quiet epel-release
    if [ $? != 0 ]; then
        yum install -y https://dl.fedoraproject.org/pub/epel/epel-release-latest-8.noarch.rpm
        yum check-update
    fi
}

prep_centos8()
{
    echo "Enabling EPEL repository..."
    rpm -q --quiet epel-release
    if [ $? != 0 ]; then
        yum install -y https://dl.fedoraproject.org/pub/epel/epel-release-latest-8.noarch.rpm
        yum check-update
    fi
}

prep_centos()
{
    if [ $MAJOR == 8 ]; then
        prep_centos8
    else
        prep_centos7
    fi
}

prep_rhel()
{
   if [ $MAJOR == 8 ]; then
        prep_rhel8
    else
        prep_rhel7
    fi
}

install()
{
    if [ $FLAVOR == "ubuntu" ] || [ $FLAVOR == "debian" ]; then
        prep_ubuntu

        echo "Installing packages..."
        apt install -y "${UB_LIST[@]}"
    fi

    # Enable EPEL on CentOS/RHEL
    if [ $FLAVOR == "centos" ]; then
        prep_centos
    elif [ $FLAVOR == "rhel" ]; then
        prep_rhel
    fi

    if [ $FLAVOR == "rhel" ] || [ $FLAVOR == "centos" ]; then
        echo "Installing RHEL/CentOS packages..."
        yum install -y "${RH_LIST[@]}"
        if [ $MAJOR -lt "8" ]; then
            if [ $FLAVOR == "centos" ]; then
                yum --enablerepo=base install -y devtoolset-9
            else
                yum install -y devtoolset-9
            fi
	    fi
	fi

    if [ $FLAVOR == "fedora" ]; then
        echo "Installing Fedora packages..."
        yum install -y "${FD_LIST[@]}"
    fi

    if [ $FLAVOR == "sles" ] ; then
        echo "Installing SUSE packages..."
        ${SUDO} zypper install -y "${SUSE_LIST[@]}"
    fi

}

update_package_list

if (( $EUID != 0 )); then
    echo "Please run as root or with a sudo prefix"
    exit
fi

if [ $validate == 1 ]; then
    validate
else
    install
fi
