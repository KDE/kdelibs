#include "notepad.h" // this plugin applies to a notepad part
#include <QtGui/QTextEdit>
#include "plugin_spellcheck.h"
#include <kaction.h>
#include <kactioncollection.h>
#include <kmessagebox.h>
#include <kpluginfactory.h>
#include <klocale.h>
#include <kdebug.h>

PluginSpellCheck::PluginSpellCheck( QObject* parent,
                                    const QVariantList& )
    : Plugin( parent )
{
    KAction* act = new KAction( "&Select current line (plugin)", this );
    actionCollection()->addAction( "spellcheck", act );
    connect(act, SIGNAL(triggered()), this, SLOT(slotSpellCheck()));
}

PluginSpellCheck::~PluginSpellCheck()
{
}

void PluginSpellCheck::slotSpellCheck()
{
    kDebug() << "Plugin parent : " << parent()->objectName() << " (" << parent()->metaObject()->className() << ")";
    // The parent is assumed to be a NotepadPart
    if ( !parent()->inherits("NotepadPart") )
       KMessageBox::error(0,"You just called the spell-check action on a wrong part (not NotepadPart)");
    else
    {
         NotepadPart * part = (NotepadPart *) parent();
         QTextEdit * widget = (QTextEdit *) part->widget();
         widget->selectAll(); //selects current line !
    }
}

K_PLUGIN_FACTORY(PluginSpellCheckFactory, registerPlugin<PluginSpellCheck>();)
K_EXPORT_PLUGIN(PluginSpellCheckFactory("spellcheckplugin"))

#include <plugin_spellcheck.moc>
