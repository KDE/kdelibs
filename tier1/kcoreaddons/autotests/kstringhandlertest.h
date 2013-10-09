#ifndef KSTRINGHANDLERTEST_H
#define KSTRINGHANDLERTEST_H

#include <QtCore/QObject>

class KStringHandlerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void capwords();
    void tagURLs();
    void perlSplit();
    void obscure();
    void preProcessWrap_data();
    void preProcessWrap();

private:
    static QString test;
};

#endif
