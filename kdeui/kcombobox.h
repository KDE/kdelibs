/* This file is part of the KDE libraries

   Copyright (c) 2000 Dawit Alemayehu <adawit@earthlink.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation; either
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

#ifndef _KCOMBOBOX_H
#define _KCOMBOBOX_H

#include <qcombobox.h>
#include <qlineedit.h>
#include <qpopupmenu.h>
#include <qlistbox.h>

#include <kcompletion.h>


/**
 * A combined button, line-edit and a popup list widget.
 *
 * This widget inherits from QComboBox and implements
 * the following additional functionalities : a built-in
 * for automatic as well as manual completion and rotation
 * ( the ability to iterate through a given list) features,
 * configurable key-bindings to activate these features and
 * a popup-menu item that can be used to allow the user to
 * change completion modes on the fly based on their preference.
 *
 * KComboBox emits a few more additional signals as well:
 * The main ones being the @ref comepltion and the @ref
 * rotation signal metioned above.  The completion signal
 * is intended to be connected to a slot that will assist
 * the user in filling out the remaining text while the
 * rotation signals, both @ref rotateUp and @ref rotateDown,
 * are intended to be used to transverse through some kind
 * of list in opposing directions.  The @ref returnPressed
 * signals are emitted when the user presses the return key.
 *
 * This widget by default creates a completion object whenever
 * you invoke the member function @ref completionObject for the
 * first time.  You can also assign your own completion object
 * through @ref setCompletionObject function whenever you want
 * to control the kind of completion object that needs to be
 * used.  Additionally, when you create a completion object through
 * either @ref completionObject or @ref setCompletionObject this
 * widget will be automatically enabled to handle the signals.  If
 * you do not need this feature, simply use the appropriate accessor
 * methods or the boolean paramters on the above function to shut
 * them off.
 *
 * The default key-binding for completion and rotation is
 * determined from the global settings in @ref KStdAccel.
 * However, these values can be set locally to override these
 * global settings.  Simply invoking @ref useGlobalSettings
 * then allows you to immediately default the bindings back
 * to the global settings again.  You can also default the
 * key-bindings by simply invoking the @ref setXXXKey method
 * without any argumet.  Note that if this widget is not
 * editable, i.e. it is constructed as a "select-only" widget,
 * then only one completion mode, CompletionAuto, is allowed.
 * All the other modes are simply ignored.  The CompletionAuto
 * mode in this case allows you to automatically select an item
 * the list that matches the pressed key-codes.  For example, if
 * you have a list of countries, typing the first few letters of
 * the name attempts to find a match and if one is found it will
 * be selected as the current item.
 *
 * @sect Example:
 *
 * To enable the basic completion feature :
 *
 * <pre>
 * KComboBox *combo = new KComboBox( true, this, "mywidget" );
 * KCompletion *comp = combo->completionObject();
 * // Connect to the return pressed signal - optional
 * connect(combo,SIGNAL(returnPressed(const QString&)),comp,SLOT(addItem(const QString&));
 * </pre>
 *
 * To use a customized completion objects or your
 * own completion object :
 *
 * <pre>
 * KComboBox *combo = new KComboBox( this,"mywidget" );
 * KURLCompletion *comp = new KURLCompletion();
 * combo->setCompletionObject( comp );
 * // Connect to the return pressed signal - optional
 * connect(combo,SIGNAL(returnPressed(const QString&)),comp,SLOT(addItem(const QString&));
 * </pre>
 *
 * Other miscelanous functions :
 *
 * <pre>
 * // Tell the widget not to handle completion and rotation
 * combo->setHandleSignals( false );
 * // set your own completion key for manual completions.
 * combo->setCompletionKey( Qt::End );
 * // Shows the context (popup) menu
 * combo->setEnableContextMenu( false );
 * // Temporarly disable signal emition
 * combo->disableSignals();
 * // Default the key-bindings to system settings.
 * combo->useGlobalSettings();
 * </pre>
 *
 * @short An enhanced combo box.
 * @author Dawit Alemayehu <adawit@earthlink.net>
 */
class KComboBox : public QComboBox, public KCompletionBase
{
  Q_OBJECT

public:

    /**
    * Constructs a read-only or rather select-only combo box with a parent object
    * and a name.
    *
    * @param parent the parent object of this widget
    * @param name the name of this widget
    */
    KComboBox( QWidget *parent=0, const char *name=0 );

    /**
    * Constructs a "read-write" or "read-only" combo box depending on the value of
    * the first argument( bool rw ) with a parent, a name.
    *
    * @param rw when @p true widget will be editable.
    * @param parent the parent object of this widget.
    * @param name the name of this widget.
    */
    KComboBox( bool rw, QWidget *parent=0, const char *name=0 );

    /**
    * Destructor.
    */
    virtual ~KComboBox();

    /**
    * Returns the current cursor position.
    *
    * This method always returns a -1 if the combo-box is NOT
    * editable (read-write).
    *
    * @return current cursor position.
    */
    int cursorPosition() const { return ( m_pEdit ) ? m_pEdit->cursorPosition() : -1; }

