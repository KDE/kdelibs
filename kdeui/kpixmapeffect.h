#ifndef __KPIXMAP_EFFECT_H
#define __KPIMAP_EFFECT_H

#include <kpixmap.h>

/**
 * This class includes various pixmap based graphical effects. Everything is
 * static, so there is no need to create an instance of this class. You can
 * just call the static methods. They are encapsulated here merely to provide
 * a common namespace.
 */
class KPixmapEffect
{
public:
    /**
     * Draws a pyramid gradient from color ca to color cb.
     */
    enum GradientType { PyramidGradient, RectangleGradient, ConicGradient };

    static void advancedGradient(KPixmap &pixmap, const QColor &ca,
                                 const QColor &cb, GradientType eff,
                                 int ncols=3);
};
    

#endif
