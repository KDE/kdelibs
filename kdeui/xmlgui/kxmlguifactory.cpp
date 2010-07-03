/* This file is part of the KDE libraries
   Copyright (C) 1999,2000 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2000 Kurt Granroth <granroth@kde.org>

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

#include "kxmlguifactory.h"
#include "kxmlguifactory_p.h"
#include "kxmlguiclient.h"
#include "kxmlguibuilder.h"

#include <assert.h>

#include <QtCore/QDir>
#include <QtXml/QDomDocument>
#include <QtCore/QFile>
#include <QtCore/QTextIStream>
#include <QtGui/QWidget>
#include <QtCore/QDate>
#include <QtCore/QVariant>
#include <QTextCodec>

#include <kdebug.h>
#include <kcomponentdata.h>
#include <kglobal.h>
#include <kshortcut.h>
#include <kstandarddirs.h>

#include "kaction.h"
#include "kshortcutsdialog.h"
#include "kactioncollection.h"

using namespace KXMLGUI;

class KXMLGUIFactoryPrivate : public BuildState
{
public:
    enum ShortcutOption { SetActiveShortcut = 1, SetDefaultShortcut = 2};

    KXMLGUIFactoryPrivate()
    {
        static const QString &defaultMergingName = KGlobal::staticQString( "<default>" );
        static const QString &actionList = KGlobal::staticQString( "actionlist" );
        static const QString &name = KGlobal::staticQString( "name" );

        m_rootNode = new ContainerNode( 0L, QString(), 0L );
        m_defaultMergingName = defaultMergingName;
        tagActionList = actionList;
        attrName = name;
    }
    ~KXMLGUIFactoryPrivate()
    {
        delete m_rootNode;
    }

    void pushState()
    {
        m_stateStack.push( *this );
    }

    void popState()
    {
        BuildState::operator=( m_stateStack.pop() );
    }

    bool emptyState() const { return m_stateStack.isEmpty(); }

    QWidget *findRecursive( KXMLGUI::ContainerNode *node, bool tag );
    QList<QWidget*> findRecursive( KXMLGUI::ContainerNode *node, const QString &tagName );
    void applyActionProperties( const QDomElement &element,
        ShortcutOption shortcutOption = KXMLGUIFactoryPrivate::SetActiveShortcut );
    void configureAction( QAction *action, const QDomNamedNodeMap &attributes,
        ShortcutOption shortcutOption = KXMLGUIFactoryPrivate::SetActiveShortcut );
    void configureAction( QAction *action, const QDomAttr &attribute,
        ShortcutOption shortcutOption = KXMLGUIFactoryPrivate::SetActiveShortcut );

    QDomDocument shortcutSchemeDoc(KXMLGUIClient *client);
    void applyShortcutScheme(KXMLGUIClient *client, QDomDocument scheme);
    void refreshActionProperties(KXMLGUIClient *client, QDomDocument doc);
    void saveDefaultActionProperties(KXMLGUIClient *client);

    ContainerNode *m_rootNode;

    QString m_defaultMergingName;

    /*
     * Contains the container which is searched for in ::container .
     */
    QString m_containerName;

    /*
     * List of all clients
     */
    QList<KXMLGUIClient*> m_clients;

    QString tagActionList;

    QString attrName;

    BuildStateStack m_stateStack;
};

QString KXMLGUIFactory::readConfigFile( const QString &filename, const KComponentData &_componentData )
{
    QString xml_file;

    if (!QDir::isRelativePath(filename))
        xml_file = filename;
    else
    {
        KComponentData componentData = _componentData.isValid() ? _componentData : KGlobal::mainComponent();
        xml_file = KStandardDirs::locate("data", componentData.componentName() + '/' + filename);
        if ( !QFile::exists( xml_file ) )
          xml_file = KStandardDirs::locate( "data", filename );
    }

    QFile file( xml_file );
    if ( xml_file.isEmpty() || !file.open( QIODevice::ReadOnly ) )
    {
        kError(240) << "No such XML file" << filename;
        return QString();
    }

    QByteArray buffer(file.readAll());
    return QString::fromUtf8(buffer.constData(), buffer.size());
}

