#ifndef KLPDDIALOGIMPL_H
#define KLPDDIALOGIMPL_H

#include "kdialogimpl.h"

#include <qdatetime.h>
#include <qdict.h>

struct PrintcapEntry;

class KLpdDialogImpl : public KDialogImpl
{
public:
	KLpdDialogImpl(KPrinter *pr, QObject *parent = 0, const char *name = 0);
	~KLpdDialogImpl();

	KPrinterItemList* printersList();
	void printerSelected(KPrintDialog *dlg, int index);
	KPrinterPropertyDialog* buildPropertyDialog(KPrintDialog *dlg, int index);
	void setDefaultPrinter(KPrintDialog*,int);

private:
	void reload();
	void loadPrintcapFile(const QString& filename);

private:
	QDateTime	checktime_;
	QDict<PrintcapEntry>	entries_;
	QString		defaultprinter_;
};

#endif
