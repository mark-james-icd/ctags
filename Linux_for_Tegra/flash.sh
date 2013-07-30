#!/bin/bash

# Copyright (c) 2011-2012, NVIDIA CORPORATION.  All rights reserved.
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
# flash.sh: Flash the target board.
#	    flash.sh performs the best in LDK release environment.
#
# Usage: Place the board in recovery mode and run:
#
#	flash.sh [options] <target board> <root_device>
#
#	for more detail enter 'flash.sh -h'
#
# Examples:
# ./flash.sh cardhu mmcblk0p1			- boot cardhu from internal emmc
# ./flash.sh cardhu mmcblk1p1			- boot cardhu from SDCARD
# ./flash.sh ventana sda1			- boot ventana from USB device
# ./flash.sh -N <IP addr>:/nfsroot ventana eth0	- boot ventana from NFS
# ./flash.sh harmony mmcblk0p1			- boot cardhu harmony SDCARD
#						- NOTE: mmcblk0p1 is SDCARD
#						-       for harmony
# ./flash.sh -k 6 harmony mmcblk1p1		- update harmony kernel
#						- in partition 6.
#
# Optional Environment Variables:
# ROOTFS_SIZE ------------ Linux RootFS size (internal emmc/nand only).
# ODMDATA ---------------- Odmdata to be used.
# FLASHAPP --------------- Flash application running in host machine.
# FLASHER ---------------- Flash server running in target machine.
# BOOTLOADER ------------- Bootloader binary to be flashed
# BCTFILE ---------------- BCT file to be used.
# CFGFILE ---------------- CFG file to be used.
# KERNEL_IMAGE ----------- Linux kernel zImage file to be flashed.
# ROOTFS ----------------- Linux RootFS directory name.
# ROOTFS_TYPE ------------ yaffs2. Valid only for Harmony internal NAND.
# NFSROOT ---------------- NFSROOT i.e. <my IP addr>:/exported/rootfs_dir
# NFSARGS ---------------- Static Network assignments.
#			   <C-ipa>:<S-ipa>:<G-ipa>:<netmask>
# KERNEL_INITRD ---------- Initrd file to be flashed.
# CMDLINE ---------------- Target cmdline. See help for more information.
# UBOOT_TEXT_BASE -------- U-boot Image Load Address.
# UIMAGE_LABEL ----------- Kernel version for U-boot image header.
# UIMAGE_NAME ------------ uImage file name.
# UBOOTSCRIPT ------------ U-boot HUSH boot script file.
#
LDK_DIR=$(cd `dirname $0` && pwd)
target_board=
target_sku=
target_rootdev=
target_partid=0
# Reserved Fixed partitions:
#	2 - BCT (absolute)
#	3 - PT  (strongly recommended)
#	4 - EBT (strongly recommended)
DFLT_FLASHAPPNAME="nvflash"
DFLT_FLASHERNAME="fastboot.bin"
DFLT_BOOTLOADERNAME="fastboot.bin"
MIN_KERN_PARTID=5;
rootdev_type="external";
rootfs_size=${ROOTFS_SIZE};
rootfs_type=${ROOTFS_TYPE};
odmdata=${ODMDATA};
flashapp=${FLASHAPP};
flasher=${FLASHER};
bootloader=${BOOTLOADER};
ubootscript=${UBOOTSCRIPT}
bootdelay=2;
bctfile=${BCTFILE};
cfgfile=${CFGFILE};
kernel_image=${KERNEL_IMAGE-${LDK_DIR}/kernel/zImage};
rootfs=${ROOTFS-${LDK_DIR}/rootfs};
nfsroot=${NFSROOT};
nfsargs=${NFSARGS};
kernel_initrd=${KERNEL_INITRD};
cmdline=
usercmdline=${CMDLINE};
ignorefastbootcmdline="false";
zflag="false";
bootdev_type=${BOOTDEV_TYPE};
uboot_text_base=;
uimage_label=;
uimage_name=${UIMAGE_NAME-vmlinux.uimg};
reuse_systemimg="false";

pr_conf()
{
	echo "target_board=${target_board}";
	echo "target_rootdev=${target_rootdev}";
	echo "rootdev_type=${rootdev_type}";
	echo "rootfs_size=${rootfs_size}";
	echo "odmdata=${odmdata}";
	echo "flashapp=${flashapp}";
	echo "flasher=${flasher}";
	echo "bootloader=${bootloader}";
	echo "ubootscript=${ubootscript}";
	echo "bctfile=${bctfile}";
	echo "cfgfile=${cfgfile}";
	echo "kernel_image=${kernel_image}";
	echo "rootfs=${rootfs}";
	echo "nfsroot=${nfsroot}";
	echo "nfsargs=${nfsargs}";
	echo "kernel_initrd=${kernel_initrd}";
	echo "usercmdline=${usercmdline}";
	echo "bootdev_type=${bootdev_type}";
	exit 0;
}

validIP ()
{
	local ip=$1;
	local ret=1;

	if [[ $ip =~ ^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$ ]]; then
		OIFS=${IFS};
		IFS='.';
		ip=($ip);
		IFS=${OIFS};
		[[ ${ip[0]} -le 255 && ${ip[1]} -le 255 && \
		   ${ip[2]} -le 255 && ${ip[3]} -le 255 ]];
		ret=$?;
	fi;
	return ${ret};
}

netmasktbl=(\
	"255.255.255.252" \
	"255.255.255.248" \
	"255.255.255.240" \
	"255.255.255.224" \
	"255.255.255.192" \
	"255.255.255.128" \
	"255.255.255.0" \
	"255.255.254.0" \
	"255.255.252.0" \
	"255.255.248.0" \
	"255.255.240.0" \
	"255.255.224.0" \
	"255.255.192.0" \
	"255.255.128.0" \
	"255.255.0.0" \
	"255.254.0.0" \
	"255.252.0.0" \
	"255.248.0.0" \
	"255.240.0.0" \
	"255.224.0.0" \
	"255.192.0.0" \
	"255.128.0.0" \
	"255.0.0.0" \
);

