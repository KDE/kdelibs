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

#include <qtoolbar.h>
#include <qmainwindow.h>
#include <qcombobox.h>
#include <qmap.h>
#include <qptrlist.h>
#include <kglobal.h>
#include <qguardedptr.h>
#include <qframe.h>
#include <qiconset.h>

class QDomElement;
class QSize;
class QPixmap;
class QPopupMenu;
class QStringList;
class QDomDocument;
class QTimer;

class KLineEdit;
class KToolBar;
class KToolBarButton;
class KToolBoxManager;
class KAnimWidget;
class KPopupMenu;
class KInstance;
class KComboBox;
class KXMLGUIClient;

class KToolBarPrivate;

class KToolBarSeparator : public QFrame
{
    Q_OBJECT
public:
    KToolBarSeparator( Orientation, bool l, QToolBar *parent, const char* name=0 );

    QSize sizeHint() const;
    Orientation orientation() const { return orient; }
    QSizePolicy sizePolicy() const;
    bool showLine() const { return line; }
public slots:
   void setOrientation( Orientation );
protected:
    void styleChange( QStyle& );
private:
    Orientation orient;
    bool line;
};


 /**
  * A KDE-style toolbar.
  *
  * KToolBar can be dragged around in and between different docks.
  *
  * A KToolBar can contain all sorts of widgets.
  *
  * KToolBar can be used as a standalone widget, but @ref KMainWindow
  * provides easy factories and management of one or more toolbars.
  * Once you have a KToolBar object, you can insert items into it with the
  * insert... methods, or remove them with the @ref removeItem() method. This
  * can be done at any time; the toolbar will be automatically updated.
  * There are also many methods to set per-child properties like alignment
  * and toggle behaviour.
  *
  * KToolBar uses a global config group to load toolbar settings on
  * construction. It will reread this config group on a
  * @ref KApplication::appearanceChanged() signal.
  *
  * @short Floatable toolbar with auto resize.
  * @version $Id$
  * @author Reginald Stadlbauer <reggie@kde.org>, Stephan Kulow <coolo@kde.org>, Sven Radej <radej@kde.org>.
  */

class KToolBar : public QToolBar
{
    Q_OBJECT
    Q_ENUMS( IconText BarPosition )
    Q_PROPERTY( IconText iconText READ iconText WRITE setIconText )
    Q_PROPERTY( BarPosition barPos READ barPos WRITE setBarPos )
    Q_PROPERTY( bool fullSize READ fullSize WRITE setFullSize )
    Q_PROPERTY( int iconSize READ iconSize WRITE setIconSize )
    Q_PROPERTY( QString text READ text WRITE setText )

public:
    enum IconText{IconOnly = 0, IconTextRight, TextOnly, IconTextBottom};
    /**
     * The state of the status bar.
     * @deprecated
     **/
    enum BarStatus{Toggle, Show, Hide};
    /**
   * Possible bar positions.
   **/
    enum BarPosition{ Unmanaged, Floating, Top, Bottom, Right, Left, Flat};

  /**
   * Constructor.
   * This constructor is used by the XML-GUI. If you use it, you need
   * to call QMainWindow::addToolBar to specify the position of the toolbar.
   * So it's simpler to use the other constructor.
   *
   * The toolbar will read in various global config settings for
   * things like icon size and text position, etc.  However, some of
   * the settings will be honored only if @ref #_honor_mode is set to
   * true.  All other toolbars will be IconOnly and use Medium icons.
   *
   * @param parent      The standard toolbar parent (usually a
   *                    @ref KMainWindow)
   * @param name        The standard internal name
   * @param honor_style If true, then global settings for IconSize and IconText will be honored
   * @param readConfig  whether to apply the configuration (global and application-specific)
   */
    KToolBar( QWidget *parent, const char *name = 0, bool honor_style = FALSE, bool readConfig = TRUE );

  /**
   * Constructor for non-XML-GUI applications.
   *
   * The toolbar will read in various global config settings for
   * things like icon size and text position, etc.  However, some of
   * the settings will be honored only if @ref #_honor_mode is set to
   * true.  All other toolbars will be IconOnly and use Medium icons.
   *
   * @param parentWindow The window that should be the parent of this toolbar
   * @param dock        The position of the toolbar. Usually QMainWindow::Top.
   * @param newLine     If true, start a new line in the dock for this toolbar.
   * @param name        The standard internal name
   * @param honor_style If true, then global settings for IconSize and IconText will be honored
   * @param readConfig  whether to apply the configuration (global and application-specific)
   */
    KToolBar( QMainWindow *parentWindow, QMainWindow::ToolBarDock dock /*= QMainWindow::Top*/, bool newLine = false,
              const char *name = 0, bool honor_style = FALSE, bool readConfig = TRUE );

