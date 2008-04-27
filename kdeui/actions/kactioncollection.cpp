/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
              (C) 2002 Joseph Wenninger <jowenn@kde.org>
              (C) 2005-2007 Hamish Rodda <rodda@kde.org>

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
#include <kauthorized.h>
#include "ktoolbar.h"
#include "kxmlguiclient.h"
#include "kxmlguifactory.h"

#include "kdebug.h"
#include "kglobal.h"
#include "kaction.h"

#include <QtXml/QDomDocument>
#include <QSet>
#include <QtCore/QVariant>
#include <QHash>
#include <QChildEvent>
#include <QTimer>
#include <QAction>

#include <stdio.h>
#include "kcomponentdata.h"
#include "kconfiggroup.h"
#include "klocale.h"

class KActionCollectionPrivate
{
public:
  KActionCollectionPrivate()
  {
    q = 0;
    m_parentGUIClient = 0L;

    configIsGlobal = false;

    connectHovered = connectTriggered = false;

    configGroup = "Shortcuts";
  }

  static QList<KActionCollection*> s_allCollections;

  void _k_associatedWidgetDestroyed(QObject *obj);
  void _k_actionDestroyed(QObject *obj);

  bool writeKXMLGUIConfigFile();

  KComponentData m_componentData;

  QMap<QString, QAction*> actionByName;
  QHash<QAction *, QString> nameByAction;

  const KXMLGUIClient *m_parentGUIClient;

  QString configGroup;
  bool configIsGlobal : 1;

  bool connectTriggered : 1;
  bool connectHovered : 1;

  KActionCollection *q;

  QList<QWidget*> associatedWidgets;
};

QList<KActionCollection*> KActionCollectionPrivate::s_allCollections;

KActionCollection::KActionCollection(QObject *parent, const KComponentData &cData)
  : QObject( parent )
  , d(new KActionCollectionPrivate)
{
  d->q = this;
  KActionCollectionPrivate::s_allCollections.append(this);

  setComponentData(cData);
}

KActionCollection::KActionCollection( const KXMLGUIClient *parent )
  : QObject( 0 )
  , d(new KActionCollectionPrivate)
{
  d->q = this;
  KActionCollectionPrivate::s_allCollections.append(this);

  d->m_parentGUIClient=parent;
  d->m_componentData = parent->componentData();
}

KActionCollection::~KActionCollection()
{
  KActionCollectionPrivate::s_allCollections.removeAll(this);

  delete d;
}

void KActionCollection::clear()
{
  QList<QAction *> actions = d->nameByAction.keys();
  d->actionByName.clear();
  d->nameByAction.clear();
  qDeleteAll(actions);
}

QAction* KActionCollection::action( const QString& name ) const
{
  QAction* action = 0L;

  if ( !name.isEmpty() )
    action = d->actionByName.value (name);

  return action;
}

QAction* KActionCollection::action( int index ) const
{
  // ### investigate if any apps use this at all
  return actions().value(index);
}

int KActionCollection::count() const
{
  return d->nameByAction.count();
}

bool KActionCollection::isEmpty() const
{
  return count() == 0;
}

void KActionCollection::setComponentData(const KComponentData &cData)
{
  if (count()>0) {
    // The actioncollection contains actions.  Trigger which have to be
    // registered permanently with some service ( currently only global
    // shortcuts ) will stop to work when the action ( through it's collection
    // ) changes the component. Prohibit that.
    kWarning(129) << "Attempt to call setComponentData() on a KActionCollection containing actions!";
  }

  if (cData.isValid()) {
    d->m_componentData = cData;
  } else {
    d->m_componentData = KGlobal::mainComponent();
  }
}

KComponentData KActionCollection::componentData() const
{
  return d->m_componentData;
}

const KXMLGUIClient *KActionCollection::parentGUIClient() const
{
	return d->m_parentGUIClient;
}

QList<QAction*> KActionCollection::actions() const
{
  return d->actionByName.values();
}

const QList< QAction* > KActionCollection::actionsWithoutGroup( ) const
{
  QList<QAction*> ret;
  foreach (QAction* action, d->actionByName)
    if (!action->actionGroup())
      ret.append(action);
  return ret;
}

const QList< QActionGroup * > KActionCollection::actionGroups( ) const
{
  QSet<QActionGroup*> set;
  foreach (QAction* action, d->actionByName)
    if (action->actionGroup())
      set.insert(action->actionGroup());
  return set.toList();
}

KAction *KActionCollection::addAction(const QString &name, KAction *action)
{
    QAction* ret = addAction(name, static_cast<QAction*>(action));
    Q_ASSERT(ret == action);
    return action;
}

