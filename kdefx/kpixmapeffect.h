/* This file is part of the KDE libraries
    Copyright (C) 1998, 1999 Christian Tibirna <ctibirna@total.net>
              (C) 1998, 1999 Daniel M. Duley <mosfet@kde.org>
              (C) 1998, 1999 Dirk A. Mueller <mueller@kde.org>

*/

// $Id$

#ifndef __KPIXMAP_EFFECT_H
#define __KPIXMAP_EFFECT_H


#include <qsize.h>
class KPixmap;
class QColor;

/**
 * This class includes various pixmap-based graphical effects.
 *
 * Everything is
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
     * Creates a gradient from color a to color b of the specified type.
     *
     * @param pixmap The pixmap to process.
     * @param ca Color a.
     * @param cb Color b.
     * @param type The type of gradient.
     * @param ncols The number of colors to use when not running on a
     * truecolor display. The gradient will be dithered to this number of
     * colors. Pass 0 to prevent dithering.
     * @return Returns the generated pixmap, for convenience.
     */
    static KPixmap& gradient(KPixmap& pixmap, const QColor &ca, const QColor &cb,
                            GradientType type, int ncols=3);

    /**
     * Creates an unbalanced gradient.
     *
     * An unbalanced gradient is a gradient where the transition from
     * color a to color b is not linear, but in this case, exponential.
     *
     * @param pixmap The pixmap that should be written.
     * @param ca Color a.
     * @param cb Color b.
     * @param type The type of gradient.
     * @param xfactor The x decay length. Use a value between -200 and 200.
     * @param yfactor The y decay length.
     * @param ncols The number of colors. See #gradient.
     * @return The generated pixmap, for convencience.
     */
    static KPixmap& unbalancedGradient(KPixmap& pixmap, const QColor &ca,
                   const QColor &cb, GradientType type, int xfactor = 100,
                   int yfactor = 100, int ncols=3);

    /**
     * Creates a pixmap of a given size with the given pixmap.
     *
     * if the
     * given size is bigger than the size of the pixmap, the pixmap is
     * tiled.
     *
     * @param pixmap This is the source pixmap
     * @param size   The size the new pixmap should have.
     * @return The generated, tiled pixmap.
     */
    static KPixmap createTiled(const KPixmap& pixmap, QSize size);

    /**
     * Either brightens or dims a pixmap by a specified ratio.
     *
     * @param pixmap The pixmap to process.
     * @param ratio The ratio to use. Use negative value to dim.
     * @return Returns The @ref pixmap(), provided for convenience.
     */
    static KPixmap& intensity(KPixmap& pixmap, float ratio);

    /**
     * Modifies the intensity of a pixmap's RGB channel component.
     *
     * @param pixmap The pixmap to process.
     * @param ratio value. Use negative value to dim.
     * @param channel Which channel(s) should be modified
     * @return Returns the @ref pixmap(), provided for convenience.
     */
    static KPixmap& channelIntensity(KPixmap& pixmap, float ratio,
                                    RGBComponent channel);

    /**
     * Blends the provided pixmap into a background of the indicated color.
     *
     * @param pixmap The pixmap to process.
     * @param initial_intensity this parameter takes values from -1 to 1:
     *              @li If positive, it tells how much to fade the image in its
     *                              less affected spot.
     *              @li If negative, it tells roughly indicates how much of the image
     *                              remains unaffected
     * @param bgnd Indicates the color of the background to blend in.
     * @param eff Lets you choose what kind of blending you like.
     * @param anti_dir Blend in the opposite direction (makes no much sense
     *                  with concentric blending effects).
     * @return Returns the @ref pixmap(), provided for convenience.
     */
    static KPixmap& blend(KPixmap& pixmap, float initial_intensity,
                         const QColor &bgnd, GradientType eff,
                         bool anti_dir=false, int ncols=3);

    /**
     * Builds a hash on any given pixmap.
     *
     * @param pixmap The pixmap to process.
     * @param lite The hash faces the indicated lighting (cardinal poles)
     * @param spacing How many unmodified pixels inbetween hashes.
     * @return Returns The @ref pixmap(), provided for convenience.
     */
    static KPixmap& hash(KPixmap& pixmap, Lighting lite=NorthLite,
                        unsigned int spacing=0, int ncols=3);

    /**
     * Creates a pattern from a pixmap.
     *
     * The given pixmap is "flattened"
     * between color a to color b.
     *
     * @param pixmap The pixmap to process.
     * @param ca Color a.
     * @param cb Color b.
     * @param ncols The number of colors to use. The image will be
     * dithered to this depth. Pass zero to prevent dithering.
     * @return The @ref pixmap(), provided for convenience.
     */
    static KPixmap pattern(const KPixmap& pixmap, QSize size,
                   const QColor &ca, const QColor &cb, int ncols=8);

    /**
     * Recolors a pixmap.
     *
     * The most dark color will become color a,
     * the most bright one color b, and in between.
     *
     * @param pixmap The pixmap to process.
     * @param ca Color a.
     * @param cb Color b.
     * @param ncols The number of colors to use. Pass zero to prevent
     * dithering.
     * @return Returns the @ref pixmap(), provided for convenience.
     */
    static KPixmap& fade(KPixmap& pixmap, double val, const QColor &color);

    /**
     * Converts a pixmap to grayscale.
     *
     * @param pixmap The pixmap to process.
     * @param fast Set to @p true in order to use a faster but non-photographic
     * quality algorithm. Appropriate for things such as toolbar icons.
     * @return Returns the @ref pixmap(), provided for convenience.
     */
    static KPixmap& toGray(KPixmap& pixmap, bool fast=false);

    /**
     * Desaturates a pixmap.
     *
     * @param pixmap The pixmap to process.
     * @param desat A value between 0 and 1 setting the degree of desaturation
     * @return Returns The @ref pixmap(), provided for convenience.
     */
    static KPixmap& desaturate(KPixmap& pixmap, float desat = 0.3);

    /**
     * Modifies the contrast of a pixmap.
     *
     * @param pixmap The pixmap to process.
     * @param c A contrast value between -255 and 255.
     * @return Returns the @ref pixmap(), provided for convenience.
     */
    static KPixmap& contrast(KPixmap& pixmap, int c);

    /**
     * Dithers a pixmap using Floyd-Steinberg dithering for low-color
     * situations.
     *
     * @param pixmap The pixmap to process.
     * @param palette The color palette to use.
     * @param size The size of the palette.
     * @return Returns the @ref pixmap(), provided for convenience.
     */
    static KPixmap& dither(KPixmap &pixmap, const QColor *palette, int size);

    /**
     * Calculate a 'selected' pixmap, for instance a selected icon
     * on the desktop.
     * @param pixmap the pixmap to select
     * @param col the selected color, usually from QColorGroup::highlight().
     */
    static KPixmap selectedPixmap( const KPixmap &pixmap, const QColor &col );
};


#endif
