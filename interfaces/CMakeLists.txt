

project(interfaces)

add_subdirectory( kregexpeditor )
find_package(Phonon4Qt5 4.6.60)
if (PHONON_FOUND)
    add_subdirectory( kmediaplayer )
endif (PHONON_FOUND)
add_subdirectory( kio )
add_subdirectory( terminal )
add_subdirectory( khexedit )
add_subdirectory( kimproxy )
add_subdirectory( kspeech )

########### install files ###############





