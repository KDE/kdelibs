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
#include "kstyle.moc"

#include <QtCore/qalgorithms.h>
#include <QtCore/QCache>
#include <QtCore/QEvent>
#include <QtCore/QVariant>
#include <QtGui/QAbstractItemView>
#include <QtGui/QApplication>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QFormLayout>
#include <QtGui/QIcon>
#include <QtGui/QLabel>
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>
#include <QtGui/QScrollBar>
#include <QtGui/QStyleOption>

#include <kcomponentdata.h>
#include <kglobal.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kicon.h>
#include <kiconloader.h>

#include "kglobalsettings.h"

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
    KComponentData m_componentData;

    QHash<QString, int> styleElements;
    int hintCounter, controlCounter, subElementCounter;
};

KStylePrivate::KStylePrivate() : m_componentData()
{
    if(KGlobal::hasMainComponent())
    {
        m_componentData = KGlobal::mainComponent();
    } else
    {
        QString name(QApplication::applicationName());

        if(name.isEmpty())
            name=qAppName();

        if(name.isEmpty())
            name="KStyle";

        m_componentData = KComponentData(name.toLatin1(), name.toLatin1(), KComponentData::SkipMainComponentRegistration);
    }
    selectionCache.setMaxCost(10);
    controlCounter = subElementCounter = X_KdeBase;
    hintCounter = X_KdeBase+1; //sic! X_KdeBase is covered by SH_KCustomStyleElement
}


// ----------------------------------------------------------------------------


KStyle::KStyle() : clickedLabel(0), d(new KStylePrivate)
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


