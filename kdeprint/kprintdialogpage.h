#ifndef KPRINTDIALOGPAGE_H
#define KPRINTDIALOGPAGE_H

#include <qwidget.h>
#include <qmap.h>

class KMPrinter;
class DrMain;

class KPrintDialogPage : public QWidget
{
	Q_OBJECT
public:
	KPrintDialogPage(QWidget *parent = 0, const char *name = 0);
	KPrintDialogPage(KMPrinter *pr, DrMain *dr = 0, QWidget *parent = 0, const char *name = 0);
	virtual ~KPrintDialogPage();

	virtual void getOptions(QMap<QString,QString>& opts, bool incldef = false);
	virtual void setOptions(const QMap<QString,QString>& opts);
	virtual bool isValid(QString&);
	int id() const 				{ return m_ID; }
	void setId(int ID)			{ m_ID = ID; }
	QString	title() const 			{ return m_title; }
	void setTitle(const QString& txt)	{ m_title = txt; }
	DrMain* driver() 			{ return m_driver; }
	KMPrinter* printer()			{ return m_printer; }

protected:
	KMPrinter	*m_printer;
	DrMain		*m_driver;
	int 		m_ID;
	QString		m_title;
};

#endif
