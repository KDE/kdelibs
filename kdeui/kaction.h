/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

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

/**
 * The KAction class (and derived and super classes) provide a way to
 * easily encapsulate a "real" user-selected action or event in your
 * program.  For instance, a user may want to "paste" the contents of
 * the clipboard or "scroll down" a document or "quit" the
 * application.  These are all <em>actions</em> -- events that the
 * user causes to happen.  The KAction class allows the developer to
 * deal with this actions in an easy and intuitive manner.
 *
 * Specifically, the KAction class encapsulated the various attributes
 * to an event/action.  For instance, an action might have an icon
 * that goes along with it (a clipboard for a "paste" action or
 * scissors for a "cut" action).  The action might have some text to
 * describe the action.  It will certainly have a method or function
 * that actually <em>executes</em> the action!  All these attributes
 * are contained within the KAction object.
 *
 * The advantage of dealing with Actions is that you can manipulate
 * the Action without regard to the GUI representation of it.  For
 * instance, in the "normal" way of dealing with actions like "cut",
 * you would manually insert a item for Cut into a menu and a button
 * into a toolbar.  If you want to disable the cut action for a moment
 * (maybe nothing is selected), you woud have to hunt down the pointer
 * to the menu item and the toolbar button and disable both
 * individually.  Setting up the menu and toolbar items requires very
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
 * one-to-one relationship between the "real" action and <em>all</em>
 * GUI representations of it.
 *
 * General Usage:
 * The steps to using Actions are roughly as follows
 * 
 * 1) Decide which attributes you want to associate with a given
 *    action (icons, text, keyboard accelerator, etc)
 * 2) Create the action using KAction (or derived or super class)
 * 3) "Plug" the Action into whatever GUI element you want.  This is
 *    typically a menu or toolbar
 *
 * Detailed Example:
 * Here is an example of enabling a "New [document]" action
 * <PRE>
 * KAction *newAct = KAction(i18n("&New"), QIconSet(BarIcon("filenew")),
                             KStdAccel::openNew(), this, SLOT(fileNew()),
                             this);
 * </PRE>
 * This line creates our Action.  It says that wherever this action is
 * displayed, it will use "&New" as the text, the standard icon, and
 * the standard accelerator.  It further says that whenver this action
 * is invoked, it will use the <TT>fileNew()</TT> slot to execute it.
 *
 * <PRE>
 * QPopupMenu *file = new QPopupMenu;
 * newAct->plug(file);
 * </PRE>
 * That just inserted the Action into the File menu.  You can totally
 * forget about that!  In the future, all manipulation of the item is
 * done through the <TT>newAct</TT> object.
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
     * Constructs an action with text and potential keyboard
     * accelerator.. but nothing else.  Use this only if you really
     * know what you are doing.
     *
     * @param text The text that will be displayed
     * @param accel The corresponding keyboard accelerator (shortcut)
     * @param parent This action's parent
     * @param name An internal name for this action
     */
    KAction( const QString& text, int accel = 0, QObject* parent = 0, const char* name = 0 );

    /**
     * Constructs an action with text, potential keyboard
     * accelerator, and a SLOT to call when this action is invoked by
     * the user.  If you do not want or have a keyboard accelerator,
     * set the <TT>accel</TT> param to 0.
     *
     * This is the most common KAction used when you do not have a
     * corresponding icon.
     *
     * @param text The text that will be displayed
     * @param accel The corresponding keyboard accelerator (shortcut).
     * @param receiver The SLOT's parent
     * @param slot The SLOT to invoke to execute this action
     * @param parent This action's parent
     * @param name An internal name for this action
     */
    KAction( const QString& text, int accel,
	     const QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );

    /**
     * Constructs an action with text, icon, and a potential keyboard
     * accelerator.  This Action cannot execute any command.  Use this
     * only if you really know what you are doing.
     *
     * @param text The text that will be displayed
     * @param pix The icons that go with this action
     * @param accel The corresponding keyboard accelerator (shortcut).
     * @param parent This action's parent
     * @param name An internal name for this action
     */
    KAction( const QString& text, const QIconSet& pix, int accel = 0,
	     QObject* parent = 0, const char* name = 0 );

    /**
     * Constructs an action with text, icon, potential keyboard
     * accelerator, and a SLOT to call when this action is invoked by
     * the user.  If you do not want or have a keyboard accelerator,
     * set the <TT>accel</TT> param to 0.
     *
     * This is the other common KAction used.  Use it when you
     * <em>do</em> have a corresponding icon.
     *
     * @param text The text that will be displayed
     * @param pix The icon to display
     * @param accel The corresponding keyboard accelerator (shortcut).
     * @param receiver The SLOT's parent
     * @param slot The SLOT to invoke to execute this action
     * @param parent This action's parent
     * @param name An internal name for this action
     */
    KAction( const QString& text, const QIconSet& pix, int accel,
	     const QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );

    /**
     * Construct a null action.
     *
     * @param parent This action's parent
     * @param name An internal name for this action
     */
    KAction( QObject* parent = 0, const char* name = 0 );

    /**
     * "Plug" or insert this action into a given widget.  This will
     * typically be a menu or a toolbar.  From this point on, you will
     * never need to directly manipulate the item in the menu or
     * toolbar -- you do all enabling/disabling/manipulation directly
     * with your KAction object.
     *
     * @param w The GUI element to display this action
     */
    virtual int plug( QWidget *w, int index = -1 );

    /**
     * "Unplug" or remove this action from a given widget.  This will
     * typically be a menu or a toolbar.  This is rarely used in
     * "normal" application.  Typically, it would be used if your
     * application has several views or modes -- each with a
     * completely different menu structure.  If you simply want to
     * disable an action for a given period, use @ref setEnabled
     * instead.
     *
     * @param w Remove the action from this GUI element
     */
    virtual void unplug( QWidget *w );

    /**
     * This allows you to enable or disable <em>all</em> instances of
     * this action in all GUI elements.
     *
     * @param b true to enable, false to disable
     */
    virtual void setEnabled( bool b );

    /**
     * Set the text for this action
     *
     * @param text The text
     */
    virtual void setText( const QString& text );

    /**
     * Associate some icons with this action
     *
     * @param iconSet The icons
     */
    virtual void setIconSet( const QIconSet& iconSet );

};

