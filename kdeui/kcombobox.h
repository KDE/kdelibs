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

#include <kaction.h>
#include <kcompletion.h>


/**
 * A combined button, line-edit and a popup list widget.
 *
 * This widget inherits from QComboBox and enhances it with the following
 * added functionalities : a popup menu that provides basic features
 * such as copy/cut/paste to manipulate content through the mouse, a
 * built-in hook into @ref a KCompletion object which provides automatic
 * manual completion, and the ability to change which keyboard key is used
 * for this feature.  Furthermore, since it inherits form QComboBox it can
 * be used as a drop-in replacement for it where the above functionalities
 * are needed and/or useful.
 *
 * KComboBox emits a few more additional signals than @ref QComboBox, the
 * main ones being the @ref comepltion and the @ref rotation signal.  The
 * completion signal is intended to be connected to a slot that will assist
 * the user in filling out the remaining text. While the rotation signals,
 * both @ref rotateUp and @ref rotateDown, are intended to be used to transverse
 * through some kind of list in opposing directions.  The @ref returnPressed
 * signals are emitted when the user presses the return key and the reamining
 * signals, @ref clicked and @ref pressed are emitted to allow the list box
 * to be closed on a single mouse click event. Otherwise, the listbox items
 * are only selectable through a double-click.  See @ref clicked, @ref pressed
 * and @ref setSelectedItem for detials.
 *
 * The default key-binding for completion and rotation is determined from the
 * global settings in @ref KStdAccel.  However, these values can be set locally to
 * override the global settings.  Simply invoking @see useGlobalSettings then
 * allows you to immediately default the bindings back to the global settings
 * again.  You can also default the key-bindings by simply invoking the @ref setXXXKey
 * method without any argumet.
 *
 * @sect A small example:
 *
 * To enable the basic completion feature :
 *
 * <pre>
 * KComboBox *combo = new KComboBox( true, this, "mywidget" );
 *
 * combo->enableCompletion(); // enables completion and automatically deletes that object at the end.
 *
 * combo->autoHighLightItem(); // highlights items in the list box on mouse over
 *
 * KCompletion *comp = edit->completionObject();
 *
 * // Connect to the return pressed signal - optional
 * connect( combo, SIGNAL( returnPressed( const QString& ) ), combo->completionObject(), SLOT( addItem( const QString& ) );
 *
 * // Connect either clicked or pressed signal to the setSelectedItem slot to
 * // close combo-box on single-clicks.  Otherwise, it requires a double-click.
 * connect( combo, SIGNAL( clicked( int ) ), combo, SLOT( setSelectedItem( int ) ) );
 * </pre>
 *
 * To use a customized completion objects derived from KCompletion simply
 * use setCompletionObject to create the completion object instead :
 *
 * <pre>
 * KComboBox *combo = new KComboBox( this,"mywidget" );
 *
 * KURLCompletion *comp = new KURLCompletion();
 *
 * combo->setCompletionObject( comp );
 * </pre>
 *
 * Of course @ref setCompletionObject can also be used to assign the base
 * KCompletion class as the comepltion object.  This is specailly important
 * when you share a single completion object across multiple widgets.
 *
 * See @ref setCompletionObject and @ref enableCompletion for detailed
 * information.
 *
 * @short An enhanced combo box.
 * @author Dawit Alemayehu <adawit@earthlink.net>
 */
class KComboBox : public QComboBox
{
  Q_OBJECT

public:

    /**
    * Constructs a combo box widget with a parent object
    * and a name.
    *
    *
    */
    KComboBox( QWidget *parent=0, const char *name=0 );

    /**
    * Constructs a combo box widget in "select-only" or "read-write"
    * mode with a parent, a name and a context menu.
    *
    *
    *
    */
    KComboBox( bool rw, QWidget *parent=0, const char *name=0,
               bool showContext=true, bool showModeChager=true );
    /**
    * Destructor
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
    int cursorPosition() const { return (m_pEdit) ? m_pEdit->cursorPosition() : -1; }

    /**
    * Re-implemented from QComboBox.
    *
    * If true, the completion mode will be set to automatic.
    * Otherwise, it is defaulted to the gloabl setting.
    *
    * @param @p autocomplete flag to enable/disable automatic completion mode.
    */
    virtual void setAutoCompletion( bool autocomplete );

