/* This file is part of the KDE libraries
    Copyright (C) 2000 Reginald Stadlbauer (reggie@kde.org)
    (C) 1997, 1998 Stephan Kulow (coolo@kde.org)
    (C) 1997, 1998 Sven Radej (radej@kde.org)
    (C) 1997, 1998 Mark Donohoe (donohoe@kde.org)
    (C) 1997, 1998 Matthias Ettrich (ettrich@kde.org)
    (C) 1999, 2000 Kurt Granroth (granroth@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    */

#ifndef KTOOLBAR_H
#define KTOOLBAR_H

#include <q3frame.h>
#include <q3ptrlist.h>
#include <q3toolbar.h>
#include <qcombobox.h>
#include <qmap.h>
#include <qpointer.h>

#include <kglobal.h>

class Q3MainWindow;
class QMenu;
class QDomDocument;
class QDomElement;
class QIcon;
class QPixmap;
class QSize;
class QStringList;
class QTimer;

class KLineEdit;
class KToolBar;
class KToolBarButton;
class KToolBoxManager;
class KAnimWidget;
class KMenu;
class KInstance;
class KComboBox;
class KXMLGUIClient;

class KToolBarPrivate;

class KDEUI_EXPORT KToolBarSeparator : public Q3Frame
{
    Q_OBJECT
public:
    KToolBarSeparator( Qt::Orientation, bool l, Q3ToolBar *parent, const char* name=0 );

    QSize sizeHint() const;
    Qt::Orientation orientation() const { return orient; }
    QSizePolicy sizePolicy() const;
    bool showLine() const { return line; }
public slots:
   void setOrientation( Qt::Orientation );
protected:
    void styleChange( QStyle& );
    void drawContents( QPainter* );
private:
    Qt::Orientation orient;
    bool line;
};


 /**
  * @short Floatable toolbar with auto resize.
  *
  * A KDE-style toolbar.
  *
  * KToolBar can be dragged around in and between different docks.
  *
  * A KToolBar can contain all sorts of widgets.
  *
  * KToolBar can be used as a standalone widget, but KMainWindow
  * provides easy factories and management of one or more toolbars.
  * Once you have a KToolBar object, you can insert items into it with the
  * insert... methods, or remove them with the removeItem() method. This
  * can be done at any time; the toolbar will be automatically updated.
  * There are also many methods to set per-child properties like alignment
  * and toggle behavior.
  *
  * KToolBar uses a global config group to load toolbar settings on
  * construction. It will reread this config group on a
  * KApplication::appearanceChanged() signal.
  *
  * @author Reginald Stadlbauer <reggie@kde.org>, Stephan Kulow <coolo@kde.org>, Sven Radej <radej@kde.org>.
  */

class KDEUI_EXPORT KToolBar : public Q3ToolBar
{
    Q_OBJECT
    Q_ENUMS( IconText BarPosition )
    Q_PROPERTY( IconText iconText READ iconText WRITE setIconText )
    Q_PROPERTY( BarPosition barPos READ barPos WRITE setBarPos )
    Q_PROPERTY( bool fullSize READ fullSize WRITE setFullSize )
    Q_PROPERTY( int iconSize READ iconSize WRITE setIconSize )
    Q_PROPERTY( QString text READ text WRITE setText )

public:
// KDE4: Reuse QStyleToolButton
    enum IconText{IconOnly = 0, IconTextRight, TextOnly, IconTextBottom};
    /**
     * Possible bar positions.
     */
    enum BarPosition{ Unmanaged, Floating, Top, Bottom, Right, Left, Flat};

  /**
   * Normal constructor.
   * This constructor is used by the XML-GUI. If you use it, you need
   * to call QMainWindow::addToolBar to specify the position of the toolbar.
   * So it's simpler to use the other constructor.
   *
   * The toolbar will read in various global config settings for
   * things like icon size and text position, etc.  However, some of
   * the settings will be honored only if @p honorStyle is set to
   * true.  All other toolbars will be IconOnly and use Medium icons.
   *
   * @param parent      The standard toolbar parent (usually a
   * KMainWindow)
   * @param name        The standard internal name
   * @param honorStyle  If true, then global settings for IconSize and IconText will be honored
   * @param readConfig  whether to apply the configuration (global and application-specific)
   */
    KToolBar( QWidget *parent, const char *name = 0, bool honorStyle = false, bool readConfig = true );

