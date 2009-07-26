/*  This file is part of the KDE libraries

    This class was originally inspired by Torben Weis'
    fileentry.cpp for KFM II.

    Copyright (C) 1997 Sven Radej <sven.radej@iname.com>
    Copyright (c) 1999 Patrick Ward <PAT_WARD@HP-USA-om5.om.hp.com>
    Copyright (c) 1999 Preston Brown <pbrown@kde.org>

    Completely re-designed:
    Copyright (c) 2000,2001 Dawit Alemayehu <adawit@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License (LGPL) as published by the Free Software Foundation;
    either version 2 of the License, or (at your option) any later
    version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KLINEEDIT_H
#define KLINEEDIT_H

#include <QtGui/QLineEdit>

#include <kcompletion.h>

class QAction;
class QMenu;
class KCompletionBox;
class KUrl;
class KLineEditPrivate;

/**
 * An enhanced QLineEdit widget for inputting text.
 *
 * \b Detail \n
 *
 * This widget inherits from QLineEdit and implements the following
 * additional functionalities: a completion object that provides both
 * automatic and manual text completion as well as multiple match iteration
 * features, configurable key-bindings to activate these features and a
 * popup-menu item that can be used to allow the user to set text completion
 * modes on the fly based on their preference.
 *
 * To support these new features KLineEdit also emits a few more
 * additional signals.  These are: completion( const QString& ),
 * textRotation( KeyBindingType ), and returnPressed( const QString& ).
 * The completion signal can be connected to a slot that will assist the
 * user in filling out the remaining text.  The text rotation signal is
 * intended to be used to iterate through the list of all possible matches
 * whenever there is more than one match for the entered text.  The
 * @p returnPressed( const QString& ) signals are the same as QLineEdit's
 * except it provides the current text in the widget as its argument whenever
 * appropriate.
 *
 * This widget by default creates a completion object when you invoke
 * the completionObject( bool ) member function for the first time or
 * use setCompletionObject( KCompletion*, bool ) to assign your own
 * completion object.  Additionally, to make this widget more functional,
 * KLineEdit will by default handle the text rotation and completion
 * events internally when a completion object is created through either one
 * of the methods mentioned above.  If you do not need this functionality,
 * simply use KCompletionBase::setHandleSignals( bool ) or set the
 * boolean parameter in the above functions to false.
 *
 * The default key-bindings for completion and rotation is determined
 * from the global settings in KStandardShortcut.  These values, however,
 * can be overridden locally by invoking KCompletionBase::setKeyBinding().
 * The values can easily be reverted back to the default setting, by simply
 * calling useGlobalSettings(). An alternate method would be to default
 * individual key-bindings by using setKeyBinding() with the default
 * second argument.
 *
 * If @p EchoMode for this widget is set to something other than @p QLineEdit::Normal,
 * the completion mode will always be defaulted to KGlobalSettings::CompletionNone.
 * This is done purposefully to guard against protected entries such as passwords being
 * cached in KCompletion's list. Hence, if the @p EchoMode is not QLineEdit::Normal, the
 * completion mode is automatically disabled.
 *
 * A read-only KLineEdit will have the same background color as a
 * disabled KLineEdit, but its foreground color will be the one used
 * for the read-write mode. This differs from QLineEdit's implementation
 * and is done to give visual distinction between the three different modes:
 * disabled, read-only, and read-write.
 *
 * KLineEdit has also a password mode which depends of globals KDE settings.  Use
 * KLineEdit::setPasswordMode instead of QLineEdit::echoMode property to have a password field.
 *
 * \b Usage \n
 *
 * To enable the basic completion feature:
 *
 * \code
 * KLineEdit *edit = new KLineEdit( this );
 * KCompletion *comp = edit->completionObject();
 * // Connect to the return pressed signal - optional
 * connect(edit,SIGNAL(returnPressed(const QString&)),comp,SLOT(addItem(const QString&)));
 * \endcode
 *
 * To use a customized completion objects or your
 * own completion object:
 *
 * \code
 * KLineEdit *edit = new KLineEdit( this );
 * KUrlCompletion *comp = new KUrlCompletion();
 * edit->setCompletionObject( comp );
 * // Connect to the return pressed signal - optional
 * connect(edit,SIGNAL(returnPressed(const QString&)),comp,SLOT(addItem(const QString&)));
 * \endcode
 *
 * Note if you specify your own completion object you have to either delete
 * it when you don't need it anymore, or you can tell KLineEdit to delete it
 * for you:
 * \code
 * edit->setAutoDeleteCompletionObject( true );
 * \endcode
 *
 * <b>Miscellaneous function calls :</b>\n
 *
 * \code
 * // Tell the widget to not handle completion and iteration automatically.
 * edit->setHandleSignals( false );
 *
 * // Set your own key-bindings for a text completion mode.
 * edit->setKeyBinding( KCompletionBase::TextCompletion, Qt::End );
 *
 * // Hide the context (popup) menu
 * edit->setContextMenuPolicy( Qt::NoContextMenu );
 *
 * // Default the key-bindings back to the default system settings.
 * edit->useGlobalKeyBindings();
 * \endcode
 *
 * \image html klineedit.png "KDE Line Edit Widgets with clear-button and clickMessage"
 *
 * @author Dawit Alemayehu <adawit@kde.org>
 */

