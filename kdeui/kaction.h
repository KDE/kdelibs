/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>

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

#ifndef __kaction_h__
#define __kaction_h__ $Id$

#include <qaction.h>
#include <qfontdatabase.h>
#include <kstdaccel.h>

class KAccel;
class KConfig;
class KURL;
class KActionPrivate;
class KToggleActionPrivate;
class KRadioActionPrivate;
class KSelectActionPrivate;
class KListActionPrivate;
class KRecentFilesActionPrivate;
class KFontActionPrivate;
class KFontSizeActionPrivate;
class KActionMenuPrivate;
class KActionSeparatorPrivate;

/**
 * The KAction class (and derived and super classes) provide a way to
 * easily encapsulate a "real" user-selected action or event in your
 * program.  For instance, a user may want to "paste" the contents of
 * the clipboard or "scroll down" a document or "quit" the
 * application.  These are all @bf actions -- events that the
 * user causes to happen.  The KAction class allows the developer to
 * deal with this actions in an easy and intuitive manner.
 *
 * Specifically, the KAction class encapsulated the various attributes
 * to an event/action.  For instance, an action might have an icon
 * that goes along with it (a clipboard for a "paste" action or
 * scissors for a "cut" action).  The action might have some text to
 * describe the action.  It will certainly have a method or function
 * that actually @bf executes the action!  All these attributes
 * are contained within the KAction object.
 *
 * The advantage of dealing with Actions is that you can manipulate
 * the Action without regard to the GUI representation of it.  For
 * instance, in the "normal" way of dealing with actions like "cut",
 * you would manually insert a item for Cut into a menu and a button
 * into a toolbar.  If you want to disable the cut action for a moment
 * (maybe nothing is selected), you woud have to hunt down the pointer
 * to the menu item and the toolbar button and disable both
 * individually.  Setting the menu item and toolbar item up uses very
 * similar code.. but has to be done twice!
 *
 * With the Action concept, you simply "plug" the Action into whatever
 * GUI element you want.  The KAction class will then take care of
 * correctly defining the menu item (with icons, accelerators, text,
 * etc) or toolbar button.. or whatever.  From then on, if you
 * manipulate the Action at all, the effect will propogate through all
 * GUI representations of it.  Back to the "cut" example: if you want
 * to disable the Cut Action, you would simply do
 * 'cutAction->setEnabled(false)' and the menuitem and button would
 * instantly be disabled!
 *
 * This is the biggest advantage to the Action concept -- there is a
 * one-to-one relationship between the "real" action and @p all
 * GUI representations of it.
 *
 * @sect General Usage:
 *
 * The steps to using actions are roughly as follows
 *
 * @li Decide which attributes you want to associate with a given
 *     action (icons, text, keyboard accelerator, etc)
 * @li Create the action using KAction (or derived or super class).
 * @li "Plug" the Action into whatever GUI element you want.  This is
 *     typically a menu or toolbar.
 *
 * @sect Detailed Example:
 *
 * Here is an example of enabling a "New [document]" action
 * <PRE>
 * KAction *newAct = new KAction(i18n("&New"), QIconSet(BarIcon("filenew")),
								 KStdAccel::key(KStdAccel::New), this, SLOT(fileNew()),
								 this);
 * </PRE>
 * This line creates our action.  It says that wherever this action is
 * displayed, it will use "&New" as the text, the standard icon, and
 * the standard accelerator.  It further says that whenever this action
 * is invoked, it will use the fileNew() slot to execute it.
 *
 * <PRE>
 * QPopupMenu *file = new QPopupMenu;
 * newAct->plug(file);
 * </PRE>
 * That just inserted the action into the File menu.  You can totally
 * forget about that!  In the future, all manipulation of the item is
 * done through the newAct object.
 *
 * <PRE>
 * newAct->plug(toolBar());
 * </PRE>
 * And this inserted the Action into the main toolbar as a button.
 *
 * That's it!
 *
 * If you want to disable that action sometime later, you can do so
 * with
 * <PRE>
 * newAct->setEnabled(false)
 * </PRE>
 * and both the menuitem in File and the toolbar button will instantly
 * be disabled.
 *
 * Note: if you are using a "standard" action like "new", "paste",
 * "quit", or any other action described in the KDE UI Standards,
 * please use the methods in the @ref KStdAction class rather then
 * defining your own.
 *
 * @see KStdAction
 * @short Class to encapsulate user-driven action or event
 */
