kde4_header()

if(KDE4_BUILD_TESTS)

# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${KDE4_KDEUI_INCLUDES} )


########### next target ###############

set(kaboutdialogtest_SRCS
kaboutdialogtest.cpp
)

kde4_automoc(${kaboutdialogtest_SRCS})

kde4_add_executable(kaboutdialogtest RUN_UNINSTALLED ${kaboutdialogtest_SRCS})

target_link_libraries(kaboutdialogtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kblendtest_SRCS
kblendtest.cpp
)

kde4_automoc(${kblendtest_SRCS})

kde4_add_executable(kblendtest RUN_UNINSTALLED ${kblendtest_SRCS})

target_link_libraries(kblendtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kbuttonboxtest_SRCS
kbuttonboxtest.cpp
)

kde4_automoc(${kbuttonboxtest_SRCS})

kde4_add_executable(kbuttonboxtest RUN_UNINSTALLED ${kbuttonboxtest_SRCS})

target_link_libraries(kbuttonboxtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kcharselecttest_SRCS
kcharselecttest.cpp
)

kde4_automoc(${kcharselecttest_SRCS})

kde4_add_executable(kcharselecttest RUN_UNINSTALLED ${kcharselecttest_SRCS})

target_link_libraries(kcharselecttest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kcolortest_SRCS
kcolortest.cpp
)

kde4_automoc(${kcolortest_SRCS})

kde4_add_executable(kcolortest RUN_UNINSTALLED ${kcolortest_SRCS})

target_link_libraries(kcolortest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kcolordlgtest_SRCS
kcolordlgtest.cpp
)

kde4_automoc(${kcolordlgtest_SRCS})

kde4_add_executable(kcolordlgtest RUN_UNINSTALLED ${kcolordlgtest_SRCS})

target_link_libraries(kcolordlgtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kcomboboxtest_SRCS
kcomboboxtest.cpp
)

kde4_automoc(${kcomboboxtest_SRCS})

kde4_add_executable(kcomboboxtest RUN_UNINSTALLED ${kcomboboxtest_SRCS})

target_link_libraries(kcomboboxtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kcompletiontest_kdeui_SRCS
kcompletionuitest.cpp
)

kde4_automoc(${kcompletiontest_kdeui_SRCS})

kde4_add_executable(kcompletiontest_kdeui RUN_UNINSTALLED ${kcompletiontest_kdeui_SRCS})

target_link_libraries(kcompletiontest_kdeui ${KDE4_KDEUI_LIBS})




set(kcompletiontest_SRCS kcompletioncoretest.cpp )

kde4_automoc(${kcompletiontest_SRCS})

kde4_add_executable(kcompletiontest RUN_UNINSTALLED ${kcompletiontest_SRCS})

target_link_libraries(kcompletiontest  ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kcompletiontest ${EXECUTABLE_OUTPUT_PATH}/kcompletiontest)



########### next target ###############

set(kdatepicktest_SRCS
kdatepicktest.cpp
)

kde4_automoc(${kdatepicktest_SRCS})

kde4_add_executable(kdatepicktest RUN_UNINSTALLED ${kdatepicktest_SRCS})

target_link_libraries(kdatepicktest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kdatewidgettest_SRCS
kdatewidgettest.cpp
)

kde4_automoc(${kdatewidgettest_SRCS})

kde4_add_executable(kdatewidgettest RUN_UNINSTALLED ${kdatewidgettest_SRCS})

target_link_libraries(kdatewidgettest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kdialogbasetest_SRCS
kdialogbasetest.cpp
)

kde4_automoc(${kdialogbasetest_SRCS})

kde4_add_executable(kdialogbasetest RUN_UNINSTALLED ${kdialogbasetest_SRCS})

target_link_libraries(kdialogbasetest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kdualcolortest_SRCS
kdualcolortest.cpp
)

kde4_automoc(${kdualcolortest_SRCS})

kde4_add_executable(kdualcolortest RUN_UNINSTALLED ${kdualcolortest_SRCS})

