/*
 * This file is part of the KDE Libraries
 * Copyright (C) 1999-2001 Mirko Boehm (mirko@kde.org) and
 * Espen Sand (espen@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef K3ABOUTDIALOG_P_H
#define K3ABOUTDIALOG_P_H

#include <QtGui/QWidget>

class QLabel;
class QFrame;
class QTabWidget;
class QVBoxLayout;
class K3AboutContainer;
class K3AboutContainerBasePrivate;

/**
 * Used internally by K3AboutDialog
 * @internal
 */
class K3AboutContainerBase : public QWidget
{
  Q_OBJECT

  public:
    enum LayoutType
    {
      AbtPlain         = 0x0001,
      AbtTabbed        = 0x0002,
      AbtTitle         = 0x0004,
      AbtImageLeft     = 0x0008,
      AbtImageRight    = 0x0010,
      AbtImageOnly     = 0x0020,
      AbtProduct       = 0x0040,
      AbtKDEStandard   = AbtTabbed|AbtTitle|AbtImageLeft,
      AbtAppStandard   = AbtTabbed|AbtTitle|AbtProduct,
      AbtImageAndTitle = AbtPlain|AbtTitle|AbtImageOnly
    };

  public:
    K3AboutContainerBase( int layoutType, QWidget *parent = 0 );
    QSize sizeHint( void ) const;

    void setTitle( const QString &title );
    void setImage( const QString &fileName );
    void setImageBackgroundColor( const QColor &color );
    void setImageFrame( bool state );
    void setProgramLogo( const QString &fileName );
    void setProgramLogo( const QPixmap &pixmap );
    void setProduct( const QString &appName, const QString &version,
		     const QString &author, const QString &year );

    QFrame *addTextPage( const QString &title, const QString &text,
			 bool richText=false, int numLines=10 );
    QFrame *addLicensePage( const QString &title, const QString &text,
			 int numLines=10 );
    K3AboutContainer *addContainerPage( const QString &title,
      Qt::Alignment childAlignment = Qt::AlignCenter, Qt::Alignment innerAlignment = Qt::AlignCenter );
    K3AboutContainer *addScrolledContainerPage( const QString &title,
      Qt::Alignment childAlignment = Qt::AlignCenter, Qt::Alignment innerAlignment = Qt::AlignCenter );

    QFrame *addEmptyPage( const QString &title );

    K3AboutContainer *addContainer( Qt::Alignment childAlignment, Qt::Alignment innerAlignment );

  protected:
    void fontChange( const QFont &oldFont );

  private:
    QVBoxLayout *mTopLayout;
    QLabel *mImageLabel;
    QLabel *mTitleLabel;
    QLabel *mIconLabel;
    QLabel *mVersionLabel;
    QLabel *mAuthorLabel;
    QFrame *mImageFrame;
    QTabWidget *mPageTab;
    QFrame *mPlainSpace;

    K3AboutContainerBasePrivate* const d;
};

#endif

