#define libc_malloc         __libc_malloc
#define libc_free           __libc_free
#define libc_realloc        __libc_realloc
#define libc_memalign       __libc_memalign
#define libc_valloc         __libc_valloc
#define libc_pvalloc        __libc_pvalloc
#define libc_calloc         __libc_calloc
/*      return libc_icalloc( n, elem_size, chunks );*/
/*      return libc_icommaloc( n, sizes, chunks );*/
#define libc_cfree          __libc_free
/*      return libc_mtrim( s );*/
/*      return libc_musable( m );*/
/*      libc_mstats();*/
#define libc_mallinfo       __libc_mallinfo
#define libc_mallopt        __libc_mallopt

void* __libc_malloc(size_t);
void __libc_free(void*);
void* __libc_calloc(size_t, size_t);
void* __libc_realloc(void*, size_t);
void* __libc_memalign(size_t, size_t);
void* __libc_valloc(size_t);
/*void** independent_calloc(size_t, size_t, void**);*/
/*void** independent_comalloc(size_t, size_t*, void**);*/
void* __libc_pvalloc(size_t);
void __libc_cfree(void*);
/*int      malloc_trim(size_t);*/
/*size_t   malloc_usable_size(void*);*/
/*void     malloc_stats();*/
struct mallinfo __libc_mallinfo(void);
int __libc_mallopt(int, int);
