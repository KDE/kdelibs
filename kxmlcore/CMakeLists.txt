
########### next target ###############

set(kxmlcore_LIB_SRCS HashTable.cpp )

kde4_automoc(${kxmlcore_LIB_SRCS})

kde4_add_library(kxmlcore SHARED ${kxmlcore_LIB_SRCS})

set_target_properties(kxmlcore PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kxmlcore  DESTINATION ${LIB_INSTALL_DIR} )

########### install files ###############
install( FILES HashMap.h         HashTraits.h         HashMapPtrSpec.h           SharedPtr.h          HashSet.h         HashFunctions.h         HashTable.h   Noncopyable.h OwnArrayPtr.h OwnPtr.h DESTINATION  ${INCLUDE_INSTALL_DIR}/kxmlcore )

