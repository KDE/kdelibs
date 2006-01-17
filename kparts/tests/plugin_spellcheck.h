#ifndef __plugin_spellcheck_h
#define __plugin_spellcheck_h

#include <kparts/plugin.h>

class PluginSpellCheck : public KParts::Plugin
{
    Q_OBJECT
public:
    PluginSpellCheck( QObject* parent = 0, const char* name = 0, 
                      const QStringList& = QStringList() );
    virtual ~PluginSpellCheck();

public Q_SLOTS:
    void slotSpellCheck();
};

#endif
