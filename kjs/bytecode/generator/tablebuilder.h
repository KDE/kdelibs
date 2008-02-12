/*
 *  A utilitity for building various tables and specializations for the
 *  KJS Frostbyte bytecode
 *
 *  Copyright (C) 2007, 2008 Maks Orlovich (maksim@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#ifndef TABLE_BUILDER_H
#define TABLE_BUILDER_H

#include "parser.h"

#include <QString>
#include <QList>
#include <QHash>

struct Operation
{
    QString name;
};

struct Type
{
    QString name;
    QString nativeName;
    bool im, reg, align8;
};

struct ConversionInfo
{
    QString name;
    int  cost;
    bool checked;
};

class TableBuilder: public Parser
{
public:
    TableBuilder(QTextStream* inStream, QTextStream* hStream, QTextStream* cppStream);

    void generateCode();
private:
    virtual void handleType(const QString& type, const QString& nativeName, bool im, bool rg, bool al8);
    virtual void handleConversion(const QString& name, bool immediate, bool checked,
                                  const QString& from, const QString& to, int cost);
    virtual void handleOperation(const QString& name);
    virtual void handleImpl(const QString& fnName, QStringList sig);
    virtual void handleTile(const QString& fnName, QStringList sig);

    void printConversionInfo(const QHash<QString, QHash<QString, ConversionInfo> >& table, bool last);

    QTextStream* hStream;
    QTextStream* cppStream;

    QList<Type> types;
    QStringList typeNames;

    QStringList        operationNames;
    QList<Operation>   operations;

    QStringList conversionNames;

    QHash<QString, QHash<QString, ConversionInfo> > imConversions;
    QHash<QString, QHash<QString, ConversionInfo> > rgConversions;
};

#endif
// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
