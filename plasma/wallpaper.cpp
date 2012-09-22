/*
 *   Copyright 2008 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 by Petri Damsten <damu@iki.fi>
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

#include "wallpaper.h"

#include "config-plasma.h"

#include <QColor>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QAction>
#include <QQueue>
#include <QTimer>
#include <QRunnable>
#include <QThreadPool>

#include <kdebug.h>
#include <kglobal.h>
#include <kservicetypetrader.h>
#include <kstandarddirs.h>
#include <klocale.h>

#if !PLASMA_NO_KIO
#include <kio/job.h>
#endif

#include <version.h>
#include <qstandardpaths.h>

#include "package.h"
#include "pluginloader.h"
#include "private/dataengineconsumer_p.h"
#include "private/packages_p.h"
#include "private/wallpaper_p.h"

namespace Plasma
{

class SaveImageThread : public QRunnable
{
    QImage m_image;
    QString m_filePath;

    public:
    SaveImageThread(const QImage &image, const QString &filePath)
    {
        m_image = image;
        m_filePath = filePath;
    }

    void run()
    {
        m_image.save(m_filePath);
    }
};

LoadImageThread::LoadImageThread(const QString &filePath)
{
    m_filePath = filePath;
}

void LoadImageThread::run()
{
    QImage image;
    image.load(m_filePath);
    emit done(image);
}

class WallpaperWithPaint : public Wallpaper
{
public:
    WallpaperWithPaint(QObject *parent, const QVariantList &args)
        : Wallpaper(parent, args)
    {
    }

    virtual void paint(QPainter *painter, const QRectF &exposedRect)
    {
        if (d->script) {
            d->script->paint(painter, exposedRect);
        }
    }
};

Wallpaper::Wallpaper(QObject * parentObject)
    : d(new WallpaperPrivate(KService::serviceByStorageId(QString()), this))
{
    setParent(parentObject);
}

Wallpaper::Wallpaper(QObject *parentObject, const QVariantList &args)
    : d(new WallpaperPrivate(KService::serviceByStorageId(args.count() > 0 ?
                             args[0].toString() : QString()), this))
{
    // now remove first item since those are managed by Wallpaper and subclasses shouldn't
    // need to worry about them. yes, it violates the constness of this var, but it lets us add
    // or remove items later while applets can just pretend that their args always start at 0
    QVariantList &mutableArgs = const_cast<QVariantList &>(args);
    if (!mutableArgs.isEmpty()) {
        mutableArgs.removeFirst();
    }

    setParent(parentObject);
}

Wallpaper::~Wallpaper()
{
    delete d;
}

void Wallpaper::addUrls(const QList<QUrl> &urls)
{
    if (d->script) {
        d->script->addUrls(urls);
    }
}

KPluginInfo::List Wallpaper::listWallpaperInfo(const QString &formFactor)
{
    QString constraint;
    if (!formFactor.isEmpty()) {
        constraint.append("[X-Plasma-FormFactors] ~~ '").append(formFactor).append("'");
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Wallpaper", constraint);
    return KPluginInfo::fromServices(offers);
}

KPluginInfo::List Wallpaper::listWallpaperInfoForMimeType(const QString &mimeType, const QString &formFactor)
{
    QString constraint = QString("'%1' in [X-Plasma-DropMimeTypes]").arg(mimeType);
    if (!formFactor.isEmpty()) {
        constraint.append("[X-Plasma-FormFactors] ~~ '").append(formFactor).append("'");
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Wallpaper", constraint);
#ifndef NDEBUG
    kDebug() << offers.count() << constraint;
#endif
    return KPluginInfo::fromServices(offers);
}

bool Wallpaper::supportsMimetype(const QString &mimetype) const
{
    return d->wallpaperDescription.isValid() &&
           d->wallpaperDescription.service()->hasMimeType(mimetype);
}

Wallpaper *Wallpaper::load(const QString &wallpaperName, const QVariantList &args)
{
    if (wallpaperName.isEmpty()) {
        return 0;
    }

    QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(wallpaperName);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Wallpaper", constraint);

    if (offers.isEmpty()) {
#ifndef NDEBUG
        kDebug() << "offers is empty for " << wallpaperName;
#endif
        return 0;
    }

    KService::Ptr offer = offers.first();
    QVariantList allArgs;
    allArgs << offer->storageId() << args;

    if (!offer->property("X-Plasma-API").toString().isEmpty()) {
#ifndef NDEBUG
        kDebug() << "we have a script using the"
                 << offer->property("X-Plasma-API").toString() << "API";
#endif
        return new WallpaperWithPaint(0, allArgs);
    }

    KPluginLoader plugin(*offer);

    if (!Plasma::isPluginVersionCompatible(plugin.pluginVersion())) {
        return 0;
    }

    QString error;
    Wallpaper *wallpaper = offer->createInstance<Plasma::Wallpaper>(0, allArgs, &error);

    if (!wallpaper) {
#ifndef NDEBUG
        kDebug() << "Couldn't load wallpaper \"" << wallpaperName << "\"! reason given: " << error;
#endif
    }

    return wallpaper;
}

Wallpaper *Wallpaper::load(const KPluginInfo &info, const QVariantList &args)
{
    if (!info.isValid()) {
        return 0;
    }
    return load(info.pluginName(), args);
}

QString Wallpaper::name() const
{
    if (!d->wallpaperDescription.isValid()) {
        return i18n("Unknown Wallpaper");
    }

    return d->wallpaperDescription.name();
}

QString Wallpaper::icon() const
{
    if (!d->wallpaperDescription.isValid()) {
        return QString();
    }

    return d->wallpaperDescription.icon();
}

QString Wallpaper::pluginName() const
{
    if (!d->wallpaperDescription.isValid()) {
        return QString();
    }

    return d->wallpaperDescription.pluginName();
}

KServiceAction Wallpaper::renderingMode() const
{
    return d->mode;
}

QList<KServiceAction> Wallpaper::listRenderingModes() const
{
    if (!d->wallpaperDescription.isValid()) {
        return QList<KServiceAction>();
    }

    return d->wallpaperDescription.service()->actions();
}

QRectF Wallpaper::boundingRect() const
{
    return d->boundingRect;
}

bool Wallpaper::isInitialized() const
{
    return d->initialized;
}

void Wallpaper::setBoundingRect(const QRectF &boundingRect)
{
    d->boundingRect = boundingRect;

    if (d->targetSize != boundingRect.size()) {
        d->targetSize = boundingRect.size();
        emit renderHintsChanged();
    }
}

void Wallpaper::setRenderingMode(const QString &mode)
{
    if (d->mode.name() == mode) {
        return;
    }

    d->mode = KServiceAction();
    if (!mode.isEmpty()) {
        QList<KServiceAction> modes = listRenderingModes();

        foreach (const KServiceAction &action, modes) {
            if (action.name() == mode) {
                d->mode = action;
                break;
            }
        }
    }
}

void Wallpaper::restore(const KConfigGroup &config)
{
    init(config);
    d->initialized = true;
}

void Wallpaper::init(const KConfigGroup &config)
{
    if (d->script) {
        d->initScript();
        d->script->initWallpaper(config);
    }
}

void Wallpaper::save(KConfigGroup &config)
{
    if (d->script) {
        d->script->save(config);
    }
}

QWidget *Wallpaper::createConfigurationInterface(QWidget *parent)
{
    if (d->script) {
        return d->script->createConfigurationInterface(parent);
    } else {
        return 0;
    }
}

void Wallpaper::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (d->script) {
        return d->script->mouseMoveEvent(event);
    }
}

void Wallpaper::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (d->script) {
        return d->script->mousePressEvent(event);
    }
}

void Wallpaper::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (d->script) {
        return d->script->mouseReleaseEvent(event);
    }
}

void Wallpaper::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if (d->script) {
        return d->script->wheelEvent(event);
    }
}

DataEngine *Wallpaper::dataEngine(const QString &name) const
{
    return d->dataEngine(name);
}

bool Wallpaper::configurationRequired() const
{
    return d->needsConfig;
}

void Wallpaper::setConfigurationRequired(bool needsConfig, const QString &reason)
{
    //TODO: implement something for reason. first, we need to decide where/how
    //      to communicate it to the user
    Q_UNUSED(reason)

    if (d->needsConfig == needsConfig) {
        return;
    }

    d->needsConfig = needsConfig;
    emit configurationRequired(needsConfig);
}

bool Wallpaper::isUsingRenderingCache() const
{
    return d->cacheRendering;
}

void Wallpaper::setUsingRenderingCache(bool useCache)
{
    d->cacheRendering = useCache;
}

void Wallpaper::setResizeMethodHint(Wallpaper::ResizeMethod resizeMethod)
{
    const ResizeMethod method = qBound(ScaledResize, resizeMethod, LastResizeMethod);
    if (method != d->lastResizeMethod) {
        d->lastResizeMethod = method;
        emit renderHintsChanged();
    }
}

Wallpaper::ResizeMethod Wallpaper::resizeMethodHint() const
{
    return d->lastResizeMethod;
}

void Wallpaper::setTargetSizeHint(const QSizeF &targetSize)
{
    if (targetSize != d->targetSize) {
        d->targetSize = targetSize;
        emit renderHintsChanged();
    }
}

QSizeF Wallpaper::targetSizeHint() const
{
    return d->targetSize;
}

void Wallpaper::render(const QImage &image, const QSize &size,
                       Wallpaper::ResizeMethod resizeMethod, const QColor &color)
{
    if (image.isNull()) {
        return;
    }

    d->renderWallpaper(QString(), image, size, resizeMethod, color);
}

void Wallpaper::render(const QString &sourceImagePath, const QSize &size,
                       Wallpaper::ResizeMethod resizeMethod, const QColor &color)
{
    if (sourceImagePath.isEmpty() || !QFile::exists(sourceImagePath)) {
        //kDebug() << "failed on:" << sourceImagePath;
        return;
    }

    d->renderWallpaper(sourceImagePath, QImage(), size, resizeMethod, color);
}

void WallpaperPrivate::renderWallpaper(const QString &sourceImagePath, const QImage &image, const QSize &size,
                                       Wallpaper::ResizeMethod resizeMethod, const QColor &color)
{
    resizeMethod = qBound(Wallpaper::ScaledResize, resizeMethod, Wallpaper::LastResizeMethod);
    if (lastResizeMethod != resizeMethod) {
        lastResizeMethod = resizeMethod;
        emit q->renderHintsChanged();
    }

    if (cacheRendering) {
        QFileInfo info(sourceImagePath);
        QString cache = cacheKey(sourceImagePath, size, resizeMethod, color);
        if (findInCache(cache, info.lastModified().toTime_t())) {
            return;
        }
    }

    WallpaperRenderRequest request;
    renderToken = request.token;
    request.requester = q;
    request.providedImage = image;
    request.file = sourceImagePath;
    request.size = size;
    request.resizeMethod = resizeMethod;
    request.color = color;
    WallpaperRenderThread::render(request);
    //kDebug() << "rendering" << sourceImagePath << ", token is" << d->renderToken;
}

WallpaperPrivate::WallpaperPrivate(KService::Ptr service, Wallpaper *wallpaper) :
    q(wallpaper),
    wallpaperDescription(service),
    package(0),
    renderToken(-1),
    lastResizeMethod(Wallpaper::ScaledResize),
    script(0),
    cacheRendering(false),
    initialized(false),
    needsConfig(false),
    scriptInitialized(false),
    previewing(false),
    needsPreviewDuringConfiguration(false)
{
    if (wallpaperDescription.isValid()) {
        QString api = wallpaperDescription.property("X-Plasma-API").toString();

        if (!api.isEmpty()) {
            const QString path = KStandardDirs::locate("data",
                    "plasma/wallpapers/" + wallpaperDescription.pluginName() + '/');
            package = new Package(PluginLoader::self()->loadPackage("Plasma/Wallpaper", api));
            package->setPath(path);

            if (package->isValid()) {
                script = Plasma::loadScriptEngine(api, q);
            }

            if (!script) {
#ifndef NDEBUG
                kDebug() << "Could not create a" << api << "ScriptEngine for the"
                         << wallpaperDescription.name() << "Wallpaper.";
#endif
                delete package;
                package = 0;
            }
        }
    }
}

QString WallpaperPrivate::cacheKey(const QString &sourceImagePath, const QSize &size,
                                   int resizeMethod, const QColor &color) const
{
    const QString id = QString("%5_%3_%4_%1x%2")
                              .arg(size.width()).arg(size.height()).arg(color.name())
                              .arg(resizeMethod).arg(sourceImagePath);
    return id;
}

QString WallpaperPrivate::cachePath(const QString &key) const
{
    return QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1Char('/') + "plasma-wallpapers/" + key + ".png";
}

void WallpaperPrivate::newRenderCompleted(const WallpaperRenderRequest &request, const QImage &image)
{
#ifndef NDEBUG
    kDebug() << request.token << renderToken;
#endif
    if (request.token != renderToken) {
        //kDebug() << "render token mismatch" << token << renderToken;
        return;
    }

    if (cacheRendering) {
        q->insertIntoCache(cacheKey(request.file, request.size, request.resizeMethod, request.color), image);
    }

    //kDebug() << "rendering complete!";
    emit q->renderCompleted(image);
}

// put all setup routines for script here. at this point we can assume that
// package exists and that we have a script engine
void WallpaperPrivate::setupScriptSupport()
{
    Q_ASSERT(package);
#ifndef NDEBUG
    kDebug() << "setting up script support, package is in" << package->path()
             << ", main script is" << package->filePath("mainscript");
#endif

    const QString translationsPath = package->filePath("translations");
    if (!translationsPath.isEmpty()) {
        KGlobal::dirs()->addResourceDir("locale", translationsPath);
        KLocale::global()->insertCatalog(wallpaperDescription.pluginName());
    }
}

void WallpaperPrivate::initScript()
{
    if (script && !scriptInitialized) {
        setupScriptSupport();
        script->init();
        scriptInitialized = true;
    }
}

bool WallpaperPrivate::findInCache(const QString &key, unsigned int lastModified)
{
    if (cacheRendering) {
        QString cache = cachePath(key);
        if (QFile::exists(cache)) {
            if (lastModified > 0) {
                QFileInfo info(cache);
                if (info.lastModified().toTime_t() < lastModified) {
                    return false;
                }
            }

            LoadImageThread *loadImageT = new LoadImageThread(cache);
            q->connect(loadImageT, SIGNAL(done(QImage)), q, SIGNAL(renderCompleted(QImage)));
            QThreadPool::globalInstance()->start(loadImageT);

            return true;
        }
    }

    return false;
}

bool Wallpaper::findInCache(const QString &key, QImage &image, unsigned int lastModified)
{
    if (d->cacheRendering) {
        QString cache = d->cachePath(key);
        if (QFile::exists(cache)) {
            if (lastModified > 0) {
                QFileInfo info(cache);
                if (info.lastModified().toTime_t() < lastModified) {
                    return false;
                }
            }

            image.load(cache);
            return true;
        }
    }

    return false;
}

void Wallpaper::insertIntoCache(const QString& key, const QImage &image)
{
    //TODO: cache limits?
    if (key.isEmpty()) {
        return;
    }

    if (d->cacheRendering) {
        if (image.isNull()) {
            QFile::remove(d->cachePath(key));
        } else {
            QThreadPool::globalInstance()->start(new SaveImageThread(image, d->cachePath(key)));
        }
    }
}

QList<QAction*> Wallpaper::contextualActions() const
{
    return d->contextActions;
}

void Wallpaper::setContextualActions(const QList<QAction*> &actions)
{
    d->contextActions = actions;
}

bool Wallpaper::isPreviewing() const
{
    return d->previewing;
}

void Wallpaper::setPreviewing(bool previewing)
{
    d->previewing = previewing;
}

bool Wallpaper::needsPreviewDuringConfiguration() const
{
    return d->needsPreviewDuringConfiguration;
}

void Wallpaper::setPreviewDuringConfiguration(const bool preview)
{
    d->needsPreviewDuringConfiguration = preview;
}

Package Wallpaper::package() const
{
    return d->package ? *d->package : Package();
}

} // Plasma namespace


#include "moc_wallpaper.cpp"
#include "private/moc_wallpaper_p.cpp"
