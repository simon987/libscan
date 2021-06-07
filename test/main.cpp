#include <gtest/gtest.h>
#include "test_util.h"

extern "C" {
#include "../libscan/arc/arc.h"
#include "../libscan/text/text.h"
#include "../libscan/ebook/ebook.h"
#include "../libscan/comic/comic.h"
#include "../libscan/media/media.h"
#include "../libscan/ooxml/ooxml.h"
#include "../libscan/mobi/scan_mobi.h"
#include "../libscan/raw/raw.h"
#include "../libscan/msdoc/msdoc.h"
#include <libavutil/avutil.h>
}

static scan_arc_ctx_t arc_recurse_media_ctx;
static scan_arc_ctx_t arc_list_ctx;

static scan_text_ctx_t text_500_ctx;

static scan_ebook_ctx_t ebook_ctx;
static scan_ebook_ctx_t ebook_500_ctx;

static scan_comic_ctx_t comic_ctx;

static scan_comic_ctx_t comic_big_ctx;

static scan_media_ctx_t media_ctx;

static scan_ooxml_ctx_t ooxml_500_ctx;

static scan_mobi_ctx_t mobi_500_ctx;

static scan_raw_ctx_t raw_ctx;

static scan_msdoc_ctx_t msdoc_ctx;

static scan_msdoc_ctx_t msdoc_text_ctx;


document_t LastSubDoc;

void _parse_media(parse_job_t *job) {
    parse_media(&media_ctx, &job->vfile, &LastSubDoc);
}


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

    ASSERT_EQ(get_meta(&doc, MetaContent), nullptr);
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

TEST(Text, Utf16LE) {

    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/text/pain_is_beauty.log", &f, &doc);
    parse_text(&text_500_ctx, &f, &doc);

    ASSERT_GE(strlen(get_meta(&doc, MetaContent)->str_val), 200);

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

    size_t size_before = store_size;

    parse_ebook(&ebook_500_ctx, &f, "application/pdf", &doc);

    ASSERT_STREQ(get_meta(&doc, MetaTitle)->str_val, "Microsoft Word - A531 Candlemaking-01.doc");
    ASSERT_STREQ(get_meta(&doc, MetaAuthor)->str_val, "Dafydd Prichard");
    ASSERT_NEAR(strlen(get_meta(&doc, MetaContent)->str_val), 500, 4);
    ASSERT_NE(get_meta(&doc, MetaContent)->str_val[0], ' ');
    ASSERT_NE(size_before, store_size);
    ASSERT_EQ(get_meta(&doc, MetaPages)->int_val, 16);

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

TEST(Ebook, Pdf2) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/ebook/pdf2.pdf", &f, &doc);

    parse_ebook(&ebook_500_ctx, &f, "application/pdf", &doc);

    cleanup(&doc, &f);
}

TEST(Ebook, PdfBlank) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/ebook/blank.pdf", &f, &doc);

    parse_ebook(&ebook_500_ctx, &f, "application/pdf", &doc);

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

TEST(Ebook, EpubBlankFirstPage) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/ebook/EpubBlankFirstPage.epub", &f, &doc);

    parse_ebook(&ebook_500_ctx, &f, "application/epub+zip", &doc);

    ASSERT_STREQ(get_meta(&doc, MetaTitle)->str_val, "Design Culture");
    ASSERT_NEAR(strlen(get_meta(&doc, MetaContent)->str_val), 500, 4);
    cleanup(&doc, &f);
}

/* Comic */
TEST(Comic, ComicCbz) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/ebook/lost_treasure.cbz", &f, &doc);

    size_t size_before = store_size;

    parse_comic(&comic_ctx, &f, &doc);

    ASSERT_NE(size_before, store_size);

    cleanup(&doc, &f);
}

TEST(Comic, ComicCbr) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/ebook/laugh.cbr", &f, &doc);

    size_t size_before = store_size;

    parse_comic(&comic_ctx, &f, &doc);

    ASSERT_NE(size_before, store_size);

    cleanup(&doc, &f);
}

