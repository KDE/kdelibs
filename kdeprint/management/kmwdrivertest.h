#ifndef KMWDRIVERTEST_H
#define KMWDRIVERTEST_H

#include "kmwizardpage.h"

class QLabel;
class QPushButton;
class DrMain;

class KMWDriverTest : public KMWizardPage
{
	Q_OBJECT
public:
	KMWDriverTest(QWidget *parent = 0, const char *name = 0);
	~KMWDriverTest();

	void initPrinter(KMPrinter*);
	void updatePrinter(KMPrinter*);

protected slots:
	void slotTest();
	void slotSettings();

private:
	QLabel	*m_manufacturer;
	QLabel	*m_model;
	QLabel	*m_driverinfo;
	QPushButton	*m_test;
	QPushButton	*m_settings;
	DrMain		*m_driver;
};

#endif
