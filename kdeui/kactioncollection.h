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

#ifndef kactioncollection_h
#define kactioncollection_h

#include <qobject.h>

#include <kdelibs_export.h>

class KAction;
class KConfigBase;
class KInstance;
class KXMLGUIClient;

class QActionGroup;
class QString;

/**
 * \short A container for a set of KAction objects.
 *
 * KActionCollection acts as the owning QObject for a set of KAction objects.  It
 * allows them to be grouped for organized presentation of configuration to the user,
 * saving + loading of configuration, and optionally for automatic plugging into
 * specified widget(s).
 *
 * Additionally, KActionCollection provides several convenience functions for locating
 * named actions, and actions grouped by QActionGroup.
 */
class KDEUI_EXPORT KActionCollection : public QObject
{
  friend class KAction;
  friend class KXMLGUIClient;

  Q_OBJECT

  Q_PROPERTY( bool enabled READ isEnabled WRITE setEnabled RESET forgetEnabled )

public:
  /**
   * Constructor.  Allows specification of a KInstance other than the default
   * global instance, where needed.
   */
  explicit KActionCollection( QObject *parent, KInstance *instance = 0 );

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
   * Returns whether this action collection as a whole has been disabled via 
   * setEnabled() or not.
   *
   * \note this does not check each action to see if its state has been changed
   *       elsewhere.
   */
  bool isEnabled() const;

  /**
   * Allows you to enable or disable all actions in this action collection.
   * The state is remembered, and returned in isEnabled().
   */
  void setEnabled(bool enable);

  /**
   * Forgets the enabled state of the action collection.  Actions added
   * to the collection will not have their enabled state changed, unless there
   * is a subsequent call to setEnabled().
   */
  void forgetEnabled();

  /**
   * This sets the default shortcut context for new actions created in this
   * collection.  The default is not to assign a default context, and this
   * can be restored by passing -1.
   *
   * If this collection has an associated widget, and there is no default
   * shortcut context specified, all actions' contexts are
   * automatically changed to Qt::WidgetShortcut.
   */
  void setDefaultShortcutContext(Qt::ShortcutContext context);

  /**
   * Retrieves the default shortcut context for new actions created in this
   * collection.  The default is -1, ie. do not assign a default context to
   * added actions.
   *
   * If this collection has an associated widget, and there is no default
   * shortcut context specified, all actions will be changed to
   * Qt::WidgetShortcut.  However, this function will not return Qt::WidgetShortcut
   * when no widget is associated with this action collection and no default has
   * been set.
   */
  Qt::ShortcutContext defaultShortcutContext() const;

  /**
   * Applies the current default context (if there is one set) to all of the
   * actions in this action collection.
   */
  void applyDefaultShortcutContext();

  /**
   * Set an associated widget (clears any others).  Associated widgets automatically have all actions 
   * in the action collection added to themselves.
   *
   * Shortcut context will automatically be set to Qt::WidgetShortcut, if no defaultShortcutContext() has been set.
   *
   * \sa addAssociatedWidget(), removeAssociatedWidget(), clearAssociatedWidgets() and associatedWidgets().
   */
  void setAssociatedWidget(QWidget* widget);

  /**
   * Add an associated widget.  Associated widgets automatically have all actions 
   * in the action collection added to themselves.
   *
   * Shortcut context will automatically be set to Qt::WidgetShortcut, if no defaultShortcutContext() has been set.
   *
   * \sa setAssociatedWidget(), removeAssociatedWidget(), clearAssociatedWidgets() and associatedWidgets().
   */
  void addAssociatedWidget(QWidget* widget);

  /**
   * Remove an associated widget.  Removes all actions in this collection from 
   * the removed associated widget.
   *
   * Shortcut context will not be reverted from Qt::WidgetShortcut, which would have been
   * assigned if no defaultShortcutContext() was set for this action collection.
   *
   * \sa addAssociatedWidget(), setAssociatedWidget(), clearAssociatedWidgets(), and associatedWidgets().
   */
  void removeAssociatedWidget(QWidget* widget);

  /**
   * Clears all associated widgets.  All actions in this collection will be removed
   * from associated widgets.
   *
   * Shortcut contexts for all actions will automatically be reverted from Qt::WidgetShortcut,
   * to the defaultShortcutContext() for this action collection.
   *
   * \sa addAssociatedWidget(), setAssociatedWidget(), removeAssociatedWidget(), and associatedWidgets().
   */
  void clearAssociatedWidgets();

  /**
   * Returns a list of widgets currently associated with this action collection.
   *
   * Associations are created to enable custom widgets to provide keyboard interactivity
   * via KActions without having to use QWidget::grabShortcut().  An example of its use
   * is katepart, which creates actions for each editor command and then sets its view
   * as an associated widget.
   *
   * \sa addAssociatedWidget(), setAssociatedWidget(), removeAssociatedWidget(), and clearAssociatedWidgets().
   */
  const QList<QWidget*>& associatedWidgets() const;