  /**
   * Constructor for non-XML-GUI applications.
   *
   * The toolbar will read in various global config settings for
   * things like icon size and text position, etc.  However, some of
   * the settings will be honored only if @p honorStyle is set to
   * true.  All other toolbars will be IconOnly and use Medium icons.
   *
   * @param parentWindow The window that should be the parent of this toolbar
   * @param dock        The position of the toolbar. Usually QMainWindow::Top.
   * @param newLine     If true, start a new line in the dock for this toolbar.
   * @param name        The standard internal name
   * @param honorStyle  If true, then global settings for IconSize and IconText will be honored
   * @param readConfig  whether to apply the configuration (global and application-specific)
   */
    KToolBar( Q3MainWindow *parentWindow, Qt::ToolBarDock dock /*= QMainWindow::Top*/, bool newLine = false,
              const char *name = 0, bool honorStyle = false, bool readConfig = true );

  /**
   * Constructor for non-XML-GUI applications.
   *
   * The toolbar will read in various global config settings for
   * things like icon size and text position, etc.  However, some of
   * the settings will be honored only if @p honorStyle is set to
   * true.  All other toolbars will be IconOnly and use Medium icons.
   *
   * @param parentWindow The window that should be the parent of this toolbar
   * @param dock        Another widget than the mainwindow to dock toolbar to.
   * @param newLine     If true, start a new line in the dock for this toolbar.
   * @param name        The standard internal name
   * @param honorStyle  If true, then global settings for IconSize and IconText will be honored
   * @param readConfig  whether to apply the configuration (global and application-specific)
   */
    KToolBar( Q3MainWindow *parentWindow, QWidget *dock, bool newLine = false,
              const char *name = 0, bool honorStyle = false, bool readConfig = true );

  /**
   * Destructor
   */
    virtual ~KToolBar();

  /**
   * Insert a button (a KToolBarButton) with a pixmap.  The
   * pixmap is loaded by the button itself based on the global icon
   * settings.
   *
   * You should connect to one or more signals in KToolBar:
   * clicked() , pressed() , released() , or
   * highlighted()  and if the button is a toggle button
   * ( setToggle() ) toggled() .  Those signals have @p id
   * of a button that caused the signal.  If you want to bind a popup
   * to button, see setButton().
   *
   * @param icon    The name of the icon to use as the active pixmap
   * @param id      The id of this button
   * @param enabled Enable or disable the button at startup
   * @param text    The tooltip or toolbar text (depending on state)
   * @param index   The position of the button. (-1 = at end).
   * @param _instance instance this button will belong to
   *
   * @return The item index.
   */
    int insertButton(const QString& icon, int id, bool enabled = true,
		     const QString& text = QString::null, int index=-1,
		     KInstance *_instance = KGlobal::instance());

   /**
   * This is the same as above, but with specified signals and
   * slots to which this button will be connected.
   *
   * You can add more signals with addConnection().
   *
   * @param icon     The name of the icon to use as the active pixmap
   * @param id       The id of this button
   * @param signal   The signal to connect to
   * @param receiver The slot's parent
   * @param slot     The slot to receive the signal specified in that argument.
   * @param enabled  Enable or disable the button at startup
   * @param text     The tooltip or toolbar text (depending on state)
   * @param index    The position of the button. (-1 = at end).
   * @param _instance instance this button will belong to
   *
   * @return The item index.
   */
    int insertButton(const QString& icon, int id, const char *signal,
		     const QObject *receiver, const char *slot,
		     bool enabled = true, const QString& text = QString::null,
		     int index=-1, KInstance *_instance = KGlobal::instance() );

