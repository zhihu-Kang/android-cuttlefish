/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "host/commands/cvd/selector/selector_constants.h"

#include <unistd.h>

#include <deque>

#include "common/libs/fs/shared_buf.h"
#include "common/libs/fs/shared_fd.h"
#include "common/libs/utils/environment.h"
#include "common/libs/utils/files.h"
#include "common/libs/utils/users.h"

namespace cuttlefish {
namespace selector {

static bool CheckReadWritePermit(const std::string& dir) {
  if (dir.empty()) {
    return false;
  }
  std::string file("_placeholder_._134_");
  std::string full_path =
      (*dir.rbegin() == '/') ? (dir + file) : (dir + "/" + file);
  // this file is required only to make FileExists() true.
  SharedFD new_file = SharedFD::Creat(full_path, S_IRUSR | S_IWUSR);
  if (!new_file->IsOpen()) {
    return false;
  }
  return RemoveFile(full_path);
}

Result<std::string> ParentOfAutogeneratedHomes(const uid_t uid) {
  std::deque<std::string> try_dirs = {
      StringFromEnv("TMPDIR", ""),
      StringFromEnv("TEMP", ""),
      StringFromEnv("TMP", ""),
      "/tmp",
      "/var/tmp",
      "/usr/tmp",
  };

  auto system_wide_home = SystemWideUserHome(uid);
  if (system_wide_home.ok()) {
    try_dirs.emplace_back(*system_wide_home);
  }
  try_dirs.emplace_back(AbsolutePath("."));
  while (!try_dirs.empty()) {
    const auto candidate = std::move(try_dirs.front());
    try_dirs.pop_front();
    if (candidate.empty() || !EnsureDirectoryExists(candidate).ok()) {
      continue;
    }
    if (CheckReadWritePermit(candidate)) {
      return AbsolutePath(candidate);
    }
  }
  return CF_ERR("Tried all candidate directories but none was read-writable.");
}

}  // namespace selector
}  // namespace cuttlefish