QString KStyle::defaultStyle()
{
#if defined(Q_WS_X11) && !defined(Q_WS_MAEMO_5)
    return QString("oxygen");
#else
    return QString(); // native style
#endif
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
QRect KStyle::itemTextRect(const QFontMetrics &fm, const QRect &r,
                       int flags, bool enabled,
                       const QString &text) const
{
    return QCommonStyle::itemTextRect(fm, r, flags, enabled, text);
}
QRect KStyle::itemPixmapRect(const QRect &r, int flags, const QPixmap &pixmap) const
{
    return QCommonStyle::itemPixmapRect(r, flags, pixmap);
}
void KStyle::drawItemText(QPainter *painter, const QRect &rect,
                      int flags, const QPalette &pal, bool enabled,
                      const QString &text, QPalette::ColorRole textRole) const
{
    QCommonStyle::drawItemText(painter, rect, flags, pal, enabled,
                               text, textRole);
}
void KStyle::drawItemPixmap(QPainter *painter, const QRect &rect,
                            int alignment, const QPixmap &pixmap) const
{
    QCommonStyle::drawItemPixmap(painter, rect, alignment, pixmap);
}
QPalette KStyle::standardPalette() const
{
    return KGlobalSettings::createApplicationPalette(
        KSharedConfig::openConfig(d->m_componentData));
}

QIcon KStyle::standardIconImplementation(StandardPixmap standardIcon, const QStyleOption *option,
                                         const QWidget *widget) const
{
    switch (standardIcon) {
        case QStyle::SP_DesktopIcon:
            return KIcon("user-desktop");
        case QStyle::SP_TrashIcon: 
            return KIcon("user-trash");
        case QStyle::SP_ComputerIcon:
            return KIcon("computer");
        case QStyle::SP_DriveFDIcon:
            return KIcon("media-floppy");
        case QStyle::SP_DriveHDIcon:
            return KIcon("drive-harddisk");
        case QStyle::SP_DriveCDIcon:
        case QStyle::SP_DriveDVDIcon:
            return KIcon("drive-optical");
        case QStyle::SP_DriveNetIcon:
            return KIcon("folder-remote");
        case QStyle::SP_DirHomeIcon:
            return KIcon("user-home");
        case QStyle::SP_DirOpenIcon:
            return KIcon("document-open-folder");
        case QStyle::SP_DirClosedIcon:
            return KIcon("folder");
        case QStyle::SP_DirIcon:
            return KIcon("folder");
        case QStyle::SP_DirLinkIcon:
            return KIcon("folder"); //TODO: generate (!?) folder with link emblem
        case QStyle::SP_FileIcon:
            return KIcon("text-plain"); //TODO: look for a better icon
        case QStyle::SP_FileLinkIcon:
            return KIcon("text-plain"); //TODO: generate (!?) file with link emblem
        case QStyle::SP_FileDialogStart:
            return KIcon("media-playback-start"); //TODO: find correct icon
        case QStyle::SP_FileDialogEnd:
            return KIcon("media-playback-stop"); //TODO: find correct icon
        case QStyle::SP_FileDialogToParent:
            return KIcon("go-up");
        case QStyle::SP_FileDialogNewFolder:
            return KIcon("folder-new");
        case QStyle::SP_FileDialogDetailedView:
            return KIcon("view-list-details");
        case QStyle::SP_FileDialogInfoView:
            return KIcon("document-properties");
        case QStyle::SP_FileDialogContentsView:
            return KIcon("view-list-icons");
        case QStyle::SP_FileDialogListView:
            return KIcon("view-list-text");
        case QStyle::SP_FileDialogBack:
            return KIcon("go-previous");
        case QStyle::SP_MessageBoxInformation:
            return KIcon("dialog-information");
        case QStyle::SP_MessageBoxWarning:
            return KIcon("dialog-warning");
        case QStyle::SP_MessageBoxCritical:
            return KIcon("dialog-error");
        case QStyle::SP_MessageBoxQuestion:
            return KIcon("dialog-information");
        case QStyle::SP_DialogOkButton:
            return KIcon("dialog-ok");
        case QStyle::SP_DialogCancelButton:
            return KIcon("dialog-cancel");
        case QStyle::SP_DialogHelpButton:
            return KIcon("help-contents");
        case QStyle::SP_DialogOpenButton:
            return KIcon("document-open");
        case QStyle::SP_DialogSaveButton:
            return KIcon("document-save");
        case QStyle::SP_DialogCloseButton:
            return KIcon("dialog-close");
        case QStyle::SP_DialogApplyButton:
            return KIcon("dialog-ok-apply");
        case QStyle::SP_DialogResetButton:
            return KIcon("document-revert");
        case QStyle::SP_DialogDiscardButton:
            return KIcon("dialog-cancel");
        case QStyle::SP_DialogYesButton:
            return KIcon("dialog-ok-apply");
        case QStyle::SP_DialogNoButton:
            return KIcon("dialog-cancel");
        case QStyle::SP_ArrowUp:
            return KIcon("go-up");
        case QStyle::SP_ArrowDown:
            return KIcon("go-down");
        case QStyle::SP_ArrowLeft:
            return KIcon("go-previous-view");
        case QStyle::SP_ArrowRight:
            return KIcon("go-next-view");
        case QStyle::SP_ArrowBack:
            return KIcon("go-previous");
        case QStyle::SP_ArrowForward:
            return KIcon("go-next");
        case QStyle::SP_BrowserReload:
            return KIcon("view-refresh");
        case QStyle::SP_BrowserStop:
            return KIcon("process-stop");
        case QStyle::SP_MediaPlay:
            return KIcon("media-playback-start");
        case QStyle::SP_MediaStop:
            return KIcon("media-playback-stop");
        case QStyle::SP_MediaPause:
            return KIcon("media-playback-pause");
        case QStyle::SP_MediaSkipForward:
            return KIcon("media-skip-forward");
        case QStyle::SP_MediaSkipBackward:
            return KIcon("media-skip-backward");
        case QStyle::SP_MediaSeekForward:
            return KIcon("media-seek-forward");
        case QStyle::SP_MediaSeekBackward:
            return KIcon("media-seek-backward");
        case QStyle::SP_MediaVolume:
            return KIcon("audio-volume-medium");
        case QStyle::SP_MediaVolumeMuted:
            return KIcon("audio-volume-muted");

        default:
            return QStyle::standardIconImplementation(standardIcon, option, widget);
    }
}

QPixmap KStyle::standardPixmap(StandardPixmap standardPixmap, const QStyleOption *opt,
                               const QWidget *widget) const
{
    return QCommonStyle::standardPixmap(standardPixmap, opt, widget);
}
QPixmap KStyle::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap,
                                    const QStyleOption *opt) const
{
    return QCommonStyle::generatedIconPixmap(iconMode, pixmap, opt);
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



void KStyle::drawKStylePrimitive(WidgetType widgetType, int primitive,
                                 const QStyleOption* opt,
                                 const QRect &r, const QPalette &pal,
                                 State flags, QPainter* p,
                                 const QWidget* widget,
                                 KStyle::Option* kOpt) const
{
    switch (widgetType)
    {
        case WT_Tree:
        {
        switch (primitive)
        {
            case Tree::VerticalBranch:
            case Tree::HorizontalBranch:
                //### FIXME: set sane color.
                p->fillRect(r, QBrush(Qt::Dense4Pattern));
                return;
            case Tree::ExpanderOpen:
            case Tree::ExpanderClosed:
            {
                p->setPen(pal.text().color());
                drawInsideRect(p, r); //the border.
                int signLineSize = r.width()/4;
                p->drawLine(r.center().x() - signLineSize, r.center().y(),
                            r.center().x() + signLineSize, r.center().y()); //-
                if (primitive == Tree::ExpanderClosed) //vertical line of +
                    p->drawLine(r.center().x(), r.center().y() - signLineSize,
                                r.center().x(), r.center().y() + signLineSize);
                return;
            }
            default:
                break;
        }

        break;
        }

        case WT_SpinBox:
        {
        switch (primitive)
        {
            case SpinBox::PlusSymbol:
            case SpinBox::MinusSymbol:
            {
                p->setPen( pal.buttonText().color() );

                int l = qMin( r.width()-2, r.height()-2 );
                QPoint c = r.center();

                p->drawLine( c.x()-l/2, c.y(), c.x()+l/2, c.y() );
                if (primitive == SpinBox::PlusSymbol ) {
                    p->drawLine( c.x(), c.y()-l/2, c.x(), c.y()+l/2 );
                }

                return;
            }
            default:
                break;
        }

        break;
        }

        case WT_GroupBox:
        {
            if (primitive == GroupBox::FlatFrame) {
                QPen oldPen = p->pen();
                p->setPen(pal.color(QPalette::WindowText) );
                p->drawLine(r.topLeft(), r.topRight() );
                p->setPen(oldPen);
            }

            break;
        }

        case WT_ToolBoxTab:
        {
            if (primitive == ToolBoxTab::Panel) {
                drawKStylePrimitive(WT_ToolButton, ToolButton::Panel, opt, r, pal, flags, p, widget);
            }

            break;
        }

        case WT_DockWidget:
        {
            switch (primitive)
            {
                case DockWidget::TitlePanel:
                    p->fillRect(r, pal.color(QPalette::Highlight) );
                    return;

                case DockWidget::SeparatorHandle:
                    return;

                default:
                    break;
            }

            break;
        }

        case WT_Window:
        {
            switch (primitive)
            {
                case Window::TitlePanel:
                    p->fillRect(r, pal.color(QPalette::Highlight) );
                    return;

                case Window::ButtonMenu:
                {
                    KStyle::TitleButtonOption* tbkOpts =
                            extractOption<KStyle::TitleButtonOption*>(kOpt);
                    if (!tbkOpts->icon.isNull()) {
                        tbkOpts->icon.paint(p, r);
                    } else {
                        QStyleOption tool(0);
                        tool.palette = pal;
                        // TODO: give it a nice KDE logo.
                        QPixmap pm = standardPixmap(SP_TitleBarMenuButton, &tool, widget);
                        tool.rect = r;
                        p->save();
                        drawItemPixmap(p, r, Qt::AlignCenter, pm);
                        p->restore();
                    }
                    return;
                }

                case Window::ButtonMin:
                case Window::ButtonMax:
                case Window::ButtonRestore:
                case Window::ButtonClose:
                case Window::ButtonShade:
                case Window::ButtonUnshade:
                case Window::ButtonHelp:
                {
                    KStyle::TitleButtonOption* tbkOpts =
                            extractOption<KStyle::TitleButtonOption*>(kOpt);
                    State bflags = flags;
                    bflags &= ~State_Sunken;
                    if (tbkOpts->active)
                        bflags |= State_Sunken;
                    drawKStylePrimitive(WT_ToolButton, ToolButton::Panel, opt, r, pal, bflags, p, widget);
                    return;
                }
            }

            break;
        }

        case WT_TabBar:
        {
            // For vertical text fallback, provide the generic text implementation
            // a transformed rotated painter, with rect swizzled appropriately
            if (primitive == TabBar::EastText || primitive == TabBar::WestText)
            {
                QTransform tr;

                if (primitive == TabBar::WestText)
                {
                    tr.translate(r.x(), r.height() + r.y());
                    tr.rotate(-90);
                }
                else
                {
                    tr.translate(r.width() + r.x(), r.y());
                    tr.rotate(90);
                }

                p->save();
                p->setTransform(tr, true);
                drawKStylePrimitive(WT_TabBar, Generic::Text, opt,
                    QRect(0, 0, r.height(), r.width()), pal, flags, p, widget, kOpt);
                p->restore();
            }
            break;
        }

        default:
            break;
    }

    if (primitive == Generic::Text)
    {
        KStyle::TextOption* textOpts = extractOption<KStyle::TextOption*>(kOpt);

        //### debug
        //p->setPen(Qt::green);
        //drawInsideRect(p, r);

        QColor col = textOpts->color.color(pal);
        QPen   old = p->pen();
        p->setPen(col);
        drawItemText(p, r, Qt::AlignVCenter | Qt::TextShowMnemonic | textOpts->hAlign, pal, flags & State_Enabled,
                        textOpts->text);
        p->setPen(old);
    }
    else if (primitive == Generic::Icon)
    {
        KStyle::IconOption* iconOpts = extractOption<KStyle::IconOption*>(kOpt);
        QIcon::Mode mode;
        QIcon::State iconState;
        
        // Select the correct icon from the iconset
        if (flags & State_Enabled)
            if (iconOpts->active)
                mode = QIcon::Active;
            else
                mode = QIcon::Normal;
        else
            mode = QIcon::Disabled;

        if(  (flags & State_On) || (flags & State_Sunken) ) 
            iconState = QIcon::On;
        else 
            iconState = QIcon::Off;
        
        QSize size = iconOpts->size;
        if(!size.isValid())
            size = QSize(pixelMetric(PM_SmallIconSize), pixelMetric(PM_SmallIconSize));
        QPixmap icon = iconOpts->icon.pixmap(size, mode, iconState);
        p->drawPixmap(centerRect(r, icon.size()), icon);
    }
    else if (primitive == Generic::FocusIndicator)
    {
        QPen pen;
        pen.setWidth(0);
        pen.setStyle(Qt::DotLine);
        p->setPen(pen);
        drawInsideRect(p, r);
    }
    else if (primitive >= Generic::ArrowUp && primitive <= Generic::ArrowLeft)
    {
        //### FIXME: Helper for these sorts of things, as Keramik has virtually
        //identical code!
        KStyle::ColorOption* colorOpt   = extractOption<KStyle::ColorOption*>(kOpt);
        QColor               arrowColor = colorOpt->color.color(pal);

        QPolygon poly;

        switch (primitive)
        {
            case Generic::ArrowUp:
                poly.setPoints(QCOORDARRLEN(u_arrow), u_arrow);
                break;

            case Generic::ArrowDown:
                poly.setPoints(QCOORDARRLEN(d_arrow), d_arrow);
                break;

            case Generic::ArrowLeft:
                poly.setPoints(QCOORDARRLEN(l_arrow), l_arrow);
                break;

            default:
                poly.setPoints(QCOORDARRLEN(r_arrow), r_arrow);
        }

        if ( flags & State_Enabled )
        {
            //CHECKME: Why is the -1 needed?
            poly.translate(r.x() + r.width()/2 - 1, r.y() + r.height()/2);

            p->setPen(arrowColor);
            p->drawPolygon(poly);
        }
        else
        {
            //Disabled ones ignore color parameter
            poly.translate(r.x() + r.width()/2, r.y() + r.height()/2 + 1);
            p->setPen( pal.color( QPalette::Light ) );
            p->drawPolygon(poly);
            poly.translate(-1,-1);
            p->setPen(pal.mid().color());
            p->drawPolygon(poly);
        }

    }
#if 0 //Reenable if you need a debug aid
    else
    {
        p->setPen(Qt::red);
        drawInsideRect(p, r);
    }
#endif
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

QSize KStyle::expandDim(const QSize& orig, WidgetType wt, int baseMarginMetric,
                        const QStyleOption* opt, const QWidget* w, bool rotated) const
{
    int addWidth =  2*widgetLayoutProp(wt, baseMarginMetric + MainMargin, opt, w) +
                    widgetLayoutProp(wt, baseMarginMetric + Left, opt, w) +
                    widgetLayoutProp(wt, baseMarginMetric + Right, opt, w);

    int addHeight = 2*widgetLayoutProp(wt, baseMarginMetric + MainMargin, opt, w) +
                    widgetLayoutProp(wt, baseMarginMetric + Top, opt, w) +
                    widgetLayoutProp(wt, baseMarginMetric + Bot, opt, w);

    return QSize(orig.width() + (rotated? addHeight: addWidth), 
                 orig.height() + (rotated? addWidth: addHeight));
}

QRect KStyle::insideMargin(const QRect &orig, WidgetType wt,
                           int baseMarginMetric,
                           const QStyleOption* opt, const QWidget* w) const
{
    int x1 = orig.topLeft().x();
    int y1 = orig.topLeft().y();
    int x2 = orig.bottomRight().x();
    int y2 = orig.bottomRight().y();

    x1 += widgetLayoutProp(wt, baseMarginMetric + MainMargin, opt, w);
    x1 += widgetLayoutProp(wt, baseMarginMetric + Left, opt, w);

    y1 += widgetLayoutProp(wt, baseMarginMetric + MainMargin, opt, w);
    y1 += widgetLayoutProp(wt, baseMarginMetric + Top, opt, w);

    x2 -= widgetLayoutProp(wt, baseMarginMetric + MainMargin, opt, w);
    x2 -= widgetLayoutProp(wt, baseMarginMetric + Right, opt, w);

    y2 -= widgetLayoutProp(wt, baseMarginMetric + MainMargin, opt, w);
    y2 -= widgetLayoutProp(wt, baseMarginMetric + Bot, opt, w);

    return QRect(x1, y1, x2 - x1 + 1, y2 - y1 + 1);
}

QRect KStyle::handleRTL(const QStyleOption* opt, const QRect& subRect) const
{
    return visualRect(opt->direction, opt->rect, subRect);
}

QPoint KStyle::handleRTL(const QStyleOption* opt, const QPoint& pos) const
{
    return visualPos(opt->direction, opt->rect, pos);
}

void KStyle::drawPrimitive(PrimitiveElement elem, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    //Extract the stuff we need out of the option
    State flags = option->state;
    QRect      r     = option->rect;
    QPalette   pal   = option->palette;

    switch (elem)
    {
        case PE_FrameFocusRect:
            drawKStylePrimitive(WT_Generic, Generic::FocusIndicator, option, r, pal, flags, painter, widget);
            return;
        case PE_IndicatorArrowUp:
            drawKStylePrimitive(WT_Generic, Generic::ArrowUp, option, r, pal, flags, painter, widget);
            return;
        case PE_IndicatorArrowDown:
            drawKStylePrimitive(WT_Generic, Generic::ArrowDown, option, r, pal, flags, painter, widget);
            return;
        case PE_IndicatorArrowLeft:
            drawKStylePrimitive(WT_Generic, Generic::ArrowLeft, option, r, pal, flags, painter, widget);
            return;
        case PE_IndicatorArrowRight:
            drawKStylePrimitive(WT_Generic, Generic::ArrowRight, option, r, pal, flags, painter, widget);
            return;
        case PE_IndicatorMenuCheckMark:
            //### check flags
            drawKStylePrimitive(WT_MenuItem, MenuItem::CheckOn, option, r, pal, flags, painter, widget);
            return;
        case PE_IndicatorCheckBox:
            if (flags & State_NoChange)
                drawKStylePrimitive(WT_CheckBox, CheckBox::CheckTriState, option, r, pal, flags, painter, widget);
            else if (flags & State_On)
                drawKStylePrimitive(WT_CheckBox, CheckBox::CheckOn, option, r, pal, flags, painter, widget);
            else
                drawKStylePrimitive(WT_CheckBox, CheckBox::CheckOff, option, r, pal, flags, painter, widget);
            return;
        case PE_IndicatorRadioButton:
            if (flags & State_On)
                drawKStylePrimitive(WT_RadioButton, RadioButton::RadioOn, option, r, pal, flags, painter, widget);
            else
                drawKStylePrimitive(WT_RadioButton, RadioButton::RadioOff, option, r, pal, flags, painter, widget);
            return;
        case PE_IndicatorBranch:
        {
            int centerX = r.x() + r.width()/2;
            int centerY = r.y() + r.height()/2;

            int expanderAdjust = 0;
            //First, determine whether we need to draw an expander.
            if (flags & State_Children)
            {
                //How large should we make it?
                int sizeLimit = qMin(qMin(r.width(), r.height()),
                                     widgetLayoutProp(WT_Tree, Tree::MaxExpanderSize, option, widget));
                if ((sizeLimit & 1) == 0)
                    --sizeLimit;

                expanderAdjust = sizeLimit/2 + 1;

                QRect expanderRect = QRect(centerX - sizeLimit/2, centerY - sizeLimit/2,
                                           sizeLimit, sizeLimit);

                drawKStylePrimitive(WT_Tree, flags & State_Open ? Tree::ExpanderOpen : Tree::ExpanderClosed,
                                    option, expanderRect, pal, flags, painter, widget);
            }

            //Now, draw the branches. The top line gets drawn unless we're completely
            //w/o any indication of a neightbor
            if (flags & (State_Item | State_Children | State_Sibling))
            {
                QRect topLine = QRect(QPoint(centerX, r.y()), QPoint(centerX, centerY - expanderAdjust));
                drawKStylePrimitive(WT_Tree, Tree::VerticalBranch, option, topLine, pal, flags, painter, widget);
            }

            //The right/left (depending on dir) line gets drawn if we have an item
            if (flags & State_Item)
            {
                QRect horLine;
                if (option->direction == Qt::LeftToRight)
                    horLine = QRect(QPoint(centerX + expanderAdjust, centerY),
                                    QPoint(r.right(), centerY));
                else
                    horLine = QRect(QPoint(r.left(), centerY),
                                    QPoint(centerX - expanderAdjust, centerY));
                drawKStylePrimitive(WT_Tree, Tree::HorizontalBranch, option, horLine, pal, flags, painter, widget);
            }

            //The bottom if we have a sibling
            if (flags & State_Sibling)
            {
                QRect botLine = QRect(QPoint(centerX, centerY + expanderAdjust),
                                      QPoint(centerX, r.bottom()));
                drawKStylePrimitive(WT_Tree, Tree::VerticalBranch, option, botLine, pal, flags, painter, widget);
            }
            return;
        }
        case PE_FrameMenu:
            drawKStylePrimitive(WT_Menu, Generic::Frame, option, r, pal, flags, painter, widget);
            return;
        case PE_IndicatorHeaderArrow:
        {
            const QStyleOptionHeader *hOpt = qstyleoption_cast<const QStyleOptionHeader *>(option);
            int primitive = 0;
            if (flags&State_UpArrow || (hOpt && hOpt->sortIndicator==QStyleOptionHeader::SortUp))
                primitive = Generic::ArrowUp;
            else if (flags&State_DownArrow || (hOpt && hOpt->sortIndicator==QStyleOptionHeader::SortDown))
                primitive = Generic::ArrowDown;
            if (primitive != 0)
                drawKStylePrimitive(WT_Header, primitive, option, r, pal, flags, painter, widget);
            return;
        }
        case PE_FrameTabBarBase:
        {
            drawKStylePrimitive(WT_TabBar, TabBar::BaseFrame,option,r,pal,flags,painter,widget);
            return;
        }
        case PE_IndicatorTabTear:
        {
            drawKStylePrimitive(WT_TabBar, TabBar::IndicatorTear,option,r,pal,flags,painter,widget);
            return;
        }
        case PE_FrameTabWidget:
        {
            drawKStylePrimitive(WT_TabWidget, Generic::Frame,option,r,pal,flags,painter,widget);
            return;
        }

        case PE_PanelLineEdit:
        {
            drawKStylePrimitive(WT_LineEdit, LineEdit::Panel,option,r,pal,flags,painter,widget);
            return;
        }

        case PE_FrameLineEdit:
        {
            drawKStylePrimitive(WT_LineEdit, Generic::Frame,option,r,pal,flags,painter,widget);
            return;
        }

        case PE_FrameGroupBox:
        {
            if (const QStyleOptionFrame *fOpt =
                qstyleoption_cast<const QStyleOptionFrame *>(option))
            {
                QStyleOptionFrameV2 fOpt2(*fOpt);

                if (fOpt2.features & QStyleOptionFrameV2::Flat) {
                    drawKStylePrimitive(WT_GroupBox, GroupBox::FlatFrame,option,r,pal,flags,painter,widget);
                } else {
                    drawKStylePrimitive(WT_GroupBox, Generic::Frame,option,r,pal,flags,painter,widget);
                }
            }
            return;
        }

        case PE_FrameStatusBar:
        {
            drawKStylePrimitive(WT_StatusBar, Generic::Frame,option,r,pal,flags,painter,widget);
            return;
        }

        case PE_FrameDockWidget:
        {
            drawKStylePrimitive(WT_DockWidget, Generic::Frame,option,r,pal,flags,painter,widget);
            return;
        }

        case PE_IndicatorDockWidgetResizeHandle:
        {
            drawKStylePrimitive(WT_DockWidget, DockWidget::SeparatorHandle, option, r, pal, flags,
                                painter, widget);
            return;
        }

        case PE_FrameWindow:
        {
            drawKStylePrimitive(WT_Window, Generic::Frame,option,r,pal,flags,painter,widget);
            return;
        }

        case PE_Frame:
        {
            drawKStylePrimitive(WT_Generic, Generic::Frame,option,r,pal,flags,painter,widget);
            return;
        }

        case PE_IndicatorToolBarHandle:
        {
            if (flags & State_Horizontal)
                drawKStylePrimitive(WT_ToolBar, ToolBar::HandleHor,
                                    option,r,pal,flags,painter,widget);
            else
                drawKStylePrimitive(WT_ToolBar, ToolBar::HandleVert,
                                    option,r,pal,flags,painter,widget);
            return;
        }

        case PE_IndicatorToolBarSeparator:
            drawKStylePrimitive(WT_ToolBar, ToolBar::Separator,option,r,pal,flags,painter,widget);
            return;

        case PE_PanelButtonCommand:
           //case PE_PanelButtonBevel: // ### CHECKME
            drawKStylePrimitive(WT_PushButton, PushButton::Panel, option, r, pal, flags, painter, widget);
            return;
        case PE_FrameDefaultButton:
            drawKStylePrimitive(WT_PushButton, PushButton::DefaultButtonFrame, option, r, pal, flags, painter, widget);
            return;

        case PE_PanelButtonTool:
            drawKStylePrimitive(WT_ToolButton, ToolButton::Panel,option,r,pal,flags,painter,widget);
            return;

        case PE_IndicatorButtonDropDown:
            drawKStylePrimitive(WT_ToolButton, Generic::ArrowDown, option, r, pal, flags, painter, widget);
            return;

        case PE_PanelItemViewItem: {

            const QStyleOptionViewItemV4 *opt = qstyleoption_cast<const QStyleOptionViewItemV4*>(option);
            const QAbstractItemView *view = qobject_cast<const QAbstractItemView *>(widget);
            bool hover = (option->state & State_MouseOver) && (!view ||
                         view->selectionMode() != QAbstractItemView::NoSelection);

            bool hasCustomBackground = opt->backgroundBrush.style() != Qt::NoBrush &&
                                        !(option->state & State_Selected);
            bool hasSolidBackground = !hasCustomBackground || opt->backgroundBrush.style() == Qt::SolidPattern;

            const qreal rounding = 2.5;

            if (!hover && !(option->state & State_Selected) && !hasCustomBackground &&
                !(opt->features & QStyleOptionViewItemV2::Alternate))
                return;

            QPalette::ColorGroup cg;
            if (option->state & State_Enabled)
                cg = (option->state & State_Active) ? QPalette::Normal : QPalette::Inactive;
            else
                cg = QPalette::Disabled;

            QColor color;

            if (hasCustomBackground && hasSolidBackground)
                color = opt->backgroundBrush.color();
            else
                color = option->palette.color(cg, QPalette::Highlight);

            if (hover && !hasCustomBackground) {
                if (!(option->state & State_Selected))
                    color.setAlphaF(.20);
                else
                    color = color.lighter(110);
            }

            if (opt && (opt->features & QStyleOptionViewItemV2::Alternate))
                painter->fillRect(option->rect, option->palette.brush(cg, QPalette::AlternateBase));

            if (!hover && !(option->state & State_Selected) && !hasCustomBackground)
                return;

            quint64 key = quint64(option->rect.height()) << 32 | color.rgba();
            SelectionTiles* tiles = d->selectionCache.object(key);
            if (!tiles && hasSolidBackground)
            {
                QImage image(32 + 16, option->rect.height(), QImage::Format_ARGB32_Premultiplied);
                image.fill(0);

                QRect r = image.rect().adjusted(0, 0, -1, -1);

                QPainterPath path1, path2;
                path1.addRoundedRect(r, rounding, rounding);
                path2.addRoundedRect(r.adjusted(1, 1, -1, -1), rounding - 1, rounding - 1);

                // items with custom background brushes always have their background drawn
                // regardless of whether they are hovered or selected or neither so
                // the gradient effect needs to be more subtle
                int lightenAmount = hasCustomBackground ? 110 : 130;
                QLinearGradient gradient(0, 0, 0, r.bottom());
                gradient.setColorAt(0, color.lighter(lightenAmount));
                gradient.setColorAt(1, color);

                QPainter p(&image);
                p.setRenderHint(QPainter::Antialiasing);
                p.translate(.5, .5);
                p.setPen(QPen(color, 1));
                p.setBrush(gradient);
                p.drawPath(path1);
                p.strokePath(path2, QPen(QColor(255, 255, 255, 64), 1));
                p.end();

                QPixmap pixmap = QPixmap::fromImage(image);

                tiles = new SelectionTiles;
                tiles->left   = pixmap.copy(0, 0, 8, image.height());
                tiles->center = pixmap.copy(8, 0, 32, image.height());
                tiles->right  = pixmap.copy(40, 0, 8, image.height());

                d->selectionCache.insert(key, tiles);
            }
            else if (hasCustomBackground && !hasSolidBackground)
            {
                const QPointF oldBrushOrigin = painter->brushOrigin();
                painter->setBrushOrigin(opt->rect.topLeft());
                painter->setBrush(opt->backgroundBrush);
                painter->setPen(Qt::NoPen);
                painter->drawRect(opt->rect);
                painter->setBrushOrigin(oldBrushOrigin);
                return;
            }

            bool roundedLeft  = false;
            bool roundedRight = false;
            if (opt) {
                roundedLeft  = (opt->viewItemPosition == QStyleOptionViewItemV4::Beginning);
                roundedRight = (opt->viewItemPosition == QStyleOptionViewItemV4::End);
                if (opt->viewItemPosition == QStyleOptionViewItemV4::OnlyOne ||
                    opt->viewItemPosition == QStyleOptionViewItemV4::Invalid ||
                    (view && view->selectionBehavior() != QAbstractItemView::SelectRows))
                {
                    roundedLeft  = true;
                    roundedRight = true;
                }
            }

            QRect r = option->rect;
            bool reverseLayout = option->direction == Qt::RightToLeft;

            if (!reverseLayout ? roundedLeft : roundedRight) {
                painter->drawPixmap(r.topLeft(), tiles->left);
                r.adjust(8, 0, 0, 0);
            }
            if (!reverseLayout ? roundedRight : roundedLeft) {
                painter->drawPixmap(r.right() - 8 + 1, r.top(), tiles->right);
                r.adjust(0, 0, -8, 0);
            }
            if (r.isValid())
                painter->drawTiledPixmap(r, tiles->center);

            return;
        }

        default:
            break;
    }

    QCommonStyle::drawPrimitive(elem, option, painter, widget);
}

void KStyle::drawControl(ControlElement element, const QStyleOption* option, QPainter* p, const QWidget* widget) const
{
    //Extract the stuff we need out of the option
    State flags = option->state;
    QRect      r     = option->rect;
    QPalette   pal   = option->palette;

    switch (element)
    {
        case CE_PushButton:
        {
            const QStyleOptionButton* bOpt = qstyleoption_cast<const QStyleOptionButton*>(option);
            if (!bOpt) return;

            //Draw the bevel outside
            drawControl(CE_PushButtonBevel, option, p, widget);

            //Now, draw the label...
            QRect labelRect = r;

            //Move inside of default indicator margin if need be
            if ((bOpt->features & QStyleOptionButton::DefaultButton) || (bOpt->features & QStyleOptionButton::AutoDefaultButton))
                labelRect = insideMargin(labelRect, WT_PushButton, PushButton::DefaultIndicatorMargin, option, widget);

            //now get the contents area
            labelRect = insideMargin(labelRect, WT_PushButton, PushButton::ContentsMargin, option, widget);

            //### do we do anything for RTL here?

            QStyleOptionButton bOptTmp = *bOpt;
            bOptTmp.rect = labelRect;
            drawControl(CE_PushButtonLabel, &bOptTmp, p, widget);

            //Finally, renderer the focus indicator if need be
            if (flags & State_HasFocus)
            {
                QRect focusRect = insideMargin(r, WT_PushButton, PushButton::FocusMargin, option, widget);

                QStyleOptionFocusRect foOpts;
                foOpts.palette         = pal;
                foOpts.rect            = focusRect;
                foOpts.state           = flags;

                drawKStylePrimitive(WT_PushButton, Generic::FocusIndicator, &foOpts, focusRect, pal, flags, p, widget);
            }

            return;
        }

        case CE_PushButtonBevel:
        {
            const QStyleOptionButton* bOpt = qstyleoption_cast<const QStyleOptionButton*>(option);
            if (!bOpt) return;

            //Check whether we should draw default indicator.
            if (bOpt->features & QStyleOptionButton::DefaultButton)
                drawPrimitive(PE_FrameDefaultButton, option, p, widget);

            QRect bevelRect = r;
            //Exclude the margin if default or auto-default
            if ((bOpt->features & QStyleOptionButton::DefaultButton) || (bOpt->features & QStyleOptionButton::AutoDefaultButton))
                bevelRect = insideMargin(r, WT_PushButton, PushButton::DefaultIndicatorMargin, option, widget);

            //Now draw the bevel itself.
            QStyleOptionButton bOptTmp = *bOpt;
            bOptTmp.rect = bevelRect;
            drawPrimitive(PE_PanelButtonCommand, &bOptTmp, p, widget);

            return;
        }

        case CE_PushButtonLabel:
        {
            const QStyleOptionButton* bOpt = qstyleoption_cast<const QStyleOptionButton*>(option);
            if (!bOpt) return;

            //Extract out coordinates for easier manipulation
            //(OK, OK, for easier stealing of code from Keramik)
            int x, y, w, h;
            r.getRect(&x, &y, &w, &h);

            //Are we active? If so, shift contents
            bool active = (flags & State_On) || (flags & State_Sunken);
            if (active)
            {
                x += widgetLayoutProp(WT_PushButton, PushButton::PressedShiftHorizontal, option, widget);
                y += widgetLayoutProp(WT_PushButton, PushButton::PressedShiftVertical, option, widget);
            }

            //Layout the stuff.
            if (bOpt->features & QStyleOptionButton::HasMenu)
            {
                int indicatorWidth = widgetLayoutProp(WT_PushButton, PushButton::MenuIndicatorSize, option, widget);
                int indicatorSpacing = widgetLayoutProp(WT_PushButton, PushButton::TextToIconSpace, option, widget);
                w -= indicatorWidth + indicatorSpacing;

                //Draw the arrow...
                drawKStylePrimitive(WT_PushButton, Generic::ArrowDown, option,
                                    handleRTL(bOpt, QRect(x + w + indicatorSpacing, y, indicatorWidth, h)),
                                    pal, flags, p, widget);
            }

            // Draw the icon if there is one
            if (!bOpt->icon.isNull())
            {
                IconOption icoOpt;
                icoOpt.icon   = bOpt->icon;
                icoOpt.size   = bOpt->iconSize;
                icoOpt.active = flags & State_HasFocus;

                if (!bOpt->text.isEmpty())
                {
                    int margin = widgetLayoutProp(WT_PushButton, PushButton::TextToIconSpace, option, widget);
                    //Center text + icon w/margin in between..

                    //Calculate length of both.
                    int length = bOpt->iconSize.width() + margin
                                  + p->fontMetrics().size(Qt::TextShowMnemonic, bOpt->text).width();

                    //Calculate offset.
                    int offset = (w - length)/2;

                    //draw icon
                    QRect rect = QRect(QPoint(x + offset, y + h/2 - bOpt->iconSize.height()/2), bOpt->iconSize);
                    drawKStylePrimitive(WT_PushButton, Generic::Icon, option,
                                        handleRTL(bOpt, rect),
                                        pal, flags, p, widget, &icoOpt);

                    //new bounding rect for the text
                    x += offset + bOpt->iconSize.width() + margin;
                    w =  length - bOpt->iconSize.width() - margin;
                }
                else
                {
                    //Icon only. Center it. (Thankfully, they killed the icon + pixmap insanity in Qt4. Whee!
                    //(no need to do anything for RTL here, it's symmetric)
                    drawKStylePrimitive(WT_PushButton, Generic::Icon, option,
                                        QRect(x, y, w, h),
                                        pal, flags, p, widget, &icoOpt);
                }
            }
            else
            {
                //Center the text
                int textW = p->fontMetrics().size(Qt::TextShowMnemonic, bOpt->text).width();
                x += (w - textW)/2;
                w =  textW;
            }

            TextOption lbOpt(bOpt->text);
            drawKStylePrimitive(WT_PushButton, Generic::Text, option, handleRTL(bOpt, QRect(x, y, w, h)),
                                    pal, flags, p, widget, &lbOpt);

            return;
        }

        case CE_DockWidgetTitle:
        {
            const QStyleOptionDockWidget* dwOpt = ::qstyleoption_cast<const QStyleOptionDockWidget*>(option);
            if (!dwOpt) return;

            QRect textRect = insideMargin(r, WT_DockWidget, DockWidget::TitleMargin, option, widget);
            drawKStylePrimitive(WT_DockWidget, DockWidget::TitlePanel, option, r, pal, flags, p, widget);

            TextOption lbOpt(dwOpt->title);
            lbOpt.color = widgetLayoutProp(WT_DockWidget, DockWidget::TitleTextColor,
                                           option, widget);
            drawKStylePrimitive(WT_DockWidget, Generic::Text, option, textRect, pal, flags, p, widget, &lbOpt);
            return;
        }

        case CE_ToolBoxTabShape:
        {
            drawKStylePrimitive(WT_ToolBoxTab, ToolBoxTab::Panel, option, r, pal, flags, p, widget);
            return;
        }
/*
        case CE_ToolBoxTabLabel:
        {
            drawKStylePrimitive(WT_ToolBoxTab, Generic::Text, option, r, pal, flags, p, widget);
            return;
        }
*/
        case CE_CheckBox:
        {
            const QStyleOptionButton* bOpt = qstyleoption_cast<const QStyleOptionButton*>(option);
            if (!bOpt) return;

            //Draw the checkbox
            QRect checkBox = subElementRect(SE_CheckBoxIndicator, option, widget);
            QStyleOptionButton bOptTmp = *bOpt;
            bOptTmp.rect = checkBox;
            drawPrimitive(PE_IndicatorCheckBox, &bOptTmp, p, widget);

            // pixmap and text label...
            bOptTmp.rect = subElementRect(SE_CheckBoxContents, option, widget);
            drawControl(CE_CheckBoxLabel, &bOptTmp, p, widget);

            //Draw the focus rect...
            if (flags & State_HasFocus)
            {
                QRect focusRect = subElementRect(SE_CheckBoxFocusRect, option, widget);
                drawKStylePrimitive(WT_CheckBox, Generic::FocusIndicator, option, focusRect,
                                    pal, flags, p, widget);
            }
            return;
        }

        case CE_CheckBoxLabel:
        {
            const QStyleOptionButton* bOpt = qstyleoption_cast<const QStyleOptionButton*>(option);
            if (!bOpt) return;

            int textShift = 0; // shift text in case there is a label pixmap
            // draw the pixmap, if there is one
            if (!bOpt->icon.isNull())
            {
                IconOption icoOpt;
                icoOpt.icon   = bOpt->icon;
                icoOpt.size   = bOpt->iconSize;
                icoOpt.active = flags & State_HasFocus;

                QRect iconRect(r.x(), r.y() + (r.height()-bOpt->iconSize.height())/2,
                               bOpt->iconSize.width(), bOpt->iconSize.height());
                drawKStylePrimitive(WT_CheckBox, Generic::Icon, option,
                                    handleRTL(bOpt, iconRect),
                                    pal, flags, p, widget, &icoOpt);

                textShift = bOpt->iconSize.width() +
                        widgetLayoutProp(WT_RadioButton, RadioButton::BoxTextSpace, option, widget);
            }


            if (!bOpt->text.isEmpty() ) {
                TextOption lbOpt(bOpt->text);
                drawKStylePrimitive(WT_CheckBox, Generic::Text, option,
                                    handleRTL(bOpt, r.adjusted(textShift,0,0,0)),
                                    pal, flags, p, widget, &lbOpt);
            }

            return;
        }

        case CE_RadioButton:
        {
            const QStyleOptionButton* bOpt = qstyleoption_cast<const QStyleOptionButton*>(option);
            if (!bOpt) return;

            //Draw the indicator
            QRect indicator = subElementRect(SE_RadioButtonIndicator, option, widget);
            QStyleOptionButton bOptTmp = *bOpt;
            bOptTmp.rect = indicator;
            drawPrimitive(PE_IndicatorRadioButton, &bOptTmp, p, widget);

            // pixmap and text label...
            bOptTmp.rect = subElementRect(SE_RadioButtonContents, option, widget);
            drawControl(CE_RadioButtonLabel, &bOptTmp, p, widget);

            //Draw the focus rect...
            if (flags & State_HasFocus)
            {
                QRect focusRect = subElementRect(SE_RadioButtonFocusRect, option, widget);
                drawKStylePrimitive(WT_RadioButton, Generic::FocusIndicator, option, focusRect,
                                    pal, flags, p, widget);
            }
            return;
        }

        case CE_RadioButtonLabel:
        {
            const QStyleOptionButton* bOpt = qstyleoption_cast<const QStyleOptionButton*>(option);
            if (!bOpt) return;

            int textShift = 0; // shift text in case there is a label pixmap
            // draw the pixmap, if there is one
            if (!bOpt->icon.isNull())
            {
                IconOption icoOpt;
                icoOpt.icon   = bOpt->icon;
                icoOpt.active = flags & State_HasFocus;
                icoOpt.size   = bOpt->iconSize;

                QRect iconRect(r.x(), r.y() + (r.height()-bOpt->iconSize.height())/2,
                               bOpt->iconSize.width(), bOpt->iconSize.height());
                drawKStylePrimitive(WT_RadioButton, Generic::Icon, option,
                                    handleRTL(bOpt, iconRect),
                                    pal, flags, p, widget, &icoOpt);

                textShift = bOpt->iconSize.width() +
                        widgetLayoutProp(WT_RadioButton, RadioButton::BoxTextSpace, option, widget);
            }

            TextOption lbOpt(bOpt->text);
            drawKStylePrimitive(WT_RadioButton, Generic::Text, option,
                                handleRTL(bOpt, r.adjusted(textShift,0,0,0)),
                                pal, flags, p, widget, &lbOpt);
            return;
        }

        //The CE_ProgressBar implementation inside QCommonStyle is acceptible.
        //We just implement the subElementRect's it uses

        case CE_ProgressBarGroove:
        {
            drawKStylePrimitive(WT_ProgressBar, ProgressBar::Groove,  option, r,
                                pal, flags, p, widget);
            return;
        }

        case CE_ProgressBarContents:
        {
            const QStyleOptionProgressBar* pbOpt = qstyleoption_cast<const QStyleOptionProgressBar*>(option);
            const QStyleOptionProgressBarV2* pbOpt2 = qstyleoption_cast<const QStyleOptionProgressBarV2*>(option);
            if  (!pbOpt) return;

            //We layout as if LTR, relying on visualRect to fix it up
            double progress    = pbOpt->progress - pbOpt->minimum;
            int steps          = qMax(pbOpt->maximum  - pbOpt->minimum, 1);
            bool busyIndicator = (pbOpt->minimum == 0 && pbOpt->maximum == 0);
            bool horizontal    = !pbOpt2 || pbOpt2->orientation == Qt::Horizontal;

            //Do we have to draw anything?
            if (!progress && ! busyIndicator)
                return;

            //Calculate width fraction
            double widthFrac;
            if (busyIndicator)
                widthFrac = widgetLayoutProp(WT_ProgressBar, ProgressBar::BusyIndicatorSize, option, widget) / 100.0;
            else
                widthFrac = progress / steps;

            //And now the pixel width
            int width = qMin(r.width(), (int)(widthFrac * double(r.width())));
            int height = qMin(r.height(), (int)(widthFrac * r.height()));

            if (busyIndicator)
            {
                int size = width;
                if (!horizontal)
                    size = height;
                //Clamp to upper width limit
                if (size > widgetLayoutProp(WT_ProgressBar, ProgressBar::MaxBusyIndicatorSize, option, widget))
                    size = widgetLayoutProp(WT_ProgressBar, ProgressBar::MaxBusyIndicatorSize, option, widget);

                //A busy indicator with width 0 is kind of useless
                if (size < 1) size = 1;


                int remSize = (horizontal ? r.width() : r.height()) - size; //The space around which we move around...
                if (remSize <= 0) remSize = 1;  //Do something non-crashy when too small...

                int pstep =  int(progress)%(2*remSize);

                if (pstep > remSize)
                {
                    //Bounce about.. We're remWidth + some delta, we want to be remWidth - delta...
                    // - ( (remWidth + some delta) - 2* remWidth )  = - (some deleta - remWidth) = remWidth - some delta..
                    pstep = -(pstep - 2*remSize);
                }

                QRect indicatorRect;
                if (horizontal)
                    indicatorRect = QRect(r.x() + pstep, r.y(), size, r.height());
                else
                    indicatorRect = QRect(r.x(), r.y() + pstep, r.width(), size);
                drawKStylePrimitive(WT_ProgressBar, ProgressBar::BusyIndicator, option, handleRTL(option, indicatorRect),
                                    pal, flags, p, widget);
            }
            else
            {
                QRect indicatorRect;
                if (horizontal)
                    indicatorRect = QRect(r.x(), r.y(), width, r.height());
                else
                    indicatorRect = QRect(r.x(), r.bottom()-height+1, r.width(), height);
                drawKStylePrimitive(WT_ProgressBar, ProgressBar::Indicator, option, handleRTL(option, indicatorRect),
                                    pal, flags, p, widget);
            }
            return;
        }

        case CE_ProgressBarLabel:
        {
            const QStyleOptionProgressBar* pbOpt = qstyleoption_cast<const QStyleOptionProgressBar*>(option);
            const QStyleOptionProgressBarV2* pbOpt2 = qstyleoption_cast<const QStyleOptionProgressBarV2*>(option);
            if (pbOpt)
            {
                TextOption lbOpt(pbOpt->text);
                bool horizontal = !pbOpt2 || pbOpt2->orientation == Qt::Horizontal;
                bool reverseLayout = option->direction == Qt::RightToLeft;

                p->save();

                // rotate label for vertical layout
                if (!horizontal && !reverseLayout)
                {
                    p->translate(r.topRight());
                    p->rotate(90.0);
                }
                else if (!horizontal)
                {
                    p->translate(r.bottomLeft());
                    p->rotate(-90.0);
                }

                if (useSideText(pbOpt))
                {
                    lbOpt.color = QPalette::ButtonText;

                    //### or other way around?
                    if (option->direction == Qt::LeftToRight)
                        lbOpt.hAlign = Qt::AlignRight;
                    else
                        lbOpt.hAlign = Qt::AlignLeft;

                    //Handle side margin.
                    int marWidth = widgetLayoutProp(WT_ProgressBar, ProgressBar::SideTextSpace, option, widget);

                    drawKStylePrimitive(WT_ProgressBar, Generic::Text, option,
                            horizontal? r.adjusted(0, marWidth, 0, -marWidth) : QRect(0, marWidth, r.height(), r.width()-marWidth),
                            pal, flags, p, widget, &lbOpt);
                }
                else
                {
                    if (pbOpt->textAlignment == Qt::AlignLeft) //TODO: Check BIDI?
                        lbOpt.hAlign = Qt::AlignHCenter;
                    else
                        lbOpt.hAlign = pbOpt->textAlignment;

                    //Now, we need to figure out the geometry of the indicator.
                    QRect progressRect;
                    double progress    = pbOpt->progress - pbOpt->minimum;
                    int steps          = qMax(pbOpt->maximum  - pbOpt->minimum, 1);
                    bool busyIndicator = (steps <= 1);

                    int width;
                    int height;
                    if (busyIndicator)
                    {
                        //how did this happen? handle as 0%
                        width = 0;
                        height = 0;
                    }
                    else
                    {
                        double widthFrac = progress / steps;;
                        width = qMin(r.width(), (int)(widthFrac * r.width()));
                        height = qMin(r.height(), (int)(widthFrac * r.height()));
                    }

                    //If there is any indicator, we do two paths, with different
                    //clipping rects, for the two colors.
                    if (width || height)
                    {
                        if (horizontal)
                            p->setClipRect(handleRTL(option, QRect(r.x(), r.y(), width, r.height())));
                        else if (!reverseLayout)
                            p->setClipRect(QRect(r.height()-height, 0, r.height(), r.width()));
                        else
                            p->setClipRect(QRect(0, 0, height, r.width()));
                        lbOpt.color = QPalette::HighlightedText;
                        drawKStylePrimitive(WT_ProgressBar, Generic::Text, option,
                                            horizontal? r: QRect(0,0,r.height(),r.width()),
                                            pal, flags, p, widget, &lbOpt);

                        if (horizontal)
                            p->setClipRect(handleRTL(option, QRect(r.x() + width, r.y(), r.width() - width, r.height())));
                        else if (!reverseLayout)
                            p->setClipRect(QRect(0, 0, r.height()-height, r.width()));
                        else
                            p->setClipRect(QRect(height, 0, r.height()-height, r.width()));
                        lbOpt.color = QPalette::ButtonText;
                        drawKStylePrimitive(WT_ProgressBar, Generic::Text, option,
                                            horizontal? r: QRect(0,0,r.height(),r.width()),
                                            pal, flags, p, widget, &lbOpt);
                        p->setClipping(false);
                    }
                    else
                    {
                        lbOpt.color = QPalette::ButtonText;
                        drawKStylePrimitive(WT_ProgressBar, Generic::Text, option,
                                            horizontal? r: QRect(0,0,r.height(),r.width()),
                                            pal, flags, p, widget, &lbOpt);
                    }
                }
                p->restore();
            }
            return;
        }

        case CE_MenuBarItem:
        {
            const QStyleOptionMenuItem* mOpt = ::qstyleoption_cast<const QStyleOptionMenuItem*>(option);
            if (!mOpt) return;

            //Bevel...
            drawKStylePrimitive(WT_MenuBarItem, MenuBarItem::Panel, option, r,
                                pal, flags, p, widget);

            //Text...
            QRect textRect = insideMargin(r, WT_MenuBarItem, MenuBarItem::Margin, option, widget);


            TextOption lbOpt(mOpt->text);
            drawKStylePrimitive(WT_MenuBarItem, Generic::Text, option, textRect,
                                pal, flags, p, widget, &lbOpt);

            return;
        }

        case CE_MenuBarEmptyArea:
        {
            drawKStylePrimitive(WT_MenuBar, MenuBar::EmptyArea,  option, r,
                                pal, flags, p, widget);
            return;
        }

        case CE_MenuEmptyArea:
        case CE_MenuVMargin:
        case CE_MenuHMargin:
        {
            drawKStylePrimitive(WT_Menu, Menu::Background,  option, r,
                                pal, flags, p, widget);
            return;
        }

        case CE_MenuItem:
        {

            //First of all,render the background.
            drawKStylePrimitive(WT_Menu, Menu::Background, option, r,
                                pal, flags, p, widget);

            const QStyleOptionMenuItem* miOpt = ::qstyleoption_cast<const QStyleOptionMenuItem*>(option);
            if (!miOpt || miOpt->menuItemType == QStyleOptionMenuItem::EmptyArea) return;

            //Remove the margin (for everything but the column background)
            QRect ir = insideMargin(r, WT_MenuItem, MenuItem::Margin, option, widget);


            //First, figure out the left column width. When CheckAlongsideIcon is disabled it's just
            // the icon column width. Otherwise it consists of CheckWidth+CheckSpace+icon column width.
            int iconColW = miOpt->maxIconWidth;
            iconColW     = qMax(iconColW, widgetLayoutProp(WT_MenuItem, MenuItem::IconWidth, option, widget));
            int checkColW = widgetLayoutProp(WT_MenuItem, MenuItem::CheckWidth, option, widget);
            int checkSpace = widgetLayoutProp(WT_MenuItem, MenuItem::CheckSpace, option, widget);

            int leftColW = iconColW;
            // only use the additional check row if the menu has checkable menuItems.
            bool checkAlongsideIcon = (miOpt->menuHasCheckableItems &&
                    widgetLayoutProp(WT_MenuItem, MenuItem::CheckAlongsideIcon, option, widget) );
            if (checkAlongsideIcon)
            {
                leftColW = checkColW + checkSpace + iconColW;
            }

            //And the right arrow column...
            int rightColW = widgetLayoutProp(WT_MenuItem, MenuItem::ArrowSpace, option, widget) +
                            widgetLayoutProp(WT_MenuItem, MenuItem::ArrowWidth, option, widget);

            //Render left column background. This is a bit tricky, since we don't use the V margin.
            QRect leftColRect(ir.x(), r.y(), leftColW, r.height());
            drawKStylePrimitive(WT_MenuItem, MenuItem::CheckColumn, option, handleRTL(option, leftColRect),
                                pal, flags, p, widget);

            //Separators: done with the bg, can paint them and bail them out.
            if (miOpt->menuItemType == QStyleOptionMenuItem::Separator)
            {
                drawKStylePrimitive(WT_MenuItem, MenuItem::Separator, option, ir, pal, flags, p, widget);
                return;
            }

            //Now paint the active indicator --- other stuff goes on top of it
            bool active = (flags & State_Selected);

            //Active indicator...
            if (active)
                drawKStylePrimitive(WT_MenuItem, MenuItem::ItemIndicator, option, handleRTL(option, r), pal, flags, p, widget);


            ColorMode textColor = (flags & State_Enabled) ? (widgetLayoutProp(WT_MenuItem, active ?
                                                                  MenuItem::ActiveTextColor :
                                                                          MenuItem::TextColor, option, widget))
                                                          : (widgetLayoutProp(WT_MenuItem, active ?
                                                                  MenuItem::ActiveDisabledTextColor:
                                                                          MenuItem::DisabledTextColor, option, widget));

            //Readjust the column rectangle back to proper height
            leftColRect = QRect(ir.x(), ir.y(), leftColW, ir.height());
            // Paint checkbox, etc.
            if (!checkAlongsideIcon && !miOpt->icon.isNull() )
            {
                // there is an icon and the item is checked, so paint a CheckIcon
                if (miOpt->checked)
                {
                    drawKStylePrimitive(WT_MenuItem, MenuItem::CheckIcon,
                                        option, handleRTL(option, leftColRect), pal, flags,
                                        p, widget);
                }
            }
            else
            {
                // paint a normal check- resp. radiomark.
                QRect checkColRect;
                if (checkAlongsideIcon)
                {
                    checkColRect = QRect(leftColRect.x(), leftColRect.y(),
                                         checkColW, leftColRect.height() );
                }
                else
                {
                    checkColRect = leftColRect;
                }

                bool checked = miOpt->checked;
                if (miOpt->checkType == QStyleOptionMenuItem::NonExclusive)
                {
                    drawKStylePrimitive(WT_MenuItem, checked ? MenuItem::CheckOn : MenuItem::CheckOff,
                                        option, handleRTL(option, checkColRect), pal, flags,
                                        p, widget);
                }
                else if (miOpt->checkType == QStyleOptionMenuItem::Exclusive)
                {
                    drawKStylePrimitive(WT_MenuItem, checked ? MenuItem::RadioOn : MenuItem::RadioOff,
                                        option, handleRTL(option, checkColRect), pal, flags,
                                        p, widget);
                }
            }
            // Paint the menu icon.
            if (!miOpt->icon.isNull())
            {
                int iconSize = pixelMetric(PM_SmallIconSize);

                QRect iconColRect;
                if (checkAlongsideIcon)
                {
                    iconColRect = QRect(leftColRect.x()+checkColW+checkSpace, leftColRect.y(),
                                        leftColRect.width()-(checkColW+checkSpace), leftColRect.height() );
                }
                else
                {
                    iconColRect = leftColRect;
                }
                IconOption icoOpt;
                icoOpt.icon   = miOpt->icon;
                icoOpt.active = flags & State_Selected;
                drawKStylePrimitive(WT_MenuItem, Generic::Icon, option,
                                    handleRTL(option, centerRect(iconColRect, iconSize, iconSize)),
                                    pal, flags, p, widget, &icoOpt);
            }

            //Now include the spacing when calculating the next columns
            leftColW += widgetLayoutProp(WT_MenuItem, MenuItem::IconSpace, option, widget);

            //Render the text, including any accel.
            QString text = miOpt->text;
            QRect   textRect = QRect(ir.x() + leftColW, ir.y(), ir.width() - leftColW - rightColW, ir.height());
            QFont   font = miOpt->font;
            const QFont oldFont = p->font();

            p->setFont(font);
            int tabPos = miOpt->text.indexOf(QLatin1Char('\t'));
            if (tabPos != -1)
            {
                text = miOpt->text.left(tabPos);
                QString accl = miOpt->text.mid (tabPos + 1);

                //Draw the accel.
                TextOption lbOpt(accl);
                lbOpt.color  = textColor;
                lbOpt.hAlign = Qt::AlignRight;
                drawKStylePrimitive(WT_MenuItem, Generic::Text, option, handleRTL(option, textRect),
                                pal, flags, p, widget, &lbOpt);
            }

            //Draw the text.
            TextOption lbOpt(text);
            lbOpt.color = textColor;
            drawKStylePrimitive(WT_MenuItem, Generic::Text, option, handleRTL(option, textRect),
                                pal, flags, p, widget, &lbOpt);

            p->setFont(oldFont);

            //Render arrow, if need be.
            if (miOpt->menuItemType == QStyleOptionMenuItem::SubMenu)
            {
                ColorOption arrowColor;
                arrowColor.color = textColor;

                int aw = widgetLayoutProp(WT_MenuItem, MenuItem::ArrowWidth, option, widget);

                QRect arrowRect(ir.x() + ir.width() - aw, ir.y(), aw, ir.height());
                drawKStylePrimitive(WT_MenuItem, option->direction == Qt::LeftToRight ?
                                                       Generic::ArrowRight : Generic::ArrowLeft,
                                    option, handleRTL(option, arrowRect), pal, flags, p, widget, &arrowColor);
            }

            return;
        }

        case CE_ScrollBarAddLine:
        case CE_ScrollBarSubLine:
        {
            const QStyleOptionSlider* slOpt = ::qstyleoption_cast<const QStyleOptionSlider*>(option);
            if (!slOpt) return;

            //Fix up the rectangle to be what we want
            r = internalSubControlRect(CC_ScrollBar, slOpt,
                element == CE_ScrollBarAddLine ? SC_ScrollBarAddLine : SC_ScrollBarSubLine, widget);
            const_cast<QStyleOption*>(option)->rect = r;


            bool doubleButton = false;

            //See whether we're a double-button...
            if (element == CE_ScrollBarAddLine && widgetLayoutProp(WT_ScrollBar, ScrollBar::DoubleBotButton, option, widget))
                doubleButton = true;
            if (element == CE_ScrollBarSubLine && widgetLayoutProp(WT_ScrollBar, ScrollBar::DoubleTopButton, option, widget))
                doubleButton = true;

            if (doubleButton)
            {
                if (flags & State_Horizontal)
                {
                    DoubleButtonOption::ActiveButton ab = DoubleButtonOption::None;

                    //Depending on RTL direction, the one on the left is either up or down.
                    bool leftAdds, rightAdds;
                    if (slOpt->direction == Qt::LeftToRight)
                    {
                        leftAdds  = false;
                        rightAdds = true;
                    }
                    else
                    {
                        leftAdds  = true;
                        rightAdds = false;
                    }

                    //Determine whether any of the buttons is active
                    if (flags & State_Sunken)
                    {
                        if (((slOpt->activeSubControls & SC_ScrollBarAddLine) && leftAdds) ||
                            ((slOpt->activeSubControls & SC_ScrollBarSubLine) && !leftAdds))
                            ab = DoubleButtonOption::Left;

                        if (((slOpt->activeSubControls & SC_ScrollBarAddLine) && rightAdds) ||
                            ((slOpt->activeSubControls & SC_ScrollBarSubLine) && !rightAdds))
                            ab = DoubleButtonOption::Right;
                    }

                    DoubleButtonOption bOpt(ab);
                    drawKStylePrimitive(WT_ScrollBar, ScrollBar::DoubleButtonHor,
                                        option, r, pal, flags, p, widget, &bOpt);

                    //Draw the left arrow..
                    QRect leftSubButton = QRect(r.x(), r.y(), r.width()/2, r.height());

                    ColorOption colOpt;
                    colOpt.color = widgetLayoutProp(WT_ScrollBar, ScrollBar::ArrowColor, option, widget);
                    if (ab == DoubleButtonOption::Left)
                        colOpt.color = widgetLayoutProp(WT_ScrollBar, ScrollBar::ActiveArrowColor, option, widget);

                    drawKStylePrimitive(WT_ScrollBar, Generic::ArrowLeft, option, leftSubButton, pal,
                                        flags, p, widget, &colOpt);

                    //Right half..
                    QRect rightSubButton;
                    rightSubButton.setBottomRight(r.bottomRight());
                    rightSubButton.setLeft       (leftSubButton.right() + 1);
                    rightSubButton.setTop        (r.top());

                    //Chose proper color
                    colOpt.color = widgetLayoutProp(WT_ScrollBar, ScrollBar::ArrowColor, option, widget);
                    if (ab == DoubleButtonOption::Right)
                        colOpt.color = widgetLayoutProp(WT_ScrollBar, ScrollBar::ActiveArrowColor, option, widget);

                    drawKStylePrimitive(WT_ScrollBar, Generic::ArrowRight, option, rightSubButton, pal,
                                        flags, p, widget, &colOpt);
                }
                else
                {
                    DoubleButtonOption::ActiveButton ab = DoubleButtonOption::None;

                    //Determine whether any of the buttons is active
                    //Qt sets both sunken and activeSubControls for active,
                    //just activeSubControls for hover.
                    if (flags & State_Sunken)
                    {
                        if (slOpt->activeSubControls & SC_ScrollBarSubLine)
                            ab = DoubleButtonOption::Top;

                        if (slOpt->activeSubControls & SC_ScrollBarAddLine)
                            ab = DoubleButtonOption::Bottom;
                    }

                    //Paint the bevel
                    DoubleButtonOption bOpt(ab);
                    drawKStylePrimitive(WT_ScrollBar, ScrollBar::DoubleButtonVert,
                                        option, r, pal, flags, p, widget, &bOpt);

                    //Paint top button.
                    ColorOption colOpt;
                    colOpt.color = widgetLayoutProp(WT_ScrollBar, ScrollBar::ArrowColor, option, widget);

                    if (ab == DoubleButtonOption::Top)
                        colOpt.color = widgetLayoutProp(WT_ScrollBar, ScrollBar::ActiveArrowColor, option, widget);


                    QRect topSubButton = QRect(r.x(), r.y(), r.width(), r.height()/2);
                    drawKStylePrimitive(WT_ScrollBar, Generic::ArrowUp, option, topSubButton, pal,
                                        flags, p, widget, &colOpt);

                    //Paint bot button
                    QRect botSubButton;
                    botSubButton.setBottomRight(r.bottomRight());
                    botSubButton.setLeft       (r.left());
                    botSubButton.setTop        (topSubButton.bottom() + 1);

                    colOpt.color = widgetLayoutProp(WT_ScrollBar, ScrollBar::ArrowColor, option, widget);

                    if (ab == DoubleButtonOption::Bottom)
                        colOpt.color = widgetLayoutProp(WT_ScrollBar, ScrollBar::ActiveArrowColor, option, widget);

                    drawKStylePrimitive(WT_ScrollBar, Generic::ArrowDown, option, botSubButton, pal,
                                        flags, p, widget, &colOpt);
                }
            }
            else
            {   // Single button
                if (flags & State_Horizontal)
                {
                    drawKStylePrimitive(WT_ScrollBar, ScrollBar::SingleButtonHor,
                                        option, r, pal, flags, p, widget);

                    int  primitive;
                    bool active   = false;

                    if (element == CE_ScrollBarAddLine)
                    {
                        if (slOpt->direction == Qt::LeftToRight)
                            primitive = Generic::ArrowRight;
                        else
                            primitive = Generic::ArrowLeft;

                        if ((slOpt->activeSubControls & SC_ScrollBarAddLine) && (flags & State_Sunken))
                            active = true;
                    }
                    else
                    {
                        if (slOpt->direction == Qt::LeftToRight)
                            primitive = Generic::ArrowLeft;
                        else
                            primitive = Generic::ArrowRight;

                        if ((slOpt->activeSubControls & SC_ScrollBarSubLine) && (flags & State_Sunken))
                            active = true;
                    }

                    ColorOption colOpt;
                    colOpt.color = widgetLayoutProp(WT_ScrollBar, ScrollBar::ArrowColor, option, widget);
                    if (active)
                        colOpt.color = widgetLayoutProp(WT_ScrollBar, ScrollBar::ActiveArrowColor, option, widget);

                    drawKStylePrimitive(WT_ScrollBar, primitive, option, r, pal,
                                        flags, p, widget, &colOpt);
                }
                else
                {
                    drawKStylePrimitive(WT_ScrollBar, ScrollBar::SingleButtonVert,
                                        option, r, pal, flags, p, widget);

                    int  primitive;
                    bool active   = false;

                    if (element == CE_ScrollBarAddLine)
                    {
                        primitive = Generic::ArrowDown;
                        if ((slOpt->activeSubControls & SC_ScrollBarAddLine) && (flags & State_Sunken))
                            active = true;
                    }
                    else
                    {
                        primitive = Generic::ArrowUp;
                        if ((slOpt->activeSubControls & SC_ScrollBarSubLine) && (flags & State_Sunken))
                            active = true;
                    }

                    ColorOption colOpt;
                    colOpt.color = widgetLayoutProp(WT_ScrollBar, ScrollBar::ArrowColor, option, widget);
                    if (active)
                        colOpt.color = widgetLayoutProp(WT_ScrollBar, ScrollBar::ActiveArrowColor, option, widget);

                    drawKStylePrimitive(WT_ScrollBar, primitive, option, r, pal,
                                        flags, p, widget, &colOpt);
                }
            }
            return;
        }

// TODO: what about CE_ScrollBarFirst, CE_ScrollBarLast...?
//         case CE_ScrollBarFirst:
//         case CE_ScrollBarLast:

        case CE_ScrollBarSlider:
        {
            drawKStylePrimitive(WT_ScrollBar,
                                (flags & State_Horizontal) ? ScrollBar::SliderHor  :
                                        ScrollBar::SliderVert,
                                option, r, pal, flags, p, widget);
            return;
        }

        case CE_ScrollBarAddPage:
        {
            const QStyleOptionSlider* slOpt = ::qstyleoption_cast<const QStyleOptionSlider*>(option);
            if (!slOpt) return;

            if (flags & State_Horizontal)
                drawKStylePrimitive(WT_ScrollBar,
                                (slOpt->direction == Qt::LeftToRight) ? ScrollBar::GrooveAreaHorRight :
                                        ScrollBar::GrooveAreaHorLeft,
                                option, r, pal, flags, p, widget);
            else
                drawKStylePrimitive(WT_ScrollBar, ScrollBar::GrooveAreaVertBottom,
                                                     option, r, pal, flags, p, widget);
            return;
        }

        case CE_ScrollBarSubPage:
        {
            const QStyleOptionSlider* slOpt = ::qstyleoption_cast<const QStyleOptionSlider*>(option);
            if (!slOpt) return;

            if (flags & State_Horizontal)
                drawKStylePrimitive(WT_ScrollBar,
                                (slOpt->direction == Qt::LeftToRight) ? ScrollBar::GrooveAreaHorLeft :
                                        ScrollBar::GrooveAreaHorRight,
                                option, r, pal, flags, p, widget);
            else
                drawKStylePrimitive(WT_ScrollBar, ScrollBar::GrooveAreaVertTop,
                                                     option, r, pal, flags, p, widget);
            return;
        }

        //QCS's CE_TabBarTab is perfectly fine, so we just handle the subbits

        case CE_TabBarTabShape:
        {
            const QStyleOptionTab* tabOpt = qstyleoption_cast<const QStyleOptionTab*>(option);
            if (!tabOpt) return;

            // TabOverlap handling
            int tabOverlap = pixelMetric(PM_TabBarTabOverlap, option, widget);
            bool beginning = tabOpt->position == QStyleOptionTab::Beginning;
            bool onlyOne = tabOpt->position == QStyleOptionTab::OnlyOneTab;
            if (!beginning && !onlyOne) {
                switch (tabSide(tabOpt)) {
                    case North:
                    case South:
                        if (option->direction == Qt::LeftToRight)
                            r.adjust(-tabOverlap, 0, 0, 0);
                        else
                            r.adjust(0, 0, tabOverlap, 0);
                        break;
                    case East:
                    case West:
                        r.adjust(0, -tabOverlap, 0, 0);
                    default:
                        break;
                }
            }

            int prim;
            switch (tabSide(tabOpt))
            {
            case North:
                prim = TabBar::NorthTab; break;
            case South:
                prim = TabBar::SouthTab; break;
            case East:
                prim = TabBar::EastTab; break;
            default:
                prim = TabBar::WestTab; break;
            }

            drawKStylePrimitive(WT_TabBar, prim, option, r, pal, flags, p, widget);

            return;
        }

        case CE_TabBarTabLabel:
        {
            const QStyleOptionTab* tabOpt = qstyleoption_cast<const QStyleOptionTab*>(option);
            if (!tabOpt) return;

            //First, we get our content region.
            QRect labelRect = subElementRect(SE_TabBarTabText, option, widget);

            Side tabSd = tabSide(tabOpt);

            //Now, what we do, depends on rotation, LTR vs. RTL, and text/icon combinations.
            //First, figure out if we have to deal with icons, and place them if need be.
            if (!tabOpt->icon.isNull())
            {
                QStyleOptionTabV3 tabV3(*tabOpt);
                QSize iconSize = tabV3.iconSize;
                if (!iconSize.isValid()) {
                    int iconExtent = pixelMetric(PM_SmallIconSize);
                    iconSize = QSize(iconExtent, iconExtent);
                }

                IconOption icoOpt;
                icoOpt.icon   = tabOpt->icon;
                icoOpt.active = flags & State_Selected;
                icoOpt.size = iconSize;

                if (tabOpt->text.isNull())
                {
                    //Icon only. Easy.
                    drawKStylePrimitive(WT_TabBar, Generic::Icon, option, labelRect,
                                        pal, flags, p, widget, &icoOpt);
                    return;
                }

                //OK, we have to stuff both icon and text. So we figure out where to stick the icon.
                QRect iconRect;

                if (tabSd == North || tabSd == South)
                {
                    //OK, this is simple affair, we just pick a side for the icon
                    //based on layout direction. (Actually, I guess text
                    //would be more accurate, but I am -so- not doing BIDI here)
                    if (tabOpt->direction == Qt::LeftToRight)
                    {
                        //We place icon on the left.
                        iconRect = QRect(labelRect.x(), labelRect.y() + (labelRect.height() - iconSize.height() + 1) / 2,
                            iconSize.width(), iconSize.height());

                        //Adjust the text rect.
                        labelRect.setLeft(labelRect.x() + iconSize.width() +
                            widgetLayoutProp(WT_TabBar, TabBar::TabTextToIconSpace, option, widget));
                    }
                    else
                    {
                        //We place icon on the right
                        iconRect = QRect(labelRect.x() + labelRect.width() - iconSize.width(),
                            labelRect.y() + (labelRect.height() - iconSize.height() + 1) / 2, iconSize.width(), iconSize.height());
                        //Adjust the text rect
                        labelRect.setWidth(labelRect.width() - iconSize.width() -
                            widgetLayoutProp(WT_TabBar, TabBar::TabTextToIconSpace, option, widget));
                    }
                }
                else
                {
                    bool aboveIcon = false;
                    if (tabSd == West && tabOpt->direction == Qt::RightToLeft)
                        aboveIcon = true;
                    if (tabSd == East && tabOpt->direction == Qt::LeftToRight)
                        aboveIcon = true;

                    if (aboveIcon)
                    {
                        iconRect = QRect(labelRect.x() + (labelRect.width() - iconSize.width() + 1) / 2, labelRect.y(),
                                         iconSize.width(), iconSize.height());
                        labelRect.setTop(labelRect.y() + iconSize.height() +
                            widgetLayoutProp(WT_TabBar, TabBar::TabTextToIconSpace, option, widget));
                    }
                    else
                    {
                        iconRect = QRect(labelRect.x() + (labelRect.width() - iconSize.width() + 1) / 2,
                            labelRect.y() + labelRect.height() - iconSize.height(),
                                         iconSize.width(), iconSize.height());
                        labelRect.setHeight(labelRect.height() - iconSize.height() -
                            widgetLayoutProp(WT_TabBar, TabBar::TabTextToIconSpace, option, widget));
                    }
                }

                //Draw the thing
                drawKStylePrimitive(WT_TabBar, Generic::Icon, option, iconRect,
                                    pal, flags, p, widget, &icoOpt);
            } //if have icon.

            //Draw text
            if (!tabOpt->text.isNull())
            {
                TextOption lbOpt(tabOpt->text);
                if (widget)
                    lbOpt.color = widget->foregroundRole();

                int primitive = Generic::Text; // For horizontal tabs

                if (tabSd == East)
                    primitive = TabBar::EastText;
                else if (tabSd == West)
                    primitive = TabBar::WestText;

                drawKStylePrimitive(WT_TabBar, primitive, option, labelRect,
                                    pal, flags, p, widget, &lbOpt);
            }

            //If need be, draw focus rect
            if (tabOpt->state & State_HasFocus)
            {
                QRect focusRect = marginAdjustedTab(tabOpt, TabBar::TabFocusMargin);
                drawKStylePrimitive(WT_TabBar, Generic::FocusIndicator, option, focusRect,
                                    pal, flags, p, widget);
            }
            return;
        }

        case CE_ToolBar:
        {
            if (flags & State_Horizontal)
                drawKStylePrimitive(WT_ToolBar, ToolBar::PanelHor,option,r,pal,flags,p,widget);
            else
                drawKStylePrimitive(WT_ToolBar, ToolBar::PanelVert,option,r,pal,flags,p,widget);

            return;
        }

        case CE_HeaderSection:
        {
            if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option)) {
                drawKStylePrimitive(WT_Header, (header->orientation==Qt::Horizontal)?Header::SectionHor:Header::SectionVert,
                                    option, r, pal, flags, p, widget);
                return;
            }
        }

        case CE_HeaderLabel:
        {
            if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option)) {
                QRect textRect = r;
                if (!header->icon.isNull()) {
                    bool enabled = flags & State_Enabled;
                    QPixmap pm = header->icon.pixmap(pixelMetric(PM_SmallIconSize), enabled?QIcon::Normal:QIcon::Disabled);

                    // TODO: respect header->iconAlignment.
                    bool reverseLayout = header->direction == Qt::RightToLeft;
                    int iy = r.top()+(r.height()-pm.height())/2;
                    int ix = reverseLayout ? r.right()-pm.width() : r.left();
                    QRect iconRect = QRect(ix, iy, pm.width(), pm.height() );

                    IconOption iconOpt;
                    iconOpt.icon = pm;
                    drawKStylePrimitive(WT_Header, Generic::Icon, option, iconRect, pal, flags, p, widget, &iconOpt);

                    // adjust the rect for the text...
                    int spacing = widgetLayoutProp(WT_Header, Header::TextToIconSpace, option, widget);
                    if (reverseLayout)
                    {
                        textRect.setRight(r.right()-iconRect.width()-spacing );
                    }
                    else
                    {
                        textRect.setLeft(r.x()+iconRect.width()+spacing );
                    }
                }

                TextOption lbOpt(header->text);
                lbOpt.hAlign = header->textAlignment;
                drawKStylePrimitive(WT_Header, Generic::Text, option, textRect, pal, flags, p, widget, &lbOpt);
            }
            return;
        }

        case CE_Splitter:
        {
            if (flags&State_Horizontal)
                drawKStylePrimitive(WT_Splitter, Splitter::HandleHor, option, r, pal, flags, p, widget);
            else
                drawKStylePrimitive(WT_Splitter, Splitter::HandleVert, option, r, pal, flags, p, widget);
            return;
        }

        default:
            break;
    }

    QCommonStyle::drawControl(element, option, p, widget);
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
            return d->m_componentData.config()->group("KDE").readEntry("SingleClick", KDE_DEFAULT_SINGLECLICK );
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
            return KGlobalSettings::showIconsOnPushButtons();

        case SH_ItemView_ArrowKeysNavigateIntoChildren:
            return true;

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

        case PM_CheckListControllerSize:
        case PM_CheckListButtonSize:
        {
            int checkBoxSize = widgetLayoutProp(WT_CheckBox, CheckBox::Size, option, widget);
            int radioButtonSize = widgetLayoutProp(WT_RadioButton, RadioButton::Size, option, widget);
            return qMax(checkBoxSize, radioButtonSize);
        }

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

