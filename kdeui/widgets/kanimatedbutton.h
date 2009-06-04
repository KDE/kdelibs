/* This file is part of the KDE libraries
   Copyright (C) 2000 Kurt Granroth <granroth@kde.org>
   Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KANIMATEDBUTTON_H
#define KANIMATEDBUTTON_H

#include <kdeui_export.h>
#include <QtGui/QToolButton>

/**
 * @short An extended version of QToolButton which can display an animated icon.
 *
 * This widget extends QToolButton with the ability to display animation
 * using a sequence of individual pixmaps.  All you need to do is pass along
 * a list of icon names and their size and everything else is taken
 * care of.
 *
 * \note if you change the iconSize() via setIconSize(), you will need to call
 *       updateIcons() also to force reloading of the correct icon size.
 *
 * @author Kurt Granroth <granroth@kde.org>
 */
class KDEUI_EXPORT KAnimatedButton : public QToolButton
{
  Q_OBJECT
  Q_PROPERTY( QString icons READ icons WRITE setIcons )

public:
  /**
   * Construct an animated tool button.
   *
   * @param parent The parent widget
   */
  explicit KAnimatedButton(QWidget *parent = 0L);

  /**
   * Destructor
   */
  virtual ~KAnimatedButton();

  /**
   * Returns the current maximum dimension (width or length) for an icon.
   */
  int iconDimensions() const;

  /**
  * Returns the current icons
  */
  QString icons() const;

  /**
   * Sets the name of the animated icons to load.  This will use the
   * KIconLoader::loadAnimated method for the actual loading.
   *
   * @param icons The name of the icons to use for the animation
   */
  void setIcons( const QString& icons );

public Q_SLOTS:
  /**
   * Starts the animation from frame 1
   */
  void start();

  /**
   * Stops the animation.  This will also reset the widget to frame 1.
   */
  void stop();

  /**
   * Updates the icons by reloading them if required.
   *
   * You must call this after you change the icon size, in order for the correct
   * size icon to be loaded.
   */
  void updateIcons();

Q_SIGNALS:
  void clicked();

protected Q_SLOTS:
  void slotTimerUpdate();

private:
  class KAnimatedButtonPrivate *const d;

    Q_PRIVATE_SLOT(d, void _k_movieFrameChanged(int))
    Q_PRIVATE_SLOT(d, void _k_movieFinished())

    Q_DISABLE_COPY(KAnimatedButton)
};

#endif //  KANIMATEDBUTTON_H