    /**
    * Re-implemented from QComboBox.
    *
    * Returns true if the current completion mode is set to automatic.
    *
    * @return true when completion mode is automatic.
    */
    bool autoCompletion() const { return m_iCompletionMode == KGlobal::CompletionAuto; }

    /**
    * Sets the @ref KCompletion object this widget will use.
    *
    * This method allows you to enable the completion feature by supplying
    * your own KCompletion object.  It provides you with control over how
    * the completion object object will be handled by this widget as well.
    *
    * The object assigned through this method, by default, is not deleted
    * when this widget is destroyed.  If you want KComboBox to handle the
    * deletion, make sure you set the flag in the parameter below to true.
    * This is done to allow you to share a single completion object across
    * multiple widgets.
    *
    * @param @p obj A @ref KCompletion or a derived child object.
    * @param @p autoDelete if true, delete the completion object on destruction.
    */
    void setCompletionObject( KCompletion* obj, bool autoDelete = false );

    /**
    * Returns a pointer to the current completion object.
    *
    * @return a pointer the completion object.
    */
    KCompletion* completionObject() const { return m_pCompObj; }

    /**
    * Returns true if the completion object is deleted upon this widget's
    * destruction.
    *
    * See @ref setCompeltionObject and @ref enableCompletion for details.
    *
    * @return true if the completion object
    */
    bool delCompletionOnExit() const { return m_bAutoDelCompObj; }

    /**
    * Disables the completion feature of this widget.
    *
    * If you use this method to disable the completion feature, it will delete
    * the object as well.  This means you have to reset the completion object
    * if you need to use it again.  For temporarily disabling completion without
    * removing the reference to the completion object, use @ref setCompletionMode
    * with KGlobal::CompletionNone as paramter.  See @ref setCompletionObject and
    * @ref setCompletionMode for details.
    */
    void disableCompletion();

    /**
    * Enables a basic completion feature for this widget.
    *
    * This is a convienence method that will automatically create a completion
    * object for you.  The completion object is an instance of the base class
    * @ref KCompletion.
    *
    * If you need to make use of a more specialized completion object, use
    * @ref setCompletionObject.  Also unlike setCompletionObject the completion
    * object created by this method will be automatically deleted when the widget
    * is destroyed.  To avoid this set the boolean paramter below to false.
    *
    * @param @p autoDelete if true, delete the completion object on destruction.
    */
    void enableCompletion( bool autoDelete = true );

    /**
    * Sets the type of completion to be used.
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
    * NOTE: if this widget is not editable i.e. it is constructed as a
    * "select-only" widget, then only two completion modes are allowed:
    * CompletionAuto and ComepltionNone.  The other modes are simply ignored.
    * The CompletionAuto mode in this case allows you to automatically select an
    * item in the list that matches the pressed key-codes.  For example, if you
    * have a list of countries, typing the first few letters of the name attempts
    * to find a match and if one is found it will be selected as the current item.
    */
    virtual void setCompletionMode( KGlobal::Completion mode );

    /**
    * Retrieves the current completion mode.
    *
    * The return values are of type @ref KGlobal::Completion. See @ref
    * setCompletionMode for details.
    *
    * Note that only two completion modes are supported if this widget is
    * not editable : CompletionNone and CompletionAuto.
    *
    * @return the completion mode.
    */
    KGlobal::Completion completionMode() const { return m_iCompletionMode; }

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
    * Sets the key-binding to be used for the two manual completion types:
    * CompletionMan and CompletionShell.
    *
    * This function expects the value of the modifier key(s) (Shift, Ctrl, Alt),
    * if present, to be @bf summed up with actual key, ex: Qt::CTRL+Qt::Key_E.
    * If no value is supplied for @p ckey or it is set to 0, then the completion
    * key will be defaulted to the global setting.  This function returns true if
    * the supplied key-binding can be successfully assigned.
    *
    * NOTE: if @p ckey is negative or the key-binding conflicts with either
    * @ref completion or @ref rotateDown keys, this function will return false.
    * Also note that this method always returns false if the widget is not editable.
    *
    * @param @p ckey Key binding to use for completion.  Default is 0.
    * @return @p true if key-binding can be successfully set.
    */
    bool setCompletionKey( int ckey = 0 );

