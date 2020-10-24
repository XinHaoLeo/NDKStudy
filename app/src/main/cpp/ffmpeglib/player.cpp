//
//
//
//@author : Leo
//@date : 2020/10/20 19:58
//@since : lightingxin@qq.com
//@desc : 
//
//
#include <jni.h>
#include <malloc.h>
#include <memory>
#include <cstring>
#include <pthread.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <unistd.h>


extern "C" {
#include <include/libavcodec/avcodec.h>
#include <include/libavformat/avformat.h>
#include <include/libswscale/swscale.h>
#include <include/libavutil/imgutils.h>
#include <include/libswresample/swresample.h>
}

#define LOGI(format, ...) __android_log_print(ANDROID_LOG_INFO, "Leo", format, ##__VA_ARGS__);
#define LOGE(format, ...) __android_log_print(ANDROID_LOG_ERROR, "Leo", format, ##__VA_ARGS__);
#define MAX_AUDIO_FRAME_SIZE 48000 * 4

const int ERROR_FILE_PATH_NULL = 0x01;

const int ERROR_OPEN_VIDEO_FILE = 0x02;

const int ERROR_VIDEO_STREAM_INFO = 0x03;

const int ERROR_VIDEO_DECODER = 0x04;

const int ERROR_OPEN_VIDEO_DECODER = 0x05;

typedef struct _Player Player;

JavaVM *javaVm;
jobject jVideoPlayer;
jobject jAudioTrack;
char *inputPath;
static Player *player;
int isPlaying = JNI_TRUE;

int initFormatCtx();

int initCodecCtx(int streamIndex);

void decodeVideoPrepare(JNIEnv *env, jobject surface);

void *decodeData(void *arg);

void decodeVideo(Player *pPlayer, AVPacket *avPacket);

void decodeAudioPrepare();

void jniAudioPrepare(JNIEnv *env);

void decodeAudio(Player *sPlayer, AVPacket *avPacket);

struct _Player {
    //封装格式上下文
    AVFormatContext *inputFormatCtx;
    //音频视频流索引位置
    int videoStreamIndex;
    int audioStreamIndex;
    //解码器上下文数组
    AVCodecContext *inputCodecCtx[2];
    //解码线程ID
    pthread_t decodeThreads;
    ANativeWindow *nativeWindow;
    SwrContext *swrCtx;
    //输入的采样格式
    enum AVSampleFormat inSampleFmt;
    //输出采样格式16bit PCM
    enum AVSampleFormat outSampleFmt;
    //输入采样率
    int inSampleRate;
    //输出采样率
    int outSampleRate;
    //输出的声道个数
    int outChannelNumber;
    //JNI
    jmethodID audioTrackWriteMid;
};

void resultToJava(int errorCode) {
    JNIEnv *env;
    javaVm->AttachCurrentThread(&env, nullptr);
    jclass jcls = env->GetObjectClass(jVideoPlayer);
    jmethodID jmid = env->GetMethodID(jcls, "receiveResult", "(I)V");
    env->CallVoidMethod(jVideoPlayer, jmid, errorCode);
    //使用DetachCurrentThread()会崩溃,还未查清楚什么问题
//    javaVm->DetachCurrentThread();
}