class KAction : public QAction
{
  Q_OBJECT
public:
    /**
     * Construct an action with text and potential keyboard
     * accelerator but nothing else.  Use this only if you really
     * know what you are doing.
     *
     * @param text The text that will be displayed.
     * @param accel The corresponding keyboard accelerator (shortcut).
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    KAction( const QString& text, int accel = 0, QObject* parent = 0, const char* name = 0 );

    /**
     * Construct an action with text, potential keyboard
     * accelerator, and a SLOT to call when this action is invoked by
     * the user.
     *
     * If you do not want or have a keyboard accelerator,
     * set the @p accel param to 0.
     *
     * This is the most common KAction used when you do not have a
     * corresponding icon.
     *
     * @param text The text that will be displayed.
     * @param accel The corresponding keyboard accelerator (shortcut).
     * @param receiver The SLOT's parent.
     * @param slot The SLOT to invoke to execute this action.
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    KAction( const QString& text, int accel,
	     const QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );

    /**
     * Construct an action with text, icon, and a potential keyboard
     * accelerator.
     *
     * This Action cannot execute any command.  Use this only if you
     * really know what you are doing.
     *
     * @param text The text that will be displayed.
     * @param pix The icons that go with this action.
     * @param accel The corresponding keyboard accelerator (shortcut).
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    KAction( const QString& text, const QIconSet& pix, int accel = 0,
	     QObject* parent = 0, const char* name = 0 );

    /**
     * Construct an action with text, automatically loaded icon, and a
     * potential keyboard accelerator.
     *
     * This Action cannot execute any command.  Use this only if you
     * really know what you are doing.
     *
     * @param text The text that will be displayed.
     * @param pix The icons that go with this action.
     * @param accel The corresponding keyboard accelerator (shortcut).
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    KAction( const QString& text, const QString& pix, int accel = 0,
	     QObject* parent = 0, const char* name = 0 );

    /**
     * Construct an action with text, icon, potential keyboard
     * accelerator, and a SLOT to call when this action is invoked by
     * the user.
     *
     * If you do not want or have a keyboard accelerator, set the
     * @p accel param to 0.
     *
     * This is the other common KAction used.  Use it when you
     * @bf do have a corresponding icon.
     *
     * @param text The text that will be displayed.
     * @param pix The icon to display.
     * @param accel The corresponding keyboard accelerator (shortcut).
     * @param receiver The SLOT's parent.
     * @param slot The SLOT to invoke to execute this action.
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    KAction( const QString& text, const QIconSet& pix, int accel,
	     const QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );

    /**
     * Construct an action with text, icon, potential keyboard
     * accelerator, and a SLOT to call when this action is invoked by
     * the user.  The icon is loaded on demand later based on where it
     * is plugged in.
     *
     * If you do not want or have a keyboard accelerator, set the
     * @p accel param to 0.
     *
     * This is the other common KAction used.  Use it when you
     * @bf do have a corresponding icon.
     *
     * @param text The text that will be displayed.
     * @param pix The icon to display.
     * @param accel The corresponding keyboard accelerator (shortcut).
     * @param receiver The SLOT's parent.
     * @param slot The SLOT to invoke to execute this action.
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    KAction( const QString& text, const QString& pix, int accel,
	     const QObject* receiver, const char* slot, QObject* parent,
         const char* name = 0 );

    /**
     * Construct a null action.
     *
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    KAction( QObject* parent = 0, const char* name = 0 );

    /**
     * Standard destructor
     */
    virtual ~KAction();

    /**
     * "Plug" or insert this action into a given widget.

     * This will
     * typically be a menu or a toolbar.  From this point on, you will
     * never need to directly manipulate the item in the menu or
     * toolbar.  You do all enabling/disabling/manipulation directly
     * with your KAction object.
     *
     * @param w The GUI element to display this action
     */
    virtual int plug( QWidget *w, int index = -1 );
	
