#ifndef KPRINTDIALOG_H
#define KPRINTDIALOG_H

#include "kprintdialogbase.h"
#include "kprinteritem.h"
#include "optionset.h"

#include <qlabel.h>

class KPrinter;
class QTabWidget;
class KPrintDialogPage;

class KPrintDialog : public KPrintDialogBase
{
	Q_OBJECT
public:
	KPrintDialog(KPrinter *printer, QWidget *parent = 0, const char *name = 0);
	~KPrintDialog();

	void setValidPrinter(bool valid);
	void setPrinterType(const QString& s);
	void setPrinterState(const QString& s);
	void setPrinterComment(const QString& s);
	void setPrinterLocation(const QString& s);

	static bool printerSetup(KPrinter *printer, QWidget *parent);

protected:
	void initialize();
	virtual void done(int);
	bool checkOutputFile();
	void reloadPrinters();

protected slots:
	void printerSelected(int);
	void propertyClicked();
	void pageSelected(QWidget *w);
	void outputFileBrowseClicked();
	void setDefaultClicked();
	void optionsClicked();

private:
	KPrinter	*printer_;
	QTabWidget	*tabs_;
	KPrinterItemList	*printerslist_;
	QList<KPrintDialogPage>	pages_;
	KPrintDialogPage	*currentpage_;
	OptionSet		options_;
};

inline void KPrintDialog::setPrinterType(const QString& s)
{ type_->setText(s); }

inline void KPrintDialog::setPrinterComment(const QString& s)
{ comment_->setText(s); }

inline void KPrintDialog::setPrinterState(const QString& s)
{ state_->setText(s); }

inline void KPrintDialog::setPrinterLocation(const QString& s)
{ location_->setText(s); }

#endif
