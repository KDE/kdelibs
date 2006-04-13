#ifndef BIND_QRectF_H
#define BIND_QRectF_H

#include <value_binding.h>
#include <static_binding.h>

class QRectF;

namespace KJSEmbed
{
   class QRectFBinding : public ValueBinding
   {
       public:
           QRectFBinding( KJS::ExecState *exec, const QRectF &value );
       private:
           static const KJS::ClassInfo info;
           virtual const KJS::ClassInfo* classInfo() const { return &info; }
   };

   KJS_BINDING( QRectF )

}
#endif // BIND_QRectF_H


