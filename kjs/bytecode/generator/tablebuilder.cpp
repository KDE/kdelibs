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
#include "tablebuilder.h"

TableBuilder::TableBuilder(QTextStream* inStream, QTextStream* hStream, QTextStream* cppStream):
    Parser(inStream), hStream(hStream), cppStream(cppStream)
{}

void TableBuilder::generateCode()
{
    parse();
}

void TableBuilder::handleType(const QString& type)
{
    types << type;
}

void TableBuilder::handleConversion(bool immediate, bool checked, QString from, QString to, int cost) {}

void TableBuilder::handleOperation(const QString& name)
{}

void TableBuilder::handleImpl(const QString& fnName, QStringList sig)
{}

void TableBuilder::handleTile(const QString& fnName, QStringList sig)
{}



// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
