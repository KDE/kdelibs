add_subdirectory(kmediaplayer)
if (Phonon4Qt5_FOUND AND Qt5DBus_FOUND)
   add_subdirectory(kfileaudiopreview)
endif ()
