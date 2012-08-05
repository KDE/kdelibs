/*
 *   Copyright 2010 Marco Martin <mart@kde.org>
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

#include "declarativewidget.h"


#include <QDeclarativeComponent>
#include <QDeclarativeItem>
#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#include <QScriptEngine>
#include <QGraphicsLinearLayout>
#include <QGraphicsScene>
#include <QTimer>

#include <kdebug.h>
#include <kdeclarative.h>


#include "private/declarative/declarativenetworkaccessmanagerfactory_p.h"
#include "private/declarative/dataenginebindings_p.h"

namespace Plasma
{

class DeclarativeWidgetPrivate
{
public:
    DeclarativeWidgetPrivate(DeclarativeWidget *parent)
        : q(parent),
          engine(0),
          component(0),
          root(0),
          delay(false)
    {
    }

    ~DeclarativeWidgetPrivate()
    {
    }

    void errorPrint();
    void execute(const QString &fileName);
    void finishExecute();
    void scheduleExecutionEnd();
    void minimumWidthChanged();
    void minimumHeightChanged();


    DeclarativeWidget *q;

    QString qmlPath;
    QDeclarativeEngine* engine;
    QScriptEngine *scriptEngine;
    QDeclarativeComponent* component;
    QObject *root;
    bool delay : 1;
};

void DeclarativeWidgetPrivate::errorPrint()
{
    QString errorStr = "Error loading QML file.\n";
    if(component->isError()){
        QList<QDeclarativeError> errors = component->errors();
        foreach (const QDeclarativeError &error, errors) {
            errorStr += (error.line()>0?QString(QString::number(error.line()) + QLatin1String(": ")):QLatin1String(""))
                + error.description() + '\n';
        }
    }
    kWarning() << component->url().toString() + '\n' + errorStr;
}

void DeclarativeWidgetPrivate::execute(const QString &fileName)
{
    if (fileName.isEmpty()) {
#ifndef NDEBUG
        kDebug() << "File name empty!";
#endif
        return;
    }

    KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(engine);
    kdeclarative.initialize();
    //binds things like kconfig and icons
    kdeclarative.setupBindings();

    component->loadUrl(QUrl::fromLocalFile(fileName));

    scriptEngine = kdeclarative.scriptEngine();
    registerDataEngineMetaTypes(scriptEngine);

    if (delay) {
        QTimer::singleShot(0, q, SLOT(scheduleExecutionEnd()));
    } else {
        scheduleExecutionEnd();
    }
}

void DeclarativeWidgetPrivate::scheduleExecutionEnd()
{
    if (component->isReady() || component->isError()) {
        finishExecute();
    } else {
        QObject::connect(component, SIGNAL(statusChanged(QDeclarativeComponent::Status)), q, SLOT(finishExecute()));
    }
}

void DeclarativeWidgetPrivate::finishExecute()
{
    if (component->isError()) {
        errorPrint();
    }

    root = component->create();

    if (!root) {
        errorPrint();
    }

#ifndef NDEBUG
    kDebug() << "Execution of QML done!";
#endif
    QGraphicsWidget *widget = dynamic_cast<QGraphicsWidget*>(root);
    QGraphicsObject *object = dynamic_cast<QGraphicsObject *>(root);


    if (object) {
        static_cast<QGraphicsItem *>(object)->setParentItem(q);
        if (q->scene()) {
            q->scene()->addItem(object);
        }
    }

    if (widget) {
        q->setPreferredSize(-1,-1);
        QGraphicsLinearLayout *lay = static_cast<QGraphicsLinearLayout *>(q->layout());
        if (!lay) {
            lay = new QGraphicsLinearLayout(q);
            lay->setContentsMargins(0, 0, 0, 0);
        }
        lay->addItem(widget);
    } else {
        q->setLayout(0);
        qreal minimumWidth = 0;
        qreal minimumHeight = 0;
        if (object) {
            minimumWidth = object->property("minimumWidth").toReal();
            minimumHeight = object->property("minimumHeight").toReal();
            object->setProperty("width", q->size().width());
            object->setProperty("height", q->size().height());
            QObject::connect(object, SIGNAL(minimumWidthChanged()), q, SLOT(minimumWidthChanged()));
            QObject::connect(object, SIGNAL(minimumHeightChanged()), q, SLOT(minimumHeightChanged()));
        }

        if (minimumWidth > 0 && minimumHeight > 0) {
            q->setMinimumSize(minimumWidth, minimumHeight);
        } else {
            q->setMinimumSize(-1, -1);
        }
    }
    emit q->finished();
}

void DeclarativeWidgetPrivate::minimumWidthChanged()
{
    qreal minimumWidth = root->property("minimumWidth").toReal();
    q->setMinimumWidth(minimumWidth);
}

void DeclarativeWidgetPrivate::minimumHeightChanged()
{
    qreal minimumHeight = root->property("minimumHeight").toReal();
    q->setMinimumHeight(minimumHeight);
}

DeclarativeWidget::DeclarativeWidget(QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
      d(new DeclarativeWidgetPrivate(this))
{
    setFlag(QGraphicsItem::ItemHasNoContents);

    d->engine = new QDeclarativeEngine(this);
    d->engine->setNetworkAccessManagerFactory(new DeclarativeNetworkAccessManagerFactory);

    d->component = new QDeclarativeComponent(d->engine, this);
}

DeclarativeWidget::~DeclarativeWidget()
{
    QDeclarativeNetworkAccessManagerFactory *factory = d->engine->networkAccessManagerFactory();
    d->engine->setNetworkAccessManagerFactory(0);
    delete factory;
    delete d;
}

void DeclarativeWidget::setQmlPath(const QString &path)
{
    d->qmlPath = path;
    d->execute(path);
}

QString DeclarativeWidget::qmlPath() const
{
    return d->qmlPath;
}

void DeclarativeWidget::setInitializationDelayed(const bool delay)
{
    d->delay = delay;
}

bool DeclarativeWidget::isInitializationDelayed() const
{
    return d->delay;
}

QDeclarativeEngine* DeclarativeWidget::engine()
{
    return d->engine;
}

QScriptEngine *DeclarativeWidget::scriptEngine() const
{
    return d->scriptEngine;
}

QObject *DeclarativeWidget::rootObject() const
{
    return d->root;
}

QDeclarativeComponent *DeclarativeWidget::mainComponent() const
{
    return d->component;
}

void DeclarativeWidget::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    QGraphicsWidget::resizeEvent(event);

    if (d->root) {
        d->root->setProperty("width", size().width());
        d->root->setProperty("height", size().height());
    }
}


} // namespace Plasma




#include "moc_declarativewidget.cpp"
