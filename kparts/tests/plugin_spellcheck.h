#ifndef __plugin_spellcheck_h
#define __plugin_spellcheck_h

#include <kplugin.h>
#include <klibloader.h>

using namespace KParts;

class PluginSpellCheck : public Plugin
{
    Q_OBJECT
public:
    PluginSpellCheck( QObject* parent = 0, const char* name = 0 );
    virtual ~PluginSpellCheck();

public slots:
    void slotSpellCheck();

};

class KPluginFactory : public KLibFactory
{
    Q_OBJECT
public:
    KPluginFactory( QObject* parent = 0, const char* name = 0 );
    ~KPluginFactory() {};

    virtual QObject* create( QObject* parent = 0, const char* name = 0, const char* name = "QObject", const QStringList &args = QStringList() );

private:
    static KInstance* s_global;
};

#endif