  /**
   * Inserts a button (a KToolBarButton) with the specified
   * pixmap.  This pixmap will be used as the "active" one and the
   * disabled and default ones will be autogenerated.
   *
   * It is recommended that you use the insertButton function that
   * allows you to specify the icon name rather then the pixmap
   * itself.  Specifying the icon name is much more flexible.
   *
   * You should connect to one or more signals in KToolBar:
   * clicked() , pressed() , released() , or
   * highlighted()  and if the button is a toggle button
   * ( setToggle() ) toggled() .  Those signals have @p id
   * of a button that caused the signal.  If you want to bind a popup
   * to button, see setButton().
   *
   * @param pixmap  The active pixmap
   * @param id      The id of this button
   * @param enabled Enable or disable the button at startup
   * @param text    The tooltip or toolbar text (depending on state)
   * @param index   The position of the button. (-1 = at end).
   *
   * @return The item index.
   */
    int insertButton(const QPixmap& pixmap, int id, bool enabled = true,
		     const QString& text = QString::null, int index=-1 );

  /**
   * This is the same as above, but with specified signals and
   * slots to which this button will be connected.
   *
   * You can add more signals with addConnection().
   *
   * @param pixmap   The name of the icon to use as the active pixmap
   * @param id       The id of this button
   * @param signal   The signal to connect to
   * @param receiver The slot's parent
   * @param slot     The slot to receive the signal specified in that argument.
   * @param enabled  Enable or disable the button at startup
   * @param text     The tooltip or toolbar text (depending on state)
   * @param index    The position of the button. (-1 = at end).
   *
   * @return The item index.
   */
    int insertButton(const QPixmap& pixmap, int id, const char *signal,
		     const QObject *receiver, const char *slot,
		     bool enabled = true, const QString& text = QString::null,
		     int index=-1 );

  /**
   * Inserts a button with popupmenu.
   *
   * Button will have small
   * triangle. You have to connect to popup's signals. The
   * signals KButton::pressed(), KButton::released(),
   * KButton::clicked() or KButton::doubleClicked() are @p not
   * emmited by
   * this button (see setDelayedPopup() for that).
   * You can add custom popups which inherit QPopupMenu to get popups
   * with tables, drawings etc. Just don't fiddle with events there.
   */
    int insertButton(const QString& icon, int id, QMenu *popup,
		     bool enabled, const QString&_text, int index=-1);

  /**
   * Inserts a button with popupmenu.
   *
   * Button will have small
   * triangle. You have to connect to popup's signals. The
   * signals KButton::pressed(), KButton::released(),
   * KButton::clicked() or KButton::doubleClicked() are @p not
   * emmited by
   * this button (see setDelayedPopup() for that).
   * You can add custom popups which inherit QPopupMenu to get popups
   * with tables, drawings etc. Just don't fiddle with events there.
   */
    int insertButton(const QPixmap& pixmap, int id, QMenu *popup,
		     bool enabled, const QString&_text, int index=-1);

  /**
   * Inserts a KLineEdit. You have to specify signals and slots to
   * which KLineEdit will be connected. KLineEdit has all slots QLineEdit
   * has, plus signals KLineEdit::completion and KLineEdit::textRotation
   * KLineEdit can be set to autoresize itself to full free width
   * in toolbar, that is to last right aligned item. For that,
   * toolbar must be set to full width (which it is by default).
   * @see setFullWidth()
   * @see setItemAutoSized()
   * @see KLineEdit
   * @return Item index.
   */
    int insertLined (const QString& text, int id,
		     const char *signal,
		     const QObject *receiver, const char *slot,
		     bool enabled = true,
		     const QString& toolTipText = QString::null,
		     int size = 70, int index =-1);

  /**
   * Inserts a KComboBox with list.
   *
   * Can be writable, but cannot contain
   * pixmaps. By default inserting policy is AtBottom, i.e. typed items
   * are placed at the bottom of the list. Can be autosized. If the size
   * argument is specified as -1, the width of the combobox is automatically
   * computed.
   *
   * @see setFullWidth()
   * @see setItemAutoSized()
   * @see KComboBox
   * @return Item index.
   */
    int insertCombo (const QStringList &list, int id, bool writable,
		     const char *signal, const QObject *receiver,
		     const char *slot, bool enabled=true,
		     const QString& tooltiptext=QString::null,
		     int size=70, int index=-1,
		     QComboBox::InsertPolicy policy = QComboBox::InsertAtBottom);

