/*  This file is part of the KDE libraries

    This class was originally inspired by Torben Weis'
    fileentry.cpp for KFM II.

    Copyright (C) 1997 Sven Radej <sven.radej@iname.com>
    Copyright (c) 1999 Patrick Ward <PAT_WARD@HP-USA-om5.om.hp.com>
    Copyright (c) 1999 Preston Brown <pbrown@kde.org>

    Completely re-designed:
    Copyright (c) 2000 Dawit Alemayehu <adawit@earthlink.net>

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

#ifndef _KLINEEDIT_H
#define _KLINEEDIT_H

#include <qlineedit.h>
#include <qpopupmenu.h>

#include <kcompletion.h>

/**
 * An enhanced QLineEdit widget for inputting text.
 *
 * This widget has the same behaviour as QLineEdit
 * with the following added functionalities : a
 * popup menu that provides basic features such as
 * copy/cut/paste to manipulate content through the
 * mouse, a built-in hook into @ref KCompletion which
 * provides automatic & manual completion as well as
 * iteration through a given list, and the ability to
 * change which keyboard keys to use for these features.
 * Additionally, since this widget inherits form QLineEdit,
 * it can be used as a drop-in replacement where the
 * above extra functionalities are needed and/or useful.
 *
 * KLineEdit emits a few more additional signals than
 * QLineEdit: @ref completion, @ref rotateUp and @ref rotateDown
 * and @returnPressed.  The completion signal can be
 * connected to a slot that will assist the user in
 * filling out the remaining text.  The two rotation
 * signals are intended to be used to iterate through a
 * list of predefined text entries.
 *
 * By default, when you create a completion object through
 * either @ref completionObject() or @ref setCompletionObject
 * this widget will be automatically enabled to handle the
 * signals.  If you do not need this feature, simply use the
 * appropriate accessor methods to turn it off.
 *
 * The default key-bindings for completion and rotation
 * are determined from the global settings in @ref KStdAccel.
 * However, these values can be set locally overriding the
 * global settings.  Simply invoking @ref useGlobalSettings
 * allows you to immediately default the bindings back to the
 * global settings again.  Also if you are interested in only
 * defaulting the key-bindings individually for each action,
 * simply call the setXXXKey methods without any argumet.  For
 * example, after locally customizing the key-binding that invokes
 * manual completion, simply invoking @ref setCompletionKey(),
 * without any argument, will result in the completion key being
 * set to 0. This will then force the key-event filter to use the
 * global value.

 * NOTE: if the EchoMode for this widget is set to something
 * other than @ref QLineEdit::Normal, the completion mode will
 * always be defaulted to KGlobal::CompletionNone.  This is
 * done purposefully to protect against protected entries such
 * as passwords being cached in KCompletion's list.  Hence, if
 * the EchoMode is not QLineEdit::Normal, the completion mode
 * is automatically disabled.
 *
 * @sect Examples:
 *
 * To enable the basic completion feature :
 *
 * <pre>
 * KLineEdit *edit = new KLineEdit( true, this, "mywidget" );
 * KCompletion *comp = edit->completionObject();
 * // Connect to the return pressed signal - optional
 * connect(edit,SIGNAL(returnPressed(const QString&)),comp,SLOT(addItem(const QString&));
 * </pre>
 *
 * To use a customized completion objects or your
 * own completion object :
 *
 * <pre>
 * KLineEdit *edit = new KLineEdit( this,"mywidget" );
 * KURLCompletion *comp = new KURLCompletion();
 * edit->setCompletionObject( comp );
 * // Connect to the return pressed signal - optional
 * connect(edit,SIGNAL(returnPressed(const QString&)),comp,SLOT(addItem(const QString&));
 * </pre>
 *
 * Other miscelanous function call examples :
 *
 * <pre>
 * // Tell the widget not to handle completion and
 * // rotation internally.
 * edit->setHandleSignals( false );
 * // Set your own completion key for manual completions.
 * edit->setCompletionKey( Qt::End );
 * // Shows the context (popup) menu
 * edit->setEnableContextMenu();
 * // Temporarly disable signal emition
 * edit->disableSignals();
 * // Default the key-bindings to system settings.
 * edit->useGlobalSettings();
 * </pre>
 *
 * @short An enhanced single line input widget.
 * @author Dawit Alemayehu <adawit@earthlink.net>
 */
class KLineEdit : public QLineEdit, public KCompletionBase
{
  Q_OBJECT

public:

    /**
    * Constructs a KLineEdit object with a default text, a parent,
    * and a name.
    *
    * @param string text to be shown in the edit widget
    * @param parent the parent object of this widget
    * @param name the name of this widget
    * @param hsig determines if this widget automatically handles both signals internally.
    */
    KLineEdit( const QString &string, QWidget *parent, const char *name = 0 );

    /**
    * Constructs a KLineEdit object with a parent and a name.
    *
    * @param string text to be shown in the edit widget
    * @param parent the parent object of this widget
    * @param name the name of this widget
    * @param hsig determines if this widget automatically handles both signals internally.
    */
    KLineEdit ( QWidget *parent=0, const char *name=0 );

    /**
    *  Destructor.
    */
    virtual ~KLineEdit ();

    /**
    * Puts cursor at the end of the string.
    *
    * This method is deprecated.  Use @ref QLineEdit::end
    * instead.
    *
    * @deprecated
    * @ref QLineEdit::end
    */
    void cursorAtEnd() { end( false ); }

