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
#include <kauthorized.h>
#include "ktoolbar.h"
#include "kxmlguiclient.h"
#include "kxmlguifactory.h"

#include <kdebug.h>
#include <kglobal.h>
#include <kaction.h>

#include <QtXml/QDomDocument>
#include <QSet>
#include <QtCore/QVariant>
#include <QHash>
#include <QChildEvent>
#include <QTimer>
#include <QAction>

#include <stdio.h>
#include <kcomponentdata.h>
#include <kconfiggroup.h>

class KActionCollectionPrivate
{
public:
  KActionCollectionPrivate()
  {
    q = 0;
    m_parentGUIClient = 0L;

    configIsGlobal = false;

    connectHighlighted = connectTriggered = false;

    configGroup = "Shortcuts";
  }

  static QList<KActionCollection*> s_allCollections;

  void _k_widgetDestroyed(QObject *obj);
  void _k_actionDestroyed(QObject *obj);

  KComponentData m_componentData;

  QMap<QString, QAction*> actionByName;
  QHash<QAction *, QString> nameByAction;

  const KXMLGUIClient *m_parentGUIClient;

  QList<QWidget*> associatedWidgets;

  bool configIsGlobal;
  QString configGroup;

  bool connectTriggered, connectHighlighted;

  KActionCollection *q;
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

    //used by KMenuMenuHandler  and KKeySequenceWidget
    action->setProperty("_k_ActionCollection" , QVariant::fromValue<QObject*>(this) );

    // look if we already have THIS action under THIS name ;)
    QMap<QString, QAction*>::const_iterator it = d->actionByName.find(index_name);
    while (it != d->actionByName.constEnd() && it.key() == index_name)
    {
        if ( it.value() == action )
            return action;

        ++it;
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

    connect(action, SIGNAL(destroyed(QObject*)), SLOT(_k_actionDestroyed(QObject*)));

    if (d->connectHighlighted)
        connect(action, SIGNAL(highlighted()), SLOT(slotActionHighlighted()));

    if (d->connectTriggered)
        connect(action, SIGNAL(triggered(bool)), SLOT(slotActionTriggered()));

    if (d->associatedWidgets.count()) {
        foreach (QWidget* w, d->associatedWidgets)
            w->addAction(action);
    }

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

  disconnect(action, SIGNAL(destroyed(QObject*)), this, SLOT(_k_actionDestroyed(QObject*)));

  if (d->connectHighlighted)
    disconnect(action, SIGNAL(highlighted()), this, SLOT(slotActionHighlighted()));

  if (d->connectTriggered)
    disconnect(action, SIGNAL(triggered(bool)), this, SLOT(slotActionTriggered()));

  if (!d->associatedWidgets.isEmpty())
    foreach (QWidget* w, d->associatedWidgets)
      w->removeAction(action);

  emit removed( action );
  return action;
}

QAction *KActionCollection::addAction(KStandardAction::StandardAction actionType, const QObject *receiver, const char *member)
{
  QAction *action = KStandardAction::create(actionType, receiver, member, this);
  return addAction(action->objectName(), action);
}

QAction *KActionCollection::addAction(KStandardAction::StandardAction actionType, const QString &name,
                                      const QObject *receiver, const char *member)
{
  QAction *action = KStandardAction::create(actionType, receiver, member, this);
  return addAction(name, action);
}

QAction *KActionCollection::addAction(const QString &name, const QObject *receiver, const char *member)
{
  QAction *a = new KAction(this);
  if (receiver && member)
    connect(a, SIGNAL(triggered()), receiver, member);
  return addAction(name, a);
}

void KActionCollection::setAssociatedWidget(QWidget* widget)
{
  clearAssociatedWidgets();
  addAssociatedWidget(widget);
}

void KActionCollection::addAssociatedWidget(QWidget* widget)
{
  d->associatedWidgets.append(widget);
  connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(_k_widgetDestroyed(QObject*)));

  foreach (QAction* action, actions()) {
    action->setShortcutContext(Qt::WidgetShortcut);
    widget->addAction(action);
  }
}

void KActionCollection::removeAssociatedWidget(QWidget* widget)
{
  d->associatedWidgets.removeAll(widget);
  disconnect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(_k_widgetDestroyed(QObject*)));

  foreach (QAction* action, actions())
    widget->removeAction(action);
}

void KActionCollection::clearAssociatedWidgets()
{
  while (d->associatedWidgets.count())
    removeAssociatedWidget(d->associatedWidgets.first());
}

