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
#include "kactioncategory.h"
#include <kcoreauthorized.h>
#include "kglobalaccel.h"
#include "kxmlguiclient.h"
#include "kxmlguifactory.h"

#include "kdebug.h"
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include <QtXml/QDomDocument>
#include <QtCore/QSet>
#include <QGuiApplication>
#include <QtCore/QMap>
#include <QtCore/QList>
#include <QAction>
#include <QMetaMethod>

#include <stdio.h>

class KActionCollectionPrivate
{
public:
  KActionCollectionPrivate()
    : m_parentGUIClient(0L),
      configGroup("Shortcuts"),
      configIsGlobal(false),
      connectTriggered(false),
      connectHovered(false),
      q(0)

  {
  }

    void setComponentForAction(QAction *action)
    {
        if (!KGlobalAccel::self()->hasShortcut(action)) {
            action->setProperty("componentName", m_componentName);
            action->setProperty("componentDisplayName", m_componentDisplayName);
        }
    }

  static QList<KActionCollection*> s_allCollections;

  void _k_associatedWidgetDestroyed(QObject *obj);
  void _k_actionDestroyed(QObject *obj);

  bool writeKXMLGUIConfigFile();

  QString m_componentName;
  QString m_componentDisplayName;

  //! Remove a action from our internal bookkeeping. Returns NULL if the
  //! action doesn't belong to us.
  QAction *unlistAction(QAction*);

  QMap<QString, QAction*> actionByName;
  QList<QAction*> actions;

  const KXMLGUIClient *m_parentGUIClient;

  QString configGroup;
  bool configIsGlobal : 1;

  bool connectTriggered : 1;
  bool connectHovered : 1;

  KActionCollection *q;

  QList<QWidget*> associatedWidgets;
};

QList<KActionCollection*> KActionCollectionPrivate::s_allCollections;

KActionCollection::KActionCollection(QObject *parent, const QString &cName)
  : QObject( parent )
  , d(new KActionCollectionPrivate)
{
  d->q = this;
  KActionCollectionPrivate::s_allCollections.append(this);

  setComponentName(cName);
}

KActionCollection::KActionCollection( const KXMLGUIClient *parent )
  : QObject( 0 )
  , d(new KActionCollectionPrivate)
{
  d->q = this;
  KActionCollectionPrivate::s_allCollections.append(this);

  d->m_parentGUIClient=parent;
  d->m_componentName = parent->componentName();
}

KActionCollection::~KActionCollection()
{
  KActionCollectionPrivate::s_allCollections.removeAll(this);

  delete d;
}

void KActionCollection::clear()
{
  d->actionByName.clear();
  qDeleteAll(d->actions);
  d->actions.clear();
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
  return d->actions.count();
}

bool KActionCollection::isEmpty() const
{
  return count() == 0;
}

void KActionCollection::setComponentName(const QString &cName)
{
  if (count() > 0) {
    // Its component name is part of an action's signature in the context of
    // global shortcuts and the semantics of changing an existing action's
    // signature are, as it seems, impossible to get right.
    // As of now this only matters for global shortcuts. We could
    // thus relax the requirement and only refuse to change the component data
    // if we have actions with global shortcuts in this collection.
    qWarning() << "this does not work on a KActionCollection containing actions!";
  }

    if (!cName.isEmpty()) {
        d->m_componentName = cName;
    } else {
        d->m_componentName = QCoreApplication::applicationName();
    }
}

QString KActionCollection::componentName() const
{
    return d->m_componentName;
}

void KActionCollection::setComponentDisplayName(const QString &displayName)
{
    d->m_componentDisplayName = displayName;
}

QString KActionCollection::componentDisplayName() const
{
    if (!d->m_componentDisplayName.isEmpty())
        return d->m_componentDisplayName;
    if (!QGuiApplication::applicationDisplayName().isEmpty())
        return QGuiApplication::applicationDisplayName();
    return QCoreApplication::applicationName();
}

const KXMLGUIClient *KActionCollection::parentGUIClient() const
{
	return d->m_parentGUIClient;
}

QList<QAction*> KActionCollection::actions() const
{
  return d->actions;
}

const QList< QAction* > KActionCollection::actionsWithoutGroup( ) const
{
  QList<QAction*> ret;
  foreach (QAction* action, d->actions)
    if (!action->actionGroup())
      ret.append(action);
  return ret;
}

const QList< QActionGroup * > KActionCollection::actionGroups( ) const
{
  QSet<QActionGroup*> set;
  foreach (QAction* action, d->actions)
    if (action->actionGroup())
      set.insert(action->actionGroup());
  return set.toList();
}

