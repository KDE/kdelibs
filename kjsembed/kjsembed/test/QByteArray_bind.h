#ifndef BIND_ByteArray_H
#define BIND_ByteArray_H

#include <object_binding.h>
#include <static_binding.h>

class QByteArray;

namespace KJSEmbed
{
   class QByteArrayBinding : public VariantBinding
   {
       public:
           QByteArrayBinding( KJS::ExecState *exec, const QByteArray &value );
           static const KJS::ClassInfo info;
           virtual const KJS::ClassInfo* classInfo() const { return &info; }
   };

   class QByteArrayData 
   { 
       public: 
           static const KJSEmbed::Method p_methods[]; 
           static const KJSEmbed::Method p_statics[]; 
           static const KJSEmbed::Enumerator p_enums[]; 
           static const KJSEmbed::Constructor p_constructor; 
           static KJS::JSObject *ctorMethod( KJS::ExecState *exec, const KJS::List &args );
           static const KJSEmbed::Enumerator *enums() { return p_enums;} 
           static const KJSEmbed::Method *methods() { return p_methods;} 
           static const KJSEmbed::Method *statics() { return p_statics;} 
           static const KJSEmbed::Constructor *constructor() { return &p_constructor;} 
           static const KJS::JSObject *construct(KJS::ExecState *exec, const KJS::List &args)
                                           { return (*p_constructor.construct)(exec,args); } 
   };

}
#endif // BIND_QByteArray_H