    /**
    * Re-implemented from QComboBox.
    *
    * If true, the completion mode will be set to automatic.
    * Otherwise, it is defaulted to the gloabl setting.  This
    * methods has been replaced by the more comprehensive @ref
    * setCompletionMode.
    *
    * @param autocomplete flag to enable/disable automatic completion mode.
    */
    virtual void setAutoCompletion( bool autocomplete );

    /**
    * Re-implemented from QComboBox.
    *
    * Returns true if the current completion mode is set
    * to automatic.  See its more comprehensive replacement
    * @ref completionMode.
    *
    * @return true when completion mode is automatic.
    */
    bool autoCompletion() const { return completionMode() == KGlobalSettings::CompletionAuto; }

    /**
    * Enables or disables the popup (context) menu.
    *
    * This method only works if this widget is editable, i.e.
    * read-write and allows you to enable/disable the context
    * menu.  It does nothing if invoked for a none-editable
    * combo-box.  Note that by default the mode changer item
    * is made visiable whenever the context menu is enabled.
    * Use * @ref hideModechanger() if you want to hide this
    * item.    Also by default, the context menu is created if
    * this widget is editable. Call this function with the
    * argument set to false to disable the popup menu.
    *
    * @param showMenu if true, show the context menu.
    * @param showMode if true, show the mode changer.
    */
    virtual void setEnableContextMenu( bool showMenu );

    /**
    * Returns true when the context menu is enabled.
    *
    * @return true if context menu is enabled.
    */
    bool isContextMenuEnabled() const { return m_bEnableMenu; }

    /**
    * Returns true if the combo-box is editable.
    *
    * @return true if combo is editable.
    */
    bool isEditable() const { return (m_pEdit!= 0); }

signals:
    /**
    * This signal is emitted when the user presses
    * the return key.  Note that this signal is only
    * emitted if this widget is editable.
    */
    void returnPressed();

    /**
    * This signal is emitted when the user presses
    * the return key.  The argument is the current
    * text being edited.  This signal is just like
    * @ref returnPressed() except it contains the
    * current text as its argument.
    *
    * Note that this signal is only emitted if this
    * widget is editable.
    */
    void returnPressed( const QString& );

    /**
    * This signal is emitted when the completion key
    * is pressed.  The argument is the current text
    * being edited.
    *
    * Note that this signal is NOT available if this
    * widget is non-editable or the completion mode is
    * set to KGlobalSettings::CompletionNone.
    */
    void completion( const QString& );

    /**
    * This signal is emitted when the rotate up key is pressed.
    *
    * Note that this signal is NOT available if this widget is non-editable
    * or the completion mode is set to KGlobalSettings::CompletionNone.
    */
    void rotateUp();

    /**
    * This signal is emitted when the rotate down key is pressed.
    *
    * Note that this signal is NOT available if this widget is non-editable
    * or the completion mode is set to KGlobalSettings::CompletionNone.
    */
    void rotateDown();

public slots:

    /**
    * Iterates in the up (previous match) direction through the
    * completion list if it is available.
    *
    * This slot is intended to make it easy to connect the rotate
    * up signal in order to make the widget itself handle rotation
    * events internally.  Note that no action is taken if there is
    * no completion object or the completion object does not contain
    * a next match.
    */
    virtual void iterateUpInList() { rotateText( completionObject()->previousMatch() ); }

    /**
    * Iterates in the down (next match) direction through the
    * completion list if it is available.
    *
    * This slot is intended to make it easy to connect the rotate
    * down signal in order to make the widget itself handle rotation
    * events internally.  Note that no action is taken if there is
    * no completion object or the completion object does not contain
    * a next match.
    */
    virtual void iterateDownInList() { rotateText( completionObject()->nextMatch() ); }

protected slots:

    /**
    *  changes the completion mode.
    *
    * This slot sets the completion mode to the one
    * requested by the end user through the popup
    * menu.
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
    virtual void aboutToShowMenu() { insertCompletionMenu( this, SLOT( showCompletionMenu() ), m_pContextMenu, m_pContextMenu->count() - 1 ); }

    /**
    * Deals with highlighting the seleted item when
    * return is pressed in the list box (editable-mode only).
    */
    virtual void itemSelected( QListBoxItem* );

    /**
    * Deals with text changes in auto completion mode.
    */
    virtual void makeCompletion( const QString& );

protected:

    /**
    * Initializes the variables upon construction.
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
    * Re-implemented for internal reasons.  API is not affected.
    *
    * See @ref QComboBox::keyPressEvent.
    */
    virtual void keyPressEvent ( QKeyEvent* );

    /**
    * Re-implemented for internal reasons.  API is not affected.
    *
    * See @ref QComboBox::mousePressEvent.
    */
    virtual bool eventFilter( QObject *, QEvent * );

private :
    // Flag that indicates whether we enable/disable
    // the context (popup) menu.
    bool m_bEnableMenu;
    // Pointer to the line editor.
    QLineEdit* m_pEdit;
    // Context Menu items.
    QPopupMenu *m_pContextMenu;

    class KComboBoxPrivate;
    KComboBoxPrivate *d;
};
#endif
