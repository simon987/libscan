#include "../libscan/text/text.h"
#include <fcntl.h>
#include "../libscan/arc/arc.h"

int main() {

    scan_text_ctx_t ctx;

    ctx.content_size = 100;
    vfile_t file;
    file.is_fs_file = TRUE;
    file.filepath = "/home/simon/Downloads/libscan/CMakeLists.txt";
    file.fd = open("/home/simon/Downloads/libscan/CMakeLists.txt", O_RDONLY);
    file.read = fs_read;

    document_t doc;
    doc.meta_head = NULL;
    doc.meta_tail = NULL;

    doc.size = 200;
    parse_text(&ctx, &file, &doc);
}