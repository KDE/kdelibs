kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#EXTRA_DIST = $(srcdir)/makeaddressee \
#				$(srcdir)/addressee.src.cpp \
#				$(srcdir)/addressee.src.h \
#				$(srcdir)/entrylist \
#				$(srcdir)/field.src.cpp
#
