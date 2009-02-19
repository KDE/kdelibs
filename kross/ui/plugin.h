/* This file is part of the KDE project
   Copyright (C) 2008 Paulo Moura Guedes <moura@kdewebdev.org>

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

#ifndef KROSS_PLUGIN_H
#define KROSS_PLUGIN_H

#include <kparts/plugin.h>

#include <kross/core/krossconfig.h>
#include <kross/core/childreninterface.h>

namespace Kross {
    class ActionCollection;
}

class QWidget;

namespace Kross
{

/** 
 * The ScriptingPlugin class loads additional actions stored in rc files with the 
 * KrossScripting format:
 * 
 * \code
 * <KrossScripting>
 *     <collection name="file" text="File">
 *         <script name="dummy_script" text="Dummy Script" comment="Dummy Script example"
 *                 file="dummy_script.py" interpreter="python" />
 *     </collection>
 * </KrossScripting>
 * \endcode
 * 
 * The 'name' attribute in collection element will be used to match the menu object name.
 * If no menu already exists with this name, a new one is created. In this example, the user will 
 * see a menu item with the text "Dummy Script" in "File" menu, which will execute the dummy_script.py script.
 * 
 * By default it tries to find kross rc files in %APPDATA%/scripts directory.
 * Clients of this class can use slotEditScriptActions() as a way to override and/or extend the
 * default script actions (if they exist at all).
 *
 * You may create multiple instances of ScriptingPlugin by using alternative c'tor.
  */
class KROSSUI_EXPORT ScriptingPlugin : public KParts::Plugin
{
    Q_OBJECT
public:

    /**
     * Constructor.
     *
     * \param parent The parent QObject this QObject is child of.
     */
    explicit ScriptingPlugin(QObject* parent = 0);

    /**
     * Allows having actions defined in a custom location, eg for project-specific actions
     *
     * \param userActionsFile scripts.rc filepath -- file may be modified by user
     * \param referenceActionsDir dir -- %APPDATA%/scripts/%referenceActionsDir% contains standard actions for this plugin instance; has a lower priority than \a userActionsFile.
     */
    ScriptingPlugin(const QString& collectionName, const QString& userActionsFile, const QString& referenceActionsDir=QString(), QObject* parent=0);

    /**
     * Destructor.
     */
    virtual ~ScriptingPlugin();

    /**
     * Re-implement in order to load additional kross scripting rc files.
     */
    virtual void setDOMDocument (const QDomDocument &document, bool merge = false);

    /**
     * Add a QObject to the list of children. The object will be published to the scripting code.
     * \param object The QObject instance that should be added to the list of children.
     * \param name The name the QObject should be known under. If not defined, the
     * QObject's objectName is used.
     */
    void addObject(QObject* object, const QString& name/* = QString()*/, ChildrenInterface::Options options/* = ChildrenInterface::NoOption*/);
    
    ///\deprecated use another addObject overload
    void addObject(QObject* object, const QString& name = QString()); //BIC

protected Q_SLOTS:

    /**
     * This slot will open/create a scriptactions.rc file at $KDEHOME/share/apps/application/scripts/
     * which will overide other kross rc files. This allows a user to extend existing menus with new actions.
     */
    virtual void slotEditScriptActions();

    /**
     * Deletes the user rc file, which has the effect of falling back to the default script actions (if any).
     */
    virtual void slotResetScriptActions();

private:
    QDomDocument buildDomDocument(const QDomDocument& document);
    void buildDomDocument(QDomDocument& document, Kross::ActionCollection* collection);
    void save();

private:
    class ScriptingPluginPrivate;
    ScriptingPluginPrivate* const d;
};

}

#endif
