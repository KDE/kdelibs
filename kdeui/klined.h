/*  This file is part of the KDE libraries

    This class was originally inspired by Torben Weis'
    fileentry.cpp for KFM II.
    Copyright (C) 1997 Sven Radej <sven.radej@iname.com>
    Copyright (c) 1999 Patrick Ward <PAT_WARD@HP-USA-om5.om.hp.com>
    Copyright (c) 1999 Preston Brown <pbrown@kde.org>
    Copyright (c) 1999 Dawit Alemayehu <adawit@earthlink.net>

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

#ifndef _KLINED_H
#define _KLINED_H

#include <qlineedit.h>
#include <qpopupmenu.h>

#include <kcompletion.h>

/**
 * This widget has the same behaviour as QLineEdit with the following
 * added functionalities : a popup menu that provides basic features
 * such as copy/cut/paste to manupliate content through the mouse, a
 * built-in hook into KCompletion which provides automatic & manual
 * completion as well as iteration through a given list, and the
 * ability to change which keyboard keys to use for these features.
 * Since this widget inherits form QLineEdit, it can be used as a
 * drop-in replacement where the above extra functionalities are
 * needed/useful.
 *
 * KLineEdit emits three more additional signals than QLineEdit: @ref
 * completion and @ref rotateUp and @ref rotateDown. The completion
 * signal can be connected to a slot that will assist the user in
 * filling out the remaining text.  The two rotation signals are used
 * to iterate through a list of predefined text entries in opposite
 * direction, i.e. rotateUp will cause an iteration in reverse of what
 * a rotateDown signal would.
 *
 * The default key-bindings for completion and rotation are determined
 * from the global settings in KStdAccel.  However, these values can
 * be set locally overriding the global settings.  Simply invoking
 * @see useGlobalSettings() allows you to immediately default the the
 * bindings back to the global settings again.  Also if you are interested
 * in only defaulting the key-bindings individually for each action, simply
 * call the setXXXKey( int ) methods without any argumet.  For example, after
 * locally customizing the key-binding that invokes completion in manual
 * modes, simply invoking setCompletionKey() will result in the completion
 * key being set to 0. This will then force the key-event filter to use the
 * global values.
 *
 * A small example:
 *
 * To use the new completion features :
 *
 * <pre>
 * KLineEdit *edit = new KLineEdit( this,"mywidget" );
 * edit->enableCompletion();
 * </pre>
 *
 * To use this widget like it was in KDE 1.1.x, simply do not
 * enable the completion object and connect to the necessary
 * signals.
 *
 * @short An enhanced single line input widget.
 * @author Dawit Alemayehu <adawit@earthlink.net>
 */
class KLineEdit : public QLineEdit
{
  Q_OBJECT

public:

    /**
    * Constructors a KLineEdit object with the given
    * parameters.
    *
    * @param <tt>string</tt> the text to be set in the editor.
    * @param <tt>parent</tt> the parent object that owns this widget.
    * @param <tt>name</tt> the name of the widget.
    * @param <tt>showMenu</tt> determines whether a context menu should be shown.
    * @param <tt>showChanger</tt> determines whether the mode changer item should be shown.
    */
    KLineEdit( const QString &string,
               QWidget *parent,
               const char *name = 0,
               bool showMenu = true,
               bool showChanger = true );
    /**
    * Constructors a KLineEdit object with the given
    * parameters.
    *
    * @param <tt>parent</tt> the parent object that owns this widget.
    * @param <tt>name</tt> the name of the widget.
    * @param <tt>showMenu</tt> determines whether a context menu should be shown.
    * @param <tt>showChanger</tt> determines whether the mode changer item should be shown.
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
    * This puts cursor at and of string. When using out of toolbar,
    * Call this in your slot connected to signal completion if necessary.
    * This method is deprecated.  Please use @see QLineEdit::end( bool )
    * instead.
    *
    * @deprecated
    * @see QLineEdit::end( bool )
    */
    void cursorAtEnd() { end( false ); }

    /**
    * Sets the internal KCompletion object managed by this widget.
    * This function allows you to set the completion object to the
    * generic one (KCompletion) or more specialized ones like
    * KURLCompletion.  Please note that you cannot set the object
    *
    * NOTE: The pointer to the KCompletion object you have supplied is
    * now managed by this widget.  Hence, it will be automatically deleted
    * when this widget's deconstructor is called.  In short, DO NOT delete
    * it yourself unless you want to crash and burn your app !!
    *
    * @param <tt>obj</tt> a KCompletion or a KCompletion derived object.
    */
    void setCompletionObject( KCompletion* obj );

    /**
    * Returns a pointer to the completion object used by this widget.
    * If the pointer is NULL, a new object is created.  Note that this
    * methods does not modify any internal variables.  If you want
    * to enable completion you have to use either @see enableCompletion()
    * or @see setCompletionObject( KCompletion* ).
    *
    * NOTE: The pointer to the KCompletion object returned by this method
    * is managed by this widget.  Hence, it will be automatically deleted
    * when this widget's deconstructor is called.  In short, DO NOT delete
    * it yourself unless you want crash and burn your app !!
    *
    * @return a pointer to the completion object.
    */
    KCompletion* completionObject() const { return (comp == 0) ? new KCompletion() : comp; }

