include_directories(${QT_INCLUDES} ${KDE4_KDECORE_INCLUDES} )

########### KFormula Library ###############

set( libkformula_PART_SRCS
   AttributeManager.cpp
   FormulaRenderer.cpp
   FormulaCursor.cpp 
   ElementFactory.cpp 
   BasicElement.cpp 
   RowElement.cpp
   FormulaElement.cpp 
   TextElement.cpp
   FractionElement.cpp
   SpaceElement.cpp
   GlyphElement.cpp
   IdentifierElement.cpp
   OperatorElement.cpp
   MultiscriptElement.cpp
   SubSupElement.cpp
   UnderOverElement.cpp
   FencedElement.cpp 
   TableElement.cpp
   TableRowElement.cpp
   TableEntryElement.cpp
   RootElement.cpp
   EncloseElement.cpp
   ActionElement.cpp
   PaddedElement.cpp
   ErrorElement.cpp
   StyleElement.cpp
   TokenElement.cpp
   NumberElement.cpp
   StringElement.cpp
   PhantomElement.cpp
   SquareRootElement.cpp
   UnknownElement.cpp
   Dictionary.cpp
   KFormula.cpp
)

kde4_add_library(kformula SHARED ${libkformula_PART_SRCS})
target_link_libraries(kformula kdeui ${QT_QTXML_LIBRARY})

set_target_properties(kformula PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
   ${KDE4_DISABLE_PROPERTY_}LINK_INTERFACE_LIBRARIES "kdeui"
)

install(TARGETS kformula DESTINATION ${LIB_INSTALL_DIR})

KDE4_ADD_WIDGET_FILES(kformulawidgets_PART_SRCS kformula.widgets)
kde4_add_plugin(kformulawidgets ${kformulawidgets_PART_SRCS})
target_link_libraries(kformulawidgets kdeui ${QT_QTXML_LIBRARY} kformula)
install(TARGETS kformulawidgets DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer )

install( FILES
  KFormula.h
  kformula_export.h
  DESTINATION  ${INCLUDE_INSTALL_DIR})

add_subdirectory( pics ) 
add_subdirectory( fonts ) 
add_subdirectory( templates )

# This is a sample application for debugging purposes
# Enable only if you want to develop KFormula widget
#add_subdirectory( sample )
