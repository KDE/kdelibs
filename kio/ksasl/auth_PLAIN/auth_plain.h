#ifndef AUTH_PLAIN_H
#define AUTH_PLAIN_H "$Id$"

#include <kinstance.h>
#include <klibloader.h>
#include <saslmodule.h>

class PlainAuthModule : public KSASLAuthModule
{
public:
	PlainAuthModule(QObject* parent = 0, const char* name = 0);
	virtual ~PlainAuthModule();
	virtual QString auth_method();
	virtual QString auth_response(const QString &challenge, const KURL &auth_url);
};

class PlainAuthModuleFactory : public KLibFactory
{
	Q_OBJECT
public:
	PlainAuthModuleFactory( QObject *parent = 0, const char *name = 0 );
	~PlainAuthModuleFactory();
	virtual QObject *create( QObject *parent = 0, const char *name = 0,
				 const char* classname = "QObject",
				 const QStringList &args = QStringList() );
	KInstance *instance();
private:
	KInstance *s_instance;
};

#endif
