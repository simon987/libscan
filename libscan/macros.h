#ifndef FALSE
#define FALSE (0)
#define BOOL int
#endif

#ifndef TRUE
#define TRUE (!FALSE)
#endif

#undef MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

#undef MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#undef ABS
#define ABS(a) (((a) < 0) ? -(a) : (a))

#define APPEND_STR_META(doc, keyname, value) \
    {meta_line_t *meta_str = malloc(sizeof(meta_line_t) + strlen(value)); \
    meta_str->key = keyname; \
    strcpy(meta_str->str_val, value); \
    APPEND_META(doc, meta_str)}

#define APPEND_LONG_META(doc, keyname, value) \
    {meta_line_t *meta_long = malloc(sizeof(meta_line_t)); \
    meta_long->key = keyname; \
    meta_long->long_val = value; \
    APPEND_META(doc, meta_long)}

#define APPEND_TN_META(doc, width, height) \
    {meta_line_t *meta_str = malloc(sizeof(meta_line_t) + 4 + 1 + 4); \
    meta_str->key = MetaThumbnail; \
    sprintf(meta_str->str_val, "%04d,%04d", width, height); \
    APPEND_META(doc, meta_str)}

#define APPEND_META(doc, meta) \
    meta->next = NULL;\
    if (doc->meta_head == NULL) {\
        doc->meta_head = meta;\
        doc->meta_tail = doc->meta_head;\
    } else {\
        doc->meta_tail->next = meta;\
        doc->meta_tail = meta;\
    }

#define APPEND_UTF8_META(doc, keyname, str) \
    text_buffer_t tex = text_buffer_create(-1); \
    text_buffer_append_string0(&tex, str); \
    text_buffer_terminate_string(&tex); \
    meta_line_t *meta_tag = malloc(sizeof(meta_line_t) + tex.dyn_buffer.cur); \
    meta_tag->key = keyname; \
    strcpy(meta_tag->str_val, tex.dyn_buffer.buf); \
    APPEND_META(doc, meta_tag) \
    text_buffer_destroy(&tex);