QAction *KActionCollection::addAction(const QString &name, QAction *action)
{
    if (!action)
        return action;

    QString index_name = name;

    if (index_name.isEmpty())
        index_name = action->objectName();
    else
        action->setObjectName(index_name);

    if( index_name.isEmpty() )
        index_name = index_name.sprintf("unnamed-%p", (void*)action);

    // look if we already have THIS action under THIS name ;)
    if (d->actionByName.value(index_name, 0) == action ) {
        // This is not a multi map!
        Q_ASSERT( d->actionByName.count(index_name)==1);
        return action;
    }

    if (!KAuthorized::authorizeKAction(index_name)) {
      // Disable this action
      action->setEnabled(false);
      action->setVisible(false);
      action->blockSignals(true);
    }

    if (QAction *oldAction = d->actionByName.value(index_name)) {
      takeAction(oldAction);
    }
    takeAction(action);
    // really insert action
    d->actionByName.insert(index_name, action);
    d->nameByAction.insert(action, index_name);

    foreach (QWidget* widget, d->associatedWidgets)
      widget->addAction(action);

    connect(action, SIGNAL(destroyed(QObject*)), SLOT(_k_actionDestroyed(QObject*)));

    if (d->connectHovered)
        connect(action, SIGNAL(hovered()), SLOT(slotActionHovered()));

    if (d->connectTriggered)
        connect(action, SIGNAL(triggered(bool)), SLOT(slotActionTriggered()));

    emit inserted( action );
    return action;
}

void KActionCollection::removeAction( QAction* action )
{
    delete takeAction( action );
}

QAction* KActionCollection::takeAction(QAction *action)
{
  QHash<QAction *, QString>::Iterator it = d->nameByAction.find(action);
  if (it == d->nameByAction.end())
    return 0;
  const QString name = *it;
  d->nameByAction.erase(it);
  d->actionByName.remove(name);

  foreach (QWidget* widget, d->associatedWidgets)
    widget->removeAction(action);

  action->disconnect(this);

  emit removed( action ); //deprecated
  return action;
}

KAction *KActionCollection::addAction(KStandardAction::StandardAction actionType, const QObject *receiver, const char *member)
{
  KAction *action = KStandardAction::create(actionType, receiver, member, this);
  return addAction(action->objectName(), action);
}

KAction *KActionCollection::addAction(KStandardAction::StandardAction actionType, const QString &name,
                                      const QObject *receiver, const char *member)
{
  KAction *action = KStandardAction::create(actionType, receiver, member, this);
  return addAction(name, action);
}

KAction *KActionCollection::addAction(const QString &name, const QObject *receiver, const char *member)
{
  KAction *a = new KAction(this);
  if (receiver && member)
    connect(a, SIGNAL(triggered()), receiver, member);
  return addAction(name, a);
}

QString KActionCollection::configGroup( ) const
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

void KActionCollection::importGlobalShortcuts( KConfigGroup* config )
{
  Q_ASSERT(config);
  if( !config || !config->exists()) {
    return;
  }

  for (QMap<QString, QAction *>::ConstIterator it = d->actionByName.constBegin();
       it != d->actionByName.constEnd(); ++it) {
      KAction *kaction = qobject_cast<KAction*>(it.value());
      if (!kaction)
          continue;

      QString actionName = it.key();

      if( kaction->isShortcutConfigurable() ) {
          QString entry = config->readEntry(actionName, QString());
          if( !entry.isEmpty() ) {
              kaction->setGlobalShortcut( KShortcut(entry), KAction::ActiveShortcut, KAction::NoAutoloading );
          } else {
              kaction->setGlobalShortcut( kaction->shortcut(KAction::DefaultShortcut), KAction::ActiveShortcut, KAction::NoAutoloading );
          }
      }
  }
}


void KActionCollection::readSettings( KConfigGroup* config )
{
  KConfigGroup cg( KGlobal::config(), configGroup() );
  if( !config )
      config = &cg;

  if( !config->exists()) {
    return;
  }

  for (QMap<QString, QAction *>::ConstIterator it = d->actionByName.constBegin();
       it != d->actionByName.constEnd(); ++it) {
      KAction *kaction = qobject_cast<KAction*>(it.value());
      if (!kaction)
          continue;


      if( kaction->isShortcutConfigurable() ) {
          QString actionName = it.key();
          QString entry = config->readEntry(actionName, QString());
          if( !entry.isEmpty() ) {
              kaction->setShortcut( KShortcut(entry), KAction::ActiveShortcut );
          } else {
              kaction->setShortcut( kaction->shortcut(KAction::DefaultShortcut) );
          }
      }
  }

  //kDebug(125) << " done";
}

