/*
    This file is part of KNewStuff2.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "installation.h"

struct KNS::InstallationPrivate {
    InstallationPrivate() {
        m_checksumpolicy = Installation::CheckIfPossible;
        m_signaturepolicy = Installation::CheckIfPossible;
        m_scope = Installation::ScopeUser;
        m_customname = false;
    }

    QString m_command;
    QString m_uninstallCommand;
    QString m_uncompression;
    QString m_standardresourcedir;
    QString m_targetdir;
    QString m_installpath;
    QString m_absoluteinstallpath;
    Installation::Policy m_checksumpolicy;
    Installation::Policy m_signaturepolicy;
    Installation::Scope m_scope;
    bool m_customname;
};

using namespace KNS;

Installation::Installation()
        : d(new InstallationPrivate)
{
}

Installation::~Installation()
{
    delete d;
}

void Installation::setUncompression(const QString& uncompression)
{
    d->m_uncompression = uncompression;
}

void Installation::setCommand(const QString& command)
{
    d->m_command = command;
}

void Installation::setUninstallCommand(const QString& command)
{
    d->m_uninstallCommand = command;
}

void Installation::setStandardResourceDir(const QString& dir)
{
    d->m_standardresourcedir = dir;
}

void Installation::setTargetDir(const QString& dir)
{
    d->m_targetdir = dir;
}

void Installation::setInstallPath(const QString& dir)
{
    d->m_installpath = dir;
}

void Installation::setAbsoluteInstallPath(const QString& dir)
{
    d->m_absoluteinstallpath = dir;
}

void Installation::setChecksumPolicy(Policy policy)
{
    d->m_checksumpolicy = policy;
}

void Installation::setSignaturePolicy(Policy policy)
{
    d->m_signaturepolicy = policy;
}

void Installation::setScope(Scope scope)
{
    d->m_scope = scope;
}

void Installation::setCustomName(bool customname)
{
    d->m_customname = customname;
}

QString Installation::uncompression() const
{
    return d->m_uncompression;
}

QString Installation::command() const
{
    return d->m_command;
}

QString Installation::uninstallCommand() const
{
    return d->m_uninstallCommand;
}

QString Installation::standardResourceDir() const
{
    return d->m_standardresourcedir;
}

QString Installation::targetDir() const
{
    return d->m_targetdir;
}

QString Installation::installPath() const
{
    return d->m_installpath;
}

QString Installation::absoluteInstallPath() const
{
    return d->m_absoluteinstallpath;
}

bool Installation::isRemote() const
{
    if (!installPath().isEmpty()) return false;
    if (!targetDir().isEmpty()) return false;
    if (!absoluteInstallPath().isEmpty()) return false;
    if (!standardResourceDir().isEmpty()) return false;
    return true;
}

Installation::Policy Installation::checksumPolicy() const
{
    return d->m_checksumpolicy;
}

Installation::Policy Installation::signaturePolicy() const
{
    return d->m_signaturepolicy;
}

bool Installation::customName() const
{
    return d->m_customname;
}

Installation::Scope Installation::scope() const
{
    return d->m_scope;
}

