#ifndef KMPROPDRIVER_H
#define KMPROPDRIVER_H

#include "kmpropwidget.h"

class QLabel;

class KMPropDriver : public KMPropWidget
{
public:
	KMPropDriver(QWidget *parent = 0, const char *name = 0);
	~KMPropDriver();

	void setPrinter(KMPrinter*);

protected:
	void configureWizard(KMWizard*);

private:
	QLabel	*m_manufacturer;
	QLabel	*m_model;
	QLabel	*m_driverinfo;
};

#endif
