/* This file is part of the KDE libraries

   Copyright (c) 2000,2001 Dawit Alemayehu <adawit@kde.org>
   Copyright (c) 2000,2001 Carsten Pfeiffer <pfeiffer@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License (LGPL) as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef _KCOMBOBOX_H
#define _KCOMBOBOX_H

#include <qlineedit.h>
#include <qcombobox.h>

#include <kcompletion.h>

class QListBoxItem;
class QPopupMenu;
class QLineEdit;

class KCompletionBox;
class KURL;

/**
 * A combined button, line-edit and a popup list widget.
 *
 * @sect Detail
 *
 * This widget inherits from @ref QComboBox and implements the following
 * additional functionalities:  a completion object that provides both automatic
 * and manual text completion as well as text rotation features, configurable
 * key-bindings to activate these features, and a popup-menu item that can be
 * used to allow the user to change the text completion mode on the fly.
 *
 * To support these new features KComboBox emits a few additional signals
 * such as @ref completion( const QString& ) and @ref textRotation( KeyBindgingType ).
 * The completion signal can be connected to a slot that will assist the user in
 * filling out the remaining text while the rotation signal can be used to traverse
 * through all possible matches whenever text completion results in multiple matches.
 * Additionally, a @ref returnPressed() and a @ref returnPressed( const QString& )
 * signals are emitted when the user presses the Enter/Return key.
 *
 * KCombobox by default creates a completion object when you invoke the
 * @ref completionObject( bool ) member function for the first time or
 * explicitly use @ref setCompletionObject( KCompletion*, bool ) to assign your
 * own completion object.  Additionally, to make this widget more functional,
 * KComboBox will by default handle text rotation and completion events
 * internally whenever a completion object is created through either one of the
 * methods mentioned above.  If you do not need this functionality, simply use
 * @ref KCompletionBase::setHandleSignals(bool) or alternatively set the boolean
 * parameter in the @p setCompletionObject call to FALSE.
 *
 * The default key-bindings for completion and rotation is determined from the
 * global settings in @ref KStdAccel. These values, however, can be overriden
 * locally by invoking @ref KCompletionBase::setKeyBinding(). The values can
 * easily be reverted back to the default setting, by simply calling
 * @ref useGlobalSettings(). An alternate method would be to default individual
 * key-bindings by usning @ref setKeyBinding() with the default second argument.
 *
 * In non-editable (select-only) mode only one completion mode, @p CompletionAuto,
 * is available. All the other modes are simply ignored. Moreover, the CompletionAuto
 * mode, unlike in the read-write mode, works by matching any typed key with the
 * first letter of entries in the combobox.
 *
 * @sect Usage
 *
 * To enable the basic completion feature:
 *
 * <pre>
 * KComboBox *combo = new KComboBox( true, this, "mywidget" );
 * KCompletion *comp = combo->completionObject();
 * // Connect to the return pressed signal - optional
 * connect(combo,SIGNAL(returnPressed(const QString&)),comp,SLOT(addItem(const QString&));
 *
 * // Provide the to be completed strings. Note that those are separate from the combo's
 * // contents.
 * comp->insertItems( someQStringList );
 * </pre>
 *
 * To use your own completion object:
 *
 * <pre>
 * KComboBox *combo = new KComboBox( this,"mywidget" );
 * KURLCompletion *comp = new KURLCompletion();
 * combo->setCompletionObject( comp );
 * // Connect to the return pressed signal - optional
 * connect(combo,SIGNAL(returnPressed(const QString&)),comp,SLOT(addItem(const QString&));
 * </pre>
 *
 * Note that you have to either delete the allocated completion object
 * when you don't need it anymore, or call
 * setAutoDeleteCompletionObject( true );
 *
 * Miscellaneous function calls:
 *
 * <pre>
 * // Tell the widget not to handle completion and rotation
 * combo->setHandleSignals( false );
 * // Set your own completion key for manual completions.
 * combo->setKeyBinding( KCompletionBase::TextCompletion, Qt::End );
 * // Hide the context (popup) menu
 * combo->setContextMenuEnabled( false );
 * </pre>
 *
 * @short An enhanced combo box.
 * @author Dawit Alemayehu <adawit@kde.org>
 */
class KComboBox : public QComboBox, public KCompletionBase
{
  Q_OBJECT
  Q_PROPERTY( bool autoCompletion READ autoCompletion WRITE setAutoCompletion )
  Q_PROPERTY( bool contextMenuEnabled READ isContextMenuEnabled WRITE setContextMenuEnabled )
  Q_PROPERTY( bool urlDropsEnabled READ isURLDropsEnabled WRITE setURLDropsEnabled )

public:

    /**
    * Constructs a read-only or rather select-only combo box with a
    * parent object and a name.
    *
    * @param parent The parent object of this widget
    * @param name The name of this widget
    */
    KComboBox( QWidget *parent=0, const char *name=0 );

    /**
    * Constructs a "read-write" or "read-only" combo box depending on
    * the value of the first argument( @p rw ) with a parent, a
    * name.
    *
    * @param rw When @p true, widget will be editable.
    * @param parent The parent object of this widget.
    * @param name The name of this widget.
    */
    KComboBox( bool rw, QWidget *parent=0, const char *name=0 );

    /**
    * Destructor.
    */
    virtual ~KComboBox();

    /**
     * Sets @p url into the edit field of the combobox. It uses
     * @ref KURL::prettyURL() so that the url is properly decoded for
     * displaying.
     */
    void setEditURL( const KURL& url );

    /**
     * Inserts @p url at position @p index into the combobox. The item will
     * be appended if @p index is negative. @ref KURL::prettyURL() is used
     * so that the url is properly decoded for displaying.
     */
    void insertURL( const KURL& url, int index = -1 );

    /**
     * Inserts @p url with the pixmap &p pixmap at position @p index into
     * the combobox. The item will be appended if @p index is negative.
     * @ref KURL::prettyURL() is used so that the url is properly decoded
     * for displaying.
     */
    void insertURL( const QPixmap& pixmap, const KURL& url, int index = -1 );

    /**
     * Replaces the item at position @p index with @p url.
     * @ref KURL::prettyURL() is used so that the url is properly decoded
     * for displaying.
     */
    void changeURL( const KURL& url, int index );

    /**
     * Replaces the item at position @p index with @p url and pixmap @p pixmap.
     * @ref KURL::prettyURL() is used so that the url is properly decoded
     * for displaying.
     */
    void changeURL( const QPixmap& pixmap, const KURL& url, int index );

    /**
    * Returns the current cursor position.
    *
    * This method always returns a -1 if the combo-box is @em not
    * editable (read-write).
    *
    * @return Current cursor position.
    */
    int cursorPosition() const { return ( lineEdit() ) ? lineEdit()->cursorPosition() : -1; }

    /**
    * Re-implemented from @ref QComboBox.
    *
    * If @p true, the completion mode will be set to automatic.
    * Otherwise, it is defaulted to the global setting.  This
    * method has been replaced by the more comprehensive
    * @ref setCompletionMode().
    *
    * @param autocomplete Flag to enable/disable automatic completion mode.
    */
    virtual void setAutoCompletion( bool autocomplete );

    /**
    * Re-implemented from QComboBox.
    *
    * Returns @p true if the current completion mode is set
    * to automatic.  See its more comprehensive replacement
    * @ref completionMode().
    *
    * @return @p true when completion mode is automatic.
    */
    bool autoCompletion() const {
        return completionMode() == KGlobalSettings::CompletionAuto;
    }

    /**
    * Enables or disable the popup (context) menu.
    *
    * This method only works if this widget is editable, i.e.
    * read-write and allows you to enable/disable the context
    * menu.  It does nothing if invoked for a none-editable
    * combo-box.  Note that by default the mode changer item
    * is made visiable whenever the context menu is enabled.
    * Use @ref hideModechanger() if you want to hide this
    * item.    Also by default, the context menu is created if
    * this widget is editable. Call this function with the
    * argument set to false to disable the popup menu.
    *
    * @param showMenu If @p true, show the context menu.
    * @param showMode If @p true, show the mode changer.
    */
    virtual void setContextMenuEnabled( bool showMenu );

    /**
    * Returns @p true when the context menu is enabled.
    */
    bool isContextMenuEnabled() const { return m_bEnableMenu; }

    /**
     * Enables/Disables handling of URL drops. If enabled and the user
     * drops an URL, the decoded URL will be inserted. Otherwise the default
     * behaviour of QComboBox is used, which inserts the encoded URL.
     *
     * @param enable If @p true, insert decoded URLs
     */
    void setURLDropsEnabled( bool enable );

    /**
     * Returns @p true when decoded URL drops are enabled
     */
    bool isURLDropsEnabled() const;

    /**
     * Convenience method which iterates over all items and checks if
     * any of them is equal to @p text.
     *
     * If @p text is an empty string, @p false
     * is returned.
     *
     * @return @p true if an item with the string @p text is in the combobox.
     */
    bool contains( const QString& text ) const;