int KStyle::layoutSpacingImplementation(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2, Qt::Orientation orientation, const QStyleOption *option, const QWidget *widget) const
{
    Q_UNUSED(control1); Q_UNUSED(control2); Q_UNUSED(orientation);

    return pixelMetric(PM_DefaultLayoutSpacing, option, widget);
}


bool KStyle::isVerticalTab(const QStyleOptionTab* tbOpt) const
{
    switch (tbOpt->shape)
    {
    case QTabBar::RoundedWest:
    case QTabBar::RoundedEast:
    case QTabBar::TriangularWest:
    case QTabBar::TriangularEast:
        return true;
    default:
        return false;
    }
}

bool KStyle::isReflectedTab(const QStyleOptionTab* tbOpt) const
{
    switch (tbOpt->shape)
    {
    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast:
    case QTabBar::RoundedSouth:
    case QTabBar::TriangularSouth:
        return true;
    default:
        return false;
    }
}

KStyle::Side KStyle::tabSide(const QStyleOptionTab* tbOpt) const
{
    switch (tbOpt->shape)
    {
    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast:
        return East;
    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest:
        return West;
    case QTabBar::RoundedNorth:
    case QTabBar::TriangularNorth:
        return North;
    default:
        return South;
    }
}

QRect KStyle::marginAdjustedTab(const QStyleOptionTab* tabOpt, int property) const
{
    QRect r = tabOpt->rect;

    //For region, we first figure out the geometry if it was normal, and adjust.
    //this takes some rotating
    bool vertical = isVerticalTab (tabOpt);
    bool flip     = isReflectedTab(tabOpt);

    QRect idializedGeometry = vertical ? QRect(0, 0, r.height(), r.width())
                                        : QRect(0, 0, r.width(),  r.height());

    QRect contentArea = insideMargin(idializedGeometry, WT_TabBar, property, tabOpt, 0);

    int leftMargin  = contentArea.x();
    int rightMargin = idializedGeometry.width() - 1 - contentArea.right();
    int topMargin   = contentArea.y();
    int botMargin   = idializedGeometry.height() - 1 - contentArea.bottom();

    if (vertical)
    {
        int t       = rightMargin;
        rightMargin = topMargin;
        topMargin   = leftMargin;
        leftMargin  = botMargin;
        botMargin   = t;

        if (!flip)
            qSwap(leftMargin, rightMargin);
    }
    else if (flip)
    {
        qSwap(topMargin, botMargin);
        //For horizontal tabs, we also want to reverse stuff for RTL!
        if (tabOpt->direction == Qt::RightToLeft)
            qSwap(leftMargin, rightMargin);
    }

    QRect geom =
        QRect(QPoint(leftMargin, topMargin),
                QPoint(r.width()  - 1 - rightMargin,
                        r.height() - 1 - botMargin));
    geom.translate(r.topLeft());
    return geom;
}