    /**
    * Disables the completion feature of this widget.  If you were using a
    * custom completion object, you would need to set it again as this method
    * deletes any reference to it.  See @see setCompletionObject.
    *
    * @return disables the completion feature.
    */
    void disableCompletion();

    /**
    * Enables the completion feature for this widget.
    *
    * @return enables the completion feature.
    */
    void enableCompletion();

    /**
    * Sets the type of completion to be used.  The completion modes supported
    * are those defined in @see class KGlobal.  These completion types are
    * CompletionNone, CompletionAuto, CompletionMan, and CompletionShell.
    *
    * @param <tt>mode</tt> - completion type.
    *        CompletionNone  - disables all completion features.
    *        CompletionAuto  - attempts to find a match and fill-in the remaining
    *                          text.
    *        CompletionMan   - acts the same way as "CompletionAuto" except the
    *                          action has to be triggered using the pre-defined
    *                          completion key.
    *        CompletionShell - attempts to mimic the completion feature found in
    *                          typcial *nix shell enviornments.
    *
    * Please note that if the EchoMode for this widget is set to something
    * other than QLineEdit::Normal, this method refuses to accept the
    * new completion mode.  This is done purposefully to protect against
    * protected entries such as passwords being cached in KCompletion's list.
    * Hence, if the EchoMode is not QLineEdit::Normal, the completion mode is
    * automatically "soft"(the completion object is not deleted) disabled.
    */
    virtual void setCompletionMode( KGlobal::Completion mode );

    /*
    * Returns the current completion mode.  The return values are of type
    * KGlobal::Completion. See @see setCompletionMode for details.
    *
    * @return the completion mode.
    */
    KGlobal::Completion completionMode() const { return m_iCompletionMode; }

    /**
    * Sets the key-binding to be used for the two manual completion types:
    * CompletionMan and CompletionShell.  This function expects the value of
    * the modifier key(s) (SHIFT, CTRL, ALT), if present, to be SUMMED up with
    * actual key i.e. Qt::CTRL+Qt::Key_End.  If no value is supplied for
    * <tt>ckey</tt> or it is set to 0, then the completion key will be defaulted
    * to the global setting.
    *
    * Returns true if the supplied key-binding can be successfully assigned,
    * i.e. it does not conflict with the rotateUp and rotateDown key bindings.
    * NOTE: if <tt>ckey</tt> has a negative value will cause this function to
    * return false.
    *
    * @param <tt>ckey</tt> - key binding to use for completion. Default is 0.
    * @return <tt>true</tt> if key-binding can be successfully set.
    */
    bool setCompletionKey( int ckey = 0 );

    /**
    * Sets the key to be used for rotating (iterating) through a list of some
    * kind to find the next match.  When this key is activated by the user a
    * @see rotateUp() signal will be emitted.  Rotation signals are only applicable
    * when the completion mode is set to CompletionShell, since rotation is typically
    * a *nix style command shell feature.  If no value is supplied for <tt>rDnkey</tt>
    * or it is set to 0, then the completion key will be defaulted to the global setting.
    *
    * Returns true if the key and a modifier sequence can successful be
    * assigned.  That is the supplied key-binding does not conflict with
    * the bindings for rotateDown and completion.  NOTE: if <tt>rDnkey</tt>
    * has a negative value, this function will always return false.
    *
    * @param <tt>rDnkey</tt> - key-binding to use for rotating up in a list.
    * @return <tt>true</tt> if the key can sucessfully be set.
    */
    bool setRotateDownKey( int rDnKey = 0 );

    /**
    * Sets the key to be used for rotating (iterating) through a list of
    * some kind to find the previous match.  When this key is activated
    * by the user a @see rotateUp() signal will be emitted.  Rotation
    * signals are only applicable when the completion mode is set to
    * CompletionShell, since rotation is typically a *nix style command
    * shell feature.  If no value is supplied for <tt>rDnkey</tt> or it
    * is set to 0, then the completion key will be defaulted to the global
    * setting.
    *
    * Returns true if the key and modifier sequence can successful, assigned.
    * That is the supplied key-binding does not conflict with the bindings for
    * rotateDown and completion.  NOTE: if <tt>rUpkey</tt> has a negative value,
    * this function will always return false.
    *
    * @param <tt>rUpkey</tt> - the key to use for rotating up in a list.
    * @return <tt>true</tt> if the key can sucessfully be set.
    */
    bool setRotateUpKey( int rUpKey = 0 );

    /**
    * Returns the current key used for completion. If the key
    * binding contains modifier key(s), the SUM of the key and
    * the modifier will be returned. See also @see setCompletionKey.
    *
    * @return the key-binding used for rotating through a list.
    */
    int completionKey() const { return m_iCompletionKey; }

    /**
    * Returns the current key used for rotating through a list
    * in one direction - usually opposite to @ref rotateDown key.
    * If the key binding contains modifier key(s), the SUM of the
    * key and the modifier(s) will be returned.  See also
    * @see setRotateUpKey.
    *
    * @return the key-binding used for rotating through up a list.
    */
    int rotateUpKey() const { return m_iRotateUpKey; }