  /**
   * Insert a KComboBox with text.
   *
   * The rest is the same as above.
   * @see setItemAutoSized()
   *
   * @see KComboBox
   * @return Item index.
   */
    int insertCombo (const QString& text, int id, bool writable,
		     const char *signal, QObject *receiver,
		     const char *slot, bool enabled=true,
		     const QString& tooltiptext=QString::null,
		     int size=70, int index=-1,
		     QComboBox::InsertPolicy policy = QComboBox::InsertAtBottom);

    /**
     * Inserts a separator into the toolbar with the given @p id.
     * @return the separator's index
     */
    int insertSeparator( int index = -1, int id = -1 );

    /**
     * Inserts a line separator into the toolbar with the given @p id.
     * @return the separator's index
     */
    int insertLineSeparator( int index = -1, int id = -1 );

  /**
   * Inserts a user-defined widget.  The widget @p must have this
   * toolbar as its parent.
   *
   * Widget must have a QWidget for base class.  Widget can be
   * autosized to full width. If you forget about it, you can get a
   * pointer to this widget with getWidget().
   * @see setItemAutoSized()
   * @return Item index.
   */
    int insertWidget(int id, int width, QWidget *_widget, int index=-1);

  /**
   * Inserts an animated widget.  A KAnimWidget will be created
   * internally using the icon name you provide.
   * This will emit a signal (clicked()) whenever the
   * animation widget is clicked.
   *
   * @see animatedWidget()
   *
   * @param id       The id for this toolbar item
   * @param receiver The parent of your slot
   * @param slot     The slot to receive the clicked() signal
   * @param icons    The name of the animation icon group to use
   * @param index    The item index
   *
   * @return The item index
   */
    int insertAnimatedWidget(int id, QObject *receiver, const char *slot,
			     const QString& icons, int index = -1);

    /**
   * This will return a pointer to the animated widget with the
   * given @p id, if it exists.
   *
   * @see insertAnimatedWidget
   * @param id The id for the widget you want to get a pointer to
   * @return A pointer to the current animated widget or 0L
   */
    KAnimWidget *animatedWidget( int id );

  /**
   * Adds connections to items.
   *
   * It is important that you
   * know the @p id of particular item. Nothing happens if you forget @p id.
   */
    void addConnection (int id, const char *signal,
                        const QObject *receiver, const char *slot);
  /**
   * Enables/disables item.
   */
    void setItemEnabled( int id, bool enabled );

  /**
   * Sets the icon for a button.
   *
   * Can be used while button is visible.
   */
    void setButtonIcon( int id, const QString& _icon );

  /**
   * Sets button pixmap.
   *
   * Can be used while button is visible.
   */
    void setButtonPixmap( int id, const QPixmap& _pixmap );

  /**
   * Sets a button icon from a QIconSet.
   *
   * Can be used while button is visible.
   */
    void setButtonIconSet( int id, const QIcon& iconset );

  /**
   * Sets a delayed popup for a button.
   *
   * Delayed popup is what you see in
   * Netscape Navigator's Previous and Next buttons: If you click them you
   * go back
   * or forth. If you press them long enough, you get a history-menu.
   * This is exactly what we do here.
   *
   * You will insert a normal button with connection (or use signals from
   * toolbar):
   * \code
   * bar->insertButton(icon, id, SIGNAL(clicked ()), this,
   *                   SLOT (slotClick()), true, "click or wait for popup");
   * \endcode
   * And then add a delayed popup:
   * \code
   * bar->setDelayedPopup (id, historyPopup);
   * \endcode
   *
   * Don't add delayed popups to buttons which have normal popups.
   *
   * You may add popups which are derived from QPopupMenu. You may
   * add popups that are already in the menu bar or are submenus of
   * other popups.
   */
    void setDelayedPopup (int id , QMenu *_popup, bool toggle = false);