class KDEUI_EXPORT KLineEdit : public QLineEdit, public KCompletionBase //krazy:exclude=qclasses
{
    friend class KComboBox;
    friend class KLineEditStyle;

    Q_OBJECT
    Q_PROPERTY( bool contextMenuEnabled READ isContextMenuEnabled WRITE setContextMenuEnabled )
    Q_PROPERTY( bool urlDropsEnabled READ urlDropsEnabled WRITE setUrlDropsEnabled )
    Q_PROPERTY( bool trapEnterKeyEvent READ trapReturnKey WRITE setTrapReturnKey )
    Q_PROPERTY( bool squeezedTextEnabled READ isSqueezedTextEnabled WRITE setSqueezedTextEnabled )
    Q_PROPERTY( QString clickMessage READ clickMessage WRITE setClickMessage )
    Q_PROPERTY( bool showClearButton READ isClearButtonShown WRITE setClearButtonShown )
    Q_PROPERTY( bool passwordMode READ passwordMode WRITE setPasswordMode )

public:

    /**
     * Constructs a KLineEdit object with a default text, a parent,
     * and a name.
     *
     * @param string Text to be shown in the edit widget.
     * @param parent The parent widget of the line edit.
     */
    explicit KLineEdit( const QString &string, QWidget *parent = 0 );

    /**
     * Constructs a line edit
     * @param parent The parent widget of the line edit.
     */
    explicit KLineEdit( QWidget *parent = 0 );

    /**
     *  Destructor.
     */
    virtual ~KLineEdit ();

    /**
     * Sets @p url into the lineedit. It uses KUrl::prettyUrl() so
     * that the url is properly decoded for displaying.
     */
    void setUrl( const KUrl& url );

     /**
     * Re-implemented from KCompletionBase for internal reasons.
     *
     * This function is re-implemented in order to make sure that
     * the EchoMode is acceptable before we set the completion mode.
     *
     * See KCompletionBase::setCompletionMode
     */
    virtual void setCompletionMode( KGlobalSettings::Completion mode );

   /**
    * Disables completion modes by makeing them non-checkable.
    *
    * The context menu allows to change the completion mode.
    * This method allows to disable some modes.
    */
    void setCompletionModeDisabled( KGlobalSettings::Completion mode, bool disable = true );

