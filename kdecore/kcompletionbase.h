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

#ifndef _KCOMPLETIONBASE_H
#define _KCOMPLETIONBASE_H

#include <kcompletion.h>

 /**
 * A base class for adding completion feature into
 * widgets.
 *
 * This is a convienence class that tries to provide
 * the common functions needed to add support for
 * completion into widgets.  Refer to @ref KLineEdit
 * or @ref KComboBox to see how to add support for
 * completion using this base class.
 *
 * @short A base class for using KCompletion in widgets
 * @author Dawit Alemayehu <adawit@earthlink.net>
 */
class KCompletionBase
{

public:

    /**
    * Basic constructor.
    */
    KCompletionBase();

    /**
    * Basic destructor.
    */
    virtual ~KCompletionBase();

    /**
    * Override this method to provide an implementation.
    *
    * This a dummy method. An implementation for setting
    * the completion object to be to the one provided by
    * the argument should be provided by the inheriting
    * child object.  The object assigned through this
    * method should not by default be deleted when the
    * this object destructor is invoked.  This behavior is
    * changeable through @ref setAutoDeleteCompletionObject.
    *
    * @param obj a @ref KCompletion or a derived child object.
    */
    virtual void setCompletionObject( KCompletion* ) {};

    /**
    * Override this method to provide an implementation.
    *
    * This is a dummy method.  An implementation for handling
    * the completion and rotation signals internally should be
    * provided by the inherting chlid object.  Note that calling
    * this function should not hinder external apps from receiving
    * these signals if they are already enabled.
    *
    * @param complete if true, handle completion & roation internally.
    */
    virtual void setHandleSignals( bool ) {};

    /**
    * Returns a pointer to the current completion object.
    *
    * If the object does not exisit, it is automatically
    * created.  Note that the completion object created
    * here is used by default to handle the signals
    * internally.  It is also deleted when this object's
    * destructor is invoked.  If you do not want these
    * default settings, use @ref setAutoDeleteCompletionObject
    * and @ref setHandleSignals to change the behavior.
    * Note that this function relies on a proper implementation
    * @ref setCompletionObject.
    *
    * @return a pointer the completion object.
    */
    KCompletion* completionObject();

    /**
    * Returns true if the completion object is deleted
    * upon this widget's destruction.
    *
    * See @ref setCompeltionObject and @ref enableCompletion
    * for details.
    *
    * @return true if the completion object
    */
    bool isCompletionObjectAutoDeleted() const { return m_bAutoDelCompObj; }

    /**
    * Sets the completion object for deletion upon this
    * widget's destruction.
    *
    * If the argument is set to true, the completion object
    * is deleted when this widget's destructor is called.
    *
    * @param autoDelete if true, delete completion object on destruction.
    */
    void setAutoDeleteCompletionObject( bool autoDelete ) { m_bAutoDelCompObj = autoDelete; }


    /**
    * Disables (temporarily) this widget's ability to emit
    * the rotation and completion signals.
    *
    * Invoking this function will cause the completion &
    * rotation signals not to be emitted.  Note that this
    * also disbales any internal handling of these signals.
    * However, unlike @ref setCompletionObject object,
    * disabling the emition of the signals through this
    * method does not delete the comlpetion object.
    */
    void disableSignals() { m_bEmitSignals = false; }

    /**
    * Enables the widget's ability to emit completion signals.
    *
    * Note that there is no need to invoke this function by
    * default.  When a completion object is created through
    * completionObject() or setCompletionObject(), these
    * signals are automatically actiavted.  Only call this
    * functions if you disbaled them manually.
    */
    void enableSignals()  { if( !m_bEmitSignals ) m_bEmitSignals = true; }

    /**
    * Returns true if the object handles the signals
    *
    * @return true if this signals are handled internally.
    */
    bool areSignalsHandled() const { return m_bHandleSignals; }

    /**
    * Sets the type of completion to be used.
    *
    * The completion modes supported are those defined in
    * @ref KGlobal.  See below.
    *
    * @param mode Completion type:
    *   @li CompletionNone:  Disables completion feature.
    *   @li CompletionAuto:  Attempts to find a match &
    *                        fills-in the remaining text.
    *   @li CompletionMan:   Acts the same as the above
    *                        except the action has to be
    *                        manually triggered through
    *                        pre-defined completion key.
    *   @li CompletionShell: Mimics the completion feature
    *                        found in typcial *nix shell
    *                        enviornments.
    */
    virtual void setCompletionMode( KGlobal::Completion mode );