QAction *KActionCollection::addAction(const QString &name, QAction *action)
{
    if (!action)
        return action;

    const QString objectName = action->objectName();
    QString indexName = name;

    if (indexName.isEmpty()) {
        // No name provided. Use the objectName.
        indexName = objectName;

    } else {

        // A name was provided. Check against objectName.
        if ((!objectName.isEmpty()) && (objectName != indexName)) {
            // The user specified a new name and the action already has a
            // different one. The objectName is used for saving shortcut
            // settings to disk. Both for local and global shortcuts.
            qDebug() << "Registering action " << objectName << " under new name " << indexName;
            // If there is a global shortcuts it's a very bad idea.
            if (KGlobalAccel::self()->hasShortcut(action)) {
                // In debug mode assert
                Q_ASSERT(!KGlobalAccel::self()->hasShortcut(action));
                // In release mode keep the old name
                kError() << "Changing action name from " << objectName << " to " << indexName << "\nignored because of active global shortcut.";
                indexName = objectName;
            }
        }

        // Set the new name
        action->setObjectName(indexName);
    }

    // No name provided and the action had no name. Make one up. This will not
    // work when trying to save shortcuts. Both local and global shortcuts.
    if( indexName.isEmpty() ) {
        indexName = indexName.sprintf("unnamed-%p", (void*)action);
        action->setObjectName(indexName);
    }

    // From now on the objectName has to have a value. Else we cannot safely
    // remove actions.
    Q_ASSERT(!action->objectName().isEmpty());

    // look if we already have THIS action under THIS name ;)
    if (d->actionByName.value(indexName, 0) == action ) {
        // This is not a multi map!
        Q_ASSERT( d->actionByName.count(indexName)==1);
        return action;
    }

    if (!KAuthorized::authorizeKAction(indexName)) {
      // Disable this action
      action->setEnabled(false);
      action->setVisible(false);
      action->blockSignals(true);
    }

    // Check if we have another action under this name
    if (QAction *oldAction = d->actionByName.value(indexName)) {
      takeAction(oldAction);
    }

    // Check if we have this action under a different name.
    // Not using takeAction because we don't want to remove it from categories,
    // and because it has the new name already.
    const int oldIndex = d->actions.indexOf(action);
    if (oldIndex != -1) {
        d->actionByName.remove(d->actionByName.key(action));
        d->actions.removeAt(oldIndex);
    }

    // Add action to our lists.
    d->actionByName.insert(indexName, action);
    d->actions.append(action);

    foreach (QWidget* widget, d->associatedWidgets) {
      widget->addAction(action);
    }

    connect(action, SIGNAL(destroyed(QObject*)), SLOT(_k_actionDestroyed(QObject*)));

    d->setComponentForAction(action);

    if (d->connectHovered)
        connect(action, SIGNAL(hovered()), SLOT(slotActionHovered()));

    if (d->connectTriggered)
        connect(action, SIGNAL(triggered(bool)), SLOT(slotActionTriggered()));

    emit inserted( action );
    return action;
}

void KActionCollection::addActions(const QList<QAction *> &actions)
{
    Q_FOREACH (QAction* action, actions)
         addAction(action->objectName(), action);
}

void KActionCollection::removeAction( QAction* action )
{
    delete takeAction( action );
}

QAction* KActionCollection::takeAction(QAction *action)
{
  if (!d->unlistAction(action))
      return NULL;

  // Remove the action from all widgets
  foreach (QWidget* widget, d->associatedWidgets) {
    widget->removeAction(action);
  }

  action->disconnect(this);

  emit removed( action ); //deprecated
  return action;
}

QAction *KActionCollection::addAction(KStandardAction::StandardAction actionType, const QObject *receiver, const char *member)
{
  QAction *action = KStandardAction::create(actionType, receiver, member, this);
  return action;
}

QAction *KActionCollection::addAction(KStandardAction::StandardAction actionType, const QString &name,
                                      const QObject *receiver, const char *member)
{
  // pass 0 as parent, because if the parent is a KActionCollection KStandardAction::create automatically
  // adds the action to it under the default name. We would trigger the
  // warning about renaming the action then.
  QAction *action = KStandardAction::create(actionType, receiver, member, 0);
  // Give it a parent for gc.
  action->setParent(this);
  // Remove the name to get rid of the "rename action" warning above
  action->setObjectName(name);
  // And now add it with the desired name.
  return addAction(name, action);
}

QAction *KActionCollection::addAction(const QString &name, const QObject *receiver, const char *member)
{
  QAction *a = new QAction(this);
  if (receiver && member)
    connect(a, SIGNAL(triggered(bool)), receiver, member);
  return addAction(name, a);
}

QKeySequence KActionCollection::defaultShortcut(QAction *action) const
{
    const QList<QKeySequence> shortcuts = defaultShortcuts(action);
    return shortcuts.isEmpty() ? QKeySequence() : shortcuts.first();
}

