#ifndef KCUPSOPTIONDLG_H
#define KCUPSOPTIONDLG_H

#include "kcupsoptiondlgbase.h"

/**
  *@author Michael Goffioul
  */

class KCupsOptionDlg : public KCupsOptionDlgBase
{
public:
	KCupsOptionDlg(QWidget *parent = 0, const char *name = 0);
	~KCupsOptionDlg();

	QString login() const;
	QString password() const;
	QString server() const;
	int port() const;
	bool anonymous() const;

	void setLoginInfo(const QString& login, const QString& passwd);
	void setServerInfo(const QString& server, int port);

	static bool configure(QWidget *parent = 0);
};

#endif
