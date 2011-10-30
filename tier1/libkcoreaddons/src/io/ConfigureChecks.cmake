####### checks for kdecore/io ###############
include(CheckIncludeFile)


if(WIN32)
    if (NOT HAVE_QFILESYSTEMWATCHER)
        # check_include_file tries to compile a file including only QtCore/QFileSystemWatcher 
        # this do not work at least with msvc 
        find_file(_HAVE_QFILESYSTEMWATCHER 
            NAMES QFileSystemWatcher
            PATHS ${QT_INCLUDE_DIR}/QtCore
        )
        if(_HAVE_QFILESYSTEMWATCHER)
            set (HAVE_QFILESYSTEMWATCHER 1)
            message(STATUS "looking for QtCore/QFileSystemWatcher - found")
        else(_HAVE_QFILESYSTEMWATCHER)
            message(STATUS "looking for QtCore/QFileSystemWatcher - not found")
        endif(_HAVE_QFILESYSTEMWATCHER)
    endif (NOT HAVE_QFILESYSTEMWATCHER)
else(WIN32)
	# The following statement is probably be used on unix
    # I leave it still disabled because I cannot verify this
    #check_include_file(QtCore/QFileSystemWatcher  HAVE_QFILESYSTEMWATCHER)
endif(WIN32)
