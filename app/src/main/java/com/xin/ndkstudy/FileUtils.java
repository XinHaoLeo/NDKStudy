package com.xin.ndkstudy;

import android.util.Log;

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
 * @date : 2020/10/13 10:08
 * @desc :
 * @since : xinxiniscool@gmail.com
 */
public class FileUtils {
    private static final FileUtils ourInstance = new FileUtils();

    public static final int ERROR = 0;
    public static final int SUCCESS = 1;

    public static FileUtils getInstance() {
        return ourInstance;
    }

    private FileUtils() {
    }

    public native void encrypt(String normalPath, String encryptPath);

    public native void decrypt(String encryptPath, String decryptPath);

    static {
        System.loadLibrary("file-lib");
    }

    public void fileResult(int code, String msg) {
        Log.d("Leo", "fileResult: code =" + code);
        Log.d("Leo", "fileResult: msg =" + msg);
        switch (code) {
            case ERROR:
                break;
            case SUCCESS:
                break;
        }
    }
}
