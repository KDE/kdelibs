#include "kmcupsconfig.h"
#include "cupsinfos.h"

#include <qlabel.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qvalidator.h>

#include <klocale.h>
#include <kcursor.h>

class PortValidator : public QIntValidator
{
public:
	PortValidator(QWidget *parent, const char *name = 0);
	virtual QValidator::State validate(QString&, int&) const;
};

PortValidator::PortValidator(QWidget *parent, const char *name)
: QIntValidator(1, 9999, parent, name)
{
}

QValidator::State PortValidator::validate(QString& txt, int&) const
{
	bool 	ok(false);
	int 	p = txt.toInt(&ok);
	if (txt.isEmpty())
		return QValidator::Intermediate;
	else if (ok && p >= bottom() && p <= top())
		return QValidator::Acceptable;
	return QValidator::Invalid;
}

//******************************************************************************************

KMCupsConfig::KMCupsConfig(QWidget *parent, const char *name)
: KDialog(parent,name,true)
{
	setCaption(i18n("CUPS settings"));

	// widget creation
	QGroupBox	*m_hostbox = new QGroupBox(0, Qt::Vertical, i18n("Server informations"), this);
	QGroupBox	*m_loginbox = new QGroupBox(0, Qt::Vertical, i18n("Account informations"), this);
	QPushButton	*m_ok = new QPushButton(i18n("OK"), this);
	m_ok->setDefault(true);
	QPushButton	*m_cancel = new QPushButton(i18n("Cancel"), this);
	QLabel	*m_hostlabel = new QLabel(i18n("Host:"), m_hostbox);
	QLabel	*m_portlabel = new QLabel(i18n("Port:"), m_hostbox);
	m_host = new QLineEdit(m_hostbox);
	m_port = new QLineEdit(m_hostbox);
	m_port->setValidator(new PortValidator(m_port));
	m_login = new QLineEdit(m_loginbox);
	QLabel	*m_loginlabel = new QLabel(i18n("User:"), m_loginbox);
	QLabel	*m_passwordlabel = new QLabel(i18n("Password:"), m_loginbox);
	m_password = new QLineEdit(m_loginbox);
	m_password->setEchoMode(QLineEdit::Password);
	m_anonymous = new QCheckBox(i18n("Use anonymous access"), m_loginbox);
	m_anonymous->setCursor(KCursor::handCursor());

	// layout creation
	QVBoxLayout	*lay0 = new QVBoxLayout(this, 10, 10);
	QHBoxLayout	*lay1 = new QHBoxLayout(0, 0, 10);
	lay0->addWidget(m_hostbox,1);
	lay0->addWidget(m_loginbox,1);
	lay0->addLayout(lay1,0);
	lay1->addStretch(1);
	lay1->addWidget(m_ok);
	lay1->addWidget(m_cancel);
	QGridLayout	*lay2 = new QGridLayout(m_hostbox->layout(), 2, 2, 10);
	lay2->setColStretch(1,1);
	lay2->addWidget(m_hostlabel,0,0);
	lay2->addWidget(m_portlabel,1,0);
	lay2->addWidget(m_host,0,1);
	lay2->addWidget(m_port,1,1);
	QGridLayout	*lay3 = new QGridLayout(m_loginbox->layout(), 3, 2, 10);
	lay3->setColStretch(1,1);
	lay3->addWidget(m_loginlabel,0,0);
	lay3->addWidget(m_passwordlabel,1,0);
	lay3->addWidget(m_login,0,1);
	lay3->addWidget(m_password,1,1);
	lay3->addMultiCellWidget(m_anonymous,2,2,0,1);

	// connections
	connect(m_ok,SIGNAL(clicked()),SLOT(accept()));
	connect(m_cancel,SIGNAL(clicked()),SLOT(reject()));
	connect(m_anonymous,SIGNAL(toggled(bool)),m_login,SLOT(setDisabled(bool)));
	connect(m_anonymous,SIGNAL(toggled(bool)),m_password,SLOT(setDisabled(bool)));

	initialize();
	resize(300,100);
}

void KMCupsConfig::initialize()
{
	CupsInfos	*inf = CupsInfos::self();
	m_host->setText(inf->host());
	m_port->setText(QString::number(inf->port()));
	if (inf->login().isEmpty())
		m_anonymous->setChecked(true);
	else
	{
		m_login->setText(inf->login());
		m_password->setText(inf->password());
	}
}

bool KMCupsConfig::configure(QWidget *parent)
{
	KMCupsConfig	dlg(parent);
	if (dlg.exec())
	{
		CupsInfos	*inf = CupsInfos::self();
		inf->setHost(dlg.m_host->text());
		inf->setPort(dlg.m_port->text().toInt());
		if (dlg.m_anonymous->isChecked())
		{
			inf->setLogin(QString::null);
			inf->setPassword(QString::null);
		}
		else
		{
			inf->setLogin(dlg.m_login->text());
			inf->setPassword(dlg.m_password->text());
		}
		return true;
	}
	return false;
}
