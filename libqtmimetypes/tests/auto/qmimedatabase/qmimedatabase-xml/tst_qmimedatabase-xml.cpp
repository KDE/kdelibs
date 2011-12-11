#include "../tst_qmimedatabase.h"

tst_qmimedatabase::tst_qmimedatabase()
{
    // To make sure that other mime packages don't change our test results
    qputenv("XDG_DATA_DIRS", SRCDIR "../../../src/mimetypes/mime");
    qputenv("XDG_DATA_HOME", QByteArray("doesnotexist"));
    qputenv("QT_NO_MIME_CACHE", "1");
}

#include "../tst_qmimedatabase.cpp"
