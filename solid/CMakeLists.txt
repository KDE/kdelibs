include_directories( ${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR}
                     ${QT_INCLUDE_DIR} )

# Cannot compile with enable-final because of too many "using namespace" in
# .cpp files, leading to conflicts e.g. between
# Solid::Backends::Hal::AcAdapter and Solid::Ifaces::AcAdapter
kde4_no_enable_final(solid)

add_subdirectory( solid )
add_subdirectory( tests )