  /**
   * Returns the KConfig group with which settings will be loaded and saved.
   */
  const QString& configGroup() const;

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
    * Read all key associations from @p config, or (if @p config
    * is zero) from the application's configuration file
    * KGlobal::config().
    *
    * The group in which the configuration is stored can be
    * set with setConfigGroup().
    */
  void readSettings( KConfigBase* config = 0 );

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
  void writeSettings( KConfigBase* config = 0, bool writeDefaults = false, KAction* oneAction = 0L ) const;

  /**
   * Doc/View model.  This lets you add the action collection of a document
   * to a view's action collection.
   */
  void addDocCollection( KActionCollection* pDoc );

  /**
   * Returns the number of actions in the collection.
   *
   * \deprecated use actions().count() instead
   */
  KDE_DEPRECATED int count() const;

  /**
   * Returns whether the action collection is empty or not.
   */
  inline bool isEmpty() const { return actions().isEmpty(); }

  /**
   * Return the KAction* at position "index" in the action collection.
   *
   * \deprecated use actions().value(int index) instead
   */
  KDE_DEPRECATED KAction* action( int index ) const;

  /**
   * Find the first action with a given \a name in the action collection.
   *
   * @param name Name of the KAction, or null to match all actions
   * @return A pointer to the first KAction in the collection which matches the parameters or
   * null if nothing matches.
   */
  KAction* action( const QString& name ) const;

  /**
   * Find all actions with a given \a name in the action collection.
   *
   * @param name Name of the KAction, or null to match all actions
   * @return A list of all KActions in the collection which match the parameters
   */
  QList<KAction*> actions( const QString& name ) const;

  /**
   * Find the first action of a given subclass of KAction in the action collection.
   *
   * @param name Name of the KAction, or null to match all actions.
   * @return A pointer to the first KAction in the collection which matches the parameters or
   * null if nothing matches.
   */
  template <class T>
  KAction* actionOfType( const QString& name ) const
  { return actionOfTypeInternal(name, ((T)0)->staticMetaObject); }

  /**
   * Find all actions of a given subclass of KAction in the action collection.
   *
   * @param name Name of the KAction, or null to match all actions.
   * @return A list of all KActions in the collection which match the parameters
   */
  template <class T>
  KAction* actionsOfType( const QString& name ) const
  { return actionsOfTypeInternal(name, ((T)0)->staticMetaObject); }

  /**
   * Returns the list of KActions which belong to this action collection.
   */
  const QList<KAction*>& actions() const;

  /**
   * Returns the list of KActions without an QAction::actionGroup() which belong to this action collection.
   */
  const QList<KAction*> actionsWithoutGroup() const;

  /**
   * Returns the list of all QActionGroups associated with actions in this action collection.
   */
  const QList<QActionGroup*> actionGroups() const;

  /**
   * Returns all KActions which are both within the supplied \a group, and
   * part of this action collection.
   */
  const QList<KAction*> actionsInGroup(QActionGroup* group) const;

  /**
   * Set the \a instance associated with this action collection.
   *
   * \param instance the KInstance which is to be associated with this action collection, or to null to indicate the default KInstance.
   */
  void setInstance( KInstance *instance );

  /** The instance with which this class is associated. */
  KInstance *instance() const;

  /**
   * The parent KXMLGUIClient, or null if not available.
   */
  const KXMLGUIClient *parentGUIClient() const;

Q_SIGNALS:
  /**
   * Indicates that \a action was inserted into this action collection.
   */
  void inserted( KAction* action );

  /**
   * Indicates that \a action was removed from this action collection.
   */
  void removed( KAction* action );

  /**
   * Indicates that \a action was highlighted
   */
  void actionHighlighted(KAction* action);

  /**
   * Indicates that \a action was triggered
   */
  void actionTriggered(KAction* action);

protected:
  /// Overridden to perform connections when someone wants to know whether an action was highlighted or triggered
  virtual void connectNotify ( const char * signal );

  virtual void slotActionTriggered();
  virtual void slotActionHighlighted();

public:
  /**
   * Add an action to the collection.
   * Generally you don't have to call this. The action inserts itself automatically
   * into its parent collection. This can be useful however for a short-lived
   * collection (e.g. for a popupmenu, where the signals from the collection are needed too).
   * (don't forget that in the simple case, a list of actions should be a simple KActionPtrList).
   * If you manually insert actions into a 2nd collection, don't forget to take them out
   * again before destroying the collection.
   * @param action The KAction to add.
   */
  void insert( KAction* action);

  /**
   * Removes an action from the collection and deletes it.
   * Since the KAction destructor removes the action from the collection, you generally
   * don't have to call this.
   * @param action The KAction to remove.
   */
  void remove( KAction* action );

  /**
   * Removes an action from the collection.
   * Since the KAction destructor removes the action from the collection, you generally
   * don't have to call this.
   * @return NULL if not found else returns action.
   * @param action the KAction to remove.
   */
  KAction* take( KAction* action );

private:
  KAction* actionOfTypeInternal( const QString& name, const QMetaObject& mo ) const;
  QList<KAction*> actionsOfTypeInternal( const QString& name, const QMetaObject& mo ) const;

  KActionCollection( const KXMLGUIClient* parent ); // used by KXMLGUIClient

  class KActionCollectionPrivate* const d;

  Q_DISABLE_COPY( KActionCollection )

  static QList<KActionCollection*> s_allCollections;
};

#endif
