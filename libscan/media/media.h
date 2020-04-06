#ifndef SIST2_MEDIA_H
#define SIST2_MEDIA_H


#include "../scan.h"

typedef struct {
    long content_size;
    int tn_size;
    float tn_qscale;

    log_callback_t log;
    logf_callback_t logf;
    store_callback_t store;
} scan_media_ctx_t;

void parse_media(scan_media_ctx_t *ctx, vfile_t *f, document_t *doc);
void init_media();

#endif
