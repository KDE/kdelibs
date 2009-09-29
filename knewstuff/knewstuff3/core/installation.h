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
#ifndef KNEWSTUFF2_INSTALLATION_H
#define KNEWSTUFF2_INSTALLATION_H

#include <knewstuff2/knewstuff_export.h>

#include <QtCore/QString>

namespace KNS
{

struct InstallationPrivate;

// FIXME: maybe it's smarter to let this class perform the installation
// so we can reduce the size of KNS::CoreEngine a bit?

/**
 * @short KNewStuff entry installation.
 *
 * The installation class stores all information related to an entry's
 * installation. However, the installation itself is performed by the
 * engine.
 *
 * @author Josef Spillner (spillner@kde.org)
 *
 * @internal
 */
class KNEWSTUFF_EXPORT Installation
{
public:
    /**
     * Constructor.
     */
    Installation();

    /**
     * Destructor.
     */
    ~Installation();

    enum Policy {
        CheckNever,
        CheckIfPossible,
        CheckAlways
    };

    enum Scope {
        ScopeUser,
        ScopeSystem
    };

    void setUncompression(const QString& uncompression);
    void setCommand(const QString& command);
    void setUninstallCommand(const QString& command);

    void setStandardResourceDir(const QString& dir);
    void setTargetDir(const QString& dir);
    void setInstallPath(const QString& dir);
    void setAbsoluteInstallPath(const QString& dir);

    void setScope(Scope scope);
    void setChecksumPolicy(Policy policy);
    void setSignaturePolicy(Policy policy);

    void setCustomName(bool customname);

    QString uncompression() const;
    QString command() const;
    QString uninstallCommand() const;

    QString standardResourceDir() const;
    QString targetDir() const;
    QString installPath() const;
    QString absoluteInstallPath() const;
    bool isRemote() const;

    Policy checksumPolicy() const;
    Policy signaturePolicy() const;
    Scope scope() const;

    bool customName() const;

private:
    Q_DISABLE_COPY(Installation)
    InstallationPrivate * const d;
};

}

#endif
