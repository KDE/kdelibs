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
     * Draws a vertical, horizontal, diagonal, crossdiagonal,pyramid, 
     * rectangle, pipecross or elliptic gradient from color ca to color cb.
     */
    static void gradient(KPixmap &pixmap, const QColor &ca,
                                 const QColor &cb, GradientType type,
                                 int ncols=3);

    /**
     * Draws a vertical, horizontal, diagonal, crossdiagonal,pyramid, 
     * rectangle, pipecross or elliptic gradient from color ca to color cb
     * using a bidirectional unbalanced (non-linear) ca-to-cb color table.
     * @ int xfactor and 
     * @ int yfactor have nonzero values between -200 and 200 with 
     * a default of 100 (values larger than 200 don't make much sense, since
     * the algorithm uses exponentials
     * Negative values of xfactor and yfactor indicate a negative direction
     * of the gradient
     */
    static void unbalancedGradient(KPixmap &pixmap, 
				   const QColor &ca, const QColor &cb, 
				   GradientType type,
				   int xfactor = 100, int yfactor = 100,
				   int ncols=3);

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
   * @ float initial_intensity;  this parameter takes values from -1 to 1: 
   *              a) if positive: how much to fade the image in its 
   *                              less affected spot
   *              b) if negative: roughly indicates how much of the image 
   *                              remains unaffected
   * @ QColor bgnd; indicates the color of the background to blend in
   * @ KPixmapEffect::GradientType eff; lets you choose what
   *    kind of blending you like
   * @ bool anti_dir; blend in the opposite direction (makes no much sense
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
   * @ Lighting lite - the hash faces the indicated lighting (cardinal poles)
   * @ int spacing - how many unmodified pixels inbetween hashes
   */
  static void hash(QImage &image, Lighting lite=NorthLite, 
		   unsigned int spacing=0);

  /**
   * As above, on a KPixmap
   */
  static void hash(KPixmap &pixmap, Lighting lite=NorthLite, 
		   unsigned int spacing=0, int ncols=3);
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
