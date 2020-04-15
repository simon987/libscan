#include <gtest/gtest.h>
#include "test_util.h"

extern "C" {
#include "../libscan/arc/arc.h"
#include "../libscan/text/text.h"
#include "../libscan/ebook/ebook.h"
#include "../libscan/media/media.h"
#include "../libscan/ooxml/ooxml.h"
#include "../libscan/mobi/scan_mobi.h"
#include <libavutil/avutil.h>
}

static scan_arc_ctx_t arc_recurse_ctx;
static scan_arc_ctx_t arc_list_ctx;

static scan_text_ctx_t text_500_ctx;

static scan_ebook_ctx_t ebook_ctx;
static scan_ebook_ctx_t ebook_500_ctx;

static scan_media_ctx_t media_ctx;

static scan_ooxml_ctx_t ooxml_500_ctx;

static scan_mobi_ctx_t mobi_500_ctx;


/* Text */

TEST(Text, BookCsvContentLen) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/text/books.csv", &f, &doc);

    parse_text(&text_500_ctx, &f, &doc);

    ASSERT_NEAR(strlen(get_meta(&doc, MetaContent)->str_val), 500, 4);
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

TEST(Text, MemNoise) {
    char content[600];

    for (char &i : content) {
        int x = rand();
        i = x == 0 ? 1 : x;
    }
    content[599] = '\0';

    vfile_t f;
    document_t doc;
    load_doc_mem((void *) content, strlen(content), &f, &doc);

    parse_text(&text_500_ctx, &f, &doc);

    ASSERT_TRUE(utf8valid(get_meta(&doc, MetaContent)->str_val) == 0);
    cleanup(&doc, &f);
}

TEST(TextMarkup, Mem1) {
    const char *content = "<<a<aa<<<>test<aaaa><>test test    <>";
    vfile_t f;
    document_t doc;
    load_doc_mem((void *) content, strlen(content), &f, &doc);

    parse_markup(&text_500_ctx, &f, &doc);

    ASSERT_STREQ(get_meta(&doc, MetaContent)->str_val, "test test test");
    cleanup(&doc, &f);
}

TEST(TextMarkup, Mem2) {
    const char *content = "<<a<aa<<<>test<aaaa><>test test    ";
    vfile_t f;
    document_t doc;
    load_doc_mem((void *) content, strlen(content), &f, &doc);

    parse_markup(&text_500_ctx, &f, &doc);

    ASSERT_STREQ(get_meta(&doc, MetaContent)->str_val, "test test test");
    cleanup(&doc, &f);
}

TEST(TextMarkup, Xml1) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/text/utf8-example.xml", &f, &doc);

    parse_markup(&text_500_ctx, &f, &doc);

    ASSERT_NEAR(strlen(get_meta(&doc, MetaContent)->str_val), 500, 4);
    ASSERT_TRUE(strstr(get_meta(&doc, MetaContent)->str_val, " BMP:𐌈 ") != nullptr);
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
    ASSERT_NEAR(strlen(get_meta(&doc, MetaContent)->str_val), 500, 4);
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

TEST(Ebook, Epub1) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/ebook/epub1.epub", &f, &doc);

    parse_ebook(&ebook_500_ctx, &f, "application/epub+zip", &doc);

    ASSERT_STREQ(get_meta(&doc, MetaTitle)->str_val, "Rabies");
    ASSERT_NEAR(strlen(get_meta(&doc, MetaContent)->str_val), 500, 4);
    cleanup(&doc, &f);
}

TEST(Ebook, ComicCbz) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/ebook/lost_treasure.cbz", &f, &doc);

    parse_ebook(&ebook_500_ctx, &f, "application/vnd.comicbook+zip", &doc);

    //TODO: Check that thumbnail was generated correctly
    cleanup(&doc, &f);
}

TEST(Ebook, ComicCbr) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/ebook/laugh.cbr", &f, &doc);

    parse_ebook(&ebook_500_ctx, &f, "application/vnd.comicbook-rar", &doc);

    //TODO: Check that thumbnail was generated correctly
    cleanup(&doc, &f);
}

/* Media (image) */

