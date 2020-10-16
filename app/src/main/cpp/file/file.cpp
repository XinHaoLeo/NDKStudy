//
//
//
//@author : Leo
//@date : 2020/10/13 10:14
//@since : lightingxin@qq.com
//@desc : 
//
//

#include "file.h"
//#include <jni.h>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <iosfwd>
#include <iostream>

const int ERROR = 0;
const int SUCCESS = 1;
const char *PASSWORD = "coolXin";

//jobject globalRef;

//JNIEXPORT void JNI_OnUnload(JavaVM *vm, void *reserved) {
//
//}

extern "C" {
JNIEXPORT void JNICALL
Java_com_xin_ndkstudy_FileUtils_encrypt(JNIEnv *env, jobject obj, jstring jNormalPath,
                                        jstring jEncryptPath) {
    LOGI("%s", "encrypt执行了")
//    globalRef = env->NewGlobalRef(obj);
    const char *normalPath = env->GetStringUTFChars(jNormalPath, JNI_FALSE);
    const char *encryptPath = env->GetStringUTFChars(jEncryptPath, JNI_FALSE);
    fileOperations(env, obj, normalPath, encryptPath);
    env->ReleaseStringUTFChars(jNormalPath, normalPath);
    env->ReleaseStringUTFChars(jEncryptPath, encryptPath);
}

JNIEXPORT void JNICALL
Java_com_xin_ndkstudy_FileUtils_decrypt(JNIEnv *env, jobject obj, jstring jEncryptPath,
                                        jstring jDecryptPath) {
    LOGI("%s", "decrypt执行了")
    const char *encryptPath = env->GetStringUTFChars(jEncryptPath, JNI_FALSE);
    const char *decryptPath = env->GetStringUTFChars(jDecryptPath, JNI_FALSE);
    fileOperations(env, obj, encryptPath, decryptPath);
    env->ReleaseStringUTFChars(jEncryptPath, encryptPath);
    env->ReleaseStringUTFChars(jDecryptPath, decryptPath);
}

}

void fileOperations(JNIEnv *env, jobject obj, const char *pReadPath, const char *pWritePath) {
    if (pReadPath == nullptr || pWritePath == nullptr) {
        const char *msg = "文件地址为空";
        fileResult(env, obj, ERROR, charToJstring(env, msg));
        return;
    }
    FILE *pReadFile = fopen(pReadPath, "rb");
    FILE *pWriteFile = fopen(pWritePath, "wb");
    if (pReadFile == nullptr || pWriteFile == nullptr) {
        const char *msg = "文件打开失败";
        fileResult(env, obj, ERROR, charToJstring(env, msg));
        return;
    }
    int i = 0,ch;
    size_t len = strlen(PASSWORD);
    while ((ch = fgetc(pReadFile)) != EOF) {
        fputc(ch ^ PASSWORD[i % len], pWriteFile);
        i++;
    }
    const char *msg = "执行成功";
    fileResult(env, obj, SUCCESS, charToJstring(env, msg));
    fclose(pWriteFile);
    fclose(pReadFile);
}

//有中文的需要指定编码格式,如全是英文直接调用 env->NewStringUTF(msg);返回给java
jstring charToJstring(JNIEnv *env, const char *msg) {
    jclass str_cls = env->FindClass("java/lang/String");
    jmethodID constructor_mid = env->GetMethodID(str_cls, "<init>", "([BLjava/lang/String;)V");
    jbyteArray bytes = env->NewByteArray(strlen(msg));
    env->SetByteArrayRegion(bytes, 0, strlen(msg), (jbyte *) msg);
    jstring charsetName = env->NewStringUTF("UTF-8");
    return (jstring) env->NewObject(str_cls, constructor_mid, bytes, charsetName);
}

void fileResult(JNIEnv *env, jobject obj, int code, jstring msg) {
    //不能直接点击fileResult跳转到java文件 优点(可供别人使用)
    jclass jcls = env->GetObjectClass(obj);
    //可直接点击fileResult跳转到java文件 缺点(包名不可更改,不能提供给别人用)
//    jclass jcls = env->FindClass("com/xin/ndkstudy/FileUtils");

//    jclass jcls = env->GetObjectClass(globalRef);
    jmethodID methodId = env->GetMethodID(jcls, "fileResult", "(ILjava/lang/String;)V");
    env->CallVoidMethod(obj, methodId, code, msg);
}