  /**
   * Turns a button into an autorepeat button.
   *
   * Toggle buttons, buttons with menus, or
   * buttons with delayed menus cannot be made into autorepeat buttons.
   * Moreover, you can and will receive
   * only the signal clicked(), but not pressed() or released().
   * When the user presses this button, you will receive the signal clicked(),
   * and if the button is still pressed after some time,
   * you will receive more clicked() signals separated by regular
   * intervals.  Since this uses QButton::setAutoRepeat() ,
   * I can't quantify 'some'.
   */
    void setAutoRepeat (int id, bool flag=true);


  /**
   * Turns button into a toggle button if @p flag is true.
   */
    void setToggle (int id, bool flag = true);

  /**
   * Toggles a togglebutton.
   *
   * If the button is a toggle button  (see setToggle())
   * the button state will be toggled. This will also cause the toolbar to
   * emit the signal KButton::toggled() with parameter @p id. You must connect to
   * this signal, or use addConnection() to connect directly to the
   * button signal KButton::toggled().
   */
    void toggleButton (int id);

  /**
   * Sets a toggle button state.
   *
   * If the button is a toggle button (see setToggle())
   * this will set its state flag. This will also emit the signal
   * KButton::toggled().
   *
   * @see setToggle()
   */
    void setButton (int id, bool flag);

  /**
   * Returns @p true if button is on and is a toggle button
   * @return @p true if button is on and is a toggle button
   * @see setToggle()
   */
    bool isButtonOn (int id) const;

  /**
   * Sets the text of a line editor.
   *
   * Cursor is set at end of text.
   */
    void setLinedText (int id, const QString& text);

  /**
   * Returns the line editor text.
   * @return the line editor text.
   */
    QString getLinedText (int id) const;

  /**
   * Inserts @p text in combobox @p id at position @p index.
   */
    void insertComboItem (int id, const QString& text, int index);

    /**
   * Inserts @p list in combobox @p id at position @p index.
   */
    void insertComboList (int id, const QStringList &list, int index);

  /**
   * Removes item @p index from combobox @p id.
   */
    void removeComboItem (int id, int index);

  /**
   * Sets item @p index to be current item in combobox @p id.
   */
    void setCurrentComboItem (int id, int index);

  /**
   * Changes item @p index in combobox @p id to text.
   *
   * @p index = -1 refers current item (one displayed in the button).
   */
    void changeComboItem  (int id, const QString& text, int index=-1);

  /**
   * Clears the combobox @p id.
   *
   * Does not delete it or hide it.
   */
    void clearCombo (int id);

  /**
   * Returns text of item @p index from combobox @p id.
   * @return text of item @p index from combobox @p id.
   *
   * @p index = -1 refers to current item.
   */

    QString getComboItem (int id, int index=-1) const;

    /**
   * Returns a pointer to the combobox with @p id.
   * @return a pointer to the combobox with @p id.
   *
   * Example:
   * \code
   * KComboBox *combo = toolbar->getCombo(combo_id);
   * \endcode
   * That way you can get access to other public methods
   * that KComboBox provides.
   */
    KComboBox * getCombo(int id);

    /**
   * Returns a pointer to KToolBarLined with @p id.
   * @return a pointer to KToolBarLined with @p id.
   *
   * Example:
   * \code
   * KLineEdit * lined = toolbar->getKToolBarLined(lined_id);
   * \endcode
   * That way you can get access to other public methods
   * that KLineEdit provides. KLineEdit is the same thing
   * as QLineEdit plus completion signals.
   */
    KLineEdit * getLined (int id);

    /**
   * Returns a pointer to KToolBarButton.
   *
   * Example:
   * \code
   * KToolBarButton * button = toolbar->getButton(button_id);
   * \endcode
   * That way you can get access to other public methods
   * that KToolBarButton provides.
   *
   *  Using this method is not recommended.
   */
    KToolBarButton * getButton (int id);

    /**
   * Align item to the right.
   *
   * This works only if toolbar is set to full width.
   * @see setFullWidth()
   */
    void alignItemRight (int id, bool right = true);

