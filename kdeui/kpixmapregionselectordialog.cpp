/* This file is part of the KDE libraries
    Copyright (C) 2004 Antonio Larrosa <larrosa@kde.org

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

#include "kpixmapregionselectordialog.h"
#include <kdialogbase.h>
#include <qdialog.h>
#include <qdesktopwidget.h>
#include <klocale.h>
#include <kdialog.h>
#include <kvbox.h>

KPixmapRegionSelectorDialog::KPixmapRegionSelectorDialog(QWidget *parent,
     bool modal ) : KDialog(parent, i18n("Select Region of Image"), Help|Ok|Cancel )
{
  enableButtonSeparator( true );
  setModal( modal );

  KVBox *vbox=new KVBox(this);
  new QLabel(i18n("Please click and drag on the image to select the region of interest:"), vbox);
  m_pixmapSelectorWidget= new KPixmapRegionSelectorWidget(vbox);

  vbox->setSpacing( KDialog::spacingHint() );

  setMainWidget(vbox);
}

KPixmapRegionSelectorDialog::~KPixmapRegionSelectorDialog()
{
}

QRect KPixmapRegionSelectorDialog::getSelectedRegion(const QPixmap &pixmap, QWidget *parent )
{
  KPixmapRegionSelectorDialog dialog(parent);

  dialog.pixmapRegionSelectorWidget()->setPixmap(pixmap);

  QDesktopWidget desktopWidget;
  QRect screen=desktopWidget.availableGeometry();
  dialog.pixmapRegionSelectorWidget()->setMaximumWidgetSize(
        (int)(screen.width()*4.0/5), (int)(screen.height()*4.0/5));

  int result = dialog.exec();

  QRect rect;

  if ( result == QDialog::Accepted )
    rect = dialog.pixmapRegionSelectorWidget()->unzoomedSelectedRegion();

  return rect;
}

QRect KPixmapRegionSelectorDialog::getSelectedRegion(const QPixmap &pixmap, int aspectRatioWidth, int aspectRatioHeight, QWidget *parent )
{
  KPixmapRegionSelectorDialog dialog(parent);

  dialog.pixmapRegionSelectorWidget()->setPixmap(pixmap);
  dialog.pixmapRegionSelectorWidget()->setSelectionAspectRatio(aspectRatioWidth,aspectRatioHeight);

  QDesktopWidget desktopWidget;
  QRect screen=desktopWidget.availableGeometry();
  dialog.pixmapRegionSelectorWidget()->setMaximumWidgetSize(
        (int)(screen.width()*4.0/5), (int)(screen.height()*4.0/5));

  int result = dialog.exec();

  QRect rect;

  if ( result == QDialog::Accepted )
    rect = dialog.pixmapRegionSelectorWidget()->unzoomedSelectedRegion();

  return rect;
}

QImage KPixmapRegionSelectorDialog::getSelectedImage(const QPixmap &pixmap, QWidget *parent )
{
  KPixmapRegionSelectorDialog dialog(parent);

  dialog.pixmapRegionSelectorWidget()->setPixmap(pixmap);

  QDesktopWidget desktopWidget;
  QRect screen=desktopWidget.availableGeometry();
  dialog.pixmapRegionSelectorWidget()->setMaximumWidgetSize(
        (int)(screen.width()*4.0/5), (int)(screen.height()*4.0/5));
  int result = dialog.exec();

  QImage image;

  if ( result == QDialog::Accepted )
    image = dialog.pixmapRegionSelectorWidget()->selectedImage();

  return image;
}

QImage KPixmapRegionSelectorDialog::getSelectedImage(const QPixmap &pixmap, int aspectRatioWidth, int aspectRatioHeight, QWidget *parent )
{
  KPixmapRegionSelectorDialog dialog(parent);

  dialog.pixmapRegionSelectorWidget()->setPixmap(pixmap);
  dialog.pixmapRegionSelectorWidget()->setSelectionAspectRatio(aspectRatioWidth,aspectRatioHeight);

  QDesktopWidget desktopWidget;
  QRect screen=desktopWidget.availableGeometry();
  dialog.pixmapRegionSelectorWidget()->setMaximumWidgetSize(
        (int)(screen.width()*4.0/5), (int)(screen.height()*4.0/5));

  int result = dialog.exec();

  QImage image;

  if ( result == QDialog::Accepted )
    image = dialog.pixmapRegionSelectorWidget()->selectedImage();

  return image;
}

