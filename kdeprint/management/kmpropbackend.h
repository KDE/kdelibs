#ifndef KMPROPBACKEND_H
#define KMPROPBACKEND_H

#include "kmpropwidget.h"

class QLabel;

class KMPropBackend : public KMPropWidget
{
public:
	KMPropBackend(QWidget *parent = 0, const char *name = 0);
	~KMPropBackend();

	void setPrinter(KMPrinter*);

protected:
	void configureWizard(KMWizard*);

private:
	QLabel	*m_type;
	QLabel	*m_uri;
};

#endif
