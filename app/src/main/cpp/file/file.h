//
//
//
//@author : Leo
//@date : 2020/10/13 10:14
//@since : lightingxin@qq.com
//@desc :
//
//

#include <jni.h>
#include <android/log.h>

#ifndef NDKSTUDY_FILE_H
#define NDKSTUDY_FILE_H

#define LOGI(format, ...) __android_log_print(ANDROID_LOG_INFO, "Leo", format, ##__VA_ARGS__);


//extern "C" {
//JNIEXPORT void JNICALL
//Java_com_xin_ndkstudy_FileUtil_encrypt(JNIEnv *, jobject, jstring, jstring);
//
//JNIEXPORT void JNICALL
//Java_com_xin_ndkstudy_FileUtil_decrypt(JNIEnv *, jobject, jstring, jstring);
//}

void fileOperations(JNIEnv *, jobject, const char *, const char *);

void fileResult(JNIEnv *, jobject, int, jstring);

jstring charToJstring(JNIEnv *, const char *);

#endif //NDKSTUDY_FILE_H
