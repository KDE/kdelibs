
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



########### install files ###############

install_files( ${MIME_INSTALL_DIR}/all FILES all.desktop allfiles.desktop )




#original Makefile.am contents follow:

#
#SUBDIRS = application text audio video image inode message multipart model uri
#
#mimetypesdata_DATA = all.desktop allfiles.desktop
#mimetypesdatadir = $(kde_mimedir)/all
