/*  This file is part of the KDE libraries

    This class was originally inspired by Torben Weis'
    fileentry.cpp for KFM II.
    Copyright (C) 1997 Sven Radej <sven.radej@iname.com>
    Copyright (c) 1999 Patrick Ward <PAT_WARD@HP-USA-om5.om.hp.com>
    Copyright (c) 1999 Preston Brown <pbrown@kde.org>

    Re-designed with addional functionality:
    Copyright (c) 1999-2000 Dawit Alemayehu <adawit@earthlink.net>

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
 * This widget has the same behaviour as QLineEdit with the following
 * added functionalities : a popup menu that provides basic features
 * such as copy/cut/paste to manipulate content through the mouse, a
 * built-in hook into @ref KCompletion which provides automatic & manual
 * completion as well as iteration through a given list, and the ability
 * to change which keyboard keys to use for these features.  Since this
 * widget inherits form QLineEdit, it can be used as a drop-in replacement
 * where the above extra functionalities are needed and/or useful.
 *
 * KLineEdit emits a few more additional signals than @ref QLineEdit:
 * @ref completion, @ref rotateUp and @ref rotateDown and @returnPressed.
 * The completion signal can be connected to a slot that will assist the
 * user in filling out the remaining text.  The two rotation signals are
 * intended to be used to iterate through a list of predefined text entries.
 *
 * By default both the completion and rotation signals are handled by this
 * widget.  If you do not want this widget to automatically handle these
 * signals, simply use the appropriate mutator methods to shut them off.
 * See @ref setHandleCompletion and @ref setHandleRotation for details.
 *
 * The default key-bindings for completion and rotation are determined
 * from the global settings in @ref KStdAccel.  However, these values can
 * be set locally overriding the global settings.  Simply invoking @see
 * useGlobalSettings allows you to immediately default the bindings back to
 * the global settings again.  Also if you are interested in only defaulting
 * the key-bindings individually for each action, simply call the setXXXKey
 * methods without any argumet.  For example, after locally customizing the
 * key-binding that invokes manual completion, simply invoking @see
 * setCompletionKey(), without any argument, will result in the completion
 * key being set to 0. This will then force the key-event filter to use the
 * global value.
 *
 * @sect Examples:
 *
 * Basic completion & rotation features enabled:
 *
 * <pre>
 * // Make sure the boolean paramter is set to true.  Otherwise,
 * // you have to turn on each feature manually. See below.
 * KLineEdit *edit = new KLineEdit( this, "mywidget", true );
 * // Insert the enteries on RETURN pressed into the completion object's list
 * connect( edit, SIGNAL( returnPressed(const QString& ) ), edit->completionObject(), SLOT( addItem( const QString& ) ) );
 * </pre>
 *
 * To use a customized completion object setCompletionObject:
 *
 * <pre>
 * KLineEdit *edit = new KLineEdit( this, "mywidget", true );
 * KURLCompletion *comp = new KURLCompletion();
 * edit->setCompletionObject( comp, false );  //KLineEdit will NOT delete the completion object.
 * // Insert the enteries on RETURN pressed into the completion object's list
 * connect( edit, SIGNAL( returnPressed( const QString& ) ), comp, SLOT( addItem( const QString& ) ) );
 * </pre>
 *
 * To enable a single feature at a time :
 *
 * <pre>
 * KLineEdit *edit = new KLineEdit( this, "mywidget" );
 * edit->setHandleCompletion( true );
 * edit->setHandleRotation( true );
 * // Insert the enteries on RETURN pressed into the completion object's list
 * connect( edit, SIGNAL( returnPressed( const QString& ) ), edit->CompletionObject(), SLOT( addItem( const QString& ) ) );
 * </pre>
 *
 * And do not forget to enable the popup menu :
 *
 * <pre>
 * edit->setEnableContextMenu( true );
 * edit->
 * </pre>
 * @short An enhanced single line input widget.
 * @author Dawit Alemayehu <adawit@earthlink.net>
 */
class KLineEdit : public QLineEdit
{
  Q_OBJECT

public:

    /**
    * Constructs a KLineEdit object with a default text, a parent,
    * and a name.
    *
    * @param @p string text to be shown in the edit widget
    * @param @p parent the parent object of this widget
    * @param @p name the name of this widget
    * @param @p hsig determines if this widget automatically handles both signals internally.
    */
    KLineEdit( const QString &string, QWidget *parent, const char *name = 0, bool hsig = true );

