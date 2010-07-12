/* This file is part of the KDE libraries
   Copyright (C) 2000, 2006 David Faure <faure@kde.org>
   Copyright 2008 Friedrich W. H. Kossebau <kossebau@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kglobalsettings.h"
#include <config.h>

#include <kconfig.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kcharsets.h>
#include <klocale.h>
#include <kprotocolinfo.h>
#include <kcomponentdata.h>
#include <kcolorscheme.h>
#include <kapplication.h>

#include <kstyle.h>

#include <QtGui/QColor>
#include <QtGui/QCursor>
#include <QtGui/QDesktopWidget>
#include <QtCore/QDir>
#include <QtGui/QFont>
#include <QtGui/QFontDatabase>
#include <QtGui/QFontInfo>
#include <QtGui/QKeySequence>
#include <QtGui/QPixmap>
#include <QtGui/QPixmapCache>
#include <QApplication>
#include <QtDBus/QtDBus>
#include <QtGui/QStyleFactory>
#include <QDesktopServices>

// next two needed so we can set their palettes
#include <QtGui/QToolTip>
#include <QtGui/QWhatsThis>

#ifdef Q_WS_WIN
#include <windows.h>
#include <kkernel_win.h>

static QRgb qt_colorref2qrgb(COLORREF col)
{
    return qRgb(GetRValue(col),GetGValue(col),GetBValue(col));
}
#endif
#ifdef Q_WS_X11
#include <X11/Xlib.h>
#ifdef HAVE_XCURSOR
#include <X11/Xcursor/Xcursor.h>
#endif
#include "fixx11h.h"
#include <QX11Info>
#endif

#include <stdlib.h>
#include <kconfiggroup.h>


//static QColor *_buttonBackground = 0;
static KGlobalSettings::GraphicEffects _graphicEffects = KGlobalSettings::NoEffects;

// KDE5: merge this with KGlobalSettings::Private
// also think to make all methods static and not expose an object,
// making KGlobalSettings rather a namespace
class KGlobalSettingsData
{
  public:
    // if adding a new type here also add an entry to DefaultFontData
    enum FontTypes
    {
        GeneralFont = 0,
        FixedFont,
        ToolbarFont,
        MenuFont,
        WindowTitleFont,
        TaskbarFont ,
        SmallestReadableFont,
        FontTypesCount
    };

  public:
    KGlobalSettingsData();
    ~KGlobalSettingsData();

  public:
    static KGlobalSettingsData* self();

  public: // access, is not const due to caching
    QFont font( FontTypes fontType );
    QFont largeFont( const QString& text );
    KGlobalSettings::KMouseSettings& mouseSettings();

  public:
    void dropFontSettingsCache();
    void dropMouseSettingsCache();

  protected:
    QFont* mFonts[FontTypesCount];
    QFont* mLargeFont;
    KGlobalSettings::KMouseSettings* mMouseSettings;
};

KGlobalSettingsData::KGlobalSettingsData()
  : mLargeFont( 0 ),
    mMouseSettings( 0 )
{
    for( int i=0; i<FontTypesCount; ++i )
        mFonts[i] = 0;
}

KGlobalSettingsData::~KGlobalSettingsData()
{
    for( int i=0; i<FontTypesCount; ++i )
        delete mFonts[i];
    delete mLargeFont;

    delete mMouseSettings;
}

K_GLOBAL_STATIC( KGlobalSettingsData, globalSettingsDataSingleton )

inline KGlobalSettingsData* KGlobalSettingsData::self()
{
    return globalSettingsDataSingleton;
}


class KGlobalSettings::Private
{
    public:
        Private(KGlobalSettings *q)
            : q(q), activated(false)
        {
        }

        void _k_slotNotifyChange(int, int);

        void propagateQtSettings();
        void kdisplaySetPalette();
        void kdisplaySetStyle();
        void kdisplaySetFont();
        void applyGUIStyle();

        /**
         * @internal
         *
         * Ensures that cursors are loaded from the theme KDE is configured
         * to use. Note that calling this function doesn't cause existing
         * cursors to be reloaded. Reloading already created cursors is
         * handled by the KCM when a cursor theme is applied.
         *
         * It is not necessary to call this function when KGlobalSettings
         * is initialized.
         */
        void applyCursorTheme();

        /**
         * drop cached values for settings that aren't in any of the previous groups
         */
        static void rereadOtherSettings();

        KGlobalSettings *q;
        bool activated;
};

KGlobalSettings* KGlobalSettings::self()
{
    K_GLOBAL_STATIC(KGlobalSettings, s_self)
    return s_self;
}