TEST(Comic, ComicCbrAsIs) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/ebook/laugh.cbr", &f, &doc);

    size_t size_before = store_size;

    parse_comic(&comic_big_ctx, &f, &doc);

    ASSERT_EQ(store_size - size_before, 92451);

    cleanup(&doc, &f);
}


TEST(Comic, ComicCbrFilters) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/ebook/cannot_parse_filters.cbr", &f, &doc);

    parse_comic(&comic_ctx, &f, &doc);

    cleanup(&doc, &f);
}


/* Media (image) */

TEST(MediaImage, ExifGps1) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/media/exif_GPS.jpg", &f, &doc);

    parse_media(&media_ctx, &f, &doc);

    ASSERT_STREQ(get_meta(&doc, MetaExifGpsLatitudeRef)->str_val, "N");
    ASSERT_STREQ(get_meta(&doc, MetaExifGpsLatitudeDMS)->str_val, "48:1 , 56585399:1000000, 0:1");

    ASSERT_STREQ(get_meta(&doc, MetaExifGpsLongitudeRef)->str_val, "E");
    ASSERT_STREQ(get_meta(&doc, MetaExifGpsLongitudeDMS)->str_val, "9:1 , 28046900:1000000, 0:1");

    cleanup(&doc, &f);
}

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

TEST(MediaImage, Mem1) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/media/test.jpeg.tar", &f, &doc);

    size_t size_before = store_size;

    parse_archive(&arc_recurse_media_ctx, &f, &doc);

    ASSERT_NE(size_before, store_size);

    cleanup(&doc, &f);
}

TEST(MediaImage, Mem2AsIs) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/media/test2.zip", &f, &doc);

    size_t size_before = store_size;

    parse_archive(&arc_recurse_media_ctx, &f, &doc);

    ASSERT_EQ(size_before + 14098, store_size);

    cleanup(&doc, &f);
}

TEST(MediaVideo, VidMkvSubDisabled) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/media/berd.mkv", &f, &doc);

    size_t size_before = store_size;
    parse_media(&media_ctx, &f, &doc);

    ASSERT_NE(size_before, store_size);
    ASSERT_EQ(get_meta(&doc, MetaContent), nullptr);

    cleanup(&doc, &f);
}

TEST(MediaVideo, VidMkvSubEnabled) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/media/berd.mkv", &f, &doc);

    size_t size_before = store_size;
    media_ctx.read_subtitles = TRUE;
    parse_media(&media_ctx, &f, &doc);
    media_ctx.read_subtitles = FALSE;

    ASSERT_NE(size_before, store_size);
    ASSERT_NE(get_meta(&doc, MetaContent), nullptr);

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

TEST(MediaVideoVfile, Vid3Ogv) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/arc/vid3.tar", &f, &doc);

    size_t size_before = store_size;

    parse_archive(&arc_recurse_media_ctx, &f, &doc);

//    ASSERT_STREQ(get_meta(&LastSubDoc, MetaMediaVideoCodec)->str_val, "theora");
    ASSERT_EQ(get_meta(&LastSubDoc, MetaMediaBitrate)->long_val, 590261);
    ASSERT_EQ(get_meta(&LastSubDoc, MetaMediaDuration)->long_val, 10);
    ASSERT_NE(size_before, store_size);

    cleanup(&doc, &f);
}

