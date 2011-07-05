INCLUDE_DIRECTORIES (${CMAKE_CURRENT_SOURCE_DIR})

INCLUDE_DIRECTORIES (
    ${KDE4_INCLUDES}
    ${QCA2_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

ADD_SUBDIRECTORY (tests)

SET (ksecretservice_lib_SRCS
    jobqueue.cpp
    queuedjob.cpp
    secrettool.cpp
    secretcodec.cpp
)

KDE4_ADD_LIBRARY (ksecretsservicelib STATIC ${ksecretservice_lib_SRCS})
TARGET_LINK_LIBRARIES (ksecretsservicelib ${QT_QTCORE_LIBRARY})

