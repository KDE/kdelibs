INCLUDES = $(all_includes) -I../kjsembed

bin_PROGRAMS = console 

console_SOURCES = jsconsole.ui console.qrc main.cpp console.cpp \ 
                  kjs_object_model.cpp numberedtextview.cpp
console_METASOURCES = AUTO
console_LDFLAGS = $(all_libraries) 
console_LDADD = -lkjsembed 