bool KXMLGUIFactory::saveConfigFile( const QDomDocument& doc,
                                     const QString& filename, const KComponentData &_componentData )
{
    KComponentData componentData = _componentData.isValid() ? _componentData : KGlobal::mainComponent();
    QString xml_file(filename);

    if (QDir::isRelativePath(xml_file))
        xml_file = KStandardDirs::locateLocal("data", componentData.componentName() + '/' + filename);

    QFile file( xml_file );
    if ( xml_file.isEmpty() || !file.open( QIODevice::WriteOnly ) )
    {
        kError(240) << "Could not write to" << filename;
        return false;
    }

    // write out our document
    QTextStream ts(&file);
    ts.setCodec( QTextCodec::codecForName( "UTF-8" ) );
    ts << doc;

    file.close();
    return true;
}

/**
 * Removes all QDomComment objects from the specified node and all its children.
 */
static void removeDOMComments( QDomNode &node )
{
    QDomNode n = node.firstChild();
    while ( !n.isNull() )
    {
        if ( n.nodeType() == QDomNode::CommentNode )
        {
            QDomNode tmp = n;
            n = n.nextSibling();
            node.removeChild( tmp );
        }
        else
        {
            QDomNode tmp = n;
            n = n.nextSibling();
            removeDOMComments( tmp );
        }
    }
}

KXMLGUIFactory::KXMLGUIFactory( KXMLGUIBuilder *builder, QObject *parent )
    : QObject( parent ),d(new KXMLGUIFactoryPrivate)
{
    d->builder = builder;
    d->guiClient = 0;
    if ( d->builder )
    {
        d->builderContainerTags = d->builder->containerTags();
        d->builderCustomTags = d->builder->customTags();
    }
}

KXMLGUIFactory::~KXMLGUIFactory()
{
    foreach (KXMLGUIClient *client, d->m_clients) {
        client->setFactory ( 0L );
    }
    delete d;
}

void KXMLGUIFactory::addClient( KXMLGUIClient *client )
{
    //kDebug(260) << client;
    if ( client->factory() ) {
        if ( client->factory() == this )
            return;
        else
            client->factory()->removeClient( client ); //just in case someone does stupid things ;-)
    }

    if (d->emptyState())
        emit makingChanges(true);
    d->pushState();

//    QTime dt; dt.start();

    d->guiClient = client;

    // add this client to our client list
    if ( !d->m_clients.contains( client ) )
        d->m_clients.append( client );
    else
        kDebug(260) << "XMLGUI client already added " << client;

    // Tell the client that plugging in is process and
    //  let it know what builder widget its mainwindow shortcuts
    //  should be attached to.
    client->beginXMLPlug( d->builder->widget() );

    // try to use the build document for building the client's GUI, as the build document
    // contains the correct container state information (like toolbar positions, sizes, etc.) .
    // if there is non available, then use the "real" document.
    QDomDocument doc = client->xmlguiBuildDocument();
    if ( doc.documentElement().isNull() )
        doc = client->domDocument();

    QDomElement docElement = doc.documentElement();

    d->m_rootNode->index = -1;

    // cache some variables

    d->clientName = docElement.attribute( d->attrName );
    d->clientBuilder = client->clientBuilder();

    if ( d->clientBuilder )
    {
        d->clientBuilderContainerTags = d->clientBuilder->containerTags();
        d->clientBuilderCustomTags = d->clientBuilder->customTags();
    }
    else
    {
        d->clientBuilderContainerTags.clear();
        d->clientBuilderCustomTags.clear();
    }

    // load shortcut schemes, user-defined shortcuts and other action properties
    d->saveDefaultActionProperties(client);
    if (!doc.isNull())
        d->refreshActionProperties(client, doc);

    BuildHelper( *d, d->m_rootNode ).build( docElement );

    // let the client know that we built its GUI.
    client->setFactory( this );

    // call the finalizeGUI method, to fix up the positions of toolbars for example.
    // ### FIXME : obey client builder
    // --- Well, toolbars have a bool "positioned", so it doesn't really matter,
    // if we call positionYourself on all of them each time. (David)
    d->builder->finalizeGUI( d->guiClient );

    // reset some variables, for safety
    d->BuildState::reset();

    client->endXMLPlug();

    d->popState();

    emit clientAdded( client );

    // build child clients
    foreach (KXMLGUIClient *child, client->childClients())
        addClient( child );

    if (d->emptyState())
        emit makingChanges(false);
/*
    QString unaddedActions;
    foreach (KActionCollection* ac, KActionCollection::allCollections())
      foreach (QAction* action, ac->actions())
        if (action->associatedWidgets().isEmpty())
          unaddedActions += action->objectName() + ' ';

    if (!unaddedActions.isEmpty())
      kWarning() << "The following actions are not plugged into the gui (shortcuts will not work): " << unaddedActions;
*/

//    kDebug() << "addClient took " << dt.elapsed();
}