QList<QKeySequence> KActionCollection::defaultShortcuts(QAction *action) const
{
      return action->property("defaultShortcuts").value<QList<QKeySequence> >();
}

void KActionCollection::setDefaultShortcut(QAction *action, const QKeySequence &shortcut)
{
    setDefaultShortcuts(action, QList<QKeySequence>() << shortcut);
}

void KActionCollection::setDefaultShortcuts(QAction *action, const QList<QKeySequence>& shortcuts)
{
    action->setProperty("defaultShortcuts", QVariant::fromValue(shortcuts));
}
bool KActionCollection::isShortcutsConfigurable(QAction *action) const
{
    // Considered as true by default
    const QVariant value = action->property("isShortcutConfigurable");
    return value.isValid() ? value.toBool() : true;
}

void KActionCollection::setShortcutsConfigurable(QAction *action, bool configurable)
{
    action->setProperty("isShortcutConfigurable", configurable);
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
      QAction *action = it.value();
      if (!action)
          continue;

      QString actionName = it.key();

      if(isShortcutsConfigurable(action)) {
          QString entry = config->readEntry(actionName, QString());
          if( !entry.isEmpty() ) {
              KGlobalAccel::self()->setShortcut(action, QKeySequence::listFromString(entry), KGlobalAccel::NoAutoloading );
          } else {
              QList<QKeySequence> defaultShortcut = KGlobalAccel::self()->defaultShortcut(action);
              KGlobalAccel::self()->setShortcut(action, defaultShortcut, KGlobalAccel::NoAutoloading );
          }
      }
  }
}


