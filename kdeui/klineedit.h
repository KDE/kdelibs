/*  This file is part of the KDE libraries

    This class was originally inspired by Torben Weis'
    fileentry.cpp for KFM II.

    Copyright (C) 1997 Sven Radej <sven.radej@iname.com>
    Copyright (c) 1999 Patrick Ward <PAT_WARD@HP-USA-om5.om.hp.com>
    Copyright (c) 1999 Preston Brown <pbrown@kde.org>

    Completely re-designed:
    Copyright (c) 2000 Dawit Alemayehu <adawit@kde.org>

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

#include <qpopupmenu.h>
#include <qlineedit.h>

#include <kcompletion.h>

/**
 * An enhanced QLineEdit widget for inputting text.
 *
 * This widget inherits from QLineEdit and implements the following
 * additional functionalities: a completion object that provides both
 * automatic and manual text completion as well as multiple match iteration
 * features, configurable key-bindings to activate these features and a
 * popup-menu item that can be used to allow the user to set text completion
 * modes on the fly based on their preference.
 *
 * To support these new features KLineEdit also emits a few more additional
 * signals.  These are: @ref completion, @ref previousMatch, @ref nextMatch
 * and @ref returnPressed.  The completion signal can be connected to a slot
 * that will assist the user in filling out the remaining text.  The two
 * iteration signals are intended to be used to iterate through the list of all
 * possible matches whenever there is more than one match for the given text.
 * The @p returnPressed signal is the same as QLineEdit's except it provides the
 * current text in the widget as its argument whenever appropriate.
 *
 * This widget by default creates a completion object when you invoke the
 * @ref completionObject member function for the first time or use
 * @ref setCompletionObject to assign your own completion object.  Additionally,
 * to make this widget more functional, KLineEdit will automatically handle the
 * iteration and completion signals internally when a completion object is created
 * through either one of the methods mentioned above.  If you do not need these
 * feature, simply use @ref KCompletionBase::setHandleSignals or alternatively the
 * boolean parameter in @p setCompletionObject, to turn them off.
 *
 * The default key-binding for completion and rotation is determined from the
 * global settings in @ref KStdAccel.  These values, however, can be set locally to
 * override the global settings through the @ref setKeyBinding member function.  To
 * default the values back, simply invoke @ref useGlobalSettings.  You can also default
 * individual key-binding by simply calling the @p setKeyBinding member function with
 * the default the second argument.
 *
 * NOTE: if the EchoMode for this widget is set to something other than @p QLineEdit::Normal,
 * the completion mode will always be defaulted to @ref KGlobal::CompletionNone.  This
 * is done purposefully to guard against protected entries such as passwords being cached
 * in KCompletion's list. Hence, if the EchoMode is not QLineEdit::Normal, the completion
 * mode is automatically disabled.
 *
 * @sect Examples:
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
	Q_OBJECT
	Q_PROPERTY( bool contextMenuEnabled READ isContextMenuEnabled WRITE setContextMenuEnabled )

public:

    /**
    * Constructs a KLineEdit object with a default text, a parent,
    * and a name.
    *
    * @param string text to be shown in the edit widget
    * @param parent the parent object of this widget
    * @param name the name of this widget
    */
    KLineEdit( const QString &string, QWidget *parent, const char *name = 0 );

    /**
    * Constructs a KLineEdit object with a parent and a name.
    *
    * @param string text to be shown in the edit widget
    * @param parent the parent object of this widget
    * @param name the name of this widget
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
    * Note that when this function is invoked with its argument
    * set to @p true, then both the context menu and the completion
    * menu item are enabled.  If you do not want to the completion
    * item to be visible simply invoke @ref hideModechanger() right
    * after calling this method.  Also by default, the context
    * menu is automatically created if this widget is editable. Thus
    * you need to call this function with the argument set to false
    * if you do not want this behaviour.
    *
    * @param showMenu if true, show the context menu.
    */
    virtual void setContextMenuEnabled( bool showMenu ) {  m_bEnableMenu = showMenu; }

    /**
    * Returns true when the context menu is enabled.
    *
    * @return @p true if context menu is enabled.
    */
    bool isContextMenuEnabled() const { return m_bEnableMenu; }
    
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
     * @see #trapReturnKey
     */
    void setTrapReturnKey( bool grab );

    /**
     * @returns true if keyevents of Key_Return or Key_Enter will be stopped
     * or if they will be propagated.
     *
     * @see #setTrapReturnKey
     */
    bool trapReturnKey() const;   

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
    * Emitted when the text rotation key-bindings are pressed.
    *
    * The argument indicates which key-binding was pressed.
    * In KLineEdit's case this can be either one of two values:
    * PrevCompletionMatch or NextCompletionMatch. See
    * @ref KCompletionBase::setKeyBinding for details.
    *
    * Note that this signal is NOT emitted if the completion
    * mode is set to CompletionNone or EchoMode is NOT normal.
    */    
    void textRotation( KCompletionBase::KeyBindingType );

public slots:

    /**
    * Iterates through all possible matches of the completed text or
    * the history list.
    *
    * This function simply iterates over all possible matches in case
    * multimple matches are found as a result of a text completion request.
    * It will have no effect if only a single match is found.
    *
    * @param type the key-binding invoked.
    */
    void rotateText( KCompletionBase::KeyBindingType /* type */ );

protected slots:

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
    * @reimplemented
    */
    virtual bool eventFilter( QObject *, QEvent * );    

private:

    // Constants that represent the ID's of the popup menu.
    // TODO: See if we can replace this mess with KActionMenu
    // in the future though this is working lovely.
    enum MenuID {
        Default=0,
        Cut,
        Copy,
        Paste,
        Clear,
        Unselect,
        SelectAll,
        NoCompletion,
        AutoCompletion,
        ShellCompletion,
        SemiAutoCompletion	
    };
    
    // Indicates whether the context
    // menu is enabled or disabled
    bool m_bEnableMenu;

    class KLineEditPrivate;
    KLineEditPrivate *d;
};

#endif
