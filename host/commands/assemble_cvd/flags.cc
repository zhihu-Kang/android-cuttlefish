#include "host/commands/assemble_cvd/flags.h"

#include <sys/statvfs.h>

#include <algorithm>
#include <iostream>
#include <fstream>

#include <android-base/strings.h>
#include <gflags/gflags.h>
#include <glog/logging.h>

#include "common/libs/utils/environment.h"
#include "common/libs/utils/files.h"
#include "host/commands/assemble_cvd/boot_image_unpacker.h"
#include "host/commands/assemble_cvd/data_image.h"
#include "host/commands/assemble_cvd/image_aggregator.h"
#include "host/commands/assemble_cvd/assembler_defs.h"
#include "host/commands/assemble_cvd/super_image_mixer.h"
#include "host/libs/config/fetcher_config.h"
#include "host/libs/vm_manager/crosvm_manager.h"
#include "host/libs/vm_manager/qemu_manager.h"
#include "host/libs/vm_manager/vm_manager.h"

using vsoc::ForCurrentInstance;
using cvd::AssemblerExitCodes;

DEFINE_string(cache_image, "", "Location of the cache partition image.");
DEFINE_string(metadata_image, "", "Location of the metadata partition image "
              "to be generated.");
DEFINE_int32(blank_metadata_image_mb, 16,
             "The size of the blank metadata image to generate, MB.");
DEFINE_int32(cpus, 2, "Virtual CPU count.");
DEFINE_string(data_image, "", "Location of the data partition image.");
DEFINE_string(data_policy, "use_existing", "How to handle userdata partition."
            " Either 'use_existing', 'create_if_missing', 'resize_up_to', or "
            "'always_create'.");
DEFINE_int32(blank_data_image_mb, 0,
             "The size of the blank data image to generate, MB.");
DEFINE_string(blank_data_image_fmt, "ext4",
              "The fs format for the blank data image. Used with mkfs.");
DEFINE_string(qemu_gdb, "",
              "Debug flag to pass to qemu. e.g. -qemu_gdb=tcp::1234");

DEFINE_int32(x_res, 720, "Width of the screen in pixels");
DEFINE_int32(y_res, 1280, "Height of the screen in pixels");
DEFINE_int32(dpi, 160, "Pixels per inch for the screen");
DEFINE_int32(refresh_rate_hz, 60, "Screen refresh rate in Hertz");
DEFINE_string(kernel_path, "",
              "Path to the kernel. Overrides the one from the boot image");
DEFINE_string(initramfs_path, "", "Path to the initramfs");
DEFINE_bool(decompress_kernel, false,
            "Whether to decompress the kernel image.");
DEFINE_string(extra_kernel_cmdline, "",
              "Additional flags to put on the kernel command line");
DEFINE_int32(loop_max_part, 7, "Maximum number of loop partitions");
DEFINE_bool(guest_enforce_security, true,
            "Whether to run in enforcing mode (non permissive).");
DEFINE_bool(guest_audit_security, true,
            "Whether to log security audits.");
DEFINE_bool(guest_force_normal_boot, true,
            "Whether to force the boot sequence to skip recovery.");
DEFINE_string(boot_image, "",
              "Location of cuttlefish boot image. If empty it is assumed to be "
              "boot.img in the directory specified by -system_image_dir.");
DEFINE_string(vendor_boot_image, "",
              "Location of cuttlefish vendor boot image. If empty it is assumed to "
              "be vendor_boot.img in the directory specified by -system_image_dir.");
DEFINE_int32(memory_mb, 2048,
             "Total amount of memory available for guest, MB.");
DEFINE_string(mobile_interface, ForCurrentInstance("cvd-mbr-"),
              "Network interface to use for mobile networking");
DEFINE_string(mobile_tap_name, ForCurrentInstance("cvd-mtap-"),
              "The name of the tap interface to use for mobile");
DEFINE_string(serial_number, ForCurrentInstance("CUTTLEFISHCVD"),
              "Serial number to use for the device");
DEFINE_string(assembly_dir,
              cvd::StringFromEnv("HOME", ".") + "/cuttlefish_assembly",
              "A directory to put generated files common between instances");
DEFINE_string(instance_dir,
              cvd::StringFromEnv("HOME", ".") + "/cuttlefish_runtime",
              "A directory to put all instance specific files");
DEFINE_string(
    vm_manager, vm_manager::CrosvmManager::name(),
    "What virtual machine manager to use, one of {qemu_cli, crosvm}");
DEFINE_string(
    gpu_mode, vsoc::kGpuModeGuestSwiftshader,
    "What gpu configuration to use, one of {guest_swiftshader, drm_virgl}");

DEFINE_string(system_image_dir, vsoc::DefaultGuestImagePath(""),
              "Location of the system partition images.");
