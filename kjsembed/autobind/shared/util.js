/* This file is part of the KDE libraries
    Copyright (C) 2005, 2006 KJSEmbed Authors
    See included AUTHORS file.

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
//debug('util.js');

// contains(string, part)
//   string - string to search
//   part   - sub-string to search for
// Returns true if part is contained in string
function contains( string, part )
{
    if (string.indexOf(part) != -1)
        return true;
    return false;
}

// Regular expression used to spot const & type args (i.e. 'const QString &').
const_ref_rx = /const\s+(\w+)\s*&/;

// Regular expression used to spot pointer type args (i.e. 'QString *').
ptr_rx = /(\w+)\s*\*/;

// findCoreParamType(paramType)
//   paramType - The potentially embellished type
// Returns the core parameter type (sans embellishments).
function findCoreParamType( paramType )
{
    var coreParamTypeMatch = const_ref_rx.exec(paramType);

    // We want the core parameter type
    if (coreParamTypeMatch != null)
        return coreParamTypeMatch[1];

//    coreParamTypeMatch = ptr_rx.exec(paramType);
//    if (coreParamTypeMatch != null)
//        return coreParamTypeMatch[1];

    return paramType;
}

// isPointer(paramType)
//   paramType The parameter type
// Returns true if the passed in type is a pointer type.
function isPointer( paramType )
{
   return (ptr_rx.exec(paramType) != null);
}

// An array of primitive Qt types, this is annoying but seems to be necessary
// 1-4 - Represent core variant types
// 2   - Is also a bool
// 3   - Is also an integer 
// 4   - Is also a floating point number
// 5   - Is a non-variant floating point number.
// 6   - Is a non-variant integer.
var data_types = {
    // Actual variant types
    "QBitArray" : 1, "QBitmap" : 1, "bool" : 2, "QBrush" : 1,
    "QByteArray" : 1, "QChar" : 1, "QColor" : 1, "QCursor" : 1,
    "QDate" : 1, "QDateTime" : 1, "double" : 4, "QFont" : 1,
    "QIcon" : 1, "QImage" : 1, "int" : 3, "QKeySequence" : 1,
    "QLine" : 1, "QLineF" : 1, "QVariantList" : 1, "QLocale" : 1,
    "qlonglong" : 3, "QVariantMap" : 1, "QPalette" : 1, "QPen" : 1,
    "QPixmap" : 1, "QPoint" : 1, "QPointArray" : 1, "QPointF" : 1,
    "QPolygon" : 1, "QRect" : 1, "QRectF" : 1, "QRegExp" : 1,
    "QRegion" : 1, "QSize" : 1, "QSizeF" : 1, "QSizePolicy" : 1,
    "QString" : 1, "QStringList" : 1, "QTextFormat" : 1,
    "QTextLength" : 1, "QTime" : 1, "uint" : 3, "qulonglong" : 3,
    "QUrl" : 1, 

     // Other necessary qglobal.h types.
     "qreal" : 5, "float" : 5, "qint8" : 6, "quint8" : 6, "qint16" : 6, "quint16" : 6, 
     "qint32" : 6, "quint32" : 6, "qint64" : 6, "quint64" : 6, 
     "qulonglong" : 6,
     "char" : 6, "uchar" : 6, "ushort" : 6, "ulong" : 6, 
    "short" : 6, "long" : 6, 
};

// function isVariant( paramType )
//   paramType - The parameter type to check (preferably sans embellishments).
// Returns true if the type can be represented in an enum
function isVariant( paramType )
{
//    debug(paramType + " isVariant " + data_types[paramType]);
    if ((data_types[paramType] >= 1) || (data_types[paramType] <= 4))
      return true;
    else
      return false;
}

// function isNumber( paramType )
//   paramType - The parameter type to check (preferably sans embellishments).
// Returns true if the type is a number (floating point or integer).
function isNumber( paramType )
{
//    debug(paramType + " isNumber " + data_types[paramType]);
    if ((data_types[paramType] >= 3) && data_types[paramType] <= 6)
      return true;
    else
      return false;
}

// function isInteger( paramType )
//   paramType - The parameter type to check (preferably sans embellishments).
// Returns true if the type is an integer type.
function isInteger( paramType )
{
//    debug(paramType + " isInteger " + data_types[paramType]);
    if ((data_types[paramType] == 3) || data_types[paramType] == 6)
      return true;
    else
      return false;
}

// function isInteger( paramType )
//   paramType - The parameter type to check (preferably sans embellishments).
// Returns true if the type is a boolean type.
function isBool( paramType )
{
//    debug(paramType + " isBool " + data_types[paramType]);
    if (data_types[paramType] == 2)
      return true;
    else
      return false;
}

// function isQtEnum( paramType )
//   paramType - The parameter type to check (preferably sans embellishments).
// Returns true if the type is a Qt namespace enum.
function isQtEnum( paramType )
{
    return methodType.indexOf('Qt::') != -1;
}

// function isCompoundEnum( paramType )
//   paramType - The parameter type to check (preferably sans embellishments).
//   compoundEnums - Associative array of enum types to containing objects
// Returns true if the type is an enum contained in a compound type.
function isCompoundEnum( paramType, compoundEnums )
{
    return (compoundEnums[paramType]);
}

// function isCompoundEnum( paramType )
//   paramType - The parameter type to check (preferably sans embellishments).
//   compoundEnums - Associative array of enum types to containing objects
// Returns true if the paramType is any known enum type.
function isEnum( paramType, globalEnums, compoundEnums )
{
    return (globalEnums[paramType] || // it is a Qt enum
            compoundEnums[paramType]);
}

