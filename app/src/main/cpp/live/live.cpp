#include <jni.h>

//
//
//
//@author : Leo
//@date : 2020/12/2 11:50
//@desc : 
//
//
#include <jni.h>
#include <cstdlib>
#include <cstring>
#include <android/log.h>
#include <pthread.h>
#include "queue.h"
#include <x264/x264.h>
#include <rtmp/rtmp.h>
#include <faac/faac.h>

#define LOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO,"Leo",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"Leo",FORMAT,##__VA_ARGS__);

#define ERROR_URL_NULL 0
#define ERROR_RTMP_INIT_FAIL 1
#define ERROR_RTMP_CONNECT_FAIL 2
#define ERROR_RTMP_CONNECT_STREAM_FAIL 3
#define ERROR_OPEN_ENCODER_FAIL 4

JavaVM *javaVm;
jobject jLivePush;
char *url;
int ylen, ulen, vlen;
x264_picture_t picin;
x264_picture_t picout;
x264_t *videoEncodeHandle;
//faac音频编码处理器
faacEncHandle audioEncodeHandle;
unsigned long nInputSamples; //输入的采样个数
unsigned long nMaxOutputBytes; //编码输出之后的字节数
pthread_mutex_t mutex;
pthread_cond_t cond;
uint32_t startTime;
bool isPush;
faacEncHandle hEncoder;

void *push(void *);

void addAACHeader();

void addRtmpPacket(RTMPPacket *packet);

void addH264Header(unsigned char *pps, unsigned char *sps, int pps_len, int sps_len);

void addH264Body(uint8_t *buf, int len);

void addAACBody(unsigned char *buf, int len);

void resultToJava(int code) {
    JNIEnv *env;
    javaVm->AttachCurrentThread(&env, nullptr);
    jclass jcls = env->GetObjectClass(jLivePush);
    jmethodID jmid = env->GetMethodID(jcls, "resultToJava", "(I)V");
    env->CallVoidMethod(jLivePush, jmid, code);
    javaVm->DetachCurrentThread();
}

