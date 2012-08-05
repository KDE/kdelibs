/*
 *   Copyright 2007 by Dan Meltzer <hydrogen@notyetimplemented.com>
 *   Copyright (C) 2008 by Alexis Ménard <darktears31@gmail.com>
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

#include "tooltip_p.h"
#include "windowpreview_p.h"

#include <QAbstractTextDocumentLayout>
#include <QBitmap>
#include <QGridLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QTextDocument>
#include <QPropertyAnimation>
#include <QTextBlock>

#include <kdebug.h>
#include <kglobalsettings.h>
#include <kwindoweffects.h>

#include <plasma/plasma.h>
#include <plasma/paintutils.h>
#include <plasma/theme.h>
#include <plasma/framesvg.h>

namespace Plasma {

class TipTextWidget : public QWidget
{
public:
    TipTextWidget(ToolTip *parent)
        : QWidget(parent),
          m_toolTip(parent),
          m_document(new QTextDocument(this))
    {
        QTextOption option = m_document->defaultTextOption();
        option.setWrapMode(QTextOption::WordWrap);
        m_document->setDefaultTextOption(option);
    }

    void setStyleSheet(const QString &css)
    {
        m_document->setDefaultStyleSheet(css);
    }

    void setContent(const ToolTipContent &data)
    {
        QString html;
        QString mainText = data.mainText();
        if (!mainText.isEmpty()) {
            if (mainText.size() < 50) {
                // don't let short texts wrap on us!
                mainText = mainText.replace(" ", "&nbsp;");
            }
            html.append("<div align=\"center\"><b>" + mainText + "</b></div>");
        }
        html.append(data.subText());

        m_anchor.clear();
        m_document->clear();
        data.registerResources(m_document);
        if (!html.isEmpty()) {
            m_document->setHtml("<p>" + html + "</p>");
        } else {
            m_document->clear();
        }
        m_document->adjustSize();

        update();
    }

    QSize minimumSizeHint() const
    {
        const int margin = 6;
        return m_document->size().toSize() + QSize(margin, margin)*2;
    }

    QSize maximumSizeHint() const
    {
        return minimumSizeHint();
    }

    void paintEvent(QPaintEvent *event)
    {
        QPainter p(this);
        m_document->drawContents(&p, event->rect());
    }

    void mousePressEvent(QMouseEvent *event)
    {
        QAbstractTextDocumentLayout *layout = m_document->documentLayout();
        if (layout) {
            m_anchor = layout->anchorAt(event->pos());
        }
    }

    void mouseReleaseEvent(QMouseEvent *event)
    {
        QAbstractTextDocumentLayout *layout = m_document->documentLayout();
        if (layout) {
            QString anchor = layout->anchorAt(event->pos());
            if (anchor == m_anchor) {
                m_toolTip->linkActivated(m_anchor, event);
            }

            m_anchor.clear();
        }
    }


private:
    ToolTip *m_toolTip;
    QTextDocument *m_document;
    QString m_anchor;
};

class ToolTipPrivate
{
    public:
        ToolTipPrivate()
        : text(0),
          imageLabel(0),
          preview(0),
          direction(Plasma::Up),
          autohide(true)
    { }

    TipTextWidget *text;
    QLabel *imageLabel;
    WindowPreview *preview;
    FrameSvg *background;
    QWeakPointer<QObject> source;
    QPropertyAnimation *animation;
    Plasma::Direction direction;
    bool autohide;
};

ToolTip::ToolTip(QWidget *parent)
    : QWidget(parent),
      d(new ToolTipPrivate())
{
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::ToolTip);
    d->preview = new WindowPreview(this);
    d->text = new TipTextWidget(this);
    d->imageLabel = new QLabel(this);

    d->imageLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    d->animation = new QPropertyAnimation(this, "pos", this);
    d->animation->setEasingCurve(QEasingCurve::InOutQuad);
    d->animation->setDuration(250);

    d->background = new FrameSvg(this);
    d->background->setImagePath("widgets/tooltip");
    d->background->setEnabledBorders(FrameSvg::AllBorders);
    updateTheme();
    connect(d->background, SIGNAL(repaintNeeded()), this, SLOT(updateTheme()));
    connect(d->preview, SIGNAL(windowPreviewClicked(WId,Qt::MouseButtons,Qt::KeyboardModifiers,QPoint)),
            this, SIGNAL(activateWindowByWId(WId,Qt::MouseButtons,Qt::KeyboardModifiers,QPoint)));

    QHBoxLayout *previewHBoxLayout = new QHBoxLayout;
    previewHBoxLayout->addWidget(d->preview);

    QHBoxLayout *iconTextHBoxLayout = new QHBoxLayout;
    iconTextHBoxLayout->addWidget(d->imageLabel);
    iconTextHBoxLayout->setAlignment(d->imageLabel, Qt::AlignTop | Qt::AlignHCenter);
    iconTextHBoxLayout->addWidget(d->text);
    iconTextHBoxLayout->setAlignment(d->text, Qt::AlignLeft | Qt::AlignVCenter);
    iconTextHBoxLayout->setStretchFactor(d->text, 1);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(previewHBoxLayout);
    mainLayout->addLayout(iconTextHBoxLayout);

    setLayout(mainLayout);
}

ToolTip::~ToolTip()
{
    delete d;
}

void ToolTip::showEvent(QShowEvent *e)
{
    checkSize();
    QWidget::showEvent(e);
    d->preview->setInfo();
    KWindowEffects::overrideShadow(winId(), true);
}

void ToolTip::hideEvent(QHideEvent *e)
{
    QWidget::hideEvent(e);
    d->animation->stop();

    QObject *source = d->source.data();
    if (source && source->metaObject()->indexOfMethod("toolTipHidden()") != -1) {
        QMetaObject::invokeMethod(source, "toolTipHidden");
    }

    KWindowEffects::highlightWindows(winId(), QList<WId>());
}

void ToolTip::mouseReleaseEvent(QMouseEvent *event)
{
    if (rect().contains(event->pos()) &&
        (!d->preview || !d->preview->geometry().contains(event->pos()))) {
        hide();
    }
}

void ToolTip::enterEvent(QEvent *)
{
    emit hovered(true);
}

void ToolTip::leaveEvent(QEvent *)
{
    emit hovered(false);
}

void ToolTip::checkSize()
{
    //FIXME: layout bugs even on qlayouts? oh, please, no.
    d->text->setMinimumSize(0, 0);
    d->text->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    d->text->setMinimumSize(d->text->minimumSizeHint());
    d->text->setMaximumSize(d->text->maximumSizeHint());

    adjustSize();
}

void ToolTip::adjustPosition(const QSize &previous, const QSize &current)
{
    if (previous != current) {
        //offsets to stop tooltips from jumping when they resize
        int deltaX = 0;
        int deltaY = 0;
        if (d->direction == Plasma::Up) {
        /*
#ifndef NDEBUG
        kDebug() << "resizing from" << current << "to" << hint
#endif
                 << "and moving from" << pos() << "to"
                 << x() << y() + (current.height() - hint.height())
                 << current.height() - hint.height();
                 */
            deltaY = previous.height() - current.height();
        } else if (d->direction == Plasma::Left) {
        /*
#ifndef NDEBUG
        kDebug() << "vertical resizing from" << current << "to" << hint
#endif
                 << "and moving from" << pos() << "to"
                 << x() + (current.width() - hint.width()) << y()
                 << current.width() - hint.width(); */
            deltaX = previous.width() - current.width();
        }

        // resize then move if we're getting smaller, vice versa when getting bigger
        // this prevents overlap with the item in the smaller case, and a repaint of
        // the tipped item when getting bigger

        move(x() + deltaX, y() + deltaY);
    }
}

