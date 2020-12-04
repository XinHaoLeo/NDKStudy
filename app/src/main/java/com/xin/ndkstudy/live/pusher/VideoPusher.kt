package com.xin.ndkstudy.live.pusher

import android.graphics.ImageFormat
import android.hardware.Camera
import android.util.Log
import android.view.SurfaceHolder
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
 *@date : 2020/12/2 14:17
 *@since :
 *@desc :
 */
class VideoPusher(private var surfaceHolder: SurfaceHolder, private var videoParams: VideoParam) :
    Pusher, SurfaceHolder.Callback, Camera.PreviewCallback {
    private val mLivePushUtils by lazy { LivePushUtils.getInstance() }
    private var mCamera: Camera? = null
    private var buffers: ByteArray? = null
    private var isPushing = false
    private var type = 0

    init {
        surfaceHolder.addCallback(this)
    }

    override fun startPush() {
        mLivePushUtils.setVideoOption(
            videoParams.getWidth(),
            videoParams.getHeight(),
            videoParams.getBitrate(),
            videoParams.getFps()
        )
        isPushing = true
    }

    override fun stopPush() {
        isPushing = false
    }

    override fun release() {
        stopPreview()
    }

    override fun surfaceChanged(holder: SurfaceHolder?, format: Int, width: Int, height: Int) {

    }

    override fun surfaceDestroyed(holder: SurfaceHolder?) {

    }

    override fun surfaceCreated(holder: SurfaceHolder?) {
        startPreview()
    }

    fun switchCamera() {
        if (videoParams.getCameraId() == Camera.CameraInfo.CAMERA_FACING_BACK) {
            videoParams.setCameraId(Camera.CameraInfo.CAMERA_FACING_FRONT)
        } else {
            videoParams.setCameraId(Camera.CameraInfo.CAMERA_FACING_BACK)
        }
        //重新预览
        stopPreview()
        startPreview()
    }

    private fun startPreview() {
        try {
            //SurfaceView初始化完成，开始相机预览
            mCamera = Camera.open(videoParams.getCameraId())
            val parameters = mCamera?.parameters
            //设置相机参数
            parameters?.previewFormat = ImageFormat.NV21 //YUV 预览图像的像素格式
            parameters?.setPreviewSize(videoParams.getWidth(), videoParams.getHeight()) //预览画面宽高
            mCamera?.parameters = parameters
            mCamera?.setPreviewDisplay(surfaceHolder)
            //获取预览图像数据
            buffers = ByteArray(videoParams.getWidth() * videoParams.getHeight() * 4)
            mCamera?.addCallbackBuffer(buffers)
            mCamera?.setPreviewCallbackWithBuffer(this)
            mCamera?.startPreview()
        } catch (e: Exception) {
            when (type) {
                0 -> {
                    videoParams.setWidth(1280)
                    videoParams.setHeight(720)
                    type++
                    startPreview()
                }
                1->{
                    videoParams.setWidth(1080)
                    videoParams.setHeight(1920)
                    type++
                    startPreview()
                }
            }
            e.printStackTrace()
        }
    }

    private fun stopPreview() {
        mCamera?.stopPreview()
        mCamera?.release()
        mCamera = null
    }

    override fun onPreviewFrame(data: ByteArray?, camera: Camera?) {
        mCamera?.addCallbackBuffer(buffers)
        Log.i("Leo", "onPreviewFrame: $data")
        if (isPushing) {
            //回调函数中获取图像数据，然后给Native代码编码
            mLivePushUtils.fireVideo(data)
        }
    }

}