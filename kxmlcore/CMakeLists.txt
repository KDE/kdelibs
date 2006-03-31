kde4_header()

########### next target ###############

set(kxmlcore_LIB_SRCS HashTable.cpp )

kde4_automoc(${kxmlcore_LIB_SRCS})

kde4_add_library(kxmlcore SHARED ${kxmlcore_LIB_SRCS})

set_target_properties(kxmlcore PROPERTIES VERSION 1.2.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} kxmlcore )

########### install files ###############
install_files( /include/kxmlcore FILES HashMap.h         HashTraits.h         HashMapPtrSpec.h           SharedPtr.h          HashSet.h         HashFunctions.h         HashTable.h   Noncopyable.h OwnArrayPtr.h OwnPtr.h)

kde4_footer()
