/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "radiobutton.h"

#include <QRadioButton>
#include <QPainter>
#include <QDir>

#include <kmimetype.h>

#include "theme.h"
#include "svg.h"

namespace Plasma
{

class RadioButtonPrivate
{
public:
    RadioButtonPrivate(RadioButton *w)
        : q(w),
          svg(0)
    {
    }

    ~RadioButtonPrivate()
    {
        delete svg;
    }

    void setPixmap()
    {
        if (imagePath.isEmpty()) {
            delete svg;
            svg = 0;
            return;
        }

        KMimeType::Ptr mime = KMimeType::findByPath(absImagePath);
        QPixmap pm(q->size().toSize());

        if (mime->is("image/svg+xml") || mime->is("image/svg+xml-compressed")) {
            if (!svg || svg->imagePath() != imagePath) {
                delete svg;
                svg = new Svg();
                svg->setImagePath(imagePath);
                QObject::connect(svg, SIGNAL(repaintNeeded()), q, SLOT(setPixmap()));
            }

            QPainter p(&pm);
            svg->paint(&p, pm.rect());
        } else {
            delete svg;
            svg = 0;
            pm = QPixmap(absImagePath);
        }

        static_cast<QRadioButton*>(q->widget())->setIcon(QIcon(pm));
    }

    RadioButton *q;
    QString imagePath;
    QString absImagePath;
    Svg *svg;
};

RadioButton::RadioButton(QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent),
      d(new RadioButtonPrivate(this))
{
    QRadioButton *native = new QRadioButton;
    connect(native, SIGNAL(toggled(bool)), this, SIGNAL(toggled(bool)));
    setWidget(native);
    native->setAttribute(Qt::WA_NoSystemBackground);
}

RadioButton::~RadioButton()
{
    delete d;
}

void RadioButton::setText(const QString &text)
{
    static_cast<QRadioButton*>(widget())->setText(text);
}

QString RadioButton::text() const
{
    return static_cast<QRadioButton*>(widget())->text();
}

void RadioButton::setImage(const QString &path)
{
    if (d->imagePath == path) {
        return;
    }

    delete d->svg;
    d->svg = 0;
    d->imagePath = path;

    bool absolutePath = !path.isEmpty() &&
                        #ifdef Q_WS_WIN
                            !QDir::isRelativePath(path)
                        #else
                            (path[0] == '/' || path.startsWith(":/"))
                        #endif
        ;

    if (absolutePath) {
        d->absImagePath = path;
    } else {
        //TODO: package support
        d->absImagePath = Theme::defaultTheme()->imagePath(path);
    }

    d->setPixmap();
}

QString RadioButton::image() const
{
    return d->imagePath;
}

void RadioButton::setStyleSheet(const QString &stylesheet)
{
    widget()->setStyleSheet(stylesheet);
}

QString RadioButton::styleSheet()
{
    return widget()->styleSheet();
}

QRadioButton *RadioButton::nativeWidget() const
{
    return static_cast<QRadioButton*>(widget());
}

void RadioButton::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    d->setPixmap();
    QGraphicsProxyWidget::resizeEvent(event);
}

void RadioButton::setChecked(bool checked)
{
    static_cast<QRadioButton*>(widget())->setChecked(checked);
}

bool RadioButton::isChecked() const
{
    return static_cast<QRadioButton*>(widget())->isChecked();
}

} // namespace Plasma

#include <radiobutton.moc>

