/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class org_ray_runtime_task_NativeTaskSubmitter */

#ifndef _Included_org_ray_runtime_task_NativeTaskSubmitter
#define _Included_org_ray_runtime_task_NativeTaskSubmitter
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     org_ray_runtime_task_NativeTaskSubmitter
 * Method:    nativeSubmitTask
 * Signature:
 * (JLorg/ray/runtime/functionmanager/FunctionDescriptor;Ljava/util/List;ILorg/ray/api/options/CallOptions;)Ljava/util/List;
 */
JNIEXPORT jobject JNICALL Java_org_ray_runtime_task_NativeTaskSubmitter_nativeSubmitTask(
    JNIEnv *, jclass, jlong, jobject, jobject, jint, jobject);

/*
 * Class:     org_ray_runtime_task_NativeTaskSubmitter
 * Method:    nativeCreateActor
 * Signature:
 * (JLorg/ray/runtime/functionmanager/FunctionDescriptor;Ljava/util/List;Lorg/ray/api/options/ActorCreationOptions;)J
 */
JNIEXPORT jbyteArray JNICALL Java_org_ray_runtime_task_NativeTaskSubmitter_nativeCreateActor(
    JNIEnv *, jclass, jlong, jobject, jobject, jobject);

/*
 * Class:     org_ray_runtime_task_NativeTaskSubmitter
 * Method:    nativeSubmitActorTask
 * Signature:
 * (JJLorg/ray/runtime/functionmanager/FunctionDescriptor;Ljava/util/List;ILorg/ray/api/options/CallOptions;)Ljava/util/List;
 */
JNIEXPORT jobject JNICALL
Java_org_ray_runtime_task_NativeTaskSubmitter_nativeSubmitActorTask(JNIEnv *, jclass,
                                                                    jlong, jbyteArray, jobject,
                                                                    jobject, jint,
                                                                    jobject);

#ifdef __cplusplus
}
#endif
#endif
