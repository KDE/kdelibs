
include_directories( ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE ${KDEWIN32_INCLUDES} )

add_definitions(-DICE_t -DTRANS_SERVER -DTRANS_CLIENT -DANSICPP -DSOCKCONN -DTCPCONN)

set(ICEAUTH_EXEC_SRC 
   iceauth.c 
   process.c 
)

kde4_add_executable(iceauth  NOGUI ${ICEAUTH_EXEC_SRC} )
target_link_libraries(iceauth ${KDEWIN32_LIBRARIES} DCOP )
install_targets(/bin iceauth )
