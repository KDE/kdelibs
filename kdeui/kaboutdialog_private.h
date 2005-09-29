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

#ifndef _KABOUTDIALOG_PRIVATE_H_
#define _KABOUTDIALOG_PRIVATE_H_

#include <qlabel.h>
class QFrame;
class QTabWidget;
class QVBoxLayout;

/**
 * Used internally by KAboutContainerBase
 * @internal
 */
class KImageTrackLabel : public QLabel
{
  Q_OBJECT

  public:
    enum MouseMode
    {
      MousePress = 1,
      MouseRelease,
      MouseDoubleClick,
      MouseMove
    };

  public:
    KImageTrackLabel( QWidget * parent, const char * name=0, WFlags f=0 );

  signals:
    void mouseTrack( int mode, const QMouseEvent *e );

  protected:
    virtual void mousePressEvent( QMouseEvent *e );
    virtual void mouseReleaseEvent( QMouseEvent *e );
    virtual void mouseDoubleClickEvent( QMouseEvent *e );
    virtual void mouseMoveEvent ( QMouseEvent *e );
};

class KAboutContainer;

class KAboutContainerBasePrivate;

/**
 * Used internally by KAboutDialog
 * @internal
 */
class KAboutContainerBase : public QWidget
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
    KAboutContainerBase( int layoutType, QWidget *parent = 0, char *name = 0 );
    virtual void show( void );
    virtual QSize sizeHint( void ) const;

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
    KAboutContainer *addContainerPage( const QString &title,
      int childAlignment = AlignCenter, int innerAlignment = AlignCenter );
    KAboutContainer *addScrolledContainerPage( const QString &title,
      int childAlignment = AlignCenter, int innerAlignment = AlignCenter );

    QFrame *addEmptyPage( const QString &title );

    KAboutContainer *addContainer( int childAlignment, int innerAlignment );

  public slots:
    virtual void slotMouseTrack( int mode, const QMouseEvent *e );
    virtual void slotUrlClick( const QString &url );
    virtual void slotMailClick( const QString &name, const QString &address );

  protected:
    virtual void fontChange( const QFont &oldFont );

  signals:
    void mouseTrack( int mode, const QMouseEvent *e );
    void urlClick( const QString &url );
    void mailClick( const QString &name, const QString &address );

  private:
    QVBoxLayout *mTopLayout;
    KImageTrackLabel *mImageLabel;
    QLabel  *mTitleLabel;
    QLabel  *mIconLabel;
    QLabel  *mVersionLabel;
    QLabel  *mAuthorLabel;
    QFrame  *mImageFrame;
    QTabWidget *mPageTab;
    QFrame  *mPlainSpace;

    KAboutContainerBasePrivate* const d;
};


#endif
