#include "gtest/gtest.h"
#include "test_util.h"

extern "C" {
#include "../libscan/arc/arc.h"
#include "../libscan/text/text.h"
}

static scan_arc_ctx_t arc_recurse_ctx;
static scan_arc_ctx_t arc_list_ctx;
static scan_text_ctx_t text_500_ctx;


TEST(Text, BookCsvContentLen) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/text/books.csv", &f, &doc);

    parse_text(&text_500_ctx, &f, &doc);

    ASSERT_NEAR(strlen(get_meta(&doc, MetaContent)->str_val), 500, 1);
    cleanup(&doc, &f);
}

TEST(Text, MemUtf8_1) {
    const char *content = "a";
    vfile_t f;
    document_t doc;
    load_doc_mem((void *) content, strlen(content), &f, &doc);

    parse_text(&text_500_ctx, &f, &doc);

    ASSERT_EQ(strlen(get_meta(&doc, MetaContent)->str_val), 1);
    cleanup(&doc, &f);
}

TEST(Text, MemUtf8_Invalid1) {
    const char *content = "12345\xE0";
    vfile_t f;
    document_t doc;
    load_doc_mem((void *) content, strlen(content), &f, &doc);

    parse_text(&text_500_ctx, &f, &doc);

    ASSERT_STREQ(get_meta(&doc, MetaContent)->str_val, "12345");
    cleanup(&doc, &f);
}

TEST(Text, MemUtf8_2) {
    const char *content = "最後測試";
    vfile_t f;
    document_t doc;
    load_doc_mem((void *) content, strlen(content), &f, &doc);

    parse_text(&text_500_ctx, &f, &doc);

    ASSERT_STREQ(get_meta(&doc, MetaContent)->str_val, "最後測試");
    cleanup(&doc, &f);
}

TEST(Text, MemUtf8_Invalid2) {
    const char *content = "最後測\xe8\xa9";
    vfile_t f;
    document_t doc;
    load_doc_mem((void *) content, strlen(content), &f, &doc);

    parse_text(&text_500_ctx, &f, &doc);

    ASSERT_STREQ(get_meta(&doc, MetaContent)->str_val, "最後測");
    cleanup(&doc, &f);
}


int main(int argc, char **argv) {
    arc_recurse_ctx.log = noop_log;
    arc_recurse_ctx.logf = noop_logf;
    arc_recurse_ctx.store = noop_store;
    arc_recurse_ctx.mode = ARC_MODE_RECURSE;
    arc_recurse_ctx.parse = nullptr; //TODO

    arc_list_ctx.log = noop_log;
    arc_list_ctx.logf = noop_logf;
    arc_list_ctx.store = noop_store;
    arc_list_ctx.mode = ARC_MODE_LIST;

    text_500_ctx.content_size = 500;
    text_500_ctx.log = noop_log;
    text_500_ctx.logf = noop_logf;

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}