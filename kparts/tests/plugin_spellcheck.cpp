#include "plugin_spellcheck.h"
#include <kaction.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <klocale.h>

PluginSpellCheck::PluginSpellCheck( QObject* parent, const char* name )
    : Plugin( parent, name )
{
    (void) new KAction( i18n( "The Action For SpellChecking" ), 0, this, SLOT(slotSpellCheck()),
                        actionCollection(), "spellcheck" );
}

PluginSpellCheck::~PluginSpellCheck()
{
}

void PluginSpellCheck::slotSpellCheck()
{
    KMessageBox::error(0L,"You just called the spell-check action");
}

KPluginFactory::KPluginFactory( QObject* parent, const char* name )
  : KLibFactory( parent, name )
{
  s_instance = new KInstance("KPluginFactory");
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

KInstance* KPluginFactory::s_instance = 0L;

#include <plugin_spellcheck.moc>
