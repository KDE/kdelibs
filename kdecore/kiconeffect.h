/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdecore.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 * with minor additions and based on ideas from
 * Torsten Rahn <torsten@kde.org>
 *
 * This is free software; it comes under the GNU Library General
 * Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 */

#ifndef KICONEFFECT_H
#define KICONEFFECT_H

#include <qimage.h>
#include <qpixmap.h>
#include <qcolor.h>
#include <qrect.h>
#include "kdelibs_export.h"

class QWidget;

class KIconEffectPrivate;

/**
 * Applies effects to icons.
 *
 * This class applies effects to icons depending on their state and
 * group. For example, it can be used to make all disabled icons
 * in a toolbar gray.
 * @see KIcon
 */
class KDECORE_EXPORT KIconEffect
{
public:
  /**
   * Create a new KIconEffect.
   */
    KIconEffect();
    ~KIconEffect();

    /**
     * This is the enumeration of all possible icon effects.
     * Note that 'LastEffect' is no valid icon effect but only
     * used internally to check for invalid icon effects.
     *
     * @li NoEffect: Don't apply any icon effect
     * @li ToGray: Tints the icon gray
     * @li Colorize: Tints the icon with an other color
     * @li ToGamma: Change the gamma value of the icon
     * @li DeSaturate: Reduce the saturation of the icon
     * @li ToMonochrome: Produces a monochrome icon
     */
    enum Effects { NoEffect, ToGray, Colorize, ToGamma, DeSaturate,
                   ToMonochrome,   ///< @since 3.4
		   LastEffect };

    /**
     * Rereads configuration.
     */
    void init();

    /**
     * Tests whether an effect has been configured for the given icon group.
     * @param group the group to check, see KIcon::Group
     * @param state the state to check, see KIcon::States
     * @returns true if an effect is configured for the given @p group
     * in @p state, otherwise false.
     * @see KIcon::Group
     * KIcon::States
     */
    bool hasEffect(int group, int state) const;

    /**
     * Returns a fingerprint for the effect by encoding
     * the given @p group and @p state into a QString. This
     * is useful for caching.
     * @param group the group, see KIcon::Group
     * @param state the state, see KIcon::States
     * @return the fingerprint of the given @p group+@p state
     */
     QString fingerprint(int group, int state) const;

    /**
     * Applies an effect to an image. The effect to apply depends on the
     * @p group and @p state parameters, and is configured by the user.
     * @param src The image.
     * @param group The group for the icon, see KIcon::Group
     * @param state The icon's state, see KIcon::States
     * @return An image with the effect applied.
     */
    QImage apply(QImage src, int group, int state) const;

    /**
     * Applies an effect to an image.
     * @param src The image.
     * @param effect The effect to apply, one of KIconEffect::Effects.
     * @param value Strength of the effect. 0 <= @p value <= 1.
     * @param rgb Color parameter for effects that need one.
     * @param trans Add Transparency if trans = true.
     * @return An image with the effect applied.
     */
    // KDE4: make them references
    QImage apply(QImage src, int effect, float value, const QColor rgb, bool trans) const;
    /**
     * @since 3.4
     */
    QImage apply(QImage src, int effect, float value, const QColor rgb, const QColor rgb2, bool trans) const;

    /**
     * Applies an effect to a pixmap.
     * @param src The pixmap.
     * @param group The group for the icon, see KIcon::Group
     * @param state The icon's state, see KIcon::States
     * @return A pixmap with the effect applied.
     */
    QPixmap apply(QPixmap src, int group, int state) const;

    /**
     * Applies an effect to a pixmap.
     * @param src The pixmap.
     * @param effect The effect to apply, one of KIconEffect::Effects.
     * @param value Strength of the effect. 0 <= @p value <= 1.
     * @param rgb Color parameter for effects that need one.
     * @param trans Add Transparency if trans = true.
     * @return A pixmap with the effect applied.
     */
    QPixmap apply(QPixmap src, int effect, float value, const QColor rgb, bool trans) const;
    /**
     * @since 3.4
     */
    QPixmap apply(QPixmap src, int effect, float value, const QColor rgb, const QColor rgb2, bool trans) const;

    /**
     * Returns an image twice as large, consisting of 2x2 pixels.
     * @param src the image.
     * @return the scaled image.
     */
    QImage doublePixels(QImage src) const;

    /**
     * Provides visual feedback to show activation of an icon on a widget.
     *
     * Not strictly an 'icon effect', but in practice that's what it looks
     * like.
     *
     * This method does nothing if the global 'Visual feedback on activation'
     * option is not activated (See kcontrol/Peripherals/Mouse).
     *
     * @param widget The widget on which the effect should be painted
     * @param rect This rectangle defines the effect's borders
     */
    static void visualActivate(QWidget *widget, const QRect &rect);

    /**
     * Tints an image gray.
     *
     * @param image The image
     * @param value Strength of the effect. 0 <= @p value <= 1
     */
    static void toGray(QImage &image, float value);

    /**
     * Colorizes an image with a specific color.
     *
     * @param image The image
     * @param col The color with which the @p image is tinted
     * @param value Strength of the effect. 0 <= @p value <= 1
     */
    static void colorize(QImage &image, const QColor &col, float value);

    /**
     * Produces a monochrome icon with a given foreground and background color
     *
     * @param image The image
     * @param white The color with which the white parts of @p image are painted
     * @param black The color with which the black parts of @p image are painted
     * @param value Strength of the effect. 0 <= @p value <= 1
     * @since 3.4
     */
    static void toMonochrome(QImage &image, const QColor &black, const QColor &white, float value);

    /**
     * Desaturates an image.
     *
     * @param image The image
     * @param value Strength of the effect. 0 <= @p value <= 1
     */
    static void deSaturate(QImage &image, float value);

    /**
     * Changes the gamma value of an image.
     *
     * @param image The image
     * @param value Strength of the effect. 0 <= @p value <= 1
     */
    static void toGamma(QImage &image, float value);

    /**
     * Renders an image semi-transparent.
     *
     * @param image The image
     */
    static void semiTransparent(QImage &image);

    /**
     * Renders a pixmap semi-transparent.
     *
     * @param pixmap The pixmap
     */
    static void semiTransparent(QPixmap &pixmap);

    /**
     * Overlays an image with an other image.
     *
     * @param src The image
     * @param overlay The image to overlay @p src with
     */
    static void overlay(QImage &src, QImage &overlay);

private:
    int mEffect[6][3];
    float mValue[6][3];
    QColor mColor[6][3];
    bool mTrans[6][3];
    KIconEffectPrivate *d;
};

#endif
