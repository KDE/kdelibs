kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )

# simple first try: add a target "doc" which calls meinproc, and have it depend on meinproc
add_custom_target(doc 
   ${KDE4_MEINPROC_EXECUTABLE} --cache ${CMAKE_CURRENT_BINARY_DIR}/index.cache.bz2 ${CMAKE_CURRENT_SOURCE_DIR}/index.docbook 
   DEPENDS meinproc )

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#
#KDE_LANG = en
#KDE_DOCS = AUTO
#KDE_DOCS_DEPS = $(MEINPROC_DEP)
#
