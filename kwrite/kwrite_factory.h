#ifndef __kwrite_factory_h__
#define __kwrite_factory_h__

#include <kparts/factory.h>

class KInstance;
class KAboutData;

class KWriteFactory : public KParts::Factory
{
    Q_OBJECT

public:

    KWriteFactory( QObject* parent = 0, const char* name = 0 );
    ~KWriteFactory();

    virtual KParts::Part* createPart( QWidget *parentWidget, const char *, QObject* parent = 0, const char* name = 0, const char* classname = "QObject", const QStringList &args = QStringList() );

    static KInstance* instance();

private:

    static KInstance* s_instance;
    static KAboutData* s_about;
};

#endif // __kwrite_factory_h__