DEFINE_string(super_image, "", "Location of the super partition image.");
DEFINE_string(misc_image, "",
              "Location of the misc partition image. If the image does not "
              "exist, a blank new misc partition image is created.");
DEFINE_string(composite_disk, "", "Location of the composite disk image. "
                                  "If empty, a composite disk is not used.");

DEFINE_bool(deprecated_boot_completed, false, "Log boot completed message to"
            " host kernel. This is only used during transition of our clients."
            " Will be deprecated soon.");
DEFINE_bool(start_vnc_server, false, "Whether to start the vnc server process.");

DEFINE_bool(start_webrtc, false, "Whether to start the webrtc process.");

DEFINE_string(
        webrtc_assets_dir,
        vsoc::DefaultHostArtifactsPath("usr/share/webrtc/assets"),
        "Path to WebRTC webpage assets.");

DEFINE_string(
        webrtc_certs_dir,
        vsoc::DefaultHostArtifactsPath("usr/share/webrtc/certs"),
        "Path to WebRTC certificates directory.");

DEFINE_string(
        webrtc_public_ip,
        "127.0.0.1",
        "Public IPv4 address of your server, a.b.c.d format");

DEFINE_bool(
        webrtc_enable_adb_websocket,
        false,
        "If enabled, exposes local adb service through a websocket.");

DEFINE_int32(vnc_server_port, ForCurrentInstance(6444),
             "The port on which the vnc server should listen");
DEFINE_string(adb_mode, "vsock_half_tunnel",
              "Mode for ADB connection."
              "'vsock_tunnel' for a TCP connection tunneled through vsock, "
              "'native_vsock' for a  direct connection to the guest ADB over "
              "vsock, 'vsock_half_tunnel' for a TCP connection forwarded to "
              "the guest ADB server, or a comma separated list of types as in "
              "'native_vsock,vsock_half_tunnel'");
DEFINE_bool(run_adb_connector, true,
            "Maintain adb connection by sending 'adb connect' commands to the "
            "server. Only relevant with -adb_mode=tunnel or vsock_tunnel");
DEFINE_string(wifi_tap_name, ForCurrentInstance("cvd-wtap-"),
              "The name of the tap interface to use for wifi");
DEFINE_int32(vsock_guest_cid,
             vsoc::GetDefaultPerInstanceVsockCid(),
             "Guest identifier for vsock. Disabled if under 3.");

DEFINE_string(uuid, vsoc::ForCurrentInstance(vsoc::kDefaultUuidPrefix),
              "UUID to use for the device. Random if not specified");
DEFINE_bool(daemon, false,
            "Run cuttlefish in background, the launcher exits on boot "
            "completed/failed");

DEFINE_string(device_title, "", "Human readable name for the instance, "
              "used by the vnc_server for its server title");
DEFINE_string(setupwizard_mode, "DISABLED",
            "One of DISABLED,OPTIONAL,REQUIRED");

DEFINE_string(qemu_binary,
              "/usr/bin/qemu-system-x86_64",
              "The qemu binary to use");
DEFINE_string(crosvm_binary,
              vsoc::DefaultHostArtifactsPath("bin/crosvm"),
              "The Crosvm binary to use");
DEFINE_bool(restart_subprocesses, true, "Restart any crashed host process");
DEFINE_string(logcat_mode, "", "How to send android's log messages from "
                               "guest to host. One of [serial, vsock]");
DEFINE_bool(enable_tombstone_receiver, true, "Enables the tombstone logger on "
            "both the guest and the host");
DEFINE_bool(use_bootloader, false, "Boots the device using a bootloader");
DEFINE_string(bootloader, "", "Bootloader binary path");
DEFINE_string(boot_slot, "", "Force booting into the given slot. If empty, "
             "the slot will be chosen based on the misc partition if using a "
             "bootloader. It will default to 'a' if empty and not using a "
             "bootloader.");