TEST(MediaVideo, VidDuplicateTags) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/media/vid_tags.mkv", &f, &doc);

    parse_media(&media_ctx, &f, &doc);

    meta_line_t *meta_content = get_meta(&doc, MetaContent);
    ASSERT_STREQ(meta_content->str_val, "he's got a point");
    ASSERT_EQ(get_meta_from(meta_content->next, MetaContent), nullptr);

    meta_line_t *meta_title = get_meta(&doc, MetaTitle);
    ASSERT_STREQ(meta_title->str_val, "cool shit");
    ASSERT_EQ(get_meta_from(meta_title->next, MetaTitle), nullptr);

    meta_line_t *meta_artist = get_meta(&doc, MetaArtist);
    ASSERT_STREQ(meta_artist->str_val, "psychicpebbles");
    ASSERT_EQ(get_meta_from(meta_artist->next, MetaArtist), nullptr);

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

TEST(Ooxml, Docx2Thumbnail) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/ooxml/embed_tn.docx", &f, &doc);

    size_t size_before = store_size;

    parse_ooxml(&ooxml_500_ctx, &f, &doc);

    ASSERT_NEAR(strlen(get_meta(&doc, MetaContent)->str_val), 500, 4);
    ASSERT_NE(size_before, store_size);

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

/* Arc */
TEST(Arc, Utf8) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/arc/test1.zip", &f, &doc);

    parse_archive(&arc_list_ctx, &f, &doc);

    ASSERT_TRUE(strstr(get_meta(&doc, MetaContent)->str_val, "arctest/ȬȬȬȬȬȬȬȬȬȬȬȬȬȬȬȬȬȬȬȬȬȬȬȬ.txt") != nullptr);

    cleanup(&doc, &f);
}

TEST(Arc, EncryptedZip) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/arc/encrypted.zip", &f, &doc);

    size_t size_before = store_size;

    strcpy(arc_recurse_media_ctx.passphrase, "sist2");
    parse_archive(&arc_recurse_media_ctx, &f, &doc);

    arc_recurse_media_ctx.passphrase[0] = '\0';

    ASSERT_NE(size_before, store_size);

    cleanup(&doc, &f);
}

/* RAW */
TEST(RAW, Panasonic) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/raw/Panasonic.RW2", &f, &doc);

    size_t size_before = store_size;

    parse_raw(&raw_ctx, &f, &doc);

    ASSERT_STREQ(get_meta(&doc, MetaMediaVideoCodec)->str_val, "raw");
    ASSERT_STREQ(get_meta(&doc, MetaExifModel)->str_val, "DMC-GX8");
    ASSERT_STREQ(get_meta(&doc, MetaExifMake)->str_val, "Panasonic");
    ASSERT_STREQ(get_meta(&doc, MetaExifIsoSpeedRatings)->str_val, "640");
    ASSERT_STREQ(get_meta(&doc, MetaExifDateTime)->str_val, "2020:07:20 10:00:34");
    ASSERT_STREQ(get_meta(&doc, MetaExifFocalLength)->str_val, "20.0");
    ASSERT_STREQ(get_meta(&doc, MetaExifFNumber)->str_val, "2.0");
    ASSERT_EQ(get_meta(&doc, MetaWidth)->int_val, 5200);
    ASSERT_EQ(get_meta(&doc, MetaHeight)->int_val, 3904);
    ASSERT_NE(size_before, store_size);

    cleanup(&doc, &f);
}

TEST(RAW, ExifGps1) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/raw/exif_gps.DNG", &f, &doc);

    size_t size_before = store_size;

    parse_raw(&raw_ctx, &f, &doc);

    ASSERT_NE(size_before, store_size);

    ASSERT_STREQ(get_meta(&doc, MetaExifGpsLatitudeDec)->str_val, "48.943088531494141");
    ASSERT_STREQ(get_meta(&doc, MetaExifGpsLongitudeDec)->str_val, "9.467448234558105");

    cleanup(&doc, &f);
}

