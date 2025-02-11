#!/bin/bash

readonly BASEDIR=$(readlink -f $(dirname $0))/../../

source "${BASEDIR}/script/common.sh"

QEMU_PATH=${BASEDIR}/lib/qemu/
QEMU_BUILD_PATH=${QEMU_PATH}/qemu-7.1.0/build/
SMDK_KERNEL_PATH=${BASEDIR}/lib/linux-6.0-rc6-smdk
ROOTFS_PATH=${QEMU_PATH}

QEMU_SYSTEM_BINARY=${QEMU_BUILD_PATH}/qemu-system-x86_64
BZIMAGE_PATH=${SMDK_KERNEL_PATH}/arch/x86_64/boot/bzImage
IMAGE_PATH=${ROOTFS_PATH}/qemu-image-2.img

if [ ! -f "${QEMU_SYSTEM_BINARY}" ]; then
	log_error "qemu-system-x86_64 binary does not exist. Run 'build_lib.sh qemu' in /path/to/SMDK/lib/"
	exit 2
fi

if [ ! -f "${BZIMAGE_PATH}" ]; then
	log_error "SMDK kernel image does not exist. Run 'build_lib.sh kernel' in /path/to/SMDK/lib/"
	exit 2
fi

if [ ! -f "${IMAGE_PATH}" ]; then
	log_error "QEMU rootfs ${IMAGE_PATH} does not exist. Run 'create_rootfs.sh' in /path/to/SMDK/lib/qemu/"
	exit 2
fi
 
sudo ${QEMU_SYSTEM_BINARY} \
    -smp 6 \
    -numa node,cpus=0-5,memdev=mem0,nodeid=0 \
    -object memory-backend-ram,id=mem0,size=4G \
    -kernel ${BZIMAGE_PATH} \
    -drive file=${IMAGE_PATH},index=0,media=disk,format=raw \
    -enable-kvm \
    -monitor telnet::45452,server,nowait \
    -serial mon:stdio \
    -nographic \
    -append "root=/dev/sda rw console=ttyS0 nokaslr memblock=debug loglevel=7" \
    -m 4G \
    -net user,net=10.0.2.0/8,host=10.0.2.2,dhcpstart=10.0.2.22,hostfwd=tcp::2242-:22,hostfwd=tcp::6372-:6379,hostfwd=tcp::11212-:11211, \
    -net nic \
