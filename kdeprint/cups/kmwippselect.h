#ifndef KMWIPPSELECT_H
#define KMWIPPSELECT_H

#include "kmwizardpage.h"

class KListBox;

class KMWIppSelect : public KMWizardPage
{
public:
	KMWIppSelect(QWidget *parent = 0, const char *name = 0);

	bool isValid(QString&);
	void initPrinter(KMPrinter*);
	void updatePrinter(KMPrinter*);

private:
	KListBox	*m_list;
};

#endif