  /**
   * Constructor for non-XML-GUI applications.
   *
   * The toolbar will read in various global config settings for
   * things like icon size and text position, etc.  However, some of
   * the settings will be honored only if @ref #_honor_mode is set to
   * true.  All other toolbars will be IconOnly and use Medium icons.
   *
   * @param parentWindow The window that should be the parent of this toolbar
   * @param dock        Another widget than the mainwindow to dock toolbar to.
   * @param newLine     If true, start a new line in the dock for this toolbar.
   * @param name        The standard internal name
   * @param honor_style If true, then global settings for IconSize and IconText will be honored
   * @param readConfig  whether to apply the configuration (global and application-specific)
   */
    KToolBar( QMainWindow *parentWindow, QWidget *dock, bool newLine = false,
              const char *name = 0, bool honor_style = FALSE, bool readConfig = TRUE );

    virtual ~KToolBar();

  /**
   * Insert a button (a @ref KToolBarButton) with a pixmap.  The
   * pixmap is loaded by the button itself based on the global icon
   * settings.
   *
   * You should connect to one or more signals in KToolBar:
   * @ref clicked() , @ref pressed() , @ref released() , or
   * @ref highlighted()  and if the button is a toggle button
   * (@ref setToggle() ) @ref toggled() .  Those signals have @p id
   * of a button that caused the signal.  If you want to bind a popup
   * to button, see @ref setButton().
   *
   * @param icon    The name of the icon to use as the active pixmap
   * @param id      The id of this button
   * @param enabled Enable or disable the button at startup
   * @param text    The tooltip or toolbar text (depending on state)
   * @param index The position of the button. (-1 = at end).
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
   * You can add more signals with @ref addConnection().
   *
   * @param icon     The name of the icon to use as the active pixmap
   * @param id       The id of this button
   * @param signal   The signal to connect to
   * @param receiver The slot's parent
   * @param enabled  Enable or disable the button at startup
   * @param text     The tooltip or toolbar text (depending on state)
   * @param index    The position of the button. (-1 = at end).
   *
   * @return The item index.
   */
    int insertButton(const QString& icon, int id, const char *signal,
		     const QObject *receiver, const char *slot,
		     bool enabled = true, const QString& text = QString::null,
		     int index=-1, KInstance *_instance = KGlobal::instance() );

  /**
   * Inserts a button (a @ref KToolBarButton) with the specified
   * pixmap.  This pixmap will be used as the "active" one and the
   * disabled and default ones will be autogenerated.
   *
   * It is recommended that you use the insertButton function that
   * allows you to specify the icon name rather then the pixmap
   * itself.  Specifying the icon name is much more flexible.
   *
   * You should connect to one or more signals in KToolBar:
   * @ref clicked() , @ref pressed() , @ref released() , or
   * @ref highlighted()  and if the button is a toggle button
   * (@ref setToggle() ) @ref toggled() .  Those signals have @p id
   * of a button that caused the signal.  If you want to bind a popup
   * to button, see @ref setButton().
   *
   * @param pixmap  The active pixmap
   * @param id      The id of this button
   * @param enabled Enable or disable the button at startup
   * @param text    The tooltip or toolbar text (depending on state)
   * @param index The position of the button. (-1 = at end).
   *
   * @return The item index.
   */
    int insertButton(const QPixmap& pixmap, int id, bool enabled = true,
		     const QString& text = QString::null, int index=-1 );

  /**
   * This is the same as above, but with specified signals and
   * slots to which this button will be connected.
   *
   * You can add more signals with @ref addConnection().
   *
   * @param icon     The name of the icon to use as the active pixmap
   * @param id       The id of this button
   * @param signal   The signal to connect to
   * @param receiver The slot's parent
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
   * signals @ref KButton::pressed(), @ref KButton::released(),
   * @ref KButton::clicked() or @ref KButton::doubleClicked() are @p not
   * emmited by
   * this button (see @ref setDelayedPopup() for that).
   * You can add custom popups which inherit @ref QPopupMenu to get popups
   * with tables, drawings etc. Just don't fiddle with events there.
   */
    int insertButton(const QString& icon, int id, QPopupMenu *popup,
		     bool enabled, const QString&_text, int index=-1);