    /**
    * Re-implemented from @ref KCompletionBase for internal reasons.
    *
    * This function is re-implemented in order to make sure that
    * the EchoMode is acceptable before we set the completion mode.
    *
    * See @ref KCompletionBase::setCompletionMode
    */
    virtual void setCompletionMode( KGlobalSettings::Completion mode );

   /**
    * Enables/disables the popup (context) menu.
    *
    * This method also allows you to enable/disable the context
    * menu. If this method is invoked without an argument, the
    * context menu will be enabled.  By default the mode changer
    * is visible when context menu is enabled.  Use either the
    * second boolean parameter or @ref hideModechanger() if you
    * do not want this item to be visible.   Also by default, the
    * context menu is created if this widget is editable. Call this
    * function with the argument set to false to disable the popup
    * menu.
    *
    * @param showMenu if true, show the context menu.
    * @param showMode if true, show the mode changer item.
    */
    virtual void setEnableContextMenu( bool showMenu );

    /**
    * Returns true when the context menu is enabled.
    *
    * @return @p true if context menu is enabled.
    */
    bool isContextMenuEnabled() const { return m_bEnableMenu; }

signals:

    /**
    * This signal is emitted when the user presses the return
    * key.  The argument is the current text.  Note that this
    * signal is NOT emitted if the widget's EchoMode is set to
    * QLineEdit::Password.
    */
    void returnPressed( const QString& );

    /**
    * Signal emitted when the completion key is pressed.
    *
    * Please note that this signal is NOT emitted if the
    * completion mode is set to CompletionNone or EchoMode is
    * NOT normal.
    */
    void completion( const QString& );

    /**
    * Signal emitted when the rotate up key is pressed.
    * See @ref KCompletionBase::setRotateUpKey.
    *
    * Note that this signal is NOT emitted if the completion
    * mode is set to CompletionNone or EchoMode is NOT normal.
    */
    void rotateUp();

    /**
    * Signal emitted when the rotate down key is pressed.
    * See @ref KCompletionBase::setRotateDownKey.
    *
    * Note that this signal is NOT emitted if the completion
    * mode is set to CompletionNone or EchoMode is NOT normal.
    */
    void rotateDown();

public slots:

    /*
    * Iterates in the up (previous match) direction through
    * the completion list if it is available.
    *
    * This slot is intended to make it easy to connect the
    * rotate up signal in order to make the widget itself handle
    * rotation events internally.  Note that no action is taken
    * if there is no completion object or the completion object
    * does not contain a next match.
    */
    virtual void iterateUpInList() { rotateText( completionObject()->previousMatch() ); }

    /*
    * Iterates in the down (next match) direction through the
    * completion list if it is available.
    *
    * This slot is intended to make it easy to connect the
    * rotate down signal in order to make the widget itself
    * handle rotation events internally.  Note that no action
    * is taken if there is no completion object or the completion
    * object does not contain a next match.
    */
    virtual void iterateDownInList() { rotateText(  completionObject()->nextMatch() ); }

protected slots:

    /**
    * Accepts the "aboutToShow" signal from the completion
    * sub-menu inserted by @ref showCompletionMenu.
    *
    * This method sets the completion mode to the one
    * requested by the end user.
    */
    virtual void selectedItem( int itemID ) { setCompletionMode( (KGlobalSettings::Completion)itemID ); }

    /**
    * Populates the sub menu before it is displayed.
    *
    * All the items are inserted by the completion base
    * class.  See @KCompletionBase::insertCompletionItems.
    * The items then invoke the slot giiven by the
    */
    virtual void showCompletionMenu() { insertCompletionItems( this, SLOT( selectedItem( int ) ) ); }

    /**
    * Inserts the completion menu item as needed.
    *
    * Since this widget comes with its own pop-up menu
    * this slot is needed to invoke the method need to
    * insert the completion menu.  This method,
    * @ref KCompletionBase::insetCompeltionMenu, is
    * defined by the KCompletionBase.
    */
    virtual void aboutToShowMenu() { insertCompletionMenu( this, SLOT( showCompletionMenu() ), m_pContextMenu, m_pContextMenu->count()-1 ); }

    /**
    * Completes the remaining text with a matching one from
    * a given list.
    */
    virtual void makeCompletion( const QString& );

protected:

    /**
    * Initializes variables.  Called from the constructors.
    */
    virtual void init();

    /**
    * Rotates the text on rotation events.
    *
    * @param string the text to replace the current one with.
    */
    void rotateText( const QString& );

    /**
    * Implementation of @ref KCompletionBase::connectSignals().
    *
    * This function simply connects the signals to appropriate
    * slots when they are handled internally.
    *
    * @param handle if true, handle completion & roation internally.
    */
    virtual void connectSignals( bool handle ) const;

    /**
    * Re-implemented for internal reasons.  API not affected.
    *
    * See @ref QLineEdit::keyPressEvent.
    */
    virtual void keyPressEvent( QKeyEvent * );

    /**
    * Re-implemented for internal reasons.  API not affected.
    *
    * See @ref QLineEdit::mousePressEvent.
    */
    virtual void mousePressEvent( QMouseEvent * );

private :
    // Pointers to the context & sub menus.
    QPopupMenu *m_pContextMenu;

    // Indicates whether the context menu is enabled
    // or disabled
    bool m_bEnableMenu;
};

#endif
