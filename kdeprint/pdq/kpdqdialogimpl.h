#ifndef KPDQDIALOGIMPL_H
#define KPDQDIALOGIMPL_H

#include "kdialogimpl.h"

#include <qdatetime.h>

class PDQMain;
class PDQPrinter;

class KPdqDialogImpl : public KDialogImpl
{
	Q_OBJECT
public:
	KPdqDialogImpl(KPrinter *pr, QObject *parent = 0, const char *name = 0);
	~KPdqDialogImpl();

	KPrinterItemList* printersList();
	void printerSelected(KPrintDialog *dlg, int index);
	KPrinterPropertyDialog* buildPropertyDialog(KPrintDialog *dlg, int index);
	void setDefaultPrinter(KPrintDialog*,int);
	bool configure(KPrintDialog*);

	PDQPrinter* printerWithOptions(const QString& prname, const OptionSet& opts);

protected slots:
	void slotSaveRequested(KPrinterItem*,const OptionSet&);

private:
	void reload();

private:
	PDQMain	*pdq_;
	QDateTime	checktime_;
};

#endif
