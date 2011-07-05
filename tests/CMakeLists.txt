INCLUDE_DIRECTORIES (${CMAKE_CURRENT_BINARY_DIR})

KDE4_ADD_EXECUTABLE (ksecretservice_lib_queuedjobtest queuedjobtest.cpp)
TARGET_LINK_LIBRARIES (ksecretservice_lib_queuedjobtest
   ksecretsservicelib
   ${QT_QTTEST_LIBRARIES}
)

ADD_TEST (QueuedJobTest ksecretservice_lib_queuedjobtest)
