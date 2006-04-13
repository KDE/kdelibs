<QtGui>

#include <QRectF_bind.h>

using namespace KJSEmbed;

const KJS::ClassInfo QRectFBinding::info = { "QRectFBinding", &ValueBinding::info, 0, 0 };
QRectFBinding::QRectF( KJS::ExecState *exec, const QRectF &value )
   : ValueBinding(exec, value)
{
   StaticBinding::publish(exec, this, QRectF::methods() );
   StaticBinding::publish(exec, this, ValueFactory::methods() );
}


namespace QRectFNS
{

// bool isNull() const 
START_VARIANT_METHOD( isNull, QRectF )
   bool tmp = value.isNull();
   result = KJSEmbed::createValue( exec, "bool", tmp );
END_VARIANT_METHOD

// bool isEmpty() const 
START_VARIANT_METHOD( isEmpty, QRectF )
   bool tmp = value.isEmpty();
   result = KJSEmbed::createValue( exec, "bool", tmp );
END_VARIANT_METHOD

// bool isValid() const 
START_VARIANT_METHOD( isValid, QRectF )
   bool tmp = value.isValid();
   result = KJSEmbed::createValue( exec, "bool", tmp );
END_VARIANT_METHOD

// QRectF normalized() const 
START_VARIANT_METHOD( normalized, QRectF )
   QRectF tmp = value.normalized();
   result = KJSEmbed::createValue( exec, "QRectF", tmp );
END_VARIANT_METHOD

// qreal left() const 
START_VARIANT_METHOD( left, QRectF )
   qreal tmp = value.left();
   result = KJSEmbed::createValue( exec, "qreal", tmp );
END_VARIANT_METHOD

// qreal top() const 
START_VARIANT_METHOD( top, QRectF )
   qreal tmp = value.top();
   result = KJSEmbed::createValue( exec, "qreal", tmp );
END_VARIANT_METHOD

// qreal right() const 
START_VARIANT_METHOD( right, QRectF )
   qreal tmp = value.right();
   result = KJSEmbed::createValue( exec, "qreal", tmp );
END_VARIANT_METHOD

// qreal bottom() const 
START_VARIANT_METHOD( bottom, QRectF )
   qreal tmp = value.bottom();
   result = KJSEmbed::createValue( exec, "qreal", tmp );
END_VARIANT_METHOD

// qreal x() const 
START_VARIANT_METHOD( x, QRectF )
   qreal tmp = value.x();
   result = KJSEmbed::createValue( exec, "qreal", tmp );
END_VARIANT_METHOD

// qreal y() const 
START_VARIANT_METHOD( y, QRectF )
   qreal tmp = value.y();
   result = KJSEmbed::createValue( exec, "qreal", tmp );
END_VARIANT_METHOD

// void setLeft(qreal pos)
START_VARIANT_METHOD( setLeft, QRectF )
   qreal pos = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
    value.setLeft(pos);
END_VARIANT_METHOD

// void setTop(qreal pos)
START_VARIANT_METHOD( setTop, QRectF )
   qreal pos = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
    value.setTop(pos);
END_VARIANT_METHOD

// void setRight(qreal pos)
START_VARIANT_METHOD( setRight, QRectF )
   qreal pos = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
    value.setRight(pos);
END_VARIANT_METHOD

// void setBottom(qreal pos)
START_VARIANT_METHOD( setBottom, QRectF )
   qreal pos = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
    value.setBottom(pos);
END_VARIANT_METHOD

// void setX(qreal pos)
START_VARIANT_METHOD( setX, QRectF )
   qreal pos = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
    value.setX(pos);
END_VARIANT_METHOD

// void setY(qreal pos)
START_VARIANT_METHOD( setY, QRectF )
   qreal pos = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
    value.setY(pos);
END_VARIANT_METHOD

// QPointF topLeft() const 
START_VARIANT_METHOD( topLeft, QRectF )
   QPointF tmp = value.topLeft();
   result = KJSEmbed::createValue( exec, "QPointF", tmp );
END_VARIANT_METHOD

// QPointF bottomRight() const 
START_VARIANT_METHOD( bottomRight, QRectF )
   QPointF tmp = value.bottomRight();
   result = KJSEmbed::createValue( exec, "QPointF", tmp );
END_VARIANT_METHOD

// QPointF topRight() const 
START_VARIANT_METHOD( topRight, QRectF )
   QPointF tmp = value.topRight();
   result = KJSEmbed::createValue( exec, "QPointF", tmp );
END_VARIANT_METHOD

// QPointF bottomLeft() const 
START_VARIANT_METHOD( bottomLeft, QRectF )
   QPointF tmp = value.bottomLeft();
   result = KJSEmbed::createValue( exec, "QPointF", tmp );
END_VARIANT_METHOD

// QPointF center() const 
START_VARIANT_METHOD( center, QRectF )
   QPointF tmp = value.center();
   result = KJSEmbed::createValue( exec, "QPointF", tmp );
END_VARIANT_METHOD

// void setTopLeft(const QPointF &p)
START_VARIANT_METHOD( setTopLeft, QRectF )
   const QPointF & p = KJSEmbed::extractValue<const QPointF &>(exec, args, 0);
    value.setTopLeft(p);
END_VARIANT_METHOD

// void setBottomRight(const QPointF &p)
START_VARIANT_METHOD( setBottomRight, QRectF )
   const QPointF & p = KJSEmbed::extractValue<const QPointF &>(exec, args, 0);
    value.setBottomRight(p);
END_VARIANT_METHOD

// void setTopRight(const QPointF &p)
START_VARIANT_METHOD( setTopRight, QRectF )
   const QPointF & p = KJSEmbed::extractValue<const QPointF &>(exec, args, 0);
    value.setTopRight(p);
END_VARIANT_METHOD

// void setBottomLeft(const QPointF &p)
START_VARIANT_METHOD( setBottomLeft, QRectF )
   const QPointF & p = KJSEmbed::extractValue<const QPointF &>(exec, args, 0);
    value.setBottomLeft(p);
END_VARIANT_METHOD

// void moveLeft(qreal pos)
START_VARIANT_METHOD( moveLeft, QRectF )
   qreal pos = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
END_VARIANT_METHOD

// void moveTop(qreal pos)
START_VARIANT_METHOD( moveTop, QRectF )
   qreal pos = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
END_VARIANT_METHOD

// void moveRight(qreal pos)
START_VARIANT_METHOD( moveRight, QRectF )
   qreal pos = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
END_VARIANT_METHOD

// void moveBottom(qreal pos)
START_VARIANT_METHOD( moveBottom, QRectF )
   qreal pos = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
END_VARIANT_METHOD

// void moveTopLeft(const QPointF &p)
START_VARIANT_METHOD( moveTopLeft, QRectF )
   const QPointF & p = KJSEmbed::extractValue<const QPointF &>(exec, args, 0);
END_VARIANT_METHOD

// void moveBottomRight(const QPointF &p)
START_VARIANT_METHOD( moveBottomRight, QRectF )
   const QPointF & p = KJSEmbed::extractValue<const QPointF &>(exec, args, 0);
END_VARIANT_METHOD

// void moveTopRight(const QPointF &p)
START_VARIANT_METHOD( moveTopRight, QRectF )
   const QPointF & p = KJSEmbed::extractValue<const QPointF &>(exec, args, 0);
END_VARIANT_METHOD

// void moveBottomLeft(const QPointF &p)
START_VARIANT_METHOD( moveBottomLeft, QRectF )
   const QPointF & p = KJSEmbed::extractValue<const QPointF &>(exec, args, 0);
END_VARIANT_METHOD

// void moveCenter(const QPointF &p)
START_VARIANT_METHOD( moveCenter, QRectF )
   const QPointF & p = KJSEmbed::extractValue<const QPointF &>(exec, args, 0);
END_VARIANT_METHOD

// void translate(qreal dx, qreal dy)
START_VARIANT_METHOD( translate, QRectF )
   qreal dx = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
   qreal dy = KJSEmbed::extractObject<qreal>(exec, args, 1, 0);
END_VARIANT_METHOD

// void translate(const QPointF &p)
START_VARIANT_METHOD( translate, QRectF )
   const QPointF & p = KJSEmbed::extractValue<const QPointF &>(exec, args, 0);
END_VARIANT_METHOD

// QRectF translated(qreal dx, qreal dy) const 
START_VARIANT_METHOD( translated, QRectF )
   qreal dx = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
   qreal dy = KJSEmbed::extractObject<qreal>(exec, args, 1, 0);
END_VARIANT_METHOD

// QRectF translated(const QPointF &p) const 
START_VARIANT_METHOD( translated, QRectF )
   const QPointF & p = KJSEmbed::extractValue<const QPointF &>(exec, args, 0);
END_VARIANT_METHOD

// void moveTo(qreal x, qreal t)
START_VARIANT_METHOD( moveTo, QRectF )
   qreal x = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
   qreal t = KJSEmbed::extractObject<qreal>(exec, args, 1, 0);
END_VARIANT_METHOD

// void moveTo(const QPointF &p)
START_VARIANT_METHOD( moveTo, QRectF )
   const QPointF & p = KJSEmbed::extractValue<const QPointF &>(exec, args, 0);
END_VARIANT_METHOD

// void setRect(qreal x, qreal y, qreal w, qreal h)
START_VARIANT_METHOD( setRect, QRectF )
   qreal x = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
   qreal y = KJSEmbed::extractObject<qreal>(exec, args, 1, 0);
   qreal w = KJSEmbed::extractObject<qreal>(exec, args, 2, 0);
   qreal h = KJSEmbed::extractObject<qreal>(exec, args, 3, 0);
    value.setRect(h);
END_VARIANT_METHOD

// void getRect(qreal *x, qreal *y, qreal *w, qreal *h) const 
START_VARIANT_METHOD( getRect, QRectF )
   qreal * x = KJSEmbed::extractObject<qreal *>(exec, args, 0, 0);
   qreal * y = KJSEmbed::extractObject<qreal *>(exec, args, 1, 0);
   qreal * w = KJSEmbed::extractObject<qreal *>(exec, args, 2, 0);
   qreal * h = KJSEmbed::extractObject<qreal *>(exec, args, 3, 0);
END_VARIANT_METHOD

// void setCoords(qreal x1, qreal y1, qreal x2, qreal y2)
START_VARIANT_METHOD( setCoords, QRectF )
   qreal x1 = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
   qreal y1 = KJSEmbed::extractObject<qreal>(exec, args, 1, 0);
   qreal x2 = KJSEmbed::extractObject<qreal>(exec, args, 2, 0);
   qreal y2 = KJSEmbed::extractObject<qreal>(exec, args, 3, 0);
    value.setCoords(y2);
END_VARIANT_METHOD

// void getCoords(qreal *x1, qreal *y1, qreal *x2, qreal *y2) const 
START_VARIANT_METHOD( getCoords, QRectF )
   qreal * x1 = KJSEmbed::extractObject<qreal *>(exec, args, 0, 0);
   qreal * y1 = KJSEmbed::extractObject<qreal *>(exec, args, 1, 0);
   qreal * x2 = KJSEmbed::extractObject<qreal *>(exec, args, 2, 0);
   qreal * y2 = KJSEmbed::extractObject<qreal *>(exec, args, 3, 0);
END_VARIANT_METHOD

// void adjust(qreal x1, qreal y1, qreal x2, qreal y2)
START_VARIANT_METHOD( adjust, QRectF )
   qreal x1 = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
   qreal y1 = KJSEmbed::extractObject<qreal>(exec, args, 1, 0);
   qreal x2 = KJSEmbed::extractObject<qreal>(exec, args, 2, 0);
   qreal y2 = KJSEmbed::extractObject<qreal>(exec, args, 3, 0);
END_VARIANT_METHOD

// QRectF adjusted(qreal x1, qreal y1, qreal x2, qreal y2) const 
START_VARIANT_METHOD( adjusted, QRectF )
   qreal x1 = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
   qreal y1 = KJSEmbed::extractObject<qreal>(exec, args, 1, 0);
   qreal x2 = KJSEmbed::extractObject<qreal>(exec, args, 2, 0);
   qreal y2 = KJSEmbed::extractObject<qreal>(exec, args, 3, 0);
END_VARIANT_METHOD

// QSizeF size() const 
START_VARIANT_METHOD( size, QRectF )
   QSizeF tmp = value.size();
   result = KJSEmbed::createValue( exec, "QSizeF", tmp );
END_VARIANT_METHOD

// qreal width() const 
START_VARIANT_METHOD( width, QRectF )
   qreal tmp = value.width();
   result = KJSEmbed::createValue( exec, "qreal", tmp );
END_VARIANT_METHOD

// qreal height() const 
START_VARIANT_METHOD( height, QRectF )
   qreal tmp = value.height();
   result = KJSEmbed::createValue( exec, "qreal", tmp );
END_VARIANT_METHOD

// void setWidth(qreal w)
START_VARIANT_METHOD( setWidth, QRectF )
   qreal w = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
    value.setWidth(w);
END_VARIANT_METHOD

// void setHeight(qreal h)
START_VARIANT_METHOD( setHeight, QRectF )
   qreal h = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
    value.setHeight(h);
END_VARIANT_METHOD

// void setSize(const QSizeF &s)
START_VARIANT_METHOD( setSize, QRectF )
   const QSizeF & s = KJSEmbed::extractValue<const QSizeF &>(exec, args, 0);
    value.setSize(s);
END_VARIANT_METHOD

// bool contains(const QPointF &p) const 
START_VARIANT_METHOD( contains, QRectF )
   const QPointF & p = KJSEmbed::extractValue<const QPointF &>(exec, args, 0);
END_VARIANT_METHOD

// bool contains(qreal x, qreal y) const 
START_VARIANT_METHOD( contains, QRectF )
   qreal x = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
   qreal y = KJSEmbed::extractObject<qreal>(exec, args, 1, 0);
END_VARIANT_METHOD

// bool contains(const QRectF &r) const 
START_VARIANT_METHOD( contains, QRectF )
   const QRectF & r = KJSEmbed::extractValue<const QRectF &>(exec, args, 0);
END_VARIANT_METHOD

// QRectF unite(const QRectF &r) const 
START_VARIANT_METHOD( unite, QRectF )
   const QRectF & r = KJSEmbed::extractValue<const QRectF &>(exec, args, 0);
END_VARIANT_METHOD

// QRectF intersect(const QRectF &r) const 
START_VARIANT_METHOD( intersect, QRectF )
   const QRectF & r = KJSEmbed::extractValue<const QRectF &>(exec, args, 0);
END_VARIANT_METHOD

// bool intersects(const QRectF &r) const 
START_VARIANT_METHOD( intersects, QRectF )
   const QRectF & r = KJSEmbed::extractValue<const QRectF &>(exec, args, 0);
END_VARIANT_METHOD

// QRect toRect() const 
START_VARIANT_METHOD( toRect, QRectF )
   QRect tmp = value.toRect();
   result = KJSEmbed::createValue( exec, "QRect", tmp );
END_VARIANT_METHOD
}