bool KStyle::useSideText(const QStyleOptionProgressBar* pbOpt) const
{
    if (widgetLayoutProp(WT_ProgressBar, ProgressBar::SideText) == 0)
        return false;

    if (!pbOpt) return false; //Paranoia

    if (!pbOpt->textVisible) return false; //Don't allocate side margin if text display is off...

    if (pbOpt->textAlignment & Qt::AlignHCenter) return false; //### do we want this? we don't
                                                              //force indicator to the side outside
                                                              //the main otherwise.

    if (pbOpt->minimum == pbOpt->maximum) return false;

    int widthAlloc = pbOpt->fontMetrics.width(QLatin1String("100%"));

    if (pbOpt->fontMetrics.width(pbOpt->text) > widthAlloc)
        return false; //Doesn't fit!

    return true;
}

int KStyle::sideTextWidth(const QStyleOptionProgressBar* pbOpt) const
{
    return pbOpt->fontMetrics.width(QLatin1String("100%")) +
                                    2*widgetLayoutProp(WT_ProgressBar, ProgressBar::SideTextSpace);
}

QRect KStyle::subElementRect(SubElement sr, const QStyleOption* option, const QWidget* widget) const
{
    QRect r = option->rect;

    switch (sr)
    {
        case SE_PushButtonContents:
        {
            const QStyleOptionButton* bOpt = qstyleoption_cast<const QStyleOptionButton*>(option);
            if (!bOpt) return r;

            if ((bOpt->features & QStyleOptionButton::DefaultButton) || (bOpt->features & QStyleOptionButton::AutoDefaultButton))
                r = insideMargin(r, WT_PushButton, PushButton::DefaultIndicatorMargin, option, widget);

            return insideMargin(r, WT_PushButton, PushButton::ContentsMargin, option, widget);
        }

        case SE_PushButtonFocusRect:
        {
            const QStyleOptionButton* bOpt = qstyleoption_cast<const QStyleOptionButton*>(option);
            if (!bOpt) return r;

            if ((bOpt->features & QStyleOptionButton::DefaultButton) || (bOpt->features & QStyleOptionButton::AutoDefaultButton))
                r = insideMargin(r, WT_PushButton, PushButton::DefaultIndicatorMargin, option, widget);

            return insideMargin(r, WT_PushButton, PushButton::FocusMargin, option, widget);
        }

        case SE_ToolBoxTabContents:
        {
            return insideMargin(r, WT_ToolBoxTab, ToolBoxTab::Margin, option, widget);
        }

        case SE_CheckBoxContents:
        {
            r.setX(r.x() + widgetLayoutProp(WT_CheckBox, CheckBox::Size, option, widget) +
                           widgetLayoutProp(WT_CheckBox, CheckBox::BoxTextSpace, option, widget));
            return handleRTL(option, r);
        }

        case SE_RadioButtonContents:
        {
            r.setX(r.x() + widgetLayoutProp(WT_RadioButton, RadioButton::Size, option, widget) +
                    widgetLayoutProp(WT_RadioButton, RadioButton::BoxTextSpace, option, widget));
            return handleRTL(option, r);
        }

        case SE_CheckBoxFocusRect:
        {
            const QStyleOptionButton* bOpt = qstyleoption_cast<const QStyleOptionButton*>(option);
            if (!bOpt) return r;

            QRect ret;

            if (bOpt->text.isEmpty())
            {
                // first convert, so we can deal with logical coords
                QRect checkRect =
                        handleRTL(option, subElementRect(SE_CheckBoxIndicator, option, widget) );
                ret = insideMargin(checkRect, WT_CheckBox, CheckBox::NoLabelFocusMargin, option, widget);
            }
            else
            {
                // first convert, so we can deal with logical coords
                QRect contentsRect =
                        handleRTL(option, subElementRect(SE_CheckBoxContents, option, widget) );
                ret = insideMargin(contentsRect, WT_CheckBox, CheckBox::FocusMargin, option, widget);
            }
            // convert back to screen coords
            return handleRTL(option, ret);
        }

        case SE_RadioButtonFocusRect:
        {
            // first convert it back to logical coords
            QRect contentsRect =
                    handleRTL(option, subElementRect(SE_RadioButtonContents, option, widget) );

            // modify the rect and convert back to screen coords
            return handleRTL(option,
                             insideMargin(contentsRect, WT_RadioButton,
                                          RadioButton::FocusMargin, option, widget) );
        }

        case SE_ProgressBarGroove:
        {
            const QStyleOptionProgressBar* pbOpt = ::qstyleoption_cast<const QStyleOptionProgressBar*>(option);
            if (useSideText(pbOpt))
            {
                r.setWidth(r.width() - sideTextWidth(pbOpt));
                return r;
            }

            //Centering mode --- could be forced or side... so the groove area is everything
            return r;
        }

        case SE_ProgressBarContents:
        {
            QRect grooveRect = subElementRect(SE_ProgressBarGroove, option, widget);
            return insideMargin(grooveRect, WT_ProgressBar, ProgressBar::GrooveMargin, option, widget);
        }

        case SE_ProgressBarLabel:
        {
            const QStyleOptionProgressBar* pbOpt = ::qstyleoption_cast<const QStyleOptionProgressBar*>(option);
            if (useSideText(pbOpt))
            {
                int width = sideTextWidth(pbOpt);
                return QRect(r.x() + r.width() - width, r.y(), width, r.height());
            }

            //The same as the contents area..
            return subElementRect(SE_PushButtonContents, option, widget);
        }

        // SE_TabWidgetTabPane implementation in QCommonStyle is perfectly fine.
        case SE_TabWidgetTabContents:
        {
            const QStyleOptionTabWidgetFrame* tabOpt = qstyleoption_cast<const QStyleOptionTabWidgetFrame*>(option);
            if (!tabOpt) break;

            // Don't apply the custom margin when documentMode is enabled.
            if (tabOpt->lineWidth == 0) break;

            // use QCommonStyle's SE_TabWidgetTabPane, and adjust the result
            // according to the custom frame width.
            QRect pane = QCommonStyle::subElementRect(SE_TabWidgetTabPane, option, widget);
            int m   = widgetLayoutProp(WT_TabWidget, TabWidget::ContentsMargin, option, widget);
            int top = m+widgetLayoutProp(WT_TabWidget, TabWidget::ContentsMargin+Top,
                                         option, widget);
            int bot = m+widgetLayoutProp(WT_TabWidget, TabWidget::ContentsMargin+Bot,
                                         option, widget);
            int left = m+widgetLayoutProp(WT_TabWidget, TabWidget::ContentsMargin+Left,
                                         option, widget);
            int right = m+widgetLayoutProp(WT_TabWidget, TabWidget::ContentsMargin+Right,
                                         option, widget);

            switch (tabOpt->shape) {
                case QTabBar::RoundedNorth:
                case QTabBar::TriangularNorth:
                    return pane.adjusted(left,top,-right,-bot);
                case QTabBar::RoundedEast:
                case QTabBar::TriangularEast:
                    return pane.adjusted(bot,left, -top,-right);
                case QTabBar::RoundedSouth:
                case QTabBar::TriangularSouth:
                    return pane.adjusted(right,bot, -left,-top);
                case QTabBar::RoundedWest:
                case QTabBar::TriangularWest:
                    return pane.adjusted(top,right, -bot,-left);
            }
        }

        case SE_TabBarTabText:
        {
            const QStyleOptionTab* tabOpt = qstyleoption_cast<const QStyleOptionTab*>(option);
            if (!tabOpt) return QRect();

            QRect r = marginAdjustedTab(tabOpt, TabBar::TabContentsMargin);
            QStyleOptionTabV3 tov3(*tabOpt);

            switch (tov3.shape)
            {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
                if (tov3.direction == Qt::LeftToRight)
                    r.adjust(tov3.leftButtonSize.width(), 0, -tov3.rightButtonSize.width(), 0);
                else
                    r.adjust(tov3.rightButtonSize.width(), 0, -tov3.leftButtonSize.width(), 0);
                break;
            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
                r.adjust(0, tov3.leftButtonSize.width(), 0, -tov3.rightButtonSize.width());
                break;
            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
                r.adjust(0, tov3.rightButtonSize.width(), 0, -tov3.leftButtonSize.width());
                break;
            }

            return r;
        }

        default:
            break;
    }

    return QCommonStyle::subElementRect(sr, option, widget);
}