    /**
    * Retrieves the current completion mode.
    *
    * The return values are of type @ref KGlobal::Completion.
    * See @ref setCompletionMode for details.
    *
    * @return the completion mode.
    */
    KGlobal::Completion completionMode() const { return m_iCompletionMode; }

    /**
    * Sets the key-binding(s) to be used for rotating through
    * a list to find the next match.
    *
    * When this key is activated by the user a @ref rotateDown
    * signal will be emitted.  If no value is supplied for
    * rDnkey or it is set to 0, then the completion key will
    * be defaulted to the global setting.  This method returns
    * false if rDnkey is negative or the supplied key-binding
    * conflicts with either @ref completion or @ref rotateUp keys.
    *
    * @param rDnkey key-binding used to rotate up in a list.
    * @return true if key-binding can successfully be set.
    */
    bool setRotateDownKey( int rDnKey = 0 );

    /**
    * Sets the key-binding to be used for rotating through a
    * list to find the previous match.
    *
    * When this key is activated by the user a @ref rotateUp
    * signal will be emitted.  If no value is supplied for
    * rUpkey or it is set to 0, then the completion key
    * will be defaulted to the global setting.  This method
    * returns false if rUpkey is negative or the supplied
    * key-binding conflicts with either @ref completion or
    * @ref rotateDown keys.
    *
    * @param rUpkey key-binding used to rotate down in a list.
    * @return  true if key-binding can successfully be set.
    */
    bool setRotateUpKey( int rUpKey = 0 );

    /**
    * Sets the key-binding to be used for the two manual
    * completion types: CompletionMan and CompletionShell.
    *
    * This function expects the value of the modifier keys
    * (Shift, Ctrl, Alt), if present, to be @bf summed up
    * with actual key, ex: Qt::CTRL+Qt::Key_E.  If no value
    * is supplied for  ckey or it is set to 0, then the
    * completion key will be defaulted to the global setting.
    * This function returns true if the supplied key-binding
    * can be successfully assigned.
    *
    * Note that if  ckey is negative or the key-binding
    * conflicts with either @ref completion or @ref rotateDown
    * keys, this function will return false.  This method
    * will also always returns false if the widget is not
    * editable.
    *
    * @param ckey Key binding to use for completion
    * @return true if key-binding is successfully set.
    */
    bool setCompletionKey( int ckey = 0 );

    /**
    * Returns the key-binding used for completion.
    *
    * If the key binding contains modifier key(s), the @bf sum
    * of the key and the modifier will be returned. See also
    * @ref setCompletionKey.  Note that this method is only
    * useful when this widget is editable.  Otherwise this
    * method has no meaning.
    *
    * @return the key-binding used for rotating through a list.
    */
    int completionKey() const { return m_iCompletionKey; }

    /**
    * Returns the key-binding used for rotating up in a list.
    *
    * This methods returns the key used to iterate through a
    * list in the "UP" direction.  This is opposite to what
    * the @ref rotateDown key does.
    *
    * If the key binding contains modifier key(s), the SUM of
    * their values is returned.  See also @ref setRotateUpKey.
    *
    * @return the key-binding used for rotating up in a list.
    */
    int rotateUpKey() const { return m_iRotateUpKey; }

    /**
    * Returns the key-binding used for rotating down in a list.
    *
    * This methods returns the key used to iterate through a
    * list in the "DOWN" direction.  This is opposite to what
    * the @ref rotateDown key does.
    *
    * If the key binding contains modifier key(s), the SUM of
    * their values is returned.  See also @ref setRotateDownKey.
    *
    * @return the key-binding used for rotating down in a list.
    */
    int rotateDownKey() const { return m_iRotateDnKey; }

    /**
    * Sets this object to use global values for key-bindings.
    *
    * This method changes the values of the key bindings for
    * rotation and completion features to the default values
    * provided in KGlobal.
    *
    * By default this object uses the global key-bindings.
    * There is no need to call this method unless you have
    * locally modified the key bindings and want to revert
    * back.
    */
    void useGlobalSettings();


protected:
    // Stores the completion key locally
    int m_iCompletionKey;
    // Stores the Rotate up key locally
    int m_iRotateUpKey;
    // Stores the Rotate down key locally
    int m_iRotateDnKey;

    // Flag that determined whether the completion object
    // should be deleted when this object is destroyed.
    bool m_bAutoDelCompObj;
    // Determines whether this widget handles completion signals
    // internally or not
    bool m_bHandleSignals;
    // Determines whether this widget fires rotation signals
    bool m_bEmitSignals;

    // Stores the completion mode locally.
    KGlobal::Completion m_iCompletionMode;
    // Pointer to Completion object.
    KCompletion* m_pCompObj;
};

#endif
