#ifndef KCUPSDIALOGIMPL_H
#define KCUPSDIALOGIMPL_H

#include "kdialogimpl.h"
#include "globalppd.h"
#include "ipprequest.h"

#include <qdatetime.h>

class KCupsDialogImpl : public KDialogImpl
{
	Q_OBJECT
public:
	KCupsDialogImpl(KPrinter *pr, QObject *parent = 0, const char *name = 0);
	~KCupsDialogImpl();

	KPrinterItemList* printersList();
	void printerSelected(KPrintDialog *dlg, int index);
	KPrinterPropertyDialog* buildPropertyDialog(KPrintDialog *dlg, int index);
	void setDefaultPrinter(KPrintDialog *dlg, int index);
	bool configure(KPrintDialog *dlg);

private slots:
	void slotSaveRequested(KPrinterItem*, const OptionSet&);

private:
	void processRequest(IppRequest& req);
	void loadServerPrinters();
	void loadOptionsFile(const QString& filename);
	void loadOptionsFiles();
	bool saveOptionsFile(const QString& filename);

	void closePPD();
	void closeRequest();

private:
	QDateTime	checktime_;
	IppRequest	*request_;
	global_ppd_file_t	*ppd_;
};

#endif