void  KStyle::drawComplexControl (ComplexControl cc, const QStyleOptionComplex* opt,
                                   QPainter *p,      const QWidget* w) const
{
    //Extract the stuff we need out of the option
    State flags = opt->state;
    QRect      r     = opt->rect;
    QPalette   pal   = opt->palette;

    switch (cc)
    {
        case CC_ScrollBar:
        {
            QStyleOptionComplex* mutableOpt = const_cast<QStyleOptionComplex*>(opt);
            if ((mutableOpt->subControls & SC_ScrollBarSubLine) || (mutableOpt->subControls & SC_ScrollBarAddLine))
            {
                //If we paint one of the buttons, must paint both!
                mutableOpt->subControls |= SC_ScrollBarSubPage | SC_ScrollBarAddLine;
            }
            //Note: we falldown to the base intentionally
        }
        break;

        case CC_Q3ListView:
        {
            const QStyleOptionQ3ListView* lvOpt = qstyleoption_cast<const QStyleOptionQ3ListView*>(opt);
            Q_ASSERT (lvOpt);

            if (lvOpt->subControls & SC_Q3ListView)
                QCommonStyle::drawComplexControl(cc, opt, p, w);

            if (lvOpt->items.isEmpty())
                return;

            // If we have a branch or are expanded...
            if (lvOpt->subControls & (SC_Q3ListViewBranch | SC_Q3ListViewExpand))
            {
                QStyleOptionQ3ListViewItem item  = lvOpt->items.at(0);

                int y = r.y();

                QStyleOption opt; //For painting
                opt.palette   = lvOpt->palette;
                opt.direction = Qt::LeftToRight;

                //Remap the painter so (0,0) corresponds to the origin
                //of the widget, to help out the line align code.
                //Extract the paint offset. Here be dragons
                //(and not the cute green Patron of the project, either)
                int cX = w ? w->property("contentsX").toInt() : 0;
                int cY = w ? w->property("contentsY").toInt() : 0;

                QPoint adjustCoords = p->matrix().map(QPoint(0,0)) + QPoint(cX, cY);
                p->translate(-adjustCoords);

                if (lvOpt->activeSubControls == SC_All && (lvOpt->subControls & SC_Q3ListViewExpand)) {
                    //### CHECKME: this is from KStyle3, and needs to be re-checked/tested
                    // We only need to draw a vertical line
                    //Route through the Qt4 style-call.
                    QStyleOption opt;
                    opt.rect  = QRect(r.topLeft() + adjustCoords, r.size());
                    opt.state = State_Sibling;
                    drawPrimitive(PE_IndicatorBranch, &opt, p, 0);
                } else {
                    int childPos = 1;

                    // Draw all the expand/close boxes, and nearby branches
                    while (childPos < lvOpt->items.size() && y < r.height())
                    {
                        const QStyleOptionQ3ListViewItem& child = lvOpt->items.at(childPos);
                        if (!(child.features & QStyleOptionQ3ListViewItem::Visible))
                        {
                            childPos++;
                            continue;
                        }

                        //Route through the Qt4 style-call.
                        opt.rect  = QRect(r.x() + adjustCoords.x(), y + adjustCoords.y(),
                                          r.width(), child.height);
                        opt.state = State_Item;

                        if (child.features & QStyleOptionQ3ListViewItem::Expandable || child.childCount)
                        {
                            opt.state |= State_Children;
                            opt.state |= (child.state & State_Open);
                        }

                        //See if we have a visible sibling
                        int siblingPos = 0;
                        for (siblingPos = childPos + 1; siblingPos < lvOpt->items.size(); ++siblingPos)
                        {
                            if (lvOpt->items.at(siblingPos).features & QStyleOptionQ3ListViewItem::Visible)
                            {
                                opt.state |= State_Sibling;
                                break;
                            }
                        }

                        //If on screen, paint it
                        if (y + child.height > 0)
                            drawPrimitive(PE_IndicatorBranch, &opt, p, 0);

                        if (!siblingPos)
                            break;

                        //If we have a sibling, and an expander, also have to draw
                        //a line for below the immediate area
                        if ((opt.state & State_Children) && (opt.state & State_Sibling))
                        {
                            opt.state = State_Sibling;
                            opt.rect  = QRect(r.x() + adjustCoords.x(),
                                              y + adjustCoords.y() + child.height,
                                              r.width(), child.totalHeight - child.height);
                            if (opt.rect.height())
                                drawPrimitive(PE_IndicatorBranch, &opt, p, 0);
                        }

                        y += child.totalHeight;
                        childPos = siblingPos;
                    } //loop through items
                } //complex case

                p->translate(adjustCoords);
            } //if have branch or expander
        } //CC_Q3ListView
        break;

        case CC_Slider:
        {
            if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(opt))
            {
                QRect groove = subControlRect(CC_Slider, slider, SC_SliderGroove, w);
                QRect handle = subControlRect(CC_Slider, slider, SC_SliderHandle, w);
                bool hor = slider->orientation == Qt::Horizontal;

                if (slider->subControls & SC_SliderTickmarks)
                {
                    // TODO: make tickmarks customizable with Slider::Tickmark-primitives?
                    QStyleOptionSlider tmpSlider = *slider;
                    tmpSlider.subControls = SC_SliderTickmarks;
                    QCommonStyle::drawComplexControl(cc, &tmpSlider, p, w);
                }

                if ((slider->subControls & SC_SliderGroove) && groove.isValid())
                {
                    drawKStylePrimitive(WT_Slider, hor ? Slider::GrooveHor : Slider::GrooveVert, opt, groove, pal, flags, p, w);
                }

                if (slider->subControls & SC_SliderHandle)
                {
                    drawKStylePrimitive(WT_Slider, hor ? Slider::HandleHor : Slider::HandleVert, opt, handle, pal, flags, p, w);

                    if (slider->state & State_HasFocus) {
                        QRect focus = subElementRect(SE_SliderFocusRect, slider, w);
                        drawKStylePrimitive(WT_Slider, Generic::FocusIndicator, opt, focus, pal, flags, p, w, 0);
                    }
                }
            } //option OK
            return;
        } //CC_Slider

        case CC_SpinBox:
        {
            if (const QStyleOptionSpinBox *sb = qstyleoption_cast<const QStyleOptionSpinBox *>(opt) )
            {
                bool activeSbUp = sb->activeSubControls&SC_SpinBoxUp && (flags & State_Sunken);
                bool activeSbDown = sb->activeSubControls&SC_SpinBoxDown && (flags & State_Sunken);

                if (sb->subControls & SC_SpinBoxFrame)
                {
                    drawKStylePrimitive(WT_SpinBox, Generic::Frame, opt, r, pal, flags, p, w);
                }

                if (sb->subControls & SC_SpinBoxEditField)
                {
                    QRect editField = subControlRect(CC_SpinBox, opt, SC_SpinBoxEditField, w);
                    drawKStylePrimitive(WT_SpinBox, SpinBox::EditField, opt, editField, pal, flags, p, w);
                }

                QRect upRect, downRect;
                if (sb->subControls & (SC_SpinBoxUp | SC_SpinBoxDown))
                {
                    upRect   = subControlRect(CC_SpinBox, opt, SC_SpinBoxUp,   w);
                    downRect = subControlRect(CC_SpinBox, opt, SC_SpinBoxDown, w);
                    QRect buttonAreaRect = upRect | downRect;
                    drawKStylePrimitive(WT_SpinBox, SpinBox::ButtonArea, opt, buttonAreaRect, pal, flags, p, w);
                }

                if (sb->subControls & SC_SpinBoxUp)
                {
                    // adjust the sunken state flag...
                    State upFlags = flags;
                    if (activeSbUp)
                        upFlags |= State_Sunken;
                    else
                        upFlags &= ~State_Sunken;

                    drawKStylePrimitive(WT_SpinBox, SpinBox::UpButton, opt, upRect, pal, upFlags, p, w);

                    // draw symbol...
                    int primitive;
                    if (sb->buttonSymbols == QAbstractSpinBox::PlusMinus)
                        primitive = SpinBox::PlusSymbol;
                    else
                        primitive = Generic::ArrowUp;
                    drawKStylePrimitive(WT_SpinBox, primitive, opt, upRect, pal, upFlags, p, w);
                }

                if (sb->subControls & SC_SpinBoxDown)
                {
                    // adjust the sunken state flag...
                    State downFlags = flags;
                    if (activeSbDown)
                        downFlags |= State_Sunken;
                    else
                        downFlags &= ~State_Sunken;

                    drawKStylePrimitive(WT_SpinBox, SpinBox::DownButton, opt, downRect, pal, downFlags, p, w);

                    // draw symbol...
                    int primitive;
                    if (sb->buttonSymbols == QAbstractSpinBox::PlusMinus)
                        primitive = SpinBox::MinusSymbol;
                    else
                        primitive = Generic::ArrowDown;
                    drawKStylePrimitive(WT_SpinBox, primitive, opt, downRect, pal, downFlags, p, w);
                }

                return;
            } //option OK
        } //CC_SpinBox

        case CC_ComboBox:
        {
            if (const QStyleOptionComboBox *cb = qstyleoption_cast<const QStyleOptionComboBox *>(opt) )
            {
                if (cb->subControls & SC_ComboBoxFrame)
                {
                    drawKStylePrimitive(WT_ComboBox, Generic::Frame, opt, r, pal, flags, p, w);

                    // focus indicator
                    if (cb->state & State_HasFocus) {
                        QRect editField = subControlRect(CC_ComboBox, opt, SC_ComboBoxEditField, w);
                        QRect focusRect = insideMargin(editField, WT_ComboBox, ComboBox::FocusMargin, opt, w);
                        drawKStylePrimitive(WT_ComboBox, Generic::FocusIndicator, opt, focusRect, pal, flags, p, w, 0);
                    }
                }

                if (cb->subControls & SC_ComboBoxEditField)
                {
                    QRect editField = subControlRect(CC_ComboBox, opt, SC_ComboBoxEditField, w);
                    drawKStylePrimitive(WT_ComboBox, ComboBox::EditField, opt, editField, pal, flags, p, w);
                }

                if (cb->subControls & SC_ComboBoxArrow)
                {
                    QRect buttonRect = subControlRect(CC_ComboBox, opt, SC_ComboBoxArrow, w);
                    drawKStylePrimitive(WT_ComboBox, ComboBox::Button, opt, buttonRect, pal, flags, p, w);

                    // draw symbol...
                    drawKStylePrimitive(WT_ComboBox, Generic::ArrowDown, opt, buttonRect, pal, flags, p, w);
                }

                return;
            } //option OK
            break;
        } //CC_Combo

        case CC_ToolButton:
        {
            if (const QStyleOptionToolButton *tool = qstyleoption_cast<const QStyleOptionToolButton *>(opt)) {
                QRect buttonRect = subControlRect(cc, tool, SC_ToolButton, w);
                QRect menuRect = subControlRect(cc, tool, SC_ToolButtonMenu, w);

                // State_AutoRaise: only draw button when State_MouseOver
                State bflags = tool->state;
                if (bflags & State_AutoRaise) {
                    if (!(bflags & State_MouseOver)) {
                        bflags &= ~State_Raised;
                    }
                }
                State mflags = bflags;

                QStyleOption tOpt(0);
                tOpt.palette = pal;

                if (tool->subControls & SC_ToolButton) {
                    if (bflags & (State_Sunken | State_On | State_Raised)) {
                        tOpt.rect = buttonRect;
                        tOpt.state = bflags;
                        drawPrimitive(PE_PanelButtonTool, &tOpt, p, w);
                    }
                }

                if (tool->subControls & SC_ToolButtonMenu) {
                    tOpt.rect = menuRect;
                    tOpt.state = mflags;
                    drawPrimitive(PE_IndicatorButtonDropDown, &tOpt, p, w);
                } else if (tool->features & QStyleOptionToolButton::HasMenu) {
                    // This is requesting KDE3-style arrow indicator, per Qt 4.4 behavior. Qt 4.3 prefers to hide
                    // the fact of the menu's existence. Whee! Since we don't know how to paint this right,
                    // though, we have to have some metrics set for it to look nice.
                    int size = widgetLayoutProp(WT_ToolButton, ToolButton::InlineMenuIndicatorSize, opt, w);

                    if (size) {
                        int xOff = widgetLayoutProp(WT_ToolButton, ToolButton::InlineMenuIndicatorXOff, opt, w);
                        int yOff = widgetLayoutProp(WT_ToolButton, ToolButton::InlineMenuIndicatorYOff, opt, w);

                        QRect r = QRect(buttonRect.right() + xOff, buttonRect.bottom() + yOff, size, size);
                        tOpt.rect  = r;
                        tOpt.state = bflags;
                        drawPrimitive(PE_IndicatorButtonDropDown, &tOpt, p, w);
                    }
                }

                if (flags & State_HasFocus) {
                    QRect focusRect = insideMargin(r, WT_ToolButton, ToolButton::FocusMargin, opt, w);
                    tOpt.rect = focusRect;
                    tOpt.state = bflags;
                    drawKStylePrimitive(WT_ToolButton, Generic::FocusIndicator, &tOpt, focusRect, pal, bflags, p, w);
                }

                // CE_ToolButtonLabel expects a readjusted rect, for the button area proper
                QStyleOptionToolButton labelOpt = *tool;
                labelOpt.rect = buttonRect;
                drawControl(CE_ToolButtonLabel, &labelOpt, p, w);

                return;
            }
            break;
        } //CC_ToolButton

        case CC_TitleBar:
        {
            const QStyleOptionTitleBar *tb =
                    qstyleoption_cast<const QStyleOptionTitleBar *>(opt);
            if (!tb)
                break;

            // title bar
            drawKStylePrimitive(WT_Window, Window::TitlePanel, opt, r, pal, flags, p, w);

            // TODO: different color depending on Active/inactive state
            // draw title text
            QRect textRect = subControlRect(CC_TitleBar, tb, SC_TitleBarLabel, w);
            TextOption textOpt(tb->text);
            textOpt.color = widgetLayoutProp(WT_Window, Window::TitleTextColor, opt, w);
            drawKStylePrimitive(WT_Window, Generic::Text, opt, textRect,
                                pal, flags, p, w, &textOpt);

            TitleButtonOption buttonKOpt;
            buttonKOpt.icon = tb->icon;

            if ((tb->subControls & SC_TitleBarSysMenu) &&
                 (tb->titleBarFlags & Qt::WindowSystemMenuHint))
            {
                buttonKOpt.active = (tb->activeSubControls & SC_TitleBarSysMenu)
                        && (tb->state & State_Sunken);
                QRect br = subControlRect(CC_TitleBar, tb, SC_TitleBarSysMenu, w);
                drawKStylePrimitive(WT_Window, Window::ButtonMenu, opt, br, pal, flags, p, w,
                                   &buttonKOpt);
            }

            if ((tb->subControls & SC_TitleBarMinButton) &&
                 (tb->titleBarFlags & Qt::WindowMinimizeButtonHint))
            {
                buttonKOpt.active = (tb->activeSubControls & SC_TitleBarMinButton)
                        && (tb->state & State_Sunken);
                QRect br = subControlRect(CC_TitleBar, tb, SC_TitleBarMinButton, w);
                drawKStylePrimitive(WT_Window, Window::ButtonMin, opt, br, pal, flags, p, w,
                                    &buttonKOpt);
            }

            if ((tb->subControls & SC_TitleBarMaxButton) &&
                 (tb->titleBarFlags & Qt::WindowMaximizeButtonHint))
            {
                buttonKOpt.active = (tb->activeSubControls & SC_TitleBarMaxButton)
                        && (tb->state & State_Sunken);
                QRect br = subControlRect(CC_TitleBar, tb, SC_TitleBarMaxButton, w);
                drawKStylePrimitive(WT_Window, Window::ButtonMax, opt, br, pal, flags, p, w,
                                    &buttonKOpt);
            }

            if ((tb->subControls & SC_TitleBarCloseButton) &&
                 (tb->titleBarFlags & Qt::WindowSystemMenuHint))
            {
//                 bool hover = (tb->activeSubControls & SC_TitleBarCloseButton)
//                         && (tb->state & State_MouseOver);
                buttonKOpt.active = (tb->activeSubControls & SC_TitleBarCloseButton)
                        && (tb->state & State_Sunken);
                QRect br = subControlRect(CC_TitleBar, tb, SC_TitleBarCloseButton, w);
                drawKStylePrimitive(WT_Window, Window::ButtonClose, opt, br, pal, flags, p, w,
                                    &buttonKOpt);
            }

            if ((tb->subControls & SC_TitleBarNormalButton) &&
                 (((tb->titleBarFlags & Qt::WindowMinimizeButtonHint) &&
                 (tb->titleBarState & Qt::WindowMinimized)) ||
                 ((tb->titleBarFlags & Qt::WindowMaximizeButtonHint) &&
                 (tb->titleBarState & Qt::WindowMaximized))))
            {
                buttonKOpt.active = (tb->activeSubControls & SC_TitleBarNormalButton)
                        && (tb->state & State_Sunken);
                QRect br = subControlRect(CC_TitleBar, tb, SC_TitleBarNormalButton, w);
                drawKStylePrimitive(WT_Window, Window::ButtonRestore, opt, br, pal, flags, p, w,
                                    &buttonKOpt);
            }

            if (tb->subControls & SC_TitleBarShadeButton)
            {
                buttonKOpt.active = (tb->activeSubControls & SC_TitleBarShadeButton)
                        && (tb->state & State_Sunken);
                QRect br = subControlRect(CC_TitleBar, tb, SC_TitleBarShadeButton, w);
                drawKStylePrimitive(WT_Window, Window::ButtonShade, opt, br, pal, flags, p, w,
                                    &buttonKOpt);
            }

            if (tb->subControls & SC_TitleBarUnshadeButton)
            {
                buttonKOpt.active = (tb->activeSubControls & SC_TitleBarUnshadeButton)
                        && (tb->state & State_Sunken);
                QRect br = subControlRect(CC_TitleBar, tb, SC_TitleBarUnshadeButton, w);
                drawKStylePrimitive(WT_Window, Window::ButtonUnshade, opt, br, pal, flags, p, w,
                                    &buttonKOpt);
            }

            if ((tb->subControls & SC_TitleBarContextHelpButton)
                && (tb->titleBarFlags & Qt::WindowContextHelpButtonHint))
            {
                buttonKOpt.active = (tb->activeSubControls & SC_TitleBarContextHelpButton)
                        && (tb->state & State_Sunken);
                QRect br = subControlRect(CC_TitleBar, tb, SC_TitleBarContextHelpButton, w);
                drawKStylePrimitive(WT_Window, Window::ButtonHelp, opt, br, pal, flags, p, w,
                                    &buttonKOpt);
            }

            return;
        } // CC_TitleBar

        default:
            break;
    } //switch

    QCommonStyle::drawComplexControl(cc, opt, p, w);
}


