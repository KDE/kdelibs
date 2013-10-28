/**
 * KStyle for KDE4
 * Copyright (C) 2004-2005 Maksim Orlovich <maksim@kde.org>
 * Copyright (C) 2005,2006 Sandro Giessl <giessl@kde.org>
 *
 * Based in part on the following software:
 *  KStyle for KDE3
 *      Copyright (C) 2001-2002 Karol Szwed <gallium@kde.org>
 *      Portions  (C) 1998-2000 TrollTech AS
 *  Keramik for KDE3,
 *      Copyright (C) 2002      Malte Starostik   <malte@kde.org>
 *                (C) 2002-2003 Maksim Orlovich  <maksim@kde.org>
 *      Portions  (C) 2001-2002 Karol Szwed     <gallium@kde.org>
 *                (C) 2001-2002 Fredrik Höglund <fredrik@kde.org>
 *                (C) 2000 Daniel M. Duley       <mosfet@kde.org>
 *                (C) 2000 Dirk Mueller         <mueller@kde.org>
 *                (C) 2001 Martijn Klingens    <klingens@kde.org>
 *                (C) 2003 Sandro Giessl      <sandro@giessl.com>
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 * Many thanks to Bradley T. Hughes for the 3 button scrollbar code.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kstyle.h"

#include <QtCore/qalgorithms.h>
#include <QtCore/QCache>
#include <QtCore/QEvent>
#include <QtCore/QVariant>
#include <QAbstractItemView>
#include <QApplication>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QIcon>
#include <QLabel>
#include <QKeyEvent>
#include <QPainter>
#include <QScrollBar>
#include <QStyleOption>
#include <QPushButton>
#include <QToolBar>

#include <kconfiggroup.h>
#include <QDebug>
#include <kiconloader.h>
#include <kcolorscheme.h>

//### FIXME: Who to credit these to?
static const qint32 u_arrow[]={-1,-3, 0,-3, -2,-2, 1,-2, -3,-1, 2,-1, -4,0, 3,0, -4,1, 3,1};
static const qint32 d_arrow[]={-4,-2, 3,-2, -4,-1, 3,-1, -3,0, 2,0, -2,1, 1,1, -1,2, 0,2};
static const qint32 l_arrow[]={-3,-1, -3,0, -2,-2, -2,1, -1,-3, -1,2, 0,-4, 0,3, 1,-4, 1,3};
static const qint32 r_arrow[]={-2,-4, -2,3, -1,-4, -1,3, 0,-3, 0,2, 1,-2, 1,1, 2,-1, 2,0};
#define QCOORDARRLEN(x) sizeof(x)/(sizeof(qint32)*2)


/**
 TODO: lots of missing widgets, SH_ settings, etc.

 Minor stuff:
    ProgressBar::Precision handling
*/


// ----------------------------------------------------------------------------


// For item view selections
struct SelectionTiles
{
    QPixmap left, center, right;
};


// ----------------------------------------------------------------------------

static const QStyle::StyleHint SH_KCustomStyleElement = (QStyle::StyleHint)0xff000001;
static const int X_KdeBase = 0xff000000;

class KStylePrivate
{
public:
    KStylePrivate();
    QCache<quint64, SelectionTiles> selectionCache;

    QHash<QString, int> styleElements;
    int hintCounter, controlCounter, subElementCounter;
};

KStylePrivate::KStylePrivate()
{
    selectionCache.setMaxCost(10);
    controlCounter = subElementCounter = X_KdeBase;
    hintCounter = X_KdeBase+1; //sic! X_KdeBase is covered by SH_KCustomStyleElement
}


// ----------------------------------------------------------------------------


