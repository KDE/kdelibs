#ifndef KSTRINGHANDLERTEST_H
#define KSTRINGHANDLERTEST_H

#include <QtCore/QObject>

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
    void perlSplit();

private:
    static QString test;
};

#endif