QRect KStyle::internalSubControlRect (ComplexControl control, const QStyleOptionComplex* option,
                                       SubControl subControl, const QWidget* widget) const
{
    QRect r = option->rect;

    if (control == CC_ScrollBar)
    {
        switch (subControl)
        {
            //The "top" arrow
            case SC_ScrollBarSubLine:
            {
                int majorSize;
                if (widgetLayoutProp(WT_ScrollBar, ScrollBar::DoubleTopButton, option, widget))
                    majorSize = widgetLayoutProp(WT_ScrollBar, ScrollBar::DoubleButtonHeight, option, widget);
                else
                    majorSize = widgetLayoutProp(WT_ScrollBar, ScrollBar::SingleButtonHeight, option, widget);

                if (option->state & State_Horizontal)
                    return handleRTL(option, QRect(r.x(), r.y(), majorSize, r.height()));
                else
                    return handleRTL(option, QRect(r.x(), r.y(), r.width(), majorSize));

            }

            //The "bottom" arrow
            case SC_ScrollBarAddLine:
            {
                int majorSize;
                if (widgetLayoutProp(WT_ScrollBar, ScrollBar::DoubleBotButton, option, widget))
                    majorSize = widgetLayoutProp(WT_ScrollBar, ScrollBar::DoubleButtonHeight, option, widget);
                else
                    majorSize = widgetLayoutProp(WT_ScrollBar, ScrollBar::SingleButtonHeight, option, widget);

                if (option->state & State_Horizontal)
                    return handleRTL(option, QRect(r.right() - majorSize + 1, r.y(), majorSize, r.height()));
                else
                    return handleRTL(option, QRect(r.x(), r.bottom() - majorSize + 1, r.width(), majorSize));
            }

            default:
                break;
        }
    }

    return QRect();
}


