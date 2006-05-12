
########### next target ###############

#set(CREATE_HASH_TABLE ${CMAKE_SOURCE_DIR}/kjs/create_hash_table )
#
#macro(CREATE_LUT _in_FILE _out_FILE)
#
#  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_out_FILE}
#    COMMAND ${PERL_EXECUTABLE}
#    ARGS ${CREATE_HASH_TABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${_in_FILE} >  ${CMAKE_CURRENT_BINARY_DIR}/${_out_FILE}
#    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_in_FILE}
#  )
#
#  kde4_add_file_dependancy(${CMAKE_CURRENT_SOURCE_DIR}/${_in_FILE}  ${CMAKE_CURRENT_BINARY_DIR}/${_out_FILE} )
#
#endmacro(CREATE_LUT)

#create_lut(kjs_dom.cpp kjs_dom.lut.h)
#create_lut(kjs_html.cpp kjs_html.lut.h)
#create_lut(kjs_window.cpp kjs_window.lut.h)
#create_lut(kjs_css.cpp kjs_css.lut.h)
#create_lut(kjs_events.cpp kjs_events.lut.h)
#create_lut(kjs_navigator.cpp kjs_navigator.lut.h)
#create_lut(kjs_mozilla.cpp kjs_mozilla.lut.h)
#create_lut(kjs_range.cpp kjs_range.lut.h)
#create_lut(kjs_traversal.cpp kjs_traversal.lut.h)
#create_lut(kjs_views.cpp kjs_views.lut.h)
#create_lut(xmlhttprequest.cpp xmlhttprequest.lut.h)
#create_lut(xmlserializer.cpp xmlserializer.lut.h)
#create_lut(domparser.cpp domparser.lut.h)


########### install files ###############


