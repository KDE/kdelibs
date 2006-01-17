#ifndef KSTRINGHANDLERTEST_H
#define KSTRINGHANDLERTEST_H

#include <qobject.h>

class KStringHandlerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void word();
    void insword();
    void setword();
    void remrange();
    void remword();
    void capwords();
    void reverse();
    void center();
    void tagURLs();

private:
    static QString test;
};

#endif
