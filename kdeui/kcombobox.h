/* This file is part of the KDE libraries

   Copyright (c) 2000 Dawit Alemayehu <adawit@kde.org>
                 2000 Carsten Pfeiffer <pfeiffer@kde.org>

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
 * This widget inherits from @ref QComboBox and implements
 * the following additional functionalities:  a completion
 * object that provides both automatic and manual text
 * completion as well as text rotation features, configurable
 * key-bindings to activate these features and a popup-menu
 * item that can be used to allow the user to set text completion
 * modes on the fly based on their preference.
 *
 * To support these new features KComboBox also emits a few
 * more additional signals as well.  The main ones being the
 * @ref completion() and @ref rotation() signals.  The completion signal
 * is intended to be connected to a slot that will assist the user
 * in filling out the remaining text while the rotation signals, both
 * @ref rotateUp and @ref rotateDown, are intended to be used to
 * transverse through some kind of list in opposing directions.
 * The @ref previousMatch and @ref nextMatch signals are used to
 * iterate through all possible matches whenever there are more than
 * one possible text completion matches and the @ref returnPressed
 * signals are emitted when the user presses the return key.
 *
 * This widget by default creates a completion object when you invoke
 * the @ref #completionObject() member function for the first time or use
 * @ref #setCompletionObject() to assign your own completion object.
 * Additionally, to make this widget more functional, KComboBox will
 * automatically handle the iteration and completion signals internally
 * when a completion object is created through either one of the methods
 * mentioned above.  If you do not need these feature, simply use
 * @ref KCompletionBase::setHandleSignals() or alternatively the boolean
 * parameter in @ref #setCompletionObject(), to turn them off.
 *
 * The default key-binding for completion and rotation is determined
 * from the global settings in @ref KStdAccel.  These values, however,
 * can be set locally to override the global settings through the
 * @ref #setKeyBinding() member function.  To default the values back,
 * simply invoke @ref #useGlobalSettings().  You can also default
 * individual key-binding by simply calling the @ref #setKeyBinding() member
 * function with the default the second argument.
 *
 * Note that if this widget is not editable, i.e. it is constructed as a
 * "select-only" widget, then only one completion mode, @p CompletionAuto,
 * is allowed.  All the other modes are simply ignored.  The @p CompletionAuto
 * mode in this case allows you to automatically select an item from the list
 * that matches the key-code of the first key pressed.
 *
 * @sect Example:
 *
 * To enable the basic completion feature:
 *
 * <pre>
 * KComboBox *combo = new KComboBox( true, this, "mywidget" );
 * KCompletion *comp = combo->completionObject();
 * // Connect to the return pressed signal - optional
 * connect(combo,SIGNAL(returnPressed(const QString&)),comp,SLOT(addItem(const QString&));
 * </pre>
 *
 * To use your own completion object:
 *
 * <pre>
 * KComboBox *combo = new KComboBox( this,"mywidget" );
 * KURLCompletion *comp = new KURLCompletion();
 * combo->setCompletionObject( comp );
 * // Connect to the return pressed signal - optional
 * connect(combo,SIGNAL(returnPressed(const QString&)),comp,SLOT(addItem(const QString&));
 * </pre>
 *
 * Miscellaneous function calls:
 *
 * <pre>
 * // Tell the widget not to handle completion and rotation
 * combo->setHandleSignals( false );
 * // Set your own completion key for manual completions.
 * combo->setKeyBinding( KCompletionBase::TextCompletion, Qt::End );
 * // Hide the context (popup) menu
 * combo->setContextMenuEnabled( false );
 * // Temporarly disable signal emition
 * combo->disableSignals();
 * // Default the all key-bindings to their system-wide settings.
 * combo->useGlobalKeyBindings();
 * </pre>
 *
 * @short An enhanced combo box.
 * @author Dawit Alemayehu <adawit@kde.org>
 */
class KComboBox : public QComboBox, public KCompletionBase
{
  Q_OBJECT
  Q_PROPERTY( bool autoCompletion READ autoCompletion WRITE setAutoCompletion )
  Q_PROPERTY( bool contextMenuEnabled READ isContextMenuEnabled WRITE setContextMenuEnabled )

public:
	
    /**
    * Constructs a read-only or rather select-only combo box with a parent object
    * and a name.
    *
    * @p parent the parent object of this widget
    * @p name the name of this widget
    */
    KComboBox( QWidget *parent=0, const char *name=0 );

