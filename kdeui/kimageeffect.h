/* This file is part of the KDE libraries
    Copyright (C) 1998, 1999 Christian Tibirna <ctibirna@total.net>
              (C) 1998, 1999 Daniel M. Duley <mosfet@kde.org>
              (C) 1998, 1999 Dirk A. Mueller <mueller@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
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

// $Id$

#ifndef __KIMAGE_EFFECT_H
#define __KIMAGE_EFFECT_H

class QImage;

/**
 * This class includes various QImage based graphical effects. Everything is
 * static, so there is no need to create an instance of this class. You can
 * just call the static methods. They are encapsulated here merely to provide
 * a common namespace.
 */

class KImageEffect
{
public:
    enum GradientType { VerticalGradient, HorizontalGradient,
                        DiagonalGradient, CrossDiagonalGradient,
                        PyramidGradient, RectangleGradient,
                        PipeCrossGradient, EllipticGradient };
    enum RGBComponent { Red, Green, Blue };

    enum Lighting {NorthLite, NWLite, WestLite, SWLite,
                   SouthLite, SELite, EastLite, NELite};

    /**
     * Create a gradient from color a to color b of the specified type.
     *
     * @param size The desired size of the gradient.
     * @param ca Color a
     * @param cb Color b
     * @param type The type of gradient.
     * @param ncols The number of colors to use when not running on a
     * truecolor display. The gradient will be dithered to this number of
     * colors. Pass 0 to prevent dithering.
     */
    static QImage gradient(const QSize &size, const QColor &ca,
                           const QColor &cb, GradientType type, int ncols=3);

    /**
     * Create an unbalanced gradient.
     * An unbalanced gradient is a gradient where the transition from
     * color a to color b is not linear, but in this case, exponential.
     *
     * @param size The desired size of the gradient.
     * @param ca Color a
     * @param cb Color b
     * @param type The type of gradient.
     * @param xfactor The x decay length. Use a value between -200 and 200.
     * @param yfactor The y decay length.
     * @param ncols The number of colors. See KPixmapEffect:gradient.
     */
    static QImage unbalancedGradient(const QSize &size, const QColor &ca,
	    const QColor &cb, GradientType type, int xfactor = 100,
	    int yfactor = 100, int ncols = 3);

     /**
     * Blends the provided image into a background of the indicated color
     * @param initial_intensity this parameter takes values from -1 to 1:
     *              a) if positive: how much to fade the image in its
     *                              less affected spot
     *              b) if negative: roughly indicates how much of the image
     *                              remains unaffected
     * @param bgnd indicates the color of the background to blend in
     * @param eff lets you choose what kind of blending you like
     * @param anti_dir blend in the opposite direction (makes no much sense
     *                  with concentric blending effects)
     */
    static QImage& blend(QImage &image, float initial_intensity,
                      const QColor &bgnd, GradientType eff,
                      bool anti_dir=false);

    /**
     * Modifies the intensity of a pixmap's RGB channel component.
     *
     * @param image The QImage to process.
     * @param percent percent value. Use a negative value to dim.
     * @param channel which channel(s) should be modified
     * @return Returns the @ref #image, provided for convenience.
     */
    static QImage& channelIntensity(QImage &image, float percent,
                                    RGBComponent channel);

    /**
     * Fade an image to a certain background color. The number of colors
     * will not be changed.
     *
     * @param image The QImage to process.
     * @param val The strength of the effect. 0 <= val <= 1.
     * @param color The background color.
     * @return Returns the @ref #image, provided for convenience.
     */
    static QImage& fade(QImage &img, float val, const QColor &color);


    /**
     * This recolors an image. The most dark color will become color a,
     * This recolors a pixmap. The most dark color will become color a,
     * the most bright one color b, and in between.
     *
     * @param image A QImage to process.
     * @param ca Color a
     * @param cb Color b
     */
    static QImage& flatten(QImage &image, const QColor &ca,
           const QColor &cb, int ncols=0);

    /**
     * Builds a hash on any given QImage
     *
     * @param image The QImage to process
     * @param lite - the hash faces the indicated lighting (cardinal poles)
     * @param spacing - how many unmodified pixels inbetween hashes
     * @return Returns the @ref #image, provided for convenience.
     */
    static QImage& hash(QImage &image, Lighting lite=NorthLite,
                        unsigned int spacing=0);

    /**
     * Either brightens or dims the image by a specified percent.
     * For example, .5 will modify the colors by 50%.
     *
     * @param image The QImage to process.
     * @param percent the percent value. Use a negative value to dim.
     * @return Returns the @ref #image, provided for convenience.
     */
    static QImage& intensity(QImage &image, float percent);

    /**
     * Converts an image to grayscale.
     *
     * @param image The QImage to process.
     * @param fast Set to true in order to use a faster but non-photographic
     * quality algorithm. Appropriate for things such as toolbar icons.
     * @return Returns the @ref #image, provided for convenience.
     */
    static QImage& toGray(QImage &image, bool fast = false);

private:

    /**
     * Helper function to fast calc some altered (lighten, shaded) colors
     *
     */
    static unsigned int lHash(unsigned int c);
    static unsigned int uHash(unsigned int c);
};

#endif
