#!/bin/bash

#
# Copyright (C) 2018 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

print_command() {
  binary=$1; shift
  binary_args=("$@")
  printf %s "${binary}"
  for i in "${binary_args[@]}"; do
    case "$i" in
      -*) printf "\\%s  %s " $'\n' "$i" ;;
      *) printf "%s " "$i" ;;
    esac
  done
  echo
}

exec_run() {
  binary=$1; shift
  binary_args=("$@")
  print_command "${binary}" "${binary_args[@]}"
  exec "${binary}" "${binary_args[@]}"
}

run() {
  binary=$1; shift
  binary_args=("$@")
  print_command "${binary}" "${binary_args[@]}"
  "${binary}" "${binary_args[@]}"
}

default_instance_number() {
    if [[ "${USER::5}" == "vsoc-" ]]; then
        echo "${USER: -2}"
    else
        echo "01"
    fi
}
CUTTLEFISH_INSTANCE="${CUTTLEFISH_INSTANCE:-$(default_instance_number)}"
default_instance_name="cvd-${CUTTLEFISH_INSTANCE}"
default_uuid="699acfc4-c8c4-11e7-882b-5065f31dc1${CUTTLEFISH_INSTANCE}"
default_dir="${HOME}/cuttlefish_runtime"
default_internal_dir="${default_internal_dir}"
default_mobile_tap_name="cvd-mtap-${CUTTLEFISH_INSTANCE}"
default_wifi_tap_name="cvd-wtap-${CUTTLEFISH_INSTANCE}"

qemu_binary=${qemu_binary=/usr/bin/qemu-system-x86_64}
dtc_binary=${dtc_binary:-dtc}

if [[ -z "${ivshmem_vector_count}" ]]; then
    echo "The required ivshmem_vector_count environment variable is not set" >&2
    exit 1
fi

if [[ "${qemu_binary##*/}" = "qemu-system-aarch64" ]]; then
  # On ARM, the early console can be PCI, and ISA is not supported
  kernel_console_serial="pci-serial"
  machine="virt,gic_version=2"
  cpu=cortex-a53
else
  # On x86, the early console must be ISA, not PCI, so we start to get kernel
  # messages as soon as possible. ISA devices do not have 'addr' assignments.
  kernel_console_serial="isa-serial"
  machine="pc-i440fx-2.8,accel=kvm"
  cpu=host
fi

# Put anything here that might affect the machine configuration generated by
# QEMU. Anything which connects statefully to another service (like a socket)
# should be added in another section below.
args=(
    -name "guest=${instance_name:-${default_instance_name}},debug-threads=on"
    -machine "${machine},usb=off,dump-guest-core=off"
    -m "${memory_mb:-2048}"
    -realtime mlock=off
    -smp "${cpus:-2},sockets=${cpus:-2},cores=1,threads=1"
    -uuid "${uuid:-${default_uuid}}"
    -display none
    -no-user-config
    -nodefaults
    -rtc "base=utc"
    -no-shutdown
    -boot "strict=on"
    -kernel "${kernel_image_path:-${HOME}/kernel}"
    -append "${kernel_cmdline:-"loop.max_part=7 console=ttyS0 androidboot.console=ttyS1 androidboot.hardware=vsoc enforcing=0 audit=1 androidboot.selinux=permissive mac80211_hwsim.radios=0 security=selinux buildvariant=userdebug  androidboot.serialno=CUTTLEFISHCVD01 androidboot.lcd_density=160 androidboot.boot_devices=pci0000:00/0000:00:03.0"}"
    -device "piix3-usb-uhci,id=usb,addr=0x1.0x2"
    -device "virtio-serial-pci,id=virtio-serial0"
)

IFS=';' read -ra virtual_disk_array <<< "$virtual_disk_paths"
virtual_disk_index=0
for virtual_disk in "${virtual_disk_array[@]}"; do
  if [[ $virtual_disk_index == 0 ]]; then
    bootindex=",bootindex=1"
  else
    bootindex=""
  fi
  args+=(
    -drive "file=${virtual_disk},format=raw,if=none,id=drive-virtio-disk${virtual_disk_index},aio=threads"
    -device "virtio-blk-pci,scsi=off,drive=drive-virtio-disk${virtual_disk_index},id=virtio-disk${virtual_disk_index}${bootindex}"
  )
  virtual_disk_index=$((virtual_disk_index + 1))
done

args+=(
    -netdev "tap,id=hostnet0,ifname=${wifi_tap_name:-${default_wifi_tap_name}},script=no,downscript=no"
    -device "virtio-net-pci,netdev=hostnet0,id=net0"
    -netdev "tap,id=hostnet1,ifname=${mobile_tap_name:-${default_mobile_tap_name}},script=no,downscript=no"
    -device "virtio-net-pci,netdev=hostnet1,id=net1"
    -device "virtio-balloon-pci,id=balloon0"
    -object "rng-random,id=objrng0,filename=/dev/urandom"
    -device "virtio-rng-pci,rng=objrng0,id=rng0,max-bytes=1024,period=2000"
    -cpu "${cpu}"
    -msg "timestamp=on"
    -device "AC97"
)

# The services providing these sockets don't expect multiple connections,
# so we must not have them in 'args' when we dump the machine FDT. It's
# OK to add them now, after the dumping and patching has completed.
# The (maybe patched) DTB can also be provided now.

if [[ "${use_bootloader}" = "true" ]]; then
  args+=(
    -bios "${bootloader}"
  )
fi

args+=(
    -chardev "socket,id=charmonitor,path=${monitor_path:-${default_internal_dir}/qemu_monitor.sock},server,nowait"
    -mon "chardev=charmonitor,id=monitor,mode=control"
    -chardev "file,id=charserial0,path=${kernel_log_pipe_name:-${default_internal_dir}/kernel-log},append=on"
    -device "${kernel_console_serial},chardev=charserial0,id=serial0"
    -chardev "socket,id=charserial1,path=${console_path:-${default_dir}/console},server,nowait"
    -device "${kernel_console_serial},chardev=charserial1,id=serial1"
    -chardev "socket,path=${ivshmem_qemu_socket_path:-${default_internal_dir}/ivshmem_socket_qemu},id=ivsocket"
    -device "ivshmem-doorbell,chardev=ivsocket,vectors=${ivshmem_vector_count}"
)

if [[ "${logcat_mode}" == "serial" ]]; then
    args+=(
        -chardev "file,id=charchannel0,path=${logcat_path:-${default_dir}/logcat},append=on"
        -device "virtserialport,bus=virtio-serial0.0,nr=1,chardev=charchannel0,id=channel0,name=cf-logcat"
    )
fi

if [[ -n "${gdb_flag}" ]]; then
  args+=(-gdb "${gdb_flag}")
fi

if [[ -n "${ramdisk_image_path}" ]]; then
  args+=(-initrd "${ramdisk_image_path}")
fi

if [[ -n "${usb_v1_socket_name}" ]]; then
  args+=(
      -chardev "socket,id=charchannel1,path=${usb_v1_socket_name:-${default_internal_dir}/usb-v1}"
      -device "virtserialport,bus=virtio-serial0.0,nr=2,chardev=charchannel1,id=channel1,name=cf-gadget-usb-v1"
  )
fi

if [[ ${vsock_guest_cid:-0} -gt 2 ]]; then
  args+=(-device "vhost-vsock-pci,guest-cid=${vsock_guest_cid}")
fi

export QEMU_AUDIO_DRV=none
exec_run "${qemu_binary}" "${args[@]}"
