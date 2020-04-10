#include "text.h"

scan_code_t parse_text(scan_text_ctx_t *ctx, vfile_t *f, document_t *doc) {

    int to_read = MIN(ctx->content_size, f->info.st_size);

    char *buf = malloc(to_read);
    int ret = f->read(f, buf, to_read);
    if (ret < 0) {
        CTX_LOG_ERRORF(doc->filepath, "read() returned error code: [%d]", ret)
        free(buf);
        return SCAN_ERR_READ;
    }

    text_buffer_t tex = text_buffer_create(ctx->content_size);
    text_buffer_append_string(&tex, buf, to_read);
    text_buffer_terminate_string(&tex);

    APPEND_STR_META(doc, MetaContent, tex.dyn_buffer.buf);

    free(buf);
    text_buffer_destroy(&tex);

    return SCAN_OK;
}

#define MAX_MARKUP_SIZE 1024 * 1024

scan_code_t parse_markup(scan_text_ctx_t *ctx, vfile_t *f, document_t *doc) {

    int to_read = MIN(MAX_MARKUP_SIZE, f->info.st_size);

    char *buf = malloc(to_read);
    int ret = f->read(f, buf, to_read);
    if (ret < 0) {
        CTX_LOG_ERRORF(doc->filepath, "read() returned error code: [%d]", ret)
        free(buf);
        return SCAN_ERR_READ;
    }

    text_buffer_t tex = text_buffer_create(ctx->content_size);
    text_buffer_append_markup(&tex, buf);
    text_buffer_terminate_string(&tex);

    APPEND_STR_META(doc, MetaContent, tex.dyn_buffer.buf);

    free(buf);
    text_buffer_destroy(&tex);

    return SCAN_OK;
}
