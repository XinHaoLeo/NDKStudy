#include <jni.h>
#include <string>
//#include <android/log.h>
#include <iostream>
#include "Father.h"
#include "Son.h"
#include "Daughter.h"

using namespace std;

//#define LOGI(format, ...) __android_log_print(ANDROID_LOG_INFO, "Leo", format, ##__VA_ARGS__);
//#define LOGE(format, ...) __android_log_pring(ANDROID_LOG_ERROR,"Leo",format,##__VA_ARGS__);

void test(jint type);

extern "C"
JNIEXPORT jstring JNICALL
Java_com_xin_ndkstudy_util_TestUtils_contentAppend(JNIEnv *env, jobject thiz, jstring jstr, jint type) {

    const char *content = env->GetStringUTFChars(jstr, JNI_FALSE);
    char *contents = const_cast<char *>(content);
//    char text[] = "I love C Plus Plus";
    string text = "I love C Plus Plus\n";
    text.append(contents);
    test(type);
    jstring appendContent = env->NewStringUTF(text.c_str());
    env->ReleaseStringUTFChars(jstr, content);
    LOGI("content = %s", text.c_str())
    return appendContent;
}

void poly(Father &father) {
    father.eat();
    father.running();
}

void test(jint type) {
    Father father;
    Son son;
    Daughter daughter;
    switch (type) {
        case 0:
            poly(father);
            break;
        case 1:
            poly(son);
            break;
        case 2:
            poly(daughter);
            break;
        default:
            break;
    }
}
