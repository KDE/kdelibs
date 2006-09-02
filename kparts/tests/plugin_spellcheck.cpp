#include "notepad.h" // this plugin applies to a notepad part
#include <QTextEdit>
#include "plugin_spellcheck.h"
#include <kaction.h>
#include <kgenericfactory.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>

PluginSpellCheck::PluginSpellCheck( QObject* parent,
                                    const QStringList& )
    : Plugin( parent )
{
    KAction* act = new KAction( "&Select current line (plugin)",
                                actionCollection(), "spellcheck" );
    connect(act, SIGNAL(triggered()), this, SLOT(slotSpellCheck()));
}

PluginSpellCheck::~PluginSpellCheck()
{
}

void PluginSpellCheck::slotSpellCheck()
{
    kDebug() << "Plugin parent : " << parent()->objectName() << " (" << parent()->metaObject()->className() << ")" << endl;
    // The parent is assumed to be a NotepadPart
    if ( !parent()->inherits("NotepadPart") )
       KMessageBox::error(0L,"You just called the spell-check action on a wrong part (not NotepadPart)");
    else
    {
         NotepadPart * part = (NotepadPart *) parent();
         QTextEdit * widget = (QTextEdit *) part->widget();
         widget->selectAll(); //selects current line !
    }
}

K_EXPORT_COMPONENT_FACTORY( libspellcheckplugin,
                            KGenericFactory<PluginSpellCheck> )

#include <plugin_spellcheck.moc>
