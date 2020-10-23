package com.xin.ndkstudy.activity

import android.os.Bundle
import android.os.Environment
import androidx.appcompat.app.AppCompatActivity
import com.xin.ndkstudy.R
import com.xin.ndkstudy.VideoUtils
import kotlinx.android.synthetic.main.activity_video_player.*
import java.io.File

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
 *@date : 2020/10/20 19:03
 *@since : xinxiniscool@gmail.com
 *@desc :
 */
class PlayerActivity : AppCompatActivity() {

    private val mVideoUtils by lazy { VideoUtils.getInstance() }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_video_player)
        mVideoUtils.init()
        btStart.setOnCheckedChangeListener { _, isChecked ->
            if (isChecked) {
                val sdcardPath = Environment.getExternalStorageDirectory().absolutePath
                val inputPath = File("$sdcardPath/ffmpeg.mp4").absolutePath
                mVideoUtils.play(inputPath, videoView.holder.surface)
            } else {
                finish()
            }
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        mVideoUtils.release()
    }
}