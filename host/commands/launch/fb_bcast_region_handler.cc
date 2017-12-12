/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include <glog/logging.h>
#include <gflags/gflags.h>
#include "common/vsoc/framebuffer/fb_bcast_region_view.h"

// TODO(jemoreira): Change these to something sensible once everything is
// ported.
DEFINE_int32(x_res, 1280, "Width of the screen in pixels");
DEFINE_int32(y_res, 720, "Height of the screen in pixels");
DEFINE_int32(dpi, 160, "Pixels per inch for the screen");

void IniitializeFBBroadcastRegion(const char* domain) {
  vsoc::framebuffer::FBBroadcastRegionView region;
  if (!region.Open(domain)) {
    LOG(INFO) << "Framebuffer region was not found";
    return;
  }
  auto dest = region.data();
  dest->x_res = FLAGS_x_res;
  dest->y_res = FLAGS_y_res;
  dest->dpi = FLAGS_dpi;
}
