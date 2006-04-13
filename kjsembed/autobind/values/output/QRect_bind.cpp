<QtGui>

#include <QRect_bind.h>

using namespace KJSEmbed;

const KJS::ClassInfo QRectBinding::info = { "QRectBinding", &ValueBinding::info, 0, 0 };
QRectBinding::QRect( KJS::ExecState *exec, const QRect &value )
   : ValueBinding(exec, value)
{
   StaticBinding::publish(exec, this, QRect::methods() );
   StaticBinding::publish(exec, this, ValueFactory::methods() );
}


namespace QRectNS
{

// bool isNull() const 
START_VARIANT_METHOD( isNull, QRect )
   bool tmp = value.isNull();
   result = KJSEmbed::createValue( exec, "bool", tmp );
END_VARIANT_METHOD

// bool isEmpty() const 
START_VARIANT_METHOD( isEmpty, QRect )
   bool tmp = value.isEmpty();
   result = KJSEmbed::createValue( exec, "bool", tmp );
END_VARIANT_METHOD

// bool isValid() const 
START_VARIANT_METHOD( isValid, QRect )
   bool tmp = value.isValid();
   result = KJSEmbed::createValue( exec, "bool", tmp );
END_VARIANT_METHOD

// int left() const 
START_VARIANT_METHOD( left, QRect )
   int tmp = value.left();
   result = KJSEmbed::createValue( exec, "int", tmp );
END_VARIANT_METHOD

// int top() const 
START_VARIANT_METHOD( top, QRect )
   int tmp = value.top();
   result = KJSEmbed::createValue( exec, "int", tmp );
END_VARIANT_METHOD

// int right() const 
START_VARIANT_METHOD( right, QRect )
   int tmp = value.right();
   result = KJSEmbed::createValue( exec, "int", tmp );
END_VARIANT_METHOD

// int bottom() const 
START_VARIANT_METHOD( bottom, QRect )
   int tmp = value.bottom();
   result = KJSEmbed::createValue( exec, "int", tmp );
END_VARIANT_METHOD

// QRect normalized() const 
START_VARIANT_METHOD( normalized, QRect )
   QRect tmp = value.normalized();
   result = KJSEmbed::createValue( exec, "QRect", tmp );
END_VARIANT_METHOD

// int x() const 
START_VARIANT_METHOD( x, QRect )
   int tmp = value.x();
   result = KJSEmbed::createValue( exec, "int", tmp );
END_VARIANT_METHOD

// int y() const 
START_VARIANT_METHOD( y, QRect )
   int tmp = value.y();
   result = KJSEmbed::createValue( exec, "int", tmp );
END_VARIANT_METHOD

// void setLeft(int pos)
START_VARIANT_METHOD( setLeft, QRect )
   int pos = KJSEmbed::extractValue<int>(exec, args, 0);
    value.setLeft(pos);
END_VARIANT_METHOD

// void setTop(int pos)
START_VARIANT_METHOD( setTop, QRect )
   int pos = KJSEmbed::extractValue<int>(exec, args, 0);
    value.setTop(pos);
END_VARIANT_METHOD

// void setRight(int pos)
START_VARIANT_METHOD( setRight, QRect )
   int pos = KJSEmbed::extractValue<int>(exec, args, 0);
    value.setRight(pos);
END_VARIANT_METHOD

// void setBottom(int pos)
START_VARIANT_METHOD( setBottom, QRect )
   int pos = KJSEmbed::extractValue<int>(exec, args, 0);
    value.setBottom(pos);
END_VARIANT_METHOD

// void setX(int x)
START_VARIANT_METHOD( setX, QRect )
   int x = KJSEmbed::extractValue<int>(exec, args, 0);
    value.setX(x);
END_VARIANT_METHOD

// void setY(int y)
START_VARIANT_METHOD( setY, QRect )
   int y = KJSEmbed::extractValue<int>(exec, args, 0);
    value.setY(y);
END_VARIANT_METHOD

// void setTopLeft(const QPoint &p)
START_VARIANT_METHOD( setTopLeft, QRect )
   const QPoint & p = KJSEmbed::extractValue<const QPoint &>(exec, args, 0);
    value.setTopLeft(p);
END_VARIANT_METHOD

// void setBottomRight(const QPoint &p)
START_VARIANT_METHOD( setBottomRight, QRect )
   const QPoint & p = KJSEmbed::extractValue<const QPoint &>(exec, args, 0);
    value.setBottomRight(p);
END_VARIANT_METHOD

// void setTopRight(const QPoint &p)
START_VARIANT_METHOD( setTopRight, QRect )
   const QPoint & p = KJSEmbed::extractValue<const QPoint &>(exec, args, 0);
    value.setTopRight(p);
END_VARIANT_METHOD

// void setBottomLeft(const QPoint &p)
START_VARIANT_METHOD( setBottomLeft, QRect )
   const QPoint & p = KJSEmbed::extractValue<const QPoint &>(exec, args, 0);
    value.setBottomLeft(p);
END_VARIANT_METHOD

// QPoint topLeft() const 
START_VARIANT_METHOD( topLeft, QRect )
   QPoint tmp = value.topLeft();
   result = KJSEmbed::createValue( exec, "QPoint", tmp );
END_VARIANT_METHOD

// QPoint bottomRight() const 
START_VARIANT_METHOD( bottomRight, QRect )
   QPoint tmp = value.bottomRight();
   result = KJSEmbed::createValue( exec, "QPoint", tmp );
END_VARIANT_METHOD

// QPoint topRight() const 
START_VARIANT_METHOD( topRight, QRect )
   QPoint tmp = value.topRight();
   result = KJSEmbed::createValue( exec, "QPoint", tmp );
END_VARIANT_METHOD

// QPoint bottomLeft() const 
START_VARIANT_METHOD( bottomLeft, QRect )
   QPoint tmp = value.bottomLeft();
   result = KJSEmbed::createValue( exec, "QPoint", tmp );
END_VARIANT_METHOD

// QPoint center() const 
START_VARIANT_METHOD( center, QRect )
   QPoint tmp = value.center();
   result = KJSEmbed::createValue( exec, "QPoint", tmp );
END_VARIANT_METHOD

// void moveLeft(int pos)
START_VARIANT_METHOD( moveLeft, QRect )
   int pos = KJSEmbed::extractValue<int>(exec, args, 0);
END_VARIANT_METHOD

// void moveTop(int pos)
START_VARIANT_METHOD( moveTop, QRect )
   int pos = KJSEmbed::extractValue<int>(exec, args, 0);
END_VARIANT_METHOD

// void moveRight(int pos)
START_VARIANT_METHOD( moveRight, QRect )
   int pos = KJSEmbed::extractValue<int>(exec, args, 0);
END_VARIANT_METHOD

// void moveBottom(int pos)
START_VARIANT_METHOD( moveBottom, QRect )
   int pos = KJSEmbed::extractValue<int>(exec, args, 0);
END_VARIANT_METHOD

// void moveTopLeft(const QPoint &p)
START_VARIANT_METHOD( moveTopLeft, QRect )
   const QPoint & p = KJSEmbed::extractValue<const QPoint &>(exec, args, 0);
END_VARIANT_METHOD

// void moveBottomRight(const QPoint &p)
START_VARIANT_METHOD( moveBottomRight, QRect )
   const QPoint & p = KJSEmbed::extractValue<const QPoint &>(exec, args, 0);
END_VARIANT_METHOD

// void moveTopRight(const QPoint &p)
START_VARIANT_METHOD( moveTopRight, QRect )
   const QPoint & p = KJSEmbed::extractValue<const QPoint &>(exec, args, 0);
END_VARIANT_METHOD

// void moveBottomLeft(const QPoint &p)
START_VARIANT_METHOD( moveBottomLeft, QRect )
   const QPoint & p = KJSEmbed::extractValue<const QPoint &>(exec, args, 0);
END_VARIANT_METHOD

// void moveCenter(const QPoint &p)
START_VARIANT_METHOD( moveCenter, QRect )
   const QPoint & p = KJSEmbed::extractValue<const QPoint &>(exec, args, 0);
END_VARIANT_METHOD

// void translate(int dx, int dy)
START_VARIANT_METHOD( translate, QRect )
   int dx = KJSEmbed::extractValue<int>(exec, args, 0);
   int dy = KJSEmbed::extractValue<int>(exec, args, 0);
END_VARIANT_METHOD

// void translate(const QPoint &p)
START_VARIANT_METHOD( translate, QRect )
   const QPoint & p = KJSEmbed::extractValue<const QPoint &>(exec, args, 0);
END_VARIANT_METHOD

// QRect translated(int dx, int dy) const 
START_VARIANT_METHOD( translated, QRect )
   int dx = KJSEmbed::extractValue<int>(exec, args, 0);
   int dy = KJSEmbed::extractValue<int>(exec, args, 0);
END_VARIANT_METHOD

// QRect translated(const QPoint &p) const 
START_VARIANT_METHOD( translated, QRect )
   const QPoint & p = KJSEmbed::extractValue<const QPoint &>(exec, args, 0);
END_VARIANT_METHOD

// void moveTo(int x, int t)
START_VARIANT_METHOD( moveTo, QRect )
   int x = KJSEmbed::extractValue<int>(exec, args, 0);
   int t = KJSEmbed::extractValue<int>(exec, args, 0);
END_VARIANT_METHOD

// void moveTo(const QPoint &p)
START_VARIANT_METHOD( moveTo, QRect )
   const QPoint & p = KJSEmbed::extractValue<const QPoint &>(exec, args, 0);
END_VARIANT_METHOD

// void setRect(int x, int y, int w, int h)
START_VARIANT_METHOD( setRect, QRect )
   int x = KJSEmbed::extractValue<int>(exec, args, 0);
   int y = KJSEmbed::extractValue<int>(exec, args, 0);
   int w = KJSEmbed::extractValue<int>(exec, args, 0);
   int h = KJSEmbed::extractValue<int>(exec, args, 0);
    value.setRect(h);
END_VARIANT_METHOD

// void getRect(int *x, int *y, int *w, int *h) const 
START_VARIANT_METHOD( getRect, QRect )
   int * x = KJSEmbed::extractValue<int *>(exec, args, 0);
   int * y = KJSEmbed::extractValue<int *>(exec, args, 0);
   int * w = KJSEmbed::extractValue<int *>(exec, args, 0);
   int * h = KJSEmbed::extractValue<int *>(exec, args, 0);
END_VARIANT_METHOD

// void setCoords(int x1, int y1, int x2, int y2)
START_VARIANT_METHOD( setCoords, QRect )
   int x1 = KJSEmbed::extractValue<int>(exec, args, 0);
   int y1 = KJSEmbed::extractValue<int>(exec, args, 0);
   int x2 = KJSEmbed::extractValue<int>(exec, args, 0);
   int y2 = KJSEmbed::extractValue<int>(exec, args, 0);
    value.setCoords(y2);
END_VARIANT_METHOD

// void getCoords(int *x1, int *y1, int *x2, int *y2) const 
START_VARIANT_METHOD( getCoords, QRect )
   int * x1 = KJSEmbed::extractValue<int *>(exec, args, 0);
   int * y1 = KJSEmbed::extractValue<int *>(exec, args, 0);
   int * x2 = KJSEmbed::extractValue<int *>(exec, args, 0);
   int * y2 = KJSEmbed::extractValue<int *>(exec, args, 0);
END_VARIANT_METHOD

// void adjust(int x1, int y1, int x2, int y2)
START_VARIANT_METHOD( adjust, QRect )
   int x1 = KJSEmbed::extractValue<int>(exec, args, 0);
   int y1 = KJSEmbed::extractValue<int>(exec, args, 0);
   int x2 = KJSEmbed::extractValue<int>(exec, args, 0);
   int y2 = KJSEmbed::extractValue<int>(exec, args, 0);
END_VARIANT_METHOD

// QRect adjusted(int x1, int y1, int x2, int y2) const 
START_VARIANT_METHOD( adjusted, QRect )
   int x1 = KJSEmbed::extractValue<int>(exec, args, 0);
   int y1 = KJSEmbed::extractValue<int>(exec, args, 0);
   int x2 = KJSEmbed::extractValue<int>(exec, args, 0);
   int y2 = KJSEmbed::extractValue<int>(exec, args, 0);
END_VARIANT_METHOD

// QSize size() const 
START_VARIANT_METHOD( size, QRect )
   QSize tmp = value.size();
   result = KJSEmbed::createValue( exec, "QSize", tmp );
END_VARIANT_METHOD

// int width() const 
START_VARIANT_METHOD( width, QRect )
   int tmp = value.width();
   result = KJSEmbed::createValue( exec, "int", tmp );
END_VARIANT_METHOD

// int height() const 
START_VARIANT_METHOD( height, QRect )
   int tmp = value.height();
   result = KJSEmbed::createValue( exec, "int", tmp );
END_VARIANT_METHOD

// void setWidth(int w)
START_VARIANT_METHOD( setWidth, QRect )
   int w = KJSEmbed::extractValue<int>(exec, args, 0);
    value.setWidth(w);
END_VARIANT_METHOD

// void setHeight(int h)
START_VARIANT_METHOD( setHeight, QRect )
   int h = KJSEmbed::extractValue<int>(exec, args, 0);
    value.setHeight(h);
END_VARIANT_METHOD

// void setSize(const QSize &s)
START_VARIANT_METHOD( setSize, QRect )
   const QSize & s = KJSEmbed::extractValue<const QSize &>(exec, args, 0);
    value.setSize(s);
END_VARIANT_METHOD

// bool contains(const QPoint &p, bool proper=false) const 
START_VARIANT_METHOD( contains, QRect )
   const QPoint & p = KJSEmbed::extractValue<const QPoint &>(exec, args, 0);
   bool proper = KJSEmbed::extractValue<bool>(exec, args, false);
END_VARIANT_METHOD

// bool contains(int x, int y) const 
START_VARIANT_METHOD( contains, QRect )
   int x = KJSEmbed::extractValue<int>(exec, args, 0);
   int y = KJSEmbed::extractValue<int>(exec, args, 0);
END_VARIANT_METHOD

// bool contains(int x, int y, bool proper) const 
START_VARIANT_METHOD( contains, QRect )
   int x = KJSEmbed::extractValue<int>(exec, args, 0);
   int y = KJSEmbed::extractValue<int>(exec, args, 0);
   bool proper = KJSEmbed::extractValue<bool>(exec, args, 0);
END_VARIANT_METHOD

// bool contains(const QRect &r, bool proper=false) const 
START_VARIANT_METHOD( contains, QRect )
   const QRect & r = KJSEmbed::extractValue<const QRect &>(exec, args, 0);
   bool proper = KJSEmbed::extractValue<bool>(exec, args, false);
END_VARIANT_METHOD

// QRect unite(const QRect &r) const 
START_VARIANT_METHOD( unite, QRect )
   const QRect & r = KJSEmbed::extractValue<const QRect &>(exec, args, 0);
END_VARIANT_METHOD

// QRect intersect(const QRect &r) const 
START_VARIANT_METHOD( intersect, QRect )
   const QRect & r = KJSEmbed::extractValue<const QRect &>(exec, args, 0);
END_VARIANT_METHOD

// bool intersects(const QRect &r) const 
START_VARIANT_METHOD( intersects, QRect )
   const QRect & r = KJSEmbed::extractValue<const QRect &>(exec, args, 0);
END_VARIANT_METHOD
}

