#ifndef __KPIXMAP_EFFECT_H
#define __KPIMAP_EFFECT_H

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
    /**
     * Draws a vertical, horizontal, diagonal, crossdiagonal,pyramid, 
     * rectangle, pipecross or elliptic gradient from color ca to color cb.
     */
    static void gradient(KPixmap &pixmap, const QColor &ca,
                                 const QColor &cb, GradientType type,
                                 int ncols=3);

    /**
     * Draws a vertical, horizontal, diagonal, crossdiagonal,pyramid, 
     * rectangle, pipecross or elliptic gradient from color ca to color cb
     * using an unbalanced (non-linear) ca-to-cb color table.
     * int balance has a nonzero value between 1 and 100 with 
     * a default of 30 (values larger than 100 don't make much sense, since
     * the algorithm uses exponentials (I'd really need an algorithms book)
     */
    static void unbalancedGradient(KPixmap &pixmap, const QColor &ca,
                                 const QColor &cb, GradientType type,
                                 int factor = 30, int ncols=3);

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


#endif