void KActionCollection::readSettings( KConfigGroup* config )
{
  KConfigGroup cg( KSharedConfig::openConfig(), configGroup() );
  if( !config )
      config = &cg;

  if( !config->exists()) {
    return;
  }

  for (QMap<QString, QAction *>::ConstIterator it = d->actionByName.constBegin();
       it != d->actionByName.constEnd(); ++it) {
      QAction *action = it.value();
      if (!action)
          continue;

      if(isShortcutsConfigurable(action)) {
          QString actionName = it.key();
          QString entry = config->readEntry(actionName, QString());
          if( !entry.isEmpty() ) {
              action->setShortcuts(QKeySequence::listFromString(entry));
          } else {
              action->setShortcuts(defaultShortcuts(action));
          }
      }
  }

  //qDebug() << " done";
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

      QAction *action = it.value();
      if (!action)
          continue;
      QString actionName = it.key();

      // If the action name starts with unnamed- spit out a warning. That name
      // will change at will and will break loading writing
      if (actionName.startsWith(QLatin1String("unnamed-"))) {
          kError() << "Skipped exporting Shortcut for action without name " << action->text() << "!";
          continue;
      }

      if(isShortcutsConfigurable(action) && KGlobalAccel::self()->hasShortcut(action)) {
          bool bConfigHasAction = !config->readEntry( actionName, QString() ).isEmpty();
          bool bSameAsDefault = (KGlobalAccel::self()->shortcut(action) == KGlobalAccel::self()->defaultShortcut(action));
          // If we're using a global config or this setting
          //  differs from the default, then we want to write.
          KConfigGroup::WriteConfigFlags flags = KConfigGroup::Persistent;
          if (configIsGlobal())
              flags |= KConfigGroup::Global;
          if( writeAll || !bSameAsDefault ) {
              QString s = QKeySequence::listToString(KGlobalAccel::self()->shortcut(action));
              if( s.isEmpty() )
                  s = "none";
              qDebug() << "\twriting " << actionName << " = " << s;
              config->writeEntry( actionName, s, flags );
          }
          // Otherwise, this key is the same as default
          //  but exists in config file.  Remove it.
          else if( bConfigHasAction ) {
              qDebug() << "\tremoving " << actionName << " because == default";
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

    qDebug() << "xmlFile=" << kxmlguiClient->xmlFile();

    QString attrShortcut = QLatin1String("shortcut");

    // Read XML file
    QString sXml(KXMLGUIFactory::readConfigFile(kxmlguiClient->xmlFile(), q->componentName()));
    QDomDocument doc;
    doc.setContent( sXml );

    // Process XML data

    // Get hold of ActionProperties tag
    QDomElement elem = KXMLGUIFactory::actionPropertiesElement( doc );

    // now, iterate through our actions
    for (QMap<QString, QAction *>::ConstIterator it = actionByName.constBegin();
         it != actionByName.constEnd(); ++it) {
      QAction *action = it.value();
      if (!action) {
        continue;
      }

      QString actionName = it.key();

      // If the action name starts with unnamed- spit out a warning and ignore
      // it. That name will change at will and will break loading writing
      if (actionName.startsWith(QLatin1String("unnamed-"))) {
          kError() << "Skipped writing shortcut for action " << actionName << "(" << action->text() << ")!";
          continue;
      }

      bool bSameAsDefault = (action->shortcuts() == q->defaultShortcuts(action));
      qDebug() << "name = " << actionName
                  << " shortcut = " << QKeySequence::listToString(action->shortcuts())
                  << " globalshortcut = " << QKeySequence::listToString(KGlobalAccel::self()->shortcut(action))
                  << " def = " << QKeySequence::listToString(q->defaultShortcuts(action));

      // now see if this element already exists
      // and create it if necessary (unless bSameAsDefault)
      QDomElement act_elem = KXMLGUIFactory::findActionByName( elem, actionName, !bSameAsDefault );
      if ( act_elem.isNull() )
        continue;

      if( bSameAsDefault ) {
        act_elem.removeAttribute( attrShortcut );
        //qDebug() << "act_elem.attributes().count() = " << act_elem.attributes().count();
        if( act_elem.attributes().count() == 1 )
          elem.removeChild( act_elem );
      } else {
        act_elem.setAttribute( attrShortcut, QKeySequence::listToString(action->shortcuts()) );
      }
    }

    // Write back to XML file
    KXMLGUIFactory::saveConfigFile(doc, kxmlguiClient->localXMLFile(), q->componentName());
    return true;
}


void KActionCollection::writeSettings( KConfigGroup* config, bool writeAll, QAction* oneAction ) const
{
    // If the caller didn't provide a config group we try to save the KXMLGUI
    // Configuration file. If that succeeds we are finished.
    if (config==0 && d->writeKXMLGUIConfigFile() ) {
        return;
    }

    KConfigGroup cg(KSharedConfig::openConfig() , configGroup() );
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

        QAction *action = it.value();
        if (!action) {
            continue;
        }

        QString actionName = it.key();

        // If the action name starts with unnamed- spit out a warning and ignore
        // it. That name will change at will and will break loading writing
        if (actionName.startsWith(QLatin1String("unnamed-"))) {
            kError() << "Skipped saving Shortcut for action without name " << action->text() << "!";
            continue;
        }

        // Write the shortcut
        if(isShortcutsConfigurable(action)) {
            bool bConfigHasAction = !config->readEntry( actionName, QString() ).isEmpty();
            bool bSameAsDefault = (action->shortcuts() == defaultShortcuts(action));
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
                QString s = QKeySequence::listToString(action->shortcuts());
                if( s.isEmpty() )
                    s = "none";
                qDebug() << "\twriting " << actionName << " = " << s;
                config->writeEntry( actionName, s, flags );

            } else if( bConfigHasAction ) {
                // Otherwise, this key is the same as default but exists in
                // config file. Remove it.
                qDebug() << "\tremoving " << actionName << " because == default";
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
  // obj isn't really a QAction anymore. So make sure we don't do fancy stuff
  // with it.
  QAction *action = static_cast<QAction*>(obj);

  if (!unlistAction(action))
      return;

  //HACK the object we emit is partly destroyed
  emit q->removed(action); //deprecated. remove in KDE5
}

void KActionCollection::connectNotify(const QMetaMethod& signal)
{
  if (d->connectHovered && d->connectTriggered)
    return;

  if (signal.methodSignature() == "actionHighlighted(QAction*)" ||
      signal.methodSignature() == "actionHovered(QAction*)") {
    if (!d->connectHovered) {
      d->connectHovered = true;
      foreach (QAction* action, actions())
        connect(action, SIGNAL(hovered()), SLOT(slotActionHovered()));
    }

  } else if (signal.methodSignature() == "actionTriggered(QAction*)") {
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


QAction *KActionCollectionPrivate::unlistAction(QAction* action)
{
  // ATTENTION:
  //   This method is called with an QObject formerly known as a QAction
  //   during _k_actionDestroyed(). So don't do fancy stuff here that needs a
  //   real QAction!

  // Get the index for the action
  int index = actions.indexOf(action);

  // Action not found.
  if (index==-1) return NULL;

  // An action collection can't have the same action twice.
  Q_ASSERT(actions.indexOf(action,index+1)==-1);

  // Get the actions name
  const QString name = action->objectName();

  // Remove the action
  actionByName.remove(name);
  actions.removeAt(index);

  // Remove the action from the categories. Should be only one
  QList<KActionCategory*> categories = q->findChildren<KActionCategory*>();
  foreach (KActionCategory *category, categories) {
      category->unlistAction(action);
  }

  return action;
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

#include "moc_kactioncollection.cpp"