KGlobalSettings::KGlobalSettings()
    : QObject(0), d(new Private(this))
{
    QDBusConnection::sessionBus().connect( QString(), "/KGlobalSettings", "org.kde.KGlobalSettings",
                                           "notifyChange", this, SLOT(_k_slotNotifyChange(int,int)) );
}

KGlobalSettings::~KGlobalSettings()
{
    delete d;
}

void KGlobalSettings::activate()
{
    if (!d->activated) {
        d->activated = true;

        d->kdisplaySetStyle(); // implies palette setup
        d->kdisplaySetFont();
        d->propagateQtSettings();
    }
}

int KGlobalSettings::dndEventDelay()
{
    KConfigGroup g( KGlobal::config(), "General" );
    return g.readEntry("StartDragDist", QApplication::startDragDistance());
}

bool KGlobalSettings::singleClick()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readEntry("SingleClick", KDE_DEFAULT_SINGLECLICK );
}

bool KGlobalSettings::smoothScroll()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readEntry("SmoothScroll", KDE_DEFAULT_SMOOTHSCROLL );
}

KGlobalSettings::TearOffHandle KGlobalSettings::insertTearOffHandle()
{
    int tearoff;
    bool effectsenabled;
    KConfigGroup g( KGlobal::config(), "KDE" );
    effectsenabled = g.readEntry( "EffectsEnabled", false);
    tearoff = g.readEntry("InsertTearOffHandle", KDE_DEFAULT_INSERTTEAROFFHANDLES);
    return effectsenabled ? (TearOffHandle) tearoff : Disable;
}

bool KGlobalSettings::changeCursorOverIcon()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readEntry("ChangeCursor", KDE_DEFAULT_CHANGECURSOR);
}

int KGlobalSettings::autoSelectDelay()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readEntry("AutoSelectDelay", KDE_DEFAULT_AUTOSELECTDELAY);
}

KGlobalSettings::Completion KGlobalSettings::completionMode()
{
    int completion;
    KConfigGroup g( KGlobal::config(), "General" );
    completion = g.readEntry("completionMode", -1);
    if ((completion < (int) CompletionNone) ||
        (completion > (int) CompletionPopupAuto))
      {
        completion = (int) CompletionPopup; // Default
      }
  return (Completion) completion;
}

bool KGlobalSettings::showContextMenusOnPress ()
{
    KConfigGroup g(KGlobal::config(), "ContextMenus");
    return g.readEntry("ShowOnPress", true);
}

int KGlobalSettings::contextMenuKey ()
{
    KConfigGroup g(KGlobal::config(), "Shortcuts");
    QString s = g.readEntry ("PopupMenuContext", "Menu");

    // this is a bit of a code duplication with KShortcut,
    // but seeing as that is all in kdeui these days there's little choice.
    // this is faster for what we're really after here anyways
    // (less allocations, only processing the first item always, etc)
    if (s == QLatin1String("none")) {
        return QKeySequence()[0];
    }

    const QStringList shortCuts = s.split(';');

    if (shortCuts.count() < 1) {
        return QKeySequence()[0];
    }

    s = shortCuts.at(0);

    if ( s.startsWith( QLatin1String("default(") ) ) {
        s = s.mid( 8, s.length() - 9 );
    }

    return QKeySequence::fromString(s)[0];
}

// NOTE: keep this in sync with kdebase/workspace/kcontrol/colors/colorscm.cpp
QColor KGlobalSettings::inactiveTitleColor()
{
#ifdef Q_WS_WIN
    return qt_colorref2qrgb(GetSysColor(COLOR_INACTIVECAPTION));
#else
    KConfigGroup g( KGlobal::config(), "WM" );
    return g.readEntry( "inactiveBackground", QColor(224,223,222) );
#endif
}

// NOTE: keep this in sync with kdebase/workspace/kcontrol/colors/colorscm.cpp
QColor KGlobalSettings::inactiveTextColor()
{
#ifdef Q_WS_WIN
    return qt_colorref2qrgb(GetSysColor(COLOR_INACTIVECAPTIONTEXT));
#else
    KConfigGroup g( KGlobal::config(), "WM" );
    return g.readEntry( "inactiveForeground", QColor(75,71,67) );
#endif
}

// NOTE: keep this in sync with kdebase/workspace/kcontrol/colors/colorscm.cpp
QColor KGlobalSettings::activeTitleColor()
{
#ifdef Q_WS_WIN
    return qt_colorref2qrgb(GetSysColor(COLOR_ACTIVECAPTION));
#else
    KConfigGroup g( KGlobal::config(), "WM" );
    return g.readEntry( "activeBackground", QColor(48,174,232));
#endif
}

