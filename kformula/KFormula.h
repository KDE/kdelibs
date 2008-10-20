/* This file is part of the KDE project
   Copyright (C) 2008 John Tapsell <tapsell@kde.org>

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

#ifndef KFORMULA_H
#define KFORMULA_H

#include <QtGui/QWidget>
#include <kdemacros.h>
#include <kcomponentdata.h>
#include "kformula_export.h"

class QString;
class KFormulaPrivate;

class KFORMULA_EXPORT KFormula : public QWidget {
  Q_OBJECT
  Q_PROPERTY( QString mathML READ mathML WRITE setMathML )
  Q_PROPERTY( Qt::Alignment alignment READ alignment WRITE setAlignment )

  public:
    KFormula(QWidget *parent);
    virtual ~KFormula();

    QString mathML() const;
    void paintEvent ( QPaintEvent * event );
    virtual QSize sizeHint() const;
    Qt::Alignment alignment () const;
    void setAlignment ( Qt::Alignment );
  public Q_SLOTS:
    void setMathML(const QString &mathml);
    void setFont(const QFont &font);
  private:
    KFormulaPrivate *const d;
};

#endif