    /**
     * By default, KComboBox recognizes Key_Return and Key_Enter
     * and emits
     * the @ref returnPressed() signals, but it also lets the event pass,
     * for example causing a dialog's default-button to be called.
     *
     * Call this method with @p trap equal to true to make KComboBox
     * stop these
     * events. The signals will still be emitted of course.
     *
     * Only affects read-writable comboboxes.
     *
     * @see setTrapReturnKey()
     */
    void setTrapReturnKey( bool trap );

    /**
     * @return @p true if keyevents of Key_Return or Key_Enter will
     * be stopped or if they will be propagated.
     *
     * @see setTrapReturnKey ()
     */
    bool trapReturnKey() const;

    /**
    * Re-implemented for internal reasons.  API not affected.
    *
    * @reimplemented
    */
    virtual bool eventFilter( QObject *, QEvent * );

    /**
     * @returns the completion-box, that is used in completion mode
     * @ref KGlobalSettings::CompletionPopup and @ref KGlobalSettings::CompletionPopupAuto.
     * This method will create a completion-box by calling
     * @ref KLineEdit::completionBox, if none is there, yet.
     *
     * @param create Set this to false if you don't want the box to be created
     *               i.e. to test if it is available.
     */
    KCompletionBox * completionBox( bool create = true );

    /**
     * Re-implemented for internal reasons.  API remains unaffected.
     * NOTE: Only editable comboboxes can have a line editor. As such
     * any attempt to assign a line-edit to a non-editable combobox will
     * simply be ignored.
     */
    virtual void setLineEdit( QLineEdit * );

signals:
    /**
    * Emitted when the user presses the Enter key.
    *
    * Note that this signal is only emitted when the widget is editable.
    */
    void returnPressed();

    /**
    * Emitted when the user presses the Enter key.
    *
    * The argument is the current text being edited.  This signal is just like
    * @ref returnPressed() except it contains the current text as its argument.
    *
    * Note that this signal is only emitted when the
    * widget is editable.
    */
    void returnPressed( const QString& );

    /**
    * Emitted when the completion key is pressed.
    *
    * The argument is the current text being edited.
    *
    * Note that this signal is @em not available when the widget is non-editable
    * or the completion mode is set to @p KGlobalSettings::CompletionNone.
    */
    void completion( const QString& );

    /**
     * Emitted when the shortcut for substring completion is pressed.
     */
    void substringCompletion( const QString& );
    
   /**
    * Emitted when the text rotation key-bindings are pressed.
    *
    * The argument indicates which key-binding was pressed. In this case this
    * can be either one of four values: @p PrevCompletionMatch, 
    * @p NextCompletionMatch, @p RotateUp or @p RotateDown. See @ref 
    * KCompletionBase::setKeyBinding() for details.
    *
    * Note that this signal is @em NOT emitted if the completion
    * mode is set to CompletionNone.
    */
    void textRotation( KCompletionBase::KeyBindingType );
    
    /**
     * Emitted whenever the completion mode is changed by the user 
     * through the context menu.
     */
    void completionModeChanged( KGlobalSettings::Completion );

    /**
     * Emitted before the context menu is displayed.
     *
     * The signal allows you to add your own entries into the context menu.
     * Note that you MUST NOT store the pointer to the QPopupMenu since it is
     * created and deleted on demand.  Otherwise, you can crash your app.
     *
     * @param the context menu about to be displayed
     */
    void aboutToShowContextMenu( QPopupMenu * );

public slots:

    /**
    * Iterates through all possible matches of the completed text
    * or the history list.
    *
    * Depending on the value of the argument, this function either
    * iterates through the history list of this widget or the all
    * possible matches in whenever multiple matches result from a
    * text completion request.  Note that the all-possible-match
    * iteration will not work if there are no previous matches, i.e.
    * no text has been completed and the *nix shell history list
    * rotation is only available if the insertion policy for this
    * widget is set either @p QComobBox::AtTop or @p QComboBox::AtBottom.
    * For other insertion modes whatever has been typed by the user
    * when the rotation event was initiated will be lost.
    *
    * @param type The key-binding invoked.
    */
    void rotateText( KCompletionBase::KeyBindingType /* type */ );

    /**
     * Sets the completed text in the line-edit appropriately.
     *
     * This function is an implementation for
     * @ref KCompletionBase::setCompletedText.
     */
    virtual void setCompletedText( const QString& );

    /**
     * Sets @p items into the completion-box if @ref completionMode() is
     * CompletionPopup. The popup will be shown immediately.
     */
    void setCompletedItems( const QStringList& items );

    /**
     * Selects the first item that matches @p item. If there is no such item,
     * it is inserted at position @p index if @p insert is true. Otherwise,
     * no item is selected.
     */
    void setCurrentItem( const QString& item, bool insert = false, int index = -1 );
    void setCurrentItem(int index) { QComboBox::setCurrentItem(index); }

protected slots:

