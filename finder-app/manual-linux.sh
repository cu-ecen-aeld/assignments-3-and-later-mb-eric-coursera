#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.15.163
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE0=aarch64-none-linux-gnu
CROSS_COMPILE=${CROSS_COMPILE0}-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # TODO: Add your kernel build steps here
    make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- mrproper
    make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- defconfig
    make -j4 ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- all
    make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- modules
    make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- dtbs
fi


#start qemu with
#    QEMU_AUDIO_DRV=none qemu-system-arm -m 256M -nograhic -M versatilepb -kernel zImage 

echo "Adding the Image in outdir"

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]; then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories
mkdir -p ${OUTDIR}/rootfs/bin ${OUTDIR}/rootfs/dev ${OUTDIR}/rootfs/etc ${OUTDIR}/rootfs/home ${OUTDIR}/rootfs/lib
mkdir -p ${OUTDIR}/rootfs/lib64 ${OUTDIR}/rootfs/proc ${OUTDIR}/rootfs/sbin ${OUTDIR}/rootfs/sys ${OUTDIR}/rootfs/tmp
mkdir -p ${OUTDIR}/rootfs/usr ${OUTDIR}/rootfs/var
mkdir -p ${OUTDIR}/rootfs/usr/bin ${OUTDIR}/rootfs/usr/lib ${OUTDIR}/rootfs/usr/sbin
mkdir -p ${OUTDIR}/rootfs/var/log

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
    git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
    make distclean
    make defconfig
else
    cd busybox
fi

# TODO: Make and install busybox
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install

echo "Library dependencies"
${CROSS_COMPILE}readelf -a ${OUTDIR}/rootfs/bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a ${OUTDIR}/rootfs/bin/busybox | grep "Shared library"

#      [Requesting program interpreter: /lib/ld-linux-aarch64.so.1]
##
# 0x0000000000000001 (NEEDED)             Shared library: [libm.so.6]
# 0x0000000000000001 (NEEDED)             Shared library: [libresolv.so.2]
# 0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]
##

DEPLIBDIR_ROOT=$(realpath $(dirname $(which ${CROSS_COMPILE}gcc))/..)

cp ${DEPLIBDIR_ROOT}/${CROSS_COMPILE0}/libc/lib/ld-linux-aarch64.so.1 ${OUTDIR}/rootfs/lib
cp ${DEPLIBDIR_ROOT}/${CROSS_COMPILE0}/libc/lib64/libm.so.6 ${OUTDIR}/rootfs/lib64
cp ${DEPLIBDIR_ROOT}/${CROSS_COMPILE0}/libc/lib64/libresolv.so.2 ${OUTDIR}/rootfs/lib64
cp ${DEPLIBDIR_ROOT}/${CROSS_COMPILE0}/libc/lib64/libc.so.6 ${OUTDIR}/rootfs/lib64

## DEPLIBDIR_ROOT
# /opt/gcc-arm/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu

# TODO: Make device nodes
sudo rm -f ${OUTDIR}/rootfs/dev/null
sudo rm -f ${OUTDIR}/rootfs/dev/console
sudo mknod -m 666 ${OUTDIR}/rootfs/dev/null    c 1 3
sudo mknod -m 620 ${OUTDIR}/rootfs/dev/console c 5 1

# TODO: Clean and build the writer utility
cd ${FINDER_APP_DIR}

make clean
make CROSS_COMPILE=${CROSS_COMPILE}


# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
cp writer ${OUTDIR}/rootfs/home/
cp finder.sh ${OUTDIR}/rootfs/home/
cp finder-test.sh ${OUTDIR}/rootfs/home/
cp autorun-qemu.sh ${OUTDIR}/rootfs/home/
mkdir -p ${OUTDIR}/rootfs/home/conf
cp conf/username.txt ${OUTDIR}/rootfs/home/conf/
cp conf/assignment.txt ${OUTDIR}/rootfs/home/conf/


# TODO: Chown the root directory
cd "$OUTDIR/rootfs"
find . | cpio --format=newc --create --verbose --owner root:root > ${OUTDIR}/initramfs.cpio
# TODO: Create initramfs.cpio.gz
gzip -f ${OUTDIR}/initramfs.cpio


cp ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}/