    /**
    * Constructs a "read-write" or "read-only" combo box depending on the value of
    * the first argument( bool rw ) with a parent, a name.
    *
    * @p rw when @p true widget will be editable.
    * @p parent the parent object of this widget.
    * @p name the name of this widget.
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
    * @returns current cursor position.
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
    * @p autocomplete flag to enable/disable automatic completion mode.
    */
    virtual void setAutoCompletion( bool autocomplete );

    /**
    * Re-implemented from QComboBox.
    *
    * Returns true if the current completion mode is set
    * to automatic.  See its more comprehensive replacement
    * @ref completionMode.
    *
    * @returns true when completion mode is automatic.
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
    * @p showMenu if true, show the context menu.
    * @p showMode if true, show the mode changer.
    */
    virtual void setContextMenuEnabled( bool showMenu );

    /**
    * Returns true when the context menu is enabled.
    *
    * @returns true if context menu is enabled.
    */
    bool isContextMenuEnabled() const { return m_bEnableMenu; }

    /**
    * Returns true if the combo-box is editable.
    *
    * @returns true if combo is editable.
    */
    bool isEditable() const { return !m_pEdit.isNull() ; }

    /**
     * Convenience method which iterates over all items and checks if
     * any of them is equal to @p text. If @p text is an empty string, false
     * is returned.
     * @returns true if an item with the string @p text is in the combobox.
     */
    bool contains( const QString& text ) const;

    /**
     * By default, KComboBox recognizes Key_Return and Key_Enter and emits
     * the @ref returnPressed() signals, but it also lets the event passr,
     * for example causing a dialog's default-button to be called.
     *
     * Call this method with @p grab = true to make KComboBox stop these
     * events. The signals will still be emitted of course.
     *
     * Only affects read-writable comboboxes.
     *
     * @see #grabReturnKeyEvents
     */
    void setTrapReturnKey( bool grab );

    /**
     * @returns true if keyevents of Key_Return or Key_Enter will be stopped
     * or if they will be propagated.
     *
     * @see #setGrabReturnKeyEvents
     */
    bool trapReturnKey() const;

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
    * Emitted when the text rotation key-bindings are pressed.
    *
    * The argument indicates which key-binding was pressed.
    * In this case this can be either one of four values:
    * PrevCompletionMatch, NextCompletionMatch, RotateUp or
    * rotateDown. See @ref KCompletionBase::setKeyBinding for
    * details.
    *
    * Note that this signal is NOT emitted if the completion
    * mode is set to CompletionNone.
    */
    void textRotation( KCompletionBase::KeyBindingType );

public slots:

    /**
    * Iterates through all possible matches of the completed text
    * or the history list.
    *
    * Depending on the value of the argument, this function either
    * iterates through the history list of this widget or the all
    * possible matches in whenever multiple matches result from a
    * text completion request.  Note that the all possible match
    * iteration will not work if there are no previous matches, i.e.
    * no text has been completed and the *nix shell history list
    * rotation is only available if the insertion policy for this
    * widget is set either @p QComobBox::AtTop or @p QComboBox::AtBottom.
    * For other insertion modes whatever has been typed by the user
    * when the rotation event was initiated will be lost.
    *
    * @p type the key-binding invoked.
    */
    void rotateText( KCompletionBase::KeyBindingType /* type */ );

protected slots:

    /**
    * Deals with highlighting the seleted item when
    * return is pressed in the list box (editable-mode only).
    */
    virtual void itemSelected( QListBoxItem* );

    /**
    * Completes text according to the completion mode.
    *
    * Note: this method is @p not invoked if the completion mode is
    * set to CompletionNone.  Also if the mode is set to @p CompletionShell
    * and multiple matches are found, this method will complete the
    * text to the first match with a beep to inidicate that there are
    * more matches.  Then any successive completion key event iterates
    * through the remaining matches.  This way the rotation functionality
    * is left to iterate through the list as usual.
    */
    virtual void makeCompletion( const QString& );

protected:

    /**
    * Initializes the variables upon construction.
    */
    virtual void init();

    /**
    * Implementation of @ref KCompletionBase::connectSignals().
    *
    * This function simply connects the signals to appropriate
    * slots when they are handled internally.
    *
    * @p handle if true, handle completion & roation internally.
    */
    virtual void connectSignals( bool handle ) const;

    /**
    * @reimplemented
    */
    virtual void keyPressEvent ( QKeyEvent* );

    /**
    * @reimplemented
    */
    virtual bool eventFilter( QObject *, QEvent * );

private :