	virtual void plugAccel(KAccel *accel, bool configurable = true);

    /**
     * "Unplug" or remove this action from a given widget.

     * This will
     * typically be a menu or a toolbar.  This is rarely used in
     * "normal" application.  Typically, it would be used if your
     * application has several views or modes, each with a
     * completely different menu structure.  If you simply want to
     * disable an action for a given period, use @ref setEnabled()
     * instead.
     *
     * @param w Remove the action from this GUI element.
     */
    virtual void unplug( QWidget *w );
	
	virtual void unplugAccel();
	
	virtual bool isPlugged() const;

    virtual void setText(const QString &text);
    virtual void setAccel(int a);
    virtual void setEnabled(bool enable);
    virtual void setIconSet( const QIconSet &iconSet );
    virtual void setIcon( const QString& icon );
	
protected slots:
	virtual void slotDestroyed();
	virtual void slotKeycodeChanged();
	
protected:
    virtual void setText(int i, const QString &text);
    virtual void setEnabled(int i, bool enable);
    virtual void setIconSet(int i, const QIconSet &iconSet);

private:
	KAccel  *kaccel;

    KActionPrivate *d;
};

/**
 *  Checkbox like action.
 *
 *  This action provides two states: checked or not.
 *
 *  @short Checkbox like action.
 */
class KToggleAction : public QToggleAction
{
    Q_OBJECT

public:

    /**
     * Construct a toggle action with text and potential keyboard
     * accelerator but nothing else. Use this only if you really
     * know what you are doing.
     *
     * @param text The text that will be displayed.
     * @param accel The corresponding keyboard accelerator (shortcut).
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    KToggleAction( const QString& text, int accel = 0, QObject* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param accel The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KToggleAction( const QString& text, int accel,
		   const QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The icons that go with this action.
     *  @param accel The corresponding keyboard accelerator (shortcut).
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KToggleAction( const QString& text, const QIconSet& pix, int accel = 0,
	     QObject* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The dynamically loaded icon that goes with this action.
     *  @param accel The corresponding keyboard accelerator (shortcut).
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KToggleAction( const QString& text, const QString& pix, int accel = 0,
                   QObject* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The icons that go with this action.
     *  @param accel The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KToggleAction( const QString& text, const QIconSet& pix, int accel,
		   const QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The dynamically loaded icon that goes with this action.
     *  @param accel The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KToggleAction( const QString& text, const QString& pix, int accel,
                   const QObject* receiver, const char* slot,
                   QObject* parent, const char* name = 0 );

    /**
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KToggleAction( QObject* parent = 0, const char* name = 0 );

    /**
     *  "Plug" or insert this action into a given widget.
     *
     *  This will typically be a menu or a toolbar.
     *  From this point on, you will never need to directly
     *  manipulate the item in the menu or toolbar.
     *  You do all enabling/disabling/manipulation directly with your KToggleAction object.
     *
     *  @param widget The GUI element to display this action.
     *  @param index  The index of the item.
     */
    virtual int plug( QWidget*, int index = -1 );

    virtual void unplug( QWidget *w );

    /**
     *  Sets the state of the action.
     */
    virtual void setChecked( bool );

    /**
     *  Retrieves the actual state of the action.
     */
    bool isChecked() const;

    virtual void setText(const QString &text) 
       { QToggleAction::setText(text); }   // nasty compilers...

protected slots:

    virtual void slotActivated();

protected:
    virtual void setChecked( int id, bool checked );
    virtual void setText(int i, const QString &text);


    bool locked, locked2;
    bool checked;

    KToggleActionPrivate *d;
};