KStyle::KStyle() : d(new KStylePrivate)
{
    //Set up some default metrics...
    setWidgetLayoutProp(WT_Generic, Generic::DefaultFrameWidth, 2);
    setWidgetLayoutProp(WT_Generic, Generic::DefaultLayoutSpacing, 6);
    setWidgetLayoutProp(WT_Generic, Generic::DefaultLayoutMargin, 9);

    setWidgetLayoutProp(WT_PushButton, PushButton::ContentsMargin, 5);
    setWidgetLayoutProp(WT_PushButton, PushButton::FocusMargin,    3);
    setWidgetLayoutProp(WT_PushButton, PushButton::PressedShiftHorizontal, 2);
    setWidgetLayoutProp(WT_PushButton, PushButton::PressedShiftVertical,   2);
    setWidgetLayoutProp(WT_PushButton, PushButton::MenuIndicatorSize,      8);
    setWidgetLayoutProp(WT_PushButton, PushButton::TextToIconSpace,        6);

    setWidgetLayoutProp(WT_Splitter, Splitter::Width, 6); //As KStyle in KDE3

    setWidgetLayoutProp(WT_CheckBox, CheckBox::Size, 16);
    setWidgetLayoutProp(WT_CheckBox, CheckBox::BoxTextSpace, 6);
    setWidgetLayoutProp(WT_CheckBox, CheckBox::NoLabelFocusMargin, 1);

    setWidgetLayoutProp(WT_RadioButton, RadioButton::Size, 16);
    setWidgetLayoutProp(WT_RadioButton, RadioButton::BoxTextSpace, 6);

    setWidgetLayoutProp(WT_DockWidget, DockWidget::TitleTextColor,
                        ColorMode(QPalette::HighlightedText));
    setWidgetLayoutProp(WT_DockWidget, DockWidget::TitleMargin, 2);
    setWidgetLayoutProp(WT_DockWidget, DockWidget::FrameWidth, 3);
    setWidgetLayoutProp(WT_DockWidget, DockWidget::SeparatorExtent, 6);

    setWidgetLayoutProp(WT_ProgressBar, ProgressBar::GrooveMargin,  2);
    setWidgetLayoutProp(WT_ProgressBar, ProgressBar::SideTextSpace, 3); //(Matches QCommonStyle)
    setWidgetLayoutProp(WT_ProgressBar, ProgressBar::MaxBusyIndicatorSize, 10000);
    setWidgetLayoutProp(WT_ProgressBar, ProgressBar::BusyIndicatorSize,    10);
    setWidgetLayoutProp(WT_ProgressBar, ProgressBar::Precision,            1);

    setWidgetLayoutProp(WT_MenuBar, MenuBar::ItemSpacing,   14);
    setWidgetLayoutProp(WT_MenuBar, MenuBar::Margin,        2);
    setWidgetLayoutProp(WT_MenuBar, MenuBar::Margin + Left,  4);
    setWidgetLayoutProp(WT_MenuBar, MenuBar::Margin + Right, 4);

    setWidgetLayoutProp(WT_MenuBarItem, MenuBarItem::Margin, 1);

    setWidgetLayoutProp(WT_Menu, Menu::FrameWidth, 1);
    setWidgetLayoutProp(WT_Menu, Menu::Margin,     3);
    setWidgetLayoutProp(WT_Menu, Menu::ScrollerHeight, 10);
    setWidgetLayoutProp(WT_Menu, Menu::TearOffHeight, 10);

    setWidgetLayoutProp(WT_MenuItem, MenuItem::CheckWidth, 12);
    setWidgetLayoutProp(WT_MenuItem, MenuItem::CheckSpace, 3);
    setWidgetLayoutProp(WT_MenuItem, MenuItem::IconWidth, 12);
    setWidgetLayoutProp(WT_MenuItem, MenuItem::IconSpace, 3);
    setWidgetLayoutProp(WT_MenuItem, MenuItem::ArrowWidth, 11);
    setWidgetLayoutProp(WT_MenuItem, MenuItem::ArrowSpace, 3);
    setWidgetLayoutProp(WT_MenuItem, MenuItem::Margin,     2);
    setWidgetLayoutProp(WT_MenuItem, MenuItem::SeparatorHeight, 0); //the margins give enough rooms
    setWidgetLayoutProp(WT_MenuItem, MenuItem::MinHeight,  16);
    setWidgetLayoutProp(WT_MenuItem, MenuItem::TextColor, ColorMode(QPalette::Text));
    setWidgetLayoutProp(WT_MenuItem, MenuItem::ActiveTextColor, ColorMode(QPalette::HighlightedText));
    setWidgetLayoutProp(WT_MenuItem, MenuItem::DisabledTextColor,       ColorMode(QPalette::Text));
    setWidgetLayoutProp(WT_MenuItem, MenuItem::ActiveDisabledTextColor, ColorMode(QPalette::Text));
    setWidgetLayoutProp(WT_MenuItem, MenuItem::AccelSpace, 16);

    //KDE default is single top button, double bottom one
    setWidgetLayoutProp(WT_ScrollBar, ScrollBar::DoubleTopButton, 0);
    setWidgetLayoutProp(WT_ScrollBar, ScrollBar::DoubleBotButton, 1);
    setWidgetLayoutProp(WT_ScrollBar, ScrollBar::SingleButtonHeight, 16);
    setWidgetLayoutProp(WT_ScrollBar, ScrollBar::DoubleButtonHeight, 32);
    setWidgetLayoutProp(WT_ScrollBar, ScrollBar::BarWidth, 16);
    setWidgetLayoutProp(WT_ScrollBar, ScrollBar::ArrowColor,
                            ColorMode(ColorMode::BWAutoContrastMode, QPalette::Button));
    setWidgetLayoutProp(WT_ScrollBar, ScrollBar::ActiveArrowColor,
                            ColorMode(ColorMode::BWAutoContrastMode, QPalette::ButtonText));

    setWidgetLayoutProp(WT_TabBar, TabBar::TabContentsMargin, 6);
    setWidgetLayoutProp(WT_TabBar, TabBar::TabFocusMargin, 3);
    setWidgetLayoutProp(WT_TabBar, TabBar::TabOverlap, 0);
    setWidgetLayoutProp(WT_TabBar, TabBar::BaseHeight, 2);
    setWidgetLayoutProp(WT_TabBar, TabBar::BaseOverlap, 2);
    setWidgetLayoutProp(WT_TabBar, TabBar::ScrollButtonWidth, 10);
    setWidgetLayoutProp(WT_TabBar, TabBar::TabTextToIconSpace, 6);

    setWidgetLayoutProp(WT_TabWidget, TabWidget::ContentsMargin, 2);

    setWidgetLayoutProp(WT_Tree, Tree::MaxExpanderSize, 9);

    setWidgetLayoutProp(WT_Slider, Slider::HandleThickness, 20);
    setWidgetLayoutProp(WT_Slider, Slider::HandleLength, 16);

    setWidgetLayoutProp(WT_SpinBox, SpinBox::FrameWidth, 1);
    setWidgetLayoutProp(WT_SpinBox, SpinBox::ButtonWidth, 16);
    setWidgetLayoutProp(WT_SpinBox, SpinBox::ButtonSpacing, 1);
    setWidgetLayoutProp(WT_SpinBox, SpinBox::ButtonMargin+Right, 1);
    setWidgetLayoutProp(WT_SpinBox, SpinBox::ButtonMargin+Top, 1);
    setWidgetLayoutProp(WT_SpinBox, SpinBox::ButtonMargin+Bot, 1);

    setWidgetLayoutProp(WT_ComboBox, ComboBox::FrameWidth, 1);
    setWidgetLayoutProp(WT_ComboBox, ComboBox::ButtonWidth, 16);
    setWidgetLayoutProp(WT_ComboBox, ComboBox::ButtonMargin+Right, 1);
    setWidgetLayoutProp(WT_ComboBox, ComboBox::ButtonMargin+Top, 1);
    setWidgetLayoutProp(WT_ComboBox, ComboBox::ButtonMargin+Bot, 1);
    setWidgetLayoutProp(WT_ComboBox, ComboBox::FocusMargin, 1);

    setWidgetLayoutProp(WT_Header, Header::ContentsMargin, 3);
    setWidgetLayoutProp(WT_Header, Header::TextToIconSpace, 3);
    setWidgetLayoutProp(WT_Header, Header::MarkSize, 9);

    setWidgetLayoutProp(WT_GroupBox, GroupBox::FrameWidth, 2);
    setWidgetLayoutProp(WT_GroupBox, GroupBox::TextAlignTop, false);
    setWidgetLayoutProp(WT_GroupBox, GroupBox::TitleTextColor, ColorMode(QPalette::Text));

    setWidgetLayoutProp(WT_ToolBar, ToolBar::HandleExtent, 6);
    setWidgetLayoutProp(WT_ToolBar, ToolBar::SeparatorExtent, 6);
    setWidgetLayoutProp(WT_ToolBar, ToolBar::ExtensionExtent, 10);
    setWidgetLayoutProp(WT_ToolBar, ToolBar::FrameWidth, 2);
    setWidgetLayoutProp(WT_ToolBar, ToolBar::ItemSpacing, 3);
    setWidgetLayoutProp(WT_ToolBar, ToolBar::ItemMargin, 1);

    setWidgetLayoutProp(WT_ToolButton, ToolButton::ContentsMargin, 5);
    setWidgetLayoutProp(WT_ToolButton, ToolButton::FocusMargin,    3);
    setWidgetLayoutProp(WT_ToolButton, ToolButton::MenuIndicatorSize, 11);

    setWidgetLayoutProp(WT_ToolBoxTab, ToolBoxTab::Margin, 0);

    setWidgetLayoutProp(WT_Window, Window::TitleTextColor, ColorMode(QPalette::HighlightedText));
    setWidgetLayoutProp(WT_Window, Window::TitleHeight, 20);
    setWidgetLayoutProp(WT_Window, Window::TitleMargin, 2);
    setWidgetLayoutProp(WT_Window, Window::NoTitleFrame, 0);
    setWidgetLayoutProp(WT_Window, Window::ButtonWidth, 16);
    setWidgetLayoutProp(WT_Window, Window::ButtonSpace, 2);
    setWidgetLayoutProp(WT_Window, Window::ButtonToTextSpace, 3);
}