    /**
    * @deprecated.
    */
    virtual void itemSelected( QListBoxItem* ) {};

    /**
    * Completes text according to the completion mode.
    *
    * Note: this method is @p not invoked if the completion mode is
    * set to CompletionNone.  Also if the mode is set to @p CompletionShell
    * and multiple matches are found, this method will complete the
    * text to the first match with a beep to inidicate that there are
    * more matches.  Then any successive completion key event iterates
    * through the remaining matches.  This way the rotation functionality
    * is left to iterate through the list as usual.
    */
    virtual void makeCompletion( const QString& );

protected:
    /*
    * This function simply sets the lineedit text and
    * highlights the text appropriately if the boolean
    * value is set to true.
    *
    * @param
    * @param
    */
    virtual void setCompletedText( const QString& /* */, bool /*marked*/ );

    /**
     * Reimplemented for internal reasons, the API is not affected.
     */
    virtual void create( WId = 0, bool initializeWindow = true,
                         bool destroyOldWindow = true );

    virtual void wheelEvent( QWheelEvent *ev );

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
        PopupCompletion,
        SemiAutoCompletion
    };

    /**
     * Initializes the variables upon construction.
     */
    void init();

    // BC: No longer needed...
    bool m_bEnableMenu;

    // Flag to determine whether return-key events should be propagated...
    bool m_trapReturnKey; // ### BCI: unused, remove in KDE4

protected:
    virtual void virtual_hook( int id, void* data );

private:
    class KComboBoxPrivate;
    KComboBoxPrivate *d;
};


class KPixmapProvider;

/**
 * A combobox which implements a history like a unix shell. You can navigate
 * through all the items by using the Up or Down arrows (configurable of
 * course). Additionally, weighted completion is available. So you should
 * load and save the completion list to preserve the weighting between
 * sessions.
 *
 * KHistoryCombo obeys the HISTCONTROL environment variable to determine
 * whether duplicates in the history should be tolerated in
 * @ref addToHistory() or not. During construction of KHistoryCombo,
 * duplicates will be disabled when HISTCONTROL is set to "ignoredups" or
 * "ignoreboth". Otherwise, duplicates are enabled by default.
 *
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 * @short A combobox for offering a history and completion
 */
class KHistoryCombo : public KComboBox
{
    Q_OBJECT
    Q_PROPERTY( QStringList historyItems READ historyItems WRITE setHistoryItems )

public:
    /**
     * Constructs a "read-write" combobox. A read-only history combobox
     * doesn't make much sense, so it is only available as read-write.
     * Completion will be used automatically for the items in the combo.
     *
     * The insertion-policy is set to NoInsertion, you have to add the items
     * yourself via the slot @ref addToHistory. If you want every item added,
     * use
     *
     * <pre>
     * connect( combo, SIGNAL( activated( const QString& )),
     *          combo, SLOT( addToHistory( const QString& )));
     * </pre>
     *
     * Use @ref QComboBox::setMaxCount() to limit the history.
     *
     * @p parent the parent object of this widget.
     * @p name the name of this widget.
     */
    KHistoryCombo( QWidget *parent = 0L, const char *name = 0L );

    // ### merge these two constructors
    /**
     * Same as the previous constructor, but additionally has the option
     * to specify whether you want to let KHistoryCombo handle completion
     * or not. If set to @p true, KHistoryCombo will sync the completion to the
     * contents of the combobox.
     */
    KHistoryCombo( bool useCompletion,
		   QWidget *parent = 0L, const char *name = 0L );

    /**
     * Destructs the combo, the completion-object and the pixmap-provider
     */
    ~KHistoryCombo();

    /**
     * Inserts @p items into the combobox. @p items might get
     * truncated if it is longer than @ref maxCount()
     *
     * @see #historyItems
     */
    inline void setHistoryItems( QStringList items ) {
        setHistoryItems(items, false);
    }