TEST(MediaImage, Exif1) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/media/exiftest1.jpg", &f, &doc);

    parse_media(&media_ctx, &f, &doc);

    ASSERT_STREQ(get_meta(&doc, MetaContent)->str_val, "I don't know if it's a thing mostly done for high end "
                                                       "hotels or what, but I've seen it in a few places in Thailand: "
                                                       "There's a tradition of flower folding, doing a sort of light "
                                                       "origami with the petals of lotus and other flowers, to make "
                                                       "cute little ornaments.");
    ASSERT_STREQ(get_meta(&doc, MetaExifMake)->str_val, "NIKON CORPORATION");
    ASSERT_STREQ(get_meta(&doc, MetaExifModel)->str_val, "NIKON D7000");
    ASSERT_STREQ(get_meta(&doc, MetaExifDateTime)->str_val, "2019:11:08 14:37:59");
    ASSERT_STREQ(get_meta(&doc, MetaExifExposureTime)->str_val, "1:160");
    ASSERT_STREQ(get_meta(&doc, MetaArtist)->str_val, "FinalDoom");
    ASSERT_STREQ(get_meta(&doc, MetaExifSoftware)->str_val, "Adobe Photoshop Lightroom 5.7 (Windows)");
    ASSERT_STREQ(get_meta(&doc, MetaExifFNumber)->str_val, "53:10");
    ASSERT_STREQ(get_meta(&doc, MetaExifFocalLength)->str_val, "900:10");
    ASSERT_STREQ(get_meta(&doc, MetaExifIsoSpeedRatings)->str_val, "400");
    ASSERT_STREQ(get_meta(&doc, MetaExifExposureTime)->str_val, "1:160");

    //TODO: Check that thumbnail was generated correctly
    cleanup(&doc, &f);
}

TEST(MediaVideo, Vid3Mp4) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/media/vid3.mp4", &f, &doc);

    parse_media(&media_ctx, &f, &doc);

    ASSERT_STREQ(get_meta(&doc, MetaTitle)->str_val, "Helicopter (((Accident))) - "
                                                     "https://archive.org/details/Virginia_Helicopter_Crash");
    ASSERT_STREQ(get_meta(&doc, MetaMediaVideoCodec)->str_val, "h264");
    ASSERT_EQ(get_meta(&doc, MetaMediaBitrate)->long_val, 825169);
    ASSERT_EQ(get_meta(&doc, MetaMediaDuration)->long_val, 10);

    //TODO: Check that thumbnail was generated correctly
    cleanup(&doc, &f);
}

TEST(MediaVideo, Vid3Ogv) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/media/vid3.ogv", &f, &doc);

    parse_media(&media_ctx, &f, &doc);

    ASSERT_STREQ(get_meta(&doc, MetaMediaVideoCodec)->str_val, "theora");
    ASSERT_EQ(get_meta(&doc, MetaMediaBitrate)->long_val, 590261);
    ASSERT_EQ(get_meta(&doc, MetaMediaDuration)->long_val, 10);

    //TODO: Check that thumbnail was generated correctly
    cleanup(&doc, &f);
}

TEST(MediaVideo, Vid3Webm) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/media/vid3.webm", &f, &doc);

    parse_media(&media_ctx, &f, &doc);

    ASSERT_STREQ(get_meta(&doc, MetaMediaVideoCodec)->str_val, "vp8");
    ASSERT_EQ(get_meta(&doc, MetaMediaBitrate)->long_val, 343153);
    ASSERT_EQ(get_meta(&doc, MetaMediaDuration)->long_val, 10);

    //TODO: Check that thumbnail was generated correctly
    cleanup(&doc, &f);
}

//TODO: test music file with embedded cover art

TEST(MediaAudio, MusicMp3) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/media/02-The Watchmaker-Barry James_spoken.mp3", &f, &doc);

    parse_media(&media_ctx, &f, &doc);

    ASSERT_STREQ(get_meta(&doc, MetaArtist)->str_val, "Barry James");
    ASSERT_STREQ(get_meta(&doc, MetaAlbum)->str_val, "Strange Slumber, Music for Wonderful Dreams");
    ASSERT_STREQ(get_meta(&doc, MetaTitle)->str_val, "The Watchmaker");
    ASSERT_STREQ(get_meta(&doc, MetaGenre)->str_val, "New Age");
    ASSERT_STREQ(get_meta(&doc, MetaContent)->str_val, "http://magnatune.com/artists/barry_james");
    ASSERT_STREQ(get_meta(&doc, MetaMediaAudioCodec)->str_val, "mp3");

    cleanup(&doc, &f);
}

