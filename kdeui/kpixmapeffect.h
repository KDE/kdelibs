/*
 * (C)Copyright 1999
 * Cristian Tibirna  <ctibirna@total.net>
 * Daniel M. Duley <mosfet@kde.org>
 * Dirk A. Mueller <dmuell@gmx.net>
 *
 * $Id: $
 */

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
     * The same as above, only works on a KPixmap now.
     */
    static void gradient(KPixmap &pixmap, const QColor &ca,
                                 const QColor &cb, GradientType type,
                                 int ncols=3);

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
     * Same as above, only this works on a KPixmap.
     */
    static void unbalancedGradient(KPixmap &pixmap, const QColor &ca, 
	    const QColor &cb, GradientType type, int xfactor = 100, 
	    int yfactor = 100, int ncols=3);

    /**
     * Either brightens or dims the image by a specified percent.
     * For example, .5 will modify the colors by 50%. All percent values
     * should be positive, use bool brighten to set if the image gets
     * brightened or dimmed.
     */
    static void intensity(QImage &image, float percent, bool brighten=true);

    /**
     * Either brightens or dims a pixmap by a specified percent.
     */
    inline static void intensity(KPixmap &pixmap, float percent,
                                 bool brighten = true);
    /**
     * Modifies the intensity of a image's RGB channel component.
     */
    static void channelIntensity(QImage &image, float percent,
                                 RGBComponent channel,
                                 bool brighten = true);
    /**
     * Modifies the intensity of a pixmap's RGB channel component.
     */
    inline static void channelIntensity(KPixmap &pixmap, float percent,
                                        RGBComponent channel,
                                        bool brighten = true);
    
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
    static void blend(QImage &image, float initial_intensity, 
		    const QColor &bgnd, GradientType eff,
		    bool anti_dir=false);

    /**
     * Blends the provided pixmap (see the other method prototype)
     */
    static void blend(KPixmap &pixmap, float initial_intensity, 
		    const QColor &bgnd, GradientType eff,
		    bool anti_dir=false, int ncols=3);

    /**
     * Builds a hash on any given QImage
     * @param lite - the hash faces the indicated lighting (cardinal poles)
     * @param spacing - how many unmodified pixels inbetween hashes
     */
    static void hash(QImage &image, Lighting lite=NorthLite, 
		   unsigned int spacing=0);

    /**
     * As above, on a KPixmap
     */
    static void hash(KPixmap &pixmap, Lighting lite=NorthLite, 
		       unsigned int spacing=0, int ncols=3);


    /**
     * Create an image with a pattern. The pattern is an 8x8 bitmap, 
     * specified by an array of 8 integers.
     *
     * @param size The desired size.
     * @param ca Color a
     * @param cb Color b
     * @param pattern The pattern.
     * @return A QImage painted with the pattern.
     */
    static QImage pattern(const QSize &size, const QColor &ca,
	    const QColor &cb, unsigned pattern[8]);

    /**
     * Same a above, only this works on a KPixmap.
     */
    static void pattern(KPixmap &pixmap, const QColor &ca,
	    const QColor &cb, unsigned pattern[8]);

    /**
     * Create a pattern from an image. KPixmapEffect::flatten()
     * is used to recolor the image between color a to color b.
     *
     * @param img A QImage containing the pattern.
     * @param ncols The number of colors to use. The image will be
     * dithered to this depth. Pass zero to prevent dithering.
     */
    static QImage pattern(const QSize &size, const QColor &ca,
	    const QColor &cb, QImage img, int ncols=0);

    /**
     * The same as above, only this works on a pixmap.
     * No dithering is done if the display is truecolor.
     */
    static void pattern(KPixmap &pixmap, const QColor &ca,
	    const QColor &cb, QImage img, int ncols=8);
    
    /**
     * This recolors an image. The most dark color will become color a, 
     * the most bright one color b, and in between.
     *
     * @param image A QImage to process.
     * @param ca Color a
     * @param cb Color b
     * @param ncols The number of colors to use. Pass zero to prevent
     * dithering.
     */
    static void flatten(QImage &image, const QColor &ca, 
	    const QColor &cb, int ncols=0);
};



inline void KPixmapEffect::intensity(KPixmap &pixmap, float percent,
                                     bool brighten)
{
    QImage image = pixmap.convertToImage();
    intensity(image, percent, brighten);
    pixmap.convertFromImage(image);
}

inline void KPixmapEffect::channelIntensity(KPixmap &pixmap, float percent,
                                            RGBComponent channel,
                                            bool brighten)
{
    QImage image = pixmap.convertToImage();
    channelIntensity(image, percent, channel, brighten);
    pixmap.convertFromImage(image);
}


/**
 * Helper function to fast calc some altered (lighten, shaded) colors (CT)
 *
 */
unsigned int lHash(unsigned int c);
unsigned int uHash(unsigned int c);

#endif
