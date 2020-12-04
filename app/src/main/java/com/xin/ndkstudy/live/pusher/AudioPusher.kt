package com.xin.ndkstudy.live.pusher

import android.media.AudioFormat
import android.media.AudioRecord
import android.media.MediaRecorder.AudioSource
import com.xin.ndkstudy.live.parm.AudioParam
import com.xin.ndkstudy.util.LivePushUtils
import com.xin.ndkstudy.util.ThreadUtils


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
 *@date : 2020/12/2 14:37
 *@since :
 *@desc :
 */
class AudioPusher(private var audioParam: AudioParam) : Pusher {

    private val mLivePushUtils by lazy { LivePushUtils.getInstance() }
    private var audioRecord: AudioRecord? = null
    private var isPushing = false
    private var minBufferSize = 0

    init {
        val channelConfig: Int =
            if (audioParam.getChannel() == 1) AudioFormat.CHANNEL_IN_MONO else AudioFormat.CHANNEL_IN_STEREO
        //最小缓冲区大小
        minBufferSize = AudioRecord.getMinBufferSize(
            audioParam.getSampleRateInHz(),
            channelConfig,
            AudioFormat.ENCODING_PCM_16BIT
        )
        audioRecord = AudioRecord(
            AudioSource.MIC,
            audioParam.getSampleRateInHz(),
            channelConfig,
            AudioFormat.ENCODING_PCM_16BIT, minBufferSize
        )

    }

    override fun startPush() {
        isPushing = true
        mLivePushUtils.setAudioOptions(audioParam.getSampleRateInHz(), audioParam.getChannel())
        //启动一个录音子线程
        ThreadUtils.getInstance().newFixedThreadPool().execute {
            //开始录音
            audioRecord?.startRecording()
            while (isPushing) {
                //通过AudioRecord不断读取音频数据
                val buffer = ByteArray(minBufferSize)
                audioRecord?.read(buffer, 0, buffer.size)?.let {
                    if (it > 0) {
                        //传给Native代码，进行音频编码
                        mLivePushUtils.fireAudio(buffer, it)
                    }
                }
            }
        }
    }

    override fun stopPush() {
        isPushing = false
        audioRecord?.stop()
    }

    override fun release() {
        audioRecord?.release()
    }

}