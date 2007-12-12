find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)

include(NepomukMacros)

NEPOMUK_GENERATE_FROM_ONTOLOGY(
   ${CMAKE_SOURCE_DIR}/../../ontologies/nao.nrl
   ${CMAKE_SOURCE_DIR}
   DUMMY1
   DUMMY2
   DUMMY3
   ${CMAKE_SOURCE_DIR}/../resource.h.in
   ${CMAKE_SOURCE_DIR}/../resource.cpp.in
 )
