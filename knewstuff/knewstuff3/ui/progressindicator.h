/*
    This file is part of KNewStuff2.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>
    Copyright (c) 2008 Jeremy Whiting <jeremy@scitools.com>
    Copyright (c) 2009 Frederik Gladhorn <gladhorn@kde.org>

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

#ifndef KNEWSTUFF3_UI_PROGRESSINDICATOR_H
#define KNEWSTUFF3_UI_PROGRESSINDICATOR_H

#include <QtGui/QFrame>
#include <QtCore/QHash>
#include <QtCore/QString>

class KJob;
class QVBoxLayout;
class QProgressBar;
class QPushButton;

namespace KNS3 {

/**
 * Embedded progress indicator for the download dialog.
 *
 * The indicator can display various asynchronous operations at once.
 * Each operation can also individually be cancelled.
 *
 * @internal
 */
class ProgressIndicator : public QFrame
{
    Q_OBJECT
public:
    ProgressIndicator(QWidget *parent);
    
public Q_SLOTS:
     void addJob(KJob*, const QString& label);
    
private Q_SLOTS:
    void jobFinished(KJob*);

private:
    void calculateAverage();

    QHash<KJob*, QString> m_jobs;
    
    QVBoxLayout *m_detailsvbox;
    QProgressBar *m_pb;
    
    uint m_finished;
    uint m_total;
};
}

#endif
