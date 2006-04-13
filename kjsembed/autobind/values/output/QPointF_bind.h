#ifndef BIND_QPointF_H
#define BIND_QPointF_H

#include <value_binding.h>
#include <static_binding.h>

class QPointF;

namespace KJSEmbed
{
   class QPointFBinding : public ValueBinding
   {
       public:
           QPointFBinding( KJS::ExecState *exec, const QPointF &value );
       private:
           static const KJS::ClassInfo info;
           virtual const KJS::ClassInfo* classInfo() const { return &info; }
   };

   KJS_BINDING( QPointF )

}
#endif // BIND_QPointF_H


