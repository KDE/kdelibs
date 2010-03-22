/*
    This file is part of KNewStuff2.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>
    Copyright (c) 2008 Jeremy Whiting <jpwhiting@kde.org>
    Copyright (c) 2009-2010 Frederik Gladhorn <gladhorn@kde.org>

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
#include <kpixmapsequence.h>

class QVBoxLayout;
class QLabel;
class QString;
class KPixmapSequenceWidget;

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
    void busy(const QString& message);
    void error(const QString& message);
    void idle(const QString& message);

private:
    QVBoxLayout *m_detailsvbox;
    QLabel* m_statusLabel;
    KPixmapSequenceWidget* busyWidget;

    KPixmapSequence m_busyPixmap;
    KPixmapSequence m_errorPixmap;
};
}

#endif
