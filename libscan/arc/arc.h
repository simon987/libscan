#ifndef SCAN_ARC_H
#define SCAN_ARC_H

#include <archive.h>
#include <archive_entry.h>
#include <fcntl.h>
#include "../scan.h"

# define ARC_SKIPPED (-1)
#define ARC_MODE_SKIP 0
#define ARC_MODE_LIST 1
#define ARC_MODE_SHALLOW 2
#define ARC_MODE_RECURSE 3
typedef int archive_mode_t;

typedef struct {
    archive_mode_t mode;

    parse_callback_t parse;
    log_callback_t log;
    logf_callback_t logf;
    store_callback_t store;
    char passphrase[4096];
} scan_arc_ctx_t;

#define ARC_BUF_SIZE 8192

typedef struct {
    vfile_t *f;
    char buf[ARC_BUF_SIZE];
} arc_data_t;

static int vfile_open_callback(struct archive *a, void *user_data) {
    arc_data_t *data = (arc_data_t *) user_data;

    if (!data->f->is_fs_file) {
        SHA1_Init(&data->f->sha1_ctx);
    }

    return ARCHIVE_OK;
}

static long vfile_read_callback(struct archive *a, void *user_data, const void **buf) {
    arc_data_t *data = (arc_data_t *) user_data;

    *buf = data->buf;
    long ret = data->f->read(data->f, data->buf, sizeof(data->buf));

    if (!data->f->is_fs_file && ret > 0) {
        data->f->has_checksum = TRUE;
        safe_sha1_update(&data->f->sha1_ctx, (unsigned char*)data->buf, ret);
    }

    return ret;
}

static int vfile_close_callback(struct archive *a, void *user_data) {
    arc_data_t *data = (arc_data_t *) user_data;

    if (!data->f->is_fs_file) {
        SHA1_Final((unsigned char *) data->f->sha1_digest, &data->f->sha1_ctx);
    }

    return ARCHIVE_OK;
}

int arc_open(scan_arc_ctx_t *ctx, vfile_t *f, struct archive **a, arc_data_t *arc_data, int allow_recurse);

int should_parse_filtered_file(const char *filepath, int ext);

scan_code_t parse_archive(scan_arc_ctx_t *ctx, vfile_t *f, document_t *doc);

int arc_read(struct vfile *f, void *buf, size_t size);

int arc_read_rewindable(struct vfile *f, void *buf, size_t size);

void arc_close(struct vfile *f);

#endif
