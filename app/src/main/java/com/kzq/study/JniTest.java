package com.kzq.study;

/**
 * created by kzq at 2018/5/25
 **/
public class JniTest {

    public static native String mp4toyuv(String path);

    static {
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("avformat-57");
        System.loadLibrary("avutil-55");
        System.loadLibrary("swresample-2");
        System.loadLibrary("swscale-4");
        System.loadLibrary("ffmpegdemo");
    }
}