extern "C" {
JNIEXPORT void JNICALL
Java_com_xin_ndkstudy_util_LivePushUtils_init(JNIEnv *env, jobject thiz) {
    env->GetJavaVM(&javaVm);
    jLivePush = env->NewGlobalRef(thiz);
}

JNIEXPORT void JNICALL
Java_com_xin_ndkstudy_util_LivePushUtils_startPush(JNIEnv *env, jobject thiz, jstring jurl) {
    const char *input = env->GetStringUTFChars(jurl, JNI_FALSE);
    if (input == nullptr) {
        resultToJava(ERROR_URL_NULL);
        LOGE("%s", "推流地址为空")
        return;
    }
    url = (char *) malloc(strlen(input) + 1);
    memset(url, 0, strlen(input) + 1);
    memcpy(url, input, strlen(input));

    pthread_mutex_init(&mutex, nullptr);
    pthread_cond_init(&cond, nullptr);

    create_queue();

    pthread_t push_id;
    pthread_create(&push_id, nullptr, push, nullptr);

    env->ReleaseStringUTFChars(jurl, input);
}

JNIEXPORT void JNICALL
Java_com_xin_ndkstudy_util_LivePushUtils_stopPush(JNIEnv *env, jobject thiz) {
    isPush = false;
}

JNIEXPORT void JNICALL
Java_com_xin_ndkstudy_util_LivePushUtils_release(JNIEnv *env, jobject thiz) {
    isPush = false;
    env->DeleteGlobalRef(jLivePush);
}

JNIEXPORT void JNICALL
Java_com_xin_ndkstudy_util_LivePushUtils_setVideoOption(JNIEnv *env, jobject thiz, jint width,
                                                        jint height, jint bitrate, jint fps) {
    x264_param_t param;
//    x264_param_default_preset(&param,"fast","zerolatency");
    x264_param_default_preset(&param, "ultrafast", "zerolatency");
    param.i_csp = X264_CSP_I420;
    param.i_width = width;
    param.i_height = height;
    ylen = width / height;
    ulen = ylen / 4;
    vlen = ylen / 4;
    //参数i_rc_method表示码率控制，CQP(恒定质量)，CRF(恒定码率)，ABR(平均码率)
    //恒定码率，会尽量控制在固定码率
    param.rc.i_rc_method = X264_RC_CRF;
    param.rc.i_bitrate = bitrate / 1000; //* 码率(比特率,单位Kbps)
    param.rc.i_vbv_max_bitrate = bitrate / 1000 * 1.2; //瞬时最大码率

    //码率控制不通过timebase和timestamp，而是fps
    param.b_vfr_input = 0;
    param.i_fps_num = fps; //* 帧率分子
    param.i_fps_den = 1; //* 帧率分母
    param.i_timebase_den = param.i_fps_num;
    param.i_timebase_num = param.i_fps_den;
    param.i_threads = 1;//并行编码线程数量，0默认为多线程

    //是否把SPS和PPS放入每一个关键帧
    //SPS Sequence Parameter Set 序列参数集，PPS Picture Parameter Set 图像参数集
    //为了提高图像的纠错能力
    param.b_repeat_headers = 1;
    //设置Level级别
    param.i_level_idc = 51;
    //设置Profile档次
    //baseline级别，没有B帧
    x264_param_apply_profile(&param, "baseline");

    //x264_picture_t（输入图像）初始化
    x264_picture_alloc(&picin, param.i_csp, param.i_width, param.i_height);
    picin.i_pts = 0;
    //打开编码器
    videoEncodeHandle = x264_encoder_open(&param);
    if (videoEncodeHandle) {
        LOGI("open video encoder success")
    } else {
        LOGE("open video encoder fail")
        resultToJava(ERROR_OPEN_ENCODER_FAIL);
    }
}

JNIEXPORT void JNICALL
Java_com_xin_ndkstudy_util_LivePushUtils_setAudioOptions(JNIEnv *env, jobject thiz,
                                                         jint sampleRateInHz, jint channel) {
    audioEncodeHandle = faacEncOpen(sampleRateInHz, channel, &nInputSamples, &nMaxOutputBytes);
    if (!audioEncodeHandle) {
        LOGE("open audio encoder fail")
        return;
    }
    //设置音频编码参数
    faacEncConfigurationPtr config = faacEncGetCurrentConfiguration(audioEncodeHandle);
    config->mpegVersion = MPEG4;
    config->allowMidside = 1;
    config->aacObjectType = LOW;
    config->outputFormat = 0; //输出是否包含ADTS头
    config->useTns = 1; //时域噪音控制,大概就是消爆音
    config->useLfe = 0;
//	config->inputFormat = FAAC_INPUT_16BIT;
    config->quantqual = 100;
    config->bandWidth = 0; //频宽
    config->shortctl = SHORTCTL_NORMAL;

    if (!faacEncSetConfiguration(audioEncodeHandle, config)) {
        LOGE("音频编码器配置失败")
        resultToJava(ERROR_OPEN_ENCODER_FAIL);
        return;
    }
    LOGI("音频编码器配置成功")
}

JNIEXPORT void JNICALL
Java_com_xin_ndkstudy_util_LivePushUtils_fireVideo(JNIEnv *env, jobject thiz, jbyteArray data) {
    jbyte *nv21 = env->GetByteArrayElements(data, JNI_FALSE);
    uint8_t *u = picin.img.plane[1];
    uint8_t *v = picin.img.plane[2];
    //nv21 4:2:0 Formats, 12 Bits per Pixel
    //nv21与yuv420p，y个数一致，uv位置对调
    //nv21转yuv420p  y = w*h,u/v=w*h/4
    //nv21 = yvu yuv420p=yuv y=y u=y+1+1 v=y+1
    memcpy(picin.img.plane[0], nv21, ylen);
    int i;
    for (i = 0; i < ulen; i++) {
        *(u + i) = *(nv21 + ylen + i * 2 + 1);
        *(v + i) = *(nv21 + ylen + i * 2);
    }
    //h264编码得到NALU数组
    x264_nal_t *nal = nullptr; //NAL
    int n_nal = -1; //NALU的个数
    //进行h264编码
    if (x264_encoder_encode(videoEncodeHandle, &nal, &n_nal, &picin, &picout) < 0) {
        LOGE("编码失败");
        return;
    }
    //使用rtmp协议将h264编码的视频数据发送给流媒体服务器
    //帧分为关键帧和普通帧，为了提高画面的纠错率，关键帧应包含SPS和PPS数据
    int sps_len = 0, pps_len = 0;
    unsigned char sps[100];
    unsigned char pps[100];
    memset(sps, 0, 100);
    memset(pps, 0, 100);
    picin.i_pts += 1; //顺序累加
    //遍历NALU数组，根据NALU的类型判断
    for (i = 0; i < n_nal; i++) {
        if (nal[i].i_type == NAL_SPS) {
            //复制SPS数据
            sps_len = nal[i].i_payload - 4;
            memcpy(sps, nal[i].p_payload + 4, sps_len); //不复制四字节起始码
        } else if (nal[i].i_type == NAL_PPS) {
            //复制PPS数据
            pps_len = nal[i].i_payload - 4;
            memcpy(pps, nal[i].p_payload + 4, pps_len); //不复制四字节起始码
            //发送序列信息
            //h264关键帧会包含SPS和PPS数据
            LOGI("sps_len =%d,pps_len=%d", sps_len, pps_len)
            addH264Header(pps, sps, pps_len, sps_len);
        } else {
            //发送帧信息
            addH264Body(nal[i].p_payload, nal[i].i_payload);
        }
        LOGI("fire video %d", i)
    }
    env->ReleaseByteArrayElements(data, nv21, JNI_FALSE);
}
JNIEXPORT void JNICALL
Java_com_xin_ndkstudy_util_LivePushUtils_fireAudio(JNIEnv *env, jobject thiz, jbyteArray data,
                                                   jint len) {
    int *pcmbuf;
    unsigned char *bitbuf;
    jbyte *buffer = env->GetByteArrayElements(data, JNI_FALSE);
    pcmbuf = (int *) malloc(nInputSamples * sizeof(int));
    bitbuf = (unsigned char *) malloc(nMaxOutputBytes * sizeof(unsigned char));
    int nByteCount = 0;
    unsigned int nBufferSize = (unsigned int) len / 2;
    auto *buf = (unsigned short *) buffer;
    while (nByteCount < nBufferSize) {
        int audioLength = nInputSamples;
        if ((nByteCount + nInputSamples) >= nBufferSize) {
            audioLength = nBufferSize - nByteCount;
        }
        int i;
        for (i = 0; i < audioLength; i++) {//每次从实时的pcm音频队列中读出量化位数为8的pcm数据。
            int s = ((int16_t *) buf + nByteCount)[i];
            pcmbuf[i] = s << 8;//用8个二进制位来表示一个采样量化点（模数转换）
        }
        nByteCount += nInputSamples;
        //利用FAAC进行编码，pcmbuf为转换后的pcm流数据，audioLength为调用faacEncOpen时得到的输入采样数，bitbuf为编码后的数据buff，nMaxOutputBytes为调用faacEncOpen时得到的最大输出字节数
        int byteslen = faacEncEncode(audioEncodeHandle, pcmbuf, audioLength, bitbuf,
                                     nMaxOutputBytes);
        if (byteslen < 1) continue;

        addAACBody(bitbuf, byteslen);//从bitbuf中得到编码后的aac数据流，放到数据队列
    }
    env->ReleaseByteArrayElements(data, buffer, JNI_FALSE);
    if (bitbuf) free(bitbuf);
    if (pcmbuf) free(pcmbuf);
}
}