void KActionCollection::exportGlobalShortcuts( KConfigGroup* config, bool writeAll ) const
{
  Q_ASSERT(config);
  if (!config) {
    return;
  }

  QList<QAction*> writeActions = actions();

  for (QMap<QString, QAction *>::ConstIterator it = d->actionByName.constBegin();
       it != d->actionByName.constEnd(); ++it) {

      KAction *kaction = qobject_cast<KAction*>(it.value());
      if (!kaction)
          continue;
      QString actionName = it.key();

      if( kaction->isShortcutConfigurable() && kaction->isGlobalShortcutEnabled() ) {
          bool bConfigHasAction = !config->readEntry( actionName, QString() ).isEmpty();
          bool bSameAsDefault = (kaction->globalShortcut() == kaction->globalShortcut(KAction::DefaultShortcut));
          // If we're using a global config or this setting
          //  differs from the default, then we want to write.
          KConfigGroup::WriteConfigFlags flags = KConfigGroup::Persistent;
          if (configIsGlobal())
              flags |= KConfigGroup::Global;
          if( writeAll || !bSameAsDefault ) {
              QString s = kaction->globalShortcut().toString();
              if( s.isEmpty() )
                  s = "none";
              kDebug(125) << "\twriting " << actionName << " = " << s;
              config->writeEntry( actionName, s, flags );
          }
          // Otherwise, this key is the same as default
          //  but exists in config file.  Remove it.
          else if( bConfigHasAction ) {
              kDebug(125) << "\tremoving " << actionName << " because == default";
              config->deleteEntry( actionName, flags );
          }
      }
  }

  config->sync();
}


bool KActionCollectionPrivate::writeKXMLGUIConfigFile()
{
    const KXMLGUIClient *kxmlguiClient = q->parentGUIClient();
    // return false if there is no KXMLGUIClient
    if (!kxmlguiClient || kxmlguiClient->xmlFile().isEmpty()) {
        return false;
    }

    kDebug(129) << "xmlFile=" << kxmlguiClient->xmlFile();

    QString attrShortcut = QLatin1String("shortcut");

    // Read XML file
    QString sXml(KXMLGUIFactory::readConfigFile(kxmlguiClient->xmlFile(), q->componentData()));
    QDomDocument doc;
    doc.setContent( sXml );

    // Process XML data

    // Get hold of ActionProperties tag
    QDomElement elem = KXMLGUIFactory::actionPropertiesElement( doc );

    // now, iterate through our actions
    for (QMap<QString, QAction *>::ConstIterator it = actionByName.constBegin();
         it != actionByName.constEnd(); ++it) {
      KAction *kaction = qobject_cast<KAction*>(it.value());
      if (!kaction) {
        continue;
      }

      QString actionName = it.key();

      bool bSameAsDefault = (kaction->shortcut() == kaction->shortcut(KAction::DefaultShortcut));
      kDebug(129) << "name = " << actionName 
                  << " shortcut = " << kaction->shortcut(KAction::ActiveShortcut).toString()
                  << " globalshortcut = " << kaction->globalShortcut(KAction::ActiveShortcut).toString()
                  << " def = " << kaction->shortcut(KAction::DefaultShortcut).toString();

      // now see if this element already exists
      // and create it if necessary (unless bSameAsDefault)
      QDomElement act_elem = KXMLGUIFactory::findActionByName( elem, actionName, !bSameAsDefault );
      if ( act_elem.isNull() )
        continue;

      if( bSameAsDefault ) {
        act_elem.removeAttribute( attrShortcut );
        //kDebug(129) << "act_elem.attributes().count() = " << act_elem.attributes().count();
        if( act_elem.attributes().count() == 1 )
          elem.removeChild( act_elem );
      } else {
        act_elem.setAttribute( attrShortcut, kaction->shortcut().toString() );
      }
    }

    // Write back to XML file
    KXMLGUIFactory::saveConfigFile(doc, kxmlguiClient->xmlFile(), q->componentData());
    return true;
}


