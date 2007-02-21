rm -f *.cpp
cat > CMakeLists.txt << EOF

if(KDE4_BUILD_TESTS)

# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH \${CMAKE_CURRENT_BINARY_DIR} )
include_directories( \${KDE4_KDECORE_INCLUDES} \${CMAKE_SOURCE_DIR}/solid )
set(CMAKE_EXE_LINKER_FLAGS -v ; true )
add_definitions (-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_FROM_ASCII)
set(includetest_SRCS main.cpp
EOF

echo "int main() { return 0; }"  >> main.cpp

list=`find ../ -name *.h | grep -v tests/ | fgrep -v _p.h`
for i in $list ; do 
  filename=`echo $i | sed -e "s,../,,; s,/,_,g; s,\.h,.cpp,"`
  if grep -q ^$filename blacklist.txt; then
     continue
  fi
  if ! grep -q EXPORT $i; then
     continue
  fi
  basename=`basename $i`
  if ! grep -q "/$basename" "$1"; then
     continue
  fi
  echo "#include \"$i\"" > $filename
  echo "    $filename" >> CMakeLists.txt
done

cat >> CMakeLists.txt << EOF
)
kde4_add_executable(includetest NOGUI \${includetest_SRCS})
endif(KDE4_BUILD_TESTS)

ADD_CUSTOM_TARGET(update DEPENDS \${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)
add_custom_command(OUTPUT \${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
                   COMMAND \${CMAKE_CURRENT_SOURCE_DIR}/update.sh ARGS \${CMAKE_BINARY_DIR}/install_manifest.txt
                   DEPENDS \${CMAKE_CURRENT_SOURCE_DIR}/update.sh
                           \${CMAKE_BINARY_DIR}/install_manifest.txt  
                           \${CMAKE_CURRENT_SOURCE_DIR}/blacklist.txt
                   WORKING_DIRECTORY \${CMAKE_CURRENT_SOURCE_DIR})

EOF
