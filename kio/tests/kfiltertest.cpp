#include "kfilterdev.h"
#include "kcomprmanager.h"
#include <qfile.h>
#include <kdebug.h>
#include <kinstance.h>

void test_block( const QString & fileName )
{
    QFile f(fileName);
    KFilterBase * filter = KComprManager::self()->findFilterByFileName( fileName );
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
    KFilterBase * filter = KComprManager::self()->findFilterByFileName( fileName );
    filter->setDevice( &f );
    KFilterDev dev(filter);
    dev.open( IO_ReadOnly );
    int ch;
    while ( ( ch = dev.getch() ) != -1 )
        printf("%c",ch);
    dev.close();
    delete filter;
}

int main()
{
    KInstance instance("kfiltertest");
    kdDebug() << " -- testgz_block -- " << endl;
    test_block("test.gz");
    kdDebug() << " -- testgz_getch -- " << endl;
    test_getch("test.gz");
    kdDebug() << " -- testbz2_block -- " << endl;
    test_block("test.bz2");
    kdDebug() << " -- testbz2_getch -- " << endl;
    test_getch("test.bz2");
    return 0;
}
