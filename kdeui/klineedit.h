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
 * KLineEdit emits three more additional signals than @ref QLineEdit: 
 * @ref completion, @ref rotateUp and @ref rotateDown. The completion
 * signal can be connected to a slot that will assist the user in
 * filling out the remaining text.  The two rotation signals are intended
 * to be used to iterate through a list of predefined text entries.
 * The rotateUp signal should be connected to a slot that rotates through
 * a list in opposite direction of the rotateDown signal.
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
 * @sect A small example:
 *
 * To enable a basic completion feature :
 *
 * <pre>
 * KLineEdit *edit = new KLineEdit( this,"mywidget" );
 * edit->enableCompletion();
 * KCompletion *comp = edit->completionObject();
 * // connect to either of the returnPressed signals
 * connect( edit, SIGNAL( returnPressed(const QString& ) ), comp, SLOT( addItem( const QString& ) ) );
 * </pre>
 *
 * To use a customized completion object such as KURLCompletion
 * use setCompletionObject(...) instead :
 *
 * <pre>
 * KLineEdit *edit = new KLineEdit( this,"mywidget" );
 * KURLCompletion *comp = new KURLCompletion();
 * edit->setCompletionObject( comp );
 * </pre>
 *
 * Of course setCompletionObject(...) can also be used to assign the base
 * KCompletion class as the comepltion object.  This is specailly
 * important when you share a single completion object across multiple
 * widgets.
 *
 * See @ref setCompletionObject and @ref enableCompletion for detailed
 * information.
 *
 *
 * @short An enhanced single line input widget.
 * @author Dawit Alemayehu <adawit@earthlink.net>
 */
class KLineEdit : public QLineEdit
{
  Q_OBJECT

public:
    /*
    * Constructs a KLineEdit object with a default text, a parent,
    * a name and a context menu.
    *
    * @param @p string text to be shown in the edit widget
    * @param @p parent the parent object of this widget
    * @param @p name the name of this widget
    * @param @p showMenu flag to show/hide the popup menu
    * @param @p showChanger flag to show/hide the completion mode item in popup menu
    */
    KLineEdit( const QString &string,
               QWidget *parent,
               const char *name = 0,
               bool showMenu = true,
               bool showChanger = true );
    /*
    * Constructs a KLineEdit object with a parent, a name and a
    * a context menu.
    *
    * @param @p string text to be shown in the edit widget
    * @param @p parent the parent object of this widget
    * @param @p name the name of this widget
    * @param @p showMenu flag to show/hide the popup menu
    * @param @p showChanger flag to show/hide the completion mode item in popup menu
    */
    KLineEdit ( QWidget *parent=0,
                const char *name=0,
                bool showMenu = true,
                bool showChanger = true );

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
    * This function allows you to enable the completion feature by supplying
    * your own KCompletion object.  It also enables you to control how this
    * completion object will be handled by this widget (see below).
    *
    * The completion object assigned using method is not, by default, deleted
    * when this object is destroyed.  If you want KLineEdit to handle the
    * deletion, make sure you set the the boolean parameter to true.  This
    * is done to allow you to share a single completion object across mulitple
    * widgets.  Additionally, this method automatically sets this widget to handle
    * the rotation signals.  If this is not desired, simply invokde @ref
    * setHandleRotationSignals( false ) after calling this function.
    *
    * @param @p obj a @ref KCompletion or a derived child object.
    * @param @p autoDelete if true, delete the completion object on destruction.
    */
    void setCompletionObject( KCompletion* obj, bool autoDelete = false );

    /*
    * Returns a pointer to the current completion object.
    *
    * @return the completion object or null if one does not exist.
    */
    KCompletion* completionObject() const { return m_pCompObj; }

    /*
    * Returns true if the completion object is deleted upon this widget's
    * destruction.
    *
    * See @ref setCompeltionObject and @ref enableCompletion for details.
    *
    * @return true if the completion object is deleted
    */
    bool isCompletionObjectDeleted() const { return m_bAutoDelCompObj; }

    /**
    * Disables the completion feature of this widget.
    *
    * If you were using a custom completion object, you would need to set it
    * again after calling this method as the reference to the comepltion object
    * is deleted.
    *
    * See also @see setCompletionObject.
    */
    void disableCompletion();

    /**
    * Enables a basic completion feature for this widget.
    *
    * This is a convienence method that will automatically create a completion
    * object for you.  The completion object is an instance of the base class
    * @ref KCompletion.  To make use of a more specialized completion object,
    * use @ref setCompletionObject.  Additionally, this method automatically
    * sets this widget to handle the rotation signals.  If this is not desired,
    * simply invokde @ref setHandleRotationSignals( false ) after calling this
    * function.
    *
    * @param @p autoDelete if true, delete the completion object on destruction.
    */
    void enableCompletion( bool autoDelete = true );

    /*
    * Sets this widget to handle rotation signals internally.
    *
    * When this function is invoked with a default argument or the argument
    * set to "true", KLineEdit will automatically handle rotation signals.
    * This is the default action chosen whenever the completion feature is
    * enabled through either @ref setCompletionObject or @ref enableCompletion
    * member functions.  To stop KLineEdit from handling completion signals
    * simply invoke this function with the argument set to "false".  Note that
    * this does not hinder you from connecting and receiving the rotation signals
    * externally.
    *
    * @param @p autoHandle if true, handle rotation signals.
    */
    void setHandleRotationSignals( bool autoHandle = true );

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

    /*
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
    * Returns true when the context menu is enabled.
    *
    * @return @p true if context menu is enabled.
    */
    bool isContextMenuEnabled() const { return m_bShowContextMenu; }

    /**
    * Returns true if the mode changer item is visible in
    * the context menu.
    *
    * @return @p true if the mode changer is visible in context menu.
    */
    bool isModeChangerEnabled() const { return m_bShowModeChanger; }

    /**
    * Enables/disables the popup (context) menu.
    *
    * This method also allows you to enable/disable the context menu. If this
    * method is invoked without an argument, the context menu will be disabled.
    *
    * @param @p showMenu if true, shows the context menu.
    */
    virtual void setEnabledContextMenu( bool showMenu = false );

    /**
    * Enables/disables the completion mode changer item in the context menu.
    *
    * This function allows you to enable or disable the completion mode changer
    * as needed without having to disable the popup menu.  If enabled the user
    * can change the comepltion mode on the fly.
    *
    * @param <tt>showChanger</tt>if true, shows the mode changer.
    */
    virtual void setEnabledModeChanger( bool showChanger = false );

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
    * Deals with the processing of all text completions.
    */
    virtual void makeCompletion( const QString& );

    /*
    * Re-emitts the returnPressed signal with the current
    * text as its argument.
    */
    void slotReturnPressed();

protected:

    /**
    * Initializes variables.  Called from the constructors.
    */
    virtual void initialize( bool , bool );

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
    // Holds the id of where the Mode switcher
    // item is inserted.
    int m_iSubMenuId;
    // Holds the length of the entry.
    int m_iPrevlen;
    // Holds the current cursor position.
    int m_iPrevpos;

    // Indicates whether the context menu is enabled
    // or disabled
    bool m_bShowContextMenu;
    // Indicates whether the mode switcher item will be
    // available in the context (popup) menu.
    bool m_bShowModeChanger;
    // Determines whether the completion object should be
    // deleted when this widget is destroyed.
    bool m_bAutoDelCompObj;
    // Determines whether this widget handles rotation signals
    // internally or not
    bool m_bHandleRotationSignals;

    // Stores the completion mode locally.
    KGlobal::Completion m_iCompletionMode;

};

#endif