    /**
    * Constructs a KLineEdit object with a parent and a name.
    *
    * @param @p string text to be shown in the edit widget
    * @param @p parent the parent object of this widget
    * @param @p name the name of this widget
    * @param @p hsig determines if this widget automatically handles both signals internally.
    */
    KLineEdit ( QWidget *parent=0, const char *name=0, bool hsig = true );

    /**
    *  Destructor.
    */
    virtual ~KLineEdit ();

    /**
    * Puts cursor at the end of the string.
    * 
    * This method is deprecated.  Use @see QLineEdit::end instead.
    *
    * @deprecated
    * @see QLineEdit::end
    */
    void cursorAtEnd() { end( false ); }

    /**
    * Sets the @ref KCompletion object this widget will use.
    *
    * This function allows you to set the completion object to be used by
    * this widget.  It also enables you to control how this completion object
    * will be handled by this widget (see below).
    *
    * The completion object assigned using method is by default NOT deleted when
    * this widget is destroyed.  If you want KLineEdit to delete this object when
    * its destructor is called, be sure to set the boolean parameter, autoDelete,
    * to "true".  This is especially usefully if you want to share one completion
    * object across mulitple widgets.  You can also use the member functions
    * @ref setDeleteCompletionOnExit and @ref deleteCompletionOnExit to change this
    * value.  This widget's reference to a completion object by either deleting the
    * object manually yourself or calling this function with the first argument set
    * to null - setCompletionObject( 0 ). Be aware, however, that doing so will stop
    * this widget from automatically handling completion and rotation signals even if
    * you have explicitly called the setHandleXXXX methods.
    *
    * @param a @ref KCompletion or a derived child object.
    * @param @p autoDelete if true, the completion object is deleted in the destructor.
    */
    void setCompletionObject( KCompletion*,  bool autoDelete = false );

    /**
    * Returns true if the completion object is deleted upon this widget's
    * destruction.
    *
    * See also @ref setCompeltionObject, @ref setHandleCompletion,
    * @ref deleteCompletionOnExit and @ref setDeleteCompletionOnExit.
    *
    * @return true if the completion object is deleted
    */
    bool deleteCompletionObject() const { return m_bAutoDelCompObj; }

    /**
    * Sets the completion object for deletion upon this widget's destruction.
    *
    * If the argument is set to true, the completion object is deleted when
    * this widget's destructor is called.  Be careful not to set this to true
    * if you want to re-use the completion objects in your own code.
    *
    * @param @p autoDelete if set to true the completion object is deleted on exit.
    */
    void setDeleteCompletionObject( bool autoDelete ) { m_bAutoDelCompObj = autoDelete; }

    /**
    * Returns a pointer to the current completion object.
    *
    * @return the completion object or null if one does not exist.
    */
    KCompletion* completionObject() const { return m_pCompObj; }

    /**
    * Enables/disables this widget's ability to emit completion signals.
    *
    * Note that if you invoke this function with the argument set to false,
    * no completion signals will be emitted.  Thus, this widget will not be
    * able to handle the completion signals even if setHandleCompletion has
    * been or is called.  Also note that disabling the emition of the
    * completion signals through this method does NOT delete the comlpetion
    * object if one has already been created.  See also @ref setHandleCompletion
    * and @ref setHandleCompletion.
    *
    * @param @p emit if true emits completion signal.
    */
    void setEnableCompletionSignal( bool enable ) { m_bEmitCompletion = enable; }

    /**
    * Enables/disables this widget's ability to emit rotation signals.
    *
    * Note that if you invoke this function with the argument se to false,
    * no rotation signals will be emitted.   Thus, this widget will not be
    * able to handle the rotation signals even if @ref setHandleRotation has
    * been or is called.  See also @see setHandleRotation.
    */
    void setEnableRotationSignal( bool enable ) { m_bEmitRotation = enable; }

    /**
    * Sets this widget to handle the completion signals internally.
    *
    * When this function is invoked with the argument set to "true", KComboBox
    * will automatically handle rotation signals.  To stop KLineEdit from
    * handling the comepltion signal internally simply invoke this function with
    * with the argument set to "false".
    *
    * Note that calling this function does not hinder you from connecting and
    * hence receiving the completion signals externally.
    *
    * @param @p complete when true enables this widget to handle completion.
    */
    void setHandleCompletion( bool complete );