QList<QWidget*> KActionCollection::associatedWidgets() const
{
  return d->associatedWidgets;
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

void KActionCollection::readSettings( KConfigGroup* config )
{
  kDebug(125) << " ( \"" << configGroup() << "\", " << config << " ) start";
  KConfigGroup cg( KGlobal::config(), configGroup() );
  if( !config )
      config = &cg;

  kDebug(125) << "\treadSettings( \"" << configGroup() << "\", " << config << " )";
  if( !config->exists())
    return;

  for (QMap<QString, QAction *>::ConstIterator it = d->actionByName.constBegin();
       it != d->actionByName.constEnd(); ++it) {
      KAction *kaction = qobject_cast<KAction*>(it.value());
      if (!kaction)
          continue;

      QString actionName = it.key();

      if( kaction->isShortcutConfigurable() ) {
          QString entry = config->readEntry(actionName, QString());
          if( !entry.isEmpty() ) {
              if( entry == "none" )
                  kaction->setShortcut( KShortcut(), KAction::ActiveShortcut );
              else
                  kaction->setShortcut( KShortcut(entry), KAction::ActiveShortcut );
          }
          else // default shortcut
              kaction->setShortcut( kaction->shortcut(KAction::DefaultShortcut) );

          kDebug(125) << "\t" << actionName << " = '" << entry << "'";
      }
  }

  kDebug(125) << " done";
}

void KActionCollection::writeSettings( KConfigGroup* config, bool writeAll, QAction* oneAction ) const
{
  kDebug(125) << configGroup() << ", " << config << ", " << writeAll << ", " << configIsGlobal() << " )";

  if (parentGUIClient() && !parentGUIClient()->xmlFile().isEmpty()) {
    kDebug(129) << "KActionCollection::save(): xmlFile = " << parentGUIClient()->xmlFile();

    QString attrShortcut  = QLatin1String("shortcut");

    // Read XML file
    QString sXml(KXMLGUIFactory::readConfigFile(parentGUIClient()->xmlFile(), false, componentData()));
    QDomDocument doc;
    doc.setContent( sXml );

    // Process XML data

    // Get hold of ActionProperties tag
    QDomElement elem = KXMLGUIFactory::actionPropertiesElement( doc );

    // now, iterate through our actions
    for (QMap<QString, QAction *>::ConstIterator it = d->actionByName.constBegin();
         it != d->actionByName.constEnd(); ++it) {
      KAction *kaction = qobject_cast<KAction*>(it.value());
      if (!kaction)
        continue;

      QString actionName = it.key();

      bool bSameAsDefault = (kaction->shortcut(KAction::ActiveShortcut) == kaction->shortcut(KAction::DefaultShortcut));
      //kDebug(129) << "name = " << sName << " shortcut = " << shortcut(i).toStringInternal() << " def = " << shortcutDefault(i).toStringInternal();

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
    KXMLGUIFactory::saveConfigFile(doc, parentGUIClient()->xmlFile(), componentData());
    return;
  }

  KConfigGroup cg(KGlobal::config() , configGroup() );
  if (!config)
      config = &cg;

  QList<QAction*> writeActions;
  if (oneAction)
    writeActions.append(oneAction);
  else
    writeActions = actions();

  for (QMap<QString, QAction *>::ConstIterator it = d->actionByName.constBegin();
       it != d->actionByName.constEnd(); ++it) {
      KAction *kaction = qobject_cast<KAction*>(it.value());
      if (!kaction)
          continue;

      QString actionName = it.key();

      if( kaction->isShortcutConfigurable() ) {
          bool bConfigHasAction = !config->readEntry( actionName, QString() ).isEmpty();
          bool bSameAsDefault = (kaction->shortcut(KAction::ActiveShortcut) == kaction->shortcut(KAction::DefaultShortcut));
          // If we're using a global config or this setting
          //  differs from the default, then we want to write.
          KConfigGroup::WriteConfigFlags flags = KConfigGroup::Persistent;
          if (configIsGlobal())
              flags |= KConfigGroup::Global;
          if( writeAll || !bSameAsDefault ) {
              QString s = kaction->shortcut().toString();
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

void KActionCollection::slotActionTriggered( )
{
  QAction* action = qobject_cast<QAction*>(sender());
  if (action)
    emit actionTriggered(action);
}

void KActionCollection::slotActionHighlighted( )
{
  QAction* action = qobject_cast<QAction*>(sender());
  if (action)
    emit actionHighlighted(action);
}

void KActionCollectionPrivate::_k_actionDestroyed( QObject *obj )
{
    QAction *action = static_cast<QAction*>(obj);
    q->takeAction(action);
}

void KActionCollectionPrivate::_k_widgetDestroyed( QObject *obj )
{
    QWidget *widget = static_cast<QWidget*>(obj);
    associatedWidgets.removeAll(widget);
}

void KActionCollection::connectNotify ( const char * signal )
{
  if (d->connectHighlighted && d->connectTriggered)
    return;

  if (QMetaObject::normalizedSignature(SIGNAL(actionHighlighted(QAction*))) == signal) {
    if (!d->connectHighlighted) {
      d->connectHighlighted = true;
      foreach (QAction* action, actions())
        connect(action, SIGNAL(highlighted()), SLOT(slotActionHighlighted()));
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

/* vim: et sw=2 ts=2
 */

#include "kactioncollection.moc"