KStyle::~KStyle()
{
    // this is just for stupid msvc compiler to force the creation of
    // DoubleButtonOption::defaultOption() inside kstyle lib
    // hope the optimizer won't throw it away
    const DoubleButtonOption* bOpt = extractOption<const DoubleButtonOption*>(NULL);
    Q_UNUSED(bOpt)
#ifdef __GNUC__
#warning "mem leak: need to delete bOpt"
#endif
    delete d;
}

/*
    Custom Style Element runtime extension:
    We reserve one StyleHint to let the effective style inform widgets whether it supports certain
    string based style elements.
    As this could lead to number conflicts (i.e. an app utilizing one of the hints itself for other
    purposes) there're various safety mechanisms to rule out such interference.

    1) It's most unlikely that a widget in some 3rd party app will accidentally call a general
    QStyle/KStyle styleHint() or draw*() and (unconditionally) expect a valid return, however:
    a. The StyleHint is not directly above Qt's custom base, assuming most 3rd party apps would
    - in case - make use of such
    b. In order to be accepted, the StyleHint query must pass a widget with a perfectly matching
    name, containing the typical element prefix ("CE_", etc.) and being supported by the current style
    c. Instead using Qt's fragile qstyleoption_cast on the QStyleOption provided to the StyleHint
    query, try to dump out a string and hope for the best, we now manipulate the widgets objectName().
    Plain Qt dependent widgets can do that themselves and if a widget uses KStyle's convenience access
    functions, it won't notice this at all

    2) The key problem is that a common KDE widget will run into an apps custom style which will then
    falsely respond to the styleHint() call with an invalid value.
    To prevent this, supporting styles *must* set a Q_CLASSINFO "X-KDE-CustomElements".

    3) If any of the above traps snaps, the returned id is 0 - the QStyle default, indicating
    that this element is not supported by the current style.

    Obviously, this contains the "diminished clean" action to (temporarily) manipulate the
    objectName() of a const QWidget* - but this happens completely inside KStyle or the widget, if
    it does not make use of KStyles static convenience functions.
    My biggest worry here would be, that in a multithreaded environment a thread (usually not being
    owner of the widget) does something crucially relying on the widgets name property...
    This however would also have to happen during the widget construction or stylechanges, when
    the functions in doubt will typically be called.
    So this is imho unlikely causing any trouble, ever.
*/