  /**
   * Inserts a button with popupmenu.
   *
   * Button will have small
   * triangle. You have to connect to popup's signals. The
   * signals @ref KButton::pressed(), @ref KButton::released(),
   * @ref KButton::clicked() or @ref KButton::doubleClicked() are @p not
   * emmited by
   * this button (see @ref setDelayedPopup() for that).
   * You can add custom popups which inherit @ref QPopupMenu to get popups
   * with tables, drawings etc. Just don't fiddle with events there.
   */
    int insertButton(const QPixmap& pixmap, int id, QPopupMenu *popup,
		     bool enabled, const QString&_text, int index=-1);

  /**
   * Inserts a @ref KLineEdit. You have to specify signals and slots to
   * which KLineEdit will be connected. KLineEdit has all slots QLineEdit
   * has, plus signals @ref KLineEdit::completion and @ref KLineEdit::textRotation
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
   * Inserts a @ref KComboBox with list.
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
		     QComboBox::Policy policy = QComboBox::AtBottom);

  /**
   * Insert a @ref KComboBox with text.
   *
   *  The rest is the same as above.
   * @see setItemAutoSized()
   *
   * @see KComboBox
   * @return Item index.
   */
    int insertCombo (const QString& text, int id, bool writable,
		     const char *signal, QObject *recevier,
		     const char *slot, bool enabled=true,
		     const QString& tooltiptext=QString::null,
		     int size=70, int index=-1,
		     QComboBox::Policy policy = QComboBox::AtBottom);

    /**
     * Inserts a separator into the toolbar with the given id.
     * Returns the separator's index
     */
    int insertSeparator( int index = -1, int id = -1 ); 

    /**
     * Inserts a line separator into the toolbar with the given id.
     * Returns the separator's index
     */
    int insertLineSeparator( int index = -1, int id = -1 ); 

  /**
   * Inserts a user-defined widget.  The widget @p must have this
   * toolbar as its parent.
   *
   * Widget must have a QWidget for base class.  Widget can be
   * autosized to full width. If you forget about it, you can get a
   * pointer to this widget with @ref getWidget().
   * @see setItemAutoSized()
   * @return Item index.
   */
    int insertWidget(int id, int width, QWidget *_widget, int index=-1);

  /**
   * Inserts an animated widget.  A @ref KAnimWidget will be created
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
   * This will return a pointer to the given animated widget, if it
   * exists.
   *
   * @see insertAnimatedWidget
   *
   * @param id The id for the widget you want to get a pointer to
   *
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
    void setButtonIconSet( int id, const QIconSet& iconset );

  /**
   * Sets a delayed popup for a button.
   *
   * Delayed popup is what you see in
   * Netscape Navigator's Previous and Next buttons: If you click them you
   * go back
   * or forth. If you press them long enough, you get a history-menu.
   * This is exactly what we do here.
   *
   * You will insert normal a button with connection (or use signals from
   * toolbar):
   * <pre>
   * bar->insertButton(icon, id, SIGNAL(clicked ()), this,
   *                   SLOT (slotClick()), true, "click or wait for popup");
   * </pre> And then add a delayed popup:
   * <pre>
   * bar->setDelayedPopup (id, historyPopup); </pre>
   *
   * Don't add delayed popups to buttons which have normal popups.
   *
   * You may add popups which are derived from @ref QPopupMenu. You may
   * add popups that are already in the menu bar or are submenus of
   * other popups.
   */
    void setDelayedPopup (int id , QPopupMenu *_popup, bool toggle = false);

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
   * intervals.  Since this uses @ref QButton::setAutoRepeat() ,
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
   * If the button is a toggle button  (see @ref setToggle())
   * the button state will be toggled. This will also cause the toolbar to
   * emit the signal @ref KButton::toggled() with parameter @p id. You must connect to
   * this signal, or use @ref addConnection() to connect directly to the
   * button signal @ref KButton::toggled().
   */
    void toggleButton (int id);