   /**
    * Enables/disables the popup (context) menu.
    *
    * Note that when this function is invoked with its argument
    * set to @p true, then both the context menu and the completion
    * menu item are enabled.  If you do not want to the completion
    * item to be visible simply invoke hideModechanger() right
    * after calling this method.  Also by default, the context
    * menu is automatically created if this widget is editable. Thus
    * you need to call this function with the argument set to false
    * if you do not want this behavior.
    *
    * @param showMenu If @p true, show the context menu.
    * @deprecated use setContextMenuPolicy
    */
    virtual KDE_DEPRECATED void setContextMenuEnabled( bool showMenu );

    /**
     * Returns @p true when the context menu is enabled.
     * @deprecated use contextMenuPolicy
     */
    KDE_DEPRECATED bool isContextMenuEnabled() const;

    /**
     * Enables/Disables handling of URL drops. If enabled and the user
     * drops an URL, the decoded URL will be inserted. Otherwise the default
     * behavior of QLineEdit is used, which inserts the encoded URL.
     *
     * @param enable If @p true, insert decoded URLs
     */
    void setUrlDropsEnabled( bool enable );

    /**
     * Returns @p true when decoded URL drops are enabled
     */
    bool urlDropsEnabled() const;

    /**
     * By default, KLineEdit recognizes @p Key_Return and @p Key_Enter and emits
     * the returnPressed() signals, but it also lets the event pass,
     * for example causing a dialog's default-button to be called.
     *
     * Call this method with @p trap = @p true to make @p KLineEdit stop these
     * events. The signals will still be emitted of course.
     *
     * @see trapReturnKey()
     */
    void setTrapReturnKey( bool trap );

    /**
     * @returns @p true if keyevents of @p Key_Return or
     * @p Key_Enter will be stopped or if they will be propagated.
     *
     * @see setTrapReturnKey ()
     */
    bool trapReturnKey() const;

    /**
     * @returns the completion-box, that is used in completion mode
     * KGlobalSettings::CompletionPopup.
     * This method will create a completion-box if none is there, yet.
     *
     * @param create Set this to false if you don't want the box to be created
     *               i.e. to test if it is available.
     */
    KCompletionBox * completionBox( bool create = true );

    /**
     * Reimplemented for internal reasons, the API is not affected.
     */
    virtual void setCompletionObject( KCompletion *, bool hsig = true );

    /**
     * Reimplemented for internal reasons, the API is not affected.
     */
    virtual void copy() const;

    /**
     * Enable text squeezing whenever the supplied text is too long.
     * Only works for "read-only" mode.
     *
     * Note that once text squeezing is enabled, QLineEdit::text()
     * and QLineEdit::displayText() return the squeezed text. If
     * you want the original text, use @ref originalText.
     *
     * @see QLineEdit
     */
    void setSqueezedTextEnabled( bool enable );

    /**
     * Returns true if text squeezing is enabled.
     * This is only valid when the widget is in read-only mode.
     */
    bool isSqueezedTextEnabled() const;

    /**
     * Returns the original text if text squeezing is enabled.
     * If the widget is not in "read-only" mode, this function
     * returns the same thing as QLineEdit::text().
     *
     * @see QLineEdit
     */
    QString originalText() const;

    /**
     * Returns the text as given by the user (i.e. not autocompleted)
     * if the widget has autocompletion disabled, this function
     * returns the same as QLineEdit::text().
     * @since 4.2.2
     */
    QString userText() const;

    /**
     * Set the completion-box to be used in completion mode
     * KGlobalSettings::CompletionPopup.
     * This will do nothing if a completion-box already exists.
     *
     * @param box The KCompletionBox to set
    */
    void setCompletionBox( KCompletionBox *box );

    /**
     * This makes the line edit display a grayed-out hinting text as long as
     * the user didn't enter any text. It is often used as indication about
     * the purpose of the line edit.
     */
    void setClickMessage( const QString &msg );

    /**
     * @return the message set with setClickMessage
     */
    QString clickMessage() const;

    /**
     * This makes the line edit display an icon on one side of the line edit
     * which, when clicked, clears the contents of the line edit.
     * This is useful for such things as location or search bars.
     **/
    void setClearButtonShown(bool show);

    /**
     * @return whether or not the clear button is shown
     **/
    bool isClearButtonShown() const;

