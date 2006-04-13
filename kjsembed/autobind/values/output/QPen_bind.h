#ifndef BIND_QPen_H
#define BIND_QPen_H

#include <value_binding.h>
#include <static_binding.h>

class QPen;

namespace KJSEmbed
{
   class QPenBinding : public ValueBinding
   {
       public:
           QPenBinding( KJS::ExecState *exec, const QPen &value );
       private:
           static const KJS::ClassInfo info;
           virtual const KJS::ClassInfo* classInfo() const { return &info; }
   };

   KJS_BINDING( QPen )

}
#endif // BIND_QPen_H


