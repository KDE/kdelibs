#ifndef KMWBANNERS_H
#define KMWBANNERS_H

#include "kmwizardpage.h"

class QComboBox;

class KMWBanners : public KMWizardPage
{
public:
	KMWBanners(QWidget *parent = 0, const char *name = 0);

	void initPrinter(KMPrinter*);
	void updatePrinter(KMPrinter*);

private:
	QComboBox	*m_start, *m_end;
};

#endif
