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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
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
 * @sect Detail
 *
 * This widget inherits from @ref QLineEdit and implements the following
 * additional functionalities: a completion object that provides both
 * automatic and manual text completion as well as multiple match iteration
 * features, configurable key-bindings to activate these features and a
 * popup-menu item that can be used to allow the user to set text completion
 * modes on the fly based on their preference.
 *
 * To support these new features KLineEdit also emits a few more
 * additional signals.  These are: @ref completion( const QString& ),
 * textRotation( KeyBindingType ), and @ref returnPressed( const QString& ).
 * The completion signal can be connected to a slot that will assist the
 * user in filling out the remaining text.  The text rotation signal is
 * intended to be used to iterate through the list of all possible matches
 * whenever there is more than one match for the entered text.  The
 * @p returnPressed( const QString& ) signals are the same as @ref QLineEdit's
 * except it provides the current text in the widget as its argument whenever
 * appropriate.
 *
 * This widget by default creates a completion object when you invoke
 * the @ref completionObject( bool ) member function for the first time or
 * use @ref setCompletionObject( KCompletion*, bool ) to assign your own
 * completion object.  Additionally, to make this widget more functional,
 * KLineEdit will by default handle the text rotation and completion
 * events internally when a completion object is created through either one
 * of the methods mentioned above.  If you do not need this functionality,
 * simply use @ref KCompletionBase::setHandleSignals( bool ) or set the
 * boolean parameter in the above functions to FALSE.
 *
 * The default key-bindings for completion and rotation is determined
 * from the global settings in @ref KStdAccel.  These values, however,
 * can be overriden locally by invoking @ref KCompletionBase::setKeyBinding().
 * The values can easily be reverted back to the default setting, by simply
 * calling @ref useGlobalSettings(). An alternate method would be to default
 * individual key-bindings by usning @ref setKeyBinding() with the default
 * second argument.
 *
 * NOTE that if the @p EchoMode for this widget is set to something other
 * than @p QLineEdit::Normal, the completion mode will always be defaulted
 * to @ref KGlobalSettings::CompletionNone.  This is done purposefully to guard
 * against protected entries such as passwords being cached in @ref KCompletion's
 * list. Hence, if the @p EchoMode is not QLineEdit::Normal, the completion
 * mode is automatically disabled.
 *
 * @sect Usage
 *
 * To enable the basic completion feature :
 *
 * <pre>
 * KLineEdit *edit = new KLineEdit( this, "mywidget" );
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
 * Note that you have to either delete the allocated completion object
 * when you don't need it anymore, or call
 * setAutoDeleteCompletionObject( true );
 *
 * @sect Miscellaneous function calls :
 *
 * <pre>
 * // Tell the widget not to handle completion and
 * // iteration internally.
 * edit->setHandleSignals( false );
 * // Set your own completion key for manual completions.
 * edit->setKeyBinding( KCompletionBase::TextCompletion, Qt::End );
 * // Hide the context (popup) menu
 * edit->setContextMenuEnabled( false );
 * // Temporarly disable signal emitions
 * // (both completion & iteration signals)
 * edit->disableSignals();
 * // Default the key-bindings to system settings.
 * edit->useGlobalKeyBindings();
 * </pre>
 *
 * @short An enhanced single line input widget.
 * @author Dawit Alemayehu <adawit@kde.org>
 */

class KLineEdit : public QLineEdit, public KCompletionBase
{
    friend class KComboBox;

    Q_OBJECT
    Q_PROPERTY( bool contextMenuEnabled READ isContextMenuEnabled WRITE setContextMenuEnabled )
    Q_PROPERTY( bool urlDropsEnabled READ isURLDropsEnabled WRITE setURLDropsEnabled )

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
     * @param string Text to be shown in the edit widget.
     * @param parent The parent object of this widget.
     * @param name The name of this widget.
     */
    KLineEdit ( QWidget *parent=0, const char *name=0 );

    /**
     *  Destructor.
     */
    virtual ~KLineEdit ();

    /**
     * Sets @p url into the lineedit. It uses @ref KURL::prettyURL() so
     * that the url is properly decoded for displaying.
     */
    void setURL( const KURL& url );

