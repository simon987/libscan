#ifndef SIST2_MEDIA_H
#define SIST2_MEDIA_H


#include "../scan.h"

typedef struct {
    log_callback_t log;
    logf_callback_t logf;
    store_callback_t store;

    int tn_size;
    float tn_qscale;
    long max_media_buffer;
} scan_media_ctx_t;

void parse_media(scan_media_ctx_t *ctx, vfile_t *f, document_t *doc);
void init_media();

int store_image_thumbnail(scan_media_ctx_t *ctx, void* buf, size_t buf_len, document_t *doc);

#endif
