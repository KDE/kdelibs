#ifndef KMWEND_H
#define KMWEND_H

#include "kmwizardpage.h"

class QTextView;

class KMWEnd : public KMWizardPage
{
public:
	KMWEnd(QWidget *parent = 0, const char *name = 0);

	void initPrinter(KMPrinter*);

private:
	QTextView	*m_view;
};

#endif
