#ifndef KMWDRIVER_H
#define KMWDRIVER_H

#include "kmwizardpage.h"

class KMDriverDbWidget;

class KMWDriver : public KMWizardPage
{
public:
	KMWDriver(QWidget *parent = 0, const char *name = 0);

	bool isValid(QString&);
	void updatePrinter(KMPrinter*);
	void initPrinter(KMPrinter*);

private:
	KMDriverDbWidget	*m_widget;
	bool			m_initialized;
};

#endif
