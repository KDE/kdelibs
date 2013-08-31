/*
    Copyright 2013  Albert Vaca <albertvaka@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "lineediturldropeventfilter.h"

#include <QLineEdit>
#include <QEvent>
#include <qmimedata.h>
#include <QDropEvent>

static const char s_kdeUriListMime[] = "application/x-kde4-urilist";

LineEditUrlDropEventFilter::LineEditUrlDropEventFilter(QObject *parent)
    : QObject(parent)
{

}

LineEditUrlDropEventFilter::~LineEditUrlDropEventFilter()
{

}

bool LineEditUrlDropEventFilter::eventFilter(QObject* obj, QEvent* ev)
{
    //Handle only drop events
    if (ev->type() != QEvent::Drop) {
        return false;
    }
    QDropEvent *dropEv = static_cast<QDropEvent*>(ev);

    //Handle only url drops, we check the mimetype for the standard or kde's urllist
    //It would be interesting to handle urls that don't have any mimetype set (like a drag and drop from kate)
    const QMimeData* data = dropEv->mimeData();
    if (!data->hasUrls() && !data->hasFormat(QLatin1String(s_kdeUriListMime))) {
        return false;
    }

    //Our object should be a QLineEdit
    QLineEdit* line = qobject_cast<QLineEdit*>(obj);
    if (!line) {
        return false;
    }

    QString content = data->text();
    line->setText(content);
    line->setCursorPosition(content.length());

    ev->accept();
    return true;

}