namespace {

const std::string kKernelDefaultPath = "kernel";
const std::string kInitramfsImg = "initramfs.img";
const std::string kRamdiskConcatExt = ".concat";

bool ResolveInstanceFiles() {
  if (FLAGS_system_image_dir.empty()) {
    LOG(ERROR) << "--system_image_dir must be specified.";
    return false;
  }

  // If user did not specify location of either of these files, expect them to
  // be placed in --system_image_dir location.
  std::string default_boot_image = FLAGS_system_image_dir + "/boot.img";
  SetCommandLineOptionWithMode("boot_image", default_boot_image.c_str(),
                               google::FlagSettingMode::SET_FLAGS_DEFAULT);
  std::string default_cache_image = FLAGS_system_image_dir + "/cache.img";
  SetCommandLineOptionWithMode("cache_image", default_cache_image.c_str(),
                               google::FlagSettingMode::SET_FLAGS_DEFAULT);
  std::string default_data_image = FLAGS_system_image_dir + "/userdata.img";
  SetCommandLineOptionWithMode("data_image", default_data_image.c_str(),
                               google::FlagSettingMode::SET_FLAGS_DEFAULT);
  std::string default_metadata_image = FLAGS_system_image_dir + "/metadata.img";
  SetCommandLineOptionWithMode("metadata_image", default_metadata_image.c_str(),
                               google::FlagSettingMode::SET_FLAGS_DEFAULT);
  std::string default_super_image = FLAGS_system_image_dir + "/super.img";
  SetCommandLineOptionWithMode("super_image", default_super_image.c_str(),
                               google::FlagSettingMode::SET_FLAGS_DEFAULT);
  std::string default_misc_image = FLAGS_system_image_dir + "/misc.img";
  SetCommandLineOptionWithMode("misc_image", default_misc_image.c_str(),
                               google::FlagSettingMode::SET_FLAGS_DEFAULT);
  std::string default_composite_disk = FLAGS_system_image_dir + "/composite.img";
  SetCommandLineOptionWithMode("composite_disk", default_composite_disk.c_str(),
                               google::FlagSettingMode::SET_FLAGS_DEFAULT);
  std::string default_vendor_boot_image = FLAGS_system_image_dir
                                        + "/vendor_boot.img";
  SetCommandLineOptionWithMode("vendor_boot_image",
                               default_vendor_boot_image.c_str(),
                               google::FlagSettingMode::SET_FLAGS_DEFAULT);

  return true;
}

std::string GetCuttlefishEnvPath() {
  return cvd::StringFromEnv("HOME", ".") + "/.cuttlefish.sh";
}

std::string GetLegacyConfigFilePath(const vsoc::CuttlefishConfig& config) {
  return config.ForDefaultInstance().PerInstancePath("cuttlefish_config.json");
}

int GetHostPort() {
  constexpr int kFirstHostPort = 6520;
  return vsoc::ForCurrentInstance(kFirstHostPort);
}

int NumStreamers() {
  auto start_flags = {FLAGS_start_vnc_server, FLAGS_start_webrtc};
  return std::count(start_flags.begin(), start_flags.end(), true);
}

// Initializes the config object and saves it to file. It doesn't return it, all
// further uses of the config should happen through the singleton
bool InitializeCuttlefishConfiguration(
    const cvd::BootImageUnpacker& boot_image_unpacker,
    const cvd::FetcherConfig& fetcher_config) {
  // At most one streamer can be started.
  CHECK(NumStreamers() <= 1);

  vsoc::CuttlefishConfig tmp_config_obj;
  tmp_config_obj.set_assembly_dir(FLAGS_assembly_dir);
  auto instance = tmp_config_obj.ForDefaultInstance();
  // Set this first so that calls to PerInstancePath below are correct
  instance.set_instance_dir(FLAGS_instance_dir);
  if (!vm_manager::VmManager::IsValidName(FLAGS_vm_manager)) {
    LOG(ERROR) << "Invalid vm_manager: " << FLAGS_vm_manager;
    return false;
  }
  if (!vm_manager::VmManager::IsValidName(FLAGS_vm_manager)) {
    LOG(ERROR) << "Invalid vm_manager: " << FLAGS_vm_manager;
    return false;
  }
  tmp_config_obj.set_vm_manager(FLAGS_vm_manager);
  tmp_config_obj.set_gpu_mode(FLAGS_gpu_mode);
  if (vm_manager::VmManager::ConfigureGpuMode(tmp_config_obj.vm_manager(),
                                              tmp_config_obj.gpu_mode()).empty()) {
    LOG(ERROR) << "Invalid gpu_mode=" << FLAGS_gpu_mode <<
               " does not work with vm_manager=" << FLAGS_vm_manager;
    return false;
  }

  instance.set_serial_number(FLAGS_serial_number);

  tmp_config_obj.set_cpus(FLAGS_cpus);
  tmp_config_obj.set_memory_mb(FLAGS_memory_mb);

  tmp_config_obj.set_dpi(FLAGS_dpi);
  tmp_config_obj.set_setupwizard_mode(FLAGS_setupwizard_mode);
  tmp_config_obj.set_x_res(FLAGS_x_res);
  tmp_config_obj.set_y_res(FLAGS_y_res);
  tmp_config_obj.set_refresh_rate_hz(FLAGS_refresh_rate_hz);
  tmp_config_obj.set_gdb_flag(FLAGS_qemu_gdb);
  std::vector<std::string> adb = android::base::Split(FLAGS_adb_mode, ",");
  tmp_config_obj.set_adb_mode(std::set<std::string>(adb.begin(), adb.end()));
  instance.set_host_port(GetHostPort());
  instance.set_adb_ip_and_port("127.0.0.1:" + std::to_string(GetHostPort()));

  instance.set_device_title(FLAGS_device_title);
  std::string discovered_kernel = fetcher_config.FindCvdFileWithSuffix(kKernelDefaultPath);
  std::string foreign_kernel = FLAGS_kernel_path.size() ? FLAGS_kernel_path : discovered_kernel;
  if (foreign_kernel.size()) {
    tmp_config_obj.set_kernel_image_path(foreign_kernel);
    tmp_config_obj.set_use_unpacked_kernel(false);
  } else {
    tmp_config_obj.set_kernel_image_path(
        tmp_config_obj.AssemblyPath(kKernelDefaultPath.c_str()));
    tmp_config_obj.set_use_unpacked_kernel(true);
  }
  tmp_config_obj.set_decompress_kernel(FLAGS_decompress_kernel);
  if (FLAGS_decompress_kernel) {
    tmp_config_obj.set_decompressed_kernel_image_path(
        tmp_config_obj.AssemblyPath("vmlinux"));
  }

  auto ramdisk_path = tmp_config_obj.AssemblyPath("ramdisk.img");
  auto vendor_ramdisk_path = tmp_config_obj.AssemblyPath("vendor_ramdisk.img");
  if (!boot_image_unpacker.HasRamdiskImage()) {
    LOG(INFO) << "A ramdisk is required, but the boot image did not have one.";
    return false;
  }

  tmp_config_obj.set_boot_image_kernel_cmdline(boot_image_unpacker.kernel_cmdline());
  tmp_config_obj.set_loop_max_part(FLAGS_loop_max_part);
  tmp_config_obj.set_guest_enforce_security(FLAGS_guest_enforce_security);
  tmp_config_obj.set_guest_audit_security(FLAGS_guest_audit_security);
  tmp_config_obj.set_guest_force_normal_boot(FLAGS_guest_force_normal_boot);
  tmp_config_obj.set_extra_kernel_cmdline(FLAGS_extra_kernel_cmdline);

  tmp_config_obj.set_virtual_disk_paths({FLAGS_composite_disk});

  tmp_config_obj.set_ramdisk_image_path(ramdisk_path);
  tmp_config_obj.set_vendor_ramdisk_image_path(vendor_ramdisk_path);

  std::string discovered_ramdisk = fetcher_config.FindCvdFileWithSuffix(kInitramfsImg);
  std::string foreign_ramdisk = FLAGS_initramfs_path.size () ? FLAGS_initramfs_path : discovered_ramdisk;
  if (foreign_kernel.size() && !foreign_ramdisk.size()) {
    // If there's a kernel that's passed in without an initramfs, that implies
    // user error or a kernel built with no modules. In either case, let's
    // choose to avoid loading the modules from the vendor ramdisk which are
    // built for the default cf kernel. Once boot occurs, user error will
    // become obvious.
    tmp_config_obj.set_final_ramdisk_path(ramdisk_path);
  } else {
    tmp_config_obj.set_final_ramdisk_path(ramdisk_path + kRamdiskConcatExt);
    if(foreign_ramdisk.size()) {
      tmp_config_obj.set_initramfs_path(foreign_ramdisk);
    }
  }

  tmp_config_obj.set_deprecated_boot_completed(FLAGS_deprecated_boot_completed);
  tmp_config_obj.set_logcat_receiver_binary(
      vsoc::DefaultHostArtifactsPath("bin/logcat_receiver"));
  tmp_config_obj.set_config_server_binary(
      vsoc::DefaultHostArtifactsPath("bin/config_server"));

  instance.set_mobile_bridge_name(FLAGS_mobile_interface);
  instance.set_mobile_tap_name(FLAGS_mobile_tap_name);

  instance.set_wifi_tap_name(FLAGS_wifi_tap_name);

  instance.set_vsock_guest_cid(FLAGS_vsock_guest_cid);

  instance.set_uuid(FLAGS_uuid);

  tmp_config_obj.set_qemu_binary(FLAGS_qemu_binary);
  tmp_config_obj.set_crosvm_binary(FLAGS_crosvm_binary);
  tmp_config_obj.set_console_forwarder_binary(
      vsoc::DefaultHostArtifactsPath("bin/console_forwarder"));
  tmp_config_obj.set_kernel_log_monitor_binary(
      vsoc::DefaultHostArtifactsPath("bin/kernel_log_monitor"));

  tmp_config_obj.set_enable_vnc_server(FLAGS_start_vnc_server);
  tmp_config_obj.set_vnc_server_binary(
      vsoc::DefaultHostArtifactsPath("bin/vnc_server"));
  instance.set_vnc_server_port(FLAGS_vnc_server_port);

  tmp_config_obj.set_enable_webrtc(FLAGS_start_webrtc);
  tmp_config_obj.set_webrtc_binary(
      vsoc::DefaultHostArtifactsPath("bin/webRTC"));
  tmp_config_obj.set_webrtc_assets_dir(FLAGS_webrtc_assets_dir);
  tmp_config_obj.set_webrtc_public_ip(FLAGS_webrtc_public_ip);
  tmp_config_obj.set_webrtc_certs_dir(FLAGS_webrtc_certs_dir);

  tmp_config_obj.set_webrtc_enable_adb_websocket(
          FLAGS_webrtc_enable_adb_websocket);

  tmp_config_obj.set_restart_subprocesses(FLAGS_restart_subprocesses);
  tmp_config_obj.set_run_adb_connector(FLAGS_run_adb_connector);
  tmp_config_obj.set_adb_connector_binary(
      vsoc::DefaultHostArtifactsPath("bin/adb_connector"));
  tmp_config_obj.set_socket_vsock_proxy_binary(
      vsoc::DefaultHostArtifactsPath("bin/socket_vsock_proxy"));
  tmp_config_obj.set_run_as_daemon(FLAGS_daemon);

  tmp_config_obj.set_data_policy(FLAGS_data_policy);
  tmp_config_obj.set_blank_data_image_mb(FLAGS_blank_data_image_mb);
  tmp_config_obj.set_blank_data_image_fmt(FLAGS_blank_data_image_fmt);

  tmp_config_obj.set_logcat_mode(FLAGS_logcat_mode);

  tmp_config_obj.set_enable_tombstone_receiver(FLAGS_enable_tombstone_receiver);
  tmp_config_obj.set_tombstone_receiver_binary(
      vsoc::DefaultHostArtifactsPath("bin/tombstone_receiver"));

  tmp_config_obj.set_use_bootloader(FLAGS_use_bootloader);
  tmp_config_obj.set_bootloader(FLAGS_bootloader);

  if (!FLAGS_boot_slot.empty()) {
      tmp_config_obj.set_boot_slot(FLAGS_boot_slot);
  }

  tmp_config_obj.set_cuttlefish_env_path(GetCuttlefishEnvPath());

  auto config_file = GetConfigFilePath(tmp_config_obj);
  auto config_link = vsoc::GetGlobalConfigFileLink();
  // Save the config object before starting any host process
  if (!tmp_config_obj.SaveToFile(config_file)) {
    LOG(ERROR) << "Unable to save config object";
    return false;
  }
  auto legacy_config_file = GetLegacyConfigFilePath(tmp_config_obj);
  if (!tmp_config_obj.SaveToFile(legacy_config_file)) {
    LOG(ERROR) << "Unable to save legacy config object";
    return false;
  }
  setenv(vsoc::kCuttlefishConfigEnvVarName, config_file.c_str(), true);
  if (symlink(config_file.c_str(), config_link.c_str()) != 0) {
    LOG(ERROR) << "Failed to create symlink to config file at " << config_link
               << ": " << strerror(errno);
    return false;
  }

  return true;
}

void SetDefaultFlagsForQemu() {
  // TODO(b/144119457) Use the serial port.
  SetCommandLineOptionWithMode("logcat_mode", cvd::kLogcatVsockMode,
                               google::FlagSettingMode::SET_FLAGS_DEFAULT);
}

void SetDefaultFlagsForCrosvm() {
  SetCommandLineOptionWithMode("logcat_mode", cvd::kLogcatVsockMode,
                               google::FlagSettingMode::SET_FLAGS_DEFAULT);
}

bool ParseCommandLineFlags(int* argc, char*** argv) {
  google::ParseCommandLineNonHelpFlags(argc, argv, true);
  bool invalid_manager = false;
  if (FLAGS_vm_manager == vm_manager::QemuManager::name()) {
    SetDefaultFlagsForQemu();
  } else if (FLAGS_vm_manager == vm_manager::CrosvmManager::name()) {
    SetDefaultFlagsForCrosvm();
  } else {
    std::cerr << "Unknown Virtual Machine Manager: " << FLAGS_vm_manager
              << std::endl;
    invalid_manager = true;
  }
  if (NumStreamers() == 0) {
    // This makes the vnc server the default streamer unless the user requests
    // another via a --star_<streamer> flag, while at the same time it's
    // possible to run without any streamer by setting --start_vnc_server=false.
    SetCommandLineOptionWithMode("start_vnc_server", "true",
                                 google::FlagSettingMode::SET_FLAGS_DEFAULT);
  }
  google::HandleCommandLineHelpFlags();
  if (invalid_manager) {
    return false;
  }
  // Set the env variable to empty (in case the caller passed a value for it).
  unsetenv(vsoc::kCuttlefishConfigEnvVarName);

  return ResolveInstanceFiles();
}

bool CleanPriorFiles() {
  // Everything on the instance directory
  std::string prior_files = FLAGS_instance_dir + "/*";
  // Everything in the assembly directory
  prior_files += " " + FLAGS_assembly_dir + "/*";
  // The environment file
  prior_files += " " + GetCuttlefishEnvPath();
  // The global link to the config file
  prior_files += " " + vsoc::GetGlobalConfigFileLink();
  LOG(INFO) << "Assuming prior files of " << prior_files;
  std::string lsof_cmd = "lsof -t " + prior_files + " >/dev/null 2>&1";
  int rval = std::system(lsof_cmd.c_str());
  // lsof returns 0 if any of the files are open
  if (WEXITSTATUS(rval) == 0) {
    LOG(ERROR) << "Clean aborted: files are in use";
    return false;
  }
  std::string clean_command = "rm -rf " + prior_files;
  rval = std::system(clean_command.c_str());
  if (WEXITSTATUS(rval) != 0) {
    LOG(ERROR) << "Remove of files failed";
    return false;
  }
  return true;
}

bool DecompressKernel(const std::string& src, const std::string& dst) {
  cvd::Command decomp_cmd(vsoc::DefaultHostArtifactsPath("bin/extract-vmlinux"));
  decomp_cmd.AddParameter(src);
  auto output_file = cvd::SharedFD::Creat(dst.c_str(), 0666);
  if (!output_file->IsOpen()) {
    LOG(ERROR) << "Unable to create decompressed image file: "
               << output_file->StrError();
    return false;
  }
  decomp_cmd.RedirectStdIO(cvd::Subprocess::StdIOChannel::kStdOut, output_file);
  auto decomp_proc = decomp_cmd.Start();
  return decomp_proc.Started() && decomp_proc.Wait() == 0;
}

void ValidateAdbModeFlag(const vsoc::CuttlefishConfig& config) {
  auto adb_modes = config.adb_mode();
  adb_modes.erase(vsoc::AdbMode::Unknown);
  if (adb_modes.size() < 1) {
    LOG(INFO) << "ADB not enabled";
  }
}

} // namespace

