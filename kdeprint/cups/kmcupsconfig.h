#ifndef KMCUPSCONFIG_H
#define KMCUPSCONFIG_H

#include <kdialog.h>

class QLineEdit;
class QCheckBox;

class KMCupsConfig : public KDialog
{
public:
	KMCupsConfig(QWidget *parent = 0, const char *name = 0);

	static bool configure(QWidget *parent = 0);

protected:
	void initialize();

protected:
	QLineEdit	*m_host, *m_port, *m_login, *m_password;
	QCheckBox	*m_anonymous;
};

#endif
