//
//
//
//@author : Leo
//@date : 2020/10/12 18:38
//@since : lightingxin@qq.com
//@desc : 
//
//
#include <android/log.h>

#ifndef NDKSTUDY_FATHER_H
#define NDKSTUDY_FATHER_H

#define LOGI(format, ...) __android_log_print(ANDROID_LOG_INFO, "Leo", format, ##__VA_ARGS__);

class Father{
public:
    virtual void eat();

    virtual void running();
};

#endif //NDKSTUDY_FATHER_H