namespace {

std::vector<ImagePartition> disk_config() {
  std::vector<ImagePartition> partitions;
  partitions.push_back(ImagePartition {
    .label = "super",
    .image_file_path = FLAGS_super_image,
  });
  partitions.push_back(ImagePartition {
    .label = "userdata",
    .image_file_path = FLAGS_data_image,
  });
  partitions.push_back(ImagePartition {
    .label = "cache",
    .image_file_path = FLAGS_cache_image,
  });
  partitions.push_back(ImagePartition {
    .label = "metadata",
    .image_file_path = FLAGS_metadata_image,
  });
  partitions.push_back(ImagePartition {
    .label = "boot",
    .image_file_path = FLAGS_boot_image,
  });
  partitions.push_back(ImagePartition {
    .label = "misc",
    .image_file_path = FLAGS_misc_image
  });
  return partitions;
}

bool ShouldCreateCompositeDisk() {
  if (FLAGS_vm_manager == vm_manager::CrosvmManager::name()) {
    // The crosvm implementation is very fast to rebuild but also more brittle due to being split
    // into multiple files. The QEMU implementation is slow to build, but completely self-contained
    // at that point. Therefore, always rebuild on crosvm but check if it is necessary for QEMU.
    return true;
  }
  auto composite_age = cvd::FileModificationTime(FLAGS_composite_disk);
  for (auto& partition : disk_config()) {
    auto partition_age = cvd::FileModificationTime(partition.image_file_path);
    if (partition_age >= composite_age) {
      LOG(INFO) << "composite disk age was \"" << std::chrono::system_clock::to_time_t(composite_age) << "\", "
                << "partition age was \"" << std::chrono::system_clock::to_time_t(partition_age) << "\"";
      return true;
    }
  }
  return false;
}

bool ConcatRamdisks(const std::string& new_ramdisk_path, const std::string& ramdisk_a_path,
  const std::string& ramdisk_b_path) {
  // clear out file of any pre-existing content
  std::ofstream new_ramdisk(new_ramdisk_path, std::ios_base::binary | std::ios_base::trunc);
  std::ifstream ramdisk_a(ramdisk_a_path, std::ios_base::binary);
  std::ifstream ramdisk_b(ramdisk_b_path, std::ios_base::binary);

  if(!new_ramdisk.is_open() || !ramdisk_a.is_open() || !ramdisk_b.is_open()) {
    return false;
  }

  new_ramdisk << ramdisk_a.rdbuf() << ramdisk_b.rdbuf();
  return true;
}

off_t AvailableSpaceAtPath(const std::string& path) {
  struct statvfs vfs;
  if (statvfs(path.c_str(), &vfs) != 0) {
    int error_num = errno;
    LOG(ERROR) << "Could not find space available at " << path << ", error was "
               << strerror(error_num);
    return 0;
  }
  return vfs.f_bsize * vfs.f_bavail; // block size * free blocks for unprivileged users
}

off_t USERDATA_IMAGE_RESERVED = 4l * (1l << 30l); // 4 GiB
off_t AGGREGATE_IMAGE_RESERVED = 12l * (1l << 30l); // 12 GiB

bool CreateCompositeDisk(const vsoc::CuttlefishConfig& config) {
  if (FLAGS_composite_disk.empty()) {
    LOG(ERROR) << "asked to create composite disk, but path was empty";
    return false;
  }
  if (!cvd::SharedFD::Open(FLAGS_composite_disk.c_str(), O_WRONLY | O_CREAT, 0644)->IsOpen()) {
    LOG(ERROR) << "Could not ensure " << FLAGS_composite_disk << " exists";
    return false;
  }
  if (FLAGS_vm_manager == vm_manager::CrosvmManager::name()) {
    auto existing_size = cvd::FileSize(FLAGS_data_image);
    auto available_space = AvailableSpaceAtPath(FLAGS_data_image);
    if (available_space < USERDATA_IMAGE_RESERVED - existing_size) {
      // TODO(schuffelen): Duplicate this check in run_cvd when it can run on a separate machine
      LOG(ERROR) << "Not enough space in fs containing " << FLAGS_data_image;
      LOG(ERROR) << "Wanted " << (USERDATA_IMAGE_RESERVED - existing_size);
      LOG(ERROR) << "Got " << available_space;
      return false;
    }
    std::string header_path = config.AssemblyPath("gpt_header.img");
    std::string footer_path = config.AssemblyPath("gpt_footer.img");
    create_composite_disk(disk_config(), header_path, footer_path, FLAGS_composite_disk);
  } else {
    auto existing_size = cvd::FileSize(FLAGS_composite_disk);
    auto available_space = AvailableSpaceAtPath(FLAGS_composite_disk);
    if (available_space < AGGREGATE_IMAGE_RESERVED - existing_size) {
      LOG(ERROR) << "Not enough space to create " << FLAGS_composite_disk;
      LOG(ERROR) << "Wanted " << (AGGREGATE_IMAGE_RESERVED - existing_size);
      LOG(ERROR) << "Got " << available_space;
      return false;
    }
    aggregate_image(disk_config(), FLAGS_composite_disk);
  }
  return true;
}

} // namespace