void KXMLGUIFactory::refreshActionProperties()
{
    foreach (KXMLGUIClient *client, d->m_clients)
    {
        d->guiClient = client;
        QDomDocument doc = client->xmlguiBuildDocument();
        if ( doc.documentElement().isNull() )
        {
            client->reloadXML();
            doc = client->domDocument();
        }
        d->refreshActionProperties(client, doc);
    }
    d->guiClient = 0;
}

void KXMLGUIFactoryPrivate::refreshActionProperties(KXMLGUIClient *client, QDomDocument doc)
{
    // try to find and apply shortcuts schemes
    QDomDocument scheme = shortcutSchemeDoc(client);
    applyShortcutScheme(client, scheme);

    // try to find and apply user-defined shortcuts
    QDomElement actionPropElement = KXMLGUIFactory::actionPropertiesElement(doc);
    if (actionPropElement.childNodes().isEmpty())
        doc.documentElement().removeChild(actionPropElement);

    if ( !actionPropElement.isNull() )
        applyActionProperties( actionPropElement );
}

void KXMLGUIFactoryPrivate::saveDefaultActionProperties(KXMLGUIClient *client)
{
    // This method is called every time the user activated a new
    // kxmlguiclient. We only want to execute the following code only once in
    // the lifetime of an action.
    foreach (QAction *action, client->actionCollection()->actions())
    {
        // Skip actions we have seen already.
        if (action->property("_k_DefaultShortcut").isValid()) continue;

        if (KAction* kaction = qobject_cast<KAction*>(action))
        {
            // Check if the default shortcut is set
            KShortcut defaultShortcut = kaction->shortcut(KAction::DefaultShortcut);
            KShortcut activeShortcut  = kaction->shortcut(KAction::ActiveShortcut);

            // Check if we have an empty default shortcut and an non empty
            // custom shortcut. This should only happen if a developer called
            // QAction::setShortcut on an KAction. Print out a warning and
            // correct the mistake
            if ((!activeShortcut.isEmpty()) && defaultShortcut.isEmpty())
            {
                kError(240) << "Shortcut for KAction " << kaction->objectName() << kaction->text() << "set with QShortcut::setShortcut()! See KAction documentation.";
                kaction->setProperty("_k_DefaultShortcut", activeShortcut);
            }
            else
            {
                kaction->setProperty("_k_DefaultShortcut", defaultShortcut);
            }
        }
        else
        {
            // A QAction used with KXMLGUI? Set our property and ignore it.
            kError(240) << "Attempt to use QAction" << action->objectName() << "with KXMLGUIFactory!";
            action->setProperty("_k_DefaultShortcut", KShortcut());
        }

    }
}

void KXMLGUIFactory::changeShortcutScheme(const QString &scheme)
{
    kDebug(260) << "Changing shortcut scheme to" << scheme;
    KConfigGroup cg = KGlobal::config()->group( "Shortcut Schemes" );
    cg.writeEntry("Current Scheme", scheme);

    refreshActionProperties();
}

