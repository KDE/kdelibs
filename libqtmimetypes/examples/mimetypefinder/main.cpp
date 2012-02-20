#include "qmimedatabase.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (argc <= 1) {
        printf( "No filename specified\n" );
        return 1;
    }
    QString option;
    int fnPos = 1;
    if (argc > 2) {
        option = QString::fromLatin1(argv[1]);
        ++fnPos;
    }
    const QString fileName = QFile::decodeName(argv[fnPos]);
    //int accuracy;
    QMimeDatabase db;
    QMimeType mime;
    if (fileName == QLatin1String("-")) {
        QFile qstdin;
        qstdin.open(stdin, QIODevice::ReadOnly);
        const QByteArray data = qstdin.readAll();
        //mime = QMimeType::findByContent(data, &accuracy);
        mime = db.mimeTypeForData(data);
    } else if (option == QLatin1String("-c")) {
        mime = db.mimeTypeForFile(fileName, QMimeDatabase::MatchContent);
    } else if (option == QLatin1String("-f")) {
        mime = db.mimeTypeForFile(fileName, QMimeDatabase::MatchExtension);
    } else {
        mime = db.mimeTypeForFile(fileName);
    }
    if ( mime.isValid() /*&& !mime.isDefault()*/ ) {
        printf("%s\n", mime.name().toLatin1().constData());
        //printf("(accuracy %d)\n", accuracy);
    } else {
        return 1; // error
    }

    return 0;
}
