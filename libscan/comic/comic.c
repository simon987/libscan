#include "comic.h"
#include "../media/media.h"
#include "../arc/arc.h"

#include <stdlib.h>
#include <archive.h>


void parse_comic(scan_comic_ctx_t *ctx, vfile_t *f, document_t *doc) {
    struct archive *a = NULL;
    struct archive_entry *entry = NULL;
    arc_data_t arc_data;

    int ret = arc_open(f, &a, &arc_data, TRUE);
    if (ret != ARCHIVE_OK) {
        CTX_LOG_ERRORF(f->filepath, "(cbr.c) [%d] %s", ret, archive_error_string(a))
        archive_read_free(a);
        return;
    }

    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        struct stat info = *archive_entry_stat(entry);
        if (S_ISREG(info.st_mode)) {
            const char* utf8_name = archive_entry_pathname_utf8(entry);
            const char* file_path = utf8_name == NULL ? archive_entry_pathname(entry) : utf8_name;

            char *p = strrchr(file_path, '.');
            if (p != NULL && strcmp(p, ".png") == 0 || strcmp(p, ".jpg") == 0 || strcmp(p, ".jpeg") == 0) {
                size_t entry_size = archive_entry_size(entry);
                void* buf = malloc(entry_size);
                archive_read_data(a, buf, entry_size);

                ret = store_image_thumbnail((scan_media_ctx_t*)ctx, buf, entry_size, doc, file_path);
                free(buf);

                if (ret == TRUE) {
                    break;
                }
            }
        }
    }

    archive_read_free(a);
}
