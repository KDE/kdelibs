kde4_header()

if(KDE4_BUILD_TESTS)

include_directories( ${CMAKE_SOURCE_DIR}/kdefx ${CMAKE_SOURCE_DIR}/kdecore ${CMAKE_SOURCE_DIR}/kdeui ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kaboutdialogtest_SRCS
kaboutdialogtest.cpp
)

kde4_automoc(${kaboutdialogtest_SRCS})

kde4_add_executable(kaboutdialogtest ${kaboutdialogtest_SRCS})

target_link_libraries(kaboutdialogtest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kblendtest_SRCS
kblendtest.cpp
)

kde4_automoc(${kblendtest_SRCS})

kde4_add_executable(kblendtest ${kblendtest_SRCS})

target_link_libraries(kblendtest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kbuttonboxtest_SRCS
kbuttonboxtest.cpp
)

kde4_automoc(${kbuttonboxtest_SRCS})

kde4_add_executable(kbuttonboxtest ${kbuttonboxtest_SRCS})

target_link_libraries(kbuttonboxtest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kcharselecttest_SRCS
kcharselecttest.cpp
)

kde4_automoc(${kcharselecttest_SRCS})

kde4_add_executable(kcharselecttest ${kcharselecttest_SRCS})

target_link_libraries(kcharselecttest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kcolortest_SRCS
kcolortest.cpp
)

kde4_automoc(${kcolortest_SRCS})

kde4_add_executable(kcolortest ${kcolortest_SRCS})

target_link_libraries(kcolortest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kcolordlgtest_SRCS
kcolordlgtest.cpp
)

kde4_automoc(${kcolordlgtest_SRCS})

kde4_add_executable(kcolordlgtest ${kcolordlgtest_SRCS})

target_link_libraries(kcolordlgtest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kcomboboxtest_SRCS
kcomboboxtest.cpp
)

kde4_automoc(${kcomboboxtest_SRCS})

kde4_add_executable(kcomboboxtest ${kcomboboxtest_SRCS})

target_link_libraries(kcomboboxtest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kcompletiontest_SRCS
kcompletiontest.cpp
)

kde4_automoc(${kcompletiontest_SRCS})

kde4_add_executable(kcompletiontest ${kcompletiontest_SRCS})

target_link_libraries(kcompletiontest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kdatepicktest_SRCS
kdatepicktest.cpp
)

kde4_automoc(${kdatepicktest_SRCS})

kde4_add_executable(kdatepicktest ${kdatepicktest_SRCS})

target_link_libraries(kdatepicktest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kdatewidgettest_SRCS
kdatewidgettest.cpp
)

kde4_automoc(${kdatewidgettest_SRCS})

kde4_add_executable(kdatewidgettest ${kdatewidgettest_SRCS})

target_link_libraries(kdatewidgettest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kdialogbasetest_SRCS
kdialogbasetest.cpp
)

kde4_automoc(${kdialogbasetest_SRCS})

kde4_add_executable(kdialogbasetest ${kdialogbasetest_SRCS})

target_link_libraries(kdialogbasetest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kdualcolortest_SRCS
kdualcolortest.cpp
)

kde4_automoc(${kdualcolortest_SRCS})

kde4_add_executable(kdualcolortest ${kdualcolortest_SRCS})

