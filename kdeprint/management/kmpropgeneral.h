#ifndef KMPROPGENERAL_H
#define KMPROPGENERAL_H

#include "kmpropwidget.h"

class QLabel;

class KMPropGeneral : public KMPropWidget
{
public:
	KMPropGeneral(QWidget *parent = 0, const char *name = 0);
	~KMPropGeneral();

	void setPrinter(KMPrinter*);

protected:
	void configureWizard(KMWizard*);

private:
	QLabel	*m_name;
	QLabel	*m_location;
	QLabel	*m_description;
};

#endif
