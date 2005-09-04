#ifndef KURLMIMETEST_H
#define KURLMIMETEST_H

#include <qobject.h>

class KURLMimeTest : public QObject
{
    Q_OBJECT
public slots:
    void testURLList();
    void testOneURL();
};


#endif