void *push(void *) {
    JNIEnv *env;
    javaVm->AttachCurrentThread(&env, nullptr);
    RTMP *rtmp = RTMP_Alloc();
    if (!rtmp) {
        resultToJava(ERROR_RTMP_INIT_FAIL);
        LOGE("RTMP init fail")
        goto end;
    }
    RTMP_Init(rtmp);
    rtmp->Link.timeout = 5;
    RTMP_SetupURL(rtmp, url);
    //发布数据
    RTMP_EnableWrite(rtmp);
    if (!RTMP_Connect(rtmp, nullptr)) {
        resultToJava(ERROR_RTMP_CONNECT_FAIL);
        LOGE("RTMP connect fail")
        goto end;
    }
    startTime = RTMP_GetTime();
    if (!RTMP_ConnectStream(rtmp, 0)) {
        resultToJava(ERROR_RTMP_CONNECT_STREAM_FAIL);
        LOGE("RTMP connect stream fail")
        goto end;
    }
    isPush = true;
    addAACHeader();
    while (isPush) {
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&cond, &mutex);
        auto *packet = (RTMPPacket *) queue_get_first();
        if (packet) {
            //取出一个删除一个
            queue_delete_first();
            //RTMP 协议
            packet->m_nInfoField2 = rtmp->m_stream_id;
            //1为加入rtmp队列,不立马发送
            int i = RTMP_SendPacket(rtmp, packet, TRUE);
            if (!i) {
                LOGE("RTMP disconnect")
                RTMPPacket_Free(packet);
                pthread_mutex_unlock(&mutex);
                goto end;
            }
            LOGI("RTMP send packet")
            RTMPPacket_Free(packet);
        }
    }

    end:
    LOGI("release")
    free(url);
    RTMP_Close(rtmp);
    RTMP_Free(rtmp);
    javaVm->DetachCurrentThread();
    return nullptr;
}

