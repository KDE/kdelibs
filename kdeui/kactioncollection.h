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

class QString;

class QAction;
class KAction;
class KConfigBase;
class KInstance;
class KXMLGUIClient;
class QActionGroup;

// TODO KAction port - remove typedef
typedef QList<KAction *> KActionPtrList;

/**
 * A managed set of KAction objects.
 *
 * If you set the tooltips on KActions and want the tooltip to show in statusbar
 * (recommended) then you will need to connect a couple of the actionclass signals
 * to the toolbar.
 * The easiest way of doing this is in your KMainWindow subclass, where you create
 * a statusbar, do:
 *
 * \code
 * actionCollection()->setHighlightingEnabled(true);
 * connect(actionCollection(), SIGNAL( actionStatusText( const QString & ) ),
 *           statusBar(), SLOT( message( const QString & ) ) );
 * connect(actionCollection(), SIGNAL( clearStatusText() ),
 *           statusBar(), SLOT( clear() ) );
 * \endcode
 *
 * \todo emit support signals
 */
class KDEUI_EXPORT KActionCollection : public QObject
{
  friend class KAction;
  friend class KXMLGUIClient;

  Q_OBJECT

public:
  KActionCollection( QObject *parent, KInstance *instance = 0 );

  /**
   * Destructor.
   */
  virtual ~KActionCollection();

  /**
   * Clears the entire actionCollection, deleting all actions.
   */
  void clear();

  /**
   * This sets the default shortcut context for new actions created in this
   * collection.  The default is Qt::WindowShortcut (as with Qt).
   *
   * If this collection has an associated widget, the default is changed to
   * Qt::WidgetShortcut.
   */
  void setDefaultShortcutContext(Qt::ShortcutContext context);
  Qt::ShortcutContext defaultShortcutContext() const;

  /**
   * Set widget(s) which will automatically have actions that are added to this
   * collection added to them.
   *
   * Shortcut focus will automatically be set to Qt::WidgetShortcut, and will not
   * be reverted if the associated widgets are cleared.
   */
  void setAssociatedWidget(QWidget* widget);
  void addAssociatedWidget(QWidget* widget);
  void removeAssociatedWidget(QWidget* widget);
  void clearAssociatedWidgets();
  const QList<QWidget*>& associatedWidgets() const;

  const QString& configGroup() const;
  bool configIsGlobal() const;

  void setConfigGroup( const QString& group );
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
    * Write the current configurable associations to @p config,
    * or (if @p config is zero) to the application's
    * configuration file.
    */
  void writeSettings( KConfigBase* config = 0, bool writeAll = true ) const;

  /**
   * Doc/View model.  This lets you add the action collection of a document
   * to a view's action collection.
   */
  void addDocCollection( KActionCollection* pDoc );

  /** Returns the number of actions in the collection */
  int count() const;
  bool isEmpty() const { return (count() == 0); }
  /**
   * Return the KAction* at position "index" in the action collection.
   * @see count()
   */
  KAction* action( int index ) const;
  /**
   * Find an action (optionally, of a given subclass of KAction) in the action collection.
   * @param name Name of the KAction.
   * @param classname Name of the KAction subclass.
   * @return A pointer to the first KAction in the collection which matches the parameters or
   * null if nothing matches.
   */
  KAction* action( const char* name, const char* classname = 0 ) const;

  const QList<KAction*> actions() const;

  const QList<KAction*> actionsWithoutGroup() const;

  const QList<QActionGroup*> actionGroups() const;

  /**
   * This function returns all KActions which are both within the supplied \a group, and
   * part of this action collection.
   */
  const QList<KAction*> actionsInGroup(QActionGroup* group) const;

  void setInstance( KInstance *instance );

  /** The instance with which this class is associated. */
  KInstance *instance() const;

  /**
   * The parent KXMLGUIClient, return 0L if not available.
   */
  const KXMLGUIClient *parentGUIClient() const;

  /** Deprecated **/
  KDE_DEPRECATED void setXMLFile( const QString& );
  const KDE_DEPRECATED QString& xmlFile() const;


Q_SIGNALS:
  void inserted( KAction* );
  void removed( KAction* );

protected:
  virtual void childEvent ( QChildEvent * event );

    virtual void virtual_hook( int id, void* data );

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

private slots:
  void processAddedChildren();

private:
    KActionCollection( const KXMLGUIClient* parent ); // used by KXMLGUIClient
    class KActionCollectionPrivate;
    KActionCollectionPrivate* const d;
};

#endif