/*
    The functions called by the real style implementation to add support for a certain element.
    Checks for well-formed string (containing the element prefix) and returns 0 otherwise.
    Checks whether the element is already supported or inserts it otherwise; Returns the proper id
    NOTICE: We could check for "X-KDE-CustomElements", but this would bloat style start up times
    (if they e.g. register 100 elements or so)
*/


static inline int newStyleElement(const QString &element, const char *check, int &counter, QHash<QString, int> *elements)
{
    if (!element.contains(check))
        return 0;
    int id = elements->value(element, 0);
    if (!id) {
        ++counter;
        id = counter;
        elements->insert(element, id);
    }
    return id;
}

QStyle::StyleHint KStyle::newStyleHint(const QString &element)
{
    return (StyleHint)newStyleElement(element, "SH_", d->hintCounter, &d->styleElements);
}

QStyle::ControlElement KStyle::newControlElement(const QString &element)
{
    return (ControlElement)newStyleElement(element, "CE_", d->controlCounter, &d->styleElements);
}

KStyle::SubElement KStyle::newSubElement(const QString &element)
{
    return (SubElement)newStyleElement(element, "SE_", d->subElementCounter, &d->styleElements);
}

/*
    The functions called by widgets that request custom element support, passed to the effective style.
    Collected in a static inline function due to similarity.
*/

static inline int customStyleElement(QStyle::StyleHint type, const QString &element, QWidget *widget)
{
    if (!widget || widget->style()->metaObject()->indexOfClassInfo("X-KDE-CustomElements") < 0)
        return 0;

    const QString originalName = widget->objectName();
    widget->setObjectName(element);
    const int id = widget->style()->styleHint(type, 0, widget);
    widget->setObjectName(originalName);
    return id;
}

QStyle::StyleHint KStyle::customStyleHint(const QString &element, const QWidget *widget)
{
    return (StyleHint) customStyleElement(SH_KCustomStyleElement, element, const_cast<QWidget*>(widget));
}

QStyle::ControlElement KStyle::customControlElement(const QString &element, const QWidget *widget)
{
    return (ControlElement) customStyleElement(SH_KCustomStyleElement, element, const_cast<QWidget*>(widget));
}

QStyle::SubElement KStyle::customSubElement(const QString &element, const QWidget *widget)
{
    return (SubElement) customStyleElement(SH_KCustomStyleElement, element, const_cast<QWidget*>(widget));
}

void KStyle::polish(QWidget *w)
{
    if (qobject_cast<QLabel*>(w) ) {
        w->installEventFilter(this);
    }

    // Enable hover effects in all itemviews
    if (QAbstractItemView *itemView = qobject_cast<QAbstractItemView*>(w) ) {
        itemView->viewport()->setAttribute(Qt::WA_Hover);
    }

    if (QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(w)) {
        QPushButton* button = box->button(QDialogButtonBox::Ok);
        if (button && button->shortcut().isEmpty()) {
            button->setShortcut(Qt::CTRL | Qt::Key_Return);
        }
    }

    QCommonStyle::polish(w);
}
void KStyle::unpolish(QWidget *w)
{
    if (qobject_cast<QLabel*>(w) ) {
        w->removeEventFilter(this);
    }

    QCommonStyle::unpolish(w);
}
void KStyle::polish(QApplication *a)
{
    QCommonStyle::polish(a);
}
void KStyle::unpolish(QApplication *a)
{
    QCommonStyle::unpolish(a);
}
void KStyle::polish(QPalette &pal)
{
    QCommonStyle::polish(pal);
}
QPalette KStyle::standardPalette() const
{
    return KColorScheme::createApplicationPalette(KSharedConfig::openConfig());
}

