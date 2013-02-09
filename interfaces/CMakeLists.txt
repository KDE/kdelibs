

project(interfaces)

add_subdirectory( kregexpeditor )
find_package(Phonon4Qt5 4.6.60)
if (Phonon4Qt5_FOUND)
    add_subdirectory( kmediaplayer )
endif (Phonon4Qt5_FOUND)
add_subdirectory( kio )
add_subdirectory( terminal )
add_subdirectory( khexedit )
add_subdirectory( kimproxy )
add_subdirectory( kspeech )

########### install files ###############