    /**
    * Sets this widget to handle rotation signals internally.
    *
    * When this function is invoked with the argument set to "true", KComboBox
    * will automatically handle rotation signals.  To stop KLineEdit from
    * handling the rotation signal internally simply invoke this function with
    * with the argument set to "false".
    *
    * Note that calling this function does not hinder you from connecting and
    * hence receiving the rotation signals externally.
    *
    * @param @p autoHandle when true handle rotation signals internally.
    */
    void setHandleRotation( bool rotate );

    /**
    * Returns true if this widget handles completion signal internally.
    *
    * @return true when this widget handles completion signal.
    */
    bool handlesCompletion() const { return m_bHandleCompletion; }

    /**
    * Returns true if this widget handles rotation signal internally.
    *
    * @return true when this widget handles rotation signal.
    */
    bool handlesRotation() const { return m_bHandleRotation; }

    /**
    * Set the type of completion to be used.
    *
    * The completion modes supported are those defined in @ref KGlobal.
    * These completion types are CompletionNone, CompletionAuto,
    * CompletionMan, and CompletionShell.
    *
    * @param mode Completion type:
    *        @li CompletionNone  - Disables all completion features.
    *        @li CompletionAuto  - Attempts to find a match and fill-in
    *                              the remaining text.
    *        @li CompletionMan   - Acts the same way as "CompletionAuto"
    *                              except the action has to be triggered
    *                              using the pre-defined completion key.
    *        @li CompletionShell - Attempts to mimic the completion feature
    *                              found in typcial *nix shell enviornments.
    *
    * Please note that if the EchoMode for this widget is set to something
    * other than @ref QLineEdit::Normal, this method refuses to accept the
    * new completion mode.  This is done purposefully to protect against
    * protected entries such as passwords being cached in KCompletion's list.
    * Hence, if the EchoMode is not QLineEdit::Normal, the completion mode is
    * automatically "soft" (the completion object is not deleted) disabled.
    */
    virtual void setCompletionMode( KGlobal::Completion mode );

    /**
    * Retrieve the current completion mode.
    *
    * The return values are of type @ref KGlobal::Completion.  See
    * @ref setCompletionMode for details.
    *
    * @return the completion mode.
    */
    KGlobal::Completion completionMode() const { return m_iCompletionMode; }

    /**
    * Set the key-binding to be used for the two manual completion types:
    * CompletionMan and CompletionShell.
    *
    * This function expects the value of the modifier key(s) (Shift, Ctrl, Alt),
    * if present, to be @bf SUMMED up with actual key, ex: Qt::CTRL + Qt::Key_E.
    * If no value is supplied for @p ckey or it is set to 0, then the completion
    * key will be defaulted to the global setting.  This function returns true if
    * the supplied key-binding can be successfully assigned.
    *
    * NOTE: if @p ckey is negative or the key-binding conflicts with either
    * @ref completion or @ref rotateDown keys, this function will return false.
    *
    * @param @p ckey Key binding to use for completion.  Default is 0.
    * @return @p true if key-binding can be successfully set.
    */
    bool setCompletionKey( int ckey = 0 );

    /**
    * Set the key-binding to be used for rotating through a list to find the
    * next match.
    *
    * When this key is activated by the user a @ref rotateDown signal will be
    * emitted.  If no value is supplied for @p rDnkey or it is set to 0, then
    * the completion key will be defaulted to the global setting.  This method
    * returns false if @p rDnkey is negative or the supplied key-binding
    * conflicts with either @ref completion or @ref rotateUp keys.
    *
    * @param @p rDnkey the key-binding to use for rotating up in a list.
    * @return @p true if key-binding can successfully be set.
    */
    bool setRotateDownKey( int rDnKey = 0 );

    /**
    * Sets the key-binding to be used for rotating through a list to find the
    * previous match.
    *
    * When this key is activated by the user a @ref rotateUp signal will be
    * emitted.  If no value is supplied for @p rUpkey or it is set to 0, then
    * the completion key will be defaulted to the global setting.  This method
    * returns false if @p rUpkey is negative or the supplied key-binding
    * conflicts with either @ref completion or @ref rotateDown keys.
    *
    * @param @p rUpkey the key-binding to use for rotating down in a list.
    * @return @p true if key-binding can successfully be set.
    */
    bool setRotateUpKey( int rUpKey = 0 );

