#ifndef _KDIGEST_AUTH_MODULE_H
#define _KDIGEST_AUTH_MODULE_H "$Id$"

#include <qmap.h>

#include <klibloader.h>
#include <saslmodule.h>

class DigestAuthModule
	: public KSASLAuthModule
{
public:
	DigestAuthModule( QObject* parent = 0, const char* name = 0 );
	virtual ~DigestAuthModule();
	virtual QString auth_method();
	virtual QString auth_response(const QString &challenge, const KURL &auth_url);

protected:
	void reset();
	void decNonceCount() {nonce_count--;}
	bool generateResponse(QString &s);

	void UseAuthString(const QString &challenge);
	unsigned int nonce_count;
	char cnonce[128];
	KURL request_url;

	void AddKey(const QString &line, QMap<QString, QString> &);
	void AddKey(const QString &k, const QString &v, QMap<QString, QString> &m);
	QMap<QString,QString> keys;
};

class KDigestAuthModuleFactory : public KLibFactory
{
    Q_OBJECT
public:
    KDigestAuthModuleFactory( QObject *parent = 0, const char *name = 0 );
    ~KDigestAuthModuleFactory();

    virtual QObject *create( QObject *parent = 0, const char *name = 0,
                             const char* classname = "QObject",
                             const QStringList &args = QStringList() );

    static KInstance *instance();

private:
    static KInstance *s_instance;

};
#endif
