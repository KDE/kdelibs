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

#include <kcompletionbase.h>


/**
 * A combined button, line-edit and a popup list widget.
 *
 * This widget inherits from QComboBox and enhances it
 * with the following functionalities : a built-in @ref
 * KCompletion object which provides automatic & manual
 * completion, the ability to change which keyboard key
 * is used to activate this feature and rotation signals
 * that can be used to iterate through some type of list.
 * They key-bindings for the rotation feature are also
 * configurable and can be interchanged between system and
 * local settings easily.
 *
 * Also since this widget inherits form QComboBox it can
 * easily be used as a drop-in replacement where the above
 * functionalities are needed and/or useful.
 *
 * KComboBox emits a few more additional signals than @ref
 * QComboBox, the main ones being the @ref comepltion and
 * the @ref rotation signal metioned above.  The completion
 * signal is intended to be connected to a slot that will
 * assist the user in filling out the remaining text while
 * the rotation signals, both @ref rotateUp and @ref rotateDown,
 * are intended to be used to transverse through some kind
 * of list in opposing directions.  The @ref returnPressed
 * signals are emitted when the user presses the return key.
 *
 * By default both the completion and rotation signals are
 * automatically handled by this widget.  If you do not need
 * these features, simply use the appropriate accessor methods
 * to shut them off.
 *
 * The default key-binding for completion and rotation is
 * determined from the global settings in @ref KStdAccel.
 * However, these values can be set locally to override these
 * global settings.  Simply invoking @ref useGlobalSettings
 * then allows you to immediately default the bindings back
 * to the global settings again.  You can also default the
 * key-bindings by simply invoking the @ref setXXXKey method
 * without any argumet.  Note that if this widget is not
 * editable, i.e. it is constructed as a "select-only" widget,
 * then only one completion mode, CompletionAuto, is allowed.
 * All the other modes are simply ignored.  The CompletionAuto
 * mode in this case allows you to automatically select
 * an item in the list that matches the pressed key-codes.
 * For example, if you have a list of countries, typing
 * the first few letters of the name attempts to find a
 * match and if one is found it will be selected as the
 * current item.
 *
 * @sect Example:
 *
 * To enable the basic completion feature :
 *
 * <pre>
 * KComboBox *combo = new KComboBox( true, this, "mywidget" );
 * KCompletion *comp = combo->completionObject();
 * // Connect to the return pressed signal - optional
 * connect(combo,SIGNAL(returnPressed(const QString&)),comp,SLOT(addItem(const QString&));
 * </pre>
 *
 * To use a customized completion objects or your
 * own completion object :
 * <pre>
 * KComboBox *combo = new KComboBox( this,"mywidget" );
 * KURLCompletion *comp = new KURLCompletion();
 * combo->setCompletionObject( comp );
 * // Connect to the return pressed signal - optional
 * connect(combo,SIGNAL(returnPressed(const QString&)),comp,SLOT(addItem(const QString&));
 * <pre>
 *
 * Other miscelanous functions :
 * <pre>
 * // Tell the widget not to handle completion and rotation
 * combo->setHandleSignals( false );
 * // set your own completion key for manual completions.
 * combo->setCompletionKey( Qt::End );
 * // Shows the context (popup) menu
 * combo->setEnableContextMenu();
 * // Temporarly disable signal emition
 * combo->disableSignals();
 * // Default the key-bindings to system settings.
 * combo->useGlobalSettings();
 * </pre>
 *
 * @short An enhanced combo box.
 * @author Dawit Alemayehu <adawit@earthlink.net>
 */
class KComboBox : public QComboBox, public KCompletionBase
{
  Q_OBJECT

public:

    /**
    * Constructs a read-only or rather select-only combo box with a parent object
    * and a name.
    *
    * @param parent the parent object of this widget
    * @param name the name of this widget
    */
    KComboBox( QWidget *parent=0, const char *name=0 );

    /**
    * Constructs a "read-write" or "read-only" combo box depending on the value of
    * the first argument( bool rw ) with a parent, a name.
    *
    * @param string text to be shown in the edit widget
    * @param parent the parent object of this widget
    * @param name the name of this widget
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
    * @return current cursor position.
    */
    int cursorPosition() const { return ( m_pEdit ) ? m_pEdit->cursorPosition() : -1; }

    /**
    * Re-implemented from QComboBox.
    *
    * This function now always returns 0.  All the
    * functions needed to manipulate the line edit
    * with the execption of echomode are supplied
    * here.  Methods that affect the funcationality
    * of this widget are not made available.
    *
    * @return always a NULL pointer.
    */
    QLineEdit* lineEdit() const { return 0; }

    /**
    * Re-implemented from QComboBox.
    *
    * If true, the completion mode will be set to automatic.
    * Otherwise, it is defaulted to the gloabl setting.  This
    * methods has been replaced by the more comprehensive @ref
    * setCompletionMode.
    *
    * @param autocomplete flag to enable/disable automatic completion mode.
    */
    virtual void setAutoCompletion( bool autocomplete );

    /**
    * Re-implemented from QComboBox.
    *
    * Returns true if the current completion mode is set
    * to automatic.  See its more comprehensive replacement
    * @ref completionMode.
    *
    * @return true when completion mode is automatic.
    */
    bool autoCompletion() const { return m_iCompletionMode == KGlobal::CompletionAuto; }

