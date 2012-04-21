

project(interfaces)

add_subdirectory( ktexteditor )
add_subdirectory( kregexpeditor )
if (NOT KDE_NO_PHONON)
  add_subdirectory( kmediaplayer )
endif()
add_subdirectory( kio )
add_subdirectory( terminal )
add_subdirectory( khexedit )
add_subdirectory( kimproxy )
add_subdirectory( kspeech )

########### install files ###############





