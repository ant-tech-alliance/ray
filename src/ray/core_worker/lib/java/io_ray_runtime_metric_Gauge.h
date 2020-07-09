// Copyright 2017 The Ray Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class io_ray_runtime_metric_Gauge */

#ifndef _Included_io_ray_runtime_metric_Gauge
#define _Included_io_ray_runtime_metric_Gauge
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     io_ray_runtime_metric_Gauge
 * Method:    registerGaugeNative
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/util/List;)J
 */
JNIEXPORT jlong JNICALL Java_io_ray_runtime_metric_Gauge_registerGaugeNative(
    JNIEnv *, jobject, jstring, jstring, jstring, jobject);

/*
 * Class:     io_ray_runtime_metric_Gauge
 * Method:    unregisterGauge
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_io_ray_runtime_metric_Gauge_unregisterGauge(JNIEnv *, jobject,
                                                                        jlong);

/*
 * Class:     io_ray_runtime_metric_Gauge
 * Method:    recordNative
 * Signature: (JDLjava/util/List;Ljava/util/List;)V
 */
JNIEXPORT void JNICALL Java_io_ray_runtime_metric_Gauge_recordNative(JNIEnv *, jobject,
                                                                     jlong, jdouble,
                                                                     jobject, jobject);

#ifdef __cplusplus
}
#endif
#endif
