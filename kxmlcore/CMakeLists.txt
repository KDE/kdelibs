
########### next target ###############
add_definitions(-DBUILDING_KDE__)
include_directories( ${_KDE4_PLATFORM_INCLUDE_DIRS} )

set(kxmlcore_LIB_SRCS HashTable.cpp )

kde4_automoc(${kxmlcore_LIB_SRCS})

kde4_add_library(kxmlcore SHARED ${kxmlcore_LIB_SRCS})

set_target_properties(kxmlcore PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kxmlcore  DESTINATION ${LIB_INSTALL_DIR} )

########### install files ###############
# FIXME: Actually, most of these should NOT be installed. The only
# ones that need be are those needed from instaled KJS headers
install( FILES AlwaysInline.h Assertions.h FastMalloc.h HashCountedSet.h HashMap.h HashSet.h HashFunctions.h HashTable.h HashTraits.h Noncopyable.h OwnArrayPtr.h OwnPtr.h PassRefPtr.h Platform.h RefPtr.h SharedPtr.h DESTINATION  ${INCLUDE_INSTALL_DIR}/kxmlcore )

