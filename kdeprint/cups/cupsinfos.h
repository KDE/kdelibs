#ifndef CUPSINFOS_H
#define CUPSINFOS_H

#include <qstring.h>

class CupsInfos
{
public:
	static CupsInfos* self();

	CupsInfos();
	~CupsInfos();

	const QString& host() const;
	int port() const;
	const QString& login() const;
	const QString& password() const;

	void setHost(const QString& s);
	void setPort(int p);
	void setLogin(const QString& s);
	void setPassword(const QString& s);

	const char* getPasswordCB();

	void load();
	void save();

private:
	static CupsInfos	*unique_;

	QString	host_;
	int	port_;
	QString	login_;
	QString	password_;

	int	count_;
};

inline const QString& CupsInfos::host() const
{ return host_; }

inline int CupsInfos::port() const
{ return port_; }

inline const QString& CupsInfos::login() const
{ return login_; }

inline const QString& CupsInfos::password() const
{ return password_; }

#endif
