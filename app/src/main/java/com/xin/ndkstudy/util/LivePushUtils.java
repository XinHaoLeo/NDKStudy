package com.xin.ndkstudy.util;

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
 * @date : 2020/12/2 14:14
 * @desc :
 * @since :
 */
public class LivePushUtils {
    private static LivePushUtils ourInstance;

    public static LivePushUtils getInstance() {
        if (ourInstance == null) {
            synchronized (LivePushUtils.class) {
                if (ourInstance == null) {
                    ourInstance = new LivePushUtils();
                }
            }
        }
        return ourInstance;
    }

    private LivePushUtils() {
    }

    public void resultToJava(int code){

    }

    public native void init();

    public native void startPush(String url);

    public native void stopPush();

    public native void release();

    public native void setVideoOption(int width, int height, int bitrate, int fps);

    public native void setAudioOptions(int sampleRateInHz, int channel);

    public native void fireVideo(byte[] data);

    public native void fireAudio(byte[] data, int len);


    static {
        System.loadLibrary("live");
    }
}
