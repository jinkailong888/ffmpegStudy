
#include<com_kzq_study_JniTest.h>
#include <malloc.h>
#include <string.h>
#include <libavformat/avformat.h>
#include <android/log.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

/**
 * jstring转constr char* 辅助函数
 * @param env
 * @param jstr
 * @return
 */
char *Jstring2CStr(JNIEnv *env, jstring jstr) {
    char *rtn = NULL;
    jclass clsstring = (*env)->FindClass(env, "java/lang/String");
    jstring strencode = (*env)->NewStringUTF(env, "GB2312");
    jmethodID mid = (*env)->GetMethodID(env, clsstring, "getBytes", "(Ljava/lang/String;)[B");
    jbyteArray barr = (jbyteArray) (*env)->CallObjectMethod(env, jstr, mid, strencode);
    jsize alen = (*env)->GetArrayLength(env, barr);
    jbyte *ba = (*env)->GetByteArrayElements(env, barr, JNI_FALSE);
    if (alen > 0) {
        rtn = (char *) malloc((size_t) (alen + 1)); //new char[alen+1];
        memcpy(rtn, ba, (size_t) alen);
        rtn[alen] = 0;
    }
    (*env)->ReleaseByteArrayElements(env, barr, ba, 0);

    return rtn;
}

/**
 * MP4解码为YUV数据
 * @param env
 * @param cls
 * @param file
 * @return
 */

JNIEXPORT jstring JNICALL Java_com_kzq_study_JniTest_mp4toyuv
        (JNIEnv *env, jclass cls, jstring parent, jstring file) {

    int videoindex, i;
    AVFormatContext *pFormatCtx;
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;

    AVFrame *pFrame, *pFrameYUV;
    uint8_t *out_buffer;
    AVPacket *packet;
    int y_size;
    int ret, got_picture;
    struct SwsContext *img_convert_ctx;

    const char *cparent = Jstring2CStr(env, parent);

    char *cfile = Jstring2CStr(env, file);

    const char *cpath = strcat(cfile, cparent);

    char *cyuv = strncpy(cyuv, cpath, strlen(cpath) - strlen(".apk"));

    __android_log_print(ANDROID_LOG_DEBUG, "MP4toYUV",
                        "parent path: %s, mp4 path: %s, yuv path: %s\n", cparent, cpath,
                        strcat(cyuv, ".yuv"));

    FILE *fp_yuv = fopen(strcat(cyuv, ".yuv"), "wb+");

    av_register_all();
    avformat_network_init();
    pFormatCtx = avformat_alloc_context();

    if ((avformat_open_input(&pFormatCtx, cpath, NULL, NULL)) < 0) {
        __android_log_print(ANDROID_LOG_DEBUG, "MP4toYUV", "Couldn't open input stream.\n");
        return (*env)->NewStringUTF(env, "Couldn't open input stream.\n");
    }

    if ((avformat_find_stream_info(pFormatCtx, NULL)) < 0) {
        __android_log_print(ANDROID_LOG_DEBUG, "MP4toYUV", "Couldn't find stream info.\n");
        return (*env)->NewStringUTF(env, "Couldn't find stream info.\n");
    }

    videoindex = -1;
    for (i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoindex = i;
            break;
        }
    }
    __android_log_print(ANDROID_LOG_DEBUG, "MP4toYUV", "video stream index %d.\n", videoindex);


    if (videoindex == -1) {
        __android_log_print(ANDROID_LOG_DEBUG, "MP4toYUV", "Couldn't find video stream.\n");
        return (*env)->NewStringUTF(env, "Couldn't find video stream.\n");
    }

    pCodecCtx = pFormatCtx->streams[videoindex]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL) {
        __android_log_print(ANDROID_LOG_DEBUG, "MP4toYUV", "Couldn't find decoder.\n");
        return (*env)->NewStringUTF(env, "Couldn't find decoder.\n");
    }

    if ((avcodec_open2(pCodecCtx, pCodec, NULL)) < 0) {
        __android_log_print(ANDROID_LOG_DEBUG, "MP4toYUV", "Couldn't open codec.\n");
        return (*env)->NewStringUTF(env, "Couldn't open codec.\n");
    }

    pFrame = av_frame_alloc();
    pFrameYUV = av_frame_alloc();
    out_buffer = (uint8_t *) av_malloc(
            (size_t) av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width,
                                              pCodecCtx->height, 1));
    av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer,
                         AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);
    packet = (AVPacket *) av_malloc(sizeof(AVPacket));
    //Output Info-----------------------------
    __android_log_print(ANDROID_LOG_DEBUG, "MP4toYUV",
                        "--------------- File Information ----------------\n");
    av_dump_format(pFormatCtx, 0, cpath, 0);
    __android_log_print(ANDROID_LOG_DEBUG, "MP4toYUV",
                        "-------------------------------------------------\n");
    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                     pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P,
                                     SWS_BICUBIC, NULL, NULL, NULL);

    while ((av_read_frame(pFormatCtx, packet)) >= 0) {
        if (packet->stream_index == videoindex) {

            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);//解码一帧压缩数据
            if (ret < 0) {
                __android_log_print(ANDROID_LOG_DEBUG, "MP4toYUV", "Decode Error.\n");
                return (*env)->NewStringUTF(env, "Decode Error.\n");
            }
            if (got_picture) {
                sws_scale(img_convert_ctx, (const uint8_t *const *) pFrame->data, pFrame->linesize,
                          0, pCodecCtx->height,
                          pFrameYUV->data, pFrameYUV->linesize);

                y_size = pCodecCtx->width * pCodecCtx->height;
                fwrite(pFrameYUV->data[0], 1, (size_t) y_size, fp_yuv);    //Y
                fwrite(pFrameYUV->data[1], 1, (size_t) (y_size / 4), fp_yuv);  //U
                fwrite(pFrameYUV->data[2], 1, (size_t) (y_size / 4), fp_yuv);  //V
                __android_log_print(ANDROID_LOG_DEBUG, "MP4toYUV", "Succeed to decode 1 frame!\n");
            }
        }
        av_packet_unref(packet);
    }

    while (1) {
        ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
        if (ret < 0)
            break;
        if (!got_picture)
            break;
        sws_scale(img_convert_ctx, (const uint8_t *const *) pFrame->data, pFrame->linesize, 0,
                  pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);

        y_size = pCodecCtx->width * pCodecCtx->height;
        fwrite(pFrameYUV->data[0], 1, (size_t) y_size, fp_yuv);    //Y
        fwrite(pFrameYUV->data[1], 1, (size_t) (y_size / 4), fp_yuv);  //U
        fwrite(pFrameYUV->data[2], 1, (size_t) (y_size / 4), fp_yuv);  //V

        __android_log_print(ANDROID_LOG_DEBUG, "MP4toYUV", "Succeed to decode 1 frame!\n");
    }

    sws_freeContext(img_convert_ctx);

    fclose(fp_yuv);

    av_frame_free(&pFrameYUV);
    av_frame_free(&pFrame);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);

    __android_log_print(ANDROID_LOG_DEBUG, "MP4toYUV", "mp4 to yuv ok.\n");

    return (*env)->NewStringUTF(env, "mp4 to yuv ok.\n");

}
