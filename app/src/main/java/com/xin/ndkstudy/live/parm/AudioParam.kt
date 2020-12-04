package com.xin.ndkstudy.live.parm

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
 *@date : 2020/12/2 14:08
 *@since :
 *@desc :
 */
class AudioParam() {

    // 采样率
    private var sampleRateInHz = 44100

    // 声道个数
    private var channel = 1

    constructor(sampleRateInHz: Int, channel: Int) : this() {
        this.sampleRateInHz = sampleRateInHz
        this.channel = channel
    }

    fun getSampleRateInHz(): Int {
        return sampleRateInHz
    }

    fun setSampleRateInHz(sampleRateInHz: Int) {
        this.sampleRateInHz = sampleRateInHz
    }

    fun getChannel(): Int {
        return channel
    }

    fun setChannel(channel: Int) {
        this.channel = channel
    }
}