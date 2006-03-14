/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
              (C) 2002 Joseph Wenninger <jowenn@kde.org>
              (C) 2005-2006 Hamish Rodda <rodda@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kactioncollection.h"

#include "kaction.h"
#include "kactionshortcutlist.h"
#include "ktoolbar.h"
#include "kxmlguiclient.h"
#include "kxmlguifactory.h"

#include <kconfig.h>
#include <kdebug.h>

#include <QSet>
#include <qvariant.h>
#include <QHash>
#include <QChildEvent>
#include <QTimer>

#include <stdio.h>

class KActionCollection::KActionCollectionPrivate
{
public:
  KActionCollectionPrivate()
  {
    m_instance = 0;
    m_parentGUIClient = 0L;

    defaultShortcutContext = Qt::WindowShortcut;
    configIsGlobal = false;
  }

  KInstance *m_instance;
  QList<KActionCollection*> m_docList;

  QMultiMap<QByteArray, KAction*> m_actionDict;
  const KXMLGUIClient *m_parentGUIClient;

  QList<QWidget*> associatedWidgets;
  QList<QObject*> addedChildren;

  bool configIsGlobal;
  QString configGroup;
  QString xmlFile;

  Qt::ShortcutContext defaultShortcutContext;
};

void KActionCollection::setXMLFile( const QString& xmlFile )
{
  d->xmlFile = xmlFile;
}

const QString& KActionCollection::xmlFile() const
{
  return d->xmlFile;
}

KActionCollection::KActionCollection( QObject *parent,
                                      KInstance *instance )
  : QObject( parent )
  , d(new KActionCollectionPrivate)
{
  setInstance( instance );
}

KActionCollection::KActionCollection( const KXMLGUIClient *parent )
  : QObject( 0 )
  , d(new KActionCollectionPrivate)
{
  d->m_parentGUIClient=parent;
  d->m_instance=parent->instance();
}

KActionCollection::~KActionCollection()
{
  delete d;
}

void KActionCollection::setDefaultShortcutContext( Qt::ShortcutContext context )
{
  d->defaultShortcutContext = context;
}

Qt::ShortcutContext KActionCollection::defaultShortcutContext( ) const
{
  return d->defaultShortcutContext;
}

void KActionCollection::addDocCollection( KActionCollection* pDoc )
{
  d->m_docList.append( pDoc );
}

void KActionCollection::clear()
{
  foreach(KAction* pAction, d->m_actionDict.values())
    delete pAction;
}

KAction* KActionCollection::action( const char* name, const char* classname ) const
{
  KAction* pAction = 0;

  if ( !classname && name )
    pAction = d->m_actionDict.value (name);

  else {
    foreach( KAction* itAction, d->m_actionDict )
    {
      if ( ( !name || !itAction->objectName().compare( name ) )  &&
          ( !classname || !strcmp( itAction->metaObject()->className(), classname ) ) ) {
        pAction = itAction;
        break;
      }
    }
  }

  if( !pAction ) {
    for( int i = 0; i < d->m_docList.count() && !pAction; i++ )
      pAction = d->m_docList[i]->action( name, classname );
  }

  return pAction;
}

KAction* KActionCollection::action( int index ) const
{
  return *(d->m_actionDict.begin() + index);
}

int KActionCollection::count() const
{
  return d->m_actionDict.count();
}

void KActionCollection::setInstance( KInstance *instance )
{
  if ( instance )
    d->m_instance = instance;
  else
    d->m_instance = KGlobal::instance();
}

KInstance *KActionCollection::instance() const
{
  return d->m_instance;
}

const KXMLGUIClient *KActionCollection::parentGUIClient() const
{
	return d->m_parentGUIClient;
}

const QList< KAction * > KActionCollection::actions( ) const
{
  QList<KAction*> ret;
  foreach (QObject* object, children())
    if (KAction* action = qobject_cast<KAction*>(object))
      ret.append(action);
  return ret;
}

const QList< KAction * > KActionCollection::actionsWithoutGroup( ) const
{
  QList<KAction*> ret;
  foreach (QObject* object, children())
    if (KAction* action = qobject_cast<KAction*>(object))
      if (!action->actionGroup())
        ret.append(action);
  return ret;
}

