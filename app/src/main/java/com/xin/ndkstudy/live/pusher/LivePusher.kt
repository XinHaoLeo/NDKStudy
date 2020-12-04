package com.xin.ndkstudy.live.pusher

import android.hardware.Camera
import android.view.SurfaceHolder
import com.xin.ndkstudy.live.parm.AudioParam
import com.xin.ndkstudy.live.parm.VideoParam
import com.xin.ndkstudy.util.LivePushUtils


/**
 *
 *   █████▒█    ██  ▄████▄   ██ ▄█▀       ██████╗ ██╗   ██╗ ██████╗
 * ▓██   ▒ ██  ▓██▒▒██▀ ▀█   ██▄█▒        ██╔══██╗██║   ██║██╔════╝
 * ▒████ ░▓██  ▒██░▒▓█    ▄ ▓███▄░        ██████╔╝██║   ██║██║  ███╗
 * ░▓█▒  ░▓▓█  ░██░▒▓▓▄ ▄██▒▓██ █▄        ██╔══██╗██║   ██║██║   ██║
 * ░▒█░   ▒▒█████▓ ▒ ▓███▀ ░▒██▒ █▄       ██████╔╝╚██████╔╝╚██████╔╝
 *  ▒ ░   ░▒▓▒ ▒ ▒ ░ ░▒ ▒  ░▒ ▒▒ ▓▒       ╚═════╝  ╚═════╝  ╚═════╝
 *  ░     ░░▒░ ░ ░   ░  ▒   ░ ░▒ ▒░
 *  ░ ░    ░░░ ░ ░ ░        ░ ░░ ░
 *           ░     ░ ░      ░  ░
 *@author : Leo
 *@date : 2020/12/2 13:49
 *@since :
 *@desc :
 */
class LivePusher(
    private val width: Int,
    private val height: Int,
    private val surfaceHolder: SurfaceHolder
) : SurfaceHolder.Callback {

    private lateinit var mVideoPusher: VideoPusher
    private lateinit var mAudioPusher: AudioPusher
    private val mLiveVideoUtils by lazy { LivePushUtils.getInstance() }

    init {
        surfaceHolder.addCallback(this)
        prepare()
    }

    /**
     * 预览准备
     */
    private fun prepare() {

        //实例化视频推流器
//        val videoParam = VideoParam(480, 320, CameraCharacteristics.LENS_FACING_BACK)
        if (Camera.getNumberOfCameras() > 0) {
            val videoParam = VideoParam(width, height, Camera.CameraInfo.CAMERA_FACING_BACK)
            mVideoPusher = VideoPusher(surfaceHolder, videoParam)
        }
        //实例化音频推流器
        val audioParam = AudioParam()
        mAudioPusher = AudioPusher(audioParam)
    }

    fun switchCamera() {
        mVideoPusher.switchCamera()
    }

    fun startPush(url: String) {
        mVideoPusher.startPush()
        mAudioPusher.startPush()
        mLiveVideoUtils.startPush(url)
    }

    /**
     * 停止推流
     */
    fun stopPush() {
        mVideoPusher.stopPush()
        mAudioPusher.stopPush()
        mLiveVideoUtils.stopPush()
    }

    /**
     * 释放资源
     */
    private fun release() {
        mVideoPusher.release()
        mAudioPusher.release()
        mLiveVideoUtils.release()
    }


    override fun surfaceChanged(holder: SurfaceHolder?, format: Int, width: Int, height: Int) {

    }

    override fun surfaceDestroyed(holder: SurfaceHolder?) {
        stopPush()
        release()
    }

    override fun surfaceCreated(holder: SurfaceHolder?) {
        mLiveVideoUtils.init()
    }


}