/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdecore.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 *
 * This is free software; it comes under the GNU Library General 
 * Public License, version 2. See the file "COPYING.LIB" for the 
 * exact licensing terms.
 */

#ifndef __KIconEffect_h_Included__
#define __KIconEffect_h_Included__

#include <qimage.h>
#include <qpixmap.h>

class KIconEffectPrivate;

class KIconEffect
{
public:
    KIconEffect();
    ~KIconEffect();

    void init();

    enum Effects { NoEffect, ToGray, DeSaturate, Emboss, LastEffect };

    QImage apply(QImage src, int group, int state);
    QImage apply(QImage src, int effect, float value);

    QPixmap apply(QPixmap src, int group, int state);
    QPixmap apply(QPixmap src, int effect, float value);

private:
    void toGray(QImage &image);
    void deSaturate(QImage &image, float value);

    int mEffect[4][3];
    float mValue[4][3];
    KIconEffectPrivate *d;
};

#endif
