#include <stdio.h>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <kurl.h>
#include <kfilemetainfo.h>
#include <qmimedatabase.h>

int main (int argc, char **argv)
{

	if (argc < 2) {
		printf("usage: %s <file>\n", argv[0]);
		return 1;
	}

        QMimeDatabase db;
	for (int i = 1; i < argc; i++) {
		QString file = QFile::decodeName(argv[i]);
		qWarning("File: %s", file.toLocal8Bit().data());
		QMimeType mime = db.mimeTypeForFile(file);
		qWarning("Mime type (mimeTypeForFile): %s", mime.name().toLatin1().constData());
		KFileMetaInfo meta(file, QString(), KFileMetaInfo::TechnicalInfo | KFileMetaInfo::ContentInfo);
	}

	return 0;
}
