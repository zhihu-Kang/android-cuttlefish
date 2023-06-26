/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include "host/commands/cvd/parser/fetch_cvd_parser.h"

#include <string>
#include <vector>

#include <json/json.h>

#include "host/commands/cvd/parser/cf_configs_common.h"

namespace cuttlefish {
namespace {

#define EMPTY_CREDENTIAL ""
#define EMPTY_DEFAULT_BUILD ""
#define EMPTY_SYSTEM_BUILD ""
#define EMPTY_KERNEL_BUILD ""

void InitFetchInstanceConfigs(Json::Value& instances) {
  // Handle common flags
  InitStringConfig(instances, "disk", "default_build", EMPTY_DEFAULT_BUILD);
  InitStringConfig(instances, "disk", "system_build", EMPTY_SYSTEM_BUILD);
  InitStringConfig(instances, "disk", "kernel_build", EMPTY_KERNEL_BUILD);
}

void InitFetchCvdConfigs(Json::Value& root) {
  if (!root.isMember("credential_source")) {
    root["credential_source"] = EMPTY_CREDENTIAL;
  }
  InitFetchInstanceConfigs(root["instances"]);
}

FetchCvdInstanceConfig ParseFetchInstanceConfigs(const Json::Value& instance) {
  FetchCvdInstanceConfig result;
  result.default_build = instance["disk"]["default_build"].asString();
  result.system_build = instance["disk"]["system_build"].asString();
  result.kernel_build = instance["disk"]["kernel_build"].asString();
  if (result.default_build != EMPTY_DEFAULT_BUILD ||
      result.system_build != EMPTY_SYSTEM_BUILD ||
      result.kernel_build != EMPTY_KERNEL_BUILD) {
    result.should_fetch = true;
  } else {
    result.should_fetch = false;
  }

  return result;
}

FetchCvdConfig GenerateFetchCvdFlags(const Json::Value& root) {
  FetchCvdConfig result;
  result.credential_source = root["credential_source"].asString();
  int num_instances = root["instances"].size();
  for (unsigned int i = 0; i < num_instances; i++) {
    auto instance_config = ParseFetchInstanceConfigs(root["instances"][i]);
    result.instances.emplace_back(instance_config);
  }

  return result;
}

}  // namespace

FetchCvdConfig ParseFetchCvdConfigs(Json::Value& root) {
  InitFetchCvdConfigs(root);
  return GenerateFetchCvdFlags(root);
}

}  // namespace cuttlefish