    /**
     * Puts the text cursor at the end of the string.
     *
     * This method is deprecated.  Use @ref QLineEdit::end()
     * instead.
     *
     * @deprecated
     * @ref QLineEdit::end()
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
    * Note that when this function is invoked with its argument
    * set to @p true, then both the context menu and the completion
    * menu item are enabled.  If you do not want to the completion
    * item to be visible simply invoke @ref hideModechanger() right
    * after calling this method.  Also by default, the context
    * menu is automatically created if this widget is editable. Thus
    * you need to call this function with the argument set to false
    * if you do not want this behaviour.
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
     * behaviour of QLineEdit is used, which inserts the encoded URL.
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
     * the @ref returnPressed() signals, but it also lets the event pass,
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
     * Enables text update if and only if the new text is different
     * from the current one.  Otherwise, the call to setText is ignored.
     *
     * @param true enable smart text update, false otherwise.
     */
    void setEnableSmartTextUpdate (bool);
    
    /**
     * Re-implemented for internal reasons.  API not affected.
     *
     * @reimplemented
     */
    virtual bool eventFilter( QObject *, QEvent * );

    /**
     * @returns the completion-box, that is used in completion mode
     * @ref KGlobalSettings::CompletionPopup.
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
    
signals:

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
     * Emitted when the text rotation key-bindings are pressed.
     *
     * The argument indicates which key-binding was pressed.
     * In KLineEdit's case this can be either one of two values:
     * @ref PrevCompletionMatch or @ref NextCompletionMatch. See
     * @ref KCompletionBase::setKeyBinding for details.
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
     * @param the context menu about to be displayed
     */
    void aboutToShowContextMenu( QPopupMenu* );

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
    void rotateText( KCompletionBase::KeyBindingType /* type */ );

    /**
     * See @ref KCompletionBase::setCompletedText.
     */
    virtual void setCompletedText( const QString& );

    /**
     * Sets @p items into the completion-box if @ref completionMode() is
     * CompletionPopup. The popup will be shown immediately.
     */
    void setCompletedItems( const QStringList& items );

    /**
     * Reimplemented to workaround a buggy QLineEdit::clear()
     * (changing the clipboard to the text we just had in the lineedit)
     */
    virtual void clear();
    
    /**
     * Squeezes @p text into the line edit. 
     * This can only be used with read-only line-edits.
     */
    void setSqueezedText( const QString &text);
    
    /**
     * Re-implemented for internal reasons.  API is unaffected.
     */
    virtual void setText (const QString &text);

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

protected:

    /**
    * Re-implemented for internal reasons.  API not affected.
    *
    * See @ref QLineEdit::resizeEvent().
    */
    virtual void resizeEvent( QResizeEvent * );

    /**
    * Re-implemented for internal reasons.  API not affected.
    *
    * See @ref QLineEdit::keyPressEvent().
    */
    virtual void keyPressEvent( QKeyEvent * );

    /**
    * Re-implemented for internal reasons.  API not affected.
    *
    * See @ref QLineEdit::mousePressEvent().
    */
    virtual void mousePressEvent( QMouseEvent * );

    /**
    * Re-implemented for internal reasons.  API not affected.
    *
    * See @ref QWidget::mouseDoubleClickEvent().
    */
    virtual void mouseDoubleClickEvent( QMouseEvent * );

    /**
    * Re-implemented for internal reasons.  API not affected.
    *
    * See @ref QLineEdit::createPopupMenu().
    */
    virtual QPopupMenu *createPopupMenu();

    /**
    * Re-implemented to handle URI drops.
    *
    * See @ref QLineEdit::dropEvent().
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
     * Reimplemented for internal reasons, the API is not affected.
     */
    virtual void create( WId = 0, bool initializeWindow = true,
                         bool destroyOldWindow = true );

private slots:
    void completionMenuActivated( int id );
    void tripleClickTimeout();  // resets possibleTripleClick

private:
    // Constants that represent the ID's of the popup menu.
    // TODO: See if we can replace this mess with KActionMenu
    // in the future though it's working lovely.
    enum MenuID {
        Default = 42,
        NoCompletion,
        AutoCompletion,
        ShellCompletion,
        PopupCompletion,
        SemiAutoCompletion
    };

    /**
     * Initializes variables.  Called from the constructors.
     */
    void init();

    /**
     * Creates the completion box
     */
    void makeCompletionBox();

    /**
     * Checks whether we should/should not consume a key used as
     * an accelerator.
     */
    bool overrideAccel (const QKeyEvent* e);

    bool m_bEnableMenu;

    bool possibleTripleClick;  // set in mousePressEvent, deleted in tripleClickTimeout

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KLineEditPrivate;
    KLineEditPrivate *d;
};

#endif
