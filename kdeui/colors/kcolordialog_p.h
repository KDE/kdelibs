/* This file is part of the KDE libraries
    Copyright (C) 2007 Olivier Goffart

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

#ifndef KCOLORDIALOGP_H
#define KCOLORDIALOGP_H

#include <QtGui/QWidget>

namespace KDEPrivate {

/**
 * A color palette in table form.
 *
 * @author Waldo Bastian <bastian@kde.org>
 **/
class  KColorTable : public QWidget
{
  Q_OBJECT
public:
  explicit KColorTable( QWidget *parent, int minWidth=210, int cols = 16);
  ~KColorTable();
  
  void addToCustomColors( const QColor &);
  void addToRecentColors( const QColor &);
  
  QString name() const;
  
public Q_SLOTS:
  void setColors(const QString &collectionName);

Q_SIGNALS:
  void colorSelected( const QColor &, const QString & );
  void colorDoubleClicked( const QColor &, const QString & );

private:
  Q_PRIVATE_SLOT(d, void slotColorCellSelected( int index , const QColor& ))
  Q_PRIVATE_SLOT(d, void slotColorCellDoubleClicked( int index , const QColor& ))
  Q_PRIVATE_SLOT(d, void slotColorTextSelected( const QString &colorText ))
  Q_PRIVATE_SLOT(d, void slotSetColors( const QString &_collectionName ))
  Q_PRIVATE_SLOT(d, void slotShowNamedColorReadError( void ))

  void readNamedColor( void );

private:
  virtual void setPalette(const QPalette& p) { QWidget::setPalette(p); }
  
private:
  class KColorTablePrivate;
  friend class KColorTablePrivate;
  KColorTablePrivate *const d;
  
  Q_DISABLE_COPY(KColorTable)
};

}

#endif
