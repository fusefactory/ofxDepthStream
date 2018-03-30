// This file is part of the Orbbec Astra SDK [https://orbbec3d.com]
// Copyright (c) 2015-2017 Orbbec 3D
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Be excellent to each other.
#ifndef BODY_CAPI_H
#define BODY_CAPI_H

#include <astra_core/capi/astra_defines.h>
#include <astra_core/capi/astra_types.h>
#include "body_types.h"

ASTRA_BEGIN_DECLS

ASTRA_API_EX astra_status_t astra_reader_get_bodystream(astra_reader_t reader,
                                                        astra_bodystream_t* bodyStream);

ASTRA_API_EX astra_status_t astra_bodystream_get_body_features(astra_bodystream_t bodyStream,
                                                               astra_body_id_t id,
                                                               astra_body_tracking_feature_flags_t* features);

ASTRA_API_EX astra_status_t astra_bodystream_set_body_features(astra_bodystream_t bodyStream,
                                                               astra_body_id_t id,
                                                               astra_body_tracking_feature_flags_t features);

ASTRA_API_EX astra_status_t astra_bodystream_get_default_body_features(astra_bodystream_t bodyStream,
                                                                       astra_body_tracking_feature_flags_t* features);

ASTRA_API_EX astra_status_t astra_bodystream_set_default_body_features(astra_bodystream_t bodyStream,
                                                                       astra_body_tracking_feature_flags_t features);

ASTRA_API_EX astra_status_t astra_frame_get_bodyframe(astra_reader_frame_t readerFrame,
                                                      astra_bodyframe_t* bodyFrame);

ASTRA_API_EX astra_status_t astra_bodyframe_get_frameindex(astra_bodyframe_t bodyFrame,
                                                           astra_frame_index_t* index);

ASTRA_API_EX astra_status_t astra_bodyframe_info(astra_bodyframe_t bodyFrame,
                                                 astra_bodyframe_info_t* info);

ASTRA_API_EX astra_status_t astra_bodyframe_bodymask(astra_bodyframe_t bodyFrame,
                                                     astra_bodymask_t* bodyMask);

// Internal use only
ASTRA_API_EX astra_status_t astra_bodyframe_bodymask_ptr(astra_bodyframe_t bodyFrame,
                                                         uint32_t* width,
                                                         uint32_t* height,
                                                         void** dataPtr);

ASTRA_API_EX astra_status_t astra_bodyframe_copy_bodymask_data(astra_bodyframe_t bodyFrame,
                                                               void* data);

ASTRA_API_EX astra_status_t astra_bodyframe_floor_info(astra_bodyframe_t bodyFrame,
                                                       astra_floor_info_t* floorInfo);

ASTRA_API_EX astra_status_t astra_bodyframe_floormask_ptr(astra_bodyframe_t bodyFrame,
                                                          uint32_t* width,
                                                          uint32_t* height,
                                                          void** dataPtr);

ASTRA_API_EX astra_status_t astra_bodyframe_copy_floormask_data(astra_bodyframe_t bodyFrame,
                                                                void* data);

// Internal use only
ASTRA_API_EX astra_status_t astra_bodyframe_floor_info_ptr(astra_bodyframe_t bodyFrame,
                                                           astra_floor_info_t** floorInfo);

ASTRA_API_EX astra_status_t astra_bodyframe_body_list(astra_bodyframe_t bodyFrame,
                                                      astra_body_list_t* bodyList);

ASTRA_API_EX astra_status_t orbbec_body_tracking_set_license(const char* licenseString);

ASTRA_END_DECLS

#endif // BODY_CAPI_H
