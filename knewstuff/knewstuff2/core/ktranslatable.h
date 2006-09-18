#ifndef KTRANSLATABLE_H
#define KTRANSLATABLE_H

#include <qstring.h>
#include <qmap.h>
#include <qstringlist.h>

class KTranslatable
{
public:
	KTranslatable();
	KTranslatable(QString string);
	typedef QMap<QString, QString> Map;
	typedef QMapIterator<QString, QString> Iterator;

	void addString(QString lang, QString string);
	QString representation();
	QString translated(QString lang);
	QStringList strings();
	QStringList languages();
	QMap<QString, QString> stringmap();
	bool isTranslated();
	bool isEmpty();
private:
	QMap<QString, QString> m_strings;
};

#endif
