#ifndef KPRINTDIALOG_H
#define KPRINTDIALOG_H

#include <kdialog.h>
#include <qlist.h>

class KPrinter;
class KPrintDialogPage;
class QLabel;
class QComboBox;
class QCheckBox;
class QPushButton;
class QLineEdit;

class KPrintDialog : public KDialog
{
	Q_OBJECT
public:
	KPrintDialog(QWidget *parent = 0, const char *name = 0);
	~KPrintDialog();

	void setFlags(int f);
	void setDialogPages(QList<KPrintDialogPage> *pages);
	void initialize(KPrinter*);

	static bool printerSetup(KPrinter*, QWidget*);

protected slots:
	void slotPrinterSelected(int);
	void slotBrowse();
	void slotProperties();
	void slotSetDefault();
	void slotOptions();
	virtual void done(int);
	void slotFilePrintToggled(bool);

protected:
	bool checkOutputFile();

protected:
	QLabel	*m_type, *m_state, *m_comment, *m_location;
	QPushButton	*m_properties, *m_default, *m_options, *m_filebrowse, *m_ok;
	QCheckBox	*m_preview, *m_printtofile;
	QLineEdit	*m_file;
	QComboBox	*m_printers;

	QList<KPrintDialogPage>	m_pages;
	KPrinter		*m_printer;
};

#endif
