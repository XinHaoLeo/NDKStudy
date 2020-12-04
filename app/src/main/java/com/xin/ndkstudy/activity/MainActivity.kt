package com.xin.ndkstudy.activity

import android.os.Bundle
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import com.xin.ndkstudy.*
import com.xin.ndkstudy.ext.setOnNoRepeatClickListener
import com.xin.ndkstudy.ext.startActivity
import com.xin.ndkstudy.util.TestUtils
import kotlinx.android.synthetic.main.activity_main.*


class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val contentAppend = TestUtils.getInstance()
            .contentAppend("test", 1)
        sample_text.text = contentAppend
        Log.d("Leo", "text =$contentAppend")

        btJumpFile.setOnNoRepeatClickListener {
            startActivity<FileActivity> {  }
        }

        btJumpPlayer.setOnNoRepeatClickListener {
            startActivity<PlayerActivity> {  }
        }

        btJumpLive.setOnNoRepeatClickListener {
            startActivity<LiveActivity> {  }
        }
    }

}
