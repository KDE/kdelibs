/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Stephan Kulow (coolo@kde.org)
              (C) 1997, 1998 Sven Radej (radej@kde.org)
              (C) 1997, 1998 Mark Donohoe (donohoe@kde.org)
              (C) 1997, 1998 Matthias Ettrich (ettrich@kde.org)
              (C) 2000 Kurt Granroth (granroth@kde.org)

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

// $Id$
#ifndef _KTOOLBARBUTTON_H
#define _KTOOLBARBUTTON_H

#include <qpixmap.h>
#include <qtoolbutton.h>
#include <qintdict.h>
#include <qstring.h>
#include <kglobal.h>

class KToolBar;
class KToolBarButtonPrivate;
class KInstance;
class QEvent;
class QPopupMenu;
class QPainter;

/**
 * A toolbar button. This is used internally by @ref KToolBar, use the
 * KToolBar methods instead.
 * @internal
 */
class KToolBarButton : public QToolButton
{
  Q_OBJECT

public:
  /**
   * Construct a button with an icon loaded by the button itself.
   * This will trust the button to load the correct icon with the
   * correct size.
   *
   * @param icon   Name of icon to load (may be absolute or relative)
   * @param id     Id of this button
   * @param parent This button's parent
   * @param name   This button's internal name
   * @param txt    This button's text (in a tooltip or otherwise)
   */
  KToolBarButton(const QString& icon, int id, QWidget *parent,
                 const char *name=0L, const QString &txt=QString::null,
                 KInstance *_instance = KGlobal::instance());

  /**
   * Construct a button with an existing pixmap.  It is not
   * recommended that you use this as the internal icon loading code
   * will almost always get it "right".
   *
   * @param icon   Name of icon to load (may be absolute or relative)
   * @param id     Id of this button
   * @param parent This button's parent
   * @param name   This button's internal name
   * @param txt    This button's text (in a tooltip or otherwise)
   */
  KToolBarButton(const QPixmap& pixmap, int id, QWidget *parent,
                 const char *name=0L, const QString &txt=QString::null);

  /**
   * Construct a separator button
   *
   * @param parent This button's parent
   * @param name   This button's internal name
   */
  KToolBarButton(QWidget *parent=0L, const char *name=0L);

  /**
   * Standard destructor
   */
  ~KToolBarButton();

#ifndef KDE_NO_COMPAT
  /**
   * @deprecated
   * Set the pixmap directly for this button.  This pixmap should be
   * the active one... the dimmed and disabled pixmaps are constructed
   * based on this one.  However, don't use this function unless you
   * are positive that you don't want to use @ref setIcon.
   *
   * @param pixmap The active pixmap
   */
  // this one is from QButton, so #ifdef-ing it out doesn't break BC
  virtual void setPixmap(const QPixmap &pixmap);

  /**
   * @deprecated
   * Force the button to use this pixmap as the default one rather
   * then generating it using effects.
   *
   * @param pixmap The pixmap to use as the default (normal) one
   */
  void setDefaultPixmap(const QPixmap& pixmap);

  /**
   * @deprecated
   * Force the button to use this pixmap when disabled one rather then
   * generating it using effects.
   *
   * @param pixmap The pixmap to use when disabled
   */
  void setDisabledPixmap(const QPixmap& pixmap);
#endif

  /**
   * Set the text for this button.  The text will be either used as a
   * tooltip (IconOnly) or will be along side the icon
   *
   * @param text The button (or tooltip) text
   */
  virtual void setText(const QString &text);

  /**
   * Set the icon for this button. The icon will be loaded internally
   * with the correct size. This function is preferred over @ref setIconSet
   *
   * @param icon The name of the icon
   */
  virtual void setIcon(const QString &icon);

  /// @since 3.1
  virtual void setIcon( const QPixmap &pixmap )
  { QToolButton::setIcon( pixmap ); }

  /**
   * Set the pixmaps for this toolbar button from a QIconSet.
   * If you call this you don't need to call any of the other methods
   * that set icons or pixmaps.
   * @param iconset  The iconset to use
   */
  virtual void setIconSet( const QIconSet &iconset );

#ifndef KDE_NO_COMPAT
  /**
   * @deprecated
   * Set the active icon for this button.  The pixmap itself is loaded
   * internally based on the icon size...  .. the disabled and default
   * pixmaps, however will only be constructed if @ref #generate is
   * true.  This function is preferred over @ref setPixmap
   *
   * @param icon     The name of the active icon
   * @param generate If true, then the other icons are automagically
   *                 generated from this one
   */
  void setIcon(const QString &icon, bool /*generate*/ ) { setIcon( icon ); }

