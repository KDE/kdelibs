/*
    This file is part of KNewStuff2.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>

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

#include "installation.h"

class InstallationPrivate
{
  public:
  InstallationPrivate(){}
};

using namespace KNS;

Installation::Installation()
{
	d = NULL;
	m_checksumpolicy = CheckIfPossible;
	m_signaturepolicy = CheckIfPossible;
}

Installation::~Installation()
{
}

void Installation::setUncompression(const QString& uncompression)
{
	m_uncompression = uncompression;
}

void Installation::setCommand(const QString& command)
{
	m_command = command;
}

void Installation::setStandardResourceDir(const QString& dir)
{
	m_standardresourcedir = dir;
}

void Installation::setTargetDir(const QString& dir)
{
	m_targetdir = dir;
}

void Installation::setInstallPath(const QString& dir)
{
	m_installpath = dir;
}

void Installation::setChecksumPolicy(Policy policy)
{
	m_checksumpolicy = policy;
}

void Installation::setSignaturePolicy(Policy policy)
{
	m_signaturepolicy = policy;
}

QString Installation::uncompression() const
{
	return m_uncompression;
}

QString Installation::command() const
{
	return m_command;
}

QString Installation::standardResourceDir() const
{
	return m_standardresourcedir;
}

QString Installation::targetDir() const
{
	return m_targetdir;
}

QString Installation::installPath() const
{
	return m_installpath;
}

bool Installation::isRemote()
{
	if(!installPath().isEmpty()) return false;
	if(!targetDir().isEmpty()) return false;
	if(!standardResourceDir().isEmpty()) return false;
	return true;
}

Installation::Policy Installation::checksumPolicy()
{
	return m_checksumpolicy;
}

Installation::Policy Installation::signaturePolicy()
{
	return m_signaturepolicy;
}