const QList< QActionGroup * > KActionCollection::actionGroups( ) const
{
  QSet<QActionGroup*> set;
  foreach (QObject* object, children())
    if (KAction* action = qobject_cast<KAction*>(object))
      if (action->actionGroup())
        set.insert(action->actionGroup());
  return set.toList();
}

const QList< KAction* > KActionCollection::actionsInGroup( QActionGroup * group ) const
{
  QList<KAction*> ret;
  foreach (QAction* action, group->actions())
    if (action->parent() == this)
      if (KAction* kaction = qobject_cast<KAction*>(action))
        ret.append(kaction);
  return ret;
}

void KActionCollection::insert( KAction* action )
{
  if (!action)
    return;

  char unnamed_name[100];
  QByteArray name = action->objectName().toLatin1();
  if( action->objectName().isEmpty() )
  {
     sprintf(unnamed_name, "unnamed-%p", (void *)action);
     name = QByteArray(unnamed_name);
  }

  // look if we already have THIS action under THIS name ;)
  QMap<QByteArray, KAction*>::const_iterator it = d->m_actionDict.find (name);
  while (it != d->m_actionDict.end() && it.key() == name)
  {
    if ( it.value() == action )
      return;

    ++it;
  }

  // really insert action
  d->m_actionDict.insert(name, action);

  emit inserted( action );
}

void KActionCollection::remove( KAction* action )
{
  delete take( action );
}

KAction* KActionCollection::take( KAction* action )
{
  if (!action)
    return 0;

  char unnamed_name[100];
  QByteArray name = action->objectName().toLatin1();
  if( action->objectName().isEmpty() )
  {
     sprintf(unnamed_name, "unnamed-%p", (void *) action);
     name = QByteArray(unnamed_name);
  }

  KAction *a = d->m_actionDict.take( name );
  if ( !a || a != action )
      return 0;

  if ( a->parentCollection() == this )
      a->setParent(0L);

  emit removed( action );
  return a;
}

void KActionCollection::setAssociatedWidget(QWidget* widget)
{
  clearAssociatedWidgets();
  addAssociatedWidget(widget);
}

void KActionCollection::addAssociatedWidget(QWidget* widget)
{
  d->associatedWidgets.append(widget);

  foreach (KAction* action, findChildren<KAction*>()) {
    action->setShortcutContext(Qt::WidgetShortcut);
    widget->addAction(action);
  }
}

void KActionCollection::removeAssociatedWidget(QWidget* widget)
{
  d->associatedWidgets.removeAll(widget);

  foreach (KAction* action, findChildren<KAction*>())
    widget->removeAction(action);
}

void KActionCollection::clearAssociatedWidgets()
{
  while (d->associatedWidgets.count())
    removeAssociatedWidget(d->associatedWidgets.first());
}

const QList<QWidget*>& KActionCollection::associatedWidgets() const
{
  return d->associatedWidgets;
}

void KActionCollection::childEvent(QChildEvent* event)
{
  if (d->associatedWidgets.count() || defaultShortcutContext() != Qt::WindowShortcut) {
    if (event->added()) {
      if (!d->addedChildren.count())
        QTimer::singleShot(0, this, SLOT(processAddedChildren()));

      d->addedChildren.append(event->child());

    } else if (event->removed()) {
      if (QAction* action = qobject_cast<QAction*>(event->child()))
        foreach (QWidget* w, d->associatedWidgets)
          w->removeAction(action);
    }
  }

  QObject::childEvent(event);
}

void KActionCollection::processAddedChildren()
{
  foreach (QObject* object, d->addedChildren) {
    if (QAction* action = qobject_cast<QAction*>(object)) {
      if (d->associatedWidgets.count()) {
        action->setShortcutContext(Qt::WidgetShortcut);
        foreach (QWidget* w, d->associatedWidgets)
          w->addAction(action);

      } else {
        action->setShortcutContext(defaultShortcutContext());
      }
    }
  }

  d->addedChildren.clear();
}

const QString & KActionCollection::configGroup( ) const
{
  return d->configGroup;
}

void KActionCollection::setConfigGroup( const QString & group )
{
  d->configGroup = group;
}

bool KActionCollection::configIsGlobal() const
{
  return d->configIsGlobal;
}

