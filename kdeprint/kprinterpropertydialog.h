#ifndef KPRINTERPROPERTYDIALOG_H
#define KPRINTERPROPERTYDIALOG_H

#include <qtabdialog.h>
#include <qlist.h>
#include <qmap.h>

class KMPrinter;
class KPrintDialogPage;
class DrMain;

class KPrinterPropertyDialog : public QTabDialog
{
	Q_OBJECT
public:
	KPrinterPropertyDialog(KMPrinter *printer, QWidget *parent = 0, const char *name = 0);
	~KPrinterPropertyDialog();

	KMPrinter* printer() 		{ return m_printer; }
	DrMain* driver()		{ return m_driver; }
	void setDriver(DrMain* d) 	{ if (!m_driver) m_driver = d; }
	void addPage(KPrintDialogPage*);

	static void setupPrinter(KMPrinter *printer, QWidget *parent);

protected:
	bool synchronize();
	void collectOptions(QMap<QString,QString>& opts);

protected slots:
	void slotCurrentChanged(QWidget*);
	void slotSaveClicked();
	virtual void done(int);

protected:
	KMPrinter		*m_printer;
	DrMain			*m_driver;
	QList<KPrintDialogPage>	m_pages;
	KPrintDialogPage	*m_current;
	QMap<QString,QString>	m_options;
};

#endif