void KXMLGUIFactory::removeClient( KXMLGUIClient *client )
{
    //kDebug(260) << client;

    // don't try to remove the client's GUI if we didn't build it
    if ( !client || client->factory() != this )
        return;

    if (d->emptyState())
        emit makingChanges(true);

    // remove this client from our client list
    d->m_clients.removeAll( client );

    // remove child clients first (create a copy of the list just in case the
    // original list is modified directly or indirectly in removeClient())
    const QList<KXMLGUIClient*> childClients(client->childClients());
    foreach (KXMLGUIClient *child, childClients)
        removeClient(child);

    //kDebug(260) << "calling removeRecursive";

    d->pushState();

    // cache some variables

    d->guiClient = client;
    d->clientName = client->domDocument().documentElement().attribute( d->attrName );
    d->clientBuilder = client->clientBuilder();

    client->setFactory( 0L );

    // if we don't have a build document for that client, yet, then create one by
    // cloning the original document, so that saving container information in the
    // DOM tree does not touch the original document.
    QDomDocument doc = client->xmlguiBuildDocument();
    if ( doc.documentElement().isNull() )
    {
        doc = client->domDocument().cloneNode( true ).toDocument();
        client->setXMLGUIBuildDocument( doc );
    }

    d->m_rootNode->destruct( doc.documentElement(), *d );

    // reset some variables
    d->BuildState::reset();

    // This will destruct the KAccel object built around the given widget.
    client->prepareXMLUnplug( d->builder->widget() );

    d->popState();

    if (d->emptyState())
        emit makingChanges(false);

    emit clientRemoved( client );
}

QList<KXMLGUIClient*> KXMLGUIFactory::clients() const
{
    return d->m_clients;
}

QWidget *KXMLGUIFactory::container( const QString &containerName, KXMLGUIClient *client,
                                    bool useTagName )
{
    d->pushState();
    d->m_containerName = containerName;
    d->guiClient = client;

    QWidget *result = d->findRecursive( d->m_rootNode, useTagName );

    d->guiClient = 0L;
    d->m_containerName.clear();

    d->popState();

    return result;
}

QList<QWidget*> KXMLGUIFactory::containers( const QString &tagName )
{
    return d->findRecursive( d->m_rootNode, tagName );
}

void KXMLGUIFactory::reset()
{
    d->m_rootNode->reset();

    d->m_rootNode->clearChildren();
}

void KXMLGUIFactory::resetContainer( const QString &containerName, bool useTagName )
{
    if ( containerName.isEmpty() )
        return;

    ContainerNode *container = d->m_rootNode->findContainer( containerName, useTagName );

    if ( !container )
        return;

    ContainerNode *parent = container->parent;
    if ( !parent )
        return;

    //  resetInternal( container );

    parent->removeChild( container );
}

QWidget *KXMLGUIFactoryPrivate::findRecursive( KXMLGUI::ContainerNode *node, bool tag )
{
    if ( ( ( !tag && node->name == m_containerName ) ||
           ( tag && node->tagName == m_containerName ) ) &&
         ( !guiClient || node->client == guiClient ) )
        return node->container;

    foreach (ContainerNode* child, node->children)
    {
        QWidget *cont = findRecursive( child, tag );
        if ( cont )
            return cont;
    }

    return 0L;
}

// Case insensitive equality without calling toLower which allocates a new string
static inline bool equals(const QString& str1, const char* str2)
{
    return str1.compare(QLatin1String(str2), Qt::CaseInsensitive) == 0;
}
static inline bool equals(const QString& str1, const QString& str2)
{
    return str1.compare(str2, Qt::CaseInsensitive) == 0;
}


QList<QWidget*> KXMLGUIFactoryPrivate::findRecursive( KXMLGUI::ContainerNode *node,
                                                      const QString &tagName )
{
    QList<QWidget*> res;

    if ( equals(node->tagName, tagName) )
        res.append( node->container );

    foreach (KXMLGUI::ContainerNode* child, node->children)
        res << findRecursive( child, tagName );

    return res;
}

void KXMLGUIFactory::plugActionList( KXMLGUIClient *client, const QString &name,
                                     const QList<QAction*> &actionList )
{
    d->pushState();
    d->guiClient = client;
    d->actionListName = name;
    d->actionList = actionList;
    d->clientName = client->domDocument().documentElement().attribute( d->attrName );

    d->m_rootNode->plugActionList( *d );

    d->BuildState::reset();
    d->popState();
}

void KXMLGUIFactory::unplugActionList( KXMLGUIClient *client, const QString &name )
{
    d->pushState();
    d->guiClient = client;
    d->actionListName = name;
    d->clientName = client->domDocument().documentElement().attribute( d->attrName );

    d->m_rootNode->unplugActionList( *d );

    d->BuildState::reset();
    d->popState();
}