NO_ENUMS( QRectF ) 
NO_STATICS( QRectF ) 



START_CTOR( QRectF, QRectF, 0 )
   if (args.size() == 0 )
   {
       return new KJSEmbed::QRectFBinding(exec, QRectF())
   }
   if (args.size() == 2 )
   {
   const QPointF & topleft = KJSEmbed::extractValue<const QPointF &>(exec, args, 0);
   const QSizeF & size = KJSEmbed::extractValue<const QSizeF &>(exec, args, 0);
       return new KJSEmbed::QRectFBinding(exec, QRectF(topleft, size))
   }
   if (args.size() == 4 )
   {
   qreal left = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
   qreal top = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
   qreal width = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
   qreal height = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
       return new KJSEmbed::QRectFBinding(exec, QRectF(left, top, width, height))
   }
   if (args.size() == 1 )
   {
   const QRect & rect = KJSEmbed::extractValue<const QRect &>(exec, args, 0);
       return new KJSEmbed::QRectFBinding(exec, QRectF(rect))
   }
END_CTOR


START_METHOD_LUT( QRectF )
    { isNull, 0, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::isNull },
    { isEmpty, 0, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::isEmpty },
    { isValid, 0, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::isValid },
    { normalized, 0, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::normalized },
    { left, 0, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::left },
    { top, 0, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::top },
    { right, 0, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::right },
    { bottom, 0, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::bottom },
    { x, 0, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::x },
    { y, 0, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::y },
    { setLeft, 1, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::setLeft },
    { setTop, 1, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::setTop },
    { setRight, 1, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::setRight },
    { setBottom, 1, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::setBottom },
    { setX, 1, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::setX },
    { setY, 1, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::setY },
    { topLeft, 0, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::topLeft },
    { bottomRight, 0, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::bottomRight },
    { topRight, 0, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::topRight },
    { bottomLeft, 0, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::bottomLeft },
    { center, 0, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::center },
    { setTopLeft, 1, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::setTopLeft },
    { setBottomRight, 1, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::setBottomRight },
    { setTopRight, 1, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::setTopRight },
    { setBottomLeft, 1, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::setBottomLeft },
    { moveLeft, 1, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::moveLeft },
    { moveTop, 1, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::moveTop },
    { moveRight, 1, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::moveRight },
    { moveBottom, 1, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::moveBottom },
    { moveTopLeft, 1, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::moveTopLeft },
    { moveBottomRight, 1, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::moveBottomRight },
    { moveTopRight, 1, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::moveTopRight },
    { moveBottomLeft, 1, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::moveBottomLeft },
    { moveCenter, 1, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::moveCenter },
    { translate, 2, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::translate },
    { translate, 1, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::translate },
    { translated, 2, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::translated },
    { translated, 1, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::translated },
    { moveTo, 2, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::moveTo },
    { moveTo, 1, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::moveTo },
    { setRect, 4, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::setRect },
    { getRect, 4, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::getRect },
    { setCoords, 4, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::setCoords },
    { getCoords, 4, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::getCoords },
    { adjust, 4, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::adjust },
    { adjusted, 4, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::adjusted },
    { size, 0, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::size },
    { width, 0, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::width },
    { height, 0, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::height },
    { setWidth, 1, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::setWidth },
    { setHeight, 1, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::setHeight },
    { setSize, 1, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::setSize },
    { contains, 1, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::contains },
    { contains, 2, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::contains },
    { contains, 1, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::contains },
    { unite, 1, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::unite },
    { intersect, 1, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::intersect },
    { intersects, 1, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::intersects },
    { toRect, 0, KJS::DontDelete|KJS::ReadOnly, &QRectFNS::toRect },
END_METHOD_LUT

