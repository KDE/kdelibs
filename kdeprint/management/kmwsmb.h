#ifndef KMWSMB_H
#define KMWSMB_H

#include "kmwizardpage.h"

class SmbView;
class QLineEdit;

class KMWSmb : public KMWizardPage
{
	Q_OBJECT
public:
	KMWSmb(QWidget *parent = 0, const char *name = 0);

	bool isValid(QString&);
	void updatePrinter(KMPrinter*);
	void initPrinter(KMPrinter*);

protected slots:
	void slotScan();
	void slotAbort();
	void slotPrinterSelected(const QString& work, const QString& server, const QString& printer);

protected:
	SmbView	*m_view;
	QLineEdit	*m_work, *m_server, *m_printer;
};

#endif
