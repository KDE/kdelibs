#ifndef KMPROPBANNERS_H
#define KMPROPBANNERS_H

#include "kmpropwidget.h"

class QLabel;

class KMPropBanners : public KMPropWidget
{
public:
	KMPropBanners(QWidget *parent = 0, const char *name = 0);
	~KMPropBanners();

	void setPrinter(KMPrinter*);

protected:
	void configureWizard(KMWizard*);

private:
	QLabel	*m_startbanner;
	QLabel	*m_stopbanner;
};

#endif
