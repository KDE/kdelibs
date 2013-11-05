set(CMAKE_REQUIRED_LIBRARIES Qt5::Core)
check_cxx_source_compiles(
"#include <QtCore/QFileSystemWatcher>
int main()
{
    QFileSystemWatcher *watcher = new QFileSystemWatcher();
    delete watcher;
    return 0;
}" HAVE_QFILESYSTEMWATCHER)