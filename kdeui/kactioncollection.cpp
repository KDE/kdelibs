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
#include "ktoolbar.h"
#include "kxmlguiclient.h"
#include "kxmlguifactory.h"

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>

#include <qdom.h>
#include <QSet>
#include <qvariant.h>
#include <QHash>
#include <QChildEvent>
#include <QTimer>

#include <stdio.h>

QList<KActionCollection*> KActionCollection::s_allCollections;

class KActionCollectionPrivate
{
public:
  KActionCollectionPrivate()
  {
    m_instance = 0;
    m_parentGUIClient = 0L;

    defaultShortcutContext = static_cast<Qt::ShortcutContext>(-1);
    configIsGlobal = false;

    connectHighlighted = connectTriggered = false;

    configGroup = "Shortcuts";

    enabled = Unchanged;
  }

  KInstance *m_instance;
  QList<KActionCollection*> m_docList;

  QMultiMap<QString, KAction*> actionDict;
  QList<KAction*> actionList;

  const KXMLGUIClient *m_parentGUIClient;

  QList<QWidget*> associatedWidgets;

  bool configIsGlobal;
  QString configGroup;

  Qt::ShortcutContext defaultShortcutContext;

  bool connectTriggered, connectHighlighted;

  enum ActionEnableStatus {
    Enabled,
    Disabled,
    Unchanged
  } enabled;
};

KActionCollection::KActionCollection( QObject *parent,
                                      KInstance *instance )
  : QObject( parent )
  , d(new KActionCollectionPrivate)
{
  s_allCollections.append(this);

  setInstance( instance );
}

KActionCollection::KActionCollection( const KXMLGUIClient *parent )
  : QObject( 0 )
  , d(new KActionCollectionPrivate)
{
  s_allCollections.append(this);

  d->m_parentGUIClient=parent;
  d->m_instance=parent->instance();
}

