/*
 *  This file is part of the KDE Libraries
 *  Copyright (C) 1999-2001 Mirko Boehm (mirko@kde.org) and 
 *  Espen Sand (espen@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */
#ifndef _KDIALOG_BASE_PRIV_H_
#define _KDIALOG_BASE_PRIV_H_

#include <kpushbutton.h>

/**
 * Used internally by KDialogBase.
 * @internal
 */
class KDEUI_EXPORT KDialogBaseButton : public KPushButton
{
  Q_OBJECT

  public:
    KDialogBaseButton( const KGuiItem &text, int key, QWidget *parent=0,
                       const char *name=0 );
    inline int id();

  private:
    int mKey;
};

/**
 * Used internally by KDialogBase.
 * @internal
 */
class KDEUI_EXPORT KDialogBaseTile : public QObject
{
  Q_OBJECT

  public:
    KDialogBaseTile( QObject *parent=0, const char *name=0 );
    ~KDialogBaseTile();

    void set( const QPixmap *pix );
    const QPixmap *get() const;
  
  public slots:
    void cleanup();

  signals:
    void pixmapChanged();

  private:
    QPixmap *mPixmap;
    class KDialogBaseTilePrivate;
    KDialogBaseTilePrivate *d;
};

#endif