    /**
    * Returns the key-binding used for completion.
    *
    * If the key binding contains modifier key(s), the @bf sum of the key and
    * the modifier will be returned. See also @see setCompletionKey.  Note that
    * this method is only useful when this widget is editable.  Otherwise this
    * method has no meaning.
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
    void useGlobalSettings() { m_iCompletionKey = 0; }

    /**
    * Sets items in the list box to be highlighted on a "mouse over" event.
    *
    * This is a convienence method to turn on highlighting when the mouse
    * cursor is over an item in the list box.  The same thing can be achived
    * by connecting to the signals from the list box directly.  This method
    * does all that work for you.  NOTE: if you opt for manaully connecting
    * these signals yourself, DO NOT invoke this method.  Otherwise you will
    * receive duplicate signals.
    *
    * @param @p highlight if true enables highlighting on mouse
    */
    void autoHighlightItems( bool highlight = true );

    /**
    * Enables or disables the automatic selection of an item from the list box
    * if widget is NOT editable.
    *
    * This method allows you to turn on the ability to select an item by
    * simply entering the first letter of an item when the combo-box is in
    * "select-only" mode.  This feature has no impact if the widget is
    * created as "read-write" or editable.
    *
    * @param @p select if true enables auto selection of items.
    */
    void autoSelectItems( bool select = true ) { m_bAutoSelect = select; }

    /**
    * Enables the default popup (context) menu for this widget.
    *
    * This method only works if this widget is editable, i.e. set to
    * read-write.  If this widget was constructed without being enabled
    * for read-write, invoking this function will accomplish nothing.
    * Hence, the return value will be false.
    *
    * @return true if popup menu can be created, i.e. widget is not read-only.
    */
    bool showContextMenu();

    /**
    * Disables the default popup (context) menu for this widget.
    *
    * This method only works if this widget is editable, i.e. set to
    * read-write.  If this widget was constructed without being enabled
    * for read-write, invoking this function is meaningless.
    *
    */
    void hideContextMenu();

    /**
    * Shows the completion mode changer in the context menu.
    *
    * This function allows to show the mode changer item in the
    * context menu again after you have disabled it with
    * @ref hideModeChanger.  Note that there is no need to call
    * this function unless you have disabled the mode changer when
    * you created the object, through the option in the constructor
    * or
    *
    * NOTE : this function is meaningless if this widget is "select-only".
    */
    void showModeChanger() { m_bShowModeChanger = true; }

    /**
    * Hides the completion mode changer in the context menu.
    *
    * This function allows you to hide the completion mode changer as
    * needed without completely disabling the popup menu.
    *
    * NOTE : this function is meaningless if this widget is "select-only".
    */
    void hideModeChanger() { m_bShowModeChanger = false; }

signals:

    /**
    * This signal is emitted when the user presses the return key.  It
    * is emitted if and only if the widget is editable (read-write).
    */
    void returnPressed();

    /**
    * This signal is emitted when the user presses the return key.  The
    * argument is the current text being edited.  This signal, just like
    * @ref returnPressed(), is only emitted if this widget is editable.
    */
    void returnPressed( const QString& );

    /**
    * This is a convienence signal emitted to make life easier.  It has
    * the integer value of the item clicked in combo-box's list box as its
    * argument.  Unlike its counter parts in @ref QListBox, however, this
    * signal is only emitted if the click action was on an actual item.
    *
    * See also @see QComboBox::listBox
    */
    void clicked( int );

    /**
    * This is a convienence signal emitted to make life easier.  It has
    * the integer value of the item pressed in combo-box's list box as its
    * argument.  Unlike its counter parts in @ref QListBox, however, this
    * signal is only emitted if the press action was on an actual item.
    *
    * See also @see QComboBox::listBox
    */
    void pressed( int );

    /**
    * This signal is emitted when the completion key is pressed.  The
    * argument is the current text being edited.
    *
    * Note that this signal is NOT available if this widget is non-editable
    * or the completion mode is set to KGlobal::CompletionNone.
    */
    void completion( const QString& );

    /**
    * This signal is emitted when the rotate up key is pressed.
    *
    * Note that this signal is NOT available if this widget is non-editable
    * or the completion mode is set to KGlobal::CompletionNone.
    */
    void rotateUp();

    /**
    * This signal is emitted when the rotate down key is pressed.
    *
    * Note that this signal is NOT available if this widget is non-editable
    * or the completion mode is set to KGlobal::CompletionNone.
    */
    void rotateDown();


public slots:

