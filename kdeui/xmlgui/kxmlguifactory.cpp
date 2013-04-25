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
#include <kshortcutschemeshelper_p.h>
#include "kxmlguiclient.h"
#include "kxmlguibuilder.h"

#include <ksharedconfig.h>

#include <QAction>
#include <QtCore/QDir>
#include <QtXml/QDomDocument>
#include <QtCore/QFile>
#include <QtCore/QCoreApplication>
#include <QtCore/QTextStream>
#include <QWidget>
#include <QtCore/QDate>
#include <QtCore/QVariant>
#include <QTextCodec>
#include <qstandardpaths.h>

#include <kconfiggroup.h>
#include <kdebug.h>

#include "kshortcutsdialog.h"
#include "kactioncollection.h"
#include "kglobalaccel.h"

Q_DECLARE_METATYPE(QList<QKeySequence>)

using namespace KXMLGUI;

class KXMLGUIFactoryPrivate : public BuildState
{
public:
    enum ShortcutOption { SetActiveShortcut = 1, SetDefaultShortcut = 2};

    KXMLGUIFactoryPrivate()
    {
        m_rootNode = new ContainerNode( 0L, QString(), 0L );
        m_defaultMergingName = QLatin1String("<default>");
        tagActionList = QLatin1String("actionlist");
        attrName = QLatin1String("name");
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
    void applyShortcutScheme(KXMLGUIClient *client, const QList<QAction*>& actions, const QDomDocument& scheme);
    void refreshActionProperties(KXMLGUIClient *client, const QList<QAction*>& actions, const QDomDocument& doc);
    void saveDefaultActionProperties(const QList<QAction*>& actions);

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

QString KXMLGUIFactory::readConfigFile(const QString &filename, const QString &_componentName)
{
    QString componentName = _componentName.isEmpty() ? QCoreApplication::applicationName() : _componentName;
    QString xml_file;

    if (!QDir::isRelativePath(filename))
        xml_file = filename;
    else
    {
        xml_file = QStandardPaths::locate(QStandardPaths::GenericDataLocation, componentName + '/' + filename);
        if ( !QFile::exists( xml_file ) )
          xml_file = QStandardPaths::locate(QStandardPaths::GenericDataLocation, filename);
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

bool KXMLGUIFactory::saveConfigFile(const QDomDocument& doc,
                                    const QString& filename, const QString &_componentName)
{
    QString componentName = _componentName.isEmpty() ? QCoreApplication::applicationName() : _componentName;
    QString xml_file(filename);

    if (QDir::isRelativePath(xml_file))
        xml_file = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + '/' + componentName + '/' + filename;

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
    d->saveDefaultActionProperties(client->actionCollection()->actions());
    if (!doc.isNull())
        d->refreshActionProperties(client, client->actionCollection()->actions(), doc);

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
        d->refreshActionProperties(client, client->actionCollection()->actions(), doc);
    }
    d->guiClient = 0;
}

static QString currentShortcutScheme()
{
    const KConfigGroup cg = KSharedConfig::openConfig()->group("Shortcut Schemes");
    return cg.readEntry("Current Scheme", "Default");
}

// Find the right ActionProperties element, otherwise return null element
static QDomElement findActionPropertiesElement(const QDomDocument& doc)
{
    const QLatin1String tagActionProp("ActionProperties");
    const QString schemeName = currentShortcutScheme();
    QDomElement e = doc.documentElement().firstChildElement();
    for( ; !e.isNull(); e = e.nextSiblingElement() ) {
        if (QString::compare(e.tagName(), tagActionProp, Qt::CaseInsensitive) == 0
            && (e.attribute("scheme", "Default") == schemeName) ) {
            return e;
        }
    }
    return QDomElement();
}

void KXMLGUIFactoryPrivate::refreshActionProperties(KXMLGUIClient *client, const QList<QAction*>& actions, const QDomDocument& doc)
{
    // try to find and apply shortcuts schemes
    QDomDocument scheme = shortcutSchemeDoc(client);
    applyShortcutScheme(client, actions, scheme);

    // try to find and apply user-defined shortcuts
    const QDomElement actionPropElement = findActionPropertiesElement(doc);
    if ( !actionPropElement.isNull() )
        applyActionProperties( actionPropElement );
}

void KXMLGUIFactoryPrivate::saveDefaultActionProperties(const QList<QAction *>& actions)
{
    // This method is called every time the user activated a new
    // kxmlguiclient. We only want to execute the following code only once in
    // the lifetime of an action.
    foreach (QAction *action, actions) {
        // Skip NULL actions or those we have seen already.
        if (!action || action->property("_k_DefaultShortcut").isValid()) continue;

        // Check if the default shortcut is set
        QList<QKeySequence> defaultShortcut = action->property("defaultShortcuts").value<QList<QKeySequence> >();
        QList<QKeySequence> activeShortcut = action->shortcuts();
        //kDebug() << action->objectName() << "default=" << defaultShortcut.toString() << "active=" << activeShortcut.toString();

        // Check if we have an empty default shortcut and an non empty
        // custom shortcut. This should only happen if a developer called
        // QAction::setShortcut on an KAction. Print out a warning and
        // correct the mistake
        if ((!activeShortcut.isEmpty()) && defaultShortcut.isEmpty()) {
            kError(240) << "Shortcut for KAction " << action->objectName() << action->text() << "set with QShortcut::setShortcut()! See KAction documentation.";
            action->setProperty("_k_DefaultShortcut", QVariant::fromValue(activeShortcut));
        } else {
            action->setProperty("_k_DefaultShortcut", QVariant::fromValue(defaultShortcut));
        }
    }
}

void KXMLGUIFactory::changeShortcutScheme(const QString &scheme)
{
    kDebug(260) << "Changing shortcut scheme to" << scheme;
    KConfigGroup cg = KSharedConfig::openConfig()->group( "Shortcut Schemes" );
    cg.writeEntry("Current Scheme", scheme);

    refreshActionProperties();
}

void KXMLGUIFactory::forgetClient( KXMLGUIClient *client )
{
    d->m_clients.removeAll( client );
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

    // Load shortcuts for these new actions
    d->saveDefaultActionProperties(actionList);
    d->refreshActionProperties(client, actionList, client->domDocument());

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
    for (QDomElement e = actionPropElement.firstChildElement();
         !e.isNull(); e = e.nextSiblingElement()) {
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
    for (int i = 0; i < attributes.length(); i++)
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
    QString attrName = attribute.name();
    // If the attribute is a deprecated "accel", change to "shortcut".
    if ( equals(attrName, "accel") )
        attrName = QLatin1String("shortcut");

    // No need to re-set name, particularly since it's "objectName" in Qt4
    if ( equals(attrName, "name") )
        return;

    if ( equals(attrName, "icon") ) {
        action->setIcon( QIcon::fromTheme( attribute.value() ) );
        return;
    }

    QVariant propertyValue;

    QVariant::Type propertyType = action->property( attrName.toLatin1() ).type();

    if ( propertyType == QVariant::Int ) {
        propertyValue = QVariant( attribute.value().toInt() );
    } else if ( propertyType == QVariant::UInt ) {
        propertyValue = QVariant( attribute.value().toUInt() );
    } else if ( propertyType == QVariant::UserType && action->property( attrName.toLatin1() ).userType() == qMetaTypeId<QList<QKeySequence> >() ) {
        // Setting the shortcut by property also sets the default shortcut (which is incorrect), so we have to do it directly
        if (attrName=="globalShortcut") {
            KGlobalAccel::self()->setShortcut(action, QKeySequence::listFromString(attribute.value()));
        } else {
            action->setShortcuts(QKeySequence::listFromString(attribute.value()));
        }
        if (shortcutOption & KXMLGUIFactoryPrivate::SetDefaultShortcut)
            action->setProperty("defaultShortcuts", QVariant::fromValue(QKeySequence::listFromString(attribute.value())));
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
    KConfigGroup cg = KSharedConfig::openConfig()->group( "Shortcut Schemes" );
    QString schemeName = cg.readEntry("Current Scheme", "Default");

    QDomDocument doc;
    if (schemeName != "Default")
    {
        // Find the document for the shortcut scheme using both current application path
        // and current xmlguiclient path but making a preference to app path
        QString schemeFileName = KShortcutSchemesHelper::shortcutSchemeFileName(client, schemeName);
        QFile schemeFile(schemeFileName);
        if (schemeFile.open(QIODevice::ReadOnly))
        {
//             kDebug(260) << "Found shortcut scheme" << schemeFileName;
            doc.setContent(&schemeFile);
        }
    }
    return doc;
}

void KXMLGUIFactoryPrivate::applyShortcutScheme(KXMLGUIClient *client, const QList<QAction*> &actions, const QDomDocument& scheme)
{
    KConfigGroup cg = KSharedConfig::openConfig()->group( "Shortcut Schemes" );
    QString schemeName = cg.readEntry("Current Scheme", "Default");

    //First clear all existing shortcuts
    if (schemeName != "Default") {
        foreach (QAction *action, actions) {
            action->setShortcuts(QList<QKeySequence>());
            // We clear the default shortcut as well because the shortcut scheme will set its own defaults
            action->setProperty("defaultShortcuts", QVariant::fromValue(QList<QKeySequence>()));
        }
    } else {
        // apply saved default shortcuts
        foreach (QAction *action, actions) {
            QVariant savedDefaultShortcut = action->property("_k_DefaultShortcut");
            if (savedDefaultShortcut.isValid()) {
                QList<QKeySequence> shortcut = savedDefaultShortcut.value<QList<QKeySequence> >();
                //kDebug() << "scheme said" << shortcut.toString() << "for action" << kaction->objectName();
                action->setShortcuts(shortcut);
                action->setProperty("defaultShortcuts", QVariant::fromValue(shortcut));
            }
        }
    }

    if (scheme.isNull())
        return;

    QDomElement docElement = scheme.documentElement();
    QDomElement actionPropElement = docElement.namedItem( QLatin1String("ActionProperties") ).toElement();

    //Check if we really have the shortcut configuration here
    if (!actionPropElement.isNull()) {
        kDebug(260) << "Applying shortcut scheme for XMLGUI client" << client->componentName();

        //Apply all shortcuts we have
        applyActionProperties(actionPropElement, KXMLGUIFactoryPrivate::SetDefaultShortcut);
    } else {
        kDebug(260) << "Invalid shortcut scheme file";
    }
}

int KXMLGUIFactory::configureShortcuts(bool letterCutsOk , bool bSaveSettings )
{
    KShortcutsDialog dlg(KShortcutsEditor::AllActions,
                         letterCutsOk ? KShortcutsEditor::LetterShortcutsAllowed : KShortcutsEditor::LetterShortcutsDisallowed,
                         qobject_cast<QWidget*>(parent()));
    foreach (KXMLGUIClient *client, d->m_clients) {
        if(client) {
            dlg.addCollection(client->actionCollection());
        }
    }
    return dlg.configure(bSaveSettings);
}

// Find or create
QDomElement KXMLGUIFactory::actionPropertiesElement( QDomDocument& doc )
{
    // first, lets see if we have existing properties
    QDomElement elem = findActionPropertiesElement(doc);

    // if there was none, create one
    if(elem.isNull()) {
        elem = doc.createElement(QLatin1String("ActionProperties"));
        elem.setAttribute("scheme", currentShortcutScheme());
        doc.documentElement().appendChild( elem );
    }
    return elem;
}

QDomElement KXMLGUIFactory::findActionByName( QDomElement& elem, const QString& sName, bool create )
{
  const QLatin1String attrName( "name" );
	for( QDomNode it = elem.firstChild(); !it.isNull(); it = it.nextSibling() ) {
		QDomElement e = it.toElement();
		if( e.attribute( attrName ) == sName )
			return e;
	}

	if( create ) {
		QDomElement act_elem = elem.ownerDocument().createElement( QLatin1String("Action") );
		act_elem.setAttribute( attrName, sName );
                elem.appendChild( act_elem );
                return act_elem;
	}
        return QDomElement();
}


/* vim: et sw=4
 */
