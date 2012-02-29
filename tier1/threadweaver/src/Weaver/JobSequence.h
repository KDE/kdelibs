/* -*- C++ -*-

   This file declares the JobSequence class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2004, 2005, 2006 Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $

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

   $Id: DebuggingAids.h 30 2005-08-16 16:16:04Z mirko $
*/

#ifndef JOBSEQUENCE_H
#define JOBSEQUENCE_H

#include "JobCollection.h"

namespace ThreadWeaver {

    /** A JobSequence is a vector of Jobs that will be executed in a sequence.

        It is implemented by automatically creating the necessary dependencies
        between the Jobs in the sequence.

        JobSequence provides a handy cleanup and unwind mechanism: the stop()
        slot. If it is called, the processing of the sequence will stop, and
        all its remaining Jobs will be dequeued.
    */
    class THREADWEAVER_EXPORT JobSequence : public JobCollection
    {
        Q_OBJECT

    public:
        explicit JobSequence ( QObject *parent = 0 );

    protected:
        /** Overload to queue the sequence. */
        void aboutToBeQueued ( WeaverInterface *weaver );

        /** reimplemented */
        void internalJobDone( Job* );

    private:
        class Private;
        Private * const d;
    };

}

#endif
