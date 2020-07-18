#include "ooxml.h"

#include "../util.h"
#include <archive.h>
#include <archive_entry.h>
#include <libxml/xmlstring.h>
#include <libxml/parser.h>

#define _X(str) ((const xmlChar*)str)

__always_inline
static int should_read_part(const char *part) {

    if (part == NULL) {
        return FALSE;
    }

    if (    // Word
            STR_STARTS_WITH(part, "word/document.xml")
            || STR_STARTS_WITH(part, "word/footnotes.xml")
            || STR_STARTS_WITH(part, "word/endnotes.xml")
            || STR_STARTS_WITH(part, "word/footer")
            || STR_STARTS_WITH(part, "word/header")
            // PowerPoint
            || STR_STARTS_WITH(part, "ppt/slides/slide")
            || STR_STARTS_WITH(part, "ppt/notesSlides/slide")
            // Excel
            || STR_STARTS_WITH(part, "xl/worksheets/sheet")
            || STR_STARTS_WITH(part, "xl/sharedStrings.xml")
            || STR_STARTS_WITH(part, "xl/workbook.xml")
            ) {
        return TRUE;
    }

    return FALSE;
}

int extract_text(scan_ooxml_ctx_t *ctx, xmlDoc *xml, xmlNode *node, text_buffer_t *buf) {
    //TODO: Check which nodes are likely to have a 't' child, and ignore nodes that aren't
    xmlErrorPtr err = xmlGetLastError();
    if (err != NULL) {
        if (err->level == XML_ERR_FATAL) {
            CTX_LOG_ERRORF("ooxml.c", "Got fatal XML error while parsing document: %s", err->message)
            return -1;
        } else {
            CTX_LOG_ERRORF("ooxml.c", "Got recoverable XML error while parsing document: %s", err->message)
        }
    }

    for (xmlNode *child = node; child; child = child->next) {
        if (*child->name == 't' && *(child->name + 1) == '\0') {
            xmlChar *text = xmlNodeListGetString(xml, child->xmlChildrenNode, 1);

            if (text) {
                int ret = text_buffer_append_string0(buf, (char *) text);
                text_buffer_append_char(buf, ' ');
                xmlFree(text);

                if (ret == TEXT_BUF_FULL) {
                    return ret;
                }
            }
        }

        if (extract_text(ctx, xml, child->children, buf) == TEXT_BUF_FULL) {
            return TEXT_BUF_FULL;
        }
    }
    return 0;
}

int xml_io_read(void *context, char *buffer, int len) {
    struct archive *a = context;
    return archive_read_data(a, buffer, len);
}

int xml_io_close(UNUSED(void *context)) {
    //noop
    return 0;
}

#define READ_PART_ERR -2

__always_inline
static int read_part(scan_ooxml_ctx_t *ctx, struct archive *a, text_buffer_t *buf, document_t *doc) {

    xmlDoc *xml = xmlReadIO(xml_io_read, xml_io_close, a, "/", NULL,
                            XML_PARSE_RECOVER | XML_PARSE_NOWARNING | XML_PARSE_NOERROR | XML_PARSE_NONET);

    if (xml == NULL) {
        CTX_LOG_ERROR(doc->filepath, "Could not parse XML")
        return READ_PART_ERR;
    }

    xmlNode *root = xmlDocGetRootElement(xml);
    if (root == NULL) {
        CTX_LOG_ERROR(doc->filepath, "Empty document")
        xmlFreeDoc(xml);
        return READ_PART_ERR;
    }

    int ret = extract_text(ctx, xml, root, buf);
    xmlFreeDoc(xml);

    return ret;
}

