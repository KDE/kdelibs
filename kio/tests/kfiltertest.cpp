#include "kfilterdev.h"
#include "kfilterbase.h"
#include <qfile.h>
#include <qtextstream.h>
#include <kdebug.h>
#include <kinstance.h>

void test_block( const QString & fileName )
{
    QFile f(fileName);
    KFilterBase * filter = KFilterBase::findFilterByFileName( fileName );
    if (!filter) return;
    filter->setDevice( &f );
    KFilterDev dev(filter);
    dev.open( IO_ReadOnly );

    // This is what KGzipDev::readAll could do, if QIODevice::readAll was virtual....

    QByteArray array(1024);
    int n;
    while ( ( n = dev.readBlock( array.data(), array.size() ) ) )
    {
        kdDebug() << "readBlock returned " << n << endl << endl;
        // QCString s(array,n+1); // Terminate with 0 before printing
        // printf("%s", s.data());

        kdDebug() << "dev.at = " << dev.at() << endl;
        kdDebug() << "f.at = " << f.at() << endl;
    }
    dev.close();
    delete filter;
}

void test_getch( const QString & fileName )
{
    QFile f(fileName);
    KFilterBase * filter = KFilterBase::findFilterByFileName( fileName );
    if (!filter) return;
    filter->setDevice( &f );
    KFilterDev dev(filter);
    dev.open( IO_ReadOnly );
    int ch;
    while ( ( ch = dev.getch() ) != -1 )
        printf("%c",ch);
    dev.close();
    delete filter;
}

void test_textstream(  const QString & fileName )
{
    QFile f(fileName);
    KFilterBase * filter = KFilterBase::findFilterByFileName( fileName );
    if (!filter) return;
    filter->setDevice( &f );
    KFilterDev dev(filter);
    dev.open( IO_ReadOnly );
    QTextStream ts( &dev );
    printf("%s\n", ts.read().latin1());
    dev.close();
    delete filter;
}

int main()
{
    KInstance instance("kfiltertest");
    kdDebug() << " -- test_block gzip -- " << endl;
    test_block("test.gz");
    kdDebug() << " -- test_getch gzip -- " << endl;
    test_getch("test.gz");
    kdDebug() << " -- test_textstream gzip -- " << endl;
    test_textstream("test.gz");

    kdDebug() << " -- test_block bzip2 -- " << endl;
    test_block("test.bz2");
    kdDebug() << " -- test_getch bzip2 -- " << endl;
    test_getch("test.bz2");

    return 0;
}
