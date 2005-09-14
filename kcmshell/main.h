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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.

*/

#ifndef kcmshell_h
#define kcmshell_h

#include <dcopobject.h>

#include <Q3CString>

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
     * Sets m_dcopName basically to @p dcopName,
     * and then registers with DCOP.
     *
     * @param dcopName name to set the DCOP name to
     * @param rootMode true if the kcmshell is embedding
     */
    void setDCOPName(const Q3CString &dcopName, bool rootMode );

    /**
     * Waits until the last instance of kcmshell with the same
     * module as this one exits, and then exits.
     */
    void waitForExit();

    /**
     * @return true if the shell is running
     */
    bool isRunning();

private slots:

    /**
     */
    void appExit( const DCOPCString &appId );

private:

    /**
     * The DCOP name which actually is registered.
     * For example "kcmshell_mouse".
     */
    QByteArray m_dcopName;

};


/**
 * Essentially a plain KCMultiDialog, but has the additional functionality
 * of allowing it to be told to request windows focus.
 *
 * @author Waldo Bastian <bastian@kde.org>
 */
class KCMShellMultiDialog : public KCMultiDialog, public DCOPObject
{
    Q_OBJECT
    K_DCOP

public:

    /**
     */
    KCMShellMultiDialog( int dialogFace, const QString& caption,
            QWidget *parent=0, const char *name=0, bool modal=false);

k_dcop:

    /**
     */
    virtual void activate( Q3CString asn_id );

};

// vim: sw=4 et sts=4
#endif //kcmshell_h