class KRadioAction : public KToggleAction
{
  Q_OBJECT
public:
    /**
     * Construct a radio action with text and potential keyboard
     * accelerator but nothing else. Use this only if you really
     * know what you are doing.
     *
     * @param text The text that will be displayed.
     * @param accel The corresponding keyboard accelerator (shortcut).
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    KRadioAction( const QString& text, int accel = 0, QObject* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param accel The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KRadioAction( const QString& text, int accel,
		  const QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The icons that go with this action.
     *  @param accel The corresponding keyboard accelerator (shortcut).
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KRadioAction( const QString& text, const QIconSet& pix, int accel = 0,
	          QObject* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The dynamically loaded icon that goes with this action.
     *  @param accel The corresponding keyboard accelerator (shortcut).
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KRadioAction( const QString& text, const QString& pix, int accel = 0,
                  QObject* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The icons that go with this action.
     *  @param accel The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KRadioAction( const QString& text, const QIconSet& pix, int accel,
		  const QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The dynamically loaded icon that goes with this action.
     *  @param accel The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KRadioAction( const QString& text, const QString& pix, int accel,
                  const QObject* receiver, const char* slot,
                  QObject* parent, const char* name = 0 );

    /**
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KRadioAction( QObject* parent = 0, const char* name = 0 );

protected:
    virtual void slotActivated();

private:
    KRadioActionPrivate *d;
};

/**
 *  Action for selecting one of several items.
 *
 *  This action shows up a submenu with a list of items.
 *  One of them can be checked. If The user clicks on an item
 *  this item will automatically be checked,
 *  the formerly checked item becomes unchecked.
 *  There can be only one item checked at a time.
 *
 *  @short Action for selecting one of several items
 */
class KSelectAction : public QSelectAction
{
    Q_OBJECT

public:

    /**
     * Construct a select action with text and potential keyboard
     * accelerator but nothing else. Use this only if you really
     * know what you are doing.
     *
     * @param text The text that will be displayed.
     * @param accel The corresponding keyboard accelerator (shortcut).
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    KSelectAction( const QString& text, int accel = 0, QObject* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param accel The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KSelectAction( const QString& text, int accel,
		   const QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The icons that go with this action.
     *  @param accel The corresponding keyboard accelerator (shortcut).
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KSelectAction( const QString& text, const QIconSet& pix, int accel = 0,
	     QObject* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The dynamically loaded icon that goes with this action.
     *  @param accel The corresponding keyboard accelerator (shortcut).
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KSelectAction( const QString& text, const QString& pix, int accel = 0,
                   QObject* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The icons that go with this action.
     *  @param accel The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KSelectAction( const QString& text, const QIconSet& pix, int accel,
		   const QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The dynamically loaded icon that goes with this action.
     *  @param accel The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KSelectAction( const QString& text, const QString& pix, int accel,
                   const QObject* receiver, const char* slot,
                   QObject* parent, const char* name = 0 );

    /**
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KSelectAction( QObject* parent = 0, const char* name = 0 );

    /**
     *  "Plug" or insert this action into a given widget.
     *
     *  This will typically be a menu or a toolbar.
     *  From this point on, you will never need to directly
     *  manipulate the item in the menu or toolbar.
     *  You do all enabling/disabling/manipulation directly with your KSelectAction object.
     *
     *  @param widget The GUI element to display this action.
     *  @param index  The index of the item.
     */
    int plug( QWidget* widget, int index = -1 );

    /**
     *  Sets the currently checked item.
     *
     *  @param index Index of the item (remember the first item is zero).
     */
    virtual void setCurrentItem( int index );	

    virtual void setItems( const QStringList &lst );

    virtual void clear();

protected slots:

    void slotActivated( const QString &text );

signals:

    void activate();

protected:
    virtual void setCurrentItem( int id, int index );
    
    virtual void setItems( int id, const QStringList &lst );
    	
    virtual void clear( int id );

private:
    bool m_lock;

    KSelectActionPrivate *d;
};

class KListAction : public KSelectAction
{
    Q_OBJECT
public:
    /**
     * Construct a list action with text and potential keyboard
     * accelerator but nothing else. Use this only if you really
     * know what you are doing.
     *
     * @param text The text that will be displayed.
     * @param accel The corresponding keyboard accelerator (shortcut).
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    KListAction( const QString& text, int accel = 0, QObject* parent = 0,
                  const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param accel The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KListAction( const QString& text, int accel, const QObject* receiver,
                  const char* slot, QObject* parent, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The icons that go with this action.
     *  @param accel The corresponding keyboard accelerator (shortcut).
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KListAction( const QString& text, const QIconSet& pix, int accel = 0,
	              QObject* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The dynamically loaded icon that goes with this action.
     *  @param accel The corresponding keyboard accelerator (shortcut).
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KListAction( const QString& text, const QString& pix, int accel = 0,
	              QObject* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The icons that go with this action.
     *  @param accel The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KListAction( const QString& text, const QIconSet& pix, int accel,
		          const QObject* receiver, const char* slot, QObject* parent,
                  const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The dynamically loaded icon that goes with this action.
     *  @param accel The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KListAction( const QString& text, const QString& pix, int accel,
                 const QObject* receiver, const char* slot, QObject* parent,
                 const char* name = 0 );

    /**
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KListAction( QObject* parent = 0, const char* name = 0 );

    /**
     *  Sets the currently checked item.
     *
     *  @param index Index of the item (remember the first item is zero).
     */
    virtual void setCurrentItem( int index );

