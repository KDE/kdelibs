#ifndef AUTH_CRAM_H
#define AUTH_CRAM_H "$Id$"

#include <kinstance.h>
#include <klibloader.h>
#include <saslmodule.h>

class CramAuthModule : public KSASLAuthModule
{
public:
	CramAuthModule(QObject* parent = 0, const char* name = 0);
	virtual ~CramAuthModule();
	virtual QString auth_method();
	virtual QString auth_response(const QString &challenge, const KURL &auth_url);
	virtual void get_usercredentials() {;}
};

class CramAuthModuleFactory : public KLibFactory
{
	Q_OBJECT
public:
	CramAuthModuleFactory( QObject *parent = 0, const char *name = 0 );
	~CramAuthModuleFactory();
	virtual QObject *create( QObject *parent = 0, const char *name = 0,
				 const char* classname = "QObject",
				 const QStringList &args = QStringList() );
	KInstance *instance();
private:
	KInstance *s_instance;
};

#endif
