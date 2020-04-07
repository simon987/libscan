#include "test_util.h"
#include <gtest/gtest.h>

#include <unistd.h>
#include <fcntl.h>

#define FILE_NOT_FOUND_ERR "Could not file, did you clone the test files repo?"


int fs_read(struct vfile *f, void *buf, size_t size) {

    if (f->fd == -1) {
        f->fd = open(f->filepath, O_RDONLY);
        if (f->fd == -1) {
            return -1;
        }
    }

    return read(f->fd, buf, size);
}

//Note: No out of bounds check
int mem_read(vfile_t *f, void *buf, size_t size) {
    memcpy(buf, f->_test_data, size);
    f->_test_data = (char *) f->_test_data + size;
    return 0;
}

void fs_close(vfile_t *f) {
    if (f->fd != -1) {
        close(f->fd);
    }
}

void load_doc_file(const char *filepath, vfile_t *f, document_t *doc) {
    doc->meta_head = nullptr;
    doc->meta_tail = nullptr;
    load_file(filepath, f);
}

void load_doc_mem(void *mem, size_t mem_len, vfile_t *f, document_t *doc) {
    doc->meta_head = nullptr;
    doc->meta_tail = nullptr;
    load_mem(mem, mem_len, f);
}

void cleanup(document_t *doc, vfile_t *f) {
    destroy_doc(doc);
    CLOSE_FILE((*f))
}

void load_file(const char *filepath, vfile_t *f) {
    stat(filepath, &f->info);
    f->fd = open(filepath, O_RDONLY);

    if (f->fd == -1) {
        FAIL() << FILE_NOT_FOUND_ERR;
    }

    f->filepath = filepath;
    f->read = fs_read;
    f->close = fs_close;
    f->is_fs_file = TRUE;
}

void load_mem(void *mem, size_t size, vfile_t *f) {
    f->filepath = "_mem_";
    f->_test_data = mem;
    f->info.st_size = size;
    f->read = mem_read;
    f->close = nullptr;
    f->is_fs_file = TRUE;
}

meta_line_t *get_meta(document_t *doc, metakey key) {
    meta_line_t *meta = doc->meta_head;
    while (meta != nullptr) {
        if (meta->key == key) {
            return meta;
        }
        meta = meta->next;
    }
    return nullptr;
}

void destroy_doc(document_t *doc) {
    meta_line_t *meta = doc->meta_head;
    while (meta != nullptr) {
        meta_line_t *tmp = meta;
        meta = tmp->next;
        free(tmp);
    }
}