NO_ENUMS( QRect ) 
NO_STATICS( QRect ) 



START_CTOR( QRect, QRect, 0 )
   if (args.size() == 0 )
   {
       return new KJSEmbed::QRectBinding(exec, QRect())
   }
   if (args.size() == 2 )
   {
   const QPoint & topleft = KJSEmbed::extractValue<const QPoint &>(exec, args, 0);
   const QPoint & bottomright = KJSEmbed::extractValue<const QPoint &>(exec, args, 0);
       return new KJSEmbed::QRectBinding(exec, QRect(topleft, bottomright))
   }
   if (args.size() == 2 )
   {
   const QPoint & topleft = KJSEmbed::extractValue<const QPoint &>(exec, args, 0);
   const QSize & size = KJSEmbed::extractValue<const QSize &>(exec, args, 0);
       return new KJSEmbed::QRectBinding(exec, QRect(topleft, size))
   }
   if (args.size() == 4 )
   {
   int left = KJSEmbed::extractValue<int>(exec, args, 0);
   int top = KJSEmbed::extractValue<int>(exec, args, 0);
   int width = KJSEmbed::extractValue<int>(exec, args, 0);
   int height = KJSEmbed::extractValue<int>(exec, args, 0);
       return new KJSEmbed::QRectBinding(exec, QRect(left, top, width, height))
   }