QIcon KStyle::standardIcon(StandardPixmap standardIcon, const QStyleOption */*option*/,
                                         const QWidget */*widget*/) const
{
    switch (standardIcon) {
        case QStyle::SP_DesktopIcon:
            return QIcon::fromTheme("user-desktop");
        case QStyle::SP_TrashIcon:
            return QIcon::fromTheme("user-trash");
        case QStyle::SP_ComputerIcon:
            return QIcon::fromTheme("computer");
        case QStyle::SP_DriveFDIcon:
            return QIcon::fromTheme("media-floppy");
        case QStyle::SP_DriveHDIcon:
            return QIcon::fromTheme("drive-harddisk");
        case QStyle::SP_DriveCDIcon:
        case QStyle::SP_DriveDVDIcon:
            return QIcon::fromTheme("drive-optical");
        case QStyle::SP_DriveNetIcon:
            return QIcon::fromTheme("folder-remote");
        case QStyle::SP_DirHomeIcon:
            return QIcon::fromTheme("user-home");
        case QStyle::SP_DirOpenIcon:
            return QIcon::fromTheme("document-open-folder");
        case QStyle::SP_DirClosedIcon:
            return QIcon::fromTheme("folder");
        case QStyle::SP_DirIcon:
            return QIcon::fromTheme("folder");
        case QStyle::SP_DirLinkIcon:
            return QIcon::fromTheme("folder"); //TODO: generate (!?) folder with link emblem
        case QStyle::SP_FileIcon:
            return QIcon::fromTheme("text-plain"); //TODO: look for a better icon
        case QStyle::SP_FileLinkIcon:
            return QIcon::fromTheme("text-plain"); //TODO: generate (!?) file with link emblem
        case QStyle::SP_FileDialogStart:
            return QIcon::fromTheme("media-playback-start"); //TODO: find correct icon
        case QStyle::SP_FileDialogEnd:
            return QIcon::fromTheme("media-playback-stop"); //TODO: find correct icon
        case QStyle::SP_FileDialogToParent:
            return QIcon::fromTheme("go-up");
        case QStyle::SP_FileDialogNewFolder:
            return QIcon::fromTheme("folder-new");
        case QStyle::SP_FileDialogDetailedView:
            return QIcon::fromTheme("view-list-details");
        case QStyle::SP_FileDialogInfoView:
            return QIcon::fromTheme("document-properties");
        case QStyle::SP_FileDialogContentsView:
            return QIcon::fromTheme("view-list-icons");
        case QStyle::SP_FileDialogListView:
            return QIcon::fromTheme("view-list-text");
        case QStyle::SP_FileDialogBack:
            return QIcon::fromTheme("go-previous");
        case QStyle::SP_MessageBoxInformation:
            return QIcon::fromTheme("dialog-information");
        case QStyle::SP_MessageBoxWarning:
            return QIcon::fromTheme("dialog-warning");
        case QStyle::SP_MessageBoxCritical:
            return QIcon::fromTheme("dialog-error");
        case QStyle::SP_MessageBoxQuestion:
            return QIcon::fromTheme("dialog-information");
        case QStyle::SP_DialogOkButton:
            return QIcon::fromTheme("dialog-ok");
        case QStyle::SP_DialogCancelButton:
            return QIcon::fromTheme("dialog-cancel");
        case QStyle::SP_DialogHelpButton:
            return QIcon::fromTheme("help-contents");
        case QStyle::SP_DialogOpenButton:
            return QIcon::fromTheme("document-open");
        case QStyle::SP_DialogSaveButton:
            return QIcon::fromTheme("document-save");
        case QStyle::SP_DialogCloseButton:
            return QIcon::fromTheme("dialog-close");
        case QStyle::SP_DialogApplyButton:
            return QIcon::fromTheme("dialog-ok-apply");
        case QStyle::SP_DialogResetButton:
            return QIcon::fromTheme("document-revert");
        case QStyle::SP_DialogDiscardButton:
            return QIcon::fromTheme("dialog-cancel");
        case QStyle::SP_DialogYesButton:
            return QIcon::fromTheme("dialog-ok-apply");
        case QStyle::SP_DialogNoButton:
            return QIcon::fromTheme("dialog-cancel");
        case QStyle::SP_ArrowUp:
            return QIcon::fromTheme("go-up");
        case QStyle::SP_ArrowDown:
            return QIcon::fromTheme("go-down");
        case QStyle::SP_ArrowLeft:
            return QIcon::fromTheme("go-previous-view");
        case QStyle::SP_ArrowRight:
            return QIcon::fromTheme("go-next-view");
        case QStyle::SP_ArrowBack:
            return QIcon::fromTheme("go-previous");
        case QStyle::SP_ArrowForward:
            return QIcon::fromTheme("go-next");
        case QStyle::SP_BrowserReload:
            return QIcon::fromTheme("view-refresh");
        case QStyle::SP_BrowserStop:
            return QIcon::fromTheme("process-stop");
        case QStyle::SP_MediaPlay:
            return QIcon::fromTheme("media-playback-start");
        case QStyle::SP_MediaStop:
            return QIcon::fromTheme("media-playback-stop");
        case QStyle::SP_MediaPause:
            return QIcon::fromTheme("media-playback-pause");
        case QStyle::SP_MediaSkipForward:
            return QIcon::fromTheme("media-skip-forward");
        case QStyle::SP_MediaSkipBackward:
            return QIcon::fromTheme("media-skip-backward");
        case QStyle::SP_MediaSeekForward:
            return QIcon::fromTheme("media-seek-forward");
        case QStyle::SP_MediaSeekBackward:
            return QIcon::fromTheme("media-seek-backward");
        case QStyle::SP_MediaVolume:
            return QIcon::fromTheme("audio-volume-medium");
        case QStyle::SP_MediaVolumeMuted:
            return QIcon::fromTheme("audio-volume-muted");

        default:
            return QIcon();
    }
}

void KStyle::drawInsideRect(QPainter* p, const QRect& r) const
{
    p->drawRect(r.x(), r.y(), r.width() - 1, r.height() - 1);
}

QRect KStyle::centerRect(const QRect &in, int w, int h) const
{
    return QRect(in.x() + (in.width() - w)/2, in.y() + (in.height() - h)/2, w, h);
}

QRect KStyle::centerRect(const QRect &in, const QSize &size) const
{
    return centerRect(in, size.width(), size.height());
}

void KStyle::setWidgetLayoutProp(WidgetType widget, int metric, int value)
{
    if (metrics.size() <= widget)
        metrics.resize(widget + 1);

    QVector<int>& widgetMetrics = metrics[widget];
    if (widgetMetrics.size() <= metric)
        widgetMetrics.resize(metric + 1);

    widgetMetrics[metric] = value;
}

int KStyle::widgetLayoutProp(WidgetType widget, int metric,
                             const QStyleOption* opt,
                             const QWidget* w ) const
{
    Q_UNUSED(opt)
    Q_UNUSED(w)

    if (metrics.size() <= widget)
        return 0;

    const QVector<int>& widgetMetrics = metrics[widget];
    if (widgetMetrics.size() <= metric)
        return 0;

    return widgetMetrics[metric];
}

