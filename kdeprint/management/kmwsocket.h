#ifndef	KMWSOCKET_H
#define	KMWSOCKET_H

#include "kmwizardpage.h"

class KListView;
class QListViewItem;
class QProgressBar;
class QLineEdit;
class KMWSocketUtil;

class KMWSocket : public KMWizardPage
{
	Q_OBJECT;
public:
	KMWSocket(QWidget *parent = 0, const char *name = 0);
	~KMWSocket();

	bool isValid(QString&);
	void updatePrinter(KMPrinter*);

protected slots:
	void slotSettings();
	void slotScan();
	void slotPrinterSelected(QListViewItem*);

private:
	KListView	*m_list;
	QProgressBar	*m_bar;
	QLineEdit	*m_printer, *m_port;
	KMWSocketUtil	*m_util;
};

#endif
