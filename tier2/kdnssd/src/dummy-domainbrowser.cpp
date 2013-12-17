/* This file is part of the KDE project
 *
 * Copyright (C) 2004 Jakub Stachowski <qbast@go2.pl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <QtCore/QStringList>
#include "domainbrowser.h"

namespace KDNSSD
{

DomainBrowser::DomainBrowser(DomainType, QObject *parent) : QObject(parent), d(0)
{}

DomainBrowser::~DomainBrowser()
{}


void DomainBrowser::startBrowse()
{}

QStringList DomainBrowser::domains() const
{
	return QStringList();
}

bool DomainBrowser::isRunning() const
{
	return false;
}

}
#include "moc_domainbrowser.cpp"