TEST(RAW, Nikon) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/raw/Nikon.NEF", &f, &doc);

    size_t size_before = store_size;

    parse_raw(&raw_ctx, &f, &doc);

    ASSERT_STREQ(get_meta(&doc, MetaMediaVideoCodec)->str_val, "raw");
    ASSERT_STREQ(get_meta(&doc, MetaExifModel)->str_val, "D750");
    ASSERT_STREQ(get_meta(&doc, MetaExifMake)->str_val, "Nikon");
    ASSERT_EQ(get_meta(&doc, MetaWidth)->int_val, 6032);
    ASSERT_EQ(get_meta(&doc, MetaHeight)->int_val, 4032);
    ASSERT_NE(size_before, store_size);

    cleanup(&doc, &f);
}

TEST(RAW, Sony) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/raw/Sony.ARW", &f, &doc);

    size_t size_before = store_size;

    parse_raw(&raw_ctx, &f, &doc);

    ASSERT_STREQ(get_meta(&doc, MetaMediaVideoCodec)->str_val, "raw");
    ASSERT_STREQ(get_meta(&doc, MetaExifModel)->str_val, "ILCE-7RM3");
    ASSERT_STREQ(get_meta(&doc, MetaExifMake)->str_val, "Sony");
    ASSERT_EQ(get_meta(&doc, MetaWidth)->int_val, 7968);
    ASSERT_EQ(get_meta(&doc, MetaHeight)->int_val, 5320);
    ASSERT_NE(size_before, store_size);

    cleanup(&doc, &f);
}

TEST(RAW, Olympus) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/raw/Olympus.ORF", &f, &doc);

    size_t size_before = store_size;

    parse_raw(&raw_ctx, &f, &doc);

    ASSERT_STREQ(get_meta(&doc, MetaMediaVideoCodec)->str_val, "raw");
    ASSERT_STREQ(get_meta(&doc, MetaExifModel)->str_val, "E-M5MarkII");
    ASSERT_STREQ(get_meta(&doc, MetaExifMake)->str_val, "Olympus");
    ASSERT_EQ(get_meta(&doc, MetaWidth)->int_val, 4640);
    ASSERT_EQ(get_meta(&doc, MetaHeight)->int_val, 3472);
    ASSERT_NE(size_before, store_size);

    cleanup(&doc, &f);
}

TEST(RAW, Fuji) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/raw/Fuji.RAF", &f, &doc);

    size_t size_before = store_size;
    parse_raw(&raw_ctx, &f, &doc);

    ASSERT_STREQ(get_meta(&doc, MetaMediaVideoCodec)->str_val, "raw");
    ASSERT_STREQ(get_meta(&doc, MetaExifModel)->str_val, "X-T2");
    ASSERT_STREQ(get_meta(&doc, MetaExifMake)->str_val, "Fujifilm");
    ASSERT_EQ(get_meta(&doc, MetaWidth)->int_val, 6032);
    ASSERT_EQ(get_meta(&doc, MetaHeight)->int_val, 4028);
    ASSERT_NE(size_before, store_size);

    cleanup(&doc, &f);
}

/* msdoc */
TEST(Msdoc, Test1Pdf) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/msdoc/test1.doc", &f, &doc);

    size_t size_before = store_size;

    parse_msdoc(&msdoc_ctx, &f, &doc);

    ASSERT_TRUE(strstr(get_meta(&doc, MetaContent)->str_val, "October 2000") != nullptr);
    ASSERT_STREQ(get_meta(&doc, MetaTitle)->str_val, "INTERNATIONAL ORGANIZATION FOR STANDARDIZATION");
    ASSERT_STREQ(get_meta(&doc, MetaAuthor)->str_val, "Oliver Morgan");
    ASSERT_EQ(get_meta(&doc, MetaPages)->int_val, 57);
    ASSERT_NEAR(strlen(get_meta(&doc, MetaContent)->str_val), msdoc_ctx.content_size, 4);
    ASSERT_NE(size_before, store_size);

    cleanup(&doc, &f);
}