  /**
   * Sets a toggle button state.
   *
   * If the button is a toggle button (see @ref setToggle())
   * this will set its state flag. This will also emit the signal
   * @ref KButton::toggled().
   *
   * @see setToggle()
   */
    void setButton (int id, bool flag);

  /**
   * @return @p true if button is on, @p false if button is off or if the
   * button is not a toggle button.
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
   * Returns a line editor text.
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
   *
   * @p index = -1 refers to current item.
   */

    QString getComboItem (int id, int index=-1) const;

    /**
   * Returns a pointer to the combobox.
   *
   * Example:
   * <pre>
   * KComboBox *combo = toolbar->getCombo(combo_id);
   * </pre>
   * That way you can get access to other public methods
   * that @ref KComboBox provides.
   */
    KComboBox * getCombo(int id);

    /**
   * Returns a pointer to KToolBarLined.
   *
   * Example:
   * <pre>
   * KLineEdit * lined = toolbar->getKTollBarLined(lined_id);
   * </pre>
   * That way you can get access to other public methods
   * that @ref KLineEdit provides. @ref KLineEdit is the same thing
   * as @ref QLineEdit plus completion signals.
   */
    KLineEdit * getLined (int id);

    /**
   * Returns a pointer to KToolBarButton.
   *
   * Example:
   * <pre>
   * KToolBarButton * button = toolbar->getButton(button_id);
   * </pre>
   * That way you can get access to other public methods
   * that @ref KToolBarButton provides.
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
   * Returns a pointer to an inserted widget.
   *
   * Wrong ids are not tested.
   * You can do with this whatever you want,
   * except change its height (hardcoded). If you change its width
   * you will probably have to call QToolBar::updateRects(true)
   * @see QWidget
   * @see updateRects()
   */
    QWidget *getWidget (int id);

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
   * Hide item.
   */
    void hideItem (int id);

    /**
   * Show item.
   */
    void showItem (int id);

    /**
   * Set toolbar to full parent size (default).
   *
   *  In full size mode the bar
   * extends over the parent's full width or height. If the mode is disabled
   * the toolbar tries to take as much space as it needs without wrapping, but
   * it does not exceed the parent box. You can force a certain width or
   * height with @ref setMaxWidth() or @ref setMaxHeight().
   *
   * If you want to use right-aligned items or auto-sized items you must use
   * full size mode.
   */
    void setFullSize(bool flag = true);

    /**
   * @return @p true if the full-size mode is enabled. Otherwise
   * it returns @false.
   */
    bool fullSize() const;

    /**
   * @deprecated use setMovingEnabled(bool) instead.
   * Enable or disable moving of toolbar.
   */
    void enableMoving(bool flag = true);

    /**
   * Set position of toolbar.
   * @see BarPosition()
   */
    void setBarPos (BarPosition bpos);

    /**
   * Returns position of toolbar.
   */
    BarPosition barPos() const;

  /**
   * @deprecated
   * Show, hide, or toggle toolbar.
   *
   * This method is provided for compatibility only,
   * please use show() and/or hide() instead.
   * @see BarStatus
   */
    bool enable(BarStatus stat);

  /**
   * @deprecated
   * Use setMaximumHeight() instead.
   */
    void setMaxHeight (int h);  // Set max height for vertical toolbars

  /**
   * @deprecated
   * Use maximumHeight() instead.
   * Returns the value that was set with @ref setMaxHeight().
   */
    int maxHeight();

  /**
   * @deprecated
   * Use setMaximumWidth() instead.
   * Set maximal width of horizontal (top or bottom) toolbar.
   */
    void setMaxWidth (int dw);

  /**
   * @deprecated
   * Use maximumWidth() instead.
   * Returns the value that was set with @ref setMaxWidth().
   */
    int maxWidth();

  /**
   * Set title for toolbar when it floats.
   *
   * Titles are however not (yet)
   * visible. You can't change toolbar's title while it's floating.
   */
    void setTitle (const QString& _title);

  /**
   * @deprecated
   * Use enableMoving() instead.
   */
    void enableFloating (bool arrrrrrgh);

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
   * Similar to @ref setIconText(IconText it) but allows you to
   * disable or enable updating.  If @p update is false, then the
   * buttons will not be updated.  This is useful only if you know
   * that you will be forcing an update later.
   */
    void setIconText(IconText it, bool update);

