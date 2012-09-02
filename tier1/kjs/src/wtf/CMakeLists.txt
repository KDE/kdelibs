
########### next target ###############
#add_definitions(-DBUILDING_KDE__)
#include_directories( ${_KDE4_PLATFORM_INCLUDE_DIRS} )

#set(wtf_LIB_SRCS HashTable.cpp )

#kde4_add_library(wtf SHARED ${wtf_LIB_SRCS})

#set_target_properties(wtf PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
#install(TARGETS wtf ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############
# FIXME: Actually, most of these should NOT be installed. The only
# ones that need be are those needed from instaled KJS headers
# KJS headers are no longer installed, so don't install WTF headers either
#install( FILES AlwaysInline.h Assertions.h FastMalloc.h HashCountedSet.h HashMap.h HashSet.h HashFunctions.h HashTable.h HashTraits.h Noncopyable.h OwnArrayPtr.h OwnPtr.h PassRefPtr.h Platform.h RefPtr.h SharedPtr.h Vector.h VectorTraits.h DESTINATION  ${INCLUDE_INSTALL_DIR}/wtf )