TEST(Msdoc, Test1Text) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/msdoc/test1.doc", &f, &doc);

    size_t size_before = store_size;

    parse_msdoc(&msdoc_text_ctx, &f, &doc);

    ASSERT_TRUE(strstr(get_meta(&doc, MetaContent)->str_val, "October 2000") != nullptr);
    ASSERT_STREQ(get_meta(&doc, MetaTitle)->str_val, "INTERNATIONAL ORGANIZATION FOR STANDARDIZATION");
    ASSERT_STREQ(get_meta(&doc, MetaAuthor)->str_val, "Oliver Morgan");
    ASSERT_NEAR(strlen(get_meta(&doc, MetaContent)->str_val), msdoc_ctx.content_size, 4);
    ASSERT_EQ(size_before, store_size);

    cleanup(&doc, &f);
}

TEST(Msdoc, Test2Pdf) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/msdoc/test2.doc", &f, &doc);

    size_t size_before = store_size;

    parse_msdoc(&msdoc_ctx, &f, &doc);

    ASSERT_TRUE(strstr(get_meta(&doc, MetaContent)->str_val, "GNU Free Documentation License") != nullptr);
    ASSERT_STREQ(get_meta(&doc, MetaTitle)->str_val, "DWARF Debugging Information Format");
    ASSERT_STREQ(get_meta(&doc, MetaAuthor)->str_val, "Ron Brender");
    ASSERT_NEAR(strlen(get_meta(&doc, MetaContent)->str_val), msdoc_ctx.content_size, 4);
    ASSERT_NE(size_before, store_size);

    cleanup(&doc, &f);
}

TEST(Msdoc, Test3Pdf) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/msdoc/test3.doc", &f, &doc);

    size_t size_before = store_size;

    parse_msdoc(&msdoc_ctx, &f, &doc);

    ASSERT_TRUE(strstr(get_meta(&doc, MetaContent)->str_val, "INTERNATIONAL PATENT CLASSIFICATION") != nullptr);
    ASSERT_STREQ(get_meta(&doc, MetaTitle)->str_val, "IPC Fixed Texts Specification");
    ASSERT_STREQ(get_meta(&doc, MetaAuthor)->str_val, "Fievet");
    ASSERT_NEAR(strlen(get_meta(&doc, MetaContent)->str_val), msdoc_ctx.content_size, 4);
    ASSERT_NE(size_before, store_size);

    cleanup(&doc, &f);
}

TEST(Msdoc, Test4Pdf) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/msdoc/test4.doc", &f, &doc);

    size_t size_before = store_size;

    parse_msdoc(&msdoc_ctx, &f, &doc);

    ASSERT_TRUE(strstr(get_meta(&doc, MetaContent)->str_val, "SQL Server international data types") != nullptr);
    ASSERT_STREQ(get_meta(&doc, MetaTitle)->str_val, "MSDN Authoring Template");
    ASSERT_STREQ(get_meta(&doc, MetaAuthor)->str_val, "Brenda Yen");
    ASSERT_NEAR(strlen(get_meta(&doc, MetaContent)->str_val), msdoc_ctx.content_size, 4);
    ASSERT_NE(size_before, store_size);

    cleanup(&doc, &f);
}

TEST(Msdoc, TestUtf8Pdf) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/msdoc/japanese.doc", &f, &doc);

    size_t size_before = store_size;

    parse_msdoc(&msdoc_ctx, &f, &doc);

    ASSERT_NE(get_meta(&doc, MetaContent), nullptr);
    ASSERT_TRUE(strstr(get_meta(&doc, MetaContent)->str_val, "调查项目 A questionnaire") != nullptr);
    ASSERT_NE(size_before, store_size);

    cleanup(&doc, &f);
}

TEST(Msdoc, TestUtf8Text) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/msdoc/japanese.doc", &f, &doc);

    parse_msdoc(&msdoc_text_ctx, &f, &doc);

    ASSERT_TRUE(strstr(get_meta(&doc, MetaContent)->str_val, "调查项目 A questionnaire") != nullptr);

    cleanup(&doc, &f);
}

