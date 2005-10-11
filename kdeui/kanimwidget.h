/* This file is part of the KDE libraries
   Copyright (C) 2000 Kurt Granroth <granroth@kde.org>

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
#ifndef KANIMWIDGET_H
#define KANIMWIDGET_H

#include <qframe.h>

#include <kdelibs_export.h>

class QStringList;
class QPainter;
class QMouseEvent;

class KAnimWidgetPrivate;
/**
 * @short Standard "About KDE" dialog box
 *
 * This is a widget used to display animation using multiple
 * individual pixmaps.  This widget allows you to deal with variable
 * size icons (e.g., ones that will change based on a global setting)
 * as it loads the icons internally.  All you need to do is pass along
 * a list of icon names and their size and everything else is taken
 * care of.
 *
 * This widget also emits a 'clicked()' signal when it received a
 * mouse press event.
 *
 * A quick example:
 * \code
 * KAnimWidget *anim = new KAnimWidget("kde", 0, this);
 * anim->start();
 * \endcode
 *
 * That example will search for the pixmaps "one.png", "two.png", and
 * "three.png" in the share/icons/small/ directories as well as the
 * app's pics directory.
 *
 * @author Kurt Granroth <granroth@kde.org>
 */
class KDEUI_EXPORT KAnimWidget : public QFrame
{
  Q_OBJECT
  Q_PROPERTY( int size READ size WRITE setSize )
  Q_PROPERTY( QString icons READ icons WRITE setIcons )

public:
  /**
   * This is the most common constructor.  Pass along the name of the
   * animated icons to use (e.g., "kde") for the animation and an
   * optional size to load and you're set.  If you omit the size, the
   * default size will be used.
   *
   * @param icons  The icons name (e.g., "kde") to use for the animation
   * @param size   The size to load
   *               You don't have to set it if the parent is a
   *               KToolBar; in this case it will use the toolbar's
   *               size.
   * @param parent The standard parent
   * @param name   The standard internal name
   */
  KAnimWidget( const QString& icons, int size = 0,
               QWidget *parent = 0L, const char *name = 0L );

  /**
   * Destructor
   */
  virtual ~KAnimWidget();

  /**
   * Sets the size of the icons.
   *
   * @param size The size of the icons
   */
  void setSize( int size );

  /**
  * Returns the current size.
  * @since 3.4
  */
  int size() const;

  /**
  * Returns the current icons
  * since 3.4
  */
  QString icons() const;

  /**
   * Sets the name of the animated icons to load.  This will use the
   * KIconLoader::loadAnimated method for the actual loading.
   *
   * @param icons The name of the icons to use for the animation
   */
  void setIcons( const QString& icons );

public slots:
  /**
   * Starts the animation from frame 1
   */
  void start();

  /**
   * Stops the animation.  This will also reset the widget to frame 1.
   */
  void stop();

signals:
  void clicked();

protected:
  virtual void drawContents( QPainter *p );
  virtual void leaveEvent( QEvent *e );
  virtual void enterEvent( QEvent *e );
  virtual void hideEvent( QHideEvent *e);
  virtual void showEvent( QShowEvent *e);
  virtual void mousePressEvent( QMouseEvent *e );
  virtual void mouseReleaseEvent( QMouseEvent *e );

protected slots:
  void slotTimerUpdate();
  void updateIcons();

protected:
  virtual void virtual_hook( int id, void* data );
private:
  KAnimWidgetPrivate *d;
};

#endif // _KANIMWIDGET_H
