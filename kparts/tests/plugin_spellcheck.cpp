#include "notepad.h" // this plugin applies to a notepad part
#include <q3multilineedit.h>
#include "plugin_spellcheck.h"
#include <kaction.h>
#include <kgenericfactory.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>

PluginSpellCheck::PluginSpellCheck( QObject* parent, const char* name, 
                                    const QStringList& )
    : Plugin( parent, name )
{
    (void) new KAction( "&Select current line (plugin)", 0, this, SLOT(slotSpellCheck()),
                        actionCollection(), "spellcheck" );
}

PluginSpellCheck::~PluginSpellCheck()
{
}

void PluginSpellCheck::slotSpellCheck()
{
    kDebug() << "Plugin parent : " << parent()->name() << " (" << parent()->className() << ")" << endl;
    // The parent is assumed to be a NotepadPart
    if ( !parent()->inherits("NotepadPart") )
       KMessageBox::error(0L,"You just called the spell-check action on a wrong part (not NotepadPart)");
    else
    {
         NotepadPart * part = (NotepadPart *) parent();
         Q3MultiLineEdit * widget = (Q3MultiLineEdit *) part->widget();
         widget->selectAll(); //selects current line !
    }
}

K_EXPORT_COMPONENT_FACTORY( libspellcheckplugin, 
                            KGenericFactory<PluginSpellCheck> );

#include <plugin_spellcheck.moc>
