/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
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

#ifndef __KIconEffect_h_Included__
#define __KIconEffect_h_Included__

#include <qimage.h>
#include <qpixmap.h>
#include <qcolor.h>
#include <qrect.h>

class QWidget;

class KIconEffectPrivate;

/**
 * Apply effects to icons.
 */
class KIconEffect
{
public:
    KIconEffect();
    ~KIconEffect();

    enum Effects { NoEffect, ToGray, Colorize, ToGamma, DeSaturate, LastEffect };

    /**
     * Reread configuration.
     */
    void init();

    /**
     * @returns true if an effect is configured for the given @p group
     * in @p state, otherwise false.
     */
    bool hasEffect(int group, int state) const;
    
    /**
     * Returns a fingerprint for the effect selected by
     * [@p group, @p state]. This is usefull for caching.
     */
     QString fingerprint(int group, int state);

    /**
     * Apply an effect to an image. The effect to apply depends on the
     * @p group and @p state parameters, and is configured by the user.
     * @param src The image.
     * @param group The group for the icon.
     * @param state The icon's state.
     * @return An image with the effect applied.
     */
    QImage apply(QImage src, int group, int state);

    /**
     * Apply an effect to an image.
     * @param src The image.
     * @param effect The effect to apply, one of @ref #KIconEffect::Effects.
     * @param value Strength of the effect. 0 <= @p value <= 1.
     * @param color Color parameter for effects that need one.
     * @param trans Add Transparency if trans = true.
     * @return An image with the effect applied.
     */
    QImage apply(QImage src, int effect, float value, const QColor rgb, bool trans);

    /**
     * Apply an effect to a pixmap.
     */
    QPixmap apply(QPixmap src, int group, int state);

    /**
     * Apply an effect to a pixmap.
     */
    QPixmap apply(QPixmap src, int effect, float value, const QColor rgb, bool trans);

    /**
     * Returns an image twice as large, consisting of 2x2 pixels.
     */
    QImage doublePixels(QImage src);

    /**
     * Provide visual feedback to show activation of an icon on a widget.
     *
     * Not strictly an 'icon effect', but in practice that's what it looks
     * like.
     */
    static void visualActivate(QWidget *, QRect);

    static void toGray(QImage &image, float value);
    static void colorize(QImage &image, const QColor &col, float value);
    static void deSaturate(QImage &image, float value);
    static void toGamma(QImage &image, float value);
    static void semiTransparent(QImage &image);
    static void semiTransparent(QPixmap &pixmap);
    static void overlay(QImage &src, QImage &overlay);

private:
    int mEffect[6][3];
    float mValue[6][3];
    QColor mColor[6][3];
    bool mTrans[6][3];
    KIconEffectPrivate *d;
};

#endif