const vsoc::CuttlefishConfig* InitFilesystemAndCreateConfig(
    int* argc, char*** argv, cvd::FetcherConfig fetcher_config) {
  if (!ParseCommandLineFlags(argc, argv)) {
    LOG(ERROR) << "Failed to parse command arguments";
    exit(AssemblerExitCodes::kArgumentParsingError);
  }

  // Clean up prior files before saving the config file (doing it after would
  // delete it)
  if (!CleanPriorFiles()) {
    LOG(ERROR) << "Failed to clean prior files";
    exit(AssemblerExitCodes::kPrioFilesCleanupError);
  }
  // Create assembly directory if it doesn't exist.
  if (!cvd::DirectoryExists(FLAGS_assembly_dir.c_str())) {
    LOG(INFO) << "Setting up " << FLAGS_assembly_dir;
    if (mkdir(FLAGS_assembly_dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0) {
      LOG(ERROR) << "Failed to create assembly directory: "
                 << FLAGS_assembly_dir << ". Error: " << errno;
      exit(AssemblerExitCodes::kAssemblyDirCreationError);
    }
  }
  // Create instance directory if it doesn't exist.
  if (!cvd::DirectoryExists(FLAGS_instance_dir.c_str())) {
    LOG(INFO) << "Setting up " << FLAGS_instance_dir;
    if (mkdir(FLAGS_instance_dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0) {
      LOG(ERROR) << "Failed to create instance directory: "
                 << FLAGS_instance_dir << ". Error: " << errno;
      exit(AssemblerExitCodes::kInstanceDirCreationError);
    }
  }

  auto internal_dir = FLAGS_instance_dir + "/" + vsoc::kInternalDirName;
  if (!cvd::DirectoryExists(internal_dir)) {
    if (mkdir(internal_dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) <
        0) {
      LOG(ERROR) << "Failed to create internal instance directory: "
                 << internal_dir << ". Error: " << errno;
      exit(AssemblerExitCodes::kInstanceDirCreationError);
    }
  }

  if (!cvd::FileHasContent(FLAGS_boot_image)) {
    LOG(ERROR) << "File not found: " << FLAGS_boot_image;
    exit(cvd::kCuttlefishConfigurationInitError);
  }

  if (!cvd::FileHasContent(FLAGS_vendor_boot_image)) {
    LOG(ERROR) << "File not found: " << FLAGS_vendor_boot_image;
    exit(cvd::kCuttlefishConfigurationInitError);
  }

  auto boot_img_unpacker =
    cvd::BootImageUnpacker::FromImages(FLAGS_boot_image,
                                       FLAGS_vendor_boot_image);

  if (!InitializeCuttlefishConfiguration(*boot_img_unpacker, fetcher_config)) {
    LOG(ERROR) << "Failed to initialize configuration";
    exit(AssemblerExitCodes::kCuttlefishConfigurationInitError);
  }
  // Do this early so that the config object is ready for anything that needs it
  auto config = vsoc::CuttlefishConfig::Get();
  if (!config) {
    LOG(ERROR) << "Failed to obtain config singleton";
    exit(AssemblerExitCodes::kCuttlefishConfigurationInitError);
  }

  if (!boot_img_unpacker->Unpack(config->ramdisk_image_path(),
                                 config->vendor_ramdisk_image_path(),
                                 config->use_unpacked_kernel()
                                     ? config->kernel_image_path()
                                     : "")) {
    LOG(ERROR) << "Failed to unpack boot image";
    exit(AssemblerExitCodes::kBootImageUnpackError);
  }

  // TODO(134522463) as part of the bootloader refactor, repack the vendor boot
  // image and use the bootloader to load both the boot and vendor ramdisk.
  // Until then, this hack to get gki modules into cuttlefish will suffice.

  // If a vendor ramdisk comes in via this mechanism, let it supercede the one
  // in the vendor boot image. This flag is what kernel presubmit testing uses
  // to pass in the kernel ramdisk.

  // If no kernel is passed in or an initramfs is made available, the default
  // vendor boot ramdisk or the initramfs provided should be appended to the
  // boot ramdisk. If a kernel IS provided with no initramfs, it is safe to
  // safe to assume that the kernel was built with no modules and expects no
  // modules for cf to run properly.
  std::string discovered_kernel = fetcher_config.FindCvdFileWithSuffix(kKernelDefaultPath);
  std::string foreign_kernel = FLAGS_kernel_path.size() ? FLAGS_kernel_path : discovered_kernel;
  std::string discovered_ramdisk = fetcher_config.FindCvdFileWithSuffix(kInitramfsImg);
  std::string foreign_ramdisk = FLAGS_initramfs_path.size () ? FLAGS_initramfs_path : discovered_ramdisk;
  if(!foreign_kernel.size() || foreign_ramdisk.size()) {
    const std::string& vendor_ramdisk_path =
      config->initramfs_path().size() ? config->initramfs_path()
                                      : config->vendor_ramdisk_image_path();
    if(!ConcatRamdisks(config->final_ramdisk_path(),
                       config->ramdisk_image_path(), vendor_ramdisk_path)) {
      LOG(ERROR) << "Failed to concatenate ramdisk and vendor ramdisk";
      exit(AssemblerExitCodes::kInitRamFsConcatError);
    }
  }

  if (config->decompress_kernel()) {
    if (!DecompressKernel(config->kernel_image_path(),
        config->decompressed_kernel_image_path())) {
      LOG(ERROR) << "Failed to decompress kernel";
      exit(AssemblerExitCodes::kKernelDecompressError);
    }
  }

  ValidateAdbModeFlag(*config);

  // Create misc if necessary
  if (!InitializeMiscImage(FLAGS_misc_image)) {
    exit(cvd::kCuttlefishConfigurationInitError);
  }

  // Create data if necessary
  if (!ApplyDataImagePolicy(*config, FLAGS_data_image)) {
    exit(cvd::kCuttlefishConfigurationInitError);
  }

  if (!cvd::FileExists(FLAGS_metadata_image)) {
    CreateBlankImage(FLAGS_metadata_image, FLAGS_blank_metadata_image_mb, "none");
  }

  if (!cvd::FileExists(config->ForDefaultInstance().access_kregistry_path())) {
    CreateBlankImage(config->ForDefaultInstance().access_kregistry_path(), 1,
                     "none", "64K");
  }

  if (SuperImageNeedsRebuilding(fetcher_config, *config)) {
    if (!RebuildSuperImage(fetcher_config, *config, FLAGS_super_image)) {
      LOG(ERROR) << "Super image rebuilding requested but could not be completed.";
      exit(cvd::kCuttlefishConfigurationInitError);
    }
  }

  if (ShouldCreateCompositeDisk()) {
    if (!CreateCompositeDisk(*config)) {
      exit(cvd::kDiskSpaceError);
    }
  }

  // Check that the files exist
  for (const auto& file : config->virtual_disk_paths()) {
    if (!file.empty() && !cvd::FileHasContent(file.c_str())) {
      LOG(ERROR) << "File not found: " << file;
      exit(cvd::kCuttlefishConfigurationInitError);
    }
  }

  return config;
}

std::string GetConfigFilePath(const vsoc::CuttlefishConfig& config) {
  return config.AssemblyPath("cuttlefish_config.json");
}
