#include "kcupsoptiondlg.h"
#include "cupsinfos.h"

#include <qlineedit.h>
#include <qvalidator.h>
#include <qcheckbox.h>

KCupsOptionDlg::KCupsOptionDlg(QWidget *parent, const char *name)
: KCupsOptionDlgBase(parent,name,true)
{
	QIntValidator	*val_ = new QIntValidator(0,9999,port_);
	port_->setValidator(val_);
}

KCupsOptionDlg::~KCupsOptionDlg()
{
}

QString KCupsOptionDlg::login() const
{
	if (anonymous_->isChecked()) return QString::null;
	else return login_->text();
}

QString KCupsOptionDlg::password() const
{
	if (anonymous_->isChecked()) return QString::null;
	else return password_->text();
}

QString KCupsOptionDlg::server() const
{
	return server_->text();
}

int KCupsOptionDlg::port() const
{
	bool	ok;
	int	p = port_->text().toInt(&ok);
	if (ok) return p;
	else return (-1);
}

void KCupsOptionDlg::setServerInfo(const QString& server, int port)
{
	server_->setText(server);
	port_->setText(QString::number(port));
}

void KCupsOptionDlg::setLoginInfo(const QString& login, const QString& passwd)
{
	if (login.isNull())
	{
		anonymous_->setChecked(true);
		login_->setText(QString::null);
		password_->setText(QString::null);
	}
	else
	{
		anonymous_->setChecked(false);
		login_->setText(login);
		password_->setText(passwd);
	}
}

bool KCupsOptionDlg::configure(QWidget *parent)
{
	CupsInfos	*infos_ = CupsInfos::self();
	KCupsOptionDlg	dlg(parent);
	bool	flag(false);

	dlg.setLoginInfo(infos_->login(),infos_->password());
	dlg.setServerInfo(infos_->host(),infos_->port());
	if ((flag=dlg.exec()))
	{
		infos_->setLogin(dlg.login());
		infos_->setPassword(dlg.password());
		infos_->setHost(dlg.server());
		infos_->setPort(dlg.port());
	}
	return flag;
}
