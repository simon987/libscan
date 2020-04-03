#include "cbr.h"
#include "../scan.h"
#include "../util.h"
#include "../arc/arc.h"

#include <stdlib.h>
#include <archive.h>


void parse_cbr(scan_cbr_ctx_t *ctx, vfile_t *f, document_t *doc) {

    size_t buf_len;
    void *buf = read_all(f, &buf_len);

    char *out_buf = malloc(buf_len * 2); // TODO: we probably only need 1.2x or 1.5x, even better would be a dynamic buffer
    size_t out_buf_used = 0;

    struct archive *rar_in = archive_read_new();
    archive_read_support_filter_none(rar_in);
    archive_read_support_format_rar(rar_in);

    archive_read_open_memory(rar_in, buf, buf_len);

    struct archive *zip_out = archive_write_new();
    archive_write_set_format_zip(zip_out);
    archive_write_open_memory(zip_out, out_buf, buf_len * 2, &out_buf_used);

    struct archive_entry *entry;
    while (archive_read_next_header(rar_in, &entry) == ARCHIVE_OK) {
        archive_write_header(zip_out, entry);

        char arc_buf[ARC_BUF_SIZE];
        int len = archive_read_data(rar_in, arc_buf, ARC_BUF_SIZE);
        while (len > 0) {
            archive_write_data(zip_out, arc_buf, len);
            len = archive_read_data(rar_in, arc_buf, ARC_BUF_SIZE);
        }
    }

    archive_write_close(zip_out);
    archive_write_free(zip_out);

    archive_read_close(rar_in);
    archive_read_free(rar_in);

    parse_ebook_mem(&ctx->ebook_ctx, out_buf, out_buf_used, "application/x-cbz", doc);
    doc->mime = ctx->cbr_mime;
    free(out_buf);
}