    /**
    * Returns the current key used for rotating through
    * a list in one direction - usually opposite to @ref
    * RotateUp key.  If the key binding contains modifier
    * key(s), the ORed value will be returned.  See also
    * @see setTotateDownKey.
    *
    * @return  the key-binding used for rotating down through a list
    */
    int rotateDownKey() const { return m_iRotateDnKey; }

    /*
    * This methods forces this widget to check the global key
    * bindings for completion and rotation each time it processes
    * a key event.  This way the widget will immediately reflect
    * the changes to the global settings.
    *
    * NOTE: By default this widget uses the global key-bindings.
    * Hence there is no need to call this method unless you have
    * locally modified the key bindings and want to change back.
    */
    void useGlobalSettings();

    /**
    * Returns true if the context menu is enabled.
    *
    * @return <tt>true</tt>if context menu is enabled.
    */
    bool isContextMenuEnabled() const { return m_bShowContextMenu; }

    /**
    * Returns true if the mode changer item in the context
    * menu is enabled.
    *
    * @return <tt>true</tt>if the mode changer is a visible item in the context menu.
    */
    bool isModeChangerEnabled() const { return m_bShowModeChanger; }

    /**
    * Show/hide the Popup menu.  This method also allows you
    * to show/hide the context or pop-up menu.  If the
    * item is visible, the user can change the comepltion mode
    * on the fly.  If this function is called without an argument,
    * the context menu will be disabled.
    *
    * @param <tt>showMenu</tt> if true, shows the context menu.
    */
    virtual void showContextMenu( bool showMenu = false );

    /**
    * Show/Hide the completion mode changer item in the context
    * menu.  Note that if completion is disabled, setting the
    * <tt>showModeChanger</tt> to <tt>true</tt> will not result
    * in the mode changer being shown in the context menu.  You
    * have to first enable completion.  See @see enableCompletion.
    * If this function is invoked without an argument, the mode
    * changer item will be hidden.
    *
    * @param <tt>showModeSwitcher</tt>if true, shows the mode changer.
    */
    virtual void showModeChanger( bool showChanger = false );

signals:

    /**
    * Connect to this signal to receive a notification when
    * the completion key, CTRL-End by default, is pressed.
    * The key used to initiate this signal can be changed
    * using @ref setCompletionKey.
    */
    void completion( const QString& );

    /**
    * Connect to this signal to receive a notification when
    * the RotateUp key is pressed.  The key used to initiate
    * this signal can be changed using @ref setRotateUpKey.
    */
    void rotateUp();

    /**
    * Connect to this signal to receive a notification when
    * the RotateDown key ( ex: Down Arrow key) is pressed.
    * The key used to initiate this signal can be changed
    * using @ref setRotateDownKey.
    */
    void rotateDown();

protected slots:
    virtual void doCopy()       { copy(); }
    virtual void doCut()        { cut(); }
    virtual void doPaste()      { paste(); }
    virtual void doClear()      { clear(); }
    virtual void doSelect()     { selectAll(); }
    virtual void doUnselect()   { deselect(); }

    virtual void modeNone()     { setCompletionMode( KGlobal::CompletionNone ); }
    virtual void modeManual()   { setCompletionMode( KGlobal::CompletionMan );  }
    virtual void modeAuto()     { setCompletionMode( KGlobal::CompletionAuto ); }
    virtual void modeShell()    { setCompletionMode( KGlobal::CompletionShell );  }


    virtual void slotShowContextMenu();
    virtual void slotShowSubMenu( int );
    virtual void slotTextChanged( const QString& );
    virtual void slotMakeCompletion( const QString& );
    virtual void slotReturnPressed();
    virtual void slotRotateUp();
    virtual void slotRotateDown();

protected:
    // Pointers for the context & sub menus.
    QPopupMenu *contextMenu, *subMenu;
    // Pointer to Completion object.
    KCompletion *comp;

    virtual void initialize( bool, bool );
    virtual void keyPressEvent( QKeyEvent * );
    virtual void mousePressEvent( QMouseEvent * );
    void rotateText( const QString& );

private :
    // Stores the completion key locally
    int m_iCompletionKey;
    // Stores the Rotate up key locally
    int m_iRotateUpKey;
    // stores the Rotate down key locally
    int m_iRotateDnKey;
    // Holds the id of where the Mode switcher
    // item is inserted.
    int subMenuID;
    // Holds the length of the entry.
    int prevlen;
    // Holds the current cursor position.
    int prevpos;

    // Flag to indicate whether we show
    // context menu or not
    bool m_bShowContextMenu;
    // Flag to indicate whether a mode switcher
    // item will be available in the popup menu.
    bool m_bShowModeChanger;
    // Flag to determine whether completion should be
    // ignored when mode is set to CompletionAuto.
    // Also used to determine whether the current text
    // needs to be inserted in KCompletion's list.
    bool m_bHasInputChanged;

    // Stores the completion mode locally.
    KGlobal::Completion m_iCompletionMode;
};

#endif
