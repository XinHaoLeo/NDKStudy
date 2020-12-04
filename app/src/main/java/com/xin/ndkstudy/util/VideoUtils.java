package com.xin.ndkstudy.util;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.util.Log;
import android.view.Surface;

/**
 * █████▒█    ██  ▄████▄   ██ ▄█▀       ██████╗ ██╗   ██╗ ██████╗
 * ▓██   ▒ ██  ▓██▒▒██▀ ▀█   ██▄█▒        ██╔══██╗██║   ██║██╔════╝
 * ▒████ ░▓██  ▒██░▒▓█    ▄ ▓███▄░        ██████╔╝██║   ██║██║  ███╗
 * ░▓█▒  ░▓▓█  ░██░▒▓▓▄ ▄██▒▓██ █▄        ██╔══██╗██║   ██║██║   ██║
 * ░▒█░   ▒▒█████▓ ▒ ▓███▀ ░▒██▒ █▄       ██████╔╝╚██████╔╝╚██████╔╝
 * ▒ ░   ░▒▓▒ ▒ ▒ ░ ░▒ ▒  ░▒ ▒▒ ▓▒       ╚═════╝  ╚═════╝  ╚═════╝
 * ░     ░░▒░ ░ ░   ░  ▒   ░ ░▒ ▒░
 * ░ ░    ░░░ ░ ░ ░        ░ ░░ ░
 * ░     ░ ░      ░  ░
 *
 * @author : Leo
 * @date : 2020/10/20 19:31
 * @desc :
 * @since : xinxiniscool@gmail.com
 */
public class VideoUtils {
    private static final VideoUtils ourInstance = new VideoUtils();

    public final static int ERROR_FILE_PATH_NULL = 0x01;

    public final static int ERROR_OPEN_VIDEO_FILE = 0x02;

    public final static int ERROR_VIDEO_STREAM_INFO = 0x03;

    public final static int ERROR_VIDEO_DECODER = 0x04;

    public static VideoUtils getInstance() {
        return ourInstance;
    }

    /**
     * 初始化AudioTrack给C++调用播放
     *
     * @param sampleRateInHz 采样率hz
     * @param channels       声道布局
     * @return AudioTrack
     */
    public AudioTrack initAudioTrack(int sampleRateInHz, int channels) {
        //固定格式的音频码流
        int audioFormat = AudioFormat.ENCODING_PCM_16BIT;
        //声道布局
        int channelConfig;
        if (channels == 1) {
            channelConfig = android.media.AudioFormat.CHANNEL_OUT_MONO;
        } else if (channels == 2) {
            channelConfig = android.media.AudioFormat.CHANNEL_OUT_STEREO;
        } else {
            channelConfig = android.media.AudioFormat.CHANNEL_OUT_STEREO;
        }
        int bufferSizeInBytes = AudioTrack.getMinBufferSize(sampleRateInHz, channelConfig, audioFormat);
        Log.d("Leo", "initAudioTrack: sampleRateInHz=" + sampleRateInHz + "----"
                + "channels=" + channels + "-----" + "bufferSizeInBytes=" + bufferSizeInBytes);
        return new AudioTrack(AudioManager.STREAM_MUSIC,
                sampleRateInHz, channelConfig,
                audioFormat,
                bufferSizeInBytes /* *4*/, AudioTrack.MODE_STREAM);
    }

    private VideoUtils() {
    }

    public native void init();

    public native void play(String input, Surface surface);

    public native void release();

    public void receiveResult(int errorCode) {
        switch (errorCode) {
            case ERROR_FILE_PATH_NULL:
                break;
            case ERROR_OPEN_VIDEO_FILE:
                break;
            default:
                break;
        }
    }

    static {
        System.loadLibrary("ffmpeg-lib");
    }
}
