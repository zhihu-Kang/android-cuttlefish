project('cvd', 'cpp', default_options: ['cpp_std=gnu++17'])

sources = [
  'android-base/file.cpp',
  'android-base/parsebool.cpp',
  'android-base/posix_strerror_r.cpp',
  'android-base/stringprintf.cpp',
  'android-base/strings.cpp',
  'android-base/threads.cpp',
  'android-base/logging.cpp',
  'common/libs/fs/shared_buf.cc',
  'common/libs/fs/shared_fd.cpp',
  'common/libs/fs/shared_fd_stream.cpp',
  'common/libs/fs/epoll.cpp',
  'common/libs/utils/archive.cpp',
  'common/libs/utils/base64.cpp',
  'common/libs/utils/environment.cpp',
  'common/libs/utils/files.cpp',
  'common/libs/utils/flag_parser.cpp',
  'common/libs/utils/flags_validator.cpp',
  'common/libs/utils/json.cpp',
  'common/libs/utils/network.cpp',
  'common/libs/utils/proc_file_utils.cpp',
  'common/libs/utils/shared_fd_flag.cpp',
  'common/libs/utils/socket2socket_proxy.cpp',
  'common/libs/utils/tcp_socket.cpp',
  'common/libs/utils/tee_logging.cpp',
  'common/libs/utils/unix_sockets.cpp',
  'common/libs/utils/users.cpp',
  'common/libs/utils/vsock_connection.cpp',
  'common/libs/utils/inotify.cpp',
  'common/libs/utils/subprocess.cpp',
  'host/commands/cvd/acloud/config.cpp',
  'host/commands/cvd/acloud/converter.cpp',
  'host/commands/cvd/acloud/create_converter_parser.cpp',
  'host/commands/cvd/build_api.cpp',
  'host/commands/cvd/client.cpp',
  'host/commands/cvd/command_sequence.cpp',
  'host/commands/cvd/common_utils.cpp',
  'host/commands/cvd/demo_multi_vd.cpp',
  'host/commands/cvd/driver_flags.cpp',
  'host/commands/cvd/epoll_loop.cpp',
  'host/commands/cvd/fetch/fetch_cvd.cc',
  'host/commands/cvd/frontline_parser.cpp',
  'host/commands/cvd/handle_reset.cpp',
  'host/commands/cvd/instance_lock.cpp',
  'host/commands/cvd/instance_manager.cpp',
  'host/commands/cvd/lock_file.cpp',
  'host/commands/cvd/logger.cpp',
  'host/commands/cvd/parser/cf_configs_common.cpp',
  'host/commands/cvd/parser/cf_configs_instances.cpp',
  'host/commands/cvd/parser/cf_flags_validator.cpp',
  'host/commands/cvd/parser/cf_metrics_configs.cpp',
  'host/commands/cvd/parser/fetch_cvd_parser.cpp',
  'host/commands/cvd/parser/instance/cf_boot_configs.cpp',
  'host/commands/cvd/parser/instance/cf_disk_configs.cpp',
  'host/commands/cvd/parser/instance/cf_graphics_configs.cpp',
  'host/commands/cvd/parser/instance/cf_security_configs.cpp',
  'host/commands/cvd/parser/instance/cf_vm_configs.cpp',
  'host/commands/cvd/parser/launch_cvd_parser.cpp',
  'host/commands/cvd/parser/launch_cvd_templates.cpp',
  'host/commands/cvd/parser/load_configs_parser.cpp',
  'host/commands/cvd/parser/selector_parser.cpp',
  'host/commands/cvd/reset_client_utils.cpp',
  'host/commands/cvd/run_server.cpp',
  'host/commands/cvd/selector/arguments_lexer.cpp',
  'host/commands/cvd/selector/arguments_separator.cpp',
  'host/commands/cvd/selector/creation_analyzer.cpp',
  'host/commands/cvd/selector/device_selector_utils.cpp',
  'host/commands/cvd/selector/group_selector.cpp',
  'host/commands/cvd/selector/instance_database.cpp',
  'host/commands/cvd/selector/instance_database_impl.cpp',
  'host/commands/cvd/selector/instance_database_types.cpp',
  'host/commands/cvd/selector/instance_database_utils.cpp',
  'host/commands/cvd/selector/instance_group_record.cpp',
  'host/commands/cvd/selector/instance_record.cpp',
  'host/commands/cvd/selector/instance_selector.cpp',
  'host/commands/cvd/selector/selector_common_parser.cpp',
  'host/commands/cvd/selector/selector_constants.cpp',
  'host/commands/cvd/selector/selector_option_parser_utils.cpp',
  'host/commands/cvd/selector/start_selector_parser.cpp',
  'host/commands/cvd/server.cc',
  'host/commands/cvd/server_client.cpp',
  'host/commands/cvd/server_command/acloud.cpp',
  'host/commands/cvd/server_command/acloud_command.cpp',
  'host/commands/cvd/server_command/acloud_common.cpp',
  'host/commands/cvd/server_command/acloud_mixsuperimage.cpp',
  'host/commands/cvd/server_command/acloud_translator.cpp',
  'host/commands/cvd/server_command/cmd_list.cpp',
  'host/commands/cvd/server_command/components.cpp',
  'host/commands/cvd/server_command/display.cpp',
  'host/commands/cvd/server_command/env.cpp',
  'host/commands/cvd/server_command/fetch.cpp',
  'host/commands/cvd/server_command/flags_collector.cpp',
  'host/commands/cvd/server_command/fleet.cpp',
  'host/commands/cvd/server_command/generic.cpp',
  'host/commands/cvd/server_command/handler_proxy.cpp',
  'host/commands/cvd/server_command/help.cpp',
  'host/commands/cvd/server_command/host_tool_target.cpp',
  'host/commands/cvd/server_command/host_tool_target_manager.cpp',
  'host/commands/cvd/server_command/load_configs.cpp',
  'host/commands/cvd/server_command/power.cpp',
  'host/commands/cvd/server_command/reset.cpp',
  'host/commands/cvd/server_command/restart.cpp',
  'host/commands/cvd/server_command/serial_launch.cpp',
  'host/commands/cvd/server_command/serial_preset.cpp',
  'host/commands/cvd/server_command/shutdown.cpp',
  'host/commands/cvd/server_command/snapshot.cpp',
  'host/commands/cvd/server_command/start.cpp',
  'host/commands/cvd/server_command/start_impl.cpp',
  'host/commands/cvd/server_command/subprocess_waiter.cpp',
  'host/commands/cvd/server_command/try_acloud.cpp',
  'host/commands/cvd/server_command/utils.cpp',
  'host/commands/cvd/server_command/version.cpp',
  'host/commands/cvd/server_constants.cpp',
  'host/commands/cvd/types.cpp',
  'host/commands/cvd/flag.cpp',
  'host/libs/allocd/alloc_utils.cpp',
  'host/libs/allocd/resource.cpp',
  'host/libs/allocd/resource_manager.cpp',
  'host/libs/allocd/utils.cpp',
  'host/libs/config/cuttlefish_config.cpp',
  'host/libs/config/fetcher_config.cpp',
  'host/libs/config/instance_nums.cpp',
  'host/libs/config/cuttlefish_config_environment.cpp',
  'host/libs/config/cuttlefish_config_instance.cpp',
  'host/libs/config/host_tools_version.cpp',
  'host/libs/web/build_api.cc',
  'host/libs/web/credential_source.cc',
  'host/libs/web/http_client/http_client.cc',
  'host/libs/web/http_client/http_client_util.cc',
  'host/libs/web/http_client/sso_client.cc',
]

