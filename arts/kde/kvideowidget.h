/*
   This file is part of KDE/aRts (Noatun) - xine integration
   Copyright (C) 2002 Ewald Snel <ewald@rambo.its.tudelft.nl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
*/

#ifndef __KVIDEOWIDGET_H
#define __KVIDEOWIDGET_H

#include <qevent.h>
#include <qwidget.h>
#include <kmedia2.h>


class KVideoWidget : public QWidget
{
Q_OBJECT

public:
    KVideoWidget( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
    virtual ~KVideoWidget();

    void embed( Arts::VideoPlayObject vpo );

    bool isEmbedded();
    bool isFullscreen();
    bool isHalfSize();
    bool isNormalSize();
    bool isDoubleSize();

    QSize sizeHint() const;
    QSizePolicy sizePolicy() const;

protected:
    virtual void focusInEvent ( QFocusEvent * );
    virtual void mousePressEvent( QMouseEvent *event );
    virtual void resizeEvent( QResizeEvent *event );
    virtual bool x11Event( XEvent *event );

public slots:
    void slotFullscreen();
    void slotWindowed();
    void slotHalfSize();
    void slotNormalSize();
    void slotDoubleSize();

signals:
    void adaptSize( int width, int height );
    void rightButtonPressed( const QPoint & );
    void zoomChanged();

private:
    QWidget *fullscreenWidget;
    bool embedded;
    bool enabled;
    int zoom;
    int videoWidth;
    int videoHeight;
};

#endif