    /**
     * @return the size used by the clear button
     * @since KDE 4.1
     **/
    QSize clearButtonUsedSize() const;

    /**
     * Do completion now. This is called automatically when typing a key for instance.
     * Emits completion() and/or calls makeCompletion(), depending on
     * emitSignals and handleSignals.
     *
     * @since 4.2.1
     */
    void doCompletion(const QString& txt);

Q_SIGNALS:

    /**
     * Emitted whenever the completion box is activated.
     */
    void completionBoxActivated (const QString &);

    /**
     * Emitted when the user presses the return key.
     *
     *  The argument is the current text.  Note that this
     * signal is @em not emitted if the widget's @p EchoMode is set to
     * QLineEdit::EchoMode.
     */
    void returnPressed( const QString& );

    /**
     * Emitted when the completion key is pressed.
     *
     * Please note that this signal is @em not emitted if the
     * completion mode is set to @p CompletionNone or @p EchoMode is
     * @em normal.
     */
    void completion( const QString& );

    /**
     * Emitted when the shortcut for substring completion is pressed.
     */
    void substringCompletion( const QString& );

    /**
     * Emitted when the text is changed NOT by autocompletion
     * @since 4.2.2
     */
    void userTextChanged( const QString & ); 

    /**
     * Emitted when the text rotation key-bindings are pressed.
     *
     * The argument indicates which key-binding was pressed.
     * In KLineEdit's case this can be either one of two values:
     * PrevCompletionMatch or NextCompletionMatch. See
     * KCompletionBase::setKeyBinding for details.
     *
     * Note that this signal is @em not emitted if the completion
     * mode is set to @p KGlobalSettings::CompletionNone or @p echoMode() is @em not  normal.
     */
    void textRotation( KCompletionBase::KeyBindingType );

    /**
     * Emitted when the user changed the completion mode by using the
     * popupmenu.
     */
    void completionModeChanged( KGlobalSettings::Completion );

    /**
     * Emitted before the context menu is displayed.
     *
     * The signal allows you to add your own entries into the
     * the context menu that is created on demand.
     *
     * NOTE: Do not store the pointer to the QPopupMenu
     * provided through since it is created and deleted
     * on demand.
     *
     * @param p the context menu about to be displayed
     */
    void aboutToShowContextMenu( QMenu * p );

    /**
     * Emitted when the user clicked on the clear button
     */
    void clearButtonClicked();

public Q_SLOTS:

    /**
     * Re-implemented for internal reasons. API not changed.
     */
    virtual void setReadOnly(bool);

    /**
     * Iterates through all possible matches of the completed text or
     * the history list.
     *
     * This function simply iterates over all possible matches in case
     * multimple matches are found as a result of a text completion request.
     * It will have no effect if only a single match is found.
     *
     * @param type The key-binding invoked.
     */
    void rotateText( KCompletionBase::KeyBindingType type );

    /**
     * See KCompletionBase::setCompletedText.
     */
    virtual void setCompletedText( const QString& );

    /**
     * Same as the above function except it allows you to temporarily
     * turn off text completion in CompletionPopupAuto mode.
     *
     *
     * @param items list of completion matches to be shown in the completion box.
     * @param autoSuggest true if you want automatic text completion (suggestion) enabled.
     */
    void setCompletedItems( const QStringList& items, bool autoSuggest = true );

    /**
     * Reimplemented to workaround a buggy QLineEdit::clear()
     * (changing the clipboard to the text we just had in the lineedit)
     */
    virtual void clear(); // ### KDE 5: check if still required

    /**
     * Squeezes @p text into the line edit.
     * This can only be used with read-only line-edits.
     */
    void setSqueezedText( const QString &text);

    /**
     * Re-implemented to enable text squeezing. API is not affected.
     */
    virtual void setText ( const QString& );

    /**
     * @brief set the line edit in password mode.
     * this change the EchoMode according to KDE preferences.
     * @param b true to set in password mode
     */
    void setPasswordMode( bool b = true );