    /**
    * TODO : NOT YET IMPLEMENTED :))
    */
    virtual void multipleCompletions( const QStringList& );

    /**
    * This slot and its counter-parts below are provided to
    * readily connect the clicked or pressed signals from the
    * user, when (s)he select an item from the list.  That way,
    * you the app developer, can determine which signal should
    * cause this widget to set the selected item and close the
    * list box.
    */
    virtual void setSelectedItem( QListBoxItem* );

    /**
    * This is the same as the above slot except the argument
    * it expects, a QString value.
    */
    virtual void setSelectedItem( const QString& );

    /**
    * This is the same as the above slot except the argument
    * it expects, an interger value.
    */
    virtual void setSelectedItem( int );

    /**
    * This slot is a reimplemention of @see QComboBox::setEditText.
    * It is re-implemeted to provide a consitent look when items are
    * completed as well as selected from the list box.  The argument
    * is the text to set in the line edit box.
    */
    virtual void setEditText( const QString& );

protected slots:

    // Slots for manupilating the content of the line-edit from the
    // context menu.
    virtual void copy()       { m_pEdit->copy(); }
    virtual void cut()        { m_pEdit->cut(); }
    virtual void paste()      { m_pEdit->paste(); }
    virtual void clear()      { m_pEdit->clear(); }
    virtual void select()     { m_pEdit->selectAll(); }
    virtual void unselect()   { m_pEdit->deselect(); }

    // Slots for manupilating completion mode from the context menu.
    virtual void modeNone()   { setCompletionMode( KGlobal::CompletionNone ); }
    virtual void modeManual() { setCompletionMode( KGlobal::CompletionMan );  }
    virtual void modeAuto()   { setCompletionMode( KGlobal::CompletionAuto ); }
    virtual void modeShell()  { setCompletionMode( KGlobal::CompletionShell );}

    // deals with text changing in the line edit in editable mode.
    virtual void itemChanged( const QString& );
    // deals with highlighting the seleted item when return
    // is pressed in the list box (editable-mode only).
    virtual void itemSelected( QListBoxItem* );
    // deals with processing text completions.
    virtual void makeCompletion( const QString& );
    // deals with highlighting an item in list box when
    // autohighlight is enabled.
    virtual void mouseOverItem( QListBoxItem* );
    // emits a clicked signal with an integer parameter
    virtual void clickItemEvent( QListBoxItem* );
    // emits a pressed signal with an integer parameter
    virtual void pressedItemEvent( QListBoxItem* );
    // emits a returnPressed signal with a QString parameter.
    virtual void returnKeyPressed();
    // populates the context menu before it is displayed
    virtual void aboutToShowMenu();
    // populates the sub menu before it is displayed
    virtual void aboutToShowSubMenu( int );

protected:
    // Initializes the variables upon construction.
    virtual void initialize();
    // Override the key-press event for "select-only" box.
    virtual void keyPressEvent ( QKeyEvent* );

private :
    // Stores the completion key locally
    int m_iCompletionKey;
    // Stores the Rotate up key locally
    int m_iRotateUpKey;
    // Stores the Rotate down key locally
    int m_iRotateDnKey;
    // Holds the length of the entry.
    int m_iPrevlen;
    // Holds the current cursor position.
    int m_iPrevpos;
    // Holds the sub-menu id once created.
    // This is needed to put check marks along
    // the selected items.
    int subMenuID;

    // Flag that indicates whether we enable/disable
    // the context (popup) menu.
    bool m_bShowContextMenu;
    // Flag that indicates whether we show/hide the mode
    // changer item in the context menu.
    bool m_bShowModeChanger;
    // Flag that determined whether the completion object
    // should be deleted when this object is destroyed.
    bool m_bAutoDelCompObj;
    //
    bool m_bAutoSelect;

    // Stores the completion mode locally.
    KGlobal::Completion m_iCompletionMode;
    // Line Editor
    QLineEdit* m_pEdit;
    // Pointer to Completion object.
    KCompletion* m_pCompObj;
    // Context Menu items.
    QPopupMenu *m_pContextMenu, *m_pSubMenu;
    // Event Filter to trap events
    virtual bool eventFilter( QObject* o, QEvent* e );
};

#endif
