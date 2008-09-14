/* This file is part of the KDE libraries
    Copyright (C) 2008 Alexander Dymo <adymo@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "kshortcutschemeshelper_p.h"

#include <QFile>
#include <QTextStream>
#include <QDomDocument>

#include <kconfiggroup.h>
#include <kaction.h>
#include <kstandarddirs.h>
#include <kactioncollection.h>
#include <kxmlguiclient.h>
#include <kdebug.h>

bool KShortcutSchemesHelper::exportActionCollection(KActionCollection *collection,
    const QString &schemeName, const QString dir)
{
    const KXMLGUIClient *client = collection->parentGUIClient();
    if (!client)
        return false;

    QString schemeFileName;
    if (!dir.isEmpty())
        schemeFileName = dir + client->componentData().componentName() + schemeName + "shortcuts.rc";
    else
        schemeFileName = shortcutSchemeFileName(client, schemeName);

    QFile schemeFile(schemeFileName);
    if (!schemeFile.open(QFile::WriteOnly | QFile::Truncate))
    {
        kDebug() << "COULD NOT WRITE" << schemeFileName;
        return false;
    }

    QDomDocument doc;
    QDomElement docElem = doc.createElement("kpartgui");
    docElem.setAttribute("version", "1");
    docElem.setAttribute("name", client->componentData().componentName());
    doc.appendChild(docElem);
    QDomElement elem = doc.createElement("ActionProperties");
    docElem.appendChild(elem);

    // now, iterate through our actions
    foreach (QAction *action, collection->actions()) {
        KAction *kaction = qobject_cast<KAction*>(action);
        if (!kaction)
            continue;

        QString actionName = kaction->objectName();
        QString shortcut = kaction->shortcut(KAction::ActiveShortcut).toString();
        if (!shortcut.isEmpty())
        {
            QDomElement act_elem = doc.createElement("Action");
            act_elem.setAttribute( "name", actionName );
            act_elem.setAttribute( "shortcut", shortcut );
            elem.appendChild(act_elem);
        }
    }

    QTextStream out(&schemeFile);
    out << doc.toString(2);
    return true;
}

QString KShortcutSchemesHelper::currentShortcutSchemeName()
{
    return KGlobal::config()->group( "Shortcut Schemes" ).readEntry("Current Scheme", "Default");
}

QString KShortcutSchemesHelper::shortcutSchemeFileName(const KXMLGUIClient *client, const QString &schemeName)
{
    return KStandardDirs::locateLocal("data",
        client->componentData().componentName() + '/' +
        client->componentData().componentName() + schemeName + "shortcuts.rc" );
}

QString KShortcutSchemesHelper::applicationShortcutSchemeFileName(const QString &schemeName)
{
    return KGlobal::dirs()->locateLocal("appdata",
        KGlobal::mainComponent().componentName() + schemeName + "shortcuts.rc");
}