int KStyle::styleHint (StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* returnData) const
{
    switch (hint)
    {
        case SH_ComboBox_ListMouseTracking:
            return true;

        case SH_MenuBar_MouseTracking:
        case SH_Menu_MouseTracking:
            return true;

        case SH_Menu_SubMenuPopupDelay:
            return 96; // Motif-like delay...

        case SH_TitleBar_NoBorder:
            return widgetLayoutProp(WT_Window, Window::NoTitleFrame, option, widget);

        case SH_GroupBox_TextLabelVerticalAlignment:
            if (widgetLayoutProp(WT_GroupBox, GroupBox::TextAlignTop, option, widget) )
                return Qt::AlignTop;
            else
                return Qt::AlignVCenter;

        case SH_GroupBox_TextLabelColor:
        {
            ColorMode cm( widgetLayoutProp(WT_GroupBox, GroupBox::TitleTextColor,
                          option, widget) );
            return cm.color(option ? option->palette : qApp->palette()).rgba();
        }

        case SH_DialogButtonLayout:
            return QDialogButtonBox::KdeLayout;

        case SH_ScrollBar_MiddleClickAbsolutePosition:
            return true;

        // Don't draw the branch as selected in tree views
        case SH_ItemView_ShowDecorationSelected:
            return false;

        case SH_ItemView_ActivateItemOnSingleClick:
            return KSharedConfig::openConfig()->group("KDE").readEntry("SingleClick", true );
        case SH_KCustomStyleElement:
            if (!widget)
                return 0;
            return d->styleElements.value(widget->objectName(), 0);

        // per HIG, align the contents in a form layout to the left
        case SH_FormLayoutFormAlignment:
            return Qt::AlignLeft | Qt::AlignTop;

        // per HIG, align the labels in a form layout to the right
        case SH_FormLayoutLabelAlignment:
            return Qt::AlignRight;

        case SH_FormLayoutFieldGrowthPolicy:
            return QFormLayout::ExpandingFieldsGrow;

        case SH_FormLayoutWrapPolicy:
            return QFormLayout::DontWrapRows;

        case SH_MessageBox_TextInteractionFlags:
            return true;

        case SH_DialogButtonBox_ButtonsHaveIcons:
        {
            // was KGlobalSettings::showIconsOnPushButtons() :
            KConfigGroup g(KSharedConfig::openConfig(), "KDE");
            return g.readEntry("ShowIconsOnPushButtons", true);
        }
        case SH_ItemView_ArrowKeysNavigateIntoChildren:
            return true;
        case SH_Widget_Animate:
        {
            KConfigGroup g(KSharedConfig::openConfig(), "KDE-Global GUI Settings");
            return g.readEntry("GraphicEffectsLevel", 0);
        }

        case SH_ToolButtonStyle:
        {
            KConfigGroup g(KSharedConfig::openConfig(), "KDE");

            bool useOthertoolbars = false;
            const QWidget *parent = widget->parentWidget();

            //If the widget parent is a QToolBar and the magic property is set
            if (parent && qobject_cast< const QToolBar* >(parent)) {
                if (parent->property("otherToolbar").isValid()) {
                    useOthertoolbars = true;
                }
            }

            QString buttonStyle;
            if (useOthertoolbars) {
                buttonStyle = g.readEntry("ToolButtonStyleOtherToolbars", "NoText").toLower();
            } else {
                buttonStyle = g.readEntry("ToolButtonStyle", "TextBesideIcon").toLower();
            }

            return buttonStyle == QLatin1String("textbesideicon") ? Qt::ToolButtonTextBesideIcon
                             : buttonStyle == QLatin1String("icontextright") ? Qt::ToolButtonTextBesideIcon
                             : buttonStyle == QLatin1String("textundericon") ? Qt::ToolButtonTextUnderIcon
                             : buttonStyle == QLatin1String("icontextbottom") ? Qt::ToolButtonTextUnderIcon
                             : buttonStyle == QLatin1String("textonly") ? Qt::ToolButtonTextOnly
                             : Qt::ToolButtonIconOnly;
        }
        default:
            break;
    };

    return QCommonStyle::styleHint(hint, option, widget, returnData);
}