// NOTE: keep this in sync with kdebase/workspace/kcontrol/colors/colorscm.cpp
QColor KGlobalSettings::activeTextColor()
{
#ifdef Q_WS_WIN
    return qt_colorref2qrgb(GetSysColor(COLOR_CAPTIONTEXT));
#else
    KConfigGroup g( KGlobal::config(), "WM" );
    return g.readEntry( "activeForeground", QColor(255,255,255) );
#endif
}

int KGlobalSettings::contrast()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readEntry( "contrast", 5 );
}

qreal KGlobalSettings::contrastF(const KSharedConfigPtr &config)
{
    if (config) {
        KConfigGroup g( config, "KDE" );
        return 0.1 * g.readEntry( "contrast", 8 );
    }
    return 0.1 * (qreal)contrast();
}

bool KGlobalSettings::shadeSortColumn()
{
    KConfigGroup g( KGlobal::config(), "General" );
    return g.readEntry( "shadeSortColumn", KDE_DEFAULT_SHADE_SORT_COLUMN );
}

bool KGlobalSettings::allowDefaultBackgroundImages()
{
    KConfigGroup g( KGlobal::config(), "General" );
    return g.readEntry( "allowDefaultBackgroundImages", KDE_DEFAULT_ALLOW_DEFAULT_BACKGROUND_IMAGES );
}

struct KFontData
{
    const char* ConfigGroupKey;
    const char* ConfigKey;
    const char* FontName;
    int Size;
    int Weight;
    QFont::StyleHint StyleHint;
};

// NOTE: keep in sync with kdebase/workspace/kcontrol/fonts/fonts.cpp
static const char GeneralId[] =      "General";
static const char DefaultFont[] =    "Sans Serif";
#ifdef Q_WS_MAC
static const char DefaultMacFont[] = "Lucida Grande";
#endif

static const KFontData DefaultFontData[KGlobalSettingsData::FontTypesCount] =
{
#ifdef Q_WS_MAC
    { GeneralId, "font",        DefaultMacFont, 13, -1, QFont::SansSerif },
    { GeneralId, "fixed",       "Monaco",       10, -1, QFont::TypeWriter },
    { GeneralId, "toolBarFont", DefaultMacFont, 11, -1, QFont::SansSerif },
    { GeneralId, "menuFont",    DefaultMacFont, 13, -1, QFont::SansSerif },
#else
    { GeneralId, "font",        DefaultFont, 9, -1, QFont::SansSerif },
    { GeneralId, "fixed",       "Monospace", 9, -1, QFont::TypeWriter },
    { GeneralId, "toolBarFont", DefaultFont,  8, -1, QFont::SansSerif },
    { GeneralId, "menuFont",    DefaultFont, 9, -1, QFont::SansSerif },
#endif
    { "WM",      "activeFont",           DefaultFont,  8, -1, QFont::SansSerif },
    { GeneralId, "taskbarFont",          DefaultFont, 9, -1, QFont::SansSerif },
    { GeneralId, "smallestReadableFont", DefaultFont,  8, -1, QFont::SansSerif }
};

QFont KGlobalSettingsData::font( FontTypes fontType )
{
    QFont* cachedFont = mFonts[fontType];

    if (!cachedFont)
    {
        const KFontData& fontData = DefaultFontData[fontType];
        cachedFont = new QFont( fontData.FontName, fontData.Size, fontData.Weight );
        cachedFont->setStyleHint( fontData.StyleHint );

        const KConfigGroup configGroup( KGlobal::config(), fontData.ConfigGroupKey );
        *cachedFont = configGroup.readEntry( fontData.ConfigKey, *cachedFont );

        mFonts[fontType] = cachedFont;
    }

    return *cachedFont;
}

QFont KGlobalSettings::generalFont()
{
    return KGlobalSettingsData::self()->font( KGlobalSettingsData::GeneralFont );
}
QFont KGlobalSettings::fixedFont()
{
    return KGlobalSettingsData::self()->font( KGlobalSettingsData::FixedFont );
}
QFont KGlobalSettings::toolBarFont()
{
    return KGlobalSettingsData::self()->font( KGlobalSettingsData::ToolbarFont );
}
QFont KGlobalSettings::menuFont()
{
    return KGlobalSettingsData::self()->font( KGlobalSettingsData::MenuFont );
}
QFont KGlobalSettings::windowTitleFont()
{
    return KGlobalSettingsData::self()->font( KGlobalSettingsData::WindowTitleFont );
}
QFont KGlobalSettings::taskbarFont()
{
    return KGlobalSettingsData::self()->font( KGlobalSettingsData::TaskbarFont );
}
QFont KGlobalSettings::smallestReadableFont()
{
    return KGlobalSettingsData::self()->font( KGlobalSettingsData::SmallestReadableFont );
}


