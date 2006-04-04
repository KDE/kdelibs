<QtGui>

#include <QPoint_bind.h>

using namespace KJSEmbed;

QPointBinding::QPoint( KJS::ExecState *exec, const QPoint &value )
   : ValueBinding(exec, value)
{
   StaticBinding::publish(exec, this, QPoint::methods() );
   StaticBinding::publish(exec, this, ValueFactory::methods() );
}


namespace QPointNS
{

// bool isNull() const 
START_VARIANT_METHOD( isNull, QPoint )
   bool tmp = value.isNull();
   result = KJSEmbed::createValue( exec, "bool", tmp );
END_VARIANT_METHOD

// int x() const 
START_VARIANT_METHOD( x, QPoint )
   int tmp = value.x();
   result = KJSEmbed::createValue( exec, "int", tmp );
END_VARIANT_METHOD

// int y() const 
START_VARIANT_METHOD( y, QPoint )
   int tmp = value.y();
   result = KJSEmbed::createValue( exec, "int", tmp );
END_VARIANT_METHOD

// void setX(int x)
START_VARIANT_METHOD( setX, QPoint )
   int x = KJSEmbed::extractValue<int>(exec, args, 0);
    value.setX(x);
END_VARIANT_METHOD

// void setY(int y)
START_VARIANT_METHOD( setY, QPoint )
   int y = KJSEmbed::extractValue<int>(exec, args, 0);
    value.setY(y);
END_VARIANT_METHOD

// int manhattanLength() const 
START_VARIANT_METHOD( manhattanLength, QPoint )
   int tmp = value.manhattanLength();
   result = KJSEmbed::createValue( exec, "int", tmp );
END_VARIANT_METHOD

// int & rx()
START_VARIANT_METHOD( rx, QPoint )
   int & tmp = value.rx();
   result = KJSEmbed::createValue( exec, "int &", tmp );
END_VARIANT_METHOD

// int & ry()
START_VARIANT_METHOD( ry, QPoint )
   int & tmp = value.ry();
   result = KJSEmbed::createValue( exec, "int &", tmp );
END_VARIANT_METHOD
}

NO_ENUMS( QPoint ) 
NO_STATICS( QPoint ) 



START_CTOR( QPoint, QPoint, 0 )
   if (args.size() == 0 )
   {
       return new KJSEmbed::QPointBinding(exec, QPoint())
   }
   if (args.size() == 2 )
   {
   int xpos = KJSEmbed::extractValue<int>(exec, args, 0);
   int ypos = KJSEmbed::extractValue<int>(exec, args, 0);
       return new KJSEmbed::QPointBinding(exec, QPoint(xpos, ypos))
   }
END_CTOR


START_METHOD_LUT( QPoint )
    { isNull, 0, KJS::DontDelete|KJS::ReadOnly, &QPointNS::isNull },
    { x, 0, KJS::DontDelete|KJS::ReadOnly, &QPointNS::x },
    { y, 0, KJS::DontDelete|KJS::ReadOnly, &QPointNS::y },
    { setX, 1, KJS::DontDelete|KJS::ReadOnly, &QPointNS::setX },
    { setY, 1, KJS::DontDelete|KJS::ReadOnly, &QPointNS::setY },
    { manhattanLength, 0, KJS::DontDelete|KJS::ReadOnly, &QPointNS::manhattanLength },
    { rx, 0, KJS::DontDelete|KJS::ReadOnly, &QPointNS::rx },
    { ry, 0, KJS::DontDelete|KJS::ReadOnly, &QPointNS::ry },
END_METHOD_LUT

