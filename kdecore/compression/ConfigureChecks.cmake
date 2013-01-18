macro_optional_find_package(BZip2)
set_package_properties(BZip2 PROPERTIES DESCRIPTION "Support for BZip2 compressed files and data streams"
                       URL "http://www.bzip.org"
                       TYPE RECOMMENDED
                      )

macro_optional_find_package(LibLZMA)
set_package_properties(LibLZMA PROPERTIES DESCRIPTION "Support for xz compressed files and data streams"
                       URL "http://tukaani.org/xz/"
                       TYPE OPTIONAL
                      )

macro_bool_to_01(BZIP2_FOUND HAVE_BZIP2_SUPPORT)
if(BZIP2_FOUND AND BZIP2_NEED_PREFIX)
    set(NEED_BZ2_PREFIX 1)
endif(BZIP2_FOUND AND BZIP2_NEED_PREFIX)

macro_bool_to_01(LIBLZMA_FOUND HAVE_XZ_SUPPORT)