    /**
    * Re-implemented from @ref KCompletionBase.
    *
    * This method allows you to enable the completion feature
    * by supplying your own KCompletion object.  The object
    * assigned through this method is not deleted when this
    * widget is destroyed.  If you want KComboBox to handle
    * the deletion, use @ref setAutoDeleteCompletionObject
    * This functionality is helpful when you want to share
    * a single completion object across multiple widgets.
    *
    * @param obj a @ref KCompletion or a derived child object.
    */
    virtual void setCompletionObject( KCompletion* );


    /**
    * Re-implemented from @ref KCompletionBase.
    *
    * When this function is invoked with the argument set to
    * "true", KComboBox will automatically handle completion
    * and rotation signals.  To stop KComboBox from handling
    * these signals internally simply invoke this function
    * with with the argument set to false.
    *
    * Note that calling this function does not hinder you from
    * connecting and hence receiving the completion signals
    * externally.
    *
    * @param complete if true, handle completion & roation internally.
    */
    virtual void setHandleSignals( bool complete );

    /**
    * Enables/disables the popup (context) menu.
    *
    * This method only works if this widget is editable, i.e.
    * read-write and allows you to enable/disable the context
    * menu. If this method is invoked without an argument, the
    * context menu will be disabled.  Note that by default the
    * mode changer is visible when context menu is enabled.
    * Use either hideModechanger() or call this function with
    * the second argument set to "false" if you do not want that
    * item to be inserted.
    *
    * @param showMenu if true, shows the context menu.
    * @param showModeChanger if true, shows the mode changer in popup menu.
    */
    virtual void setEnableContextMenu( bool showMenu = true, bool showChanger = true );

    /**
    * Shows the mode changer in the context menu.
    */
    void showModeChanger() { m_bShowModeChanger = true; }

    /**
    * Hides the mode changer in the context menu.
    */
    void hideModeChanger();

    /**
    * Returns true when the context menu is enabled.
    *
    * @return true if context menu is enabled.
    */
    bool isContextMenuEnabled() const { return m_bEnableMenu; }

    /**
    * Returns true if the mode changer item is visible in
    * the context menu.
    *
    * @return true if the mode changer is visible in context menu.
    */
    bool isModeChangerVisible() const { return m_bShowModeChanger; }

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
    * This slot is a re-implemention of @ref QComboBox::setEditText.
    * It is re-implemeted to provide a consitent look when items are
    * completed as well as selected from the list box.  The argument
    * is the text to set in the line edit box.
    */
    virtual void setEditText( const QString& );

    /**
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

    /**
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

    /**
    * Selects the text in the lined edit.  Does nothing
    * if this widget is not editable.
    */
    virtual void selectText()     { if( m_pEdit != 0 ) m_pEdit->selectAll(); }

    /**
    * Un-marks all selected text in the lined edit.  Does
    * nothing if this widget is not editable.
    */
    virtual void deselectText()   { if( m_pEdit != 0 ) m_pEdit->deselect(); }

protected slots:

    /**
    * Sets the comepltion mode to KGlobal::CompletionNone.
    */
    virtual void modeNone() { setCompletionMode( KGlobal::CompletionNone ); }

    /**
    * Sets the comepltion mode to KGlobal::CompletionManual
    */
    virtual void modeManual() { setCompletionMode( KGlobal::CompletionMan );  }

    /**
    * Sets the comepltion mode to KGlobal::CompletionAuto
    */
    virtual void modeAuto() { setCompletionMode( KGlobal::CompletionAuto ); }

    /**
    * Sets the comepltion mode to KGlobal::CompletionShell
    */
    virtual void modeShell() { setCompletionMode( KGlobal::CompletionShell );}

    /**
    * Sets the comepltion mode to the global default setting
    * defined by @ref KGlobal::completionMode().
    */
    virtual void modeDefault() { useGlobalSettings(); }

    /**
    * Deals with text changing in the line edit in
    * editable mode.
    */
    virtual void entryChanged( const QString& );

    /**
    * Deals with highlighting the seleted item when return
    * is pressed in the list box (editable-mode only).
    */
    virtual void itemSelected( QListBoxItem* );

    /**
    * Deals with text changes in auto completion mode.
    */
    virtual void makeCompletion( const QString& );

    /**
    * Emits a returnPressed signal with a QString parameter
    * that contains the current display text.
    */
    virtual void returnKeyPressed();

    /**
    * Populates the context menu before it is displayed.
    */
    virtual void aboutToShowMenu();

    /**
    * Populates the sub menu before it is displayed.
    */
    virtual void aboutToShowSubMenu( int );

    /**
    * Resets the completion object pointer when it is destroyed
    */
    void completionDestroyed() { m_pCompObj = 0; }


protected:
    /**
    * Initializes the variables upon construction.
    */
    virtual void init();

    /**
    * Overridden from QComboBox to provide automatic selection
    * in "select-only" mode.
    */
    virtual void keyPressEvent ( QKeyEvent* );

    /*
    * Rotates the text on rotation events
    */
    void rotateText( const QString& );

private :
    // Holds the length of the entry.
    int m_iPrevlen;
    // Holds the current cursor position.
    int m_iPrevpos;
    // Holds the sub-menu id once created.
    // This is needed to put check marks along
    // the selected items.
    int m_iSubMenuId;

    // Flag that indicates whether we enable/disable
    // the context (popup) menu.
    bool m_bEnableMenu;
    // Flag that indicates whether we show/hide the mode
    // changer item in the context menu.
    bool m_bShowModeChanger;

    // Pointer to the line editor.
    QLineEdit* m_pEdit;
    // Context Menu items.
    QPopupMenu *m_pContextMenu, *m_pSubMenu;
    // Event Filter to trap events
    virtual bool eventFilter( QObject* o, QEvent* e );
};

#endif