void KActionCollection::writeSettings( KConfigGroup* config, bool writeAll, QAction* oneAction ) const
{
    // If the caller didn't provide a config group we try to save the KXMLGUI
    // Configuration file. If that succeeds we are finished.
    if (config==0 && d->writeKXMLGUIConfigFile() ) {
        return;
    }

    KConfigGroup cg(KGlobal::config() , configGroup() );
    if (!config) {
        config = &cg;
    }

    QList<QAction*> writeActions;
    if (oneAction) {
        writeActions.append(oneAction);
    } else {
        writeActions = actions();
    }


    for (QMap<QString, QAction *>::ConstIterator it = d->actionByName.constBegin();
         it != d->actionByName.constEnd(); ++it) {

        // Get the action. We only handle KActions so skip QActions
        KAction *kaction = qobject_cast<KAction*>(it.value());
        if (!kaction) {
            continue;
        }

        // Write the shortcut
        if( kaction->isShortcutConfigurable() ) {
            QString actionName = it.key();
            bool bConfigHasAction = !config->readEntry( actionName, QString() ).isEmpty();
            bool bSameAsDefault = (kaction->shortcut() == kaction->shortcut(KAction::DefaultShortcut));
            // If we're using a global config or this setting
            //  differs from the default, then we want to write.
            KConfigGroup::WriteConfigFlags flags = KConfigGroup::Persistent;

            // Honor the configIsGlobal() setting
            if (configIsGlobal()) {
                flags |= KConfigGroup::Global;
            }

            if( writeAll || !bSameAsDefault ) {
                // We are instructed to write all shortcuts or the shortcut is
                // not set to its default value. Write it
                QString s = kaction->shortcut().toString();
                if( s.isEmpty() )
                    s = "none";
                kDebug(125) << "\twriting " << actionName << " = " << s;
                config->writeEntry( actionName, s, flags );

            } else if( bConfigHasAction ) {
                // Otherwise, this key is the same as default but exists in
                // config file. Remove it.
                kDebug(125) << "\tremoving " << actionName << " because == default";
                config->deleteEntry( actionName, flags );
            }
        }
    }

    config->sync();
}

void KActionCollection::slotActionTriggered( )
{
  QAction* action = qobject_cast<QAction*>(sender());
  if (action)
    emit actionTriggered(action);
}

void KActionCollection::slotActionHighlighted( )
{
	slotActionHovered();
}

void KActionCollection::slotActionHovered( )
{
  QAction* action = qobject_cast<QAction*>(sender());
  if (action) {
    emit actionHighlighted(action);
    emit actionHovered(action);
  }
}

void KActionCollectionPrivate::_k_actionDestroyed( QObject *obj )
{
  QAction *action = static_cast<QAction*>(obj);
  QHash<QAction *, QString>::Iterator it = nameByAction.find(action);
  if (it == nameByAction.end())
    return;
  const QString name = *it;
  nameByAction.erase(it);
  actionByName.remove(name);

  //HACK the object we emit is partly destroyed
  emit q->removed(action); //deprecated. remove in KDE5
}

void KActionCollection::connectNotify ( const char * signal )
{
  if (d->connectHovered && d->connectTriggered)
    return;

  if (QMetaObject::normalizedSignature(SIGNAL(actionHighlighted(QAction*))) == signal || 
      QMetaObject::normalizedSignature(SIGNAL(actionHovered(QAction*))) == signal) {
    if (!d->connectHovered) {
      d->connectHovered = true;
      foreach (QAction* action, actions())
        connect(action, SIGNAL(hovered()), SLOT(slotActionHovered()));
    }

  } else if (QMetaObject::normalizedSignature(SIGNAL(actionTriggered(QAction*))) == signal) {
    if (!d->connectTriggered) {
      d->connectTriggered = true;
      foreach (QAction* action, actions())
        connect(action, SIGNAL(triggered(bool)), SLOT(slotActionTriggered()));
    }
  }

  QObject::connectNotify(signal);
}

const QList< KActionCollection * >& KActionCollection::allCollections( )
{
    return KActionCollectionPrivate::s_allCollections;
}

void KActionCollection::associateWidget(QWidget* widget) const
{
    foreach (QAction* action, actions()) {
        if (!widget->actions().contains(action))
            widget->addAction(action);
    }
}

void KActionCollection::addAssociatedWidget(QWidget * widget)
{
  if (!d->associatedWidgets.contains(widget)) {
    widget->addActions(actions());

    d->associatedWidgets.append(widget);
    connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(_k_associatedWidgetDestroyed(QObject*)));
  }
}

void KActionCollection::removeAssociatedWidget(QWidget * widget)
{
  foreach (QAction* action, actions())
    widget->removeAction(action);

  d->associatedWidgets.removeAll(widget);
  disconnect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(_k_associatedWidgetDestroyed(QObject*)));
}

QList< QWidget * > KActionCollection::associatedWidgets() const
{
  return d->associatedWidgets;
}

void KActionCollection::clearAssociatedWidgets()
{
  foreach (QWidget* widget, d->associatedWidgets)
    foreach (QAction* action, actions())
      widget->removeAction(action);

  d->associatedWidgets.clear();
}

void KActionCollectionPrivate::_k_associatedWidgetDestroyed(QObject *obj)
{
  associatedWidgets.removeAll(static_cast<QWidget*>(obj));
}

/* vim: et sw=2 ts=2
 */

#include "kactioncollection.moc"
