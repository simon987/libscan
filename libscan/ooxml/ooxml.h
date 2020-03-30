#ifndef SCAN_OOXML_H
#define SCAN_OOXML_H

#include <stdlib.h>
#include "../scan.h"

typedef struct {
    long content_size;
    log_callback_t log;
    logf_callback_t logf;
    store_callback_t store;
} scan_ooxml_cxt_t;

void parse_doc(scan_ooxml_cxt_t *ctx, vfile_t *f, document_t *doc);

#endif
