/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef kactioncollection_h
#define kactioncollection_h

#include <qobject.h>
#include <q3valuelist.h>

#include <kdelibs_export.h>

class QString;

class KAccel;
class KAction;
class KConfigBase;
class KInstance;
class KXMLGUIClient;

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
 */
class KDEUI_EXPORT KActionCollection : public QObject
{
  friend class KAction;
  friend class KXMLGUIClient;

  Q_OBJECT

public:
  KActionCollection( QWidget *parent, KInstance *instance = 0 );
  /**
   * Use this constructor if you want the collection's actions to restrict
   * their accelerator keys to @p watch rather than the @p parent.  If
   * you don't require shortcuts, you can pass a null to the @p watch parameter.
   */
  KActionCollection( QWidget *watch, QObject* parent, KInstance *instance = 0 );
#ifndef KDE_NO_COMPAT
  KActionCollection( const KActionCollection &copy );
#endif
  virtual ~KActionCollection();

  /**
   * This sets the widget to which the keyboard shortcuts should be attached.
   * You only need to call this if a null pointer was passed in the constructor.
   */
  virtual void setWidget( QWidget *widget );

  /**
   * This indicates whether new actions which are created in this collection
   * should have their keyboard shortcuts automatically connected on
   * construction.  Set to 'false' if you will be loading XML-based settings.
   * This is automatically done by KParts.  The default is 'true'.
   * @see isAutoConnectShortcuts()
   */
  void setAutoConnectShortcuts( bool );

  /**
   * This indicates whether new actions which are created in this collection
   * have their keyboard shortcuts automatically connected on
   * construction.
   * @see setAutoConnectShortcuts()
   */
  bool isAutoConnectShortcuts();

  /**
   * This sets the default shortcut scope for new actions created in this
   * collection.  The default is ScopeUnspecified.  Ideally the default
   * would have been ScopeWidget, but that would cause some backwards
   * compatibility problems.
   */
  //void setDefaultScope( KAction::Scope );

  /**
   * Doc/View model.  This lets you add the action collection of a document
   * to a view's action collection.
   */
  bool addDocCollection( KActionCollection* pDoc );

  /** Returns the number of widgets which this collection is associated with. */
  //uint widgetCount() const;

  /**
   * Returns true if the collection has its own KAccel object.  This will be
   * the case if it was constructed with a valid widget ptr or if setWidget()
   * was called.
   */
  //bool ownsKAccel() const;

  /** @deprecated  Deprecated because of ambiguous name.  Use kaccel() */
  virtual KAccel* accel() KDE_DEPRECATED;
  /** @deprecated  Deprecated because of ambiguous name.  Use kaccel() */
  virtual const KAccel* accel() const KDE_DEPRECATED;

  /** Returns the KAccel object of the most recently set widget. */
  KAccel* kaccel();
  /** Returns the KAccel object of the most recently set widget. Const version for convenience. */
  const KAccel* kaccel() const;

  /** @internal, for KAction::kaccelCurrent() */
  KAccel* builderKAccel() const;
  /** Returns the KAccel object associated with widget #. */
  //KAccel* widgetKAccel( uint i );
  //const KAccel* widgetKAccel( uint i ) const;

  /** Returns the number of actions in the collection */
  virtual uint count() const;
  bool isEmpty() const { return (count() == 0); }
  /**
   * Return the KAction* at position "index" in the action collection.
   * @see count()
   */
  virtual KAction* action( int index ) const;
  /**
   * Find an action (optionally, of a given subclass of KAction) in the action collection.
   * @param name Name of the KAction.
   * @param classname Name of the KAction subclass.
   * @return A pointer to the first KAction in the collection which matches the parameters or
   * null if nothing matches.
   */
  virtual KAction* action( const char* name, const char* classname = 0 ) const;

  /** Returns a list of all the groups of all the KActions in this action collection.
   * @see KAction::group()
   * @see KAction::setGroup()
   */
  virtual QStringList groups() const;
  /**
   * Returns the list of actions in a particular group managed by this action collection.
   * @param group The name of the group.
   */
  virtual KActionPtrList actions( const QString& group ) const;
  /** Returns the list of actions managed by this action collection. */
  virtual KActionPtrList actions() const;

  /**
   * Used for reading shortcut configuration from a non-XML rc file.
   */
  bool readShortcutSettings( const QString& sConfigGroup = QString::null, KConfigBase* pConfig = 0 );
  /**
   * Used for writing shortcut configuration to a non-XML rc file.
   */
  bool writeShortcutSettings( const QString& sConfigGroup = QString::null, KConfigBase* pConfig = 0 ) const;

  void setInstance( KInstance *instance );
  /** The instance with which this class is associated. */
  KInstance *instance() const;

  /**
   * @deprecated
   */
  void setXMLFile( const QString& );
  /**
   * @deprecated
   */
  const QString& xmlFile() const;

