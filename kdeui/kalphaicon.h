/* vi: ts=4 sts=2 sw=2
 *
 * This file is part of the KDE project, module kdeui.
 * Copyright (C) 2000 Antonio Larrosa <larrosa@kde.org>
 *
 * This is free software; it comes under the GNU Library General
 * Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 *
 * kalphaicon.h: A helper class to draw icons with an alpha channel.
 */ 

class QPainter;
class QPixmap;
class QImage;

/**
 * This class includes some static methods to handle easier icons (or images)
 * with an alpha channel.
 * 
 * Everything is
 * static, so there is no need to create an instance of this class. You can
 * just call the static methods. They are encapsulated here merely to provide
 * a common namespace.
 *
 * @short Methods to easily draw/blend images with an alpha channel.
 * @author Antonio Larrosa <larrosa@kde.org>
 */
class KAlphaIcon
{
 public:
  /**
   * Draws an image ( @p icon ) which has an alpha channel using the painter
   * @p p. 
   * @background is the background image over which the icon is being painted
   * (note that @p background contains the whole picture, and doesn't have to
   * be of the same size than @p icon).
   *
   * @p x and @p y specifies the point on p (and on @p bg) over which  @p icon will be
   * painted.
   *
   * @return true if ok, and false if something went wrong.
   */
  static bool draw(QPainter *p, const QImage &icon, const QImage &background, int x, int y);

  /**
   * Method provided by convenience which takes the background as a QPixmap object.
   */
  static bool draw(QPainter *p, const QImage &icon, const QPixmap &background, int x, int y);
};
