
#include "meinproc_common.h"

#include "xslt.h"

#include <QDir>
#include <QFileInfo>

#include <cstdlib>

CheckFileResult checkFile( const QString &checkFilename )
{
    const QFileInfo checkFile(checkFilename);
    if (!checkFile.exists())
    {
        return CheckFileDoesNotExist;
    }
    if (!checkFile.isFile())
    {
        return CheckFileIsNotFile;
    }
    if (!checkFile.isReadable())
    {
        return CheckFileIsNotReadable;
    }
    return CheckFileSuccess;
}

CheckResult check(const QString &checkFilename, const QString &exe, const QByteArray &catalogs)
{
    const QString pwd_buffer = QDir::currentPath();
    const QFileInfo file( checkFilename );

    setenv( "XML_CATALOG_FILES", catalogs.constData(), 1 );
    if ( QFileInfo( exe ).isExecutable() ) {
        QDir::setCurrent( file.absolutePath() );
        QString cmd = exe;
        cmd += " --valid --noout ";
        cmd += file.fileName();
        cmd += " 2>&1";
        FILE *xmllint = popen( QFile::encodeName( cmd ).constData(), "r" );
        char buf[ 512 ];
        bool noout = true;
        unsigned int n;
        while ( ( n = fread(buf, 1, sizeof( buf ) - 1, xmllint ) ) ) {
            noout = false;
            buf[ n ] = '\0';
            fputs( buf, stderr );
        }
        pclose( xmllint );
        QDir::setCurrent( pwd_buffer );
        if ( !noout )
            return CheckNoOut;
    } else {
        return CheckNoXmllint;
    }
    return CheckSuccess;
}

void doOutput(QString output, bool usingStdOut, bool usingOutput, const QString &outputOption, bool replaceCharset)
{
    if (output.indexOf( "<FILENAME " ) == -1 || usingStdOut || usingOutput )
    {
        QFile file;
        if ( usingStdOut ) {
            file.open( stdout, QIODevice::WriteOnly );
        } else {
            if ( usingOutput )
                file.setFileName( outputOption );
            else
                file.setFileName( "index.html" );
            file.open(QIODevice::WriteOnly);
        }
        if (replaceCharset) replaceCharsetHeader( output );
#ifdef Q_WS_WIN
        QByteArray data = output.toUtf8();
#else
        QByteArray data = output.toLocal8Bit();
#endif
        file.write(data.data(), data.length());
        file.close();
    } else {
        int index = 0;
        while (true) {
            index = output.indexOf("<FILENAME ", index);
            if (index == -1)
                break;
            int filename_index = index + strlen("<FILENAME filename=\"");

            const QString filename = output.mid(filename_index,
                                            output.indexOf("\"", filename_index) -
                                            filename_index);

            QString filedata = splitOut(output, index);
            QFile file(filename);
            file.open(QIODevice::WriteOnly);
            if (replaceCharset) replaceCharsetHeader( filedata );
            const QByteArray data = fromUnicode( filedata );
            file.write(data.data(), data.length());
            file.close();

            index += 8;
        }
    }
}
