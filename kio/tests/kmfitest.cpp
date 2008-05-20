#include <stdio.h>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <kcomponentdata.h>
#include <kurl.h>
#include <kfilemetainfo.h>
#include <kmimetype.h>

int main (int argc, char **argv)
{
	KComponentData ins(QByteArray("kmfitest"));

	if (argc < 2) {
		printf("usage: %s <file>\n", argv[0]);
		return 1;
	}

	for (int i = 1; i < argc; i++) {
		QString file = QFile::decodeName(argv[i]);
		qWarning("File: %s", file.toLocal8Bit().data());
		KMimeType::Ptr p;
		p = KMimeType::findByPath(file);
		qWarning("Mime type (findByPath): %s", p->name().toLatin1().constData());
		KFileMetaInfo meta(file, QString(), KFileMetaInfo::TechnicalInfo | KFileMetaInfo::ContentInfo);
	}

	return 0;
}
