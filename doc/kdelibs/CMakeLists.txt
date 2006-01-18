kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#KDE_LANG = en
#KDE_MANS = AUTO
#KDE_XSL_MAN_STYLESHEET = $(top_srcdir)/kdoctools/customization/kde-man.xsl
#
#KDE_DOCS_DEPS = $(MEINPROC_DEP)
