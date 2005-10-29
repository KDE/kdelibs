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

#ifndef _KLINEEDIT_H
#define _KLINEEDIT_H

#include <qlineedit.h>
#include <kcompletion.h>

class QPopupMenu;

class KCompletionBox;
class KURL;

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
 * simply use @p setHandleSignals( bool ) or set the boolean parameter in
 * the above functions to false.
 *
 * The default key-bindings for completion and rotation is determined
 * from the global settings in KStdAccel. These values, however, can
 * be overridden locally by invoking @p setKeyBinding(). You can easily
 * revert these settings back to the default by simply calling
 * @p useGlobalSettings(). An alternate method would be to default
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
 * \b Usage \n
 *
 * To enable the basic completion feature :
 *
 * \code
 * KLineEdit *edit = new KLineEdit( this, "mywidget" );
 * KCompletion *comp = edit->completionObject();
 * // Connect to the return pressed signal - optional
 * connect(edit,SIGNAL(returnPressed(const QString&)),comp,SLOT(addItem(const QString&)));
 * \endcode
 *
 * To use a customized completion objects or your
 * own completion object :
 *
 * \code
 * KLineEdit *edit = new KLineEdit( this,"mywidget" );
 * KURLCompletion *comp = new KURLCompletion();
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
 * edit->setContextMenuEnabled( false );
 *
 * // Temporarily disable signal (both completion & iteration) emitions
 * edit->disableSignals();
 *
 * // Default the key-bindings back to the default system settings.
 * edit->useGlobalKeyBindings();
 * \endcode
 *
 * @author Dawit Alemayehu <adawit@kde.org>
 */

class KDEUI_EXPORT KLineEdit : public QLineEdit, public KCompletionBase
{
    friend class KComboBox;

    Q_OBJECT
    Q_PROPERTY( bool contextMenuEnabled READ isContextMenuEnabled WRITE setContextMenuEnabled )
    Q_PROPERTY( bool urlDropsEnabled READ isURLDropsEnabled WRITE setURLDropsEnabled )
    Q_PROPERTY( bool trapEnterKeyEvent READ trapReturnKey WRITE setTrapReturnKey )
    Q_PROPERTY( bool enableSqueezedText READ isSqueezedTextEnabled WRITE setEnableSqueezedText )

public:

    /**
     * Constructs a KLineEdit object with a default text, a parent,
     * and a name.
     *
     * @param string Text to be shown in the edit widget.
     * @param parent The parent object of this widget.
     * @param name the name of this widget
     */
    KLineEdit( const QString &string, QWidget *parent, const char *name = 0 );

    /**
     * Constructs a KLineEdit object with a parent and a name.
     *
     * @param parent The parent object of this widget.
     * @param name The name of this widget.
     */
    KLineEdit ( QWidget *parent=0, const char *name=0 );

    /**
     *  Destructor.
     */
    virtual ~KLineEdit ();

    /**
     * Sets @p url into the lineedit. It uses KURL::prettyURL() so
     * that the url is properly decoded for displaying.
     */
    void setURL( const KURL& url );

    /**
     * Puts the text cursor at the end of the string.
     *
     * This method is deprecated.  Use QLineEdit::end()
     * instead.
     *
     * @deprecated
     * QLineEdit::end()
     */
    void cursorAtEnd() { end( false ); }

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
    */
    virtual void setContextMenuEnabled( bool showMenu ) {  m_bEnableMenu = showMenu; }

    /**
     * Returns @p true when the context menu is enabled.
     */
    bool isContextMenuEnabled() const { return m_bEnableMenu; }

    /**
     * Enables/Disables handling of URL drops. If enabled and the user
     * drops an URL, the decoded URL will be inserted. Otherwise the default
     * behavior of QLineEdit is used, which inserts the encoded URL.
     *
     * @param enable If @p true, insert decoded URLs
     */
    void setURLDropsEnabled( bool enable );

    /**
     * Returns @p true when decoded URL drops are enabled
     */
    bool isURLDropsEnabled() const;

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
     * Re-implemented for internal reasons.  API not affected.
     *
     */
    virtual bool eventFilter( QObject *, QEvent * );

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
     * @since 3.2
     */
    void setEnableSqueezedText( bool enable );

    /**
     * Returns true if text squeezing is enabled.
     * This is only valid when the widget is in read-only mode.
     *
     * @since 3.2
     */
    bool isSqueezedTextEnabled() const;