extern "C" {
JNIEXPORT void JNICALL
Java_com_xin_ndkstudy_VideoUtils_init(JNIEnv *env, jobject thiz) {
    env->GetJavaVM(&javaVm);
    jVideoPlayer = env->NewGlobalRef(thiz);
    if (player == nullptr) {
        player = (Player *) malloc(sizeof(Player));
    }
}

JNIEXPORT void JNICALL
Java_com_xin_ndkstudy_VideoUtils_play(JNIEnv *env, jobject thiz, jstring jInput,
                                      jobject surface) {
    const char *input = env->GetStringUTFChars(jInput, JNI_FALSE);
    if (input == nullptr) {
        resultToJava(ERROR_FILE_PATH_NULL);
        LOGE("%s", "文件地址为空")
        return;
    }
    isPlaying = JNI_TRUE;
    inputPath = (char *) malloc(strlen(input) + 1);
    memset(inputPath, 0, strlen(input) + 1);
    memcpy(inputPath, input, strlen(input));
    if (initFormatCtx() >= 0) {
        if (initCodecCtx(player->videoStreamIndex) >= 0 &&
            initCodecCtx(player->audioStreamIndex) >= 0) {
            decodeVideoPrepare(env, surface);
            decodeAudioPrepare();
            jniAudioPrepare(env);
            pthread_create(&(player->decodeThreads), nullptr, decodeData, (void *) player);
        }
    }
    env->ReleaseStringUTFChars(jInput, input);
}

JNIEXPORT void JNICALL
Java_com_xin_ndkstudy_VideoUtils_release(JNIEnv *env, jobject thiz) {
    isPlaying = JNI_FALSE;
    if (jVideoPlayer != nullptr) {
        env->DeleteGlobalRef(jVideoPlayer);
    }
    if (jAudioTrack != nullptr) {
        env->DeleteGlobalRef(jAudioTrack);
    }
    if (player != nullptr) {
        free(player);
    }
//    if (javaVm != nullptr) {
//        javaVm->DestroyJavaVM();
//    }
}
}

int initFormatCtx() {
    av_register_all();
    AVFormatContext *pContext = avformat_alloc_context();
    if (avformat_open_input(&pContext, inputPath, nullptr, nullptr) != 0) {
        resultToJava(ERROR_OPEN_VIDEO_FILE);
        LOGE("%s", "视频文件打开失败")
        free(inputPath);
        return -1;
    }
    if (avformat_find_stream_info(pContext, nullptr) < 0) {
        resultToJava(ERROR_VIDEO_STREAM_INFO);
        LOGE("%s", "获取视频信息失败")
        free(inputPath);
        return -1;
    }
    int i = 0;
    for (; i < pContext->nb_streams; ++i) {
        if (pContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            player->videoStreamIndex = i;
        } else if (pContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            player->audioStreamIndex = i;
        }
    }
    player->inputFormatCtx = pContext;
    free(inputPath);
    return 0;
}

int initCodecCtx(int streamIndex) {
    AVFormatContext *pContext = player->inputFormatCtx;
    AVCodecContext *pAvCodecContext = pContext->streams[streamIndex]->codec;
    AVCodec *pCodec = avcodec_find_decoder(pAvCodecContext->codec_id);
    if (pCodec == nullptr) {
        resultToJava(ERROR_VIDEO_DECODER);
        LOGE("%s", "解码失败")
        return -1;
    }
    if (avcodec_open2(pAvCodecContext, pCodec, nullptr) < 0) {
        resultToJava(ERROR_OPEN_VIDEO_DECODER);
        LOGE("%s", "解码器无法打开")
        return -1;
    }
    player->inputCodecCtx[streamIndex] = pAvCodecContext;
    return 0;
}

void decodeVideoPrepare(JNIEnv *env, jobject surface) {
//    JNIEnv *env;
//    javaVm->AttachCurrentThread(&env, nullptr);
    player->nativeWindow = ANativeWindow_fromSurface(env, surface);
//    if (javaVm != nullptr)
//        javaVm->DetachCurrentThread();
}