    /**
   * Returns a pointer to the widget corresponding to @p id.
   * @return a pointer to the widget corresponding to @p id.
   *
   * Wrong ids are not tested.
   * You can do with this whatever you want,
   * except change its height (hardcoded). If you change its width
   * you will probably have to call QToolBar::updateRects(true)
   * @see QWidget
   * @see updateRects()
   */
    QWidget *getWidget (int id); // ### KDE4: make this const!

    /**
   * Set item autosized.
   *
   * This works only if the toolbar is set to full width.
   * Only @p one item can be autosized, and it has to be
   * the last left-aligned item. Items that come after this must be right
   * aligned. Items that can be right aligned are Lineds, Frames, Widgets and
   * Combos. An autosized item will resize itself whenever the toolbar geometry
   * changes to the last right-aligned item (or to end of toolbar if there
   * are no right-aligned items.)
   * @see setFullWidth()
   * @see alignItemRight()
   */
    void setItemAutoSized (int id, bool yes = true);

    /**
   * Remove all items.
   *
   * The toolbar is redrawn after it.
   */
    void clear ();

    /**
   * Remove item @p id.
   *
   * Item is deleted. Toolbar is redrawn after it.
   */
    void removeItem (int id);

    /**
   * Remove item @p id.
   *
   * Item is deleted when toolbar is redrawn.
   */
    void removeItemDelayed (int id);

    /**
   * Hide item @p id.
   */
    void hideItem (int id);

    /**
   * Show item @p id.
   */
    void showItem (int id);

    /**
   * Returns the index of the item @p id.
   * @return the index of the item @p id.
   * @since 3.2
   */
    int itemIndex (int id); // ### KDE4: make this const!

    /**
   * Returns the id of the item at the given index.
   * @since 3.2
   */
    int idAt(int index); // ### KDE4: make this const!

    /**
   * Set toolbar to full parent size (default).
   *
   * In full size mode the bar extends over the parent's full width or height.
   * If the mode is disabled the toolbar tries to take as much space as it
   * needs without wrapping, but it does not exceed the parent box. You can
   * force a certain width or height with setMaxWidth() or
   * setMaxHeight().
   *
   * If you want to use right-aligned items or auto-sized items you must use
   * full size mode.
   */
    void setFullSize(bool flag = true);

    /**
   * Returns the full-size mode enabled flag.
   * @return @p true if the full-size mode is enabled.
   */
    bool fullSize() const;

    /**
   * Set position of toolbar.
   * @see BarPosition()
   */
    void setBarPos (BarPosition bpos);

    /**
   * Returns the toolbar position.
   * @return position of toolbar.
   */
    BarPosition barPos() const;

   /**
   * Set title for toolbar when it floats.
   *
   * Titles are however not (yet)
   * visible. You can't change toolbar's title while it's floating.
   */
    void setTitle (const QString& _title);

   /**
   * Set the kind of painting for buttons.
   *
   * Choose from:
   * @li IconOnly (only icons),
   * @li IconTextRight (icon and text, text is left from icons),
   * @li TextOnly (only text),
   * @li IconTextBottom (icons and text, text is under icons).
   * @see IconText
   *
   */
    void setIconText(IconText it);
   // Note: don't merge with the next one, it breaks Qt properties

  /**
   * Similar to setIconText(IconText it) but allows you to
   * disable or enable updating.  If @p update is false, then the
   * buttons will not be updated.  This is useful only if you know
   * that you will be forcing an update later.
   */
    void setIconText(IconText it, bool update);

  /**
   * Returns the current text style for buttons.
   * @return the current text style for buttons.
   */
    IconText iconText() const;

  /**
   * Set the icon size to load. Usually you should not call
   * this, the icon size is taken care of by KIconLoader
   * and globally configured.
   * By default, the toolbar will load icons of size 32 for main
   * toolbars and 22 for other toolbars
   * @see KIconLoader.
   *
   * @param size The size to use
   */
    void setIconSize(int size);
    // Note: don't merge with the next one, it breaks Qt properties

  /**
   * Same as setIconText(int size) but allows you
   * to disable the toolbar update.
   *
   * @param size   The size to use
   * @param update If true, then the toolbar will be updated after
   *               this
   */
    void setIconSize(int size, bool update);

