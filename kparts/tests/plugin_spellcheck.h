#ifndef plugin_spellcheck_h
#define plugin_spellcheck_h

#include <kparts/plugin.h>

class PluginSpellCheck : public KParts::Plugin
{
    Q_OBJECT
public:
    PluginSpellCheck( QObject* parent = 0,
                      const QVariantList& = QVariantList() );
    virtual ~PluginSpellCheck();

public Q_SLOTS:
    void slotSpellCheck();
};

#endif