QFont KGlobalSettingsData::largeFont( const QString& text )
{
    QFontDatabase db;
    QStringList fam = db.families();

    // Move a bunch of preferred fonts to the front.
    // most preferred last
    static const char* const PreferredFontNames[] =
    {
        "Arial",
        "Sans Serif",
        "Verdana",
        "Tahoma",
        "Lucida Sans",
        "Lucidux Sans",
        "Nimbus Sans",
        "Gothic I"
    };
    static const unsigned int PreferredFontNamesCount = sizeof(PreferredFontNames)/sizeof(const char*);
    for( unsigned int i=0; i<PreferredFontNamesCount; ++i )
    {
        const QString fontName (PreferredFontNames[i]);
        if (fam.removeAll(fontName)>0)
            fam.prepend(fontName);
    }

    if (mLargeFont) {
        fam.prepend(mLargeFont->family());
        delete mLargeFont;
    }

    for(QStringList::ConstIterator it = fam.constBegin();
        it != fam.constEnd(); ++it)
    {
        if (db.isSmoothlyScalable(*it) && !db.isFixedPitch(*it))
        {
            QFont font(*it);
            font.setPixelSize(75);
            QFontMetrics metrics(font);
            int h = metrics.height();
            if ((h < 60) || ( h > 90))
                continue;

            bool ok = true;
            for(int i = 0; i < text.length(); i++)
            {
                if (!metrics.inFont(text[i]))
                {
                    ok = false;
                    break;
                }
            }
            if (!ok)
                continue;

            font.setPointSize(48);
            mLargeFont = new QFont(font);
            return *mLargeFont;
        }
    }
    mLargeFont = new QFont( font(GeneralFont) );
    mLargeFont->setPointSize(48);
    return *mLargeFont;
}
QFont KGlobalSettings::largeFont( const QString& text )
{
    return KGlobalSettingsData::self()->largeFont( text );
}

void KGlobalSettingsData::dropFontSettingsCache()
{
    for( int i=0; i<FontTypesCount; ++i )
    {
        delete mFonts[i];
        mFonts[i] = 0;
    }
    delete mLargeFont;
    mLargeFont = 0;
}

KGlobalSettings::KMouseSettings& KGlobalSettingsData::mouseSettings()
{
    if (!mMouseSettings)
    {
        mMouseSettings = new KGlobalSettings::KMouseSettings;
        KGlobalSettings::KMouseSettings& s = *mMouseSettings; // for convenience

#ifndef Q_WS_WIN
        KConfigGroup g( KGlobal::config(), "Mouse" );
        QString setting = g.readEntry("MouseButtonMapping");
        if (setting == "RightHanded")
            s.handed = KGlobalSettings::KMouseSettings::RightHanded;
        else if (setting == "LeftHanded")
            s.handed = KGlobalSettings::KMouseSettings::LeftHanded;
        else
        {
#ifdef Q_WS_X11
            // get settings from X server
            // This is a simplified version of the code in input/mouse.cpp
            // Keep in sync !
            s.handed = KGlobalSettings::KMouseSettings::RightHanded;
            unsigned char map[20];
            int num_buttons = XGetPointerMapping(QX11Info::display(), map, 20);
            if( num_buttons == 2 )
            {
                if ( (int)map[0] == 1 && (int)map[1] == 2 )
                    s.handed = KGlobalSettings::KMouseSettings::RightHanded;
                else if ( (int)map[0] == 2 && (int)map[1] == 1 )
                    s.handed = KGlobalSettings::KMouseSettings::LeftHanded;
            }
            else if( num_buttons >= 3 )
            {
                if ( (int)map[0] == 1 && (int)map[2] == 3 )
                    s.handed = KGlobalSettings::KMouseSettings::RightHanded;
                else if ( (int)map[0] == 3 && (int)map[2] == 1 )
                    s.handed = KGlobalSettings::KMouseSettings::LeftHanded;
            }
#else
        // FIXME: Implement on other platforms
#endif
        }
#endif //Q_WS_WIN
    }
#ifdef Q_WS_WIN
    //not cached
#ifndef _WIN32_WCE
    mMouseSettings->handed = (GetSystemMetrics(SM_SWAPBUTTON) ?
        KGlobalSettings::KMouseSettings::LeftHanded :
        KGlobalSettings::KMouseSettings::RightHanded);
#else
// There is no mice under wince
    mMouseSettings->handed =KGlobalSettings::KMouseSettings::RightHanded;
#endif
#endif
    return *mMouseSettings;
}
// KDE5: make this a const return?
KGlobalSettings::KMouseSettings & KGlobalSettings::mouseSettings()
{
    return KGlobalSettingsData::self()->mouseSettings();
}

