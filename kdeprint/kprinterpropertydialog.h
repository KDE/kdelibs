#ifndef KPRINTERPROPERTYDIALOG_H
#define KPRINTERPROPERTYDIALOG_H

#include <qtabdialog.h>
#include <qlist.h>

#include "optionset.h"

class KPrintDialogPage;
class KPrinterItem;

class KPrinterPropertyDialog : public QTabDialog
{
	Q_OBJECT
public:
	KPrinterPropertyDialog(bool hasSaveBtn = false, QWidget *parent = 0, const char *name = 0);
	~KPrinterPropertyDialog();

	void setPrinter(KPrinterItem *printer);
	void addPage(KPrintDialogPage *p);

signals:
	void saveRequested(KPrinterItem*, const OptionSet&);

protected:
	bool retrieveOptions(OptionSet& opts, bool incldef = false);
	bool syncOptions();

protected slots:
	void pageSelected(QWidget*);
	virtual void done(int);
	void saveClicked();

private:
	OptionSet	options_;
	QList<KPrintDialogPage>	pages_;
	KPrintDialogPage	*currentpage_;
	KPrinterItem	*printer_;
};

#endif