__always_inline
static int read_doc_props(scan_ooxml_ctx_t *ctx, struct archive *a, text_buffer_t *buf, document_t *doc) {
    xmlDoc *xml = xmlReadIO(xml_io_read, xml_io_close, a, "/", NULL,
                            XML_PARSE_RECOVER | XML_PARSE_NOWARNING | XML_PARSE_NOERROR | XML_PARSE_NONET);

    if (xml == NULL) {
        CTX_LOG_ERROR(doc->filepath, "Could not parse XML")
        return -1;
    }

    xmlNode *root = xmlDocGetRootElement(xml);
    if (root == NULL) {
        CTX_LOG_ERROR(doc->filepath, "Empty document")
        xmlFreeDoc(xml);
        return -1;
    }

    if (xmlStrEqual(root->name, _X("coreProperties"))) {
        for (xmlNode *child = root->children; child; child = child->next) {
            xmlChar *text = xmlNodeListGetString(xml, child->xmlChildrenNode, 1);
            if (text == NULL) {
                continue;
            }

            if (xmlStrEqual(child->name, _X("title"))) {
                APPEND_STR_META(doc, MetaTitle, (char *) text)
            } else if (xmlStrEqual(child->name, _X("creator"))) {
                APPEND_STR_META(doc, MetaAuthor, (char *) text)
            } else if (xmlStrEqual(child->name, _X("lastModifiedBy"))) {
                APPEND_STR_META(doc, MetaModifiedBy, (char *) text)
            }

            xmlFree(text);
        }
    }
    xmlFreeDoc(xml);

    return 0;
}

#define MAX_TN_SIZE 1024 * 1024 * 15

void read_thumbnail(scan_ooxml_ctx_t *ctx, document_t *doc, struct archive *a, struct archive_entry *entry) {
    size_t entry_size = archive_entry_size(entry);

    if (entry_size <= 0 || entry_size > MAX_TN_SIZE) {
        return;
    }

    char* buf = malloc(entry_size);
    archive_read_data(a, buf, entry_size);

    APPEND_TN_META(doc, 1, 1) // Size unknown
    ctx->store((char *) doc->uuid, sizeof(doc->uuid), buf, entry_size);
    free(buf);
}

void parse_ooxml(scan_ooxml_ctx_t *ctx, vfile_t *f, document_t *doc) {

    size_t buf_len;
    void *buf = read_all(f, &buf_len);

    struct archive *a = archive_read_new();
    archive_read_support_format_zip(a);

    int ret = archive_read_open_memory(a, buf, buf_len);
    if (ret != ARCHIVE_OK) {
        CTX_LOG_ERRORF(doc->filepath, "Could not read archive: %s", archive_error_string(a))
        archive_read_free(a);
        free(buf);
        return;
    }

    text_buffer_t tex = text_buffer_create(ctx->content_size);

    struct archive_entry *entry;
    int buffer_full = FALSE;
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        if (S_ISREG(archive_entry_stat(entry)->st_mode)) {
            const char *path = archive_entry_pathname(entry);

            if (!buffer_full && should_read_part(path) && ctx->content_size > 0) {
                ret = read_part(ctx, a, &tex, doc);
                if (ret == READ_PART_ERR) {
                    break;
                } else if (ret == TEXT_BUF_FULL) {
                    buffer_full = TRUE;
                }
            } else if (strcmp(path, "docProps/core.xml") == 0) {
                if (read_doc_props(ctx, a, &tex, doc) != 0) {
                    break;
                }
            } else if (strcmp(path, "docProps/thumbnail.jpeg") == 0) {
                read_thumbnail(ctx, doc, a, entry);
            }
        }
    }

    if (tex.dyn_buffer.cur > 0) {
        text_buffer_terminate_string(&tex);

        meta_line_t *meta = malloc(sizeof(meta_line_t) + tex.dyn_buffer.cur);
        meta->key = MetaContent;
        strcpy(meta->str_val, tex.dyn_buffer.buf);
        APPEND_META(doc, meta)
    }

    archive_read_close(a);
    archive_read_free(a);
    text_buffer_destroy(&tex);
    free(buf);
}
