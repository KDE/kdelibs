/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdeui.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 *
 * You can Freely distribute this program under the GNU Library General
 * Public License. See the file "COPYING.LIB" for the exact licensing terms.
 */

#ifndef __KPixmapIO_h_Included__
#define __KPixmapIO_h_Included__

class QPixmap;
class QImage;
class KPixmapIOPrivate;
/**
 * @short Fast QImage to/from QPixmap conversion.
 * @author Geert Jansen <jansen@kde.org>
 * @version $Id$
 *
 * KPixmapIO implements a fast path for QPixmap to/from QImage conversions.
 * It uses the MIT-SHM shared memory extension for this. If this extension is
 * not available, it will fall back to standard Qt methods.
 *
 * @sect Typical usage
 *
 * You can use KPixmapIO for load/saving pixmaps.
 *
 * <pre>
 * KPixmapIO io;
 * pixmap = io.convertToPixmap(image);
 * image = io.convertToImage(pixmap);
 * </pre>
 *
 * It also has functionality for partially updating/saving pixmaps, see
 * @ref #putImage and @ref #getImage.
 *
 * @sect KPixmapIO vs. Qt speed comparison
 *
 * Speed measurements were taken. These show that usage of KPixmapIO for
 * images up to a certain threshold size, offers no speed advantage over
 * the Qt routines. Below you can see a plot of these measurements.
 *
 * @image html kpixmapio-perf.png "Performance of KPixmapIO"
 *
 * The threshold size, amongst other causes, is determined by the shared
 * memory allocation policy. If the policy is @p ShmDontKeep, the
 * shared memory segment is discarded right after usage, and thus needs to
 * be allocated before each transfer. This introduces a a setup penalty not
 * present when the policy is @p ShmKeepAndGrow. In this case the
 * shared memory segment is kept and resized when necessary, until the
 * KPixmapIO object is destroyed.
 *
 * The default policy is @p ShmDontKeep. This policy makes sense when
 * loading pixmaps once. The corresponding threshold is taken at 5.000
 * pixels as suggested by experiments. Below this threshold, KPixmapIO
 * will not use shared memory and fall back on the Qt routines.
 *
 * When the policy is @p ShmKeepAndGrow, the threshold is taken at
 * 2.000 pixels. Using this policy, you might want to use @ref #preAllocShm
 * to pre-allocate a certain amount of shared memory, in order to avoid
 * resizes. This allocation policy makes sense in a multimedia type
 * application where you are constantly updating the screen.
 *
 * Above a couple times the threshold size, KPixmapIO's and Qt's speed become
 * linear in the number of pixels, KPixmapIO being at least 2, and mostly around
 * 4 times faster than Qt, depending on the screen and image depth.
 *
 * Speed difference seems to be the most at 16 bpp, followed by 32 and 24
 * bpp respectively. This can be explained by the relatively poor
 * implementation of 16 bit RGB packing in Qt, while at 32 bpp we need to
 * transfer more data, and thus gain more, than at 24 bpp.
 *
 * @sect Conclusion
 *
 * For large pixmaps, there's a definite speed improvement when using
 * KPixmapIO. On the other hand, there's no speed improvement for small
 * pixmaps. When you know you're only transferring small pixmaps, there's no
 * point in using it.
 */

class KPixmapIO
{
public:
    KPixmapIO();
    ~KPixmapIO();

    /**
     * Convert an image to a pixmap.
     * @param image The image to convert.
     * @return The pixmap containing the image.
     */
    QPixmap convertToPixmap(const QImage &image);

    /**
     * Convert a pixmap to an image.
     * @param pixmap The pixmap to convert.
     * @return The image.
     */
    QImage convertToImage(const QPixmap &pixmap);

    /**
     * Bitblt an image onto a pixmap.
     * @param dst The destination pixmap.
     * @param dx Destination x offset.
     * @param dy Destination y offset.
     * @param src The image to load.
     */
    void putImage(QPixmap *dst, int dx, int dy, const QImage *src);

    /**
     * This function is identical to the one above. It only differs in the
     * arguments it accepts.
     */
    void putImage(QPixmap *dst, const QPoint &offset, const QImage *src);

    /**
     * Transfer (a part of) a pixmap to an image.
     * @param src The source pixmap.
     * @param sx Source x offset.
     * @param sy Source y offset.
     * @param sw Source width.
     * @param sh Source height.
     * @return The image.
     */
    QImage getImage(const QPixmap *src, int sx, int sy, int sw, int sh);

    /**
     * This function is identical to the one above. It only differs in the
     * arguments it accepts.
     */
    QImage getImage(const QPixmap *src, const QRect &rect);

    /**
     * Shared memory allocation policies.
     */
    enum ShmPolicies {
	ShmDontKeep,
	ShmKeepAndGrow
    };

    /**
     * Set the shared memory allocation policy. See the introduction for
     * KPixmapIO for a discussion.
     * @param policy The alloction policy.
     */
    void setShmPolicy(int policy);

    /**
     * Pre-allocate shared memory. KPixmapIO will be able to transfer images
     * up to this size without resizing.
     * @param size The size of the image in @p pixels.
     */
    void preAllocShm(int size);

private:
    /*
     * Supported XImage byte orders. The notation ARGB means bytes
     * containing A:R:G:B succeed in memory.
     */
    enum ByteOrders {
	bo32_ARGB, bo32_BGRA, bo24_RGB, bo24_BGR,
	bo16_RGB_565, bo16_BGR_565, bo16_RGB_555,
	bo16_BGR_555, bo8
    };

    bool m_bShm;
    void initXImage(int w, int h);
    void doneXImage();
    void createXImage(int w, int h);
    void destroyXImage();
    void createShmSegment(int size);
    void destroyShmSegment();
    void convertToXImage(const QImage &);
    QImage convertFromXImage();
private:
    KPixmapIOPrivate* d;
};

#endif // __KPixmapIO_h_Included__
