/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
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

#ifndef KACTIONCOLLECTION_H
#define KACTIONCOLLECTION_H

#include <kdeui_export.h>
#include <kstandardaction.h>
#include <kcomponentdata.h>

#include <QtCore/QObject>

class QAction;
class KXMLGUIClient;

class QActionGroup;
class QString;

/**
 * \short A container for a set of QAction objects.
 *
 * KActionCollection acts as the owning QObject for a set of QAction objects.  It
 * allows them to be grouped for organized presentation of configuration to the user,
 * saving + loading of configuration, and optionally for automatic plugging into
 * specified widget(s).
 *
 * Additionally, KActionCollection provides several convenience functions for locating
 * named actions, and actions grouped by QActionGroup.
 */
class KDEUI_EXPORT KActionCollection : public QObject
{
  friend class KXMLGUIClient;

  Q_OBJECT

  Q_PROPERTY( QString configGroup READ configGroup WRITE setConfigGroup )
  Q_PROPERTY( bool configIsGlobal READ configIsGlobal WRITE setConfigGlobal )

public:
  /**
   * Constructor.  Allows specification of a KComponentData other than the default
   * global KComponentData, where needed.
   */
  explicit KActionCollection(QObject *parent, const KComponentData &cData = KComponentData());

  /**
   * Destructor.
   */
  virtual ~KActionCollection();

  /**
   * Access the list of all action collections in existence for this app
   */
  static const QList<KActionCollection*>& allCollections();

  /**
   * Clears the entire action collection, deleting all actions.
   */
  void clear();

  /**
   * Associate all actions in this collection to the given \a widget, and set all
   * shortcutContexts to Qt::WidgetShortcut.  This has the effect of making the actions
   * only trigger when:
   * 1) the widget (or its children, it Qt 4.4 and above) has focus
   * 2) the shortcut is pressed.
   *
   * This is important particularly when actions only apply to a specific widget, and
   * when the application is a kpart and needs to respect other parts' shortcuts.
   */
  void associateWidget(QWidget* widget) const;

  /**
   * Returns the KConfig group with which settings will be loaded and saved.
   */
  QString configGroup() const;

  /**
   * Returns whether this action collection's configuration should be global to KDE ( \e true ),
   * or specific to the application ( \e false ).
   */
  bool configIsGlobal() const;

  /**
   * Sets \a group as the KConfig group with which settings will be loaded and saved.
   */
  void setConfigGroup( const QString& group );

  /**
   * Set whether this action collection's configuration should be global to KDE ( \e true ),
   * or specific to the application ( \e false ).
   */
  void setConfigGlobal( bool global );

  /**
    * Read all key associations from @p config.
    *
    * If @p config is zero, read all key associations from the
    * application's configuration file KGlobal::config(),
    * in the group set by setConfigGroup().
    */
  void readSettings( KConfigGroup* config = 0 );

  /**
    * Write the current configurable key associations to @p config,
    * or (if @p config is zero) to the application's
    * configuration file.
    *
    * \param config Config object to save to, or null to use the application's config object.
    * \param writeDefaults set to true to write settings which are already at defaults.
    * \param oneAction pass an action here if you just want to save the values for one action, eg.
    *                  if you know that action is the only one which has changed.
    */
  void writeSettings( KConfigGroup* config = 0, bool writeDefaults = false, QAction* oneAction = 0 ) const;

  /**
   * Returns the number of actions in the collection.
   *
   * This is equivalent to actions().count().
   */
  int count() const;

  /**
   * Returns whether the action collection is empty or not.
   */
  bool isEmpty() const;

  /**
   * Return the QAction* at position "index" in the action collection.
   *
   * This is equivalent to actions().value(index);
   */
  QAction *action(int index) const;

  /**
   * Find the first action with a given \a name in the action collection.
   *
   * @param name Name of the KAction, or null to match all actions
   * @return A pointer to the first KAction in the collection which matches the parameters or
   * null if nothing matches.
   */
  QAction* action( const QString& name ) const;

  /**
   * Returns the list of KActions which belong to this action collection.
   */
  QList<QAction*> actions() const;

