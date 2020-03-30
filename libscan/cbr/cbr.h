#ifndef SCAN_CBR_H
#define SCAN_CBR_H

#include <stdlib.h>
#include "../scan.h"

typedef struct {
    log_callback_t log;
    logf_callback_t logf;
    store_callback_t store;
} scan_cbr_ctx_t;

void cbr_init();

int is_cbr(unsigned int mime);

void parse_cbr(scan_cbr_ctx_t *ctx, vfile_t *f, document_t *doc);

#endif
