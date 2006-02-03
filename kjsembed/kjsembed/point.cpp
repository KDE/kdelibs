#include "point.h"

#include <QDebug>
#include <QPoint>

using namespace KJSEmbed;

PointBinding::PointBinding( KJS::ExecState *exec, const QPoint &value )
    : ValueBinding(exec, value )
{
    StaticBinding::publish( exec, this, Point::methods() );
    StaticBinding::publish( exec, this, ValueFactory::methods() );
}

namespace PointNS
{

START_VARIANT_METHOD( callisNull, QPoint )
    bool cppValue = value.isNull();
    result = KJS::Boolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callmanhattanLength, QPoint )
    int cppValue = value.manhattanLength();
    result = KJS::Number(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callrx, QPoint )
    int cppValue = value.rx();
    result = KJS::Number(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callry, QPoint )
    int cppValue = value.ry();
    result = KJS::Number(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetX, QPoint )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    value.setX(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetY, QPoint )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    value.setY(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callx, QPoint )
    int cppValue = value.x();
    result = KJS::Number(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( cally, QPoint )
    int cppValue = value.y();
    result = KJS::Number(cppValue);
END_VARIANT_METHOD

}

START_METHOD_LUT( Point )
    {"isNull", 0, KJS::DontDelete|KJS::ReadOnly, &PointNS::callisNull},
    {"manhattanLength", 0, KJS::DontDelete|KJS::ReadOnly, &PointNS::callmanhattanLength},
    {"rx", 0, KJS::DontDelete|KJS::ReadOnly, &PointNS::callrx},
    {"ry", 0, KJS::DontDelete|KJS::ReadOnly, &PointNS::callry},
    {"setX", 0, KJS::DontDelete|KJS::ReadOnly, &PointNS::callsetX},
    {"setY", 0, KJS::DontDelete|KJS::ReadOnly, &PointNS::callsetY},
    {"x", 0, KJS::DontDelete|KJS::ReadOnly, &PointNS::callx},
    {"y", 0, KJS::DontDelete|KJS::ReadOnly, &PointNS::cally}
END_METHOD_LUT

NO_ENUMS( Point )
NO_STATICS( Point )

START_CTOR( Point, QPoint, 0 )
    if( args.size() == 2 )
    {
        return new KJSEmbed::PointBinding(exec,
                                QPoint( KJSEmbed::extractInt( exec, args, 0 ),
                                        KJSEmbed::extractInt( exec, args, 1 )
                                        ) );
    }

    return new KJSEmbed::PointBinding( exec, QPoint() );
END_CTOR

