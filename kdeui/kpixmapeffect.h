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
    enum GradientType { PyramidGradient, RectangleGradient, ConicGradient };

    /**
     * Draws a pyramid, rectangle, or conic gradient from color ca to
     * color cb.
     */
    static void advancedGradient(KPixmap &pixmap, const QColor &ca,
                                 const QColor &cb, GradientType type,
                                 int ncols=3);
    /**
     * Brightens the image by a specified percent. For example, .5 will
     * brighten the image 50%.
     */
    static void brighten(QImage &image, float percent);
    /**
     * Brightens the pixmap by a specified percent.
     */
    inline static void brighten(KPixmap &pixmap, float percent);
    /**
     * Dims the image by a specified percent. For example, .5 will
     * dim the image 50%.
     */
    static void dim(QImage &image, float percent);
    /**
     * Dims the pixmap by a specified percent.
     */
    inline static void dim(KPixmap &pixmap, float percent);
    
};

inline void KPixmapEffect::brighten(KPixmap &pixmap, float percent)
{
    QImage image = pixmap.convertToImage();
    brighten(image, percent);
    pixmap.convertFromImage(image);
}

inline void KPixmapEffect::dim(KPixmap &pixmap, float percent)
{
    QImage image = pixmap.convertToImage();
    dim(image, percent);
    pixmap.convertFromImage(image);
}
    

#endif