  /**
   * Returns the current icon size for buttons.
   * @return the current icon size for buttons.
   */
    int iconSize() const;

  /**
   * Returns the default size for this type of toolbar.
   * @return the default size for this type of toolbar.
   */
    int iconSizeDefault() const;

  /**
   * This allows you to enable or disable the context menu.
   *
   * @param enable If false, then the context menu will be disabled
   */
    void setEnableContextMenu(bool enable = true);

  /**
   * Returns the context menu enabled flag
   * @return true if the context menu is disabled
   */
    bool contextMenuEnabled() const;

  /**
   * This will inform a toolbar button to ignore certain style
   * changes.  Specifically, it will ignore IconText (always IconOnly)
   * and will not allow image effects to apply.
   *
   * @param id The button to exclude from styles
   * @param no_style If true, then it is excluded (default: true).
   */
    void setItemNoStyle(int id, bool no_style = true);

    void setFlat (bool flag);

  /**
   * Returns the total number of items in the toolbar
   * @return the total number of items in the toolbar
   */
    int count() const;

  /**
   * Instruct the toolbar to save it's current state to either the app
   * config file or to the XML-GUI resource file (whichever has
   * precedence).
   */
    void saveState(); // BIC: remove for KDE4? This doesn't appear to be used internally,
                      // and apps use saveMainWindowSettings in KMainWindow anyway.

  /**
   * Save the toolbar settings to group @p configGroup in @p config.
   */
    void saveSettings(KConfig *config, const QString &configGroup);

  /**
   * Read the toolbar settings from group @p configGroup in @p config
   * and apply them. Even default settings are re-applied if @p force is set.
   */
    void applySettings(KConfig *config, const QString &configGroup,bool force=false);

  /*
   * Tell the toolbar what XML-GUI resource file it should use to save
   * it's state.  The state of the toolbar (position, size, etc) is
   * saved in KConfig files if the application does not use XML-GUI
   * but if the app does, then it's saved the XML file.  This function
   * allows this to happen.
   *
   * @param xmlfile The XML-GUI resource file to write to
   * @param xml     The DOM document for the XML-GUI building
   * @internal
   */
  //  void setXML(const QString& xmlfile, const QDomDocument& xml);

    void setXMLGUIClient( KXMLGUIClient *client );

  /**
   * Assign a (translated) text to this toolbar. This is used
   * for the tooltip on the handle, and when listing the toolbars.
   */
    void setText( const QString & txt );

  /**
   * Returns the toolbar's text.
   * @return the toolbar's text.
   */
    QString text() const;

    void setStretchableWidget( QWidget *w );
    QSizePolicy sizePolicy() const;
    bool highlight() const;
    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    QSize minimumSize() const;

    void hide();
    void show();

    void updateRects( bool = false ) {}

    /**
     * Load state from an XML element, called by KXMLGUIBuilder
     */
    void loadState( const QDomElement &e );
    /*
     * Save state into an XML element, called by KXMLGUIBuilder
     */
    void saveState( QDomElement &e );

    /**
     * @internal
     */
    void positionYourself( bool force = false);

signals:
    /**
     * Emitted when button @p id is clicked.
     */
    void clicked(int id);

    /**
     * Emitted when button @p id is double-clicked.
     *
     * Note: you will always
     * recive two clicked() , pressed() and released() signals.
     * There is no way to avoid it - at least no easy way.
     * If you need to resolve this all you can do is set up timers
     * which wait for QApplication::doubleClickInterval() to expire.
     * If in that time you don't get this signal, you may belive that
     * button was only clicked and not double-clicked.
     * And please note that butons with popup menus do not emit this signal,
     * but those with delayed popup do.
     */
    void doubleClicked (int id);

    /**
     * Emitted when button @p id is pressed.
     */
    void pressed(int);

    /**
     * Emits when button @p id is released.
     */
    void released(int);

    /**
     * Emitted when a toggle button changes state.
     *
     * Emitted also if you change state
     * with setButton() or toggleButton()
     * If you make a button normal again, with
     * setToggle(false), this signal won't
     * be emitted.
     */
    void toggled(int);