  /**
   * Returns the list of KActions without an QAction::actionGroup() which belong to this action collection.
   */
  const QList<QAction*> actionsWithoutGroup() const;

  /**
   * Returns the list of all QActionGroups associated with actions in this action collection.
   */
  const QList<QActionGroup*> actionGroups() const;

  /**
   * Set the \a componentData associated with this action collection.
   *
   * \param componentData the KComponentData which is to be associated with this action collection,
   * or an invalid KComponentData instance to indicate the default KComponentData.
   */
  void setComponentData(const KComponentData &componentData);

  /** The KComponentData with which this class is associated. */
  KComponentData componentData() const;

  /**
   * The parent KXMLGUIClient, or null if not available.
   */
  const KXMLGUIClient *parentGUIClient() const;

Q_SIGNALS:
  /**
   * Indicates that \a action was inserted into this action collection.
   */
  void inserted( QAction* action );

  /**
   * Indicates that \a action was removed from this action collection.
   */
  void removed( QAction* action );

  /**
   * Indicates that \a action was highlighted
   */
  void actionHighlighted(QAction* action);

  /**
   * Indicates that \a action was triggered
   */
  void actionTriggered(QAction* action);

protected:
  /// Overridden to perform connections when someone wants to know whether an action was highlighted or triggered
  virtual void connectNotify ( const char * signal );

  virtual void slotActionTriggered();
  virtual void slotActionHighlighted();

public:
  /**
   * Add an action under the given name to the collection.
   *
   * Inserting an action that was previously inserted under a different name will replace the
   * old entry, i.e. the action will not be available under the old name anymore but only under
   * the new one.
   *
   * Inserting an action under a name that is already used for another action will replace
   * the other action in the collection.
   *
   * @param name The name by which the action be retrieved again from the collection.
   * @param action The action to add.
   */
  QAction *addAction(const QString &name, QAction *action);
  KAction *addAction(const QString &name, KAction *action);

  /**
   * Removes an action from the collection and deletes it.
   * @param action The action to remove.
   */
  void removeAction(QAction *action);

  /**
   * Removes an action from the collection.
   * @param action the action to remove.
   */
  QAction* takeAction(QAction *action);

  /**
   * Creates a new standard action, adds it to the collection and connects the action's triggered() signal to the
   * specified receiver/member. The newly created action is also returned.
   *
   * The action can be retrieved later from the collection by its standard name as per
   * KStandardAction::stdName.
   */
  KAction *addAction(KStandardAction::StandardAction actionType, const QObject *receiver = 0, const char *member = 0);
  /**
   * Creates a new standard action, adds to the collection under the given name and connects the action's triggered() signal to the
   * specified receiver/member. The newly created action is also returned.
   *
   * The action can be retrieved later from the collection by the specified name.
   */
  KAction *addAction(KStandardAction::StandardAction actionType, const QString &name,
                     const QObject *receiver = 0, const char *member = 0);

  /**
   * Creates a new action under the given name to the collection and connects the action's triggered()
   * signal to the specified receiver/member. The newly created action is returned.
   *
   * Inserting an action that was previously inserted under a different name will replace the
   * old entry, i.e. the action will not be available under the old name anymore but only under
   * the new one.
   *
   * Inserting an action under a name that is already used for another action will replace
   * the other action in the collection.
   *
   * @param name The name by which the action be retrieved again from the collection.
   * @param action The action to add.
   */
  KAction *addAction(const QString &name, const QObject *receiver = 0, const char *member = 0);

  /**
   * Creates a new action under the given name, adds it to the collection and connects the action's triggered()
   * signal to the specified receiver/member. The type of the action is specified by the template
   * parameter ActionType.
   */
  template<class ActionType>
  ActionType *add(const QString &name, const QObject *receiver = 0, const char *member = 0)
  {
      ActionType *a = new ActionType(this);
      if (receiver && member)
          connect(a, SIGNAL(triggered()), receiver, member);
      addAction(name, a);
      return a;
  }

private:
  Q_PRIVATE_SLOT(d, void _k_actionDestroyed(QObject *))

  KActionCollection( const KXMLGUIClient* parent ); // used by KXMLGUIClient

  class KActionCollectionPrivate* const d;
};

#endif