    /**
     * @return returns true if the lineedit is set to password mode echoing
     */
    bool passwordMode( ) const;


protected Q_SLOTS:

    /**
    * Completes the remaining text with a matching one from
    * a given list.
    */
    virtual void makeCompletion( const QString& );

    /**
     * Resets the current displayed text.
     * Call this function to revert a text completion if the user
     * cancels the request. Mostly applies to popup completions.
     */
    void userCancelled(const QString & cancelText);

protected:

    /**
     * Re-implemented for internal reasons.  API not affected.
     */
    virtual bool event( QEvent * );

    /**
    * Re-implemented for internal reasons.  API not affected.
    *
    * See QLineEdit::resizeEvent().
    */
    virtual void resizeEvent( QResizeEvent * );

    /**
    * Re-implemented for internal reasons.  API not affected.
    *
    * See QLineEdit::keyPressEvent().
    */
    virtual void keyPressEvent( QKeyEvent * );

    /**
    * Re-implemented for internal reasons.  API not affected.
    *
    * See QLineEdit::mousePressEvent().
    */
    virtual void mousePressEvent( QMouseEvent * );

    /**
    * Re-implemented for internal reasons.  API not affected.
    *
    * See QLineEdit::mouseReleaseEvent().
    */
    virtual void mouseReleaseEvent( QMouseEvent * );

    /**
    * Re-implemented for internal reasons.  API not affected.
    *
    * See QWidget::mouseDoubleClickEvent().
    */
    virtual void mouseDoubleClickEvent( QMouseEvent * );

    /**
    * Re-implemented for internal reasons.  API not affected.
    *
    * See QLineEdit::contextMenuEvent().
    */
    virtual void contextMenuEvent( QContextMenuEvent * );

    /**
    * Re-implemented for internal reasons.  API not affected.
    *
    * See QLineEdit::createStandardContextMenu().
    */
    QMenu* createStandardContextMenu();

    /**
    * Re-implemented to handle URI drops.
    *
    * See QLineEdit::dropEvent().
    */
    virtual void dropEvent( QDropEvent * );

    /**
    * This function simply sets the lineedit text and
    * highlights the text appropriately if the boolean
    * value is set to true.
    *
    * @param text
    * @param marked
    */
    virtual void setCompletedText( const QString& /*text*/, bool /*marked*/ );


    /**
     * Sets the widget in userSelection mode or in automatic completion
     * selection mode. This changes the colors of selections.
     */
    void setUserSelection( bool userSelection );

    /**
     * Reimplemented for internal reasons, the API is not affected.
     */
    virtual void create( WId = 0, bool initializeWindow = true,
                         bool destroyOldWindow = true );

    /**
     * Whether in current state text should be auto-suggested
    */
    bool autoSuggest() const;

    virtual void paintEvent( QPaintEvent *ev );

    virtual void focusInEvent( QFocusEvent *ev );

    virtual void focusOutEvent( QFocusEvent *ev );

private Q_SLOTS:
    void completionMenuActivated( QAction *act );
    void tripleClickTimeout();  // resets possibleTripleClick
    void slotRestoreSelectionColors();
    void setTextWorkaround( const QString& text );

    /**
     * updates the icon of the clear button on text change
     **/
    void updateClearButtonIcon(const QString&);

private:


    /**
     * Initializes variables.  Called from the constructors.
     */
    void init();

    bool copySqueezedText( bool clipboard ) const;

    /**
     * Properly sets the squeezed text whenever the widget is
     * created or resized.
     */
    void setSqueezedText ();

    /**
     * updates the geometry of the clear button on resize events
     **/
    void updateClearButton();

private:
    friend class KLineEditPrivate;
    KLineEditPrivate *const d;

    Q_PRIVATE_SLOT( d, void _k_slotSettingsChanged( int category ) )
    Q_PRIVATE_SLOT( d, void _k_updateUserText(const QString&) )
};

#endif
