#include <QDebug>

#include <kjsembed/object_binding.h>
#include <kjsembed/value_binding.h>
#include <kjs/object.h>

#include <bind_qlcdnumber.h>

using namespace KJSEmbed;

namespace LCDNumberNS {

START_QOBJECT_METHOD( checkOverflow, QLCDNumber )
    double arg0 = KJSEmbed::extractDouble( exec, args, 0 );
    bool b = object->checkOverflow( arg0 );
    result = KJS::Boolean( b );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( intValue, QLCDNumber )
    int i = object->intValue();
    result = KJS::Number( i );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( mode, QLCDNumber )
    int i = (int) object->mode();
    result = KJS::Number( i );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( numDigits, QLCDNumber )
    int i = (int) object->numDigits();
    result = KJS::Number( i );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( segmentStyle, QLCDNumber )
    int i = (int) object->segmentStyle();
    result = KJS::Number( i );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( setMode, QLCDNumber )
    int arg0 = KJSEmbed::extractInt( exec, args, 0 );
    object->setMode( (QLCDNumber::Mode) arg0 );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( setNumDigits, QLCDNumber )
    int arg0 = KJSEmbed::extractInt( exec, args, 0 );
    object->setNumDigits( arg0 );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( setSegmentStyle, QLCDNumber )
    int arg0 = KJSEmbed::extractInt( exec, args, 0 );
    object->setSegmentStyle( (QLCDNumber::SegmentStyle) arg0 );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( smallDecimalPoint, QLCDNumber )
    bool b = object->smallDecimalPoint();
    result = KJS::Boolean( b );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( value, QLCDNumber )
    double d = object->value();
    result = KJS::Number( d );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( display, QLCDNumber )
    qDebug() << "reached the custom function";
    // Overload on type
    if ( args[0]->type() == KJS::NumberType ) {
        double d = extractDouble( exec, args, 0 );
        object->display( d );
    }
    else {
        QString s = extractQString( exec, args, 0 );
        object->display( s );
    }
END_QOBJECT_METHOD

};

NO_ENUMS( LCDNumber )

START_METHOD_LUT( LCDNumber )
    {"checkOverflow", 1, KJS::DontDelete|KJS::ReadOnly, &LCDNumberNS::checkOverflow },
    {"intValue", 0, KJS::DontDelete|KJS::ReadOnly, &LCDNumberNS::intValue },
    {"mode", 0, KJS::DontDelete|KJS::ReadOnly, &LCDNumberNS::mode },
    {"numDigits", 0, KJS::DontDelete|KJS::ReadOnly, &LCDNumberNS::numDigits },
    {"segmentStyle", 0, KJS::DontDelete|KJS::ReadOnly, &LCDNumberNS::segmentStyle },
    {"setMode", 1, KJS::DontDelete|KJS::ReadOnly, &LCDNumberNS::setMode },
    {"setNumDigits", 1, KJS::DontDelete|KJS::ReadOnly, &LCDNumberNS::setNumDigits },
    {"setSegmentStyle", 1, KJS::DontDelete|KJS::ReadOnly, &LCDNumberNS::setSegmentStyle },
    {"smallDecimalPoint", 1, KJS::DontDelete|KJS::ReadOnly, &LCDNumberNS::smallDecimalPoint },
    {"value", 0, KJS::DontDelete|KJS::ReadOnly, &LCDNumberNS::value },
    {"display", 1, KJS::DontDelete|KJS::ReadOnly, &LCDNumberNS::display }
END_METHOD_LUT

NO_STATICS( LCDNumber )

START_CTOR( LCDNumber, QLCDNumber, 0 )
    KJSEmbed::QObjectBinding *obj = 0;
    if ( args.size() < 2 )
    {
        QWidget *parent = KJSEmbed::extractObject<QWidget>( exec, args, 0, 0 );
        QLCDNumber *lcd = new QLCDNumber( parent );
        obj = new KJSEmbed::QObjectBinding( exec, lcd );
    }
    else if ( args.size() < 3 )
    {
        uint arg0 = KJSEmbed::extractInt( exec, args, 0 );
        QWidget *parent = KJSEmbed::extractObject<QWidget>( exec, args, 1, 0 );
        QLCDNumber *lcd = new QLCDNumber( 0, parent );
        obj = new KJSEmbed::QObjectBinding( exec, lcd );
    }

    return obj;
END_CTOR