    /**
    * Returns the key-binding used for completion.
    *
    * If the key binding contains modifier key(s), the @bf sum of the key and
    * the modifier will be returned. See also @see setCompletionKey.
    *
    * @return the key-binding used for rotating through a list.
    */
    int completionKey() const { return m_iCompletionKey; }

    /**
    * Returns the key-binding used for rotating up in a list.
    *
    * This methods returns the key used to iterate through a list in the
    * "UP" direction.  This is opposite to what the @ref rotateDown key
    * does.
    *
    * If the key binding contains modifier key(s), the SUM of their values
    * is returned.  See also @ref setRotateUpKey.
    *
    * @return the key-binding used for rotating up in a list.
    */
    int rotateUpKey() const { return m_iRotateUpKey; }

    /**
    * Returns the key-binding used for rotating down in a list.
    *
    * This methods returns the key used to iterate through a list in the
    * "DOWN" direction.  This is opposite to what the @ref rotateDown key
    * does.
    *
    * If the key binding contains modifier key(s), the SUM of their values
    * is returned.  See also @ref setRotateDownKey.
    *
    * @return the key-binding used for rotating down in a list.
    */
    int rotateDownKey() const { return m_iRotateDnKey; }

    /**
    * Sets this widget to use global values for key-bindings.
    *
    * This method forces this widget to check the global key- bindings for
    * the completion and rotation features each time it processes a key event.
    * Thus, allowing this widget to immediately reflect any changes made to
    * the global settings.
    *
    * By default this widget uses the global key-bindings.  There is no need
    * to call this method unless you have locally modified the key bindings
    * and want to revert back.
    */
    void useGlobalSettings();

    /**
    * Enables/disables the popup (context) menu.
    *
    * This method also allows you to enable/disable the context menu. If this
    * method is invoked without an argument, the context menu will be disabled.
    * By default the mode changer is visible when context menu is enabled.
    * Use either hideModechanger() or call this function with the second argument set
    * to "false" if you do not want that item to be inserted.  Please note that the
    * @p showChanger flag is only used when a context menu is created.  That is it is
    * only used when you invoke this function for the first time or when you disable
    * and then subsequently  re-enable the context menu.  To show or hide the mode changer
    * item under other circumstances use @ref showModeChanger and @ref hideModeChanger.
    *
    * @param @p showMenu if true, shows the context menu.
    * @param @p showModeChanger if true, shows the mode changer item in popup menu.
    */
    virtual void setEnableContextMenu( bool showMenu = true, bool showChanger = true );

    /**
    * Makes the completion mode changer visible in the context menu.
    *
    * This function allows you to show the completion mode changer, thus, enabling
    * the user to change the comepltion mode on the fly.
    */
    void showModeChanger() { m_bShowModeChanger = true; }

    /**
    * Hides the completion mode changer in the context menu.
    */
    void hideModeChanger() { m_bShowModeChanger = false; }

    /**
    * Returns true when the context menu is enabled.
    *
    * @return @p true if context menu is enabled.
    */
    bool isContextMenuEnabled() const { return m_bEnableMenu; }

    /**
    * Returns true if the mode changer item is visible in
    * the context menu.
    *
    * @return @p true if the mode changer is visible in context menu.
    */
    bool isModeChangerVisible() const { return m_bShowModeChanger; }

signals:

    /**
    * This signal is emitted when the user presses the return key.  The
    * argument is the current text.  Note that this signal is NOT emitted
    * if the widget's EchoMode is set to QLineEdit::Password.
    */
    void returnPressed( const QString& );

    /**
    * Signal emitted when the completion key is pressed.
    *
    * Please note that this signal is NOT emitted if the completion
    * mode is set to CompletionNone or EchoMode is NOT normal.
    */
    void completion( const QString& );

    /**
    * Signal emitted when the rotate up key is pressed.
    *
    * Please note that this signal is NOT emitted if the completion
    * mode is set to CompletionNone or EchoMode is NOT normal.
    */
    void rotateUp();

    /**
    * Signal emitted when the rotate down key is pressed.
    *
    * Please note that this signal is NOT emitted if the completion
    * mode is set to CompletionNone or EchoMode is NOT normal.
    */
    void rotateDown();

public slots:

    /*
    * Iterates in the up (previous match) direction through the
    * completion list if it is available.
    *
    * This slot is intended to make it easy to connect the rotate
    * up signal in order to make the widget itself handle rotation
    * events internally.  Note that no action is taken if there is
    * no completion object or the completion object does not contain
    * a next match.
    */
    virtual void iterateUpInList();

    /*
    * Iterates in the down (next match) direction through the
    * completion list if it is available.
    *
    * This slot is intended to make it easy to connect the rotate
    * down signal in order to make the widget itself handle rotation
    * events internally.  Note that no action is taken if there is
    * no completion object or the completion object does not contain
    * a next match.
    */
    virtual void iterateDownInList();


protected slots:

    /**
    * Copies the marked text to the clipboard, if there is any,
    * and if echoMode() is Normal.  See also @ref QLineEdit::copy.
    */
    virtual void slotCopy()       { copy(); }

    /**
    * Copies the marked text to the clipboard and deletes it
    * if there is any.  See also @ref QLineEdit::cut.
    */
    virtual void slotCut()        { cut(); }

    /**
    * Inserts the text in the clipboard at the current cursor position,
    * deleting any previously marked text. See also @ref QLineEdit::paste.
    */
    virtual void slotPaste()      { paste(); }

    /**
    * Sets the comepltion mode to KGlobal::CompletionNone
    */
    virtual void modeNone()   { setCompletionMode( KGlobal::CompletionNone ); }

    /**
    * Sets the comepltion mode to KGlobal::CompletionManual
    */
    virtual void modeManual() { setCompletionMode( KGlobal::CompletionMan );  }

    /**
    * Sets the comepltion mode to KGlobal::CompletionAuto
    */
    virtual void modeAuto()   { setCompletionMode( KGlobal::CompletionAuto ); }

    /**
    * Sets the comepltion mode to KGlobal::CompletionShell
    */
    virtual void modeShell()  { setCompletionMode( KGlobal::CompletionShell );}

    /**
    * Populates the context menu before it is displayed.
    */
    virtual void aboutToShowMenu();

    /**
    * Populates the sub menu before it is displayed.
    */
    virtual void aboutToShowSubMenu( int );

    /**
    * Deals with text changes and auto completion in this
    * widget.
    */
    virtual void entryChanged( const QString& );

    /**
    * Fills in the remaining text.
    */
    virtual void makeCompletion( const QString& );

    /*
    * Resets the completion object if it is deleted externally.
    */
    void completionDestroyed() { m_pCompObj = 0; }

    /*
    * Re-emitts the returnPressed signal with the current
    * text as its argument.
    */
    void slotReturnPressed();

protected:

    /**
    * Initializes variables.  Called from the constructors.
    */
    virtual void init( bool );

    /*
    * Re-implemented from QLineEdit to filter key-events.
    */
    virtual void keyPressEvent( QKeyEvent * );

    /*
    * Re-implemented from QLineEdit to filter key-events.
    */
    virtual void mousePressEvent( QMouseEvent * );

    /*
    * Rotates the text on rotation events
    */
    void rotateText( const QString& );

    // Pointers to the context & sub menus.
    QPopupMenu *m_pContextMenu, *m_pSubMenu;
    // Pointer to the Completion object.
    KCompletion *m_pCompObj;

private :
    // Stores the completion key locally
    int m_iCompletionKey;
    // Stores the Rotate up key locally
    int m_iRotateUpKey;
    // stores the Rotate down key locally
    int m_iRotateDnKey;
    // Holds the location where the Mode
    // switcher item was inserted.
    int m_iSubMenuId;
    // Holds the length of the entry.
    int m_iPrevlen;
    // Holds the current cursor position.
    int m_iPrevpos;

    // Indicates whether the context menu is enabled
    // or disabled
    bool m_bEnableMenu;
    // Indicates whether the mode switcher item will be
    // available in the context (popup) menu.
    bool m_bShowModeChanger;
    // Determines whether the completion object should be
    // deleted when this widget is destroyed.
    bool m_bAutoDelCompObj;
    // Determines whether this widget handles rotation.
    bool m_bHandleRotation;
    // Determines whether this widget handles completions.
    bool m_bHandleCompletion;
    // This widget emits completion if true
    bool m_bEmitCompletion;
    // This widget emits rotation if true
    bool m_bEmitRotation;

    // Stores the comepltion mode locally.
    KGlobal::Completion m_iCompletionMode;
};

#endif
