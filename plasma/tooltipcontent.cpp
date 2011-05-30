/*
 * Copyright 2008 by Aaron Seigo <aseigo@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#include "tooltipcontent.h"

#include <QGraphicsWidget>
#include <QHash>
#include <QTextDocument>

#include <kiconloader.h>

namespace Plasma
{

struct ToolTipResource
{
    ToolTipResource()
    {
    }

    ToolTipResource(ToolTipContent::ResourceType t, const QVariant &v)
        : type(t),
          data(v)
    {
    }

    ToolTipContent::ResourceType type;
    QVariant data;
};

const int MAXIMUM_TEXT_LENGTH = 5000;

class ToolTipContentPrivate
{
public:
    ToolTipContentPrivate()
      : autohide(true),
        instantPopup(false),
        clickable(false),
        highlightWindows(false)
    {
    }

    QString mainText;
    QString subText;
    QPixmap image;
    QList<WId> windowsToPreview;
    QHash<QString, ToolTipResource> resources;
    QWeakPointer<QGraphicsWidget> graphicsWidget;
    bool autohide : 1;
    bool instantPopup : 1;
    bool clickable : 1;
    bool highlightWindows : 1;
};

ToolTipContent::ToolTipContent()
    : d(new ToolTipContentPrivate)
{
}

ToolTipContent::ToolTipContent(const ToolTipContent &other)
    : d(new ToolTipContentPrivate(*other.d))
{
}

ToolTipContent::~ToolTipContent()
{
    delete d;
}

ToolTipContent &ToolTipContent::operator=(const ToolTipContent &other)
{
    *d = *other.d;
    return *this;
}

ToolTipContent::ToolTipContent(const QString &mainText,
                               const QString &subText,
                               const QPixmap &image)
    : d(new ToolTipContentPrivate)
{
    setMainText(mainText);
    setSubText(subText);
    setImage(image);
}

ToolTipContent::ToolTipContent(const QString &mainText,
                               const QString &subText,
                               const QIcon &icon)
    : d(new ToolTipContentPrivate)
{
    setMainText(mainText);
    setSubText(subText);
    setImage(icon);
}

bool ToolTipContent::isEmpty() const
{
    return d->mainText.isEmpty() &&
           d->subText.isEmpty() &&
           d->image.isNull() &&
           (d->windowsToPreview.size() == 0);
}

void ToolTipContent::setMainText(const QString &text)
{
    d->mainText = text.trimmed();
}

QString ToolTipContent::mainText() const
{
    QString text = d->mainText;
    text.truncate(MAXIMUM_TEXT_LENGTH);
    return text;
}

void ToolTipContent::setSubText(const QString &text)
{
    d->subText = text.trimmed();
}

QString ToolTipContent::subText() const
{
    QString text = d->subText;
    text.truncate(MAXIMUM_TEXT_LENGTH);
    return text;
}

void ToolTipContent::setImage(const QPixmap &image)
{
    d->image = image;
}

void ToolTipContent::setImage(const QIcon &icon)
{
    d->image = icon.pixmap(IconSize(KIconLoader::Desktop));
}

QPixmap ToolTipContent::image() const
{
    return d->image;
}

void ToolTipContent::setWindowsToPreview(const QList<WId> & ids)
{
    d->windowsToPreview = ids;
}

QList<WId> ToolTipContent::windowsToPreview() const
{
    return d->windowsToPreview;
}

void ToolTipContent::setHighlightWindows(bool highlight)
{
    d->highlightWindows = highlight;
}

bool ToolTipContent::highlightWindows() const
{
    return d->highlightWindows;
}

void ToolTipContent::setAutohide(bool autohide)
{
    d->autohide = autohide;
}

bool ToolTipContent::autohide() const
{
    return d->autohide;
}

void ToolTipContent::setInstantPopup(bool enabled)
{
    d->instantPopup = enabled;
}

bool ToolTipContent::isInstantPopup() const
{
    return d->instantPopup;
}

void ToolTipContent::addResource(ResourceType type, const QUrl &path, const QVariant &resource)
{
    d->resources.insert(path.toString(), ToolTipResource(type, resource));
}

void ToolTipContent::registerResources(QTextDocument *document) const
{
    if (!document) {
        return;
    }

    QHashIterator<QString, ToolTipResource> it(d->resources);
    while (it.hasNext()) {
        it.next();
        const ToolTipResource &r = it.value();
        QTextDocument::ResourceType t(QTextDocument::ImageResource);

        switch (r.type) {
            case HtmlResource:
                t = QTextDocument::HtmlResource;
                break;
            case CssResource:
                t = QTextDocument::StyleSheetResource;
                break;
            default:
                break;
        }

        document->addResource(t, it.key(), r.data);
    }
}

void ToolTipContent::setClickable(bool clickable)
{
    d->clickable = clickable;
}

bool ToolTipContent::isClickable() const
{
    return d->clickable;
}

void ToolTipContent::setGraphicsWidget(QGraphicsWidget *widget)
{
    d->graphicsWidget = widget;
}

QGraphicsWidget *ToolTipContent::graphicsWidget() const
{
    return d->graphicsWidget.data();
}

} // namespace Plasma


