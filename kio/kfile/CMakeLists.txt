add_subdirectory(tests)

########### install files ###############

install( FILES kpropsdlgplugin.desktop  DESTINATION  ${SERVICETYPES_INSTALL_DIR} )
install( FILES kfiledialog.h kencodingfiledialog.h    kdiroperator.h kfileview.h kfilefiltercombo.h   kfiledetailview.h kcombiview.h kdiskfreesp.h    kfileiconview.h krecentdocument.h   kurlrequester.h kfilepreview.h kfile.h    kurlcombobox.h kurlrequesterdlg.h kopenwith.h kpropsdlg.h   kicondialog.h kdirsize.h kpreviewwidgetbase.h kimagefilepreview.h kfilesharedlg.h   kfiletreeview.h kfiletreeviewitem.h kfiletreebranch.h    kdirselectdialog.h kurlbar.h kpropertiesdialog.h knotifydialog.h     kcustommenueditor.h ${CMAKE_CURRENT_BINARY_DIR}/../ui_knotifywidgetbase.h  DESTINATION  ${INCLUDE_INSTALL_DIR} )