void KXMLGUIFactoryPrivate::applyActionProperties( const QDomElement &actionPropElement,
        ShortcutOption shortcutOption )
{
    for (QDomNode n = actionPropElement.firstChild();
         !n.isNull(); n = n.nextSibling() )
    {
        QDomElement e = n.toElement();
        if ( !equals(e.tagName(), "action") )
            continue;

        QAction *action = guiClient->action( e );
        if ( !action )
            continue;

        configureAction( action, e.attributes(), shortcutOption );
    }
}

void KXMLGUIFactoryPrivate::configureAction( QAction *action, const QDomNamedNodeMap &attributes,
        ShortcutOption shortcutOption )
{
    for ( uint i = 0; i < attributes.length(); i++ )
    {
        QDomAttr attr = attributes.item( i ).toAttr();
        if ( attr.isNull() )
            continue;

        configureAction( action, attr, shortcutOption );
    }
}

void KXMLGUIFactoryPrivate::configureAction( QAction *action, const QDomAttr &attribute,
        ShortcutOption shortcutOption )
{
    static const QString &attrShortcut = KGlobal::staticQString( "shortcut" );

    QString attrName = attribute.name();
    // If the attribute is a deprecated "accel", change to "shortcut".
    if ( equals(attrName, "accel") )
        attrName = attrShortcut;

    // No need to re-set name, particularly since it's "objectName" in Qt4
    if ( equals(attrName, "name") )
        return;

    if ( equals(attrName, "icon") ) {
        action->setIcon( KIcon( attribute.value() ) );
        return;
    }

    QVariant propertyValue;

    QVariant::Type propertyType = action->property( attrName.toLatin1() ).type();

    if ( propertyType == QVariant::Int ) {
        propertyValue = QVariant( attribute.value().toInt() );
    } else if ( propertyType == QVariant::UInt ) {
        propertyValue = QVariant( attribute.value().toUInt() );
    } else if ( propertyType == QVariant::UserType && action->property( attrName.toLatin1() ).userType() == qMetaTypeId<KShortcut>() ) {
        // Setting the shortcut by property also sets the default shortcut (which is incorrect), so we have to do it directly
        if (KAction* ka = qobject_cast<KAction*>(action)) {
            if (attrName=="globalShortcut") {
                ka->setGlobalShortcut(KShortcut(attribute.value()), KAction::ActiveShortcut);
            } else {
                ka->setShortcut(KShortcut(attribute.value()), KAction::ActiveShortcut);
            }
            if (shortcutOption & KXMLGUIFactoryPrivate::SetDefaultShortcut)
                ka->setShortcut(KShortcut(attribute.value()), KAction::DefaultShortcut);
            return;
        }
        propertyValue = KShortcut( attribute.value() );
    } else {
        propertyValue = QVariant( attribute.value() );
    }
    if (!action->setProperty( attrName.toLatin1(), propertyValue )) {
        kWarning() << "Error: Unknown action property " << attrName << " will be ignored!";
    }
}

QDomDocument KXMLGUIFactoryPrivate::shortcutSchemeDoc(KXMLGUIClient *client)
{
    // Get the name of the current shorcut scheme
    KConfigGroup cg = KGlobal::config()->group( "Shortcut Schemes" );
    QString schemeName = cg.readEntry("Current Scheme", "Default");

    QDomDocument doc;
    if (schemeName != "Default")
    {
        // Find the document for the shortcut scheme using both current application path
        // and current xmlguiclient path but making a preference to app path
        QString schemeFileName = KStandardDirs::locateLocal("data",
            client->componentData().componentName() + '/' +
            client->componentData().componentName() + schemeName.toLower() + "shortcuts.rc" );

        QFile schemeFile(schemeFileName);
        if (schemeFile.open(QIODevice::ReadOnly))
        {
//             kDebug(260) << "Found shortcut scheme" << schemeFileName;
            doc.setContent(&schemeFile);
            schemeFile.close();
        }
    }
    return doc;
}

