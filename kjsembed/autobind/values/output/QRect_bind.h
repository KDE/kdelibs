#ifndef BIND_QRect_H
#define BIND_QRect_H

#include <value_binding.h>
#include <static_binding.h>

class QRect;

namespace KJSEmbed
{
   class QRectBinding : public ValueBinding
   {
       public:
           QRectBinding( KJS::ExecState *exec, const QRect &value );
       private:
           static const KJS::ClassInfo info;
           virtual const KJS::ClassInfo* classInfo() const { return &info; }
   };

   KJS_BINDING( QRect )

}
#endif // BIND_QRect_H


