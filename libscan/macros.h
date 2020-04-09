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
    meta_line_t *meta_str = malloc(sizeof(meta_line_t) + strlen(value)); \
    meta_str->key = keyname; \
    strcpy(meta_str->str_val, value); \
    APPEND_META(doc, meta_str)
