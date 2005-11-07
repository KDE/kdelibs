#ifndef KFILTERTEST_H
#define KFILTERTEST_H

#include <qobject.h>

#include <QObject>

class KFilterTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void test_block_write();
    void test_block_read();
    void test_getch();
    void test_textstream();

private:
    void test_block_write( const QString & fileName );
    void test_block_read( const QString & fileName );
    void test_getch( const QString & fileName );
    void test_textstream( const QString & fileName );

    QString pathgz;
    QString pathbz2;
    QByteArray testData;
};


#endif