void KGlobalSettingsData::dropMouseSettingsCache()
{
#ifndef Q_WS_WIN
    delete mMouseSettings;
    mMouseSettings = 0;
#endif
}

QString KGlobalSettings::desktopPath()
{
    QString path = QDesktopServices::storageLocation( QDesktopServices::DesktopLocation );
    return path.isEmpty() ? QDir::homePath() : path;
}

// Autostart is not a XDG path, so we have our own code for it.
QString KGlobalSettings::autostartPath()
{
    QString s_autostartPath;
    KConfigGroup g( KGlobal::config(), "Paths" );
    s_autostartPath = KGlobal::dirs()->localkdedir() + "Autostart/";
    s_autostartPath = g.readPathEntry( "Autostart" , s_autostartPath );
    s_autostartPath = QDir::cleanPath( s_autostartPath );
    if ( !s_autostartPath.endsWith( '/' ) ) {
        s_autostartPath.append( QLatin1Char( '/' ) );
    }
    return s_autostartPath;
}

QString KGlobalSettings::documentPath()
{
    QString path = QDesktopServices::storageLocation( QDesktopServices::DocumentsLocation );
    return path.isEmpty() ? QDir::homePath() : path;
}

QString KGlobalSettings::downloadPath()
{
    // Qt 4.4.1 does not have DOWNLOAD, so we based on old code for now
    QString downloadPath = QDir::homePath();
#ifndef Q_WS_WIN
    const QString xdgUserDirs = KGlobal::dirs()->localxdgconfdir() + QLatin1String( "user-dirs.dirs" );
    if( QFile::exists( xdgUserDirs ) ) {
        KConfig xdgUserConf( xdgUserDirs, KConfig::SimpleConfig );
        KConfigGroup g( &xdgUserConf, "" );
        downloadPath  = g.readPathEntry( "XDG_DOWNLOAD_DIR", downloadPath ).remove(  '"' );
        if ( downloadPath.isEmpty() ) {
            downloadPath = QDir::homePath();
        }
    }
#endif
    downloadPath = QDir::cleanPath( downloadPath );
    if ( !downloadPath.endsWith( '/' ) ) {
        downloadPath.append( QLatin1Char(  '/' ) );
    }
    return downloadPath;
}

QString KGlobalSettings::videosPath()
{
    QString path = QDesktopServices::storageLocation( QDesktopServices::MoviesLocation );
    return path.isEmpty() ? QDir::homePath() : path;
}

QString KGlobalSettings::picturesPath()
{
    QString path = QDesktopServices::storageLocation( QDesktopServices::PicturesLocation );
    return path.isEmpty() ? QDir::homePath() :path;
}

QString KGlobalSettings::musicPath()
{
    QString path = QDesktopServices::storageLocation( QDesktopServices::MusicLocation );
    return path.isEmpty() ? QDir::homePath() : path;
}

bool KGlobalSettings::isMultiHead()
{
#ifdef Q_WS_WIN
    return GetSystemMetrics(SM_CMONITORS) > 1;
#else
    QByteArray multiHead = qgetenv("KDE_MULTIHEAD");
    if (!multiHead.isEmpty()) {
        return (multiHead.toLower() == "true");
    }
    return false;
#endif
}

bool KGlobalSettings::wheelMouseZooms()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readEntry( "WheelMouseZooms", KDE_DEFAULT_WHEEL_ZOOM );
}

QRect KGlobalSettings::splashScreenDesktopGeometry()
{
    QDesktopWidget *dw = QApplication::desktop();

    if (dw->isVirtualDesktop()) {
        KConfigGroup group(KGlobal::config(), "Windows");
        int scr = group.readEntry("Unmanaged", -3);
        if (group.readEntry("XineramaEnabled", true) && scr != -2) {
            if (scr == -3)
                scr = dw->screenNumber(QCursor::pos());
            return dw->screenGeometry(scr);
        } else {
            return dw->geometry();
        }
    } else {
        return dw->geometry();
    }
}

QRect KGlobalSettings::desktopGeometry(const QPoint& point)
{
    QDesktopWidget *dw = QApplication::desktop();

    if (dw->isVirtualDesktop()) {
        KConfigGroup group(KGlobal::config(), "Windows");
        if (group.readEntry("XineramaEnabled", true) &&
            group.readEntry("XineramaPlacementEnabled", true)) {
            return dw->screenGeometry(dw->screenNumber(point));
        } else {
            return dw->geometry();
        }
    } else {
        return dw->geometry();
    }
}

