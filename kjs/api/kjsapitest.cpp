/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2008 Harri Porten (porten@kde.org)
 *
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
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#include "kjsobject.h"
#include "kjsprototype.h"
#include "kjsinterpreter.h"

#include "qtest_kde.h"

class KJSApiTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void objectConstruction();
    void interpreterEvaluate();
    void objectProperties();
    void prototypeConstants();
    void prototypeProperties();
};

void KJSApiTest::objectConstruction()
{
    KJSInterpreter ip;
    KJSContext* ctx = ip.globalContext();

    // invalid
    QVERIFY2(!KJSObject().isValid(), "Default object is valid");

    // undefined
    QVERIFY2(KJSUndefined().isUndefined(),
             "Undefined object is not undefined");
    // null
    QVERIFY2(KJSNull().isNull(),
             "Null object is not null");

    // Boolean
    KJSBoolean boolObject(true);
    QVERIFY2(boolObject.isBoolean(), "Boolean object is not of boolean type");
    QVERIFY2(boolObject.toBoolean(ctx), "Boolean object has wrong value");
    QVERIFY2(!ctx->hasException(), "Boolean conversion threw exception");

    // Number
    KJSNumber numObject(42.0);
    QVERIFY2(numObject.isNumber(), "Number object is not of number type");
    QCOMPARE(numObject.toNumber(ctx), 42.0);
    QVERIFY2(!ctx->hasException(), "Number conversion threw exception");

    // String
    KJSString stringObject("Trunk");
    QVERIFY2(stringObject.isString(), "String object is not of string type");
    QCOMPARE(stringObject.toString(ctx), QLatin1String("Trunk"));
    QVERIFY2(!ctx->hasException(), "String conversion threw exception");

    // copying
    KJSObject copy(stringObject);
    QCOMPARE(copy.toString(ctx), QLatin1String("Trunk"));
    copy = numObject;
    QCOMPARE(copy.toNumber(ctx), 42.0);
}

void KJSApiTest::interpreterEvaluate()
{
    KJSInterpreter ip;
    KJSContext* ctx = ip.globalContext();    
    KJSObject res;

    // syntax error
    res = ip.evaluate(")(");
    QVERIFY2(!res.isValid(), "Syntax error not caught");

    res = ip.evaluate("11+22");
    QVERIFY2(res.isValid(), "Evaluation returned invalid object");
    QVERIFY2(res.isNumber(), "Evaluation returned non-number object");
    QCOMPARE(res.toNumber(ctx), 33.0);
}

void KJSApiTest::objectProperties()
{
   KJSInterpreter ip;
   KJSContext* ctx = ip.globalContext();    
 
   KJSObject global = ip.globalObject();
   KJSObject v;

   // bool
   global.setProperty(ctx, "myprop", true);
   v = global.property(ctx, "myprop");
   QVERIFY(v.isBoolean());
   QCOMPARE(v.toBoolean(ctx), true);

   // double
   global.setProperty(ctx, "myprop", 21.0);
   v = global.property(ctx, "myprop");
   QVERIFY(v.isNumber());
   QCOMPARE(v.toNumber(ctx), 21.0);

   // string (8-bit)
   global.setProperty(ctx, "myprop", "myvalue8");
   v = global.property(ctx, "myprop");
   QVERIFY(v.isString());
   QCOMPARE(v.toString(ctx), QLatin1String("myvalue8"));

   // string (Unicode)
   global.setProperty(ctx, "myprop", QLatin1String("myvalue16"));
   v = global.property(ctx, "myprop");
   QVERIFY(v.isString());
   QCOMPARE(v.toString(ctx), QLatin1String("myvalue16"));
}

void KJSApiTest::prototypeConstants()
{
    KJSInterpreter ip;
    KJSContext* ctx = ip.globalContext();    

    KJSPrototype proto(ip);

    proto.defineConstant(ctx, "d0", 44.4);
    proto.defineConstant(ctx, "s0", QLatin1String("XYZ"));

    KJSObject obj = proto.constructObject(0);

    QCOMPARE(obj.property(ctx, "d0").toNumber(ctx), 44.4);
    QCOMPARE(obj.property(ctx, "s0").toString(ctx), QLatin1String("XYZ"));
}

static struct O { int x; } o0 = { 42 };

static KJSObject getX(KJSContext* /*context*/, void* object)
{
    O* o = reinterpret_cast<O*>(object);
    int x = o->x;
    return KJSNumber(x);
}

static void setX(KJSContext* context, void* object, KJSObject value)
{
    O* o = reinterpret_cast<O*>(object);
    double n = value.toNumber(context);
    o->x = n;
}

void KJSApiTest::prototypeProperties()
{
    KJSInterpreter ip;
    KJSContext* ctx = ip.globalContext();    

    KJSPrototype proto(ip);

    proto.defineProperty(ctx, "x", getX, setX);
    proto.defineProperty(ctx, "readOnlyX", getX);

    KJSObject obj = proto.constructObject(&o0);

    QCOMPARE(int(obj.property(ctx, "x").toNumber(ctx)), 42);
    obj.setProperty(ctx, "x", KJSNumber(44));
    QCOMPARE(int(obj.property(ctx, "x").toNumber(ctx)), 43);
}

QTEST_KDEMAIN_CORE(KJSApiTest)

#include "kjsapitest.moc"
