#ifndef BIND_QPoint_H
#define BIND_QPoint_H

#include <value_binding.h>
#include <static_binding.h>

class QPoint;

namespace KJSEmbed
{
   class QPointBinding : public ValueBinding
   {
       public:
           QPointBinding( KJS::ExecState *exec, const QPoint &value );
       private:
           static const KJS::ClassInfo info;
           virtual const KJS::ClassInfo* classInfo() const { return &info; }
   };

   KJS_BINDING( QPoint )

}
#endif // BIND_QPoint_H