  /**
   * @deprecated
   * Force the button to use this icon as the default one rather
   * then generating it using effects.
   *
   * @param icon The icon to use as the default (normal) one
   */
  void setDefaultIcon(const QString& icon);

  /**
   * @deprecated
   * Force the button to use this icon when disabled one rather then
   * generating it using effects.
   *
   * @param icon The icon to use when disabled
   */
  void setDisabledIcon(const QString& icon);
#endif

  /**
   * Turn this button on or off
   *
   * @param flag true or false
   */
  void on(bool flag = true);

  /**
   * Toggle this button
   */
  void toggle();

  /**
   * Turn this button into a toggle button or disable the toggle
   * aspects of it.  This does not toggle the button itself.
   * Use @ref toggle() for that.
   *
   * @param toggle true or false
   */
  void setToggle(bool toggle = true);

  /**
   * Return a pointer to this button's popup menu (if it exists)
   */
  QPopupMenu *popup();

  /**
   * Give this button a popup menu.  There will not be a delay when
   * you press the button.  Use @ref setDelayedPopup if you want that
   * behavior. You can also make the popup-menu
   * "sticky", i.e. visible until a selection is made or the mouse is
   * clikced elsewhere, by simply setting the second argument to true.
   * This "sticky" button feature allows you to make a selection without
   * having to press and hold down the mouse while making a selection.
   *
   * @param p The new popup menu
   * @param toggle if true, makes the button "sticky" (toggled)
   */
  void setPopup (QPopupMenu *p, bool toggle = false);

  /**
   * Gives this button a delayed popup menu.
   *
   * This function allows you to add a delayed popup menu to the button.
   * The popup menu is then only displayed when the button is pressed and
   * held down for about half a second.  You can also make the popup-menu
   * "sticky", i.e. visible until a selection is made or the mouse is
   * clikced elsewhere, by simply setting the second argument to true.
   * This "sticky" button feature allows you to make a selection without
   * having to press and hold down the mouse while making a selection.
   *
   * @param p the new popup menu
   * @param toggle if true, makes the button "sticky" (toggled)
   */
  void setDelayedPopup(QPopupMenu *p, bool toggle = false);

  /**
   * Turn this button into a radio button
   *
   * @param f true or false
   */
  void setRadio(bool f = true);

  /**
   * Toolbar buttons naturally will assume the global styles
   * concerning icons, icons sizes, etc.  You can use this function to
   * explicitely turn this off, if you like.
   *
   * @param no_style Will disable styles if true
   */
  void setNoStyle(bool no_style = true);

signals:
  void clicked(int);
  void doubleClicked(int);
  void pressed(int);
  void released(int);
  void toggled(int);
  void highlighted(int, bool);

public slots:
  /**
   * This slot should be called whenever the toolbar mode has
   * potentially changed.  This includes such events as text changing,
   * orientation changing, etc.
   */
   void modeChange();
   virtual void setTextLabel(const QString&, bool tipToo);

protected:
  void paletteChange(const QPalette &);
  void leaveEvent(QEvent *e);
  void enterEvent(QEvent *e);
  void drawButton(QPainter *p);
  bool eventFilter (QObject *o, QEvent *e);
  void showMenu();
  QSize sizeHint() const;
  QSize minimumSizeHint() const;
  QSize minimumSize() const;

  /// @since 3.1
  bool isRaised() const;
  /// @since 3.1
  bool isActive() const;
  /// @since 3.1
  int iconTextMode() const;

protected slots:
  void slotClicked();
  void slotPressed();
  void slotReleased();
  void slotToggled();
  void slotDelayTimeout();

protected:
  virtual void virtual_hook( int id, void* data );
private:
  KToolBarButtonPrivate *d;
};

/**
* List of @ref KToolBarButton objects.
* @internal
* @version $Id$
*/
class KToolBarButtonList : public QIntDict<KToolBarButton>
{
public:
   KToolBarButtonList();
   ~KToolBarButtonList() {}
};

#endif