    // Flag that indicates whether we enable/disable
    // the context (popup) menu.
    bool m_bEnableMenu;

    // Pointer to the line editor.
    QGuardedPtr<QLineEdit> m_pEdit;

    // indicating if we should stop return-key events from propagating
    bool m_trapReturnKey;

    class KComboBoxPrivate;
    KComboBoxPrivate *d;
};


/**
 * A combobox which implements a history like a unix shell. You can navigate
 * through all the items by using the Up or Down arrows (configurable of
 * course). Additionally, weighted completion is available. So you should
 * load and save the completion list to preserve the weighting between
 * sessions.
 *
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 * @short A combobox for offering a history and completion
 */
class KHistoryCombo : public KComboBox
{
    Q_OBJECT
    Q_PROPERTY( QStringList historyItems READ historyItems WRITE setHistoryItems )

public:
    /**
     * Constructs a "read-write" combobox. A read-only history combobox
     * doesn't make much sense, so it is only available as read-write.
     *
     * The insertion-policy is set to NoInsertion, you have to add the items
     * yourself via the slot @ref addToHistory. If you want every item added,
     * use
     *
     * <pre>
     * connect( combo, SIGNAL( activated( const QString& )),
     *          combo, SLOT( addToHistory( const QString& )));
     * </pre>
     *
     * Use @ref QComboBox::setMaxCount() to limit the history.
     *
     * @p parent the parent object of this widget.
     * @p name the name of this widget.
     */
    KHistoryCombo( QWidget *parent = 0L, const char *name = 0L );

    /**
     * Calls @ref clearHistory and sets the list of history items. Might get
     * truncated if it is longer than @ref maxCount()
     *
     * Set @p setCompletionList to true, if you don't have a list of
     * completions. This tells KHistoryCombo to  use all the items for the
     * completion object as well.
     * You won't have the benefit of weighted completion though, so normally
     * you should do something like
     * <pre>
     * KConfig *config = kapp->config();
     * QStringList list;
     *
     * // load the history and completion list after creating the history combo
     * list = config->readEntry( "Completion list" );
     * combo->completionObject()->setItems( list );
     * list = config->readEntry( "History list" );
     * combo->setHistoryItems( list );
     *
     * [...]
     *
     * // save the history and completion list when the history combo is
     * // destroyed
     * list = combo->completionObject()->items()
     * config->writeEntry( "Completion list", list );
     * list = combo->historyItems();
     * config->writeEntry( "History list", list );
     * </pre>
     *
     * Be sure to use different names for saving with KConfig if you have more
     * than one KHistoryCombo.
     *
     * Note: When @p setCompletionList is true, the items are inserted into the
     * KCompletion object with mode KCompletion::Insertion and the mode is set
     * to KCompletion::Weighted afterwards.
     *
     * @see #historyItems
     * @see KComboBox::completionObject
     * @see KCompletion::setItems
     * @see KCompletion::items
     */
    void setHistoryItems(QStringList items,
			 bool setCompletionList = false );

    /**
     * Returns the list of history items. Empty, when this is not a read-write
     * combobox.
     *
     * @see #setHistoryItems
     */
    QStringList historyItems() const;

    /**
     * Removes all items named @p item.
     * @returns true if at least one item was removed.
     *
     * @see #addToHistory
     */
    bool removeFromHistory( const QString& item );

public slots:
    /**
     * Adds an item to the end of the history list and to the completion list.
     * If @ref maxCount() is reached, the first item of the list will be
     * removed.
     *
     * Note: By using this method and not the Q and KComboBox insertItem()
     * methods, you make sure that the combobox stays in sync with the
     * completion. It would be annoying if completion would give an item
     * not in the combobox, and vice versa.
     *
     * If an items is added twice without any other item in between, it will
     * only show up once in the combobox.
     *
     * @see #removeFromHistory
     */
    void addToHistory( const QString& item );

    /**
     * Clears the history and the completion list.
     */
    void clearHistory();

protected:
    virtual void keyPressEvent( QKeyEvent * );

private slots:
    /**
     * resets the iterate index to -1
     */
    void slotReset();

private:
    /**
     * the current position (index) in the combobox, used for Up and Down
     */
    int myIterateIndex;

    /**
     * The text typed before Up or Down was pressed.
     */
    QString myText;

    /**
     * Indicates that the user at least once rotated Up through the entire list
     * Needed to allow going back after rotation.
     */
    bool myRotated;
};


#endif
