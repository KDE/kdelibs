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

#ifndef __KPIXMAP_EFFECT_H
#define __KPIXMAP_EFFECT_H

#include <kpixmap.h>
#include <qimage.h>

/**
 * This class includes various pixmap based graphical effects. Everything is
 * static, so there is no need to create an instance of this class. You can
 * just call the static methods. They are encapsulated here merely to provide
 * a common namespace.
 */
class KPixmapEffect
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
     * @param pixmap The pixmap to process.
     * @param ca Color a
     * @param cb Color b
     * @param type The type of gradient.
     * @param ncols The number of colors to use when not running on a
     * truecolor display. The gradient will be dithered to this number of
     * colors. Pass 0 to prevent dithering.
     * @return Returns the generated pixmap, for convenience.
     */
    static KPixmap& gradient(KPixmap& pixmap, const QColor &ca, const QColor &cb,
                            GradientType type, int ncols=3);

    /**
     * Create an unbalanced gradient.
     * An unbalanced gradient is a gradient where the transition from
     * color a to color b is not linear, but in this case, exponential.
     *
     * @param pixmap The pixmap that should be written.
     * @param ca Color a
     * @param cb Color b
     * @param type The type of gradient.
     * @param xfactor The x decay length. Use a value between -200 and 200.
     * @param yfactor The y decay length.
     * @param ncols The number of colors. See KPixmapEffect:gradient.
     * @return Returns the generated pixmap, for convencience.
     */
    static KPixmap& unbalancedGradient(KPixmap& pixmap, const QColor &ca,
                   const QColor &cb, GradientType type, int xfactor = 100,
                   int yfactor = 100, int ncols=3);

    /**
     * creates a Pixmap of a given size with the given pixmap. if the
     * given size is bigger than the size of the pixmap, the pixmap is
     * tiled.
     *
     * @param pixmap This is the source pixmap
     * @param size   size the new pixmap should have
     * @return Returns the generated, tiled pixmap.
     */
    static KPixmap createTiled(const KPixmap& pixmap, QSize size);

    /**
     * Either brightens or dims a pixmap by a specified percent.
     *
     * @param pixmap The pixmap to process.
     * @param percent the percent value. Use negative value to dim.
     * @return Returns the @ref #pixmap, provided for convenience.
     */
    static KPixmap& intensity(KPixmap& pixmap, float percent);

    /**
     * Modifies the intensity of a pixmap's RGB channel component.
     *
     * @param pixmap The pixmap to process.
     * @param percent percent value. Use negative value to dim.
     * @param channel which channel(s) should be modified
     * @return Returns the @ref #pixmap, provided for convenience.
     */
    static KPixmap& channelIntensity(KPixmap& pixmap, float percent,
                                    RGBComponent channel);

    /**
     * Blends the provided pixmap into a background of the indicated color
     *
     * @param pixmap The pixmap to process.
     * @param initial_intensity this parameter takes values from -1 to 1:
     *              a) if positive: how much to fade the image in its
     *                              less affected spot
     *              b) if negative: roughly indicates how much of the image
     *                              remains unaffected
     * @param bgnd indicates the color of the background to blend in
     * @param eff lets you choose what kind of blending you like
     * @param anti_dir blend in the opposite direction (makes no much sense
     *                  with concentric blending effects)
     * @return Returns the @ref #pixmap, provided for convenience.
     */
    static KPixmap& blend(KPixmap& pixmap, float initial_intensity,
                         const QColor &bgnd, GradientType eff,
                         bool anti_dir=false, int ncols=3);

    /**
     * Builds a hash on any given Pixmap
     *
     * @param pixmap The pixmap to process.
     * @param lite - the hash faces the indicated lighting (cardinal poles)
     * @param spacing - how many unmodified pixels inbetween hashes
     * @return Returns the @ref #pixmap, provided for convenience.
     */
    static KPixmap& hash(KPixmap& pixmap, Lighting lite=NorthLite,
                        unsigned int spacing=0, int ncols=3);

    /**
     * Create a pattern from a pixmap. The given pixmap is "flattened"
     * between color a to color b.
     *
     * @param pixmap The pixmap to process.
     * @param ca Color a.
     * @param cb Color b.
     * @param ncols The number of colors to use. The image will be
     * dithered to this depth. Pass zero to prevent dithering.
     * @return Returns the @ref #pixmap, provided for convenience.
     */
    static KPixmap pattern(const KPixmap& pixmap, QSize size,
                   const QColor &ca, const QColor &cb, int ncols=8);
    	
    /**
     * This recolors a pixmap. The most dark color will become color a,
     * the most bright one color b, and in between.
     *
     * @param pixmap The pixmap to process.
     * @param ca Color a
     * @param cb Color b
     * @param ncols The number of colors to use. Pass zero to prevent
     * dithering.
     * @return Returns the @ref #pixmap, provided for convenience.
     */
    static KPixmap& fade(KPixmap& pixmap, double val, const QColor &color);

    /**
     * Converts a pixmap to grayscale.
     *
     * The formula used in the conversion is (r+b+g)/3.  This may not
     * be the best way to do the conversion from an aesthetics point
     * of view.  It is, however, very optimized and very fast.
     *
     * @param pixmap The pixmap to process.
     * @return Returns the @ref #pixmap, provided for convenience.
     */
    static KPixmap& toGray(KPixmap& pixmap);
};


#endif