void KActionCollection::setConfigGlobal( bool global )
{
  d->configIsGlobal = global;
}

void KActionCollection::readSettings( KConfigBase* config )
{
  kDebug(125) << k_funcinfo << " ( \"" << configGroup() << "\", " << config << " ) start" << endl;
  if( !config )
    config = KGlobal::config();

  QString sGroup = (!configGroup().isEmpty()) ? configGroup() : QString("Shortcuts");

  kDebug(125) << "\treadSettings( \"" << sGroup << "\", " << config << " )" << endl;
  if( !config->hasGroup( sGroup ) )
    return;

  KConfigGroup cg( config, sGroup );

  foreach (KAction* action, findChildren<KAction*>()) {
    if( action->isShortcutConfigurable() ) {
      QString entry = cg.readEntry( action->objectName(), QString() );
      if( !entry.isEmpty() ) {
        if( entry == "none" )
          action->setShortcut( KShortcut() );
        else
          action->setShortcut( KShortcut(entry) );
      }
      else // default shortcut
        action->setShortcut( action->defaultShortcut() );

      kDebug(125) << "\t" << action->objectName() << " = '" << entry << "'" << endl;
    }
  }

  kDebug(125) << k_funcinfo << " done" << endl;
}

void KActionCollection::writeSettings( KConfigBase* config, bool writeAll ) const
{
  kDebug(125) << k_funcinfo << configGroup() << ", " << config << ", " << writeAll << ", " << configIsGlobal() << " )" << endl;
  if( !config )
      config = KGlobal::config();

  QString sGroup = (!configGroup().isEmpty()) ? configGroup() : QString("Shortcuts");

  KConfigGroup cg( config, sGroup );

  foreach (KAction* action, findChildren<KAction*>()) {
    if( action->isShortcutConfigurable() ) {
      bool bConfigHasAction = !cg.readEntry( action->objectName(), QString() ).isEmpty();
      bool bSameAsDefault = (action->shortcut() == action->defaultShortcut());
      // If we're using a global config or this setting
      //  differs from the default, then we want to write.
      if( writeAll || !bSameAsDefault ) {
        QString s = action->shortcut().toStringInternal();
        if( s.isEmpty() )
          s = "none";
        kDebug(125) << "\twriting " << action->objectName() << " = " << s << endl;
        cg.writeEntry( action->objectName(), s, (configIsGlobal() ? KConfigBase::Global : KConfigBase::Normal) );
      }
      // Otherwise, this key is the same as default
      //  but exists in config file.  Remove it.
      else if( bConfigHasAction ) {
        kDebug(125) << "\tremoving " << action->objectName() << " because == default" << endl;
        cg.deleteEntry( action->objectName(), (configIsGlobal() ? KConfigBase::Global : KConfigBase::Normal) );
      }
    }
  }

  config->sync();
}

//---------------------------------------------------------------------
// KActionShortcutList
//---------------------------------------------------------------------

KActionShortcutList::KActionShortcutList( KActionCollection* pColl )
: m_actions( *pColl )
    { }
KActionShortcutList::~KActionShortcutList()
    { }
uint KActionShortcutList::count() const
    { return m_actions.count(); }
QString KActionShortcutList::name( uint i ) const
    { return m_actions.action(i)->objectName(); }
QString KActionShortcutList::label( uint i ) const
    { return m_actions.action(i)->text(); }
QString KActionShortcutList::whatsThis( uint i ) const
    { return m_actions.action(i)->whatsThis(); }
const KShortcut& KActionShortcutList::shortcut( uint i ) const
    { return m_actions.action(i)->shortcut(); }
const KShortcut& KActionShortcutList::shortcutDefault( uint i ) const
    { return m_actions.action(i)->defaultShortcut(); }
bool KActionShortcutList::isConfigurable( uint i ) const
    { return m_actions.action(i)->isShortcutConfigurable(); }
bool KActionShortcutList::setShortcut( uint i, const KShortcut& cut )
    { if (i < m_actions.count()) { m_actions.action(i)->setShortcut( cut ); return true; } return false; }
const KInstance* KActionShortcutList::instance() const
    { return m_actions.instance(); }
QVariant KActionShortcutList::getOther( Other, uint ) const
    { return QVariant(); }
