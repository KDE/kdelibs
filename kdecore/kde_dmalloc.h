#if !defined(KDE_DMALLOC_H) && defined(WITH_DMALLOC)
#define KDE_DMALLOC_H

#if !defined( NDEBUG) && defined (__cplusplus)

void *operator new (size_t size, const char *file, const int line);
void *operator new[] (size_t size, const char *file, const int line);

#define new new(__FILE__, __LINE__)

#endif

#endif
