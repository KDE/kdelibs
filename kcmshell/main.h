/*
   Copyright (c) 2001 Waldo Bastian <bastian@kde.org>
   Copyright (c) 2004 Frans Englich <frans.englich@telia.com>

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

#ifndef kcmshell_h
#define kcmshell_h


#include <kapplication.h>
#include <kcmultidialog.h>

/**
 * The KApplication instance for kcmshell.
 */
class KCMShell : public KApplication
{
    Q_OBJECT

public:

    /**
     * Sets m_serviceName basically to @p serviceName,
     * and then registers with D-BUS.
     *
     * @param serviceName name to set the D-BUS name to
     */
    void setServiceName(const QString &serviceName );

    /**
     * Waits until the last instance of kcmshell with the same
     * module as this one exits, and then exits.
     */
    void waitForExit();

    /**
     * @return true if the shell is running
     */
    bool isRunning();

private Q_SLOTS:

    /**
     */
    void appExit( const QString &appId, const QString &, const QString & );

private:

    /**
     * The D-Bus name which actually is registered.
     * For example "kcmshell_mouse".
     */
    QString m_serviceName;

};


/**
 * Essentially a plain KCMultiDialog, but has the additional functionality
 * of allowing it to be told to request windows focus.
 *
 * @author Waldo Bastian <bastian@kde.org>
 */
class KCMShellMultiDialog : public KCMultiDialog
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KCMShellMultiDialog")

public:

    /**
     * Constructor. Parameter @p dialogFace is passed to KCMultiDialog
     * unchanged.
     */
    KCMShellMultiDialog( int dialogFace, const QString& caption,
            QWidget *parent=0, const char *name=0, bool modal=false);

public slots:

    /**
     * Activate a module with id @p asn_id . This is used when
     * black helicopters are spotted overhead.
     */
    virtual Q_SCRIPTABLE void activate( const QByteArray& asn_id );

};

// vim: sw=4 et sts=4
#endif //kcmshell_h