    QString currentText();
    int currentItem();

private:
    int m_current;
    KListActionPrivate *d;
};

class KRecentFilesAction : public KListAction
{
  Q_OBJECT

public:
  /**
   *  @param text The text that will be displayed.
   *  @param accel The corresponding keyboard accelerator (shortcut).
   *  @param parent This action's parent.
   *  @param name An internal name for this action.
   *  @param maxItems The maximum number of files to display
   */
  KRecentFilesAction( const QString& text, int accel,
                      QObject* parent, const char* name = 0,
                      unsigned int maxItems = 10 );

  /**
   *  @param text The text that will be displayed.
   *  @param accel The corresponding keyboard accelerator (shortcut).
   *  @param receiver The SLOT's parent.
   *  @param slot The SLOT to invoke to execute this action.
   *  @param parent This action's parent.
   *  @param name An internal name for this action.
   *  @param maxItems The maximum number of files to display
   */
  KRecentFilesAction( const QString& text, int accel,
                      const QObject* receiver, const char* slot,
                      QObject* parent, const char* name = 0,
                      unsigned int maxItems = 10 );

  /**
   *  @param text The text that will be displayed.
   *  @param pix The icons that go with this action.
   *  @param accel The corresponding keyboard accelerator (shortcut).
   *  @param parent This action's parent.
   *  @param name An internal name for this action.
   *  @param maxItems The maximum number of files to display
   */
  KRecentFilesAction( const QString& text, const QIconSet& pix, int accel,
                      QObject* parent, const char* name = 0,
                      unsigned int maxItems = 10 );

  /**
   *  @param text The text that will be displayed.
   *  @param pix The dynamically loaded icon that goes with this action.
   *  @param accel The corresponding keyboard accelerator (shortcut).
   *  @param parent This action's parent.
   *  @param name An internal name for this action.
   *  @param maxItems The maximum number of files to display
   */
  KRecentFilesAction( const QString& text, const QString& pix, int accel,
                      QObject* parent, const char* name = 0,
                      unsigned int maxItems = 10 );

  /**
   *  @param text The text that will be displayed.
   *  @param pix The icons that go with this action.
   *  @param accel The corresponding keyboard accelerator (shortcut).
   *  @param receiver The SLOT's parent.
   *  @param slot The SLOT to invoke to execute this action.
   *  @param parent This action's parent.
   *  @param name An internal name for this action.
   *  @param maxItems The maximum number of files to display
   */
  KRecentFilesAction( const QString& text, const QIconSet& pix, int accel,
                      const QObject* receiver, const char* slot,
                      QObject* parent, const char* name = 0,
                      unsigned int maxItems = 10 );

  /**
   *  @param text The text that will be displayed.
   *  @param pix The dynamically loaded icon that goes with this action.
   *  @param accel The corresponding keyboard accelerator (shortcut).
   *  @param receiver The SLOT's parent.
   *  @param slot The SLOT to invoke to execute this action.
   *  @param parent This action's parent.
   *  @param name An internal name for this action.
   *  @param maxItems The maximum number of files to display
   */
  KRecentFilesAction( const QString& text, const QString& pix, int accel,
                      const QObject* receiver, const char* slot,
                      QObject* parent, const char* name = 0,
                      unsigned int maxItems = 10 );

