/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 *
 * This is free software; you can use this library under the GNU Library
 * General Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 */


/**
 * PTY compatibility routines. This class tries to emulate a UNIX98 PTY API
 * on various platforms.
 */
#ifndef __PTY_h_Included__
#define __PTY_h_Included__

#include <qbytearray.h>

#include <kdelibs_export.h>

class KDESU_EXPORT PTY {

public:
    /**
     * Construct a PTY object.
     */
    PTY();

    /**
     * Destructs the object. The PTY is closed if it is still open.
     */
    ~PTY();

    /**
     * Allocate a pty.
     * @return A filedescriptor to the master side.
     */
    int getpt();

    /**
     * Grant access to the slave side.
     * @return Zero if succesfull, < 0 otherwise.
     */
    int grantpt();

    /**
     * Unlock the slave side.
     * @return Zero if successful, < 0 otherwise.
     */
    int unlockpt();

    /**
     * Get the slave name.
     * @return The slave name.
     */
    QByteArray ptsname();

private:

    int ptyfd;
    QString ptyname, ttyname;

    class PTYPrivate;
    PTYPrivate *d;
};

#endif  // __PTY_h_Included__
