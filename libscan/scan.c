#include "scan.h"

#include <fcntl.h>
#include <unistd.h>

int fs_read(struct vfile *f, void *buf, size_t size) {
    if (f->fd == -1) {
        f->fd = open(f->filepath, O_RDONLY);
        if (f->fd == -1) {
            //TODO: log
//            LOG_ERRORF(f->filepath, "open(): [%d] %s", errno, strerror(errno))
            return -1;
        }
    }

    return read(f->fd, buf, size);
}


void fs_close(struct vfile *f) {
    if (f->fd != -1) {
        close(f->fd);
    }
}
