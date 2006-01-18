kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${KDE4_DATA_DIR}/kdewidgets/pics FILES klineedit.png ktextbrowser.png kpasswordedit.png kdatewidget.png               kfontrequester.png kurlrequester.png kcombobox.png kiconview.png kintspinbox.png               kled.png ksqueezedtextlabel.png  kprogress.png kurllabel.png               kruler.png kcmodule.png kdoublespinbox.png kdoublenuminput.png               klistbox.png klistview.png kactivelabel.png kiconbutton.png               kcolorcombo.png khsselector.png kcolorbutton.png               kgradientselector.png kdualcolorbutton.png               kpushbutton.png kcharselect.png kdatepicker.png               kdialog.png keditlistbox.png kfontcombo.png khistorycombo.png               kintnuminput.png krestrictedline.png ktextedit.png               kurlcomborequester.png kkeybutton.png kpalettetable.png  	     kactionselector.png kdatetable.png kdatetimewidget.png  )

kde4_footer()



#original Makefile.am contents follow:

#kdewidgetsdata_DATA =  klineedit.png ktextbrowser.png kpasswordedit.png kdatewidget.png \
#             kfontrequester.png kurlrequester.png kcombobox.png kiconview.png kintspinbox.png \
#             kled.png ksqueezedtextlabel.png  kprogress.png kurllabel.png \
#             kruler.png kcmodule.png kdoublespinbox.png kdoublenuminput.png \
#             klistbox.png klistview.png kactivelabel.png kiconbutton.png \
#             kcolorcombo.png khsselector.png kcolorbutton.png \
#             kgradientselector.png kdualcolorbutton.png \
#             kpushbutton.png kcharselect.png kdatepicker.png \
#             kdialog.png keditlistbox.png kfontcombo.png khistorycombo.png \
#             kintnuminput.png krestrictedline.png ktextedit.png \
#             kurlcomborequester.png kkeybutton.png kpalettetable.png \
#	     kactionselector.png kdatetable.png kdatetimewidget.png 
#
#
#kdewidgetsdatadir = $(kde_datadir)/kdewidgets/pics
#
#EXTRA_DIST = $(kdewidgetsdata_DATA)