QRect KGlobalSettings::desktopGeometry(const QWidget* w)
{
    QDesktopWidget *dw = QApplication::desktop();

    if (dw->isVirtualDesktop()) {
        KConfigGroup group(KGlobal::config(), "Windows");
        if (group.readEntry("XineramaEnabled", true) &&
            group.readEntry("XineramaPlacementEnabled", true)) {
            if (w)
                return dw->screenGeometry(dw->screenNumber(w));
            else return dw->screenGeometry(-1);
        } else {
            return dw->geometry();
        }
    } else {
        return dw->geometry();
    }
}

bool KGlobalSettings::showIconsOnPushButtons()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readEntry("ShowIconsOnPushButtons",
                       KDE_DEFAULT_ICON_ON_PUSHBUTTON);
}

bool KGlobalSettings::naturalSorting()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readEntry("NaturalSorting",
                       KDE_DEFAULT_NATURAL_SORTING);
}

KGlobalSettings::GraphicEffects KGlobalSettings::graphicEffectsLevel()
{
    // This variable stores whether _graphicEffects has the default value because it has not been
    // loaded yet, or if it has been loaded from the user settings or defaults and contains a valid
    // value.
    static bool _graphicEffectsInitialized = false;

    if (!_graphicEffectsInitialized) {
        _graphicEffectsInitialized = true;
        Private::rereadOtherSettings();
    }

    return _graphicEffects;
}

KGlobalSettings::GraphicEffects KGlobalSettings::graphicEffectsLevelDefault()
{
    // For now, let always enable animations by default. The plan is to make
    // this code a bit smarter. (ereslibre)

    return ComplexAnimationEffects;
}

bool KGlobalSettings::showFilePreview(const KUrl &url)
{
    KConfigGroup g(KGlobal::config(), "PreviewSettings");
    QString protocol = url.protocol();
    bool defaultSetting = KProtocolInfo::showFilePreview( protocol );
    return g.readEntry(protocol, defaultSetting );
}

bool KGlobalSettings::opaqueResize()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readEntry("OpaqueResize", KDE_DEFAULT_OPAQUE_RESIZE);
}

int KGlobalSettings::buttonLayout()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readEntry("ButtonLayout", KDE_DEFAULT_BUTTON_LAYOUT);
}

void KGlobalSettings::emitChange(ChangeType changeType, int arg)
{
    QDBusMessage message = QDBusMessage::createSignal("/KGlobalSettings", "org.kde.KGlobalSettings", "notifyChange" );
    QList<QVariant> args;
    args.append(static_cast<int>(changeType));
    args.append(arg);
    message.setArguments(args);
    QDBusConnection::sessionBus().send(message);
#ifdef Q_WS_X11
    if (qApp && qApp->type() != QApplication::Tty) {
        //notify non-kde qt applications of the change
        extern void qt_x11_apply_settings_in_all_apps();
        qt_x11_apply_settings_in_all_apps();
    }
#endif
}

void KGlobalSettings::Private::_k_slotNotifyChange(int changeType, int arg)
{
    switch(changeType) {
    case StyleChanged:
        if (activated) {
            KGlobal::config()->reparseConfiguration();
            kdisplaySetStyle();
        }
        break;

    case ToolbarStyleChanged:
        KGlobal::config()->reparseConfiguration();
        emit q->toolbarAppearanceChanged(arg);
        break;

    case PaletteChanged:
        if (activated) {
            KGlobal::config()->reparseConfiguration();
            kdisplaySetPalette();
        }
        break;

    case FontChanged:
        KGlobal::config()->reparseConfiguration();
        KGlobalSettingsData::self()->dropFontSettingsCache();
        if (activated) {
            kdisplaySetFont();
        }
        break;

    case SettingsChanged: {
        KGlobal::config()->reparseConfiguration();
        rereadOtherSettings();
        SettingsCategory category = static_cast<SettingsCategory>(arg);
        if (category == SETTINGS_MOUSE) {
            KGlobalSettingsData::self()->dropMouseSettingsCache();
        }
        if (category == SETTINGS_QT) {
            if (activated) {
                propagateQtSettings();
            }
        } else {
            emit q->settingsChanged(category);
        }
        break;
    }
    case IconChanged:
        QPixmapCache::clear();
        KGlobal::config()->reparseConfiguration();
        emit q->iconChanged(arg);
        break;

    case CursorChanged:
        applyCursorTheme();
        break;

    case BlockShortcuts:
        // FIXME KAccel port
        //KGlobalAccel::blockShortcuts(arg);
        emit q->blockShortcuts(arg); // see kwin
        break;

    case NaturalSortingChanged:
        emit q->naturalSortingChanged();
        break;

    default:
        kWarning(101) << "Unknown type of change in KGlobalSettings::slotNotifyChange: " << changeType;
    }
}

