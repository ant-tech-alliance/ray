/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_alipay_streaming_runtime_queue_impl_plasma_QueueProducerImpl */

#ifndef _Included_com_alipay_streaming_runtime_queue_impl_plasma_QueueProducerImpl
#define _Included_com_alipay_streaming_runtime_queue_impl_plasma_QueueProducerImpl
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_alipay_streaming_runtime_queue_impl_plasma_QueueProducerImpl
 * Method:    getOutputSeqIdNative
 * Signature: (J[[B)[J
 */
JNIEXPORT jlongArray JNICALL Java_com_alipay_streaming_runtime_queue_impl_plasma_QueueProducerImpl_getOutputSeqIdNative
  (JNIEnv *, jobject, jlong, jobjectArray);

/*
 * Class:     com_alipay_streaming_runtime_queue_impl_plasma_QueueProducerImpl
 * Method:    getBackPressureRatioNative
 * Signature: (J[[B)[D
 */
JNIEXPORT jdoubleArray JNICALL Java_com_alipay_streaming_runtime_queue_impl_plasma_QueueProducerImpl_getBackPressureRatioNative
  (JNIEnv *, jobject, jlong, jobjectArray);

/*
 * Class:     com_alipay_streaming_runtime_queue_impl_plasma_QueueProducerImpl
 * Method:    broadcastBarrierNative
 * Signature: (JJJ[B)V
 */
JNIEXPORT void JNICALL Java_com_alipay_streaming_runtime_queue_impl_plasma_QueueProducerImpl_broadcastBarrierNative
  (JNIEnv *, jobject, jlong, jlong, jlong, jbyteArray);

/*
 * Class:     com_alipay_streaming_runtime_queue_impl_plasma_QueueProducerImpl
 * Method:    getBufferNative
 * Signature: (JJJJ)V
 */
JNIEXPORT void JNICALL Java_com_alipay_streaming_runtime_queue_impl_plasma_QueueProducerImpl_getBufferNative
  (JNIEnv *, jobject, jlong, jlong, jlong, jlong);

/*
 * Class:     com_alipay_streaming_runtime_queue_impl_plasma_QueueProducerImpl
 * Method:    writeMessageNative
 * Signature: (JJJI)J
 */
JNIEXPORT jlong JNICALL Java_com_alipay_streaming_runtime_queue_impl_plasma_QueueProducerImpl_writeMessageNative
  (JNIEnv *, jobject, jlong, jlong, jlong, jint);

/*
 * Class:     com_alipay_streaming_runtime_queue_impl_plasma_QueueProducerImpl
 * Method:    stopProducerNative
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_alipay_streaming_runtime_queue_impl_plasma_QueueProducerImpl_stopProducerNative
  (JNIEnv *, jobject, jlong);

/*
 * Class:     com_alipay_streaming_runtime_queue_impl_plasma_QueueProducerImpl
 * Method:    closeProducerNative
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_alipay_streaming_runtime_queue_impl_plasma_QueueProducerImpl_closeProducerNative
  (JNIEnv *, jobject, jlong);

/*
 * Class:     com_alipay_streaming_runtime_queue_impl_plasma_QueueProducerImpl
 * Method:    clearCheckpointNative
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_com_alipay_streaming_runtime_queue_impl_plasma_QueueProducerImpl_clearCheckpointNative
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     com_alipay_streaming_runtime_queue_impl_plasma_QueueProducerImpl
 * Method:    hotUpdateProducerNative
 * Signature: (J[[B)V
 */
JNIEXPORT void JNICALL Java_com_alipay_streaming_runtime_queue_impl_plasma_QueueProducerImpl_hotUpdateProducerNative
  (JNIEnv *, jobject, jlong, jobjectArray);

/*
 * Class:     com_alipay_streaming_runtime_queue_impl_plasma_QueueProducerImpl
 * Method:    broadcastPartialBarrierNative
 * Signature: (JJJ[B)V
 */
JNIEXPORT void JNICALL Java_com_alipay_streaming_runtime_queue_impl_plasma_QueueProducerImpl_broadcastPartialBarrierNative
  (JNIEnv *, jobject, jlong, jlong, jlong, jbyteArray);

/*
 * Class:     com_alipay_streaming_runtime_queue_impl_plasma_QueueProducerImpl
 * Method:    clearPartialCheckpointNative
 * Signature: (JJJ)V
 */
JNIEXPORT void JNICALL Java_com_alipay_streaming_runtime_queue_impl_plasma_QueueProducerImpl_clearPartialCheckpointNative
  (JNIEnv *, jobject, jlong, jlong, jlong);

#ifdef __cplusplus
}
#endif
#endif