void KXMLGUIFactoryPrivate::applyShortcutScheme(KXMLGUIClient *client, QDomDocument scheme)
{
    static const QString &actionPropElementName = KGlobal::staticQString( "ActionProperties" );

    KConfigGroup cg = KGlobal::config()->group( "Shortcut Schemes" );
    QString schemeName = cg.readEntry("Current Scheme", "Default");

    //First clear all existing shortcuts
    if (schemeName != "Default") {
        foreach (QAction *action, client->actionCollection()->actions())
        {
            if (KAction *kaction = qobject_cast<KAction*>(action))
            {
                kaction->setShortcut(KShortcut(), KAction::ActiveShortcut);
                // We clear the default shortcut as well because the shortcut scheme will set its own defaults
                kaction->setShortcut(KShortcut(), KAction::DefaultShortcut);
            }
            else
                action->setProperty("shortcut", KShortcut());
        }
    } else {
        // apply saved default shortcuts
        foreach (QAction *action, client->actionCollection()->actions())
        {
            if (KAction *kaction = qobject_cast<KAction*>(action))
            {
                QVariant savedDefaultShortcut = kaction->property("_k_DefaultShortcut");
                if (savedDefaultShortcut.isValid())
                {
                    KShortcut shortcut = savedDefaultShortcut.value<KShortcut>();
                    kaction->setShortcut(shortcut, KAction::ActiveShortcut);
                    kaction->setShortcut(shortcut, KAction::DefaultShortcut);
                    continue;
                }
            }
            action->setProperty("shortcut", KShortcut());
        }
    }

    if (scheme.isNull())
        return;

    QDomElement docElement = scheme.documentElement();
    QDomElement actionPropElement = docElement.namedItem( actionPropElementName ).toElement();

    //Check if we really have the shortcut configuration here
    if (!actionPropElement.isNull())
    {
        kDebug(260) << "Applying shortcut scheme for XMLGUI client" << client->componentData().componentName();

        //Apply all shortcuts we have
        applyActionProperties(actionPropElement, KXMLGUIFactoryPrivate::SetDefaultShortcut);
    }
    else
        kDebug(260) << "Invalid shortcut scheme file";
}

int KXMLGUIFactory::configureShortcuts(bool letterCutsOk , bool bSaveSettings )
{
	KShortcutsDialog dlg(KShortcutsEditor::AllActions,
         letterCutsOk ? KShortcutsEditor::LetterShortcutsAllowed : KShortcutsEditor::LetterShortcutsDisallowed,
         qobject_cast<QWidget*>(parent()));
	foreach (KXMLGUIClient *client, d->m_clients)
	{
		if(client && !client->xmlFile().isEmpty())
			dlg.addCollection( client->actionCollection() );
	}
	return dlg.configure(bSaveSettings);
}

QDomElement KXMLGUIFactory::actionPropertiesElement( QDomDocument& doc )
{
	KConfigGroup cg = KGlobal::config()->group( "Shortcut Schemes" );
	QString schemeName = cg.readEntry("Current Scheme", "Default");

	const QString tagActionProp = QLatin1String("ActionProperties");
	// first, lets see if we have existing properties
	QDomElement elem;
	QDomNode it = doc.documentElement().firstChild();
	for( ; !it.isNull(); it = it.nextSibling() ) {
		QDomElement e = it.toElement();
		if( ((e.tagName() == tagActionProp) || (e.tagName() == tagActionProp.toLower()))
				&& (e.attribute("scheme", "Default") == schemeName) ) {
			elem = e;
			break;
		}
	}

	// if there was none, create one
	if( elem.isNull() ) {
		elem = doc.createElement( tagActionProp );
		elem.setAttribute( "scheme", schemeName );
		doc.documentElement().appendChild( elem );
	}
	return elem;
}

QDomElement KXMLGUIFactory::findActionByName( QDomElement& elem, const QString& sName, bool create )
{
        static const QString& attrName = KGlobal::staticQString( "name" );
	static const QString& tagAction = KGlobal::staticQString( "Action" );
	for( QDomNode it = elem.firstChild(); !it.isNull(); it = it.nextSibling() ) {
		QDomElement e = it.toElement();
		if( e.attribute( attrName ) == sName )
			return e;
	}

	if( create ) {
		QDomElement act_elem = elem.ownerDocument().createElement( tagAction );
		act_elem.setAttribute( attrName, sName );
                elem.appendChild( act_elem );
                return act_elem;
	}
        return QDomElement();
}

#include "kxmlguifactory.moc"

/* vim: et sw=4
 */
