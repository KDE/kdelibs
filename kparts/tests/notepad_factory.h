#ifndef __notepad_factory_h
#define __notepad_factory_h

#include <klibloader.h>

class KInstance;
class KAboutData;

class NotepadFactory : public KLibFactory
{
    Q_OBJECT

public:
    NotepadFactory();
    virtual ~NotepadFactory();
    virtual QObject* createObject( QObject* parent = 0, const char* name = 0, const char* classname = "QObject", const QStringList &args = QStringList() );

    static KInstance* instance();

private:

    static KInstance* s_instance;
    static KAboutData* s_about;
};

#endif
