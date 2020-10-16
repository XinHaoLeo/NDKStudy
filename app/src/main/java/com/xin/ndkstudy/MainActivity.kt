package com.xin.ndkstudy

import android.Manifest
import android.os.Build
import android.os.Bundle
import android.os.Environment
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import kotlinx.android.synthetic.main.activity_main.*
import java.io.File


class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            //为了简单方便就不写拒绝的回调,默认用户同意
            requestPermissions(arrayOf(Manifest.permission.WRITE_EXTERNAL_STORAGE), 99)
        }

        val contentAppend = TestUtils.getInstance().contentAppend("test", 1)
        sample_text.text = contentAppend
        Log.d("Leo", "text =$contentAppend")


        val picturesPath = Environment.getExternalStorageDirectory().absolutePath
        btEncrypt.setOnClickListener {
            val normalPath = File("$picturesPath/girl.jpg").absolutePath
            val encryptPath = File("$picturesPath/girlEncrypt.jpg").absolutePath
            Log.d("Leo", "normalPath:$normalPath")
            FileUtils.getInstance().encrypt(normalPath, encryptPath)
        }
        btDecrypt.setOnClickListener {
            val encryptPath = File("$picturesPath/girlEncrypt.jpg").absolutePath
            val decryptPath = File("$picturesPath/girlDecrypt.jpg").absolutePath
            FileUtils.getInstance().decrypt(encryptPath, decryptPath)
        }
    }

}
