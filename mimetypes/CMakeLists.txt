kde4_header()

add_subdirectory( application ) 
add_subdirectory( text ) 
add_subdirectory( audio ) 
add_subdirectory( video ) 
add_subdirectory( image ) 
add_subdirectory( inode ) 
add_subdirectory( message ) 
add_subdirectory( multipart ) 
add_subdirectory( model ) 
add_subdirectory( uri ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${KDE4_MIME_DIR}/all FILES all.desktop allfiles.desktop )

kde4_footer()



#original Makefile.am contents follow:

#
#SUBDIRS = application text audio video image inode message multipart model uri
#
#mimetypesdata_DATA = all.desktop allfiles.desktop
#mimetypesdatadir = $(kde_mimedir)/all
