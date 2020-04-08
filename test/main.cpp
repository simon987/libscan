#include "gtest/gtest.h"
#include "test_util.h"

extern "C" {
#include "../libscan/arc/arc.h"
#include "../libscan/text/text.h"
#include "../libscan/ebook/ebook.h"
}

static scan_arc_ctx_t arc_recurse_ctx;
static scan_arc_ctx_t arc_list_ctx;

static scan_text_ctx_t text_500_ctx;

static scan_ebook_ctx_t ebook_ctx;
static scan_ebook_ctx_t ebook_500_ctx;



/* Text */

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

TEST(Text, MemWhitespace) {
    const char *content = "\n \ttest\t\ntest test     ";
    vfile_t f;
    document_t doc;
    load_doc_mem((void *) content, strlen(content), &f, &doc);

    parse_text(&text_500_ctx, &f, &doc);

    ASSERT_STREQ(get_meta(&doc, MetaContent)->str_val, "test test test");
    cleanup(&doc, &f);
}

/* Ebook */

TEST(Ebook, CandlePdf) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/ebook/General_-_Candle_Making.pdf", &f, &doc);

    parse_ebook(&ebook_500_ctx, &f, "application/pdf", &doc);

    ASSERT_STREQ(get_meta(&doc, MetaTitle)->str_val, "Microsoft Word - A531 Candlemaking-01.doc");
    ASSERT_STREQ(get_meta(&doc, MetaAuthor)->str_val, "Dafydd Prichard");
    ASSERT_NEAR(strlen(get_meta(&doc, MetaContent)->str_val), 500, 1);
    ASSERT_NE(get_meta(&doc, MetaContent)->str_val[0], ' ');
    cleanup(&doc, &f);
}

TEST(Ebook, Utf8Pdf) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/ebook/utf8.pdf", &f, &doc);

    parse_ebook(&ebook_500_ctx, &f, "application/pdf", &doc);

    ASSERT_TRUE(STR_STARTS_WITH(get_meta(&doc, MetaContent)->str_val, "最後測試 "));
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

    ebook_ctx.content_size = 999999999999;
    ebook_ctx.store = noop_store;
    ebook_ctx.tesseract_lang = "eng";
    ebook_ctx.tesseract_path = "./tessdata";
    ebook_ctx.tn_size = 500;
    pthread_mutex_init(&ebook_ctx.mupdf_mutex, nullptr);
    ebook_ctx.log = noop_log;
    ebook_ctx.logf = noop_logf;

    ebook_500_ctx = ebook_ctx;
    ebook_500_ctx.content_size = 500;

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}