#ifndef KPRINTERIMPL_H
#define KPRINTERIMPL_H

#include <qobject.h>
#include <qstringlist.h>
#include <qmap.h>

class KPrinter;
class KMPrinter;

class KPrinterImpl : public QObject
{
public:
	KPrinterImpl(QObject *parent = 0, const char *name = 0);
	virtual ~KPrinterImpl();

	virtual bool printFiles(KPrinter*, const QStringList&);
	virtual void preparePrinting(KPrinter*);
	virtual void broadcastOption(const QString& key, const QString& value);

	KMPrinter* filePrinter()				{ return m_fileprinter; }
	void saveOptions(const QMap<QString,QString>& opts)	{ m_options = opts; }
	const QMap<QString,QString>& loadOptions() const 	{ return m_options; }

protected:
	KMPrinter		*m_fileprinter;
	QMap<QString,QString>	m_options;	// use to save current options
};

#endif
