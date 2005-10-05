/* This file is part of the KDE libraries

    Copyright (c) 2001  Martin R. Jones <mjones@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KSCREENSAVER_H
#define KSCREENSAVER_H

#include <qwidget.h>

#include <kdelibs_export.h>

class QTimer;
class KScreenSaverPrivate;
class KBlankEffectPrivate;

/**
* Provides a QWidget for a screensaver to draw into.
*
* You should derive from this widget and implement your screensaver's
* functionality.  To use libkss, provide the following constants and
* functions:
*
*   extern "C"
*   {
*       const char *kss_applicationName = "yourappname";
*       const char *kss_description = I18N_NOOP( "Your screensaver" );
*       const char *kss_version = "1.0";
*
*       KScreenSaver *kss_create( WId d )
*       {
*           // return your KScreenSaver derived screensaver
*       }
*
*       QDialog *kss_setup()
*       {
*           // return your modal setup dialog
*       }
*   }
*
* @short Provides a QWidget for a screensaver to draw into.
* @author Martin R. Jones <mjones@kde.org>
*/
class KDE_EXPORT KScreenSaver : public QWidget
{
	Q_OBJECT
public:
    /**
     * @param id The winId() of the widget to draw the screensaver into.
     */
	KScreenSaver( WId id=0 );
	~KScreenSaver();

protected:
    /**
     * You cannot create a new widget with this widget as parent, since this
     * widget may not be owned by your application.  In order to create
     * widgets with a KScreenSaver as parent, create the widget with no parent,
     * call embed(), and then show() the widget.
     *
     * @param widget The widget to embed in the screensaver widget.
     */
    void embed( QWidget *widget );

    bool eventFilter( QObject *o, QEvent * );

private:
    KScreenSaverPrivate *d;
};


/**
*
* Blanks a widget using various effects.
*
* @short Blanks a widget using various effects.
* @author Martin R. Jones <mjones@kde.org>
*/
class KBlankEffect : public QObject
{
	Q_OBJECT
public:
	KBlankEffect( QObject *parent=0 );
	~KBlankEffect();

    enum Effect { Random=-1, Blank=0, SweepRight, SweepDown, Blocks,
                  MaximumEffects };

    /**
     * Blank a widget using the specified effect.
     * Some blanking effects take some time, so you should connect to
     * doneBlank() to know when the blanking is complete.
     *
     * @param w The widget to blank.
     * @param effect The type of effect to use.
     */
    void blank( QWidget *w, Effect effect=Random );

    typedef void (KBlankEffect::*BlankEffect)();

signals:
    /**
     * emitted when a blanking effect has completed.
     */
    void doneBlank();

protected slots:
    void timeout();

protected:
    void finished();

    void blankNormal();
    void blankSweepRight();
    void blankSweepDown();
    void blankBlocks();

protected:
    static BlankEffect effects[];
    KBlankEffectPrivate *d;
};
#endif

