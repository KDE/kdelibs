#ifndef SASLMODULE_H
#define SASLMODULE_H "$Id$"

#include <qstring.h>

#include <kurl.h>

#define KSASL_MODULE_REV 1

class KSASLAuthModule {
public:
	KSASLAuthModule() {;}
	virtual ~KSASLAuthModule(){;}
	virtual QString auth_method()=0;
	virtual QString auth_response(const QString &challenge, const KURL &auth_url)=0;
};

#define DECLARE_SASL_MODULE(meth,clsnam) \
  class clsnam : public KSASLAuthModule { \
	public:\
		clsnam ();\
		virtual ~clsnam (); \
		virtual QString auth_method() {return QString(meth);}\
		virtual QString auth_response(const QString &challenge, const KURL &auth_url);\
  };\
  extern "C" {\
	KSASLAuthModule *auth_init()\
	{\
		return new clsnam ;\
        }\
        int module_version() {return KSASL_MODULE_REV;}\
  }

#endif
