/* vi: ts=4 sts=2 sw=2
 *
 * This file is part of the KDE project, module kdeui.
 * Copyright (C) 2000 Antonio Larrosa <larrosa@kde.org>
 *
 * This is free software; it comes under the GNU Library General
 * Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 *
 * kalphapainter.h: A helper class to draw icons with an alpha channel.
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
class KAlphaPainter
{
 public:
  /**
   * Draws an image ( @p icon ) which has an alpha channel using the painter
   * @p p. 
   * @p background is the background image over which the icon is being painted
   * (note that @p background contains the whole picture, and doesn't have to
   * be of the same size than @p icon).
   *
   * @p x and @p y specify the point on @p p (and on @p bg) over which  @p icon
   * will be painted.
   *
   * If @p copyOnBg is false (the default), the image will be drawn only on the
   * PaintDevice which @p p is using. If @p copyOnBg is true, the icon will
   * also be painted over the @p background image, so that next calls to this
   * method for overlapped icons result in a correct image.
   *
   * If @p relativeBg is true, the background upperleft corner (background(0,0)
   * point) is x,y, that is, it's not the whole background, but just a portion
   * of it, starting where the icon will be painted. This can be used in applications
   * that don't want to store the whole background if it can become too big.
   *
   * @return true if ok, and false if something went wrong.
   */
  static bool draw(QPainter *p, const QImage &icon, QImage &background,
			int x, int y, bool copyOnBg=false, int bgx=0, int bgy=0);

  /**
   * Method provided by convenience which takes the background as a QPixmap
   * object. Note that in this case, copyOnBg is still not implemented.
   */
  static bool draw(QPainter *p, const QImage &icon, const QPixmap &background,
			int x, int y, bool copyOnBg=false, int bgx=0, int bgy=0);

  /**
   * Method provided by convenience which takes the background as a QPixmap
   * object.
   */
  static bool draw(QPainter *p, const QPixmap &icon, QImage &background,
			int x, int y, bool copyOnBg=false, int bgx=0, int bgy=0);

  static bool draw(QPainter *p, const QPixmap &icon, const QPixmap &background,
			int x, int y, int bgx=0, int bgy=0);

private:
  class KAlphaPainterPrivate;
  KAlphaPainterPrivate *d;
};