void ToolTip::setContent(QObject *tipper, const ToolTipContent &data)
{
    //reset our size
    d->text->setContent(data);
    if (data.image().isNull()) {
        d->imageLabel->hide();
    } else {
        d->imageLabel->show();
        d->imageLabel->setPixmap(data.image());
    }

    if (data.highlightWindows() && !data.windowsToPreview().isEmpty()) {
        KWindowEffects::highlightWindows(winId(), QList<WId>() << winId() << data.windowsToPreview());
    }

    d->preview->setWindowIds(data.windowsToPreview());
    d->preview->setHighlightWindows(data.highlightWindows());

    d->autohide = data.autohide();
    d->source = tipper;

    if (isVisible()) {
        d->preview->setInfo();
        //kDebug() << "about to check size";
        checkSize();
    }
}

void ToolTip::prepareShowing()
{
    // show/hide the preview area
    d->preview->setVisible(!d->preview->isEmpty());

    layout()->activate();
    d->preview->setInfo();
    //kDebug() << "about to check size";
    checkSize();
}

void ToolTip::moveTo(const QPoint &to)
{
    if (!isVisible() ||
        !(KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects)) {
        move(to);
        return;
    }

    d->animation->stop();
    d->animation->setEndValue(to);
    d->animation->start();
}

void ToolTip::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    d->background->resizeFrame(size());
    if (Plasma::Theme::defaultTheme()->windowTranslucencyEnabled()) {
        KWindowEffects::enableBlurBehind(winId(), true, d->background->mask());
        clearMask();
    } else {
        setMask(d->background->mask());
    }
    d->preview->setInfo();

    if (isVisible()) {
        adjustPosition(e->oldSize(), e->size());
    }
}

void ToolTip::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setClipRect(e->rect());
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(e->rect(), Qt::transparent);

    d->background->paintFrame(&painter);
}

bool ToolTip::autohide() const
{
    return d->autohide;
}

void ToolTip::setDirection(Plasma::Direction direction)
{
    d->direction = direction;
}

void ToolTip::linkActivated(const QString &anchor, QMouseEvent *event)
{
    emit linkActivated(anchor, event->buttons(), event->modifiers(), event->globalPos());
}

void ToolTip::updateTheme()
{
    const int topHeight = d->background->marginSize(Plasma::TopMargin);
    const int leftWidth = d->background->marginSize(Plasma::LeftMargin);
    const int rightWidth = d->background->marginSize(Plasma::RightMargin);
    const int bottomHeight = d->background->marginSize(Plasma::BottomMargin);
    setContentsMargins(leftWidth, topHeight, rightWidth, bottomHeight);

    // Make the tooltip use Plasma's colorscheme
    QColor textColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    QPalette plasmaPalette = QPalette();
    plasmaPalette.setColor(QPalette::Window,
                           Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));
    plasmaPalette.setColor(QPalette::WindowText, textColor);
    setAutoFillBackground(true);
    setPalette(plasmaPalette);
    d->text->setStyleSheet(QString("p { color: %1; }").arg(textColor.name()));
    update();
}

} // namespace Plasma

#include "moc_tooltip_p.cpp"