class KToggleAction : public QToggleAction
{
    Q_OBJECT
public:
    KToggleAction( const QString& text, int accel = 0, QObject* parent = 0, const char* name = 0 );
    KToggleAction( const QString& text, int accel,
		   const QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
    KToggleAction( const QString& text, const QIconSet& pix, int accel = 0,
	     QObject* parent = 0, const char* name = 0 );
    KToggleAction( const QString& text, const QIconSet& pix, int accel,
		   const QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
    KToggleAction( QObject* parent = 0, const char* name = 0 );

    int plug( QWidget*, int index = -1 );

    virtual void setChecked( bool );
    bool isChecked() const;

protected slots:
    void slotActivated();

protected:
    bool locked, locked2;
    bool checked;

};

class KSelectAction : public QSelectAction
{
    Q_OBJECT

public:
    KSelectAction( const QString& text, int accel = 0, QObject* parent = 0, const char* name = 0 );
    KSelectAction( const QString& text, int accel,
		   const QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
    KSelectAction( const QString& text, const QIconSet& pix, int accel = 0,
	     QObject* parent = 0, const char* name = 0 );
    KSelectAction( const QString& text, const QIconSet& pix, int accel,
		   const QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
    KSelectAction( QObject* parent = 0, const char* name = 0 );

    int plug( QWidget*, int index = -1 );

    virtual void setCurrentItem( int i );	
    void setItems( const QStringList& lst );
    void clear();

protected slots:
    void slotActivated( const QString &text );

signals:
    void activate();

private:
    bool m_lock;
};

class KFontAction : public KSelectAction
{
    Q_OBJECT

public:
    KFontAction( const QString& text, int accel = 0, QObject* parent = 0, const char* name = 0 );
    KFontAction( const QString& text, int accel,
		 const QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
    KFontAction( const QString& text, const QIconSet& pix, int accel = 0,
		 QObject* parent = 0, const char* name = 0 );
    KFontAction( const QString& text, const QIconSet& pix, int accel,
		 const QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
    KFontAction( QObject* parent = 0, const char* name = 0 );

    void setFont( const QString &family );
    QString font() {
	return currentText();
    }

    int plug( QWidget*, int index = -1 );

private:
    QStringList fonts;

};

class KFontSizeAction : public KSelectAction
{
    Q_OBJECT

public:
    KFontSizeAction( const QString& text, int accel = 0, QObject* parent = 0, const char* name = 0 );
    KFontSizeAction( const QString& text, int accel,
		     const QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
    KFontSizeAction( const QString& text, const QIconSet& pix, int accel = 0,
		     QObject* parent = 0, const char* name = 0 );
    KFontSizeAction( const QString& text, const QIconSet& pix, int accel,
		     const QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
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

};

class KActionMenu : public QActionMenu
{
  Q_OBJECT
public:
    KActionMenu( const QString& text, QObject* parent = 0, const char* name = 0 );
    KActionMenu( const QString& text, const QIconSet& icon, QObject* parent = 0, const char* name = 0 );
    KActionMenu( QObject* parent = 0, const char* name = 0 );

    virtual int plug( QWidget* widget, int index = -1 );
    virtual void unplug( QWidget* widget );

    virtual void setEnabled( bool b );

    virtual void setText( const QString& text );

    virtual void setIconSet( const QIconSet& iconSet );
};

class KActionSeparator : public QActionSeparator
{
    Q_OBJECT
public:
    KActionSeparator( QObject* parent = 0, const char* name = 0 );

    virtual int plug( QWidget*, int index = -1 );
    virtual void unplug( QWidget* );
};


#endif
