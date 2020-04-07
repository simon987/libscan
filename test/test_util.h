#ifndef SCAN_TEST_UTIL_H
#define SCAN_TEST_UTIL_H

#include "../libscan/scan.h"

void load_file(const char *filepath, vfile_t *f);
void load_mem(void *mem, size_t size, vfile_t *f);

static void noop_logf(char *filepath, int level, char *format, ...) {
    // noop
}

static void noop_log(char *filepath, int level, char *str) {
    // noop
}

static void noop_store(char* key, size_t key_len, char *value, size_t value_len) {
    // noop
}

meta_line_t *get_meta(document_t *doc, metakey key);


#define CLOSE_FILE(f) if (f.close != NULL) {f.close(&f);};

void destroy_doc(document_t *doc);

#endif