TEST(Msdoc, TestFuzz1) {
    vfile_t f;
    document_t doc;
    load_doc_file("libscan-test-files/test_files/msdoc/fuzz_ole.doc", &f, &doc);

    size_t buf_len;
    char *buf = (char *) read_all(&f, &buf_len);

    for (int i = 0; i < 1000; i++) {
        size_t buf_len_copy = buf_len;
        char *buf_copy = (char*)malloc(buf_len);
        memcpy(buf_copy, buf, buf_len);

        fuzz_buffer(buf_copy, &buf_len_copy, 3, 8, 5);
        FILE *file = fmemopen(buf_copy, buf_len_copy, "rb");
        parse_msdoc_text(&msdoc_text_ctx, &doc, file, buf_copy, buf_len_copy);
    }
    free(buf);
    cleanup(&doc, &f);
}


int main(int argc, char **argv) {
    setlocale(LC_ALL, "");

    arc_recurse_media_ctx.log = noop_log;
    arc_recurse_media_ctx.logf = noop_logf;
    arc_recurse_media_ctx.store = counter_store;
    arc_recurse_media_ctx.mode = ARC_MODE_RECURSE;
    arc_recurse_media_ctx.parse = _parse_media;

    arc_list_ctx.log = noop_log;
    arc_list_ctx.logf = noop_logf;
    arc_list_ctx.store = counter_store;
    arc_list_ctx.mode = ARC_MODE_LIST;

    text_500_ctx.content_size = 500;
    text_500_ctx.log = noop_log;
    text_500_ctx.logf = noop_logf;

    ebook_ctx.content_size = 999999999999;
    ebook_ctx.store = counter_store;
    ebook_ctx.tesseract_lang = "eng";
    ebook_ctx.tesseract_path = "./tessdata";
    ebook_ctx.tn_size = 500;
    ebook_ctx.log = noop_log;
    ebook_ctx.logf = noop_logf;

    ebook_500_ctx = ebook_ctx;
    ebook_500_ctx.content_size = 500;

    comic_ctx.tn_qscale = 1.0;
    comic_ctx.tn_size = 500;
    comic_ctx.log = noop_log;
    comic_ctx.logf = noop_logf;
    comic_ctx.store = counter_store;

    comic_big_ctx.tn_qscale = 1.0;
    comic_big_ctx.tn_size = 5000;
    comic_big_ctx.log = noop_log;
    comic_big_ctx.logf = noop_logf;
    comic_big_ctx.store = counter_store;

    media_ctx.log = noop_log;
    media_ctx.logf = noop_logf;
    media_ctx.store = counter_store;
    media_ctx.tn_size = 500;
    media_ctx.tn_qscale = 1.0;
    media_ctx.max_media_buffer = (long) 2000 * (long) 1024 * (long) 1024;

    ooxml_500_ctx.content_size = 500;
    ooxml_500_ctx.log = noop_log;
    ooxml_500_ctx.logf = noop_logf;
    ooxml_500_ctx.store = counter_store;

    mobi_500_ctx.content_size = 500;
    mobi_500_ctx.log = noop_log;
    mobi_500_ctx.logf = noop_logf;

    raw_ctx.log = noop_log;
    raw_ctx.logf = noop_logf;
    raw_ctx.store = counter_store;
    raw_ctx.tn_size = 500;
    raw_ctx.tn_qscale = 5.0;

    msdoc_ctx.log = noop_log;
    msdoc_ctx.logf = noop_logf;
    msdoc_ctx.store = counter_store;
    msdoc_ctx.content_size = 500;
    msdoc_ctx.tn_size = 500;

    msdoc_text_ctx.log = noop_log;
    msdoc_text_ctx.logf = noop_logf;
    msdoc_text_ctx.store = counter_store;
    msdoc_text_ctx.content_size = 500;
    msdoc_text_ctx.tn_size = 0;

    av_log_set_level(AV_LOG_QUIET);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}