validNETMASK ()
{
	local i;
	local nm=$1;
	for (( i=0; i<${#netmasktbl[@]}; i++ )); do
		if [ "${nm}" = ${netmasktbl[$i]} ]; then
			return 0;
		fi;
	done;
	return 1;
}

validNFSargs ()
{
	local a=$1;

	OIFS=${IFS};
	IFS=':';
	a=($a);
	IFS=${OIFS};

	if [ ${#a[@]} -ne 4 ]; then
		return 1;
	fi;
	if ! validIP ${a[0]}; then
		return 1;
	fi;
	ipaddr=${a[0]};
	if ! validIP ${a[1]}; then
		return 1;
	fi;
	if [ "${serverip}" = "" ]; then
		serverip=${a[1]};
	fi;
	if ! validIP ${a[2]}; then
		return 1;
	fi;
	gatewayip=${a[2]};
	if ! validNETMASK ${a[3]}; then
		return 1;
	fi;
	netmask=${a[3]};
	return 0;
}

validNFSroot ()
{
	local a=$1;

	OIFS=${IFS};
	IFS=':';
	a=($a);
	IFS=${OIFS};
	if [ ${#a[@]} -ne 2 ]; then
		return 1;
	fi;
	validIP ${a[0]};
	if [ $? -ne 0 ]; then
		return 1;
	fi;
	if [[ "${a[1]}" != /* ]]; then
		return 1;
	fi;
	tftppath=${a[0]}:/tftpboot/${uimage_name};
	if [ "${serverip}" = "" ]; then
		serverip=${a[0]};
	fi;
	return 0;
}

usage ()
{
cat << EOF
Usage: sudo ./flash.sh [options] <target board> <rootdev>

   Where,
     <target board> is one of following:
         harmony, ventana, seaboard, cardhu, cardhu-a05, beaver.

     <rootdev> is one of following:

       For Harmony:
         mtdblock1 ---- internel NAND flash.
         mmcblk0p1 ---- external SDCARD or eMMC card without SDCARD inserted.
         mmcblk1p1 ---- external eMMC card when SDCARD is inserted.
         sda1 --------- external USB devices. (USB memory stick, HDD)
         usb0 --------- nfsroot via RJ45 Ethernet port (2.6.36 kernel).
         eth0 --------- nfsroot via RJ45 Ethernet port (2.6.39+ kernel).
                        nfsroot via USB Ethernet interface (2.6.36 kernel).
         eth1 --------- nfsroot via USB Ethernet interface (2.6.39+ kernel).

       For Ventana:
         mmcblk0p1 ---- internal eMMC.
         mmcblk1p1 ---- external SDCARD.
         sda1 --------- external USB devices. (USB memory stick, HDD)
         eth0 --------- nfsroot via external USB Ethernet interface.

       For Cardhu (all revisions):
         mmcblk0p1 ---- internal eMMC.
         mmcblk1p1 ---- external SDCARD.
         sda1 --------- external USB devices. (USB memory stick, HDD)
         eth0 --------- nfsroot via RJ45 Ethernet port with dock in use.
                        nfsroot via USB Ethernet interface without dock in use.
         eth1 --------- nfsroot via USB Ethernet interface with dock in use.

       For Beaver:
         mmcblk0p1 ---- internal eMMC.
         mmcblk1p1 ---- external SDCARD.
         sda1 --------- external USB devices. (USB memory stick, HDD)
         eth0 --------- nfsroot via RJ45 Ethernet port.
         eth1 --------- nfsroot via USB Ethernet interface.

   options:
   -h ------------------- print this message.
   -b <bctfile> --------- nvflash BCT file.
   -c <cfgfile> --------- nvflash config file.
   -f <flashapp> -------- Path to flash application: nvflash or tegra-rcm
   -k <partition id> ---- kernel partition id to be updated. (minimum = 5)
   -n <nfs args> -------- Static nfs network assignments
			  <Client IP>:<Server IP>:<Gateway IP>:<Netmask>
   -o <odmdata> --------- ODM data.
                          harmony:  0x300d8011
                          ventana:  0x30098011
                          seaboard: 0x300d8011
                          cardhu:   0x80080105 (A01)
                                    0x40080105 (A02+)
                                    0x80080105 (A05)
                          beaver:   0x80080105
   -r ------------------- skip building and reuse existing system.img.
   -s <ubootscript> ----- HUSH bootscript file for U-Boot.
   -t <rootfs type> ----- yaffs2. Valid only for Harmony internal NAND.
   -F <flasher> --------- Flash server such as fastboot.bin
   -L <bootloader> ------ Bootloader such as fastboot.bin or u-boot.bin
   -C <cmdline> --------- Kernel commandline.
                          WARNING:
                          Each option in this kernel commandline gets higher
                          preference over the same option from fastboot.
                          In case of NFS booting, this script adds NFS booting
                          related arguments, if -i option is omitted.
   -i ------------------- pass user kernel commandline as-is to kernel
   -D <boot Device> ----- emmc or nand.
   -K <kernel> ---------- Kernel image such as zImage.
   -I <initrd> ---------- initrd file. Null initrd is default.
   -R <rootfs dir> ------ Sample rootfs directory.
   -N <nfsroot> --------- nfsroot. i.e. <my IP addr>:/my/exported/nfs/rootfs.
   -S <size> ------------ Rootfs size in bytes. Valid only for internal rootdev.
                          KiB, MiB, GiB short hands are allowed, for example,
                          1GiB means 1024 * 1024 * 1024 bytes.

EOF
	exit $1;
}

while getopts "hb:c:f:ik:n:o:rs:t:F:L:C:D:K:I:R:N:S:Z" OPTION
do
	case $OPTION in
	h) usage 0;
	   ;;
	b) bctfile=${OPTARG};
	   ;;
	c) cfgfile=${OPTARG};
	   ;;
	k) target_partid=${OPTARG};
	   if [ $target_partid -lt $MIN_KERN_PARTID ]; then
		echo "Error: invalid partition id (min=${MIN_KERN_PARTID})";
		exit 1;
	   fi;
	   ;;
	f) flashapp=${OPTARG};
	   ;;
	n) nfsargs=${OPTARG};
	   if ! validNFSargs ${nfsargs}; then
		echo "Error: Invalid nfsargs(${nfsargs})";
		exit 1;
	   fi;
	   ;;
	o) odmdata=${OPTARG};
	   ;;
	i) ignorefastbootcmdline="true";
	   ;;
	r) reuse_systemimg="true";
	   ;;
	s) ubootscript=${OPTARG};
	   ;;
	t) rootfs_type=${OPTARG};
	   ;;
	F) flasher=${OPTARG};
	   ;;
	L) bootloader=${OPTARG};
	   ;;
	C) usercmdline=${OPTARG};
	   ;;
	D) bootdev_type=${OPTARG};
	   ;;
	K) kernel_image=${OPTARG};
	   ;;
	I) kernel_initrd=${OPTARG};
	   ;;
	R) rootfs=${OPTARG};
	   ;;
	N) nfsroot=${OPTARG};
	   if ! validNFSroot ${nfsroot}; then
		echo "Error: Invalid nfsroot(${nfsroot})";
		exit 1;
	   fi;
	   ;;
	S) rootfs_size=${OPTARG};
	   if [[ ${rootfs_size} == *KiB ]]; then
		rootfs_size=$(( ${rootfs_size%KiB} * 1024 ))
	   elif [[ ${rootfs_size} == *MiB ]]; then
		rootfs_size=$(( ${rootfs_size%MiB} * 1024 * 1024 ))
	   elif [[ ${rootfs_size} == *GiB ]]; then
		rootfs_size=$(( ${rootfs_size%GiB} * 1024 * 1024 * 1024 ))
	   fi
	   ;;
	Z) zflag="true";
	   ;;
	?) usage 1;
	   ;;
	esac
done
shift $((OPTIND - 1));
if [ $# -lt 2 ]; then
	usage 1;
fi;
target_board=$1;
target_rootdev=$2;
if [ "${target_board}" = "beaver" ]; then
	target_sku=${target_board};
	target_board="cardhu";
fi;

# Canonicalize all file path input if any.
#
if [ "${ubootscript}" != "" -a -f "${ubootscript}" ]; then
	ubootscript=`readlink -f "${ubootscript}"`;
fi;
if [ "${bctfile}" != "" -a -f "${bctfile}" ]; then
	bctfile=`readlink -f "${bctfile}"`;
fi;
if [ "${cfgfile}" != "" -a -f "${cfgfile}" ]; then
	cfgfile=`readlink -f "${cfgfile}"`;
fi;
if [ "${flashapp}" != "" -a -f "${flashapp}" ]; then
	flashapp=`readlink -f "${flashapp}"`;
fi;
if [ "${flasher}" != "" -a -f "${flasher}" ]; then
	flasher=`readlink -f "${flasher}"`;
fi;
if [ "${bootloader}" != "" -a -f "${bootloader}" ]; then
	bootloader=`readlink -f "${bootloader}"`;
fi;
if [ "${kernel_image}" != "" -a -f "${kernel_image}" ]; then
	kernel_image=`readlink -f "${kernel_image}"`;
fi;
if [ "${kernel_initrd}" != "" -a -f "${kernel_initrd}" ]; then
	kernel_initrd=`readlink -f "${kernel_initrd}"`;
fi;
if [ "${rootfs}" != "" -a -d "${rootfs}" ]; then
	rootfs=`readlink -f "${rootfs}"`;
fi;
if [ "${nfsroot}" != "" -a -d "${nfsroot}" ]; then
	nfsroot=`readlink -f "${nfsroot}"`;
fi;

# Default internal system partition size in bytes.
#
HARMONY_ROOTFS_SIZE=402653184;
VENTANA_ROOTFS_SIZE=1073741824;
SEABOARD_ROOTFS_SIZE=1073741824;
CARDHU_ROOTFS_SIZE=1073741824;

# Default odmdata.
#
HARMONY__ODMDATA=0x300d8011;
VENTANA__ODMDATA=0x30098011;
SEABOARD_ODMDATA=0x300d8011;
CARDHU___ODMDATA=0x40080105;
BEAVER___ODMDATA=0x80080105;
CARDHU_A05___ODMDATA=0x80080105;

# Default BSF(bootscript) files.
#
DFLT_BSF_DIR="${LDK_DIR}/bootloader/${target_board}";
HARMONY_NAND_BSF=harmony_nand.hush
HARMONY_NET_BSF=harmony_net.hush
VENTANA_NET_BSF=ventana_net.hush
VENTANA_EMMC_BSF=ventana_emmc.hush
SEABOARD_NAND_BSF=seaboard_nand.hush
SEABOARD_NET_BSF=seaboard_net.hush
SEABOARD_EMMC_BSF=seaboard_emmc.hush
CARDHU_NET_BSF=cardhu_net.hush
CARDHU_EMMC_BSF=cardhu_emmc.hush

# Default BCT files.
#
DFLT_BCT_DIR="${LDK_DIR}/bootloader/${target_board}/BCT";
HARMONY_NAND_BCT=harmony_a02_12Mhz_H5PS1G83EFR-S6C_333Mhz_1GB_2K8Nand_HY27UF084G2B-TP.bct;
VENTANA_EMMC_BCT=ventana_A03_12MHz_EDB8132B1PB6DF_300Mhz_1GB_emmc_THGBM1G6D4EBAI4.bct;
SEABOARD_NAND_BCT=Seaboard_A02P_MID_1GB_HYNIX_H5PS2G83AFR-S6_ddr2_333Mhz_NAND.bct;
SEABOARD_EMMC_BCT=PM282_Hynix_1GB_H5PS2G83AFR-S6C_333MHz_final_emmc_x8.bct;
CARDHU_EMMC_BCT=E1186_Hynix_1GB_H5TC2G83BFR-PBA_375MHz_110622_sdmmc4_x8.bct;
CARDHU_A05_EMMC_BCT=E1198_Hynix_2GB_H5TC4G83MFR-PBA_375MHz_111122_sdmmc4_x8.bct;
BEAVER_EMMC_BCT=Pm315_Hynix_2GB_H5TC4G83MFR-PBA_400MHz_120613_sdmmc4_x8.bct;

# Default CFG files.
#
DFLT_CFG_DIR="${LDK_DIR}/bootloader/${target_board}/cfg";
HARMONY_NAND_CFG=gnu_linux_fastboot_nand_full.cfg;
VENTANA_EMMC_CFG=gnu_linux_fastboot_emmc_full.cfg;
SEABOARD_NAND_CFG=gnu_linux_fastboot_nand_full.cfg;
SEABOARD_EMMC_CFG=gnu_linux_fastboot_emmc_full.cfg;
CARDHU_EMMC_CFG=gnu_linux_fastboot_emmc_full.cfg;

# Default UBOOT TEXT_BASES.
#
TEGRA2_UBOOT_TEXT_BASE=0x01008000;
TEGRA3_UBOOT_TEXT_BASE=0x81008000;

# Default UBOOT IMAGE LABELS.
#
TEGRA2_UIMAGE_LABEL="Linux-tegra2";
TEGRA3_UIMAGE_LABEL="Linux-tegra3";

if [ "${target_board}" = "harmony" ]; then
	bootdev_type="nand";
	uboot_text_base=${TEGRA2_UBOOT_TEXT_BASE};
	uimage_label=${TEGRA2_UIMAGE_LABEL};
	if [[ "${target_rootdev}" == mtdblock* ]]; then
		rootdev_type="internal";
		if [ "${rootfs_type}" = "" ]; then
			rootfs_type="yaffs2";
		elif [ "${rootfs_type}" != "yaffs2" ]; then
			echo "Error: Invalid rootfs type (${rootfs_type}).";
			echo "Supported FS is yaffs2.";
			exit 1;
		fi;
	elif [ "${target_rootdev}" = "eth0" -o \
	       "${target_rootdev}" = "eth1" -o \
	       "${target_rootdev}" = "usb0" ]; then
		rootdev_type="network";
	elif [[ "${target_rootdev}" != mmcblk0p* && \
	        "${target_rootdev}" != mmcblk1p* && \
	        "${target_rootdev}" != sd* ]]; then
		echo "Error: Invalid target rootdev($target_rootdev).";
		usage 1;
	fi;
	if [ "${rootfs_size}" = "" ]; then
		rootfs_size=${HARMONY_ROOTFS_SIZE};
	fi;
	if [ "${odmdata}" = "" ]; then
		odmdata=${HARMONY__ODMDATA};
	fi;
	if [ "${ubootscript}" = "" ]; then
		if [ $rootdev_type = "network" ]; then
			ubootscript="${DFLT_BSF_DIR}/${HARMONY_NET_BSF}";
		else
			ubootscript="${DFLT_BSF_DIR}/${HARMONY_NAND_BSF}";
		fi;
	fi;
	if [ "${bctfile}" = "" ]; then
		bctfile="${DFLT_BCT_DIR}/${HARMONY_NAND_BCT}";
	fi;
	if [ "${cfgfile}" = "" ]; then
		cfgfile="${DFLT_CFG_DIR}/${HARMONY_NAND_CFG}";
	fi;
elif [ "${target_board}" = ventana ]; then
	bootdev_type="emmc";
	uboot_text_base=${TEGRA2_UBOOT_TEXT_BASE};
	uimage_label=${TEGRA2_UIMAGE_LABEL};
        if [[ "${target_rootdev}" == mmcblk0* ]]; then
		rootdev_type="internal";
		if [ "${rootfs_type}" = "" ]; then
			rootfs_type="ext3";
		elif [ "${rootfs_type}" != "ext3" ]; then
			echo "Error: Invalid rootfs type (${rootfs_type}).";
			echo "${target_rootdev} supports only ext3.";
			exit 1;
		fi;
	elif [ "${target_rootdev}" = "eth0" ]; then
		rootdev_type="network";
	elif [[ "${target_rootdev}" != mmcblk1p* && \
	       "${target_rootdev}" != sd* ]]; then
		echo "Error: Invalid target rootdev($target_rootdev).";
		usage 1;
	fi;
	if [ "${rootfs_size}" = "" ]; then
		rootfs_size=${VENTANA_ROOTFS_SIZE};
	fi;
	if [ "${odmdata}" = "" ]; then
		odmdata=${VENTANA__ODMDATA};
	fi;
	if [ "${ubootscript}" = "" ]; then
		if [ $rootdev_type = "network" ]; then
			ubootscript="${DFLT_BSF_DIR}/${VENTANA_NET_BSF}";
		else
			ubootscript="${DFLT_BSF_DIR}/${VENTANA_EMMC_BSF}";
		fi;
	fi;
	if [ "${bctfile}" = "" ]; then
		bctfile="${DFLT_BCT_DIR}/${VENTANA_EMMC_BCT}";
	fi;
	if [ "${cfgfile}" = "" ]; then
		cfgfile="${DFLT_CFG_DIR}/${VENTANA_EMMC_CFG}";
	fi;
elif [ "${target_board}" = "seaboard" ]; then
	if [ "${bootdev_type}" = "" ]; then
		bootdev_type="emmc";
	fi;
	uboot_text_base=${TEGRA2_UBOOT_TEXT_BASE};
	uimage_label=${TEGRA2_UIMAGE_LABEL};
	if [[ "${target_rootdev}" == mtdblock* ]]; then
		if [ $bootdev_type != "nand" ]; then
			echo "Error: Invalid target rootdev($target_rootdev).";
			usage 1;
		fi;
		rootdev_type="internal";
		# FIXME: internal nand dev is not supported on this release.
		echo "Error: Unsupported target rootdev($target_rootdev).";
		usage 1;
	elif [[ "${target_rootdev}" == mmcblk0p* ]]; then
		if [ $bootdev_type != "emmc" ]; then
			echo "Error: Invalid target rootdev($target_rootdev).";
			usage 1;
		fi;
		rootdev_type="internal";
		# FIXME: internal emmc is not supported on this release.
		echo "Error: Unsupported target rootdev($target_rootdev).";
		usage 1;
	elif [ "${target_rootdev}" = "eth0" ]; then
		rootdev_type="network";
	elif [[ "${target_rootdev}" != mmcblk1p* && \
	       "${target_rootdev}" != sd* ]]; then
		echo "Error: Invalid target rootdev($target_rootdev).";
		usage 1;
	fi;
	if [ "${rootfs_size}" = "" ]; then
		rootfs_size=${SEABOARD_ROOTFS_SIZE};
	fi;
	if [ "${odmdata}" = "" ]; then
		odmdata=${SEABOARD_ODMDATA};
	fi;
	if [ "${ubootscript}" = "" ]; then
		if [ rbootdev_type = "nand" ]; then
			ubootscript="${DFLT_BSF_DIR}/${SEABOARD_NAND_BSF}";
		elif [ $rootdev_type = "network" ]; then
			ubootscript="${DFLT_BSF_DIR}/${SEABOARD_NET_BSF}";
		else
			ubootscript="${DFLT_BSF_DIR}/${SEABOARD_EMMC_BSF}";
		fi;
	fi;
	if [ "${bctfile}" = "" ]; then
		if [ $bootdev_type = "nand" ]; then
			bctfile="${DFLT_BCT_DIR}/${SEABOARD_NAND_BCT}";
		else
			bctfile="${DFLT_BCT_DIR}/${SEABOARD_EMMC_BCT}";
		fi;
	fi;
	if [ "${cfgfile}" = "" ]; then
		if [ $bootdev_type = "nand" ]; then
			cfgfile="${DFLT_CFG_DIR}/${SEABOARD_NAND_CFG}";
		else
			cfgfile="${DFLT_CFG_DIR}}/${SEABOARD_EMMC_CFG}";
		fi;
	fi;
elif [ "${target_board}" = "cardhu" ]; then
	bootdev_type="emmc";
	uboot_text_base=${TEGRA3_UBOOT_TEXT_BASE};
	uimage_label=${TEGRA3_UIMAGE_LABEL};
	if [[ "${target_rootdev}" == mmcblk0p* ]]; then
		rootdev_type="internal";
		if [ "${rootfs_type}" = "" ]; then
			rootfs_type="ext3";
		elif [ "${rootfs_type}" != "ext3" ]; then
			echo "Error: Invalid rootfs type (${rootfs_type}).";
			echo "${target_rootdev} supports only ext3.";
			exit 1;
		fi;
	elif [ "${target_rootdev}" = "eth0" -o \
	       "${target_rootdev}" = "eth1" ]; then
		rootdev_type="network";
	elif [[ "${target_rootdev}" != mmcblk1p* && \
	       "${target_rootdev}" != sd* ]]; then
		echo "Error: Invalid target rootdev($target_rootdev).";
		usage 1;
	fi;
	if [ "${rootfs_size}" = "" ]; then
		rootfs_size=${CARDHU_ROOTFS_SIZE};
	fi;
	if [ "${odmdata}" = "" ]; then
		if [ "${target_sku}" = "beaver" ]; then
			odmdata=${BEAVER___ODMDATA};
		else
			odmdata=${CARDHU___ODMDATA};
		fi;
	fi;
	if [ "${ubootscript}" = "" ]; then
		if [ $rootdev_type = "network" ]; then
			ubootscript="${DFLT_BSF_DIR}/${CARDHU_NET_BSF}";
		else
			ubootscript="${DFLT_BSF_DIR}/${CARDHU_EMMC_BSF}";
		fi;
	fi;
	if [ "${bctfile}" = "" ]; then
		if [ "${target_sku}" = "beaver" ]; then
			bctfile="${DFLT_BCT_DIR}/${BEAVER_EMMC_BCT}";
		else
			bctfile="${DFLT_BCT_DIR}/${CARDHU_EMMC_BCT}";
		fi;
	fi;
	if [ "${cfgfile}" = "" ]; then
		cfgfile="${DFLT_CFG_DIR}/${CARDHU_EMMC_CFG}";
	fi;
elif [ "${target_board}" = "cardhu-a05" ]; then
	bootdev_type="emmc";
	uboot_text_base=${TEGRA3_UBOOT_TEXT_BASE};
	uimage_label=${TEGRA3_UIMAGE_LABEL};
	if [[ "${target_rootdev}" == mmcblk0p* ]]; then
		rootdev_type="internal";
		if [ "${rootfs_type}" = "" ]; then
			rootfs_type="ext3";
		elif [ "${rootfs_type}" != "ext3" ]; then
			echo "Error: Invalid rootfs type (${rootfs_type}).";
			echo "${target_rootdev} supports only ext3.";
			exit 1;
		fi;
	elif [ "${target_rootdev}" = "eth0" -o \
	       "${target_rootdev}" = "eth1" ]; then
		rootdev_type="network";
	elif [[ "${target_rootdev}" != mmcblk1p* && \
	       "${target_rootdev}" != sd* ]]; then
		echo "Error: Invalid target rootdev($target_rootdev).";
		usage 1;
	fi;
	if [ "${rootfs_size}" = "" ]; then
		rootfs_size=${CARDHU_ROOTFS_SIZE};
	fi;
	if [ "${odmdata}" = "" ]; then
		odmdata=${CARDHU_A05___ODMDATA};
	fi;
	if [ "${ubootscript}" = "" ]; then
		if [ $rootdev_type = "network" ]; then
			ubootscript="${DFLT_BSF_DIR}/${CARDHU_NET_BSF}";
		else
			ubootscript="${DFLT_BSF_DIR}/${CARDHU_EMMC_BSF}";
		fi;
	fi;
	if [ "${bctfile}" = "" ]; then
		bctfile="${DFLT_BCT_DIR}/${CARDHU_A05_EMMC_BCT}";
	fi;
	if [ "${cfgfile}" = "" ]; then
		cfgfile="${DFLT_CFG_DIR}/${CARDHU_EMMC_CFG}";
	fi;
else
	echo "Error: Invalid target board."
	usage 1
fi

if [ "${flashapp}" = "" ]; then
	flashapp=${LDK_DIR}/bootloader/${DFLT_FLASHAPPNAME};
fi;
flashappname=`basename ${flashapp}`;
if [ "${flasher}" = "" ]; then
	flasher=${LDK_DIR}/bootloader/${target_board}/${DFLT_FLASHERNAME};
fi;
flashername=`basename ${flasher}`;
if [ "${bootloader}" = "" ]; then
	bootloader=${LDK_DIR}/bootloader/${target_board}/${DFLT_BOOTLOADERNAME};
fi;
bootloadername=`basename ${bootloader}`;

if [ "${zflag}" = "true" ]; then
	pr_conf ;
	exit 0;
fi;

if [ ! -f "${flashapp}" ]; then
	echo "Error: missing host side flash application($flashapp).";
	usage 1;
fi;
if [ ! -f "${flasher}" ]; then
	echo "Error: missing target side flash application($flasher).";
	usage 1;
fi;
if [ ! -f "${bootloader}" ]; then
	echo "Error: missing bootloader($bootloader).";
	usage 1;
fi;

if [ $target_partid -lt $MIN_KERN_PARTID ]; then
	if [ ! -f $bctfile ]; then
		echo "Error: missing BCT file($bctfile).";
		usage 1;
	fi;
	if [ ! -f $cfgfile ]; then
		echo "Error: missing config file($cfgfile).";
		usage 1;
	fi;
fi;
if [ ! -f "${kernel_image}" ]; then
	echo "Error: missing kernel image file($kernel_image).";
	usage 1;
fi;

if [ "${rootdev_type}" = "internal" ]; then
	if [ "${bootdev_type}" = "nand" ]; then
		# FIXME: nand boot need to be supported.
		if [ "${bootloadername}" = "u-boot.bin" ]; then
			echo "Error: U-boot is not supported for NAND RootFS.";
			exit 1;
		fi;
		dpkg -s mtd-utils > /dev/null 2>&1;
		if [ $? -ne 0 ]; then
			echo
			echo "Install mtd-utils package and run flash.sh.";
			echo "    ex) apt-get install mtd-utils";
			exit 1;
		fi;
	fi;
	if [ ${reuse_systemimg} = "false" ]; then
		if [ -z "${rootfs}" -o ! -d "${rootfs}" ]; then
			echo "Error: missing rootfs($rootfs).";
			popd > /dev/null 2>&1;
			usage 1;
		fi;
	fi;
fi;

nvflashfolder=${LDK_DIR}/bootloader;
if [ ! -d $nvflashfolder ]; then
	echo "Error: missing nvflash folder($nfvlashfolder).";
	usage 1;
fi;

pushd $nvflashfolder > /dev/null 2>&1;

RECOVERY_TAG="-e /filename=recovery.img/d";
if [ -f recovery.img ]; then
	RECOVERY_TAG="-e s/#filename=recovery.img/filename=recovery.img/";
fi;

MKUARG="";
mkimageapp="";
if [ "${bootloadername}" = "u-boot.bin" ]; then
	MKUARG+="-A arm ";
	MKUARG+="-O linux ";
	MKUARG+="-T kernel ";
	MKUARG+="-C none ";
	MKUARG+="-a ${uboot_text_base} ";
	MKUARG+="-e ${uboot_text_base} ";
	MKUARG+="-n ${uimage_label} ";
	MKUARG+="-d ${kernel_image} ";
	mkimageapp=${LDK_DIR}/bootloader/mkimage;
fi;

if [ "${rootdev_type}" = "external" -o "${rootdev_type}" = "network" ]; then
	if [ "${rootdev_type}" = "network" ]; then
		if [ "${nfsroot}" != "" ]; then
			nfsdargs="root=/dev/nfs rw netdevwait";
			cmdline+="${nfsdargs} ";
			if [ "${nfsargs}" != "" ]; then
				nfsiargs="ip=${nfsargs}";
				nfsiargs+="::${target_rootdev}:off";
			else
				nfsiargs="ip=:::::${target_rootdev}:on";
			fi;
			cmdline+="${nfsiargs} ";
			cmdline+="nfsroot=${nfsroot} ";
		fi;
	fi;
elif [ $target_partid -ge $MIN_KERN_PARTID ]; then
	echo "Just updating kernel and boot device.";
	if [ "${bootloadername}" = "u-boot.bin" ]; then
		echo "Error: not supported for u-boot.";
		exit 1;
	fi;
elif [ ${reuse_systemimg} = "true" ]; then
	echo "Reusing existing system.img... ";
	if [ ! -e system.img ]; then
		echo "file does not exist.";
		popd > /dev/null 2>&1;
		exit 1;
	fi;
	echo "done.";
elif [ "${rootfs_type}" = "ext3" ]; then
	if [ "${bootloadername}" = "u-boot.bin" ]; then
		echo -e -n "\tpopulating kernel to rootfs... ";
		mkdir -p ${rootfs}/boot;
		rm -f ${rootfs}/boot/${uimage_name};
		${mkimageapp} ${MKUARG} ${rootfs}/boot/${uimage_name};
		if [ $? -ne 0 ]; then
			echo "failed.";
			popd > /dev/null 2>&1;
			popd > /dev/null 2>&1;
			exit 1;
		fi;
		echo "done.";
	fi;
	echo "Making system.img... ";
	umount /dev/loop0 > /dev/null 2>&1;
	losetup -d /dev/loop0 > /dev/null 2>&1;
	rm -f system.img;
	if [ $? -ne 0 ]; then
		echo "clearing system.img failed.";
		popd > /dev/null 2>&1;
		exit 1;
	fi;
	bcnt=$(( ${rootfs_size} / 512 ));
	bcntdiv=$(( ${rootfs_size} % 512 ));
	if [ $bcnt -eq 0 -o $bcntdiv -ne 0 ]; then
		echo "Error: root file system size has to be 512 bytes allign.";
		popd > /dev/null 2>&1;
		exit 1;
	fi
	dd if=/dev/zero of=system.img bs=512 count=$bcnt > /dev/null 2>&1;
	if [ $? -ne 0 ]; then
		echo "making system.img failed.";
		popd > /dev/null 2>&1;
		exit 1;
	fi;
	losetup /dev/loop0 system.img > /dev/null 2>&1;
	if [ $? -ne 0 ]; then
		echo "mapping system.img to loop device failed.";
		popd > /dev/null 2>&1;
		exit 1;
	fi;
	mkfs -t ext3 /dev/loop0 > /dev/null 2>&1;
	if [ $? -ne 0 ]; then
		echo "formating filesystem on system.img failed.";
		popd > /dev/null 2>&1;
		exit 1;
	fi;
	sync;
	rm -rf mnt;
	if [ $? -ne 0 ]; then
		echo "clearing mount point failed.";
		popd > /dev/null 2>&1;
		exit 1;
	fi;
	mkdir -p mnt;
	if [ $? -ne 0 ]; then
		echo "making mount point failed.";
		popd > /dev/null 2>&1;
		exit 1;
	fi;
	mount /dev/loop0 mnt > /dev/null 2>&1;
	if [ $? -ne 0 ]; then
		echo "mounting system.img failed.";
		popd > /dev/null 2>&1;
		exit 1;
	fi;
	pushd mnt > /dev/null 2>&1;
	echo -n -e "\tpopulating rootfs from ${rootfs}... ";
	(cd ${rootfs}; tar cf - *) | tar xf - ;
	if [ $? -ne 0 ]; then
		echo "failed.";
		popd > /dev/null 2>&1;
		popd > /dev/null 2>&1;
		exit 1;
	fi;
	echo "done.";

	popd > /dev/null 2>&1;
	echo -e -n "\tSync'ing... ";
	sync; sync;	# Paranoid.
	echo "done.";
	umount /dev/loop0 > /dev/null 2>&1;
	losetup -d /dev/loop0 > /dev/null 2>&1;
	rmdir mnt > /dev/null 2>&1;
	echo "System.img built successfully. ";
elif [ "${rootfs_type}" = "yaffs2" ]; then
	if [ "${bootloadername}" = "u-boot.bin" ]; then
		echo -e -n "\tpopulating kernel to rootfs... ";
		mkdir -p ${rootfs}/boot;
		rm -f ${rootfs}/boot/${uimage_name};
		${mkimageapp} ${MKUARG} ${rootfs}/boot/${uimage_name};
		if [ $? -ne 0 ]; then
			echo "failed.";
			popd > /dev/null 2>&1;
			popd > /dev/null 2>&1;
			exit 1;
		fi;
		echo "done.";
	fi;
	echo "Making system.img... ";
	#
	# Other yaffs2 tools:
	#	./mkyaffs2 [-h|v] [-p 2048] [-o oobimg] ${rootfs} system.img;
	#	./mkyaffs2 -v ${rootfs} system.img;
	#
	./mkyaffs2image ${rootfs} system.img;
else
	echo "Error: Unsupported rootfs type (${rootfs_type}).";
	exit 1;
fi;

if [ $target_partid -ge $MIN_KERN_PARTID ]; then
	rm -f flash.bct > /dev/null 2>&1;
	rm -f flash.cfg > /dev/null 2>&1;
else
	echo -n "copying bctfile(${bctfile})... ";
	cp -f $bctfile flash.bct
	if [ $? -ne 0 ]; then
		echo "failed.";
		popd > /dev/null 2>&1;
		exit 1;
	fi;
	echo "done.";

	echo -n "copying cfgfile(${cfgfile})... ";
	CFGCONV=" ";
	if [ "${bootloadername}" != "u-boot.bin" ]; then
	    CFGCONV+="-e s/\#filename=flashboot.img/filename=boot.img/ ";
	    CFGCONV+="-e s/filename=flashboot.img/filename=boot.img/ ";

	else
	    if [ "${ubootscript}" != "" -a -f ${ubootscript} ]; then
		CFGCONV+="-e s/\#filename=flashboot.img/filename=boot.img/ ";
		CFGCONV+="-e s/filename=flashboot.img/filename=boot.img/ ";
	    else
		CFGCONV+="-e /\#filename=flashboot.img/d ";
		CFGCONV+="-e /filename=flashboot.img/d ";
		CFGCONV+="-e /filename=boot.img/d ";
	    fi;
	fi;
	CFGCONV+="-e s/filename=bootloader.bin/filename=${bootloadername}/ ";
	CFGCONV+="-e s/filename=fastboot.bin/filename=${bootloadername}/ ";
	CFGCONV+="-e s/filename=u-boot.bin/filename=${bootloadername}/ ";
	CFGCONV+="-e s/\#filename=bootloader.bin/filename=${bootloadername}/ ";
	CFGCONV+="-e s/\#filename=fastboot.bin/filename=${bootloadername}/ ";
	CFGCONV+="-e s/\#filename=u-boot.bin/filename=${bootloadername}/ ";
	CFGCONV+="-e s/size=268435456/size=${rootfs_size}/ ";
	CFGCONV+="-e s/size=134217728/size=${rootfs_size}/ ";
	CFGCONV+="-e s/size=734003200/size=${rootfs_size}/ ";
	CFGCONV+="-e s/size=471859200/size=${rootfs_size}/ ";
	CFGCONV+="-e s/size=1073741824/size=${rootfs_size}/ ";
	CFGCONV+="-e s/size=402653184/size=${rootfs_size}/ ";
	CFGCONV+="${RECOVERY_TAG} ";
	CFGCONV+="-e s/\#filename=yaffs2_gnu_system.img/filename=system.img/ ";
	CFGCONV+="-e s/filename=yaffs2_gnu_system.img/filename=system.img/ ";
	if [ "${rootfs_type}" != "yaffs2" ]; then
		CFGCONV+="-e s/filesystem_type=yaffs2/filesystem_type=basic/ ";
	fi;
	if [ "${rootdev_type}" = "external" -o \
	     "${rootdev_type}" = "network" ]; then
		CFGCONV+="-e /filename=system.img/d ";
		CFGCONV+="-e /filename=yaffs2_gnu_system.img/d ";
	fi;
	cat ${cfgfile} | sed ${CFGCONV} > flash.cfg;
	if [ $? -ne 0 ]; then
		echo "failed.";
		popd > /dev/null 2>&1;
		exit 1;
	fi;
	echo "done.";
fi;

if [ "${bootloader}" != "${nvflashfolder}/${bootloadername}" ]; then
	echo -n "copying bootloader(${bootloader})... ";
	cp -f ${bootloader} ${nvflashfolder}/${bootloadername};
	if [ $? -ne 0 ]; then
		echo "failed.";
		popd > /dev/null 2>&1;
		exit 1;
	fi;
	echo "done.";
else
	echo "Existing bootloader(${bootloader}) reused.";
fi;

if [ "${flasher}" != "${bootloader}" ]; then
	if [ "${flasher}" != "${nvflashfolder}/${flashername}" ]; then
		echo -n "copying flasher(${flasher})... ";
		cp -f ${flasher} ${nvflashfolder}/${flashername};
		if [ $? -ne 0 ]; then
			echo "failed.";
			popd > /dev/null 2>&1;
			exit 1;
		fi;
		echo "done.";
	else
		echo "Existing flasher(${flasher}) reused.";
	fi;
else
	echo "Bootloader(${bootloader}) used as flasher.";
fi;

if [ "${flashapp}" != "${nvflashfolder}/${flashappname}" ]; then
	echo -n "copying flash application(${flashapp})... ";
	cp -f ${flashapp} ${nvflashfolder}/${flashappname};
		if [ $? -ne 0 ]; then
		echo "failed.";
		popd > /dev/null 2>&1;
		exit 1;
	fi;
	echo "done.";
else
	echo "Existing flash application(${flashapp}) reused.";
fi;

if [ "${bootloadername}" = "u-boot.bin" ]; then
	echo -n "generating boot script(${ubootscript}) image... ";
	if [ "${ubootscript}" != "" -a -f ${ubootscript} ]; then
		ubootscriptname=`basename ${ubootscript}`;
		NFSCONV="-e s/bootdelay=3/bootdelay=${bootdelay}/ ";
		NFSCONV+="-e s/IPADDR/${ipaddr}/ ";
		NFSCONV+="-e s/SERVERIP/${serverip}/ ";
		NFSCONV+="-e s/GATEWAYIP/${gatewayip}/ ";
		NFSCONV+="-e s/NETMASK/${netmask}/ ";
		NFSCONV+="-e s%TFTPPATH%${tftppath}% ";
		NFSCONV+="-e s/NFSARGS/${nfsiargs}/ ";
		NFSCONV+="-e s%NFSROOT%${nfsroot}% ";
		cat ${ubootscript} | sed ${NFSCONV} > ./${ubootscriptname};
		./mkubootscript -i ./${ubootscriptname} -o ./boot.img
		if [ $? -ne 0 ]; then
			echo "Error: Encoding failed.";
			popd > /dev/null 2>&1;
			exit 1;
		else
			echo "done.";
		fi;
	else
		echo "Missing. Using embedded bootscript.";
	fi;
else
	if [ "$kernel_initrd" != "" -a -f "$kernel_initrd" ]; then
		echo -n "copying initrd(${kernel_initrd})... ";
		cp -f ${kernel_initrd} initrd;
		if [ $? -ne 0 ]; then
			echo "failed.";
			popd > /dev/null 2>&1;
			exit 1;
		fi;
		echo "done.";
	else
		echo "making zero initrd... ";
		rm -f initrd;
		if [ $? -ne 0 ]; then
			echo "failed.";
			popd > /dev/null 2>&1;
			exit 1;
		fi;
		touch initrd;
		if [ $? -ne 0 ]; then
			echo "failed.";
			popd > /dev/null 2>&1;
			exit 1;
		fi;
		echo "done.";
	fi;

	echo -n "Making Boot image... "
	MKBOOTARG="";
	MKBOOTARG+="--kernel ${kernel_image} ";
	MKBOOTARG+="--ramdisk initrd ";
	MKBOOTARG+="--board ${target_rootdev} ";
	MKBOOTARG+="--output boot.img";
	if [ "x${ignorefastbootcmdline}" == "xtrue" ]; then
		cmdline="ignorefastboot ${usercmdline}";
	else
		cmdline+="${usercmdline}";
	fi
	./mkbootimg ${MKBOOTARG} --cmdline "${cmdline}" > /dev/null 2>&1;
	if [ $? -ne 0 ]; then
		echo "failed.";
		popd > /dev/null 2>&1;
		exit 1;
	fi;
	echo "done";
fi;

if [ $target_partid -ge ${MIN_KERN_PARTID} ]; then
	echo "*** Flashing kernel update started. ***"
	FLASHARGS="--download ${target_partid} boot.img ";
	FLASHARGS+="--bl ${flashername} --go";
	LD_LIBRARY_PATH=. ./${flashappname} ${FLASHARGS};
	if [ $? -ne 0 ]; then
		echo "Failed to flash ${target_board}."
		popd > /dev/null 2>&1;
		exit 2
	fi;
	echo "*** The kernel has been updated successfully. ***"
	popd > /dev/null 2>&1;
	exit 0;
fi;

echo "*** Flashing target device started. ***"
FLASHARGS="--bct flash.bct --setbct --configfile flash.cfg ";
FLASHARGS+="--create --bl ${flashername} --odmdata $odmdata --go";
LD_LIBRARY_PATH=. ./${flashappname} ${FLASHARGS};
if [ $? -ne 0 ]; then
	echo "Failed to flash ${target_board}."
	popd > /dev/null 2>&1;
	exit 3
fi;

echo "*** The target ${target_board} has been flashed successfully. ***"
if [ "${rootdev_type}" = "internal" ]; then
	if [ "${bootdev_type}" = "nand" ]; then
		echo "Reset the board to boot from internal NAND."
	else
		echo "Reset the board to boot from internal eMMC."
	fi;
elif [ "${rootdev_type}" = "network" ]; then
	if [ "${nfsroot}" != "" ]; then
		echo -n "Make target nfsroot(${nfsroot}) exported ";
		echo "on the network and reset the board to boot";
	else
		echo -n "Make the target nfsroot exported on the network, ";
		echo -n "configure your own DHCP server with ";
		echo -n "\"option-root=<nfsroot export path>;\" ";
		echo "preperly and reset the board to boot";
	fi;
else
	echo -n "Make the target filesystem available to the device ";
	echo "and reset the board to boot from external ${target_rootdev}."
fi;
echo
popd > /dev/null 2>&1;
exit 0;
