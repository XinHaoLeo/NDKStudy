package com.xin.ndkstudy.util;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

/**
 * @author : Leo
 * @date : 2019/4/19 14:35
 * @desc : 线程池工具类
 * @since : lightingxin@qq.com
 */
public class ThreadUtils {

    private static final ThreadUtils ourInstance = new ThreadUtils();

    public static synchronized ThreadUtils getInstance() {
        return ourInstance;
    }

    private ThreadUtils() {
    }

    /**
     * @return FixThreadPool
     */
    public ExecutorService newFixedThreadPool() {
        return new ThreadPoolExecutor(3, 10,
                1, TimeUnit.MINUTES,
                new LinkedBlockingQueue<Runnable>());
    }
}