// Set by KApplication
QString kde_overrideStyle;

void KGlobalSettings::Private::applyGUIStyle()
{
  //Platform plugin only loaded on X11 systems
#ifdef Q_WS_X11
    if (!kde_overrideStyle.isEmpty()) {
        const QLatin1String currentStyleName(qApp->style()->metaObject()->className());
        if (0 != kde_overrideStyle.compare(currentStyleName, Qt::CaseInsensitive) &&
            0 != (QString(kde_overrideStyle + QLatin1String("Style"))).compare(currentStyleName, Qt::CaseInsensitive)) {
            qApp->setStyle(kde_overrideStyle);
        }
    } else {
        emit q->kdisplayStyleChanged();
    }
#else
    const QLatin1String currentStyleName(qApp->style()->metaObject()->className());

    if (kde_overrideStyle.isEmpty()) {
        const QString &defaultStyle = KStyle::defaultStyle();
        const KConfigGroup pConfig(KGlobal::config(), "General");
        const QString &styleStr = pConfig.readEntry("widgetStyle", defaultStyle);

        if (styleStr.isEmpty() ||
                // check whether we already use the correct style to return then
                // (workaround for Qt misbehavior to avoid double style initialization)
                0 == (QString(styleStr + QLatin1String("Style"))).compare(currentStyleName, Qt::CaseInsensitive) ||
                0 == styleStr.compare(currentStyleName, Qt::CaseInsensitive)) {
            return;
        }

        QStyle* sp = QStyleFactory::create( styleStr );
        if (sp && currentStyleName == sp->metaObject()->className()) {
            delete sp;
            return;
        }

        // If there is no default style available, try falling back any available style
        if ( !sp && styleStr != defaultStyle)
            sp = QStyleFactory::create( defaultStyle );
        if ( !sp )
            sp = QStyleFactory::create( QStyleFactory::keys().first() );
        qApp->setStyle(sp);
    } else if (0 != kde_overrideStyle.compare(currentStyleName, Qt::CaseInsensitive) &&
            0 != (QString(kde_overrideStyle + QLatin1String("Style"))).compare(currentStyleName, Qt::CaseInsensitive)) {
        qApp->setStyle(kde_overrideStyle);
    }
    emit q->kdisplayStyleChanged();
#endif //Q_WS_X11
}

QPalette KGlobalSettings::createApplicationPalette(const KSharedConfigPtr &config)
{
    QPalette palette;

    QPalette::ColorGroup states[3] = { QPalette::Active, QPalette::Inactive,
                                       QPalette::Disabled };

    // TT thinks tooltips shouldn't use active, so we use our active colors for all states
    KColorScheme schemeTooltip(QPalette::Active, KColorScheme::Tooltip, config);

    for ( int i = 0; i < 3 ; i++ ) {
        QPalette::ColorGroup state = states[i];
        KColorScheme schemeView(state, KColorScheme::View, config);
        KColorScheme schemeWindow(state, KColorScheme::Window, config);
        KColorScheme schemeButton(state, KColorScheme::Button, config);
        KColorScheme schemeSelection(state, KColorScheme::Selection, config);

        palette.setBrush( state, QPalette::WindowText, schemeWindow.foreground() );
        palette.setBrush( state, QPalette::Window, schemeWindow.background() );
        palette.setBrush( state, QPalette::Base, schemeView.background() );
        palette.setBrush( state, QPalette::Text, schemeView.foreground() );
        palette.setBrush( state, QPalette::Button, schemeButton.background() );
        palette.setBrush( state, QPalette::ButtonText, schemeButton.foreground() );
        palette.setBrush( state, QPalette::Highlight, schemeSelection.background() );
        palette.setBrush( state, QPalette::HighlightedText, schemeSelection.foreground() );
        palette.setBrush( state, QPalette::ToolTipBase, schemeTooltip.background() );
        palette.setBrush( state, QPalette::ToolTipText, schemeTooltip.foreground() );

        palette.setColor( state, QPalette::Light, schemeWindow.shade( KColorScheme::LightShade ) );
        palette.setColor( state, QPalette::Midlight, schemeWindow.shade( KColorScheme::MidlightShade ) );
        palette.setColor( state, QPalette::Mid, schemeWindow.shade( KColorScheme::MidShade ) );
        palette.setColor( state, QPalette::Dark, schemeWindow.shade( KColorScheme::DarkShade ) );
        palette.setColor( state, QPalette::Shadow, schemeWindow.shade( KColorScheme::ShadowShade ) );

        palette.setBrush( state, QPalette::AlternateBase, schemeView.background( KColorScheme::AlternateBackground) );
        palette.setBrush( state, QPalette::Link, schemeView.foreground( KColorScheme::LinkText ) );
        palette.setBrush( state, QPalette::LinkVisited, schemeView.foreground( KColorScheme::VisitedText ) );
    }

    return palette;
}

