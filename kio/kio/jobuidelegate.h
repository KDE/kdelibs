/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                       David Faure <faure@kde.org>
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

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

#ifndef KIO_JOBUI_H
#define KIO_JOBUI_H

#include <kjobuidelegate.h>

class KJob;
namespace KIO
{
class Job;

/**
 * A UI delegate tuned to be used with KIO Jobs.
 *
 * It uses KIO::Observer to do the tracking.
 */
class KIO_EXPORT JobUiDelegate : public KJobUiDelegate
{
    Q_OBJECT

public:
    /**
     * Constructs a new KIO Job UI delegate.
     *
     * @param showProgressInfo indicates if this delegate should
     * show the progress of the job or not
     */
    JobUiDelegate( bool showProgressInfo );

    /**
     * Destroys the KIO Job UI delegate.
     */
    virtual ~JobUiDelegate();

public:

    /**
     * Associate this job with a window given by @p window.
     * @param window the window to associate to
     * @see window()
     */
    void setWindow(QWidget *window);

    /**
     * Returns the window this job is associated with.
     * @return the associated window
     * @see setWindow()
     */
    QWidget *window() const;

    virtual void showErrorMessage();

protected:
     virtual void connectJob( KJob *job );

protected Q_SLOTS:
    void slotFinished( KJob *job, int id );
    void slotWarning( KJob *job, const QString &errorText );

private:
    class Private;
    Private * const d;
};
}

#endif
