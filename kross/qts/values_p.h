/***************************************************************************
 * plugin.cpp
 * This file is part of the KDE project
 * copyright (C)2007 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "plugin.h"

#include "../core/manager.h"
#include "../core/object.h"

#include <QByteArray>
#include <QUrl>
#include <QColor>
#include <QRect>
#include <QPoint>
#include <QSize>
#include <QWidget>
#include <QLayout>
#include <QVBoxLayout>
#include <QUiLoader>
#include <QAbstractItemView>
#include <QAbstractItemModel>
#include <QStringListModel>
#include <QScriptClass>
#include <QScriptContext>
#include <QScriptValueIterator>
#include <QDebug>

#include <kurl.h>
#include <klocale.h>

using namespace Kross;

namespace Kross {

    /**
    * The EcmaObject implements QScriptClass and Kross::Object to provide
    * custom behavior to QtScript objects.
    */
    class EcmaObject : public QScriptClass, public Kross::Object
    {
        public:
            explicit EcmaObject(QScriptEngine* engine, const QScriptValue& object = QScriptValue())
                : QScriptClass(engine), Kross::Object(), m_object(object) {}
            virtual ~EcmaObject() {}
            virtual QString name() const { return "KrossObject"; }
            virtual QScriptValue prototype() const { return m_object; }

            virtual QVariant callMethod(const QString& name, const QVariantList& args = QVariantList())
            {
                QScriptValue function = m_object.property(name);
                if( ! function.isFunction() ) {
                    krosswarning( QString("EcmaScript::callFunction No such function \"%1\"").arg(name) );
                    if( QScriptContext* context = engine()->currentContext() )
                        context->throwError(QScriptContext::ReferenceError, i18n("No such function \"%1\"", name));
                    return QVariant();
                }
                QScriptValueList arguments;
                foreach(const QVariant &v, args)
                    arguments << engine()->toScriptValue(v);
                QScriptValue result = function.call(m_object, arguments);
                return result.toVariant();
            }

            virtual QStringList methodNames()
            {
                QStringList methods;
                QScriptValueIterator it( m_object );
                while( it.hasNext() ) {
                    it.next();
                    if( it.value().isFunction() )
                        methods << it.name();
                }
                return methods;
            }

        private:
            QScriptValue m_object;
    };

    QScriptValue toByteArray(QScriptEngine *e, const QByteArray &ba) {
        return ba.isNull() ? e->nullValue() : e->newVariant(ba);
    }
    void fromByteArray(const QScriptValue &v, QByteArray &ba) {
        ba = v.isNull() ? QByteArray() : v.toString().toUtf8();
    }

    QScriptValue toUrl(QScriptEngine *e, const QUrl &url) {
        return e->newVariant( url.toString() );
    }
    void fromUrl(const QScriptValue &v, QUrl &url) {
        url.setUrl( v.toString() );
    }

    QScriptValue toKUrl(QScriptEngine *e, const KUrl &url) {
        return e->newVariant( url.url() );
    }
    void fromKUrl(const QScriptValue &v, KUrl &url) {
        url.setUrl( v.toString() );
    }

    QScriptValue toColor(QScriptEngine *e, const QColor &c) {
        return c.isValid() ? e->newVariant(c.name()) : e->nullValue();
    }
    void fromColor(const QScriptValue &v, QColor &c) {
        c.setNamedColor( v.isNull() ? QString() : v.toString() );
    }

    QScriptValue toRect(QScriptEngine *e, const QRect &r) {
        return e->toScriptValue( QVariantList() << r.x() << r.y() << r.width() << r.height() );
    }
    void fromRect(const QScriptValue &v, QRect &r) {
        r = v.isArray() ? QRect(v.property(0).toInt32(),v.property(1).toInt32(),v.property(2).toInt32(),v.property(3).toInt32()) : QRect();
    }
    QScriptValue toRectF(QScriptEngine *e, const QRectF &r) {
        return e->toScriptValue( QVariantList() << r.x() << r.y() << r.width() << r.height() );
    }
    void fromRectF(const QScriptValue &v, QRectF &r) {
        r = v.isArray() ? QRectF(v.property(0).toNumber(),v.property(1).toNumber(),v.property(2).toNumber(),v.property(3).toNumber()) : QRectF();
    }

    QScriptValue toPoint(QScriptEngine *e, const QPoint &p) {
        return e->toScriptValue( QVariantList() << p.x() << p.y() );
    }
    void fromPoint(const QScriptValue &v, QPoint &p) {
        p = v.isArray() ? QPoint(v.property(0).toInt32(),v.property(1).toInt32()) : QPoint();
    }
    QScriptValue toPointF(QScriptEngine *e, const QPointF &p) {
        return e->toScriptValue( QVariantList() << p.x() << p.y() );
    }
    void fromPointF(const QScriptValue &v, QPointF &p) {
        p = v.isArray() ? QPointF(v.property(0).toNumber(),v.property(1).toNumber()) : QPointF();
    }

    QScriptValue toSize(QScriptEngine *e, const QSize &s) {
        return e->toScriptValue( QVariantList() << s.width() << s.height() );
    }
    void fromSize(const QScriptValue &v, QSize &s) {
        s = v.isArray() ? QSize(v.property(0).toInt32(),v.property(1).toInt32()) : QSize();
    }
    QScriptValue toSizeF(QScriptEngine *e, const QSizeF &s) {
        return e->toScriptValue( QVariantList() << s.width() << s.height() );
    }
    void fromSizeF(const QScriptValue &v, QSizeF &s) {
        s = v.isArray() ? QSizeF(v.property(0).toNumber(),v.property(1).toNumber()) : QSizeF();
    }

    /*
    QScriptValue toVariantList(QScriptEngine *e, const QVariantList &l) {
        const int len = l.size();
        QScriptValue a = e->newArray(len);
        for(int i = 0; i < len; ++i)
            a.setProperty(i, e->toScriptValue(l[i]));
        return a;
    }
    void fromVariantList(const QScriptValue &v, QVariantList &l) {
        l.clear();
        const int len = v.isArray() ? v.property("length").toInt32() : 0;
        for(int i = 0; i < len; ++i)
            l << v.property(i).toVariant();
    }
    */

    QScriptValue toObjPtr(QScriptEngine *e, const Kross::Object::Ptr &ptr) {
        const EcmaObject* obj = dynamic_cast<const EcmaObject*>( ptr.data() );
        return obj ? obj->prototype() : e->nullValue();
    }
    void fromObjPtr(const QScriptValue &v, Kross::Object::Ptr &ptr) {
        ptr = new EcmaObject(v.engine(), v);
    }

    QScriptValue createWidget(QScriptContext *context, QScriptEngine *engine) {
        const QString widgetname = context->callee().prototype().property("className").toString();
        Q_ASSERT( ! widgetname.isEmpty() );
        QWidget *parent = qscriptvalue_cast<QWidget*>(context->argument(0));
        QUiLoader loader;
        QWidget *widget = loader.createWidget(widgetname, parent);
        if( ! widget )
            return context->throwError(QScriptContext::TypeError, QString("No such QWidget \"%1\"").arg(widgetname));
        if( parent && parent->layout() )
            parent->layout()->addWidget(widget);
        QScriptValue result = engine->newQObject(widget);
        //result.setPrototype(context->callee().prototype());
        return result;
    }

    QScriptValue addWidgetLayout(QScriptContext *c, QScriptEngine *engine) {
        if( QLayout *layout = dynamic_cast<QLayout*>( qscriptvalue_cast<QObject*>(c->thisObject()) ) ) {
            QGridLayout *gridLayout = dynamic_cast<QGridLayout*>(layout);
            QObject *obj = qscriptvalue_cast<QObject*>( c->argument(0) );
            if( QWidget *w = dynamic_cast<QWidget*>(obj) ) {
                if( gridLayout )
                    gridLayout->addWidget(w, c->argument(1).toInt32(), c->argument(2).toInt32(), (Qt::Alignment)c->argument(3).toInt32());
                else
                    layout->addWidget(w);
            }
            else if( QLayout *l = dynamic_cast<QLayout*>( qscriptvalue_cast<QObject*>(c->argument(0)) ) ) {
                if( gridLayout )
                    gridLayout->addLayout(l, c->argument(1).toInt32(), c->argument(2).toInt32(), (Qt::Alignment)c->argument(3).toInt32());
                else if( QBoxLayout *bl = dynamic_cast<QBoxLayout*>(layout) )
                    bl->addLayout(l);
            }
        }
        return engine->nullValue();
    }
    QScriptValue createLayout(QScriptContext *context, QScriptEngine *engine, QLayout *layout) {
        QObject *parent = qscriptvalue_cast<QObject*>(context->argument(0));
        if( QWidget *parentWidget = dynamic_cast<QWidget*>(parent) )
            parentWidget->setLayout(layout);
        else if( QBoxLayout *parentLayout = dynamic_cast<QBoxLayout*>(parent) )
            parentLayout->addLayout(layout);
        QScriptValue obj = engine->newQObject(layout);
        obj.setProperty("addWidget", engine->newFunction(addWidgetLayout));
        obj.setProperty("addLayout", engine->newFunction(addWidgetLayout));
        return obj;
    }
    QScriptValue createVBoxLayout(QScriptContext *context, QScriptEngine *engine) {
        return createLayout(context, engine, new QVBoxLayout());
    }
    QScriptValue createHBoxLayout(QScriptContext *context, QScriptEngine *engine) {
        return createLayout(context, engine, new QHBoxLayout());
    }
    QScriptValue createGridLayout(QScriptContext *context, QScriptEngine *engine) {
        return createLayout(context, engine, new QGridLayout());
    }
    QScriptValue includeFunction(QScriptContext *context, QScriptEngine *engine) {
        if (context->argumentCount() < 1)
            return engine->nullValue();
        return engine->importExtension(context->argument(0).toString());
    }

    /**
    * Initialize some core functionality like common used types we like
    * to use within scripts.
    */
    void initializeCore(QScriptEngine *engine) {
        QScriptValue global = engine->globalObject();

        // compatibility to kjs/kjsembed
        if( ! global.property("println").isValid() )
            global.setProperty("println", global.property("print"));

        // register common used types
        qScriptRegisterMetaType< QByteArray         >(engine, toByteArray,   fromByteArray);
        qScriptRegisterMetaType< QUrl               >(engine, toUrl,         fromUrl);
        qScriptRegisterMetaType< KUrl               >(engine, toKUrl,        fromKUrl);
        qScriptRegisterMetaType< QColor             >(engine, toColor,       fromColor);
        qScriptRegisterMetaType< QRect              >(engine, toRect,        fromRect);
        qScriptRegisterMetaType< QRectF             >(engine, toRectF,       fromRectF);
        qScriptRegisterMetaType< QPoint             >(engine, toPoint,       fromPoint);
        qScriptRegisterMetaType< QPointF            >(engine, toPointF,      fromPointF);
        qScriptRegisterMetaType< QSize              >(engine, toSize,        fromSize);
        qScriptRegisterMetaType< QSizeF             >(engine, toSizeF,       fromSizeF);

        // we should probably go with an own wrapper for QVariant/QObject...
        //qScriptRegisterMetaType< QVariant           >(engine, toVariant,     fromVariant);
        //qScriptRegisterMetaType< QVariantList       >(engine, toVariantList, fromVariantList);

        // register the Kross::Object::Ptr wrapper
        qScriptRegisterMetaType< Kross::Object::Ptr >(engine, toObjPtr,      fromObjPtr);

        // register the include function that allows to importExtension
        global.setProperty("include", engine->newFunction(includeFunction));
    }

    /**
    * Initialize GUI functionality like widgets the QUiLoader provides
    * and some layout-managers.
    */
    void initializeGui(QScriptEngine *engine) {
        QScriptValue global = engine->globalObject();

        // register UI widgets
        QUiLoader loader;
        foreach(const QString &widgetname, loader.availableWidgets()) {
            QScriptValue proto = engine->newObject();
            proto.setProperty("className", QScriptValue(engine, widgetname));
            QScriptValue func = engine->newFunction(createWidget);
            func.setPrototype(proto);
            global.setProperty(widgetname, func);
        }

        // register layouts
        global.setProperty("QVBoxLayout", engine->newFunction(createVBoxLayout));
        global.setProperty("QHBoxLayout", engine->newFunction(createHBoxLayout));
        global.setProperty("QGridLayout", engine->newFunction(createGridLayout));
    }

}