  /**
   * Enable highlighting notification for specific KActions.
   * This is true by default, so, by default, the highlighting
   * signals will be emitted.
   *
   * @see connectHighlight()
   * @see disconnectHighlight()
   * @see actionHighlighted()
   * @see actionHighlighted()
   * @see highlightingEnabled()
   */
  void setHighlightingEnabled( bool enable);
  /**
   * Return whether highlighting notifications are enabled.
   * @see connectHighlight()
   * @see disconnectHighlight()
   * @see actionHighlighted()
   * @see setHighlightingEnabled()
   * @see actionHighlighted()
   */
  bool highlightingEnabled() const;

  /**
   * Call this function if you want to receive a signal whenever a KAction is highlighted in a menu or a toolbar.
   * This is only needed if you do not add this action to this container.
   * You will generally not need to call this function.
   *
   * @param container A container in which the KAction is plugged (must inherit QPopupMenu or KToolBar)
   * @param action The action you are interested in
   * @see disconnectHighlight()
   * @see actionHighlighted()
   * @see setHighlightingEnabled()
   * @see highlightingEnabled()
   * @see actionHighlighted()
   */
  void connectHighlight( QWidget *container, KAction *action );
  /**
   * Disconnect highlight notifications for a particular pair of contianer and action.
   * This is only needed if you do not add this action to this container.
   * You will generally not need to call this function.
   *
   * @param container A container in which the KAction is plugged (must inherit QPopupMenu or KToolBar)
   * @param action The action you are interested in
   * @see connectHighlight()
   * @see actionHighlighted()
   * @see setHighlightingEnabled()
   * @see highlightingEnabled()
   * @see actionHighlighted()
   */
  void disconnectHighlight( QWidget *container, KAction *action );

  /**
   * The parent KXMLGUIClient, return 0L if not available.
   */
  const KXMLGUIClient *parentGUIClient() const;

signals:
  void inserted( KAction* );
  void removed( KAction* );

  /** Emitted when @p action is highlighted.
   *  This is only emitted if you have setHighlightingEnabled()
   * @see connectHighlight()
   * @see disconnectHighlight()
   * @see actionHighlighted()
   * @see setHighlightingEnabled()
   * @see highlightingEnabled()
   */
  void actionHighlighted( KAction *action );
  /** Emitted when @p action is highlighed or loses highlighting.
   *  This is only emitted if you have setHighlightingEnabled()
   * @see connectHighlight()
   * @see disconnectHighlight()
   * @see actionHighlighted()
   * @see setHighlightingEnabled()
   * @see highlightingEnabled()
   */
  void actionHighlighted( KAction *action, bool highlight );
  /** Emitted when an action is highlighted, with text
   *  being the tooltip for the action.
   *  This is only emitted if you have setHighlightingEnabled()
   *
   *  This is useful to connect to KStatusBar::message().  See
   *  this class overview for more information.
   *
   * @see setHighlightingEnabled()
   */
  void actionStatusText( const QString &text );
  /** Emitted when an action loses highlighting.
   *  This is only emitted if you have setHighlightingEnabled()
   *
   * @see setHighlightingEnabled()
   */
  void clearStatusText();

private:
  /**
   * @internal Only to be called by KXMLGUIFactory::addClient().
   * When actions are being connected, KAction needs to know what
   * widget it should connect widget-scope actions to, and what
   * main window it should connect
   */
  void beginXMLPlug( QWidget *widget );
  void endXMLPlug();
  /** @internal.  Only to be called by KXMLGUIFactory::removeClient() */
  void prepareXMLUnplug();
  void unplugShortcuts( KAccel* kaccel );

  void _clear();
  void _insert( KAction* );
  void _remove( KAction* );
  KAction* _take( KAction* );

private slots:
   void slotMenuItemHighlighted( int id );
   void slotToolBarButtonHighlighted( int id, bool highlight );
   void slotMenuAboutToHide();
   void slotDestroyed();

private:
   KAction *findAction( QWidget *container, int id );

#ifndef KDE_NO_COMPAT
public:
  KActionCollection( QObject *parent, KInstance *instance = 0 );
#endif

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

#ifndef KDE_NO_COMPAT
  KActionCollection operator+ ( const KActionCollection& ) const;
  KActionCollection& operator= ( const KActionCollection& );
  KActionCollection& operator+= ( const KActionCollection& );
#endif // !KDE_NO_COMPAT

  // KDE4: clear() doesn't need to be a slot
public slots:
  /**
   * Clears the entire actionCollection, deleting all actions.
   * @see remove
   */
  void clear();

protected:
    virtual void virtual_hook( int id, void* data );
private:
    KActionCollection( const char* name, const KXMLGUIClient* parent );
    class KActionCollectionPrivate;
    KActionCollectionPrivate *d;
};

#endif