void *decodeData(void *arg) {
    auto *pPlayer = (Player *) arg;
    AVFormatContext *pContext = pPlayer->inputFormatCtx;
    auto *pPacket = (AVPacket *) av_malloc(sizeof(AVPacket));
    int frameCount = 0;
    while (av_read_frame(pContext, pPacket) >= 0 && isPlaying) {
        if (pPacket->stream_index == pPlayer->videoStreamIndex) {
            decodeVideo(pPlayer, pPacket);
            LOGI("video frameCount:%d", frameCount++)
        } else if (pPacket->stream_index == pPlayer->audioStreamIndex) {
            decodeAudio(pPlayer, pPacket);
            LOGI("audio frameCount:%d", frameCount++)
        }
//        av_packet_free(&pPacket);
        av_packet_unref(pPacket);
//        av_free_packet(pPacket);
    }
    LOGI("isPlaying==0是自己主动退出,isPlaying:%d", isPlaying)
    LOGI("%s", "解码完成")
//    free(pPlayer->audioTrackWriteMid);
    swr_free(&(pPlayer->swrCtx));
    avformat_free_context(pPlayer->inputFormatCtx);
    avcodec_close(pPlayer->inputCodecCtx[pPlayer->videoStreamIndex]);
    avcodec_close(pPlayer->inputCodecCtx[pPlayer->audioStreamIndex]);
//    ANativeWindow_release(pPlayer->nativeWindow);
    return nullptr;
}

void decodeVideo(Player *pPlayer, AVPacket *avPacket) {
    AVFrame *yuvFrame = av_frame_alloc();
    AVFrame *rgbFrame = av_frame_alloc();
    if (yuvFrame == nullptr || rgbFrame == nullptr) {
        LOGE("yuvFrame=null,rgbFrame=null")
        resultToJava(ERROR_VIDEO_DECODER);
        return;
    }
    ANativeWindow_Buffer buffer;
    AVCodecContext *pAvCodecContext = pPlayer->inputCodecCtx[pPlayer->videoStreamIndex];
    int gotFrame;
    int ret = avcodec_decode_video2(pAvCodecContext, yuvFrame, &gotFrame, avPacket);
    if (ret < 0) {
        av_frame_free(&yuvFrame);
        av_frame_free(&rgbFrame);
        return;
    }
    if (gotFrame) {
        //设置缓冲区的属性
        ANativeWindow_setBuffersGeometry(pPlayer->nativeWindow, pAvCodecContext->width,
                                         pAvCodecContext->height, WINDOW_FORMAT_RGBA_8888);
        ANativeWindow_lock(pPlayer->nativeWindow, &buffer, nullptr);

//        int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, pAvCodecContext->width,
//                                                pAvCodecContext->height, 1);
//        auto buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
        av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, (uint8_t *) buffer.bits,
                             AV_PIX_FMT_RGBA, pAvCodecContext->width, pAvCodecContext->height, 1);
//        avpicture_fill((AVPicture *) rgbFrame, (const uint8_t *) buffer.bits, AV_PIX_FMT_RGBA,
//                       pAvCodecContext->width, pAvCodecContext->height);
        //转成RGBA格式
        SwsContext *pSwsContext = sws_getContext(pAvCodecContext->width, pAvCodecContext->height,
                                                 pAvCodecContext->pix_fmt, pAvCodecContext->width,
                                                 pAvCodecContext->height, AV_PIX_FMT_RGBA,
                                                 SWS_BILINEAR, nullptr, nullptr, nullptr);

        sws_scale(pSwsContext, (uint8_t const *const *) yuvFrame->data,
                  yuvFrame->linesize, 0, pAvCodecContext->height,
                  rgbFrame->data, rgbFrame->linesize);
        ANativeWindow_unlockAndPost(pPlayer->nativeWindow);
        usleep(1000 * 16);
    }
    av_frame_free(&yuvFrame);
    av_frame_free(&rgbFrame);
}

