#include "plugin_spellcheck.h"
#include <kaction.h>
#include <kmessagebox.h>
#include <klocale.h>

PluginSpellCheck::PluginSpellCheck( QObject* parent, const char* name )
    : Plugin( parent, name )
{
    (void) new KAction( i18n( "The Action For SpellChecking" ), 0, actionCollection(), "spellcheck" );
}

PluginSpellCheck::~PluginSpellCheck()
{
}

void PluginSpellCheck::slotSpellCheck()
{
    KMessageBox::error(0L,"You just called the spell-check action");
}

QObject* KPluginFactory::create( QObject* parent, const char* name, const char*, const QStringList & )
{
    return new PluginSpellCheck( parent, name );
}

extern "C"
{
  void* init_libspellcheck()
  {
    return new KPluginFactory;
  }

  /*Plugin *init_spellcheck(QObject *parent, const char *name)
  {
    return new PluginSpellCheck(parent, name);
  }*/

}

#include <plugin_spellcheck.moc>
