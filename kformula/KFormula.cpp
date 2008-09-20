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
#include "FormulaElement.h"
#include "FormulaRenderer.h"
#include <QDomDocument>
#include <QString>
#include <QByteArray>
#include <QPaintEvent>
#include <QPainter>
#include "KFormula.h"
#include "AttributeManager.h"
#include <kdebug.h>

struct KFormulaPrivate {
    QString mathML;
    FormulaElement formula;
    FormulaRenderer formulaRenderer;
};

KFormula::KFormula(QWidget *parent) : QWidget(parent), d(new KFormulaPrivate)
{
    QDomDocument doc;
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
    update();
}

QString KFormula::mathML() const {
    return d->mathML;
}
void KFormula::paintEvent ( QPaintEvent * event )
{
    QPainter painter(this);
    d->formulaRenderer.attributeManager()->setDefaultFont(font());
    d->formulaRenderer.layoutElement( &d->formula );
    d->formulaRenderer.paintElement( painter, &d->formula);
}
#include "KFormula.moc"