QRect KStyle::subControlRect(ComplexControl control, const QStyleOptionComplex* option,
                                SubControl subControl, const QWidget* widget) const
{
    QRect r = option->rect;

    switch (control)
    {
        case CC_ScrollBar:
        {
            switch (subControl)
            {
                //For both arrows, we return -everything-,
                //to get stuff to repaint right. See internalSubControlRect
                //for the real thing
                case SC_ScrollBarSubLine:
                case SC_ScrollBarAddLine:
                    return r;

                //The main groove area. This is used to compute the others...
                case SC_ScrollBarGroove:
                {
                    QRect top = handleRTL(option, internalSubControlRect(control, option, SC_ScrollBarSubLine, widget));
                    QRect bot = handleRTL(option, internalSubControlRect(control, option, SC_ScrollBarAddLine, widget));

                    QPoint topLeftCorner, botRightCorner;
                    if (option->state & State_Horizontal)
                    {
                        topLeftCorner  = QPoint(top.right() + 1, top.top());
                        botRightCorner = QPoint(bot.left()  - 1, top.bottom());
                    }
                    else
                    {
                        topLeftCorner  = QPoint(top.left(),  top.bottom() + 1);
                        botRightCorner = QPoint(top.right(), bot.top()    - 1);
                    }

                    return handleRTL(option, QRect(topLeftCorner, botRightCorner));
                }

                case SC_ScrollBarFirst:
                case SC_ScrollBarLast:
                    return QRect();

                case SC_ScrollBarSlider:
                {
                    const QStyleOptionSlider* slOpt = ::qstyleoption_cast<const QStyleOptionSlider*>(option);

                    //We do handleRTL here to unreflect things if need be
                    QRect groove = handleRTL(option, subControlRect(control, option, SC_ScrollBarGroove, widget));
            Q_ASSERT (slOpt);

                    if (slOpt->minimum == slOpt->maximum)
                        return groove;

                    //Figure out how much room we have..
                    int space;
                    if (option->state & State_Horizontal)
                        space = groove.width();
                    else
                        space = groove.height();

                    //Calculate the portion of this space that the slider should take up.
                    int sliderSize = int(space * float(slOpt->pageStep) /
                                            (slOpt->maximum - slOpt->minimum + slOpt->pageStep));

                    if (sliderSize < widgetLayoutProp(WT_ScrollBar, ScrollBar::MinimumSliderHeight, option, widget))
                        sliderSize = widgetLayoutProp(WT_ScrollBar, ScrollBar::MinimumSliderHeight, option, widget);

                    if (sliderSize > space)
                        sliderSize = space;

                    //What do we have remaining?
                    space = space - sliderSize;

                    //uhm, yeah, nothing much
                    if (space <= 0)
                        return groove;

                    int pos = qRound(float(slOpt->sliderPosition - slOpt->minimum)/
                                            (slOpt->maximum - slOpt->minimum)*space);
                    if (option->state & State_Horizontal)
                        return handleRTL(option, QRect(groove.x() + pos, groove.y(), sliderSize, groove.height()));
                    else
                        return handleRTL(option, QRect(groove.x(), groove.y() + pos, groove.width(), sliderSize));
                }

                case SC_ScrollBarSubPage:
                {
                    //We do handleRTL here to unreflect things if need be
                    QRect slider = handleRTL(option, subControlRect(control, option, SC_ScrollBarSlider, widget));
                    QRect groove = handleRTL(option, subControlRect(control, option, SC_ScrollBarGroove, widget));

                    //We're above the slider in the groove.
                    if (option->state & State_Horizontal)
                        return handleRTL(option, QRect(groove.x(), groove.y(), slider.x() - groove.x(), groove.height()));
                    else
                        return handleRTL(option, QRect(groove.x(), groove.y(), groove.width(), slider.y() - groove.y()));
                }

                case SC_ScrollBarAddPage:
                {
                    //We do handleRTL here to unreflect things if need be
                    QRect slider = handleRTL(option, subControlRect(control, option, SC_ScrollBarSlider, widget));
                    QRect groove = handleRTL(option, subControlRect(control, option, SC_ScrollBarGroove, widget));

                    //We're below the slider in the groove.
                    if (option->state & State_Horizontal)
                        return handleRTL(option,
                                QRect(slider.right() + 1, groove.y(), groove.right() - slider.right(), groove.height()));
                    else
                        return handleRTL(option,
                                QRect(groove.x(), slider.bottom() + 1, groove.width(), groove.bottom() - slider.bottom()));
                }

                default:
                    break;
            }
        } //CC_ScrollBar

        case CC_SpinBox:
        {
            if (const QStyleOptionSpinBox *sb = qstyleoption_cast<const QStyleOptionSpinBox *>(option)) {

                int fw = widgetLayoutProp(WT_SpinBox, SpinBox::FrameWidth, option, widget);
                int bw = widgetLayoutProp(WT_SpinBox, SpinBox::ButtonWidth, option, widget);
                int bm = widgetLayoutProp(WT_SpinBox, SpinBox::ButtonMargin, option, widget);
                int bml = bm + widgetLayoutProp(WT_SpinBox, SpinBox::ButtonMargin + Left, option, widget);
                int bmr = bm + widgetLayoutProp(WT_SpinBox, SpinBox::ButtonMargin + Right, option, widget);
                int bmt = bm + widgetLayoutProp(WT_SpinBox, SpinBox::ButtonMargin + Top, option, widget);
                int bmb = bm + widgetLayoutProp(WT_SpinBox, SpinBox::ButtonMargin + Bot, option, widget);
                int bs = widgetLayoutProp(WT_SpinBox, SpinBox::ButtonSpacing, option, widget);
                bool symmButtons = widgetLayoutProp(WT_SpinBox, SpinBox::SymmetricButtons, option, widget);
                bool supportFrameless = widgetLayoutProp(WT_SpinBox, SpinBox::SupportFrameless, option, widget);

                // SpinBox without a frame, set the corresponding layout values to 0, reduce button width.
                if (supportFrameless && !sb->frame)
                {
                    bw = bw - bmr; // reduce button with as the right button margin will be ignored.
                    fw = 0;
                    bmt = bmb = bmr = 0;
                }

                const int buttonsWidth = bw-bml-bmr;
                const int buttonsLeft = r.right()-bw+bml+1;

                // compute the height of each button...
                int availableButtonHeight = r.height()-bmt-bmb - bs;
                if (symmButtons)
                {
                    // make sure the availableButtonHeight is even by reducing the
                    // button spacing by 1 if necessary. Results in both buttons
                    // of the same height...
                    if (availableButtonHeight%2 != 0)
                    {
                        --bs;

                        // recalculate...
                        availableButtonHeight = r.height()-bmt-bmb - bs;
                    }
                }
                int heightUp = availableButtonHeight / 2;
                int heightDown = availableButtonHeight - heightUp;


                switch (subControl) {
                    case SC_SpinBoxUp:
                        return handleRTL(option,
                                         QRect(buttonsLeft, r.top()+bmt, buttonsWidth, heightUp) );
                    case SC_SpinBoxDown:
                        return handleRTL(option,
                                         QRect(buttonsLeft, r.bottom()-bmb-heightDown+1, buttonsWidth, heightDown) );
                    case SC_SpinBoxEditField:
                    {
                        QRect labelRect(r.left()+fw, r.top()+fw, r.width()-fw-bw, r.height()-2*fw);
                        labelRect = insideMargin(labelRect, WT_SpinBox, SpinBox::ContentsMargin, option, widget);
                        return handleRTL(option, labelRect );
                    }
                    case SC_SpinBoxFrame:
                        return (sb->frame || !supportFrameless) ? r : QRect();
                    default:
                        break;
                }
            } //option ok
        } //CC_SpinBox

        case CC_ComboBox:
        {
            if (const QStyleOptionComboBox *cb = qstyleoption_cast<const QStyleOptionComboBox *>(option)) {

                int fw = widgetLayoutProp(WT_ComboBox, ComboBox::FrameWidth, option, widget);
                int bw = widgetLayoutProp(WT_ComboBox, ComboBox::ButtonWidth, option, widget);
                int bm = widgetLayoutProp(WT_ComboBox, ComboBox::ButtonMargin, option, widget);
                int bml = bm + widgetLayoutProp(WT_ComboBox, ComboBox::ButtonMargin + Left, option, widget);
                int bmr = bm + widgetLayoutProp(WT_ComboBox, ComboBox::ButtonMargin + Right, option, widget);
                int bmt = bm + widgetLayoutProp(WT_ComboBox, ComboBox::ButtonMargin + Top, option, widget);
                int bmb = bm + widgetLayoutProp(WT_ComboBox, ComboBox::ButtonMargin + Bot, option, widget);
                bool supportFrameless = widgetLayoutProp(WT_ComboBox, ComboBox::SupportFrameless, option, widget);

                // ComboBox without a frame, set the corresponding layout values to 0, reduce button width.
                if (supportFrameless && !cb->frame)
                {
                    bw = bw - bmr; // reduce button with as the right button margin will be ignored.
                    fw = 0;
                    bmt = bmb = bmr = 0;
                }

                switch (subControl) {
                    case SC_ComboBoxFrame:
                        return (cb->frame || !supportFrameless) ? r : QRect();
                    case SC_ComboBoxArrow:
                        return handleRTL(option,
                                         QRect(r.right()-bw+bml+1, r.top()+bmt, bw-bml-bmr, r.height()-bmt-bmb) );
                    case SC_ComboBoxEditField:
                    {
                        QRect labelRect(r.left()+fw, r.top()+fw, r.width()-fw-bw, r.height()-2*fw);
                        labelRect = insideMargin(labelRect, WT_ComboBox, ComboBox::ContentsMargin, option, widget);
                        return handleRTL(option, labelRect );
                    }
                    case SC_ComboBoxListBoxPopup:
                        // TODO: need to add layoutProps to control the popup rect?
//                         return cb->popupRect;
                        // popupRect seems to be empty, so use QStyleOption::rect as Qt's styles do
                        return r;
                    default:
                        break;
                }
            } //option ok
        } //CC_ComboBox

        case CC_TitleBar:
        {
            const QStyleOptionTitleBar *tbOpt =
                    qstyleoption_cast<const QStyleOptionTitleBar *>(option);
            if (!tbOpt)
                break;

            QRect ret = insideMargin(r, WT_Window, Window::TitleMargin, option, widget);

            const int btnHeight = ret.height();
            const int btnWidth = widgetLayoutProp(WT_Window, Window::ButtonWidth, option, widget);
            const int btnSpace = widgetLayoutProp(WT_Window, Window::ButtonSpace, option, widget);
            const int titleSpace = widgetLayoutProp(WT_Window, Window::ButtonToTextSpace, option, widget);

            bool isMinimized = tbOpt->titleBarState & Qt::WindowMinimized;
            bool isMaximized = tbOpt->titleBarState & Qt::WindowMaximized;

            // button layout:  menu -title- help,shade,min,max,close

            bool menuCloseBtn = tbOpt->titleBarFlags & Qt::WindowSystemMenuHint;
            bool minBtn = !isMinimized &&
                    (tbOpt->titleBarFlags & Qt::WindowMinimizeButtonHint);
            bool maxBtn = !isMaximized &&
                    (tbOpt->titleBarFlags & Qt::WindowMaximizeButtonHint);
            bool restoreBtn =
                    (isMinimized && (tbOpt->titleBarFlags & Qt::WindowMinimizeButtonHint)) ||
                    (isMaximized && (tbOpt->titleBarFlags & Qt::WindowMaximizeButtonHint));
            bool shadeBtn = tbOpt->titleBarFlags & Qt::WindowShadeButtonHint;
            bool helpBtn = tbOpt->titleBarFlags & Qt::WindowContextHelpButtonHint;


            int btnOffsetCount = 0; // for button rects; count the position in the button bar

            switch (subControl) {
                case SC_TitleBarLabel:
                {
                    if (tbOpt->titleBarFlags & Qt::WindowTitleHint)
                    {
                        int cLeft = 0; // count buttons in the button bar
                        int cRight = 0;

                        if (menuCloseBtn) {
                            // menu and close button
                            ++cLeft;
                            ++cRight;
                        }
                        if (minBtn)     ++cRight;
                        if (restoreBtn) ++cRight;
                        if (maxBtn)     ++cRight;
                        if (shadeBtn)   ++cRight;
                        if (helpBtn)    ++cRight;

                        ret.adjust( cLeft*btnWidth+(cLeft-1)*btnSpace+titleSpace, 0,
                                    -(titleSpace+cRight*btnWidth+(cRight-1)*btnSpace), 0 );
                    }
                    break;
                }

                case SC_TitleBarSysMenu:
                {
                    if (tbOpt->titleBarFlags & Qt::WindowSystemMenuHint) {
                        ret.setRect(ret.left(), ret.top(), btnWidth, btnHeight);
                    }
                    break;
                }

                case SC_TitleBarContextHelpButton:
                    if (helpBtn)
                        ++btnOffsetCount;
                case SC_TitleBarMinButton:
                    if (minBtn)
                        ++btnOffsetCount;
                    else if (subControl == SC_TitleBarMinButton)
                        return QRect();
                case SC_TitleBarNormalButton:
                    if (restoreBtn)
                        ++btnOffsetCount;
                    else if (subControl == SC_TitleBarNormalButton)
                        return QRect();
                case SC_TitleBarMaxButton:
                    if (maxBtn)
                        ++btnOffsetCount;
                    else if (subControl == SC_TitleBarMaxButton)
                        return QRect();
                case SC_TitleBarShadeButton:
                    if (!isMinimized && shadeBtn)
                        ++btnOffsetCount;
                    else if (subControl == SC_TitleBarShadeButton)
                        return QRect();
                case SC_TitleBarUnshadeButton:
                    if (isMinimized && shadeBtn)
                        ++btnOffsetCount;
                    else if (subControl == SC_TitleBarUnshadeButton)
                        return QRect();
                case SC_TitleBarCloseButton:
                {
                    if (menuCloseBtn)
                        ++btnOffsetCount;
                    else if (subControl == SC_TitleBarCloseButton)
                        return QRect();
                    // set the rect for all buttons that fell through:
                    ret.setRect(ret.right()-btnOffsetCount*btnWidth-(btnOffsetCount-1)*btnSpace,
                                ret.top(), btnWidth, btnHeight);
                    break;
                }

                default:
                    return QRect();
            }

            return visualRect(tbOpt->direction, tbOpt->rect, ret);

        } // CC_TitleBar

        default:
            break;
    }

    return QCommonStyle::subControlRect(control, option, subControl, widget);
}

