/*
    This file is part of libkabc.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KABC_RESOURCECONFIGWIDGET_H
#define KABC_RESOURCECONFIGWIDGET_H

#include <qwidget.h>

#include <kconfig.h>

namespace KABC {

/**
  @short Base widget for editing resource settings
  
  The @ref loadSettings() and @ref saveSettings() slots should be
  reimplemented to store the resource specific settings.
*/
class ResourceConfigWidget : public QWidget
{
  Q_OBJECT

public:

  /**
    Constructor.
   */
  ResourceConfigWidget( QWidget *parent = 0, const char *name = 0 );

  /**
    Sets the widget to 'edit' mode. Reimplement this method if you are
    interested in the mode change (to disable some GUI element for
    example). By default the widget is in 'add' mode.
   */
  virtual void setEditMode( bool value );

public slots:
  /**
    This method is called by the @ref ResourceConfigDialog to
    load the resource specific settings. You have to reimplement
    it. @param config points to a config object where you can
    read the settings. Don't use KConfig::setGroup() on this pointer.
   */
  virtual void loadSettings( KConfig *config );

  /**
    This method is called by the @ref ResourceConfigDialog to
    save the resource specific settings. You have to reimplement
    it. @param config points to a config object where you can
    write the settings. Don't use KConfig::setGroup() on this pointer.
   */
  virtual void saveSettings( KConfig *config );

signals:
  void setResourceName( const QString &name );
  void setReadOnly( bool value );
  void setFast( bool value );

private:
  class ResourceConfigWidgetPrivate;
  ResourceConfigWidgetPrivate *d;
};

}
#endif