    /**
     * This signal is emitted when item @p id gets highlighted/unhighlighted
     * (i.e when mouse enters/exits).
     *
     * Note that this signal is emitted from
     * all buttons (normal, disabled and toggle) even when there is no visible
     * change in buttons (i.e., buttons do not raise when mouse enters).
     * The parameter @p isHighlighted is @p true when mouse enters and @p false when
     * mouse exits.
     */
    void highlighted(int id, bool isHighlighted);

    /**
     * This signal is emitted when item @p id gets highlighted/unhighlighted
     * (i.e when mouse enters/exits).
     *
     * Note that this signal is emitted from
     * all buttons (normal, disabled and toggle) even when there is no visible
     * change in buttons (i.e., buttons do not raise when mouse enters).
     */
    void highlighted(int id );

    /**
     * Emitted when toolbar changes position, or when
     * an item is removed from toolbar.
     *
     * If you subclass KMainWindow and reimplement
     * KMainWindow::resizeEvent() be sure to connect to
     * this signal. Note: You can connect this signal to a slot that
     * doesn't take parameter.
     */
    void moved( BarPosition );

    /**
     * This signal is emitted when toolbar detects changing of
     * following parameters:
     * highlighting, button-size, button-mode. This signal is
     * internal, aimed to buttons.
     * @internal
     */
    void modechange ();

    /**
     * This signal is emitted when the toolbar is getting deleted,
     * and before ~KToolbar finishes (so it's still time to remove
     * widgets from the toolbar).
     * Used by KWidgetAction.
     * @since 3.2
     */
    void toolbarDestroyed();

public:
    /**
     * Returns the global setting for "Highlight buttons under mouse"
     * @return global setting for "Highlight buttons under mouse"
     */
    static bool highlightSetting();

    /**
     * Returns the global setting for "Toolbars transparent when moving"
     * @return global setting for "Toolbars transparent when moving"
     */
    static bool transparentSetting();

    /**
     * Returns the global setting for "Icon Text"
     * @return global setting for "Icon Text"
     */
    static IconText iconTextSetting();

public slots:
    virtual void setIconText( const QString &txt )
    { Q3ToolBar::setIconText( txt ); }

protected:
    void mousePressEvent( QMouseEvent * );
    void childEvent( QChildEvent *e );
    void showEvent( QShowEvent *e );
    void resizeEvent( QResizeEvent *e );
    bool event( QEvent *e );
    void applyAppearanceSettings(KConfig *config, const QString &_configGroup, bool forceGlobal = false);
    QString settingsGroup() const;

private slots:
    void rebuildLayout();
    void slotReadConfig ();
    void slotAppearanceChanged();
    void slotIconChanged(int);
    void slotRepaint();
    void toolBarPosChanged( Q3ToolBar *tb );
    void slotContextAboutToShow();
    void slotContextAboutToHide();
    void widgetDestroyed();

    void slotContextLeft();
    void slotContextRight();
    void slotContextTop();
    void slotContextBottom();
    void slotContextFloat();
    void slotContextFlat();
    void slotContextIcons();
    void slotContextText();
    void slotContextTextRight();
    void slotContextTextUnder();
    void slotContextIconSize();

private:
    void init( bool readConfig = true, bool honorStyle = false );
    void doConnections( KToolBarButton *button );
    void insertWidgetInternal( QWidget *w, int &index, int id );
    void removeWidgetInternal( QWidget *w );
    void getAttributes( QString &position, QString &icontext, int &index );
    int dockWindowIndex();
    KMenu *contextMenu();
    void doModeChange();

    QMap<QWidget*, int > widget2id;
    typedef QMap<int, QWidget* > Id2WidgetMap;
    Id2WidgetMap id2widget;
    KMenu *context;
    Q3PtrList<QWidget> widgets;
    QTimer *layoutTimer;
    QPointer<QWidget> stretchableWidget, rightAligned;
protected:
    virtual void virtual_hook( int id, void* data );
private:
    KToolBarPrivate *d;
};

#endif