    /**
     * Returns the original text if text squeezing is enabled.
     * If the widget is not in "read-only" mode, this function
     * returns the same thing as QLineEdit::text().
     *
     * @see QLineEdit
     * @since 3.2
     */
    QString originalText() const;

    /**
     * Set the completion-box to be used in completion mode
     * KGlobalSettings::CompletionPopup.
     * This will do nothing if a completion-box already exists.
     *
     * @param box The KCompletionBox to set
     * @since 3.4
    */
    void setCompletionBox( KCompletionBox *box );

signals:

    /**
     * Emitted whenever the completion box is activated.
     * @since 3.1
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
     * Emitted when the text rotation key-bindings are pressed.
     *
     * The argument indicates which key-binding was pressed.
     * In KLineEdit's case this can be either one of two values:
     * PrevCompletionMatch or NextCompletionMatch. See
     * @p setKeyBinding for details.
     *
     * Note that this signal is @em not emitted if the completion
     * mode is set to @p KGlobalSettings::CompletionNone or
     * @p echoMode() is @em not  normal.
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
    void aboutToShowContextMenu( QPopupMenu * p );

public slots:

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
     * Sets @p items into the completion-box if completionMode() is
     * CompletionPopup. The popup will be shown immediately.
     *
     * @param items list of completion matches to be shown in the completion box.
     */
    void setCompletedItems( const QStringList& items );

    /**
     * Same as the above function except it allows you to temporarily
     * turn off text completion in CompletionPopupAuto mode.
     *
     * TODO: Merge with above function in KDE 4.
     * TODO: Does that make this or the above @deprecated ?
     *
     * @param items list of completion matches to be shown in the completion box.
     * @param autoSuggest true if you want automatic text completion (suggestion) enabled.
     */
    void setCompletedItems( const QStringList& items, bool autoSuggest );

    /**
     * Reimplemented to workaround a buggy QLineEdit::clear()
     * (changing the clipboard to the text we just had in the lineedit)
     */
    virtual void clear();

    /**
     * Squeezes @p text into the line edit.
     * This can only be used with read-only line-edits.
     * @since 3.1
     */
    void setSqueezedText( const QString &text);

    /**
     * Re-implemented to enable text squeezing. API is not affected.
     */
    virtual void setText ( const QString& );


protected slots:

    /**
    * Completes the remaining text with a matching one from
    * a given list.
    */
    virtual void makeCompletion( const QString& );

    /**
     * @deprecated.  Will be removed in the next major release!
     */
    void slotAboutToShow() {}

    /**
     * @deprecated.  Will be removed in the next major release!
     */
    void slotCancelled() {}

    /**
     * Resets the current displayed text.
     * Call this function to revert a text completion if the user
     * cancels the request. Mostly applies to popup completions.
     */
    void userCancelled(const QString & cancelText);

protected:

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
    * See QLineEdit::createPopupMenu().
    */
    virtual QPopupMenu *createPopupMenu();

    /**
    * Re-implemented to handle URI drops.
    *
    * See QLineEdit::dropEvent().
    */
    virtual void dropEvent( QDropEvent * );

    /*
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
    * Re-implemented for internal reasons.  API not affected.
    *
    * See QLineEdit::focusInEvent().
    */
    virtual void focusInEvent( QFocusEvent* );

    /**
     * Whether in current state text should be auto-suggested
     * @since 3.4
    */
    bool autoSuggest() const;

private slots:
    void completionMenuActivated( int id );
    void tripleClickTimeout();  // resets possibleTripleClick
    void slotRestoreSelectionColors();
    void setTextWorkaround( const QString& text );

private:

    // Constants that represent the ID's of the popup menu.
    enum MenuID
    {
        Default = 42,
        NoCompletion,
        AutoCompletion,
        ShellCompletion,
        PopupCompletion,
        ShortAutoCompletion,
        PopupAutoCompletion
    };

    /**
     * Initializes variables.  Called from the constructors.
     */
    void init();

    /**
     * Checks whether we should/should not consume a key used as
     * an accelerator.
     */
    bool overrideAccel (const QKeyEvent* e);

    /**
     * Properly sets the squeezed text whenever the widget is
     * created or resized.
     */
    void setSqueezedText ();

    bool m_bEnableMenu;

    bool possibleTripleClick;  // set in mousePressEvent, deleted in tripleClickTimeout

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KLineEditPrivate;
    KLineEditPrivate *d;
};

#endif
