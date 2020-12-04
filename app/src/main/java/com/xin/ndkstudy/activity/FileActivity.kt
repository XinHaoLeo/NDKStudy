package com.xin.ndkstudy.activity

import android.Manifest
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.os.Environment
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import com.xin.ndkstudy.util.FileUtils
import com.xin.ndkstudy.R
import com.xin.ndkstudy.ext.setOnNoRepeatClickListener
import kotlinx.android.synthetic.main.activity_file.*
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
 *@date : 2020/10/20 18:47
 *@since : xinxiniscool@gmail.com
 *@desc :
 */
class FileActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_file)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M
            && ContextCompat.checkSelfPermission(
                this, Manifest.permission.WRITE_EXTERNAL_STORAGE
            ) == PackageManager.PERMISSION_DENIED
        ) {
            //为了简单方便就不写拒绝的回调,默认用户同意
            requestPermissions(arrayOf(Manifest.permission.WRITE_EXTERNAL_STORAGE), 99)
        }
        val sdcardPath = Environment.getExternalStorageDirectory().absolutePath
        btEncrypt.setOnNoRepeatClickListener {
            val normalPath = File("$sdcardPath/girl.jpg").absolutePath
            val encryptPath = File("$sdcardPath/girlEncrypt.jpg").absolutePath
            Log.d("Leo", "normalPath:$normalPath")
            FileUtils.getInstance()
                .encrypt(normalPath, encryptPath)
        }
        btDecrypt.setOnNoRepeatClickListener {
            val encryptPath = File("$sdcardPath/girlEncrypt.jpg").absolutePath
            val decryptPath = File("$sdcardPath/girlDecrypt.jpg").absolutePath
            FileUtils.getInstance()
                .decrypt(encryptPath, decryptPath)
        }
    }
}