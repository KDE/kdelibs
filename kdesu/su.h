/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 *
 * This is free software; you can use this library under the GNU Library 
 * General Public License, version 2. See the file "COPYING.LIB" for the 
 * exact licensing terms.
 */

#ifndef __SU_h_Included__
#define __SU_h_Included__

#include <qcstring.h>

#include "stub.h"

/**
 * Executes a command under elevated privileges, using su.
 */

class SuProcess: public StubProcess
{
public:
    SuProcess(QCString user=0, QCString command=0);
    ~SuProcess();

    enum Errors { SuNotFound=1, SuNotAllowed, SuIncorrectPassword };

    /** Executes the command. This will wait for the command to finish. */
    int exec(const char *password, int check=0);

    /** 
     * Checks if the stub is installed and the password is correct.
     * @return Zero if everything is correct, nonzero otherwise.
     */
    int checkInstall(const char *password);

    /**
     * Checks if a password is needed.
     */
    int checkNeedPassword();

protected:
    virtual QCStringList dcopServer();

private:
    int ConverseSU(const char *password);

    class SuProcessPrivate;
    SuProcessPrivate *d;
};

#endif
