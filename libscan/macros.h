#ifndef	FALSE
#define	FALSE	(0)
#define BOOL int
#endif

#ifndef	TRUE
#define	TRUE	(!FALSE)
#endif

#undef	MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

#undef	MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#undef ABS
#define ABS(a)	   (((a) < 0) ? -(a) : (a))

#define APPEND_STR_META(doc, keyname, value) \
    {meta_line_t *meta_str = malloc(sizeof(meta_line_t) + strlen(value)); \
    meta_str->key = keyname; \
    strcpy(meta_str->str_val, value); \
    APPEND_META(doc, meta_str)}

#define APPEND_INT_META(doc, keyname, value) \
    {meta_line_t *meta_int = malloc(sizeof(meta_line_t)); \
    meta_int->key = keyname; \
    meta_int->int_val = value; \
    APPEND_META(doc, meta_int)}

#define APPEND_TN_META(doc, width, height) \
    {meta_line_t *meta_str = malloc(sizeof(meta_line_t) + 4 + 1 + 4); \
    meta_str->key = MetaThumbnail; \
    sprintf(meta_str->str_val, "%04d,%04d", width, height); \
    APPEND_META(doc, meta_str)}
