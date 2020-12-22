#include "arc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>


int should_parse_filtered_file(const char *filepath, int ext) {
    char tmp[PATH_MAX * 2];

    if (ext == 0) {
        return FALSE;
    }

    if (strncmp(filepath + ext, "tgz", 3) == 0) {
        return TRUE;
    }

    memcpy(tmp, filepath, ext - 1);
    *(tmp + ext - 1) = '\0';

    char *idx = strrchr(tmp, '.');

    if (idx == NULL) {
        return FALSE;
    }

    if (strcmp(idx, ".tar") == 0) {
        return TRUE;
    }

    return FALSE;
}

int arc_read(struct vfile *f, void *buf, size_t size) {
    size_t read = archive_read_data(f->arc, buf, size);

    if (read != size) {
        const char* error_str = archive_error_string(f->arc);
        if (error_str != NULL) {
            f->logf(f->filepath, LEVEL_ERROR, "Error reading archive file: %s", error_str);
        }
        return -1;
    }

    return read;
}

int arc_open(vfile_t *f, struct archive **a, arc_data_t *arc_data, int allow_recurse) {
    arc_data->f = f;

    if (f->is_fs_file) {
        *a = archive_read_new();
        archive_read_support_filter_all(*a);
        archive_read_support_format_all(*a);

       return archive_read_open_filename(*a, f->filepath, ARC_BUF_SIZE);
    } else if (allow_recurse) {
        *a = archive_read_new();
        archive_read_support_filter_all(*a);
        archive_read_support_format_all(*a);

        return archive_read_open(
                *a, arc_data,
                vfile_open_callback,
                vfile_read_callback,
                vfile_close_callback
        );
    } else {
        return ARC_SKIPPED;
    }
}

scan_code_t parse_archive(scan_arc_ctx_t *ctx, vfile_t *f, document_t *doc) {

    struct archive *a = NULL;
    struct archive_entry *entry = NULL;

    arc_data_t arc_data;
    arc_data.f = f;

    int ret = arc_open(f, &a, &arc_data, ctx->mode == ARC_MODE_RECURSE);
    if (ret == ARC_SKIPPED) {
        return SCAN_OK;
    }

    if (ret != ARCHIVE_OK) {
        CTX_LOG_ERRORF(f->filepath, "(arc.c) [%d] %s", ret, archive_error_string(a))
        archive_read_free(a);
        return SCAN_ERR_READ;
    }

    if (ctx->mode == ARC_MODE_LIST) {
        dyn_buffer_t buf = dyn_buffer_create();

        while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
            if (S_ISREG(archive_entry_stat(entry)->st_mode)) {
                const char* utf8_name = archive_entry_pathname_utf8(entry);
                const char* file_path = utf8_name == NULL ? archive_entry_pathname(entry) : utf8_name;

                dyn_buffer_append_string(&buf, file_path);
                dyn_buffer_write_char(&buf, ' ');
            }
        }
        dyn_buffer_write_char(&buf, '\0');

        meta_line_t *meta_list = malloc(sizeof(meta_line_t) + buf.cur);
        meta_list->key = MetaContent;
        strcpy(meta_list->str_val, buf.buf);
        APPEND_META(doc, meta_list)
        dyn_buffer_destroy(&buf);

    } else {

        parse_job_t *sub_job = malloc(sizeof(parse_job_t) + PATH_MAX * 2);

        sub_job->vfile.close = NULL;
        sub_job->vfile.read = arc_read;
        sub_job->vfile.reset = NULL;
        sub_job->vfile.arc = a;
        sub_job->vfile.filepath = sub_job->filepath;
        sub_job->vfile.is_fs_file = FALSE;
        sub_job->vfile.log = ctx->log;
        sub_job->vfile.logf = ctx->logf;
        memcpy(sub_job->parent, doc->uuid, sizeof(uuid_t));

        while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
            sub_job->vfile.info = *archive_entry_stat(entry);
            if (S_ISREG(sub_job->vfile.info.st_mode)) {

                const char* utf8_name = archive_entry_pathname_utf8(entry);

                if (utf8_name == NULL) {
                    sprintf(sub_job->filepath, "%s#/%s", f->filepath, archive_entry_pathname(entry));
                } else {
                    sprintf(sub_job->filepath, "%s#/%s", f->filepath, utf8_name);
                }
                sub_job->base = (int) (strrchr(sub_job->filepath, '/') - sub_job->filepath) + 1;

                char *p = strrchr(sub_job->filepath, '.');
                if (p != NULL) {
                    sub_job->ext = (int) (p - sub_job->filepath + 1);
                } else {
                    sub_job->ext = (int) strlen(sub_job->filepath);
                }

                ctx->parse(sub_job);
            }
        }

        free(sub_job);
    }

    archive_read_free(a);
    return SCAN_OK;
}