void KGlobalSettings::Private::kdisplaySetPalette()
{
    // Added by Sam/Harald (TT) for Mac OS X initially, but why?
    KConfigGroup cg( KGlobal::config(), "General" );
    if (cg.readEntry("nopaletteChange", false))
        return;

    if (qApp->type() == QApplication::GuiClient) {
        QApplication::setPalette( q->createApplicationPalette() );
    }
    emit q->kdisplayPaletteChanged();
    emit q->appearanceChanged();
}


void KGlobalSettings::Private::kdisplaySetFont()
{
    if (qApp->type() == QApplication::GuiClient) {
        KGlobalSettingsData* data = KGlobalSettingsData::self();

        QApplication::setFont( data->font(KGlobalSettingsData::GeneralFont) );
        const QFont menuFont = data->font( KGlobalSettingsData::MenuFont );
        QApplication::setFont( menuFont, "QMenuBar" );
        QApplication::setFont( menuFont, "QMenu" );
        QApplication::setFont( menuFont, "KPopupTitle" );
        QApplication::setFont( data->font(KGlobalSettingsData::ToolbarFont), "QToolBar" );
    }
    emit q->kdisplayFontChanged();
    emit q->appearanceChanged();
}


void KGlobalSettings::Private::kdisplaySetStyle()
{
    if (qApp->type() == QApplication::GuiClient) {
        applyGUIStyle();

        // Reread palette from config file.
        kdisplaySetPalette();
    }
}


void KGlobalSettings::Private::rereadOtherSettings()
{
    KConfigGroup g( KGlobal::config(), "KDE-Global GUI Settings" );

    // Asking for hasKey we do not ask for graphicEffectsLevelDefault() that can
    // contain some very slow code. If we can save that time, do it. (ereslibre)

    if (g.hasKey("GraphicEffectsLevel")) {
        _graphicEffects = ((GraphicEffects) g.readEntry("GraphicEffectsLevel", QVariant((int) NoEffects)).toInt());

        return;
    }

    _graphicEffects = KGlobalSettings::graphicEffectsLevelDefault();
}


void KGlobalSettings::Private::applyCursorTheme()
{
#if defined(Q_WS_X11) && defined(HAVE_XCURSOR)
    KConfig config("kcminputrc");
    KConfigGroup g(&config, "Mouse");

    QString theme = g.readEntry("cursorTheme", QString());
    int size      = g.readEntry("cursorSize", -1);

    // Default cursor size is 16 points
    if (size == -1)
    {
        QApplication *app = static_cast<QApplication*>(QApplication::instance());
        size = app->desktop()->screen(0)->logicalDpiY() * 16 / 72;
    }

    // Note that in X11R7.1 and earlier, calling XcursorSetTheme()
    // with a NULL theme would cause Xcursor to use "default", but
    // in 7.2 and later it will cause it to revert to the theme that
    // was configured when the application was started.
    XcursorSetTheme(QX11Info::display(), theme.isNull() ?
                    "default" : QFile::encodeName(theme));
    XcursorSetDefaultSize(QX11Info::display(), size);

    emit q->cursorChanged();
#endif
}


void KGlobalSettings::Private::propagateQtSettings()
{
    KConfigGroup cg( KGlobal::config(), "KDE" );

    int num = cg.readEntry("CursorBlinkRate", QApplication::cursorFlashTime());
    if ((num != 0) && (num < 200))
        num = 200;
    if (num > 2000)
        num = 2000;
    QApplication::setCursorFlashTime(num);
    num = cg.readEntry("DoubleClickInterval", QApplication::doubleClickInterval());
    QApplication::setDoubleClickInterval(num);
    num = cg.readEntry("StartDragTime", QApplication::startDragTime());
    QApplication::setStartDragTime(num);
    num = cg.readEntry("StartDragDist", QApplication::startDragDistance());
    QApplication::setStartDragDistance(num);
    num = cg.readEntry("WheelScrollLines", QApplication::wheelScrollLines());
    QApplication::setWheelScrollLines(num);

    // KDE5: this seems fairly pointless
    emit q->settingsChanged(SETTINGS_QT);
}

#include "kglobalsettings.moc"