    /**
     * Inserts @p items into the combobox. @p items might get
     * truncated if it is longer than @ref maxCount()
     *
     * Set @p setCompletionList to true, if you don't have a list of
     * completions. This tells KHistoryCombo to use all the items for the
     * completion object as well.
     * You won't have the benefit of weighted completion though, so normally
     * you should do something like
     * <pre>
     * KConfig *config = kapp->config();
     * QStringList list;
     *
     * // load the history and completion list after creating the history combo
     * list = config->readListEntry( "Completion list" );
     * combo->completionObject()->setItems( list );
     * list = config->readListEntry( "History list" );
     * combo->setHistoryItems( list );
     *
     * [...]
     *
     * // save the history and completion list when the history combo is
     * // destroyed
     * list = combo->completionObject()->items()
     * config->writeEntry( "Completion list", list );
     * list = combo->historyItems();
     * config->writeEntry( "History list", list );
     * </pre>
     *
     * Be sure to use different names for saving with KConfig if you have more
     * than one KHistoryCombo.
     *
     * Note: When @p setCompletionList is true, the items are inserted into the
     * KCompletion object with mode KCompletion::Insertion and the mode is set
     * to KCompletion::Weighted afterwards.
     *
     * @see #historyItems
     * @see KComboBox::completionObject
     * @see KCompletion::setItems
     * @see KCompletion::items
     */
    void setHistoryItems( QStringList items, bool setCompletionList );

    /**
     * Returns the list of history items. Empty, when this is not a read-write
     * combobox.
     *
     * @see #setHistoryItems
     */
    QStringList historyItems() const;

    /**
     * Removes all items named @p item.
     *
     * @return @p true if at least one item was removed.
     *
     * @see #addToHistory
     */
    bool removeFromHistory( const QString& item );

    /**
     * Sets a pixmap provider, so that items in the combobox can have a pixmap.
     * @ref KPixmapProvider is just an abstract class with the one pure virtual
     * method @ref KPixmapProvider::pixmapFor(). This method is called whenever
     * an item is added to the KHistoryComboBox. Implement it to return your
     * own custom pixmaps, or use the @ref KURLPixmapProvider from libkio,
     * which uses @ref KMimeType::pixmapForURL to resolve icons.
     *
     * Set @p prov to 0L if you want to disable pixmaps. Default no pixmaps.
     *
     * @see #pixmapProvider
     */
    void setPixmapProvider( KPixmapProvider *prov );

    /**
     * @returns the current pixmap provider.
     * @see #setPixmapProvider
     * @see KPixmapProvider
     */
    KPixmapProvider * pixmapProvider() const { return myPixProvider; }

    /**
     * Resets the current position of the up/down history. Call this
     * when you manually call @ref setCurrentItem() or @ref clearEdit().
     */
    void reset() { slotReset(); }

public slots:
    /**
     * Adds an item to the end of the history list and to the completion list.
     * If @ref maxCount() is reached, the first item of the list will be
     * removed.
     *
     * If the last inserted item is the same as @p item, it will not be
     * inserted again.
     *
     * If @ref duplicatesEnabled() is false, any equal existing item will be
     * removed before @p item is added.
     *
     * Note: By using this method and not the Q and KComboBox insertItem()
     * methods, you make sure that the combobox stays in sync with the
     * completion. It would be annoying if completion would give an item
     * not in the combobox, and vice versa.
     *
     * @see #removeFromHistory
     * @see QComboBox::setDuplicatesEnabled
     */
    void addToHistory( const QString& item );

    /**
     * Clears the history and the completion list.
     */
    void clearHistory();

signals:
    /**
     * Emitted when the history was cleared by the entry in the popup menu.
     */
    void cleared();

protected:
    /**
     * Handling key-events, the shortcuts to rotate the items.
     */
    virtual void keyPressEvent( QKeyEvent * );

    /**
     * Handling wheel-events, to rotate the items.
     */
    virtual void wheelEvent( QWheelEvent *ev );

    /**
     * Inserts @p items into the combo, honouring @ref pixmapProvider()
     * Does not update the completionObject.
     *
     * Note: @ref duplicatesEnabled() is not honored here.
     *
     * Called from @ref setHistoryItems() and @ref setPixmapProvider()
     */
    void insertItems( const QStringList& items );

    /**
     * @returns if we can modify the completion object or not.
     */
    bool useCompletion() const { return compObj() != 0L; }

private slots:
    /**
     * Resets the iterate index to -1
     */
    void slotReset();

    /**
     * Called from the popupmenu,
     * calls clearHistory() and emits cleared()
     */
    void slotClear();

    /**
     * Appends our own context menu entry.
     */
    void addContextMenuItems( QPopupMenu* );

private:
    void init( bool useCompletion );
    void rotateUp();
    void rotateDown();

    /**
     * The current position (index) in the combobox, used for Up and Down
     */
    int myIterateIndex;

    /**
     * The text typed before Up or Down was pressed.
     */
    QString myText;

    /**
     * Indicates that the user at least once rotated Up through the entire list
     * Needed to allow going back after rotation.
     */
    bool myRotated;
    KPixmapProvider *myPixProvider;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KHistoryComboPrivate;
    KHistoryComboPrivate *d;
};


#endif

