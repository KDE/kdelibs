#include "size.h"

#include <QDebug>
#include <QSize>

using namespace KJSEmbed;

SizeBinding::SizeBinding( KJS::ExecState *exec, const QSize &value )
    : ValueBinding(exec, value )
{
    StaticBinding::publish( exec, this, Size::methods() );
    StaticBinding::publish( exec, this, ValueFactory::methods() );
}

namespace SizeNS
{

START_VARIANT_METHOD( callboundedTo, QSize )
    QSize arg0 = KJSEmbed::extractValue<QSize>(exec,args, 0);
    QSize cppValue = value.boundedTo(arg0);
    result = KJSEmbed::createValue(exec, "QSize", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callexpandedTo, QSize )
    QSize arg0 = KJSEmbed::extractValue<QSize>(exec,args, 0);
    QSize cppValue = value.expandedTo(arg0);
    result = KJSEmbed::createValue(exec, "QSize", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callheight, QSize )
    int cppValue = value.height();
    result = KJS::Number(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callisEmpty, QSize )
    bool cppValue = value.isEmpty();
    result = KJS::Boolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callisNull, QSize )
    bool cppValue = value.isNull();
    result = KJS::Boolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callisValid, QSize )
    bool cppValue = value.isValid();
    result = KJS::Boolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callrheight, QSize )
    int cppValue = value.rheight();
    result = KJS::Number(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callrwidth, QSize )
    int cppValue = value.rwidth();
    result = KJS::Number(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callscale, QSize )
    if( args.size() == 2 )
    {
        QSize arg0 = KJSEmbed::extractValue<QSize>(exec,args, 0);
        Qt::AspectRatioMode arg1 = (Qt::AspectRatioMode)KJSEmbed::extractInt(exec, args, 1);
        value.scale(arg0, arg1);
    }
    else if( args.size() == 3 )
    {
        int arg0 = KJSEmbed::extractInt(exec, args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        Qt::AspectRatioMode arg2 = (Qt::AspectRatioMode)KJSEmbed::extractInt(exec, args, 2);
        value.scale(arg0, arg1, arg2);
    }
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetHeight, QSize )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    value.setHeight(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetWidth, QSize )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    value.setWidth(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( calltranspose, QSize )
    value.transpose();
END_VARIANT_METHOD

START_VARIANT_METHOD( callwidth, QSize )
    int cppValue = value.width();
    result = KJS::Number(cppValue);
END_VARIANT_METHOD

}

START_METHOD_LUT( Size )
    {"boundedTo", 0, KJS::DontDelete|KJS::ReadOnly, &SizeNS::callboundedTo},
    {"expandedTo", 0, KJS::DontDelete|KJS::ReadOnly, &SizeNS::callexpandedTo},
    {"height", 0, KJS::DontDelete|KJS::ReadOnly, &SizeNS::callheight},
    {"isEmpty", 0, KJS::DontDelete|KJS::ReadOnly, &SizeNS::callisEmpty},
    {"isNull", 0, KJS::DontDelete|KJS::ReadOnly, &SizeNS::callisNull},
    {"isValid", 0, KJS::DontDelete|KJS::ReadOnly, &SizeNS::callisValid},
    {"rheight", 0, KJS::DontDelete|KJS::ReadOnly, &SizeNS::callrheight},
    {"rwidth", 0, KJS::DontDelete|KJS::ReadOnly, &SizeNS::callrwidth},
    {"scale", 2, KJS::DontDelete|KJS::ReadOnly, &SizeNS::callscale},
    {"setHeight", 0, KJS::DontDelete|KJS::ReadOnly, &SizeNS::callsetHeight},
    {"setWidth", 0, KJS::DontDelete|KJS::ReadOnly, &SizeNS::callsetWidth},
    {"transpose", 0, KJS::DontDelete|KJS::ReadOnly, &SizeNS::calltranspose},
    {"width", 0, KJS::DontDelete|KJS::ReadOnly, &SizeNS::callwidth}
END_METHOD_LUT

NO_ENUMS( Size )
NO_STATICS( Size )

START_CTOR( Size, QSize, 0)
    if( args.size() == 0 )
    {
        return new KJSEmbed::SizeBinding( exec, QSize() );
    }
    else if( args.size() == 2 )
    {
        return new KJSEmbed::SizeBinding(exec,
                                QSize( KJSEmbed::extractInt( exec, args, 0 ),
                                        KJSEmbed::extractInt( exec, args, 1 )
                                        ) );
    }
    return new KJSEmbed::SizeBinding( exec, QSize() );
END_CTOR

