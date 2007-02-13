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
#ifndef KNEWSTUFF2_INSTALLATION_H
#define KNEWSTUFF2_INSTALLATION_H

#include <kdelibs_export.h>

#include <qstring.h>

namespace KNS {

/**
 * @short KNewStuff entry installation.
 *
 * @author Josef Spillner (spillner@kde.org)
 */
class KDE_EXPORT Installation
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

    void setUncompression(const QString& uncompression);
    void setCommand(const QString& command);

    void setStandardResourceDir(const QString& dir);
    void setTargetDir(const QString& dir);
    void setInstallPath(const QString& dir);

    QString uncompression() const;
    QString command() const;

    QString standardResourceDir() const;
    QString targetDir() const;
    QString installPath() const;

  private:
    QString m_command;
    QString m_uncompression;
    QString m_standardresourcedir;
    QString m_targetdir;
    QString m_installpath;
    class InstallationPrivate *d;
};

}

#endif