bool KActionShortcutList::setOther( Other, uint, const QVariant &)
    { return false; }
const KAction *KActionShortcutList::action( uint i) const
    { return m_actions.action(i); }

bool KActionShortcutList::save() const
{
    const KXMLGUIClient* guiClient=m_actions.parentGUIClient();
    const QString xmlFile=guiClient ? guiClient->xmlFile() : m_actions.xmlFile();
    kDebug(129) << "KActionShortcutList::save(): xmlFile = " << xmlFile << endl;

    if( m_actions.xmlFile().isEmpty() )
        return writeSettings();

    QString attrShortcut  = QLatin1String("shortcut");
    QString attrAccel     = QLatin1String("accel"); // Depricated attribute

    // Read XML file
    QString sXml( KXMLGUIFactory::readConfigFile( xmlFile, false, instance() ) );
    QDomDocument doc;
    doc.setContent( sXml );

    // Process XML data

    // Get hold of ActionProperties tag
    QDomElement elem = KXMLGUIFactory::actionPropertiesElement( doc );

    // now, iterate through our actions
    foreach (KAction* action, m_actions.findChildren<KAction*>()) {
        bool bSameAsDefault = (action->shortcut() == action->defaultShortcut());
        //kdDebug(129) << "name = " << sName << " shortcut = " << shortcut(i).toStringInternal() << " def = " << shortcutDefault(i).toStringInternal() << endl;

        // now see if this element already exists
        // and create it if necessary (unless bSameAsDefault)
        QDomElement act_elem = KXMLGUIFactory::findActionByName( elem, action->objectName(), !bSameAsDefault );
        if ( act_elem.isNull() )
            continue;

        act_elem.removeAttribute( attrAccel );
        if( bSameAsDefault ) {
            act_elem.removeAttribute( attrShortcut );
            //kdDebug(129) << "act_elem.attributes().count() = " << act_elem.attributes().count() << endl;
            if( act_elem.attributes().count() == 1 )
                elem.removeChild( act_elem );
        } else {
            act_elem.setAttribute( attrShortcut, action->shortcut().toStringInternal() );
        }
    }

    // Write back to XML file
    return KXMLGUIFactory::saveConfigFile( doc, guiClient ? guiClient->localXMLFile() : m_actions.xmlFile(), instance() );
}

//---------------------------------------------------------------------
// KActionPtrShortcutList
//---------------------------------------------------------------------

KActionPtrShortcutList::KActionPtrShortcutList( const QList<KAction*>& list )
: m_actions( list )
    { }
KActionPtrShortcutList::~KActionPtrShortcutList()
    { }
uint KActionPtrShortcutList::count() const
    { return m_actions.count(); }
QString KActionPtrShortcutList::name( uint i ) const
    { return m_actions[i]->objectName(); }
QString KActionPtrShortcutList::label( uint i ) const
    { return m_actions[i]->text(); }
QString KActionPtrShortcutList::whatsThis( uint i ) const
    { return m_actions[i]->whatsThis(); }
const KShortcut& KActionPtrShortcutList::shortcut( uint i ) const
    { return m_actions[i]->shortcut(); }
const KShortcut& KActionPtrShortcutList::shortcutDefault( uint i ) const
    { return m_actions[i]->defaultShortcut(); }
bool KActionPtrShortcutList::isConfigurable( uint i ) const
    { return m_actions[i]->isShortcutConfigurable(); }
bool KActionPtrShortcutList::setShortcut( uint i, const KShortcut& cut )
    { if (i < m_actions.count()) { m_actions[i]->setShortcut( cut ); return true; } return false; }
QVariant KActionPtrShortcutList::getOther( Other, uint ) const
    { return QVariant(); }
bool KActionPtrShortcutList::setOther( Other, uint, const QVariant &)
    { return false; }

bool KActionPtrShortcutList::save() const
    { return false; }

void KActionShortcutList::virtual_hook( int id, void* data )
{ KShortcutList::virtual_hook( id, data ); }

void KActionPtrShortcutList::virtual_hook( int id, void* data )
{ KShortcutList::virtual_hook( id, data ); }


void KActionCollection::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

/* vim: et sw=2 ts=2
 */

#include "kactioncollection.moc"