fruit_include = include_directories('/usr/local/include/')
cc = meson.get_compiler('cpp')
fruit_lib = cc.find_library('fruit', dirs: ['/usr/local/lib'])
boringssl_crypto_lib = cc.find_library('crypto', dirs: ['/usr/lib/x86_64-linux-gnu/android'])
boringssl_ssl_lib = cc.find_library('ssl', dirs: ['/usr/lib/x86_64-linux-gnu/android'])

protoc = find_program('protoc', required : true)
deps = dependency('protobuf', required : true)
protoc_generator = generator(protoc,
  output    : ['@BASENAME@.pb.cc', '@BASENAME@.pb.h'],
  arguments : ['--proto_path=@CURRENT_SOURCE_DIR@', '--cpp_out=@BUILD_DIR@', '@INPUT@'])

server = protoc_generator.process('cvd_server.proto')
internal_config = protoc_generator.process('internal_config.proto')
launch_cvd_proto = protoc_generator.process('launch_cvd.proto')
user_config = protoc_generator.process('user_config.proto')

git_version_h = vcs_tag(
  command : ['git', 'describe'],
  fallback : 'unknown',
  input : 'build_version.h.in',
  output :'version.h',
)

executable(
  'cvd',
  cpp_args: ['-Wno-reorder', '-Wno-unknown-pragmas', '-Wno-attributes', '-Wno-sign-compare', '-Wno-write-strings'],
  sources: ['host/commands/cvd/main.cc'] + sources + server + launch_cvd_proto + internal_config + user_config + git_version_h,
  dependencies: [
    boringssl_crypto_lib,
    boringssl_ssl_lib,
    dependency('fmt'),
    dependency('gflags'),
    dependency('jsoncpp'),
    dependency('libcurl'),
    dependency('libglog'),
    dependency('libxml-2.0'),
    dependency('protobuf'),
    fruit_lib,
  ],
  include_directories: [
    '/usr/include/android/',
    '/usr/local/include',
  ],
)
executable(
  'allocd',
  cpp_args: ['-Wno-reorder', '-Wno-unknown-pragmas', '-Wno-attributes', '-Wno-sign-compare', '-Wno-write-strings'],
  sources: ['host/libs/allocd/allocd.cpp'] + sources + server + launch_cvd_proto + internal_config + user_config + git_version_h,
  dependencies: [
    boringssl_crypto_lib,
    boringssl_ssl_lib,
    dependency('fmt'),
    dependency('gflags'),
    dependency('jsoncpp'),
    dependency('libcurl'),
    dependency('libglog'),
    dependency('libxml-2.0'),
    dependency('protobuf'),
    fruit_lib,
  ],
  include_directories: [
    '/usr/include/android/',
    '/usr/local/include',
  ],
)