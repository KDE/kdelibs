/* This file is part of the KDE libraries
    Copyright (C) 1998, 1999 Christian Tibirna <ctibirna@total.net>
              (C) 1998, 1999 Daniel M. Duley <mosfet@kde.org>
              (C) 1998, 1999 Dirk A. Mueller <mueller@kde.org>

*/

// $Id$

#ifndef __KIMAGE_EFFECT_H
#define __KIMAGE_EFFECT_H

class QImage;
class QSize;
class QColor;

/**
 * This class includes various @ref QImage based graphical effects.
 *
 *Everything is
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
    enum RGBComponent { Red, Green, Blue, Gray, All };

    enum Lighting {NorthLite, NWLite, WestLite, SWLite,
                   SouthLite, SELite, EastLite, NELite};

    enum ModulationType { Intensity, Saturation, HueShift, Contrast };

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
     * Blend the provided image into a background of the indicated color.
     *
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
     * Blend an image into another one, using a gradient type
     * for blending from one to another.
     *
     * @param image1 source1 and result of blending
     * @param image2 source2 of blending
     * @param gt gradient type for blending between source1 and source2
     * @param xf x decay length for unbalanced gradient tpye
     * @param yf y decay length for unbalanced gradient tpye
     */
    static QImage& blend(QImage &image1,QImage &image2,
			 GradientType gt, int xf=100, int yf=100);

    /**
     * Blend an image into another one, using a color channel of a
     * third image for the decision of blending from one to another.
     *
     * @param image1 Source 1 and result of blending
     * @param image2 Source 2 of blending
     * @param blendImage If the gray value of of pixel is 0, the result
     *               for this pixel is that of image1; for a gray value
     *               of 1, the pixel of image2 is used; for a value
     *               inbetween, a corresponding blending is used.
     * @param channel The RBG channel to use for the blending decision.
     */
    static QImage& blend(QImage &image1, QImage &image2,
			 QImage &blendImage, RGBComponent channel);

    /**
     * Blend an image into another one, using alpha in the expected way.
     * @author Rik Hemsley (rikkus) <rik@kde.org>
     */
    static bool blend(const QImage & upper, const QImage & lower, QImage & output);
// Not yet...    static bool blend(const QImage & image1, const QImage & image2, QImage & output, const QRect & destRect);

    /**
     * Blend an image into another one, using alpha in the expected way and
     * over coordinates @p x and @p y with respect to the lower image.
     * The output is a QImage which is the @p upper image already blended
     * with the @p lower one, so its size will be (in general) the same than
     * @p upper instead of the same size than @p lower like the method above.
     * In fact, the size of @p output is like upper's one only when it can be
     * painted on lower, if there has to be some clipping, output's size will
     * be the clipped area and x and y will be set to the correct up-left corner
     * where the clipped rectangle begins.
     */
    static bool blend(int &x, int &y, const QImage & upper, const QImage & lower, QImage & output);
    /**
     * Blend an image into another one, using alpha in the expected way and
     * over coordinates @p x and @p y with respect to the lower image.
     * The output is painted in the own @p lower image. This is an optimization
     * of the @ref blend method above provided by convenience.
     */
    static bool blendOnLower(int x, int y, const QImage & upper, const QImage & lower);

    /**
     * Modifies the intensity of a pixmap's RGB channel component.
     *
     * @param image The QImage to process.
     * @param percent Percent value. Use a negative value to dim.
     * @param channel Which channel(s) should be modified
     * @return The @p image, provided for convenience.
     */
    static QImage& channelIntensity(QImage &image, float percent,
                                    RGBComponent channel);

    /**
     * Fade an image to a certain background color.
     *
     * The number of colors will not be changed.
     *
     * @param image The QImage to process.
     * @param val The strength of the effect. 0 <= val <= 1.
     * @param color The background color.
     * @return Returns the @ref image(), provided for convenience.
     */
    static QImage& fade(QImage &img, float val, const QColor &color);


    /**
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
     * Build a hash on any given @ref QImage
     *
     * @param image The QImage to process
     * @param lite The hash faces the indicated lighting (cardinal poles).
     * @param spacing How many unmodified pixels inbetween hashes.
     * @return Returns the @ref image(), provided for convenience.
     */
    static QImage& hash(QImage &image, Lighting lite=NorthLite,
                        unsigned int spacing=0);

    /**
     * Either brighten or dim the image by a specified percent.
     * For example, .50 will modify the colors by 50%.
     *
     * @param image The QImage to process.
     * @param percent The percent value. Use a negative value to dim.
     * @return Returns The @ref image(), provided for convenience.
     */
    static QImage& intensity(QImage &image, float percent);

    /**
     * Modulate the image with a color channel of another image.
     *
     * @param image The QImage to modulate and result.
     * @param modImage The QImage to use for modulation.
     * @param reverse Invert the meaning of image/modImage; result is image!
     * @param type The modulation Type to use.
     * @param factor The modulation amplitude; with 0 no effect [-200;200].
     * @param channel The RBG channel of image2 to use for modulation.
     * @return Returns the @ref image(), provided for convenience.
     */
    static QImage& modulate(QImage &image, QImage &modImage, bool reverse,
		ModulationType type, int factor, RGBComponent channel);

    /**
     * Convert an image to grayscale.
     *
     * @param image The @ref QImage to process.
     * @param fast Set to @p true in order to use a faster but non-photographic
     * quality algorithm. Appropriate for things such as toolbar icons.
     * @return Returns the @ref image(), provided for convenience.
     */
    static QImage& toGray(QImage &image, bool fast = false);

    /**
     * Desaturate an image evenly.
     *
     * @param image The QImage to process.
     * @param desat A value between 0 and 1 setting the degree of desaturation
     * @return Returns the @ref image(), provided for convenience.
     */
    static QImage& desaturate(QImage &image, float desat = 0.3);

    /**
     * Modifie the contrast of an image.
     *
     * @param image The QImage to process.
     * @param c A contrast value between -255 to 255.
     * @return The @ref image(), provided for convenience.
     */
    static QImage& contrast(QImage &image, int c);

    /**
     * Dither an image using Floyd-Steinberg dithering for low-color
     * situations.
     *
     * @param image The QImage to process.
     * @param palette The color palette to use
     * @param size The size of the palette
     * @return Returns the @ref image(), provided for convenience.
     */
    static QImage& dither(QImage &img, const QColor *palette, int size);

private:

    /**
     * Helper function to fast calc some altered (lighten, shaded) colors
     *
     */
    static unsigned int lHash(unsigned int c);
    static unsigned int uHash(unsigned int c);

    /**
     * Helper function to find the nearest color to the RBG triplet
     */
    static int nearestColor( int r, int g, int b, const QColor *pal, int size );
};

#endif
