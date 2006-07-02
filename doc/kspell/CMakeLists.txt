

# simple first try: add a target "doc" which calls meinproc, and have it depend on meinproc
add_custom_target(doc 
   ${KDE4_MEINPROC_EXECUTABLE} --cache ${CMAKE_CURRENT_BINARY_DIR}/index.cache.bz2 ${CMAKE_CURRENT_SOURCE_DIR}/index.docbook 
   DEPENDS meinproc )

########### install files ###############