  /**
   * @return The current text style for buttons.
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
   * Same as @ref setIconText(int size) but allows you
   * to disable the toolbar update.
   *
   * @param size   The size to use
   * @param update If true, then the toolbar will be updated after
   *               this
   */
    void setIconSize(int size, bool update);

  /**
   * @return The current icon size for buttons.
   */
    int iconSize() const;

  /**
   * This allows you to enable or disable the context menu.
   *
   * @param enable If false, then the context menu will be disabled
   */
    void setEnableContextMenu(bool enable = true);

  /**
   * Returns whether or not the context menu is disabled
   *
   * @return The context menu state
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
   * @return the number of items in the toolbar
   */
    int count() const;

  /**
   * Instruct the toolbar to save it's current state to either the app
   * config file or to the XML-GUI resource file (whichever has
   * precedence).
   */
    void saveState();

  /**
   * Save the toolbar settings to group @p configGroup in @p config.
   */
    void saveSettings(KConfig *config, const QString &configGroup);

  /**
   * Read the toolbar settings from group @p configGroup in @p config
   * and apply them.
   */
    void applySettings(KConfig *config, const QString &configGroup);

  /**
   * Tell the toolbar what XML-GUI resource file it should use to save
   * it's state.  The state of the toolbar (position, size, etc) is
   * saved in KConfig files if the application does not use XML-GUI..
   * but if the app does, then it's saved the XML file.  This function
   * allows this to happen.
   *
   * @param xmlfile The XML-GUI resource file to write to
   * @param xml     The DOM document for the XML-GUI building
   */
  //  void setXML(const QString& xmlfile, const QDomDocument& xml);
  /* @internal */
    void setXMLGUIClient( KXMLGUIClient *client );

  /**
   * Assign a (translated) text to this toolbar. This is used
   * for the tooltip on the handle, and when listing the toolbars.
   */
    void setText( const QString & txt );

  /**
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

    void updateRects( bool = FALSE ) {}

    void loadState( const QDomElement &e );
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
     * recive two @ref clicked() , @ref pressed() and @ref released() signals.
     * There is no way to avoid it - at least no easy way.
     * If you need to resolve this all you can do is set up timers
     * which wait for @ref QApplication::doubleClickInterval() to expire.
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
     * with @ref setButton() or @ref toggleButton()
     * If you make a button normal again, with
     * setToggle(false), this signal won't
     * be emitted.
     */
    void toggled(int);

    /**
     * This signal is emitted when item id gets highlighted/unhighlighted
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
     * This signal is emitted when item id gets highlighted/unhighlighted
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
     * If you subclass @ref KMainWindow and reimplement
     * @ref KMainWindow::resizeEvent() be sure to connect to
     * this signal. Note: You can connect this signal to a slot that
     * doesn't take parameter.
     */
    void moved( BarPosition );

    /**
     * @internal
     * This signal is emited when toolbar detects changing of
     * following parameters:
     * highlighting, button-size, button-mode. This signal is
     * internal, aimed to buttons.
     */
    void modechange ();

    /**
     * @return global setting for "Highlight buttons under mouse"
     */
    static bool highlightSetting();

    /**
     * @return global setting for "Toolbars transparent when moving"
     */
    static bool transparentSetting();

    /**
     * @return global setting for "Icon Text"
     */
    static IconText iconTextSetting();

public slots:
    virtual void setIconText( const QString &txt )
    { QToolBar::setIconText( txt ); }

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
    void toolBarPosChanged( QToolBar *tb );
    void slotContextAboutToShow();
    void widgetDestroyed();

private:
    void init( bool readConfig = true, bool honorStyle = false );
    void doConnections( KToolBarButton *button );
    void insertWidgetInternal( QWidget *w, int &index, int id );
    void removeWidgetInternal( QWidget *w );
    void getAttributes( QString &position, QString &icontext, QString &index, QString &offset, QString &newLine );
    KPopupMenu *contextMenu();

    QMap<QWidget*, int > widget2id;
    typedef QMap<int, QWidget* > Id2WidgetMap;
    Id2WidgetMap id2widget;
    KPopupMenu *context;
    QPtrList<QWidget> widgets;
    QTimer *layoutTimer;
    QGuardedPtr<QWidget> stretchableWidget, rightAligned;
protected:
    virtual void virtual_hook( int id, void* data );
private:
    KToolBarPrivate *d;
};

#endif