/*
 Checks whether the point is before the bound rect for
 bound of given orientation
*/
static bool preceeds(const QPoint &pt, const QRect &bound,
                     const QStyleOption* opt)
{
    if (opt->state & QStyle::State_Horizontal)
    {
        //What's earlier depends on RTL or not
        if (opt->direction == Qt::LeftToRight)
            return pt.x() < bound.right();
        else
            return pt.x() > bound.x();
    }
    else
    {
        return pt.y() < bound.y();
    }
}

static QStyle::SubControl buttonPortion(const QRect &totalRect,
                                        const QPoint &pt,
                                        const QStyleOption* opt)
{
   if (opt->state & QStyle::State_Horizontal)
   {
        //What's earlier depends on RTL or not
        if (opt->direction == Qt::LeftToRight)
            return pt.x() < totalRect.center().x() ? QStyle::SC_ScrollBarSubLine : QStyle::SC_ScrollBarAddLine;
        else
            return pt.x() > totalRect.center().x() ? QStyle::SC_ScrollBarSubLine : QStyle::SC_ScrollBarAddLine;
    }
    else
    {
        return pt.y() < totalRect.center().y() ? QStyle::SC_ScrollBarSubLine : QStyle::SC_ScrollBarAddLine;
    }
}

QStyle::SubControl KStyle::hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex* opt,
                                             const QPoint& pt, const QWidget* w) const
{
    if (cc == CC_ScrollBar)
    {
        //First, check whether we're inside the groove or not...
        QRect groove = subControlRect(CC_ScrollBar, opt, SC_ScrollBarGroove, w);

        if (groove.contains(pt))
        {
            //Must be either page up/page down, or just click on the slider.
            //Grab the slider to compare
            QRect slider = subControlRect(CC_ScrollBar, opt, SC_ScrollBarSlider, w);

            if (slider.contains(pt))
                return SC_ScrollBarSlider;
            else if (preceeds(pt, slider, opt))
                return SC_ScrollBarSubPage;
            else
                return SC_ScrollBarAddPage;
        }
        else
        {
            //This is one of the up/down buttons. First, decide which one it is.
            if (preceeds(pt, groove, opt))
            {
                //"Upper" button
                if (widgetLayoutProp(WT_ScrollBar, ScrollBar::DoubleTopButton, 0, w))
                {
                    QRect buttonRect = internalSubControlRect(CC_ScrollBar, opt, SC_ScrollBarSubLine, w);
                    return buttonPortion(buttonRect, pt, opt);
                }
                else
                    return SC_ScrollBarSubLine; //Easy one!
            }
            else
            {
                //"Bottom" button
                if (widgetLayoutProp(WT_ScrollBar, ScrollBar::DoubleBotButton, 0, w))
                {
                    QRect buttonRect = internalSubControlRect(CC_ScrollBar, opt, SC_ScrollBarAddLine, w);
                    return buttonPortion(buttonRect, pt, opt);
                }
                else
                    return SC_ScrollBarAddLine; //Easy one!
            }
        }
    }

    return QCommonStyle::hitTestComplexControl(cc, opt, pt, w);
}


QSize KStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& contentsSize, const QWidget* widget) const
{
    switch (type)
    {
        case CT_PushButton:
        {
            const QStyleOptionButton* bOpt = qstyleoption_cast<const QStyleOptionButton*>(option);
            if (!bOpt) return contentsSize;

            QSize size = contentsSize;

            if ((bOpt->features & QStyleOptionButton::DefaultButton) || (bOpt->features & QStyleOptionButton::AutoDefaultButton))
                size = expandDim(size, WT_PushButton, PushButton::DefaultIndicatorMargin, option, widget);

            //### TODO: Handle minimum size limits, extra spacing as in current styles ??
            size = expandDim(size, WT_PushButton, PushButton::ContentsMargin, option, widget);

            if (bOpt->features & QStyleOptionButton::HasMenu) {
                size.setWidth(size.width() + widgetLayoutProp(WT_PushButton, PushButton::TextToIconSpace, option, widget));
            }

            if (!bOpt->text.isEmpty() && !bOpt->icon.isNull()) {
                // Incorporate the spacing between the icon and text. Qt sticks 4 there,
                // but we use PushButton::TextToIconSpace.
                size.setWidth(size.width() - 4 + widgetLayoutProp(WT_PushButton, PushButton::TextToIconSpace, option, widget));
            }
            return size;
        }

        case CT_ToolButton:
        {
            // We want to avoid super-skiny buttons, for things like "up" when icons + text
            // For this, we would like to make width >= height.
            // However, once we get here, QToolButton may have already put in the menu area
            // (PM_MenuButtonIndicator) into the width. So we may have to take it out, fix things
            // up, and add it back in. So much for class-independent rendering...
            QSize size = contentsSize;
            int   menuAreaWidth = 0;
            if (const QStyleOptionToolButton* tbOpt = qstyleoption_cast<const QStyleOptionToolButton*>(option)) {
                if (tbOpt->features & QStyleOptionToolButton::MenuButtonPopup)
                    menuAreaWidth = pixelMetric(QStyle::PM_MenuButtonIndicator, option, widget);
                else if (tbOpt->features & QStyleOptionToolButton::HasMenu)
                    size.setWidth(size.width() + widgetLayoutProp(WT_ToolButton, ToolButton::InlineMenuIndicatorSize, tbOpt, widget));
            }

            size.setWidth(size.width() - menuAreaWidth);
            if (size.width() < size.height())
                size.setWidth(size.height());
            size.setWidth(size.width() + menuAreaWidth);

            return expandDim(size, WT_ToolButton, ToolButton::ContentsMargin, option, widget);
        }

        case CT_CheckBox:
        {
            //Add size for indicator ### handle empty case differently?
            int indicator = widgetLayoutProp(WT_CheckBox, CheckBox::Size, option, widget);
            int spacer    = widgetLayoutProp(WT_CheckBox, CheckBox::BoxTextSpace, option, widget);

            //Make sure we include space for the focus rect margin
            QSize size = expandDim(contentsSize, WT_CheckBox, CheckBox::FocusMargin, option, widget);

            //Make sure we can fit the indicator (### an extra margin around that?)
            size.setHeight(qMax(size.height(), indicator));

            //Add space for the indicator and the icon
            size.setWidth(size.width() + indicator + spacer);

            return size;
        }

        case CT_RadioButton:
        {
            //Add size for indicator
            int indicator = widgetLayoutProp(WT_RadioButton, RadioButton::Size, option, widget);
            int spacer    = widgetLayoutProp(WT_RadioButton, RadioButton::BoxTextSpace, option, widget);

            //Make sure we include space for the focus rect margin
            QSize size = expandDim(contentsSize, WT_RadioButton, RadioButton::FocusMargin, option, widget);

            //Make sure we can fit the indicator (### an extra margin around that?)
            size.setHeight(qMax(size.height(), indicator));

            //Add space for the indicator and the icon
            size.setWidth(size.width() + indicator + spacer);

            return size;
        }

        case CT_ProgressBar:
        {
            QSize size = contentsSize;

            const QStyleOptionProgressBar* pbOpt = ::qstyleoption_cast<const QStyleOptionProgressBar*>(option);
            if (useSideText(pbOpt))
            {
                //Allocate extra room for side text
                size.setWidth(size.width() + sideTextWidth(pbOpt));
            }

            return size;
        }


        case CT_MenuBar:
        {
            int extraW = widgetLayoutProp(WT_MenuBar, MenuBar::Margin + Right, option, widget) -
                            widgetLayoutProp(WT_MenuBar, MenuBar::Margin + Left, option, widget);

            int extraH = widgetLayoutProp(WT_MenuBar, MenuBar::Margin + Bot, option, widget) -
                            widgetLayoutProp(WT_MenuBar, MenuBar::Margin + Top, option, widget);

            return QSize(contentsSize.width() + extraW, contentsSize.height() + extraH);
        }

        case CT_Menu:
        {
            int extraW = widgetLayoutProp(WT_Menu, Menu::Margin + Right, option, widget) -
                            widgetLayoutProp(WT_Menu, Menu::Margin + Left, option, widget);

            int extraH = widgetLayoutProp(WT_Menu, Menu::Margin + Bot, option, widget) -
                            widgetLayoutProp(WT_Menu, Menu::Margin + Top, option, widget);

            return QSize(contentsSize.width() + extraW, contentsSize.height() + extraH);
        }

        case CT_MenuItem:
        {
            const QStyleOptionMenuItem* miOpt = ::qstyleoption_cast<const QStyleOptionMenuItem*>(option);
            if (!miOpt) return contentsSize; //Someone is asking for trouble..

            //First, we calculate the intrinsic size of the item..
            QSize insideSize;

            switch (miOpt->menuItemType)
            {
                case QStyleOptionMenuItem::Normal:
                case QStyleOptionMenuItem::DefaultItem: //huh?
                case QStyleOptionMenuItem::SubMenu:
                {
                    int iconColW = miOpt->maxIconWidth;
                    iconColW     = qMax(iconColW, widgetLayoutProp(WT_MenuItem, MenuItem::IconWidth, option, widget));

                    int leftColW = iconColW;
                    if (miOpt->menuHasCheckableItems &&
                        widgetLayoutProp(WT_MenuItem, MenuItem::CheckAlongsideIcon, option, widget) )
                    {
                        leftColW = widgetLayoutProp(WT_MenuItem, MenuItem::CheckWidth, option, widget) +
                                widgetLayoutProp(WT_MenuItem, MenuItem::CheckSpace, option, widget) +
                                iconColW;
                    }

                    leftColW     += widgetLayoutProp(WT_MenuItem, MenuItem::IconSpace, option, widget);

                    int rightColW = widgetLayoutProp(WT_MenuItem, MenuItem::ArrowSpace, option, widget) +
                                    widgetLayoutProp(WT_MenuItem, MenuItem::ArrowWidth, option, widget);

                    QFontMetrics fm(miOpt->font);

                    int textW;
                    int tabPos = miOpt->text.indexOf(QLatin1Char('\t'));
                    if (tabPos == -1)
                    {
                        //No accel..
                        textW = contentsSize.width();
                    }
                    else
                    {
                        // The width of the accelerator is not included here since
                        // Qt will add that on separately after obtaining the
                        // sizeFromContents() for each menu item in the menu to be shown
                        // ( see QMenuPrivate::calcActionRects() )
                        textW = contentsSize.width() +
                                widgetLayoutProp(WT_MenuItem,MenuItem::AccelSpace,option,widget);
                    }

                    int h = qMax(contentsSize.height(), widgetLayoutProp(WT_MenuItem, MenuItem::MinHeight, option, widget));
                    insideSize = QSize(leftColW + textW + rightColW, h);
                    break;
                }

                case QStyleOptionMenuItem::Separator:
                {
                    insideSize = QSize(10, widgetLayoutProp(WT_MenuItem, MenuItem::SeparatorHeight, option, widget));
                }
                break;


                //Double huh if we get those.
                case QStyleOptionMenuItem::Scroller:
                case QStyleOptionMenuItem::TearOff:
                case QStyleOptionMenuItem::Margin:
                case QStyleOptionMenuItem::EmptyArea:
                    return contentsSize;
            }

            //...now apply the outermost margin.
            return expandDim(insideSize, WT_MenuItem, MenuItem::Margin, option, widget);
        }

        case CT_MenuBarItem:
            return expandDim(contentsSize, WT_MenuBarItem, MenuBarItem::Margin, option, widget);

        case CT_TabBarTab:
        {
            //With our PM_TabBarTabHSpace/VSpace, Qt should give us what we want for
            //contentsSize, so we just expand that. Qt also takes care of
            //the vertical thing.

            bool rotated = false; // indicates whether the tab is rotated by 90 degrees
            if (const QStyleOptionTab *tabOpt = qstyleoption_cast<const QStyleOptionTab*>(option)) {
                rotated = isVerticalTab(tabOpt);
            }

            return expandDim(contentsSize, WT_TabBar, TabBar::TabContentsMargin, option, widget, rotated);
        }

        case CT_TabWidget:
        {
            const QStyleOptionTabWidgetFrame* tabOpt = qstyleoption_cast<const QStyleOptionTabWidgetFrame*>(option);
            if (!tabOpt) break;

            int m = widgetLayoutProp(WT_TabWidget, TabWidget::ContentsMargin, option, widget);
            int vert = 2*m +
                    widgetLayoutProp(WT_TabWidget, TabWidget::ContentsMargin+Top, option, widget) +
                    widgetLayoutProp(WT_TabWidget, TabWidget::ContentsMargin+Bot, option, widget);
            int hor = 2*m +
                    widgetLayoutProp(WT_TabWidget, TabWidget::ContentsMargin+Left, option, widget) +
                    widgetLayoutProp(WT_TabWidget, TabWidget::ContentsMargin+Right, option, widget);

            switch (tabOpt->shape) {
                case QTabBar::RoundedNorth:
                case QTabBar::TriangularNorth:
                case QTabBar::RoundedWest:
                case QTabBar::TriangularWest:
                    return contentsSize + QSize(hor, vert);
                case QTabBar::RoundedSouth:
                case QTabBar::TriangularSouth:
                case QTabBar::RoundedEast:
                case QTabBar::TriangularEast:
                    return contentsSize + QSize(vert,hor);
            }
        }

        case CT_HeaderSection:
        {
            if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option)) {
                QSize iconSize = header->icon.isNull() ? QSize(0,0) : QSize(22,22);
                QSize textSize = header->fontMetrics.size(0, header->text);
                int iconSpacing = widgetLayoutProp(WT_Header, Header::TextToIconSpace, option, widget);
                int w = iconSize.width() + iconSpacing + textSize.width();
                int h = qMax(iconSize.height(), textSize.height() );

                return expandDim(QSize(w, h), WT_Header, Header::ContentsMargin, option, widget);
            }
        }

        case CT_ComboBox:
        {
            // TODO: Figure out what to do with the button margins
            QSize size = contentsSize;

            // Add the contents margin
            size = expandDim(size, WT_ComboBox, ComboBox::ContentsMargin, option, widget);

            // Add the button width
            size.rwidth() += widgetLayoutProp(WT_ComboBox, ComboBox::ButtonWidth, option, widget);

            // Add the frame width
            size.rwidth()  += widgetLayoutProp(WT_ComboBox, ComboBox::FrameWidth, option, widget) * 2;
            size.rheight() += widgetLayoutProp(WT_ComboBox, ComboBox::FrameWidth, option, widget) * 2;

            return size;
        }

        default:
            break;
    }

    return QCommonStyle::sizeFromContents(type, option, contentsSize, widget);
}

bool KStyle::eventFilter(QObject *obj, QEvent *ev)
{
    if (QCommonStyle::eventFilter(obj, ev) )
        return true;

    if (QLabel *lbl = qobject_cast<QLabel*>(obj) ) {
        QWidget *buddy = lbl->buddy();
        if (buddy) {
            switch (ev->type() ) {
                case QEvent::MouseButtonPress:
                {
                    QMouseEvent *mev = dynamic_cast<QMouseEvent*>(ev);
                    if (!mev) break;

                    if (lbl->rect().contains(mev->pos() ) ) {
                        clickedLabel = obj;
                        lbl->repaint();
                    }
                    break;
                }
                case QEvent::MouseButtonRelease:
                {
                    QMouseEvent *mev = dynamic_cast<QMouseEvent*>(ev);
                    if (!mev) break;

                    if (clickedLabel) {
                        clickedLabel = 0;
                        lbl->update();
                    }

                // set focus to the buddy...
                    if (lbl->rect().contains(mev->pos() ) ) {
                        buddy->setFocus(Qt::ShortcutFocusReason);
                    }
                    break;
                }
                case QEvent::Paint:
                    if (obj == clickedLabel && buddy->isEnabled()) {
                    // paint focus rect
                        QPainter p(lbl);
                        QStyleOptionFocusRect foOpts;
                        QRect foRect(0,0,lbl->width(),lbl->height());
                        foOpts.palette = lbl->palette();
                        foOpts.rect    = foRect;
                        drawKStylePrimitive(WT_Generic, Generic::FocusIndicator, &foOpts,
                                            foRect, lbl->palette(), 0, &p, lbl);
                    }
                    break;

                default:
                    break;
            }
        }
    }

    return false;
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