target_link_libraries(kdualcolortest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(keditlistboxtest_SRCS
keditlistboxtest.cpp
)

kde4_automoc(${keditlistboxtest_SRCS})

kde4_add_executable(keditlistboxtest RUN_UNINSTALLED ${keditlistboxtest_SRCS})

target_link_libraries(keditlistboxtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kfontdialogtest_SRCS
kfontdialogtest.cpp
)

kde4_automoc(${kfontdialogtest_SRCS})

kde4_add_executable(kfontdialogtest RUN_UNINSTALLED ${kfontdialogtest_SRCS})

target_link_libraries(kfontdialogtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kgradienttest_SRCS
kgradienttest.cpp
)

kde4_automoc(${kgradienttest_SRCS})

kde4_add_executable(kgradienttest RUN_UNINSTALLED ${kgradienttest_SRCS})

target_link_libraries(kgradienttest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(khashtest_SRCS
khashtest.cpp
)

kde4_automoc(${khashtest_SRCS})

kde4_add_executable(khashtest RUN_UNINSTALLED ${khashtest_SRCS})

target_link_libraries(khashtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kdesattest_SRCS
kdesattest.cpp
)

kde4_automoc(${kdesattest_SRCS})

kde4_add_executable(kdesattest RUN_UNINSTALLED ${kdesattest_SRCS})

target_link_libraries(kdesattest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kledtest_SRCS
kledtest.cpp
)

kde4_automoc(${kledtest_SRCS})

kde4_add_executable(kledtest RUN_UNINSTALLED ${kledtest_SRCS})

target_link_libraries(kledtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(klineedittest_SRCS
klineedittest.cpp
)

kde4_automoc(${klineedittest_SRCS})

kde4_add_executable(klineedittest RUN_UNINSTALLED ${klineedittest_SRCS})

target_link_libraries(klineedittest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kmessageboxtest_SRCS
kmessageboxtest.cpp
)

kde4_automoc(${kmessageboxtest_SRCS})

kde4_add_executable(kmessageboxtest RUN_UNINSTALLED ${kmessageboxtest_SRCS})

target_link_libraries(kmessageboxtest ${KDE4_KDEUI_LIBS})

########### next target ###############

set(knuminputtest_SRCS
knuminputtest.cpp
)

kde4_automoc(${knuminputtest_SRCS})

kde4_add_executable(knuminputtest RUN_UNINSTALLED ${knuminputtest_SRCS})

target_link_libraries(knuminputtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kpalettetest_SRCS
kpalettetest.cpp
)

kde4_automoc(${kpalettetest_SRCS})

kde4_add_executable(kpalettetest RUN_UNINSTALLED ${kpalettetest_SRCS})

target_link_libraries(kpalettetest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kpopuptest_SRCS
kpopuptest.cpp
)

kde4_automoc(${kpopuptest_SRCS})

kde4_add_executable(kpopuptest RUN_UNINSTALLED ${kpopuptest_SRCS})

target_link_libraries(kpopuptest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kprogressbartest_SRCS
kprogressbartest.cpp
)

kde4_automoc(${kprogressbartest_SRCS})

kde4_add_executable(kprogressbartest RUN_UNINSTALLED ${kprogressbartest_SRCS})

target_link_libraries(kprogressbartest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kprogressdialogtest_SRCS
kprogressdialogtest.cpp
)

kde4_automoc(${kprogressdialogtest_SRCS})

kde4_add_executable(kprogressdialogtest RUN_UNINSTALLED ${kprogressdialogtest_SRCS})

target_link_libraries(kprogressdialogtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(krulertest_SRCS
krulertest.cpp
)

kde4_automoc(${krulertest_SRCS})

kde4_add_executable(krulertest RUN_UNINSTALLED ${krulertest_SRCS})

target_link_libraries(krulertest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kseparatortest_SRCS
kseparatortest.cpp
)

kde4_automoc(${kseparatortest_SRCS})

kde4_add_executable(kseparatortest RUN_UNINSTALLED ${kseparatortest_SRCS})

target_link_libraries(kseparatortest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kstatusbartest_SRCS
kstatusbartest.cpp
)

kde4_automoc(${kstatusbartest_SRCS})

kde4_add_executable(kstatusbartest RUN_UNINSTALLED ${kstatusbartest_SRCS})

target_link_libraries(kstatusbartest ${KDE4_KDEUI_LIBS})

########### next target ###############

set(ktabctltest_SRCS
ktabctltest.cpp
)

kde4_automoc(${ktabctltest_SRCS})

kde4_add_executable(ktabctltest RUN_UNINSTALLED ${ktabctltest_SRCS})

target_link_libraries(ktabctltest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kunbalancedgrdtest_SRCS
kunbalancedgrdtest.cpp
)

kde4_automoc(${kunbalancedgrdtest_SRCS})

kde4_add_executable(kunbalancedgrdtest RUN_UNINSTALLED ${kunbalancedgrdtest_SRCS})

target_link_libraries(kunbalancedgrdtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kwindowtest_SRCS
kwindowtest.cpp
)

kde4_automoc(${kwindowtest_SRCS})

kde4_add_executable(kwindowtest RUN_UNINSTALLED ${kwindowtest_SRCS})

target_link_libraries(kwindowtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(itemcontainertest_SRCS
itemcontainertest.cpp
)

kde4_automoc(${itemcontainertest_SRCS})

kde4_add_executable(itemcontainertest RUN_UNINSTALLED ${itemcontainertest_SRCS})

target_link_libraries(itemcontainertest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kedittest_SRCS
kedittest.cpp
)

kde4_automoc(${kedittest_SRCS})

kde4_add_executable(kedittest RUN_UNINSTALLED ${kedittest_SRCS})

target_link_libraries(kedittest ${KDE4_KDEUI_LIBS} )


########### next target ###############

set(kxmlguitest_SRCS
kxmlguitest.cpp
)

kde4_automoc(${kxmlguitest_SRCS})

kde4_add_executable(kxmlguitest RUN_UNINSTALLED ${kxmlguitest_SRCS})

target_link_libraries(kxmlguitest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kaccelgentest_SRCS
kaccelgentest.cpp
)

kde4_automoc(${kaccelgentest_SRCS})

kde4_add_executable(kaccelgentest RUN_UNINSTALLED ${kaccelgentest_SRCS})

target_link_libraries(kaccelgentest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(ksystemtraytest_SRCS
ksystemtraytest.cpp
)

kde4_automoc(${ksystemtraytest_SRCS})

kde4_add_executable(ksystemtraytest RUN_UNINSTALLED ${ksystemtraytest_SRCS})

target_link_libraries(ksystemtraytest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(k3listviewtest_SRCS
k3listviewtest.cpp
)

kde4_automoc(${k3listviewtest_SRCS})

kde4_add_executable(k3listviewtest RUN_UNINSTALLED ${k3listviewtest_SRCS})

target_link_libraries(k3listviewtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(ktreewidgetsearchlinetest_SRCS
ktreewidgetsearchlinetest.cpp
)

kde4_automoc(${ktreewidgetsearchlinetest_SRCS})

kde4_add_executable(ktreewidgetsearchlinetest RUN_UNINSTALLED ${ktreewidgetsearchlinetest_SRCS})

target_link_libraries(ktreewidgetsearchlinetest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(ktextedittest_SRCS
ktextedittest.cpp
)

kde4_automoc(${ktextedittest_SRCS})

kde4_add_executable(ktextedittest RUN_UNINSTALLED ${ktextedittest_SRCS})

target_link_libraries(ktextedittest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kmainwindowtest_SRCS
kmainwindowtest.cpp
)

kde4_automoc(${kmainwindowtest_SRCS})

kde4_add_executable(kmainwindowtest RUN_UNINSTALLED ${kmainwindowtest_SRCS})

target_link_libraries(kmainwindowtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kmainwindowrestoretest_SRCS
kmainwindowrestoretest.cpp
)

kde4_automoc(${kmainwindowrestoretest_SRCS})

kde4_add_executable(kmainwindowrestoretest RUN_UNINSTALLED ${kmainwindowrestoretest_SRCS})

target_link_libraries(kmainwindowrestoretest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kactiontest_SRCS
kactiontest.cpp
)

kde4_automoc(${kactiontest_SRCS})

kde4_add_executable(kactiontest RUN_UNINSTALLED ${kactiontest_SRCS})

target_link_libraries(kactiontest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(ktoolbartest_SRCS
ktoolbartest.cpp
)

kde4_automoc(${ktoolbartest_SRCS})

kde4_add_executable(ktoolbartest RUN_UNINSTALLED ${ktoolbartest_SRCS})

target_link_libraries(ktoolbartest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kdatetimewidgettest_SRCS
kdatetimewidgettest.cpp
)

kde4_automoc(${kdatetimewidgettest_SRCS})

kde4_add_executable(kdatetimewidgettest RUN_UNINSTALLED ${kdatetimewidgettest_SRCS})

target_link_libraries(kdatetimewidgettest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kspelltest_SRCS
kspelltest.cpp
)

kde4_automoc(${kspelltest_SRCS})

kde4_add_executable(kspelltest RUN_UNINSTALLED ${kspelltest_SRCS})

target_link_libraries(kspelltest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kinputdialogtest_SRCS
kinputdialogtest.cpp
)

kde4_automoc(${kinputdialogtest_SRCS})

kde4_add_executable(kinputdialogtest RUN_UNINSTALLED ${kinputdialogtest_SRCS})

target_link_libraries(kinputdialogtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(ktabwidgettest_SRCS
ktabwidgettest.cpp
)

kde4_automoc(${ktabwidgettest_SRCS})

kde4_add_executable(ktabwidgettest RUN_UNINSTALLED ${ktabwidgettest_SRCS})

target_link_libraries(ktabwidgettest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kwordwraptest_SRCS
kwordwraptest.cpp
)

kde4_automoc(${kwordwraptest_SRCS})

kde4_add_executable(kwordwraptest RUN_UNINSTALLED ${kwordwraptest_SRCS})

target_link_libraries(kwordwraptest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(ksqueezedtextlabeltest_SRCS
ksqueezedtextlabeltest.cpp
)

kde4_automoc(${ksqueezedtextlabeltest_SRCS})

kde4_add_executable(ksqueezedtextlabeltest RUN_UNINSTALLED ${ksqueezedtextlabeltest_SRCS})

target_link_libraries(ksqueezedtextlabeltest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kpixmapregionselectordialogtest_SRCS
kpixmapregionselectordialogtest.cpp
)

kde4_automoc(${kpixmapregionselectordialogtest_SRCS})

kde4_add_executable(kpixmapregionselectordialogtest RUN_UNINSTALLED ${kpixmapregionselectordialogtest_SRCS})

target_link_libraries(kpixmapregionselectordialogtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kjanuswidgettest_SRCS
kjanuswidgettest.cpp
)

kde4_automoc(${kjanuswidgettest_SRCS})

kde4_add_executable(kjanuswidgettest RUN_UNINSTALLED ${kjanuswidgettest_SRCS})

target_link_libraries(kjanuswidgettest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(ktoolbarlabelactiontest_SRCS
ktoolbarlabelactiontest.cpp
)

kde4_automoc(${ktoolbarlabelactiontest_SRCS})

kde4_add_executable(ktoolbarlabelactiontest RUN_UNINSTALLED ${ktoolbarlabelactiontest_SRCS})

target_link_libraries(ktoolbarlabelactiontest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(khboxtest_SRCS
khboxtest.cpp
)

kde4_automoc(${khboxtest_SRCS})

kde4_add_executable(khboxtest RUN_UNINSTALLED ${khboxtest_SRCS})

target_link_libraries(khboxtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kbugreporttest_SRCS
kbugreporttest.cpp
)

kde4_automoc(${kbugreporttest_SRCS})

kde4_add_executable(kbugreporttest RUN_UNINSTALLED ${kbugreporttest_SRCS})

target_link_libraries(kbugreporttest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kdialogtest_SRCS
kdialogtest.cpp
)

kde4_automoc(${kdialogtest_SRCS})

kde4_add_executable(kdialogtest RUN_UNINSTALLED ${kdialogtest_SRCS})

target_link_libraries(kdialogtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kglobalacceltest_SRCS
kglobalacceltest.cpp
)

kde4_automoc(${kglobalacceltest_SRCS})

kde4_add_executable(kglobalacceltest RUN_UNINSTALLED ${kglobalacceltest_SRCS})

target_link_libraries(kglobalacceltest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kactivelabeltest_SRCS
kactivelabeltest.cpp
)

kde4_automoc(${kactivelabeltest_SRCS})

kde4_add_executable(kactivelabeltest RUN_UNINSTALLED ${kactivelabeltest_SRCS})

target_link_libraries(kactivelabeltest ${KDE4_KDEUI_LIBS})

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

##	This file is part of the KDE libraries
##    Copyright (C) 1996-1997 Matthias Kalle Dalheimer (kalle@kde.org)
##              (C) 1997-1998 Stephan Kulow (coolo@kde.org)
#
##    This library is free software; you can redistribute it and/or
##    modify it under the terms of the GNU Library General Public
##    License as published by the Free Software Foundation; either
##    version 2 of the License, or (at your option) any later version.
#
##    This library is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
##    Library General Public License for more details.
#
##    You should have received a copy of the GNU Library General Public License
##    along with this library; see the file COPYING.LIB.  If not, write to
##    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
##    Boston, MA 02110-1301, USA.
#
#INCLUDES = -I$(top_srcdir)/kdefx -I$(top_srcdir)/kdecore -I$(top_srcdir)/kdeui \
#	  $(all_includes)
#
#AM_LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS) $(KDE_RPATH)
#
#check_PROGRAMS = kaboutdialogtest kblendtest kbuttonboxtest kcharselecttest \
#	kcolortest kcolordlgtest kcomboboxtest kcompletiontest \
#	kdatepicktest kdatewidgettest kdialogbasetest \
#	kdualcolortest keditlistboxtest kfontdialogtest \
#	kgradienttest khashtest kdesattest\
#	kledtest klineedittest \
#	kmessageboxtest knuminputtest \
#	kpalettetest \
#	kpopuptest kprogresstest krulertest kseparatortest \
#	kstatusbartest ktabctltest \
#	kunbalancedgrdtest kwindowtest kwizardtest \
#	itemcontainertest kedittest \
#	kxmlguitest kaccelgentest ksystemtraytest klistviewtest \
#	ktextedittest kmainwindowtest kmainwindowrestoretest \
#	kactiontest kdatetimewidgettest \
#	kspelltest kinputdialogtest ktabwidgettest kwordwraptest \
#	ksqueezedtextlabeltest \
#	kpixmapregionselectordialogtest kjanuswidgettest ktoolbarlabelactiontest \
#	khboxtest kbugreporttest kdialogtest
#
#METASOURCES = AUTO
#
#LDADD = ../libkdeui.la
#
#kaboutdialogtest_SOURCES = kaboutdialogtest.cpp
#kblendtest_SOURCES = kblendtest.cpp
#kdesattest_SOURCES = kdesattest.cpp
#kbuttonboxtest_SOURCES = kbuttonboxtest.cpp
#kcharselecttest_SOURCES = kcharselecttest.cpp
#kcolordlgtest_SOURCES = kcolordlgtest.cpp
#kcolortest_SOURCES = kcolortest.cpp
#kcomboboxtest_SOURCES = kcomboboxtest.cpp
#kcompletiontest_SOURCES = kcompletiontest.cpp
#kdatepicktest_SOURCES = kdatepicktest.cpp
#kdatetimewidgettest_SOURCES = kdatetimewidgettest.cpp
#kdatewidgettest_SOURCES = kdatewidgettest.cpp
#kdialogbasetest_SOURCES = kdialogbasetest.cpp
#kdualcolortest_SOURCES = kdualcolortest.cpp
#kedittest_SOURCES = kedittest.cpp
#keditlistboxtest_SOURCES = keditlistboxtest.cpp
#kfontdialogtest_SOURCES = kfontdialogtest.cpp
#kgradienttest_SOURCES = kgradienttest.cpp
#khashtest_SOURCES = khashtest.cpp
#kledtest_SOURCES = kledtest.cpp
#klineedittest_SOURCES = klineedittest.cpp
#klineeditdlgtest_SOURCES = klineeditdlgtest.cpp
#kmessageboxtest_SOURCES = kmessageboxtest.cpp
#knuminputtest_SOURCES = knuminputtest.cpp
#kpalettetest_SOURCES = kpalettetest.cpp
#kpopuptest_SOURCES = kpopuptest.cpp
#kprogresstest_SOURCES = kprogresstest.cpp
#krulertest_SOURCES = krulertest.cpp
#kseparatortest_SOURCES = kseparatortest.cpp
#kstatusbartest_SOURCES = kstatusbartest.cpp
#ktabctltest_SOURCES = ktabctltest.cpp
#kunbalancedgrdtest_SOURCES = kunbalancedgrdtest.cpp
#kwindowtest_SOURCES = kwindowtest.cpp
#kwizardtest_SOURCES = kwizardtest.cpp
#itemcontainertest_SOURCES = itemcontainertest.cpp
#kxmlguitest_SOURCES = kxmlguitest.cpp
#kaccelgentest_SOURCES = kaccelgentest.cpp
#ksystemtraytest_SOURCES = ksystemtraytest.cpp
#klistviewtest_SOURCES = klistviewtest.cpp
#ktextedittest_SOURCES = ktextedittest.cpp
#kmainwindowtest_SOURCES = kmainwindowtest.cpp
#kmainwindowrestoretest_SOURCES = kmainwindowrestoretest.cpp
#kactiontest_SOURCES = kactiontest.cpp
#kspelltest_SOURCES = kspelltest.cpp
#kinputdialogtest_SOURCES = kinputdialogtest.cpp
#ktabwidgettest_SOURCES = ktabwidgettest.cpp
#kwordwraptest_SOURCES = kwordwraptest.cpp
#qxembedtest_SOURCES=qxembedtest.cpp
#ksqueezedtextlabeltest_SOURCES = ksqueezedtextlabeltest.cpp
#kjanuswidgettest_SOURCES = kjanuswidgettest.cpp
#kpixmapregionselectordialogtest_SOURCES = kpixmapregionselectordialogtest.cpp
#ktoolbarlabelactiontest_SOURCES = ktoolbarlabelactiontest.cpp
#khboxtest_SOURCES = khboxtest.cpp
#kbugreporttest_SOURCES = kbugreporttest.cpp
#kdialogtest_SOURCES = kdialogtest.cpp
#
#
