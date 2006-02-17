
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )
include_directories( ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE ${KDEWIN32_INCLUDES} )

add_definitions(-DICE_t -DTRANS_SERVER -DTRANS_CLIENT -DANSICPP -DSOCKCONN -DTCPCONN)

add_executable(iceauth iceauth.c process.c ${kICE_STAT_SRCS} )
target_link_libraries(iceauth ${KDEWIN32_LIBRARIES} )
