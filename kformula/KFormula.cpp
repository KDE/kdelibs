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
#include "KFormula.h"
#include "FormulaElement.h"
#include "FormulaRenderer.h"
#include "AttributeManager.h"
#include <kdebug.h>
#include <QDomDocument>
#include <QString>
#include <QByteArray>
#include <QPaintEvent>
#include <QPainter>
#include <QSize>

class KFormulaPrivate {
public:
    KFormula *q;
    QString mathML;
    FormulaElement formula;
    FormulaRenderer formulaRenderer;
    QSize sizeHint;
    Qt::Alignment alignment;

    int x_offset;
    int y_offset;


KFormulaPrivate() {
    x_offset = 0;
    y_offset = 0;
}

void layoutFormula() {
    formulaRenderer.attributeManager()->setDefaultFont(q->font());
    formulaRenderer.layoutElement( &formula );

    QSize newSizeHint( formula.width(), formula.height() );
    if(newSizeHint != sizeHint) {
        sizeHint = newSizeHint;
        q->updateGeometry();
    }
}

};

KFormula::KFormula(QWidget *parent) : QWidget(parent), d(new KFormulaPrivate)
{
    d->q = this;
    QDomDocument doc;
    setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    setMathML("<xml version=\"1.0\"><mfrac><mi>a</mi><mi>b</mi></mfrac></xml>");
}
KFormula::~KFormula()
{
    delete d;
}
void KFormula::setMathML(const QString &mathml) 
{

    QDomDocument doc;
    doc.setContent( mathml );
    d->formula = FormulaElement();
    d->formula.readMathML(doc.documentElement());
    d->mathML = mathml;  
    d->layoutFormula();
    update();
}

QSize KFormula::sizeHint() const {
    return d->sizeHint;
}

QString KFormula::mathML() const {
    return d->mathML;
}

void KFormula::setFont(const QFont &font) 
{
    QWidget::setFont(font);
}
void KFormula::paintEvent ( QPaintEvent * /*event */)
{
    QPainter painter(this);
    if( font() != d->formulaRenderer.attributeManager()->defaultFont() ) {
        d->layoutFormula();
    }

    if( d->alignment & Qt::AlignLeft || d->alignment & Qt::AlignJustify)
        d->x_offset = 0;
    else if( d->alignment & Qt::AlignRight)
        d->x_offset = width() - d->formula.width();
    else if( d->alignment & Qt::AlignHCenter)
        d->x_offset = (width() - d->formula.width())/2;

    if( d->alignment & Qt::AlignTop)
        d->y_offset = 0;
    else if( d->alignment & Qt::AlignBottom)
        d->y_offset = height() - d->formula.height();
    else if( d->alignment & Qt::AlignVCenter)
        d->y_offset = (height() - d->formula.height())/2;


    painter.translate(d->x_offset, d->y_offset);
    d->formulaRenderer.paintElement( painter, &d->formula);
}

Qt::Alignment KFormula::alignment () const
{
    return d->alignment;
}
void KFormula::setAlignment ( Qt::Alignment alignment)
{
    d->alignment = alignment;
    d->layoutFormula();
    update();
}
#include "KFormula.moc"
