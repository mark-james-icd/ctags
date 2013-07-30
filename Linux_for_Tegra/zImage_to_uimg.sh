#!/bin/bash

# Copyright (c) 2012, NVIDIA CORPORATION
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

set -e

# show the usages text
function ShowUsage {
    local ScriptName=$1

    echo "Use: $1 [--zImage|-z PATH] [--dest|-d DESTINATION] [--mkimage|-m PATH] [--addr|-a] [--help|-h]"
cat <<EOF
    This script creates the vmlinux.uimg from the zImage
    Options are:
    --zImage|-z PATH
                   path/location of the zImage
    --dest|-d DIRECTORY
                   directory to install vmlinux.uimg
    --mkimage|-m PATH
                   path/location of mkimage
    --addr|-a ADDRESS
                   kernel uimg address
    --help|-h
                   show this help
EOF
}

# script name
SCRIPT_NAME=`basename $0`
# save the base directory
LDK_DIR=$(cd `dirname $0` && pwd)
# default to 'package' paths/locations unless overridden
KERNEL_DIR="${LDK_DIR}/kernel"
KERNEL_DIR_ZIMAGE="${KERNEL_DIR}/zImage"
VMLINUX_DIR="${KERNEL_DIR}"
MKIMAGE=${LDK_DIR}/bootloader/mkimage
KERNEL_UIMG_ADDR=0x81008000

# parse the command line first
TGETOPT=`getopt -n "$SCRIPT_NAME" --longoptions help,zImage:,dest:,mkimage:,addr: -o hz:d:m:a: -- "$@"`

if [ $? != 0 ]; then
    echo "Terminating... wrong switch"
    ShowUsage "$SCRIPT_NAME"
    exit 1
fi

eval set -- "$TGETOPT"

while [ $# -gt 0 ]; do
    case "$1" in
        -z|--zImage) KERNEL_DIR_ZIMAGE="$2"; shift ;;
        -h|--help) ShowUsage "$SCRIPT_NAME"; exit 1 ;;
        -d|--dest) VMLINUX_DIR="$2"; shift ;;
        -m|--mkimage) MKIMAGE="$2"; shift ;;
        -a|--addr) KERNEL_UIMG_ADDR="$2"; shift ;;
        --) shift; break ;;
        -*) echo "Terminating... wrong switch: $@" >&2 ; ShowUsage "$SCRIPT_NAME"; exit 1 ;;
    esac
    shift
done

if [ $# -gt 0 ]; then
    ShowUsage "$SCRIPT_NAME"
    exit 1
fi

# fail if the zImage doesn't exist
if [ ! -f "${KERNEL_DIR_ZIMAGE}" ] ; then
    echo "FAILED - Kernel zImage not present at: ${KERNEL_DIR_ZIMAGE}"
    exit 1
fi

# fail if the path to vmlinux.uimg doesn't exist
if [ ! -d "${VMLINUX_DIR}" ] ; then
    echo "FAILED - dest path does not exist at: ${VMLINUX_DIR}"
    exit 1
fi

if [ "x${KERNEL_UIMG_ADDR}" == "xINVALID" ]; then
    echo "FAILED - you must pass in a valid address"
    exit 1
fi

# set here since it may have been re-set as an arg
FULL_VMLINUX_PATH="${VMLINUX_DIR}/vmlinux.uimg"

echo "Using MKIMAGE: ${MKIMAGE}"

# show warning if they are over-writing the previous uimg
if [ -f "${FULL_VMLINUX_PATH}" ] ; then
    echo "Overwriting previous vmlinux.uimage at: ${FULL_VMLINUX_PATH}"
fi

${MKIMAGE} -A arm                     \
           -O linux                   \
           -T kernel                  \
           -C none                    \
           -a ${KERNEL_UIMG_ADDR}     \
           -e ${KERNEL_UIMG_ADDR}     \
           -n tegra_l4t               \
           -d ${KERNEL_DIR_ZIMAGE}    \
           ${FULL_VMLINUX_PATH}       ;

echo "The vmlinux.uimg is available at: ${FULL_VMLINUX_PATH}"
