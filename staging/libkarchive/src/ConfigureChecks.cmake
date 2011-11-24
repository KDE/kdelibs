find_package(BZip2)
set_package_properties(BZip2 PROPERTIES
  URL "http://www.bzip.org"
  DESCRIPTION "Support for BZip2 compressed files and data streams"
  TYPE RECOMMENDED
  PURPOSE "Support for BZip2 compressed files and data streams"
)

find_package(LibLZMA)
set_package_properties(LibLZMA PROPERTIES
  URL "http://tukaani.org/xz/"
  DESCRIPTION "Support for xz compressed files and data streams"
  PURPOSE "Support for xz compressed files and data streams"
)

set(HAVE_BZIP2_SUPPORT ${BZIP2_FOUND})
if(BZIP2_FOUND AND BZIP2_NEED_PREFIX)
    set(NEED_BZ2_PREFIX 1)
endif(BZIP2_FOUND AND BZIP2_NEED_PREFIX)

set(HAVE_XZ_SUPPORT ${LIBLZMA_FOUND})
