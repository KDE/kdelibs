#ifndef KMWPASSWORD_H
#define KMWPASSWORD_H

#include "kmwizardpage.h"

class QLabel;
class QLineEdit;
class QCheckBox;

class KMWPassword : public KMWizardPage
{
public:
	KMWPassword(QWidget *parent = 0, const char *name = 0);

	bool isValid(QString&);
	void updatePrinter(KMPrinter*);

private:
	QLineEdit	*m_login;
	QLineEdit	*m_password;
	QCheckBox	*m_guest;
};

#endif
