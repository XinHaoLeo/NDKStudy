package com.xin.ndkstudy.activity

import android.Manifest
import android.content.Intent
import android.content.pm.PackageManager
import android.hardware.Camera
import android.os.Build
import android.os.Bundle
import android.util.Log
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import com.xin.ndkstudy.R
import com.xin.ndkstudy.ext.setOnNoRepeatClickListener
import com.xin.ndkstudy.live.pusher.LivePusher
import kotlinx.android.synthetic.main.activity_live.*


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
 *@date : 2020/12/2 13:40
 *@since :
 *@desc :
 */
class LiveActivity : AppCompatActivity() {

    private lateinit var mLivePusher: LivePusher

    companion object {
        private const val LIVE_URL = "rtmp://81.69.1.238/live/coolxin"
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_live)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (ContextCompat.checkSelfPermission(
                    this, Manifest.permission.WRITE_EXTERNAL_STORAGE
                ) == PackageManager.PERMISSION_DENIED && ContextCompat.checkSelfPermission(
                    this, Manifest.permission.CAMERA
                ) == PackageManager.PERMISSION_DENIED && ContextCompat.checkSelfPermission(
                    this, Manifest.permission.RECORD_AUDIO
                ) == PackageManager.PERMISSION_DENIED
            ) {
                //为了简单方便就不写拒绝的回调,默认用户同意全部权限
                requestPermissions(
                    arrayOf(
                        Manifest.permission.WRITE_EXTERNAL_STORAGE,
                        Manifest.permission.CAMERA,
                        Manifest.permission.RECORD_AUDIO
                    ), 99
                )
            } else {
                mLivePusher = LivePusher(640, 380, surface.holder)
                btn_push.setOnCheckedChangeListener { _, isChecked ->
                    if (isChecked) {
                        mLivePusher.startPush(LIVE_URL)
                    } else {
                        mLivePusher.stopPush()
                    }
                }
                btn_camera_switch.setOnNoRepeatClickListener {
                    Log.d("Leo", "cameraSize: " + Camera.getNumberOfCameras())
                    if (Camera.getNumberOfCameras() > 2) {
                        mLivePusher.switchCamera()
                    } else {
                        Toast.makeText(this@LiveActivity, "当前无更多摄像头可切换", Toast.LENGTH_SHORT).show()
                    }
                }
            }
        }
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
    }
}