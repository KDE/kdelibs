macro_optional_find_package(BZip2)
macro_log_feature(BZIP2_FOUND "BZip2" "Support for BZip2 compressed files and data streams" "http://www.bzip.org" FALSE "" "STRONGLY RECOMMENDED")

macro_optional_find_package(LibLZMA)
macro_log_feature(LIBLZMA_FOUND "LZMA/XZ" "Support for xz compressed files and data streams" "http://tukaani.org/xz/" FALSE "" "")

set(HAVE_BZIP2_SUPPORT ${BZIP2_FOUND})
if(BZIP2_FOUND AND BZIP2_NEED_PREFIX)
    set(NEED_BZ2_PREFIX 1)
endif(BZIP2_FOUND AND BZIP2_NEED_PREFIX)

set(HAVE_XZ_SUPPORT ${LIBLZMA_FOUND})