int KStyle::pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const
{
    switch (metric)
    {
        case PM_SmallIconSize:
        case PM_ButtonIconSize:
            return KIconLoader::global()->currentSize(KIconLoader::Small);
        case PM_ToolBarIconSize:
            return KIconLoader::global()->currentSize(KIconLoader::Toolbar);
        case PM_LargeIconSize:
            return KIconLoader::global()->currentSize(KIconLoader::Dialog);
        case PM_MessageBoxIconSize:
            // TODO return KIconLoader::global()->currentSize(KIconLoader::MessageBox);
            return KIconLoader::SizeHuge;

        case PM_DefaultFrameWidth:
            if (qstyleoption_cast<const QStyleOptionGroupBox *>(option) )
                return widgetLayoutProp(WT_GroupBox, GroupBox::FrameWidth, option, widget);
            else
                return widgetLayoutProp(WT_Generic, Generic::DefaultFrameWidth, option, widget);

        case PM_DefaultChildMargin:
        case PM_DefaultTopLevelMargin:
            return widgetLayoutProp(WT_Generic, Generic::DefaultLayoutMargin, option, widget);

        case PM_LayoutHorizontalSpacing:
        case PM_LayoutVerticalSpacing:
            // use layoutSpacingImplementation
            return -1;

        case PM_DefaultLayoutSpacing:
            return widgetLayoutProp(WT_Generic, Generic::DefaultLayoutSpacing, option, widget);

        case PM_LayoutLeftMargin:
        case PM_LayoutTopMargin:
        case PM_LayoutRightMargin:
        case PM_LayoutBottomMargin:
        {
            PixelMetric marginMetric;
            if ((option && (option->state & QStyle::State_Window))
                || (widget && widget->isWindow())) {
                marginMetric = PM_DefaultTopLevelMargin;
            } else {
                marginMetric = PM_DefaultChildMargin;
            }
            return pixelMetric(marginMetric, option, widget);
        }

        case PM_ButtonMargin:
            return 0; //Better not return anything here since we already
            //incorporated this into SE_PushButtonContents
        case PM_ButtonDefaultIndicator:
            // PushButton::DefaultIndicatorMargin is used throughout KStyle button
            // implementation code, so this probably is not necessary.
            // return it in case Apps rely on this metric, though.
            return widgetLayoutProp(WT_PushButton, PushButton::DefaultIndicatorMargin, option, widget);
        case PM_ButtonShiftHorizontal:
            return widgetLayoutProp(WT_PushButton, PushButton::PressedShiftHorizontal, option, widget);
        case PM_ButtonShiftVertical:
            return widgetLayoutProp(WT_PushButton, PushButton::PressedShiftVertical, option, widget);
        case PM_MenuButtonIndicator:
            if (qstyleoption_cast<const QStyleOptionToolButton*>(option))
                return widgetLayoutProp(WT_ToolButton, ToolButton::MenuIndicatorSize, option, widget);
            else
                return widgetLayoutProp(WT_PushButton, PushButton::MenuIndicatorSize, option, widget);

        case PM_SplitterWidth:
            return widgetLayoutProp(WT_Splitter, Splitter::Width, option, widget);

        case PM_IndicatorWidth:
        case PM_IndicatorHeight:
            return widgetLayoutProp(WT_CheckBox, CheckBox::Size, option, widget);

        case PM_ExclusiveIndicatorWidth:
        case PM_ExclusiveIndicatorHeight:
            return widgetLayoutProp(WT_RadioButton, RadioButton::Size, option, widget);

        case PM_DockWidgetFrameWidth:
            return widgetLayoutProp(WT_DockWidget, DockWidget::FrameWidth, option, widget);

        case PM_DockWidgetSeparatorExtent:
            return widgetLayoutProp(WT_DockWidget, DockWidget::SeparatorExtent, option, widget);

        // handle extent only used somewhere in Qt3support, don't care.
        // case PM_DockWidgetHandleExtent:

        case PM_DockWidgetTitleMargin:
            return widgetLayoutProp(WT_DockWidget, DockWidget::TitleMargin, option, widget);

        case PM_ProgressBarChunkWidth:
            return widgetLayoutProp(WT_ProgressBar, ProgressBar::Precision, option, widget);

        case PM_MenuBarPanelWidth:
            return 0; //Simplification: just one primitive is used and it includes the border

        case PM_MenuBarHMargin:
        {
            //Calculate how much extra space we need besides the frame size. We use the left margin
            //here, and adjust the total rect by the difference between it and the right margin
            int spaceL = widgetLayoutProp(WT_MenuBar, MenuBar::Margin, option, widget) + widgetLayoutProp(WT_MenuBar, MenuBar::Margin + Left, option, widget);

            return spaceL;
        }

        case PM_MenuBarVMargin:
        {
            //As above, we return the top one, and fudge the total size for the bottom.
            int spaceT = widgetLayoutProp(WT_MenuBar, MenuBar::Margin, option, widget) + widgetLayoutProp(WT_MenuBar, MenuBar::Margin + Top, option, widget);
            return spaceT;
        }

        case PM_MenuBarItemSpacing:
            return widgetLayoutProp(WT_MenuBar, MenuBar::ItemSpacing, option, widget);

        case PM_MenuDesktopFrameWidth:
            return 0; //### CHECKME

        case PM_MenuPanelWidth:
            return widgetLayoutProp(WT_Menu, Menu::FrameWidth, option, widget);

            /* ### seems to trigger Qt bug. So we loose the margins for now
        case PM_MenuHMargin:
        {
            //Calculate how much extra space we need besides the frame size. We use the left margin
            //here, and adjust the total rect by the difference between it and the right margin
            int spaceL = widgetLayoutProp(WT_Menu, Menu::Margin, option, widget) + widgetLayoutProp(WT_Menu, Menu::Margin + Left, option, widget) -
                    widgetLayoutProp(WT_Menu, Menu::FrameWidth, option, widget);

            return spaceL;
        }

        case PM_MenuVMargin:
        {
            //As above, we return the top one, and fudge the total size for the bottom.
            int spaceT = widgetLayoutProp(WT_Menu, Menu::Margin, option, widget) + widgetLayoutProp(WT_Menu, Menu::Margin + Top, option, widget) -
                widgetLayoutProp(WT_Menu, Menu::FrameWidth, option, widget);
            return spaceT;
        }     */

        case PM_MenuScrollerHeight:
            return widgetLayoutProp(WT_Menu, Menu::ScrollerHeight, option, widget);

        case PM_MenuTearoffHeight:
            return widgetLayoutProp(WT_Menu, Menu::TearOffHeight, option, widget);

        case PM_TabBarTabHSpace:
        {
            const QStyleOptionTab* tabOpt = qstyleoption_cast<const QStyleOptionTab*>(option);
            if (tabOpt)
            {
                //Perhaps we can avoid the extra margin...
                if (tabOpt->text.isNull() && !tabOpt->icon.isNull())
                    return 0;
                if (tabOpt->icon.isNull() && !tabOpt->text.isNull())
                    return 0;
            }

            return widgetLayoutProp(WT_TabBar, TabBar::TabTextToIconSpace, option, widget);
        }

        case PM_TabBarTabVSpace:
            return 0;

        case PM_TabBarBaseHeight:
            return widgetLayoutProp(WT_TabBar, TabBar::BaseHeight, option, widget);

        case PM_TabBarBaseOverlap:
            return widgetLayoutProp(WT_TabBar, TabBar::BaseOverlap, option, widget);

        case PM_TabBarTabOverlap:
            return widgetLayoutProp(WT_TabBar, TabBar::TabOverlap, option, widget);

        case PM_TabBarScrollButtonWidth:
            return widgetLayoutProp(WT_TabBar, TabBar::ScrollButtonWidth, option, widget);

        case PM_TabBarTabShiftVertical:
            return 1;

        case PM_TabBarTabShiftHorizontal:
            return 0;

        case PM_SliderControlThickness:
            return widgetLayoutProp(WT_Slider, Slider::HandleThickness, option, widget);

        case PM_SliderLength:
            return widgetLayoutProp(WT_Slider, Slider::HandleLength, option, widget);

        case PM_SliderThickness:
        {
            // not sure what the difference to PM_SliderControlThickness actually is
            return widgetLayoutProp(WT_Slider, Slider::HandleThickness, option, widget);
        }

        case PM_SpinBoxFrameWidth:
            return widgetLayoutProp(WT_SpinBox, SpinBox::FrameWidth, option, widget);

        case PM_ComboBoxFrameWidth:
            return widgetLayoutProp(WT_ComboBox, ComboBox::FrameWidth, option, widget);

        case PM_HeaderMarkSize:
            return widgetLayoutProp(WT_Header, Header::MarkSize, option, widget);

        case PM_HeaderMargin:
            return widgetLayoutProp(WT_Header, Header::TextToIconSpace, option, widget);

        case PM_ToolBarFrameWidth:
            return widgetLayoutProp(WT_ToolBar, ToolBar::FrameWidth, option, widget);

        case PM_ToolBarHandleExtent:
            return widgetLayoutProp(WT_ToolBar, ToolBar::HandleExtent, option, widget);

        case PM_ToolBarSeparatorExtent:
            return widgetLayoutProp(WT_ToolBar, ToolBar::SeparatorExtent, option, widget);

        case PM_ToolBarExtensionExtent:
            return widgetLayoutProp(WT_ToolBar, ToolBar::ExtensionExtent, option, widget);

        case PM_ToolBarItemMargin:
            return widgetLayoutProp(WT_ToolBar, ToolBar::ItemMargin, option, widget);

        case PM_ToolBarItemSpacing:
            return widgetLayoutProp(WT_ToolBar, ToolBar::ItemSpacing, option, widget);

        case PM_ScrollBarExtent:
            return widgetLayoutProp(WT_ScrollBar, ScrollBar::BarWidth, option, widget);

        case PM_TitleBarHeight:
            return widgetLayoutProp(WT_Window, Window::TitleHeight, option, widget);

        default:
            break;
    }

    return QCommonStyle::pixelMetric(metric, option, widget);
}

