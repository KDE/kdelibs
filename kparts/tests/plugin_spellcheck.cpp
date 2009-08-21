/*
    Copyright (c) 2000 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License or ( at
    your option ) version 3 or, at the discretion of KDE e.V. ( which shall
    act as a proxy as in section 14 of the GPLv3 ), any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

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
    // Can't use qobject_cast here, we would need NotepadPart to be in a shared library.
    if ( !parent()->inherits("NotepadPart") )
       KMessageBox::error(0,"You just called the spell-check action on a wrong part (not NotepadPart)");
    else
    {
         NotepadPart * part = (NotepadPart *) parent();
         QTextEdit * widget = qobject_cast<QTextEdit *>(part->widget());
         Q_ASSERT(widget);
         widget->selectAll();
    }
}

K_PLUGIN_FACTORY(PluginSpellCheckFactory, registerPlugin<PluginSpellCheck>();)
K_EXPORT_PLUGIN(PluginSpellCheckFactory("spellcheckplugin"))

#include <plugin_spellcheck.moc>