target_link_libraries(kdualcolortest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(keditlistboxtest_SRCS
keditlistboxtest.cpp
)

kde4_automoc(${keditlistboxtest_SRCS})

kde4_add_executable(keditlistboxtest ${keditlistboxtest_SRCS})

target_link_libraries(keditlistboxtest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kfontdialogtest_SRCS
kfontdialogtest.cpp
)

kde4_automoc(${kfontdialogtest_SRCS})

kde4_add_executable(kfontdialogtest ${kfontdialogtest_SRCS})

target_link_libraries(kfontdialogtest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kgradienttest_SRCS
kgradienttest.cpp
)

kde4_automoc(${kgradienttest_SRCS})

kde4_add_executable(kgradienttest ${kgradienttest_SRCS})

target_link_libraries(kgradienttest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(khashtest_SRCS
khashtest.cpp
)

kde4_automoc(${khashtest_SRCS})

kde4_add_executable(khashtest ${khashtest_SRCS})

target_link_libraries(khashtest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kdesattest_SRCS
kdesattest.cpp
)

kde4_automoc(${kdesattest_SRCS})

kde4_add_executable(kdesattest ${kdesattest_SRCS})

target_link_libraries(kdesattest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kledtest_SRCS
kledtest.cpp
)

kde4_automoc(${kledtest_SRCS})

kde4_add_executable(kledtest ${kledtest_SRCS})

target_link_libraries(kledtest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(klineedittest_SRCS
klineedittest.cpp
)

kde4_automoc(${klineedittest_SRCS})

kde4_add_executable(klineedittest ${klineedittest_SRCS})

target_link_libraries(klineedittest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kmessageboxtest_SRCS
kmessageboxtest.cpp
)

kde4_automoc(${kmessageboxtest_SRCS})

kde4_add_executable(kmessageboxtest ${kmessageboxtest_SRCS})

target_link_libraries(kmessageboxtest  ${QT_AND_KDECORE_LIBS} kdeui)

########### next target ###############

set(knuminputtest_SRCS
knuminputtest.cpp
)

kde4_automoc(${knuminputtest_SRCS})

kde4_add_executable(knuminputtest ${knuminputtest_SRCS})

target_link_libraries(knuminputtest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kpalettetest_SRCS
kpalettetest.cpp
)

kde4_automoc(${kpalettetest_SRCS})

kde4_add_executable(kpalettetest ${kpalettetest_SRCS})

target_link_libraries(kpalettetest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kpopuptest_SRCS
kpopuptest.cpp
)

kde4_automoc(${kpopuptest_SRCS})

kde4_add_executable(kpopuptest ${kpopuptest_SRCS})

target_link_libraries(kpopuptest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kprogresstest_SRCS
kprogresstest.cpp
)

kde4_automoc(${kprogresstest_SRCS})

#kde4_add_executable(kprogresstest ${kprogresstest_SRCS})

#target_link_libraries(kprogresstest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(krulertest_SRCS
krulertest.cpp
)

kde4_automoc(${krulertest_SRCS})

kde4_add_executable(krulertest ${krulertest_SRCS})

target_link_libraries(krulertest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kseparatortest_SRCS
kseparatortest.cpp
)

kde4_automoc(${kseparatortest_SRCS})

kde4_add_executable(kseparatortest ${kseparatortest_SRCS})

target_link_libraries(kseparatortest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kstatusbartest_SRCS
kstatusbartest.cpp
)

kde4_automoc(${kstatusbartest_SRCS})

kde4_add_executable(kstatusbartest ${kstatusbartest_SRCS})

target_link_libraries(kstatusbartest  ${QT_AND_KDECORE_LIBS} kdeui)

########### next target ###############

set(ktabctltest_SRCS
ktabctltest.cpp
)

kde4_automoc(${ktabctltest_SRCS})

kde4_add_executable(ktabctltest ${ktabctltest_SRCS})

target_link_libraries(ktabctltest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kunbalancedgrdtest_SRCS
kunbalancedgrdtest.cpp
)

kde4_automoc(${kunbalancedgrdtest_SRCS})

kde4_add_executable(kunbalancedgrdtest ${kunbalancedgrdtest_SRCS})

target_link_libraries(kunbalancedgrdtest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kwindowtest_SRCS
kwindowtest.cpp
)

kde4_automoc(${kwindowtest_SRCS})

kde4_add_executable(kwindowtest ${kwindowtest_SRCS})

target_link_libraries(kwindowtest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(itemcontainertest_SRCS
itemcontainertest.cpp
)

kde4_automoc(${itemcontainertest_SRCS})

kde4_add_executable(itemcontainertest ${itemcontainertest_SRCS})

target_link_libraries(itemcontainertest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kedittest_SRCS
kedittest.cpp
)

kde4_automoc(${kedittest_SRCS})

kde4_add_executable(kedittest ${kedittest_SRCS})

target_link_libraries(kedittest  ${QT_AND_KDECORE_LIBS} kdeui )


########### next target ###############

set(kxmlguitest_SRCS
kxmlguitest.cpp
)

kde4_automoc(${kxmlguitest_SRCS})

kde4_add_executable(kxmlguitest ${kxmlguitest_SRCS})

target_link_libraries(kxmlguitest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kaccelgentest_SRCS
kaccelgentest.cpp
)

kde4_automoc(${kaccelgentest_SRCS})

kde4_add_executable(kaccelgentest ${kaccelgentest_SRCS})

target_link_libraries(kaccelgentest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(ksystemtraytest_SRCS
ksystemtraytest.cpp
)

kde4_automoc(${ksystemtraytest_SRCS})

kde4_add_executable(ksystemtraytest ${ksystemtraytest_SRCS})

target_link_libraries(ksystemtraytest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(klistviewtest_SRCS
klistviewtest.cpp
)

kde4_automoc(${klistviewtest_SRCS})

kde4_add_executable(klistviewtest ${klistviewtest_SRCS})

target_link_libraries(klistviewtest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(ktextedittest_SRCS
ktextedittest.cpp
)

kde4_automoc(${ktextedittest_SRCS})

kde4_add_executable(ktextedittest ${ktextedittest_SRCS})

target_link_libraries(ktextedittest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kmainwindowtest_SRCS
kmainwindowtest.cpp
)

kde4_automoc(${kmainwindowtest_SRCS})

kde4_add_executable(kmainwindowtest ${kmainwindowtest_SRCS})

target_link_libraries(kmainwindowtest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kmainwindowrestoretest_SRCS
kmainwindowrestoretest.cpp
)

kde4_automoc(${kmainwindowrestoretest_SRCS})

kde4_add_executable(kmainwindowrestoretest ${kmainwindowrestoretest_SRCS})

target_link_libraries(kmainwindowrestoretest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kactiontest_SRCS
kactiontest.cpp
)

kde4_automoc(${kactiontest_SRCS})

kde4_add_executable(kactiontest ${kactiontest_SRCS})

target_link_libraries(kactiontest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kdatetimewidgettest_SRCS
kdatetimewidgettest.cpp
)

kde4_automoc(${kdatetimewidgettest_SRCS})

kde4_add_executable(kdatetimewidgettest ${kdatetimewidgettest_SRCS})

target_link_libraries(kdatetimewidgettest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kspelltest_SRCS
kspelltest.cpp
)

kde4_automoc(${kspelltest_SRCS})

kde4_add_executable(kspelltest ${kspelltest_SRCS})

target_link_libraries(kspelltest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kinputdialogtest_SRCS
kinputdialogtest.cpp
)

kde4_automoc(${kinputdialogtest_SRCS})

kde4_add_executable(kinputdialogtest ${kinputdialogtest_SRCS})

target_link_libraries(kinputdialogtest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(ktabwidgettest_SRCS
ktabwidgettest.cpp
)

kde4_automoc(${ktabwidgettest_SRCS})

kde4_add_executable(ktabwidgettest ${ktabwidgettest_SRCS})

target_link_libraries(ktabwidgettest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kwordwraptest_SRCS
kwordwraptest.cpp
)

kde4_automoc(${kwordwraptest_SRCS})

kde4_add_executable(kwordwraptest ${kwordwraptest_SRCS})

target_link_libraries(kwordwraptest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(ksqueezedtextlabeltest_SRCS
ksqueezedtextlabeltest.cpp
)

kde4_automoc(${ksqueezedtextlabeltest_SRCS})

kde4_add_executable(ksqueezedtextlabeltest ${ksqueezedtextlabeltest_SRCS})

target_link_libraries(ksqueezedtextlabeltest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kpixmapregionselectordialogtest_SRCS
kpixmapregionselectordialogtest.cpp
)

kde4_automoc(${kpixmapregionselectordialogtest_SRCS})

kde4_add_executable(kpixmapregionselectordialogtest ${kpixmapregionselectordialogtest_SRCS})

target_link_libraries(kpixmapregionselectordialogtest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kjanuswidgettest_SRCS
kjanuswidgettest.cpp
)

kde4_automoc(${kjanuswidgettest_SRCS})

kde4_add_executable(kjanuswidgettest ${kjanuswidgettest_SRCS})

target_link_libraries(kjanuswidgettest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(ktoolbarlabelactiontest_SRCS
ktoolbarlabelactiontest.cpp
)

kde4_automoc(${ktoolbarlabelactiontest_SRCS})

kde4_add_executable(ktoolbarlabelactiontest ${ktoolbarlabelactiontest_SRCS})

target_link_libraries(ktoolbarlabelactiontest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(khboxtest_SRCS
khboxtest.cpp
)

kde4_automoc(${khboxtest_SRCS})

kde4_add_executable(khboxtest ${khboxtest_SRCS})

target_link_libraries(khboxtest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kbugreporttest_SRCS
kbugreporttest.cpp
)

kde4_automoc(${kbugreporttest_SRCS})

kde4_add_executable(kbugreporttest ${kbugreporttest_SRCS})

target_link_libraries(kbugreporttest  ${QT_AND_KDECORE_LIBS} kdeui)


########### next target ###############

set(kdialogtest_SRCS
kdialogtest.cpp
)

kde4_automoc(${kdialogtest_SRCS})

kde4_add_executable(kdialogtest ${kdialogtest_SRCS})

target_link_libraries(kdialogtest  ${QT_AND_KDECORE_LIBS} kdeui)

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