  /**
   *  @param parent This action's parent.
   *  @param name An internal name for this action.
   *  @param maxItems The maximum number of files to display
   */
  KRecentFilesAction( QObject* parent = 0, const char* name = 0,
                      unsigned int maxItems = 10 );
  virtual ~KRecentFilesAction();

  unsigned int maxItems();
  void setMaxItems( unsigned int );
  
  void addURL( const KURL& );
  void removeURL( const KURL& );
  void clearURLList();

  void loadEntries( KConfig* );
  void saveEntries( KConfig* );

signals:
  void urlSelected( const KURL& );

protected slots:
  void itemSelected( const QString& );

private:
  unsigned int m_maxItems;
  KRecentFilesActionPrivate *d;
};

class KFontAction : public KSelectAction
{
    Q_OBJECT

public:
    KFontAction( const QString& text, int accel = 0, QObject* parent = 0,
                 const char* name = 0 );
    KFontAction( const QString& text, int accel,
                 const QObject* receiver, const char* slot, QObject* parent,
                 const char* name = 0 );
    KFontAction( const QString& text, const QIconSet& pix, int accel = 0,
                 QObject* parent = 0, const char* name = 0 );
    KFontAction( const QString& text, const QString& pix, int accel = 0,
                 QObject* parent = 0, const char* name = 0 );
    KFontAction( const QString& text, const QIconSet& pix, int accel,
                 const QObject* receiver, const char* slot, QObject* parent,
                 const char* name = 0 );
    KFontAction( const QString& text, const QString& pix, int accel,
                 const QObject* receiver, const char* slot, QObject* parent,
                 const char* name = 0 );

    KFontAction( QObject* parent = 0, const char* name = 0 );

    void setFont( const QString &family );
    QString font() {
	return currentText();
    }

    int plug( QWidget*, int index = -1 );

private:
    QStringList fonts;
    KFontActionPrivate *d;
};

class KFontSizeAction : public KSelectAction
{
    Q_OBJECT

public:
    KFontSizeAction( const QString& text, int accel = 0, QObject* parent = 0,
                     const char* name = 0 );
    KFontSizeAction( const QString& text, int accel, const QObject* receiver,
                     const char* slot, QObject* parent, const char* name = 0 );
    KFontSizeAction( const QString& text, const QIconSet& pix, int accel = 0,
                     QObject* parent = 0, const char* name = 0 );
    KFontSizeAction( const QString& text, const QString& pix, int accel = 0,
                     QObject* parent = 0, const char* name = 0 );
    KFontSizeAction( const QString& text, const QIconSet& pix, int accel,
                     const QObject* receiver, const char* slot,
                     QObject* parent, const char* name = 0 );
    KFontSizeAction( const QString& text, const QString& pix, int accel,
                     const QObject* receiver, const char* slot,
                     QObject* parent, const char* name = 0 );
    KFontSizeAction( QObject* parent = 0, const char* name = 0 );

    void setFontSize( int size );
    int fontSize();

protected slots:
    virtual void slotActivated( int );
    virtual void slotActivated( const QString& );

signals:
    void fontSizeChanged( int );

private:
    void init();

    bool m_lock;

    KFontSizeActionPrivate *d;
};

class KActionMenu : public QActionMenu
{
  Q_OBJECT
public:
    KActionMenu( const QString& text, QObject* parent = 0,
                 const char* name = 0 );
    KActionMenu( const QString& text, const QIconSet& icon,
                 QObject* parent = 0, const char* name = 0 );
    KActionMenu( const QString& text, const QString& icon,
                 QObject* parent = 0, const char* name = 0 );
    KActionMenu( QObject* parent = 0, const char* name = 0 );

    virtual int plug( QWidget* widget, int index = -1 );
    virtual void unplug( QWidget* widget );

protected:
    virtual void setEnabled( int id, bool b );

    virtual void setText( int id, const QString& text );

    virtual void setIconSet( int id, const QIconSet& iconSet );

private:
    KActionMenuPrivate *d;
};

class KActionSeparator : public QActionSeparator
{
    Q_OBJECT
public:
    KActionSeparator( QObject* parent = 0, const char* name = 0 );

    virtual int plug( QWidget*, int index = -1 );
    virtual void unplug( QWidget* );

private:
    KActionSeparatorPrivate *d;
};

#endif