void decodeAudioPrepare() {
    AVCodecContext *pAvCodecContext = player->inputCodecCtx[player->audioStreamIndex];
    //重采样设置参数-------------start
    //输入的采样格式
    enum AVSampleFormat inSampleFmt = pAvCodecContext->sample_fmt;
    //输出采样格式16bit PCM
    enum AVSampleFormat outSampleFmt = AV_SAMPLE_FMT_S16;
    //输入采样率
    int inSampleRate = pAvCodecContext->sample_rate;
    //输出采样率
    int outSampleRate = inSampleRate;
    //获取输入的声道布局
    //根据声道个数获取默认的声道布局（2个声道，默认立体声stereo）
    //av_get_default_channel_layout(codecCtx->channels);
    uint64_t inChLayout = pAvCodecContext->channel_layout;
    //输出的声道布局（立体声）
    uint64_t outChLayout = AV_CH_LAYOUT_STEREO;
    SwrContext *pSwrContext = swr_alloc();
    //frame->16bit 44100 PCM 统一音频采样格式与采样率
    swr_alloc_set_opts(pSwrContext, outChLayout, outSampleFmt, outSampleRate, inChLayout,
                       inSampleFmt,
                       inSampleRate, 0, nullptr);
    swr_init(pSwrContext);
    //输出的声道个数
    int outChannelNumber = av_get_channel_layout_nb_channels(outChLayout);
    player->swrCtx = pSwrContext;
    player->inSampleFmt = inSampleFmt;
    player->outSampleFmt = outSampleFmt;
    player->inSampleRate = inSampleRate;
    player->outSampleRate = outSampleRate;
    player->outChannelNumber = outChannelNumber;
}

void jniAudioPrepare(JNIEnv *env) {
    jclass jcls = env->GetObjectClass(jVideoPlayer);
    jmethodID jmid = env->GetMethodID(jcls, "initAudioTrack", "(II)Landroid/media/AudioTrack;");
    jobject jobj = env->CallObjectMethod(jVideoPlayer, jmid, player->outSampleRate,
                                         player->outChannelNumber);
    //获取到AudioTrack对象
    jclass audioTrack = env->GetObjectClass(jobj);
    jmethodID jAudioPlayMid = env->GetMethodID(audioTrack, "play", "()V");
    //调用play方法播放
    env->CallVoidMethod(jobj, jAudioPlayMid);
    jmethodID jAudioWriteMid = env->GetMethodID(audioTrack, "write", "([BII)I");
    jAudioTrack = env->NewGlobalRef(jobj);
    player->audioTrackWriteMid = jAudioWriteMid;
}

void decodeAudio(Player *sPlayer, AVPacket *avPacket) {
    AVCodecContext *pAvCodecContext = sPlayer->inputCodecCtx[sPlayer->audioStreamIndex];
    AVFrame *frame = av_frame_alloc();
    if (frame == nullptr) {
        LOGE("%s", "frame = null")
        return;
    }
    auto *buffer = (uint8_t *) av_malloc(MAX_AUDIO_FRAME_SIZE);
    int gotFrame;
    int ret = avcodec_decode_audio4(pAvCodecContext, frame, &gotFrame, avPacket);
    if (ret < 0) {
        free(buffer);
        av_frame_free(&frame);
        return;
    }
    if (gotFrame) {
        swr_convert(sPlayer->swrCtx, &buffer, MAX_AUDIO_FRAME_SIZE, (const uint8_t **) frame->data,
                    frame->nb_samples);
        int bufferSize = av_samples_get_buffer_size(nullptr, sPlayer->outChannelNumber,
                                                    frame->nb_samples, sPlayer->outSampleFmt, 1);
        JNIEnv *env;
        javaVm->AttachCurrentThread(&env, nullptr);
        jbyteArray jbarray = env->NewByteArray(bufferSize);
        jbyte *jbSample = env->GetByteArrayElements(jbarray, nullptr);
        //buffer的数据复制到jbSample
        memcpy(jbSample, buffer, bufferSize);
        //同步jbarray数据
        env->ReleaseByteArrayElements(jbarray, jbSample, 0);
        env->CallIntMethod(jAudioTrack, sPlayer->audioTrackWriteMid, jbarray, 0, bufferSize);

        env->DeleteLocalRef(jbarray);
        javaVm->DetachCurrentThread();
        usleep(1000 * 16);
    }
    free(buffer);
    av_frame_free(&frame);
}