END_CTOR


START_METHOD_LUT( QRect )
    { isNull, 0, KJS::DontDelete|KJS::ReadOnly, &QRectNS::isNull },
    { isEmpty, 0, KJS::DontDelete|KJS::ReadOnly, &QRectNS::isEmpty },
    { isValid, 0, KJS::DontDelete|KJS::ReadOnly, &QRectNS::isValid },
    { left, 0, KJS::DontDelete|KJS::ReadOnly, &QRectNS::left },
    { top, 0, KJS::DontDelete|KJS::ReadOnly, &QRectNS::top },
    { right, 0, KJS::DontDelete|KJS::ReadOnly, &QRectNS::right },
    { bottom, 0, KJS::DontDelete|KJS::ReadOnly, &QRectNS::bottom },
    { normalized, 0, KJS::DontDelete|KJS::ReadOnly, &QRectNS::normalized },
    { x, 0, KJS::DontDelete|KJS::ReadOnly, &QRectNS::x },
    { y, 0, KJS::DontDelete|KJS::ReadOnly, &QRectNS::y },
    { setLeft, 1, KJS::DontDelete|KJS::ReadOnly, &QRectNS::setLeft },
    { setTop, 1, KJS::DontDelete|KJS::ReadOnly, &QRectNS::setTop },
    { setRight, 1, KJS::DontDelete|KJS::ReadOnly, &QRectNS::setRight },
    { setBottom, 1, KJS::DontDelete|KJS::ReadOnly, &QRectNS::setBottom },
    { setX, 1, KJS::DontDelete|KJS::ReadOnly, &QRectNS::setX },
    { setY, 1, KJS::DontDelete|KJS::ReadOnly, &QRectNS::setY },
    { setTopLeft, 1, KJS::DontDelete|KJS::ReadOnly, &QRectNS::setTopLeft },
    { setBottomRight, 1, KJS::DontDelete|KJS::ReadOnly, &QRectNS::setBottomRight },
    { setTopRight, 1, KJS::DontDelete|KJS::ReadOnly, &QRectNS::setTopRight },
    { setBottomLeft, 1, KJS::DontDelete|KJS::ReadOnly, &QRectNS::setBottomLeft },
    { topLeft, 0, KJS::DontDelete|KJS::ReadOnly, &QRectNS::topLeft },
    { bottomRight, 0, KJS::DontDelete|KJS::ReadOnly, &QRectNS::bottomRight },
    { topRight, 0, KJS::DontDelete|KJS::ReadOnly, &QRectNS::topRight },
    { bottomLeft, 0, KJS::DontDelete|KJS::ReadOnly, &QRectNS::bottomLeft },
    { center, 0, KJS::DontDelete|KJS::ReadOnly, &QRectNS::center },
    { moveLeft, 1, KJS::DontDelete|KJS::ReadOnly, &QRectNS::moveLeft },
    { moveTop, 1, KJS::DontDelete|KJS::ReadOnly, &QRectNS::moveTop },
    { moveRight, 1, KJS::DontDelete|KJS::ReadOnly, &QRectNS::moveRight },
    { moveBottom, 1, KJS::DontDelete|KJS::ReadOnly, &QRectNS::moveBottom },
    { moveTopLeft, 1, KJS::DontDelete|KJS::ReadOnly, &QRectNS::moveTopLeft },
    { moveBottomRight, 1, KJS::DontDelete|KJS::ReadOnly, &QRectNS::moveBottomRight },
    { moveTopRight, 1, KJS::DontDelete|KJS::ReadOnly, &QRectNS::moveTopRight },
    { moveBottomLeft, 1, KJS::DontDelete|KJS::ReadOnly, &QRectNS::moveBottomLeft },
    { moveCenter, 1, KJS::DontDelete|KJS::ReadOnly, &QRectNS::moveCenter },
    { translate, 2, KJS::DontDelete|KJS::ReadOnly, &QRectNS::translate },
    { translate, 1, KJS::DontDelete|KJS::ReadOnly, &QRectNS::translate },
    { translated, 2, KJS::DontDelete|KJS::ReadOnly, &QRectNS::translated },
    { translated, 1, KJS::DontDelete|KJS::ReadOnly, &QRectNS::translated },
    { moveTo, 2, KJS::DontDelete|KJS::ReadOnly, &QRectNS::moveTo },
    { moveTo, 1, KJS::DontDelete|KJS::ReadOnly, &QRectNS::moveTo },
    { setRect, 4, KJS::DontDelete|KJS::ReadOnly, &QRectNS::setRect },
    { getRect, 4, KJS::DontDelete|KJS::ReadOnly, &QRectNS::getRect },
    { setCoords, 4, KJS::DontDelete|KJS::ReadOnly, &QRectNS::setCoords },
    { getCoords, 4, KJS::DontDelete|KJS::ReadOnly, &QRectNS::getCoords },
    { adjust, 4, KJS::DontDelete|KJS::ReadOnly, &QRectNS::adjust },
    { adjusted, 4, KJS::DontDelete|KJS::ReadOnly, &QRectNS::adjusted },
    { size, 0, KJS::DontDelete|KJS::ReadOnly, &QRectNS::size },
    { width, 0, KJS::DontDelete|KJS::ReadOnly, &QRectNS::width },
    { height, 0, KJS::DontDelete|KJS::ReadOnly, &QRectNS::height },
    { setWidth, 1, KJS::DontDelete|KJS::ReadOnly, &QRectNS::setWidth },
    { setHeight, 1, KJS::DontDelete|KJS::ReadOnly, &QRectNS::setHeight },
    { setSize, 1, KJS::DontDelete|KJS::ReadOnly, &QRectNS::setSize },
    { contains, 2, KJS::DontDelete|KJS::ReadOnly, &QRectNS::contains },
    { contains, 2, KJS::DontDelete|KJS::ReadOnly, &QRectNS::contains },
    { contains, 3, KJS::DontDelete|KJS::ReadOnly, &QRectNS::contains },
    { contains, 2, KJS::DontDelete|KJS::ReadOnly, &QRectNS::contains },
    { unite, 1, KJS::DontDelete|KJS::ReadOnly, &QRectNS::unite },
    { intersect, 1, KJS::DontDelete|KJS::ReadOnly, &QRectNS::intersect },
    { intersects, 1, KJS::DontDelete|KJS::ReadOnly, &QRectNS::intersects },
END_METHOD_LUT