void addAACHeader() {
    unsigned char *buffer;
    unsigned long len;
    faacEncGetDecoderSpecificInfo(hEncoder, &buffer, &len);
    int bodySize = 2 + len;
    auto *rtmpPacket = (RTMPPacket *) malloc(sizeof(RTMPPacket));
    RTMPPacket_Alloc(rtmpPacket, bodySize);
    RTMPPacket_Reset(rtmpPacket);
    char *body = rtmpPacket->m_body;
    body[0] = 0XAF;
    body[1] = 0X00;
    //buffer是AAC sequence header数据
    memcpy(&body[2], buffer, len);
    rtmpPacket->m_packetType = RTMP_PACKET_TYPE_AUDIO;
    rtmpPacket->m_nBodySize = bodySize;
    rtmpPacket->m_nChannel = 0x04;
    rtmpPacket->m_hasAbsTimestamp = 0;
    rtmpPacket->m_nTimeStamp = 0;
    rtmpPacket->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    addRtmpPacket(rtmpPacket);
//    free(buffer);
}

/**
 * 加入RTMPPacket队列，等待发送线程发送
 */
void addRtmpPacket(RTMPPacket *packet) {
    pthread_mutex_lock(&mutex);
    if (isPush) {
        queue_append_last(packet);
    }
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

void addH264Header(unsigned char *pps, unsigned char *sps, int pps_len, int sps_len) {
    int bodySize = 16 + sps_len + pps_len; //按照H264标准配置SPS和PPS，共使用了16字节
    auto *packet = (RTMPPacket *) malloc(sizeof(RTMPPacket));
    //RTMPPacket初始化
    RTMPPacket_Alloc(packet, bodySize);
    RTMPPacket_Reset(packet);

    char *body = packet->m_body;
    int i = 0;
    //二进制表示：00010111
    body[i++] = 0x17;//VideoHeaderTag:FrameType(1=key frame)+CodecID(7=AVC)
    body[i++] = 0x00;//AVCPacketType = 0表示设置AVCDecoderConfigurationRecord
    //composition time 0x000000 24bit ?
    body[i++] = 0x00;
    body[i++] = 0x00;
    body[i++] = 0x00;

    /*AVCDecoderConfigurationRecord*/
    body[i++] = 0x01;//configurationVersion，版本为1
    body[i++] = sps[1];//AVCProfileIndication
    body[i++] = sps[2];//profile_compatibility
    body[i++] = sps[3];//AVCLevelIndication
    //?
    body[i++] = 0xFF;//lengthSizeMinusOne,H264 视频中 NALU的长度，计算方法是 1 + (lengthSizeMinusOne & 3),实际测试时发现总为FF，计算结果为4.

    /*sps*/
    body[i++] = 0xE1;//numOfSequenceParameterSets:SPS的个数，计算方法是 numOfSequenceParameterSets & 0x1F,实际测试时发现总为E1，计算结果为1.
    body[i++] = (sps_len >> 8) & 0xff;//sequenceParameterSetLength:SPS的长度
    body[i++] = sps_len & 0xff;//sequenceParameterSetNALUnits
    memcpy(&body[i], sps, sps_len);
    i += sps_len;

    /*pps*/
    body[i++] = 0x01;//numOfPictureParameterSets:PPS 的个数,计算方法是 numOfPictureParameterSets & 0x1F,实际测试时发现总为E1，计算结果为1.
    body[i++] = (pps_len >> 8) & 0xff;//pictureParameterSetLength:PPS的长度
    body[i++] = (pps_len) & 0xff;//PPS
    memcpy(&body[i], pps, pps_len);
    i += pps_len;

    //Message Type，RTMP_PACKET_TYPE_VIDEO：0x09
    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    //Payload Length
    packet->m_nBodySize = bodySize;
    //Time Stamp：4字节
    //记录了每一个tag相对于第一个tag（File Header）的相对时间。
    //以毫秒为单位。而File Header的time stamp永远为0。
    packet->m_nTimeStamp = 0;
    packet->m_hasAbsTimestamp = 0;
    packet->m_nChannel = 0x04; //Channel ID，Audio和Vidio通道
    packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    //将RTMPPacket加入队列
    addRtmpPacket(packet);
}

void addH264Body(uint8_t *buf, int len) {
//去掉起始码(界定符)
    if (buf[2] == 0x00) {  //00 00 00 01
        buf += 4;
        len -= 4;
    } else if (buf[2] == 0x01) { // 00 00 01
        buf += 3;
        len -= 3;
    }
    int body_size = len + 9;
    auto *packet = (RTMPPacket *) malloc(sizeof(RTMPPacket));
    RTMPPacket_Alloc(packet, body_size);

    char *body = packet->m_body;
    //当NAL头信息中，type（5位）等于5，说明这是关键帧NAL单元
    //buf[0] NAL Header与运算，获取type，根据type判断关键帧和普通帧
    //00000101 & 00011111(0x1f) = 00000101
    int type = buf[0] & 0x1f;
    //Inter Frame 帧间压缩
    body[0] = 0x27;//VideoHeaderTag:FrameType(2=Inter Frame)+CodecID(7=AVC)
    //IDR I帧图像
    if (type == NAL_SLICE_IDR) {
        body[0] = 0x17;//VideoHeaderTag:FrameType(1=key frame)+CodecID(7=AVC)
    }
    //AVCPacketType = 1
    body[1] = 0x01; /*nal unit,NALUs（AVCPacketType == 1)*/
    body[2] = 0x00; //composition time 0x000000 24bit
    body[3] = 0x00;
    body[4] = 0x00;

    //写入NALU信息，右移8位，一个字节的读取？
    body[5] = (len >> 24) & 0xff;
    body[6] = (len >> 16) & 0xff;
    body[7] = (len >> 8) & 0xff;
    body[8] = (len) & 0xff;

    /*copy data*/
    memcpy(&body[9], buf, len);

    packet->m_hasAbsTimestamp = 0;
    packet->m_nBodySize = body_size;
    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;//当前packet的类型：Video
    packet->m_nChannel = 0x04;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
//	packet->m_nTimeStamp = -1;
    packet->m_nTimeStamp = RTMP_GetTime() - startTime;//记录了每一个tag相对于第一个tag（File Header）的相对时间
    addRtmpPacket(packet);
}

void addAACBody(unsigned char *buf, int len) {
    int body_size = 2 + len;
    auto *packet = (RTMPPacket *) malloc(sizeof(RTMPPacket));
    //RTMPPacket初始化
    RTMPPacket_Alloc(packet, body_size);
    RTMPPacket_Reset(packet);
    char *body = packet->m_body;
    //头信息配置
    /*AF 00 + AAC RAW data*/
    body[0] = 0xAF;//10 5 SoundFormat(4bits):10=AAC,SoundRate(2bits):3=44kHz,SoundSize(1bit):1=16-bit samples,SoundType(1bit):1=Stereo sound
    body[1] = 0x01;//AACPacketType:1表示AAC raw
    memcpy(&body[2], buf, len); /*spec_buf是AAC raw数据*/
    packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
    packet->m_nBodySize = body_size;
    packet->m_nChannel = 0x04;
    packet->m_hasAbsTimestamp = 0;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    packet->m_nTimeStamp = RTMP_GetTime() - startTime;
    addRtmpPacket(packet);
}