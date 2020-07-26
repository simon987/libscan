#include "raw.h"
#include <libraw/libraw.h>

#include "../media/media.h"
#include <fcntl.h>
#include <unistd.h>


#define MIN_SIZE 32

int store_thumbnail_jpeg(scan_raw_ctx_t *ctx, libraw_processed_image_t *img, document_t *doc) {
    return store_image_thumbnail((scan_media_ctx_t*)ctx, img->data, img->data_size, doc, "x.jpeg");
}

int store_thumbnail_rgb24(scan_raw_ctx_t *ctx, libraw_processed_image_t *img, document_t *doc) {

    int dstW;
    int dstH;

    if (img->width <= ctx->tn_size && img->height <= ctx->tn_size) {
        dstW = img->width;
        dstH = img->height;
    } else {
        double ratio = (double) img->width / img->height;
        if (img->width > img->height) {
            dstW = ctx->tn_size;
            dstH = (int) (ctx->tn_size / ratio);
        } else {
            dstW = (int) (ctx->tn_size * ratio);
            dstH = ctx->tn_size;
        }
    }

    if (dstW <= MIN_SIZE || dstH <= MIN_SIZE) {
        return FALSE;
    }

    AVFrame *scaled_frame = av_frame_alloc();

    struct SwsContext *sws_ctx= sws_getContext(
            img->width, img->height, AV_PIX_FMT_RGB24,
            dstW, dstH, AV_PIX_FMT_YUVJ420P,
            SIST_SWS_ALGO, 0, 0, 0
    );

    int dst_buf_len = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, dstW, dstH, 1);
    uint8_t *dst_buf = (uint8_t *) av_malloc(dst_buf_len);

    av_image_fill_arrays(scaled_frame->data, scaled_frame->linesize, dst_buf, AV_PIX_FMT_YUV420P, dstW, dstH, 1);

    const uint8_t *in_data[1] = {img->data};
    int in_line_size[1] = {3 * img->width};

    sws_scale(sws_ctx,
              in_data, in_line_size,
              0, img->height,
              scaled_frame->data, scaled_frame->linesize
    );

    scaled_frame->width = dstW;
    scaled_frame->height = dstH;
    scaled_frame->format = AV_PIX_FMT_YUV420P;

    sws_freeContext(sws_ctx);

    AVCodecContext *jpeg_encoder = alloc_jpeg_encoder(scaled_frame->width, scaled_frame->height, 1.0f);
    avcodec_send_frame(jpeg_encoder, scaled_frame);

    AVPacket jpeg_packet;
    av_init_packet(&jpeg_packet);
    avcodec_receive_packet(jpeg_encoder, &jpeg_packet);

    APPEND_TN_META(doc, scaled_frame->width, scaled_frame->height)
    ctx->store((char *) doc->uuid, sizeof(doc->uuid), (char *) jpeg_packet.data, jpeg_packet.size);

    av_packet_unref(&jpeg_packet);
    av_free(*scaled_frame->data);
    av_frame_free(&scaled_frame);
    avcodec_free_context(&jpeg_encoder);

    return TRUE;
}

void parse_raw(scan_raw_ctx_t *ctx, vfile_t *f, document_t *doc) {
    libraw_data_t *libraw_lib = libraw_init(0);

    if (!libraw_lib) {
        CTX_LOG_ERROR("raw.c", "Cannot create libraw handle")
        return;
    }

    size_t buf_len = 0;
    void *buf = read_all(f, &buf_len);

    int ret = libraw_open_buffer(libraw_lib, buf, buf_len);
    if (ret != 0) {
        CTX_LOG_ERROR(f->filepath, "Could not open raw file")
        free(buf);
        return;
    }

    if (*libraw_lib->idata.model != '\0') {
        APPEND_STR_META(doc, MetaExifModel, libraw_lib->idata.model)
    }
    if (*libraw_lib->idata.make != '\0') {
        APPEND_STR_META(doc, MetaExifMake, libraw_lib->idata.make)
    }
    if (*libraw_lib->idata.software != '\0') {
        APPEND_STR_META(doc, MetaExifSoftware, libraw_lib->idata.software)
    }
    APPEND_INT_META(doc, MetaWidth, libraw_lib->sizes.width)
    APPEND_INT_META(doc, MetaHeight, libraw_lib->sizes.height)
    char tmp[1024];
    snprintf(tmp, sizeof(tmp), "%g", libraw_lib->other.iso_speed);
    APPEND_STR_META(doc, MetaExifIsoSpeedRatings, tmp)

    if (*libraw_lib->other.desc != '\0') {
        APPEND_STR_META(doc, MetaContent, libraw_lib->other.desc)
    }
    if (*libraw_lib->other.artist != '\0') {
        APPEND_STR_META(doc, MetaArtist, libraw_lib->other.artist)
    }

    struct tm *time = localtime(&libraw_lib->other.timestamp);
    strftime(tmp, sizeof(tmp), "%Y:%m:%d %H:%M:%S", time);
    APPEND_STR_META(doc, MetaExifDateTime, tmp)

    snprintf(tmp, sizeof(tmp), "%.1f", libraw_lib->other.focal_len);
    APPEND_STR_META(doc, MetaExifFocalLength, tmp)

    snprintf(tmp, sizeof(tmp), "%.1f", libraw_lib->other.aperture);
    APPEND_STR_META(doc, MetaExifFNumber, tmp)

    int denominator = (int)roundf(1 / libraw_lib->other.shutter);
    snprintf(tmp, sizeof(tmp), "1/%d", denominator);
    APPEND_STR_META(doc, MetaExifExposureTime, tmp)

    APPEND_STR_META(doc, MetaMediaVideoCodec, "raw")

    if (ctx->tn_size <= 0) {
        free(buf);
        libraw_close(libraw_lib);
        return;
    }

    libraw_unpack_thumb(libraw_lib);

    int errc = 0;
    libraw_processed_image_t *thumb = libraw_dcraw_make_mem_thumb(libraw_lib, &errc);
    if (errc != 0) {
        free(buf);
        libraw_dcraw_clear_mem(thumb);
        libraw_close(libraw_lib);
        return;
    }

    int tn_ok = 0;
    if (libraw_lib->thumbnail.tformat == LIBRAW_THUMBNAIL_JPEG) {
        tn_ok = store_thumbnail_jpeg(ctx, thumb, doc);
    } else if (libraw_lib->thumbnail.tformat == LIBRAW_THUMBNAIL_BITMAP) {
        // TODO: technically this should work but is currently untested
        tn_ok = store_thumbnail_rgb24(ctx, thumb, doc);
    }

    libraw_dcraw_clear_mem(thumb);

    if (tn_ok == TRUE) {
        free(buf);
        libraw_close(libraw_lib);
        return;
    }

    ret = libraw_unpack(libraw_lib);
    if (ret != 0) {
        CTX_LOG_ERROR(f->filepath, "Could not unpack raw file")
        free(buf);
        libraw_close(libraw_lib);
        return;
    }

    libraw_dcraw_process(libraw_lib);

    errc = 0;
    libraw_processed_image_t *img = libraw_dcraw_make_mem_image(libraw_lib, &errc);
    if (errc != 0) {
        free(buf);
        libraw_dcraw_clear_mem(img);
        libraw_close(libraw_lib);
        return;
    }

    store_thumbnail_rgb24(ctx, img, doc);

    libraw_dcraw_clear_mem(img);
    libraw_close(libraw_lib);

    free(buf);
}
