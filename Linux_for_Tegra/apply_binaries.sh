#!/bin/bash

# Copyright (c) 2011-2012, NVIDIA CORPORATION
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#
# This script applies the binaries to the rootfs dir pointed to by
# LDK_ROOTFS_DIR variable.
#

set -e

# show the usages text
function ShowUsage {
    local ScriptName=$1

    echo "Use: $1 [--root|-r PATH] [--xabi XABI] [--help|-h]"
cat <<EOF
    This script installs tegra binaries
    Options are:
    --root|-r PATH
                   install toolchain to PATH
    --xabi XABI
                   specify the X ABI version
    --help|-h
                   show this help
EOF
}

function ShowDebug {
    echo "SCRIPT_NAME    : $SCRIPT_NAME"
    echo "LDK_ROOTFS_DIR : $LDK_ROOTFS_DIR"
}

# if the user is not root, there is not point in going forward
THISUSER=`whoami`
if [ "x$THISUSER" != "xroot" ]; then
    echo "This script requires root privilage"
    exit 1
fi

# script name
SCRIPT_NAME=`basename $0`

# empty root and no debug
DEBUG=
XABI=11

# parse the command line first
TGETOPT=`getopt -n "$SCRIPT_NAME" --longoptions help,debug,root:,xabi: -o dhr: -- "$@"`

if [ $? != 0 ]; then
    echo "Terminating... wrong switch"
    ShowUsage "$SCRIPT_NAME"
    exit 1
fi

eval set -- "$TGETOPT"

while [ $# -gt 0 ]; do
    case "$1" in
	-r|--root) LDK_ROOTFS_DIR="$2"; shift ;;
	-h|--help) ShowUsage "$SCRIPT_NAME"; exit 1 ;;
	-d|--debug) DEBUG="true" ;;
	--xabi) XABI=$2 ;;
	--) shift; break ;;
	-*) echo "Terminating... wrong switch: $@" >&2 ; ShowUsage "$SCRIPT_NAME"; exit 1 ;;
    esac
    shift
done

if [ $# -gt 0 ]; then
    ShowUsage "$SCRIPT_NAME"
    exit 1
fi

# done, now do the work, save the directory
LDK_DIR=$(cd `dirname $0` && pwd)

# use default rootfs dir if none is set
if [ -z "$LDK_ROOTFS_DIR" ] ; then
    LDK_ROOTFS_DIR="${LDK_DIR}/rootfs"
fi

echo "Using rootfs directory of: ${LDK_ROOTFS_DIR}"

if [ ! -d "${LDK_ROOTFS_DIR}" ]; then
    mkdir -p "${LDK_ROOTFS_DIR}" > /dev/null 2>&1
fi

# get the absolute path, for LDK_ROOTFS_DIR.
# otherwise, tar behaviour is unknown in last command sets
TOP=$PWD
cd "$LDK_ROOTFS_DIR"
LDK_ROOTFS_DIR="$PWD"
cd "$TOP"

# assumption: this script is part of the BSP
#             so, LDK_DIR/nv_tegra always exist
LDK_NV_TEGRA_DIR="${LDK_DIR}/nv_tegra"
LDK_KERN_DIR="${LDK_DIR}/kernel"

echo "Extracting the NVIDIA user space components to ${LDK_ROOTFS_DIR}"
pushd "${LDK_ROOTFS_DIR}" > /dev/null 2>&1
tar xpf ${LDK_NV_TEGRA_DIR}/nvidia_drivers.tbz2
popd > /dev/null 2>&1

echo "Extracting the NVIDIA gst test applications to ${LDK_ROOTFS_DIR}"
pushd "${LDK_ROOTFS_DIR}" > /dev/null 2>&1
tar xpf ${LDK_NV_TEGRA_DIR}/nv_sample_apps/nvgstapps.tbz2
popd > /dev/null 2>&1

echo "Extracting the configuration files for the supplied root filesystem to ${LDK_ROOTFS_DIR}"
pushd "${LDK_ROOTFS_DIR}" > /dev/null 2>&1
tar xpf ${LDK_NV_TEGRA_DIR}/config.tbz2
popd > /dev/null 2>&1

if [ ! -f "${LDK_ROOTFS_DIR}/usr/lib/xorg/modules/drivers/tegra_drv.abi${XABI}.so" ]; then
	echo "ERROR: X ABI version $XABI is not supported."
	exit 4
fi

echo "Installing X ABI version $XABI to ${LDK_ROOTFS_DIR}..."

pushd "${LDK_ROOTFS_DIR}/usr/lib/xorg/modules/drivers/" > /dev/null
if [ -f tegra_drv.so ]; then
	rm tegra_drv.so
fi
ln -s "tegra_drv.abi${XABI}.so" "tegra_drv.so"
popd > /dev/null

echo "Extracting the firmwares and kernel modules to ${LDK_ROOTFS_DIR}"
( cd "${LDK_ROOTFS_DIR}" ; tar jxpf "${LDK_KERN_DIR}/kernel_supplements.tbz2" )

echo "Installing vmlinux.uimg into ${LDK_ROOTFS_DIR}/boot"
sudo install --owner=root --group=root --mode=644 -D "${LDK_KERN_DIR}/vmlinux.uimg" "${LDK_ROOTFS_DIR}/boot/vmlinux.uimg"

echo "Success!"
