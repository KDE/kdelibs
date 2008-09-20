/* This file is part of the KDE project

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; only
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QtCore/QBuffer>
#include <QtXml/QDomDocument>
#include <QFile>

#include <kdebug.h>
#include <KoXmlWriter.h>
#include <KoXmlReader.h>

#include "BasicElement.h"
#include "IdentifierElement.h"
#include "NumberElement.h"
#include "OperatorElement.h"
#include "TextElement.h"
#include "SpaceElement.h"
#include "StringElement.h"
#include "GlyphElement.h"
#include "RowElement.h"
#include "FractionElement.h"
#include "RootElement.h"
#include "StyleElement.h"
#include "ErrorElement.h"
#include "PaddedElement.h"
#include "PhantomElement.h"
#include "FencedElement.h"
#include "EncloseElement.h"
#include "MultiscriptElement.h"
#include "UnderOverElement.h"
#include "ActionElement.h"
#include "FormulaElement.h"

void load(BasicElement* element, const QString& input)
{
    KoXmlDocument doc;
    doc.setContent( input );
    element->readMathML(doc.documentElement());
}

int count( const QList<BasicElement*>& list )
{
    BasicElement* element;
    int counter = 0;
    foreach ( element, list ) {
        counter += count( element->childElements() );
    }
    counter += list.count();
    return counter;
}

int main(void) {
    QTextStream cin(stdin, QIODevice::ReadOnly);
    QString input = cin.readAll();
    FormulaElement *formula = new FormulaElement();
    load(formula, input);
   
    QByteArray byteArray;
    QBuffer tmp(&byteArray);
    tmp.open(QIODevice::WriteOnly);
    KoXmlWriter writer( &tmp );
    formula->writeMathML(&writer);
    tmp.close();

    QTextStream cout(stdout, QIODevice::WriteOnly);
    cout << input << endl << "------------------------------------------------" << endl;
    cout << byteArray << endl;
}