/* OOXML */

TEST(Ooxml, Pptx1) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/ooxml/Catalist Presentation.pptx", &f, &doc);

    parse_ooxml(&ooxml_500_ctx, &f, &doc);

    ASSERT_STREQ(get_meta(&doc, MetaTitle)->str_val, "Slide 1");
    ASSERT_STREQ(get_meta(&doc, MetaAuthor)->str_val, "thofeller");
    ASSERT_STREQ(get_meta(&doc, MetaModifiedBy)->str_val, "Hofeller");
    ASSERT_NEAR(strlen(get_meta(&doc, MetaContent)->str_val), 500, 4);

    cleanup(&doc, &f);
}

TEST(Ooxml, Docx1) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/ooxml/How To Play A DVD On Windows 8.docx", &f, &doc);

    parse_ooxml(&ooxml_500_ctx, &f, &doc);

    ASSERT_STREQ(get_meta(&doc, MetaAuthor)->str_val, "Thomas");
    ASSERT_STREQ(get_meta(&doc, MetaModifiedBy)->str_val, "Thomas");
    ASSERT_NEAR(strlen(get_meta(&doc, MetaContent)->str_val), 500, 4);

    cleanup(&doc, &f);
}

TEST(Ooxml, Xlsx1) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/ooxml/xlsx1.xlsx", &f, &doc);

    parse_ooxml(&ooxml_500_ctx, &f, &doc);

    ASSERT_STREQ(get_meta(&doc, MetaAuthor)->str_val, "Bureau of Economic Analysis");
    ASSERT_STREQ(get_meta(&doc, MetaModifiedBy)->str_val, "lz");
    ASSERT_NEAR(strlen(get_meta(&doc, MetaContent)->str_val), 500, 4);

    cleanup(&doc, &f);
}

/* Mobi */
TEST(Mobi, Mobi1) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/mobi/Norse Mythology - Neil Gaiman.mobi", &f, &doc);

    parse_mobi(&mobi_500_ctx, &f, &doc);

    ASSERT_STREQ(get_meta(&doc, MetaAuthor)->str_val, "Gaiman, Neil");
    ASSERT_STREQ(get_meta(&doc, MetaTitle)->str_val, "Norse Mythology");
    ASSERT_NEAR(strlen(get_meta(&doc, MetaContent)->str_val), 500, 4);

    cleanup(&doc, &f);
}

TEST(Mobi, Azw) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/mobi/sample.azw", &f, &doc);

    parse_mobi(&mobi_500_ctx, &f, &doc);

    ASSERT_STREQ(get_meta(&doc, MetaAuthor)->str_val, "Nietzsche, Friedrich");
    ASSERT_STREQ(get_meta(&doc, MetaTitle)->str_val, "On the Genealogy of Morality (Hackett Classics)");
    ASSERT_NEAR(strlen(get_meta(&doc, MetaContent)->str_val), 500, 4);

    cleanup(&doc, &f);
}

TEST(Mobi, Azw3) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/mobi/sample.azw3", &f, &doc);

    parse_mobi(&mobi_500_ctx, &f, &doc);

    ASSERT_STREQ(get_meta(&doc, MetaAuthor)->str_val, "George Orwell; Amélie Audiberti");
    ASSERT_STREQ(get_meta(&doc, MetaTitle)->str_val, "1984");
    ASSERT_NEAR(strlen(get_meta(&doc, MetaContent)->str_val), 500, 4);

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
    ebook_ctx.log = noop_log;
    ebook_ctx.logf = noop_logf;

    ebook_500_ctx = ebook_ctx;
    ebook_500_ctx.content_size = 500;

    media_ctx.log = noop_log;
    media_ctx.logf = noop_logf;
    media_ctx.store = noop_store;
    media_ctx.tn_size = 500;
    media_ctx.tn_qscale = 1.0;

    ooxml_500_ctx.content_size = 500;
    ooxml_500_ctx.log = noop_log;
    ooxml_500_ctx.logf = noop_logf;

    mobi_500_ctx.content_size = 500;
    mobi_500_ctx.log = noop_log;
    mobi_500_ctx.logf = noop_logf;

    av_log_set_level(AV_LOG_QUIET);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}