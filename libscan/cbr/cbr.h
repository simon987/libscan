#ifndef SCAN_CBR_H
#define SCAN_CBR_H

#include <stdlib.h>
#include "../scan.h"

typedef struct {
    scan_ebook_ctx_t ebook_ctx;
    unsigned int cbr_mime;
    log_callback_t log;
    logf_callback_t logf;
    store_callback_t store;
} scan_cbr_ctx_t;

__always_inline
static int is_cbr(scan_cbr_ctx_t *ctx, unsigned int mime) {
    return mime == ctx->cbr_mime;
}

void parse_cbr(scan_cbr_ctx_t *ctx, vfile_t *f, document_t *doc);

#endif
