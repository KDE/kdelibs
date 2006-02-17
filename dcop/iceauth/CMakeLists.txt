
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )
include_directories( ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE ${KDEWIN32_INCLUDES} )

add_definitions(-DICE_t -DTRANS_SERVER -DTRANS_CLIENT -DANSICPP -DSOCKCONN -DTCPCONN)

set(ICEAUTH_EXEC_SRC 
   iceauth.c 
   process.c 
)

kde4_add_executable(iceauth  ${ICEAUTH_EXEC_SRC} )
target_link_libraries(iceauth ${KDEWIN32_LIBRARIES} DCOP )