KActionCollection::~KActionCollection()
{
  s_allCollections.removeAll(this);

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

void KActionCollection::applyDefaultShortcutContext()
{
  if (defaultShortcutContext() == -1)
    return;

  foreach (KAction* action, actions())
    action->setShortcutContext(defaultShortcutContext());
}

void KActionCollection::addDocCollection( KActionCollection* pDoc )
{
  d->m_docList.append( pDoc );
}

void KActionCollection::clear()
{
  foreach(KAction* pAction, d->actionDict.values())
    delete pAction;
}

KAction* KActionCollection::action( const QString& name ) const
{
  KAction* action = 0L;

  if ( !name.isEmpty() )
    action = d->actionDict.value (name);

  if( !action ) {
    for( int i = 0; i < d->m_docList.count() && !action; i++ )
      action = d->m_docList[i]->action( name );
  }

  return action;
}

QList<KAction*> KActionCollection::actions( const QString& name ) const
{
  QList<KAction*> ret;

  if ( !name.isEmpty() )
    ret += d->actionDict.values(name);
  else
    ret = actions();

  for( int i = 0; i < d->m_docList.count(); i++ )
    ret += d->m_docList[i]->action( name );

  return ret;
}

KAction* KActionCollection::actionOfTypeInternal( const QString& name, const QMetaObject& mo ) const
{
  if (!name.isEmpty())
    foreach (KAction* action, d->actionList)
      if (mo.cast(action))
        return action;

  foreach (KAction* action, d->actionDict.values(name))
    if (mo.cast(action))
      return action;

  KAction* action = 0L;

  for( int i = 0; i < d->m_docList.count() && !action; i++ )
    action = d->m_docList[i]->action( name );

  return action;
}

QList<KAction*> KActionCollection::actionsOfTypeInternal( const QString& name, const QMetaObject& mo ) const
{
  QList<KAction*> ret;

  if (!name.isEmpty()) {
    foreach (KAction* action, d->actionList)
      if (mo.cast(action))
        ret.append(action);

  } else {
    foreach (KAction* action, d->actionDict.values(name))
      if (mo.cast(action))
        ret.append(action);
  }

  for( int i = 0; i < d->m_docList.count(); i++ )
    ret += d->m_docList[i]->actions( name );

  return ret;
}

KAction* KActionCollection::action( int index ) const
{
  if (index < 0 || index >= d->actionList.count())
    return 0L;

  return d->actionList.value(index);
}

int KActionCollection::count() const
{
  return d->actionList.count();
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

const QList< KAction * >& KActionCollection::actions( ) const
{
  return d->actionList;
}

const QList< KAction * > KActionCollection::actionsWithoutGroup( ) const
{
  QList<KAction*> ret;
  foreach (KAction* action, actions())
    if (!action->actionGroup())
      ret.append(action);
  return ret;
}

const QList< QActionGroup * > KActionCollection::actionGroups( ) const
{
  QSet<QActionGroup*> set;
  foreach (KAction* action, actions())
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

  QString name = action->objectName();
  if( name.isEmpty() )
     name = name.sprintf("unnamed-%p", (void*)action);

  // look if we already have THIS action under THIS name ;)
  QMap<QString, KAction*>::const_iterator it = d->actionDict.find (name);
  while (it != d->actionDict.end() && it.key() == name)
  {
    if ( it.value() == action )
      return;

    ++it;
  }

  // really insert action
  d->actionDict.insert(name, action);
  d->actionList.append(action);

  if (d->connectHighlighted)
    connect(action, SIGNAL(highlighted()), SLOT(slotActionHighlighted()));

  if (d->connectTriggered)
    connect(action, SIGNAL(triggered(bool)), SLOT(slotActionTriggered()));

  if (d->associatedWidgets.count()) {
    if (defaultShortcutContext() != -1)
      action->setShortcutContext(defaultShortcutContext());
    else
      action->setShortcutContext(Qt::WidgetShortcut);
    foreach (QWidget* w, d->associatedWidgets)
      w->addAction(action);

  } else if (defaultShortcutContext() != -1) {
    action->setShortcutContext(defaultShortcutContext());
  }

  if (d->enabled != KActionCollectionPrivate::Unchanged)
    action->setEnabled(isEnabled());

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

  KAction *a = d->actionDict.take( name );
  if ( !a || a != action )
      return 0;

  int index = d->actionList.indexOf(action);
  Q_ASSERT(index != -1);
  d->actionList.removeAt(index);

  if (d->connectHighlighted)
    disconnect(action, SIGNAL(highlighted()), this, SLOT(slotActionHighlighted()));

  if (d->connectTriggered)
    disconnect(action, SIGNAL(triggered(bool)), this, SLOT(slotActionTriggered()));

  if (d->associatedWidgets.count())
    foreach (QWidget* w, d->associatedWidgets)
      w->removeAction(action);

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

  foreach (KAction* action, actions()) {
    if (defaultShortcutContext() == -1)
      action->setShortcutContext(Qt::WidgetShortcut);
    widget->addAction(action);
  }
}

void KActionCollection::removeAssociatedWidget(QWidget* widget)
{
  d->associatedWidgets.removeAll(widget);

  foreach (KAction* action, actions())
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

  kDebug(125) << "\treadSettings( \"" << configGroup() << "\", " << config << " )" << endl;
  if( !config->hasGroup( configGroup() ) )
    return;

  KConfigGroup cg( config, configGroup() );

  foreach (KAction* action, actions()) {
    if( action->isShortcutConfigurable() ) {
      QString entry = cg.readEntry( action->objectName(), QString() );
      if( !entry.isEmpty() ) {
        if( entry == "none" )
          action->setShortcut( KShortcut(), KAction::CustomShortcut );
        else
          action->setShortcut( KShortcut(entry), KAction::CustomShortcut );
      }
      else // default shortcut
        action->setShortcut( action->defaultShortcut() );

      kDebug(125) << "\t" << action->objectName() << " = '" << entry << "'" << endl;
    }
  }

  kDebug(125) << k_funcinfo << " done" << endl;
}

void KActionCollection::writeSettings( KConfigBase* config, bool writeAll, KAction* oneAction ) const
{
  kDebug(125) << k_funcinfo << configGroup() << ", " << config << ", " << writeAll << ", " << configIsGlobal() << " )" << endl;

  if (parentGUIClient() && !parentGUIClient()->xmlFile().isEmpty()) {
    kDebug(129) << "KActionCollection::save(): xmlFile = " << parentGUIClient()->xmlFile() << endl;

    QString attrShortcut  = QLatin1String("shortcut");

    // Read XML file
    QString sXml( KXMLGUIFactory::readConfigFile( parentGUIClient()->xmlFile(), false, instance() ) );
    QDomDocument doc;
    doc.setContent( sXml );

    // Process XML data

    // Get hold of ActionProperties tag
    QDomElement elem = KXMLGUIFactory::actionPropertiesElement( doc );

    // now, iterate through our actions
    foreach (KAction* action, actions()) {
        bool bSameAsDefault = (action->shortcut() == action->defaultShortcut());
        //kdDebug(129) << "name = " << sName << " shortcut = " << shortcut(i).toStringInternal() << " def = " << shortcutDefault(i).toStringInternal() << endl;

        // now see if this element already exists
        // and create it if necessary (unless bSameAsDefault)
        QDomElement act_elem = KXMLGUIFactory::findActionByName( elem, action->objectName(), !bSameAsDefault );
        if ( act_elem.isNull() )
            continue;

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
    KXMLGUIFactory::saveConfigFile( doc, parentGUIClient()->xmlFile(), instance() );
    return;
  }

  if( !config )
      config = KGlobal::config();

  KConfigGroup cg( config, configGroup() );

  QList<KAction*> writeActions;
  if (oneAction)
    writeActions.append(oneAction);
  else
    writeActions = actions();

  foreach (KAction* action, writeActions) {
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

void KActionCollection::slotActionTriggered( )
{
  KAction* action = qobject_cast<KAction*>(sender());
  if (action)
    emit actionTriggered(action);
}

void KActionCollection::slotActionHighlighted( )
{
  KAction* action = qobject_cast<KAction*>(sender());
  if (action)
    emit actionHighlighted(action);
}

void KActionCollection::connectNotify ( const char * signal )
{
  if (d->connectHighlighted && d->connectTriggered)
    return;

  if (signal == SIGNAL(actionHighlighted(KAction*))) {
    if (!d->connectHighlighted) {
      d->connectHighlighted = true;
      foreach (KAction* action, actions())
        connect(action, SIGNAL(highlighted()), SLOT(slotActionHighlighted()));
    }

  } else if (signal == SIGNAL(actionTriggered(KAction*))) {
    if (!d->connectTriggered) {
      d->connectTriggered = true;
      foreach (KAction* action, actions())
        connect(action, SIGNAL(triggered(bool)), SLOT(slotActionTriggered()));
    }
  }

  QObject::connectNotify(signal);
}

const QList< KActionCollection * >& KActionCollection::allCollections( )
{
	return s_allCollections;
}

bool KActionCollection::isEnabled( ) const
{
  return d->enabled == KActionCollectionPrivate::Unchanged || d->enabled == KActionCollectionPrivate::Enabled;
}

void KActionCollection::setEnabled( bool enable )
{
  if ((d->enabled == KActionCollectionPrivate::Unchanged) || (enable ? d->enabled == KActionCollectionPrivate::Disabled : d->enabled == KActionCollectionPrivate::Enabled)) {
    d->enabled = enable ? KActionCollectionPrivate::Enabled : KActionCollectionPrivate::Disabled;
    foreach (KAction* action, actions())
      action->setEnabled(enable);
  }
}

void KActionCollection::forgetEnabled( )
{
  d->enabled = KActionCollectionPrivate::Unchanged;
}

/* vim: et sw=2 ts=2
 */

#include "kactioncollection.moc"