int KStyle::layoutSpacing(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2, Qt::Orientation orientation, const QStyleOption *option, const QWidget *widget) const
{
    Q_UNUSED(control1); Q_UNUSED(control2); Q_UNUSED(orientation);

    return pixelMetric(PM_DefaultLayoutSpacing, option, widget);
}

bool KStyle::eventFilter(QObject *obj, QEvent *ev)
{
    return QCommonStyle::eventFilter(obj, ev);
}

KStyle::ColorMode::ColorMode(QPalette::ColorRole _role):
    mode(PaletteEntryMode),
    role(_role)
{}

KStyle::ColorMode::ColorMode(Mode _mode, QPalette::ColorRole _role):
    mode(_mode),
    role(_role)
{}

KStyle::ColorMode::operator int() const
{
    return int(role) | int(mode);
}

KStyle::ColorMode::ColorMode(int encoded)
{
    mode = (encoded & BWAutoContrastMode) ? BWAutoContrastMode : PaletteEntryMode;
    role = QPalette::ColorRole(encoded & (~BWAutoContrastMode));
}

QColor KStyle::ColorMode::color(const QPalette& palette)
{
    QColor palColor = palette.color(role);

    if (mode == BWAutoContrastMode) {
        if (qGray(palColor.rgb()) > 128) { //### CHECKME
            palColor = Qt::black;
        } else {
            palColor = Qt::white;
        }
    }
    return palColor;
}

KStyle::TextOption::TextOption()
{
    init();
}

KStyle::TextOption::TextOption(const QString& _text):
    text(_text)
{
    init();
}

void KStyle::TextOption::init()
{
    hAlign = Qt::AlignLeft; //NOTE: Check BIDI?
}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
