#ifndef KMWDRIVERSELECT_H
#define KMWDRIVERSELECT_H

#include "kmwizardpage.h"
#include "kmdbentry.h"

class KListBox;

class KMWDriverSelect : public KMWizardPage
{
public:
	KMWDriverSelect(QWidget *parent = 0, const char *name = 0);

	bool isValid(QString&);
	void initPrinter(KMPrinter*);
	void updatePrinter(KMPrinter*);

private:
	KListBox	*m_list;
	KMDBEntryList	*m_entries;
};

#endif
