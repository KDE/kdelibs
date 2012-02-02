
find_package(Qt5Transitional REQUIRED Test)

add_subdirectory(qmimedatabase-xml)
if (UNIX)
   add_subdirectory(qmimedatabase-cache)
endif()

