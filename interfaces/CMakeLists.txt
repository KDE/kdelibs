

project(interfaces)

add_subdirectory( kregexpeditor )
if (PHONON_FOUND)
    add_subdirectory( kmediaplayer )
endif (PHONON_FOUND)
add_subdirectory( kio )
add_subdirectory( terminal )
add_subdirectory( khexedit )
add_subdirectory( kimproxy )
add_subdirectory( kspeech )

########### install files ###############





