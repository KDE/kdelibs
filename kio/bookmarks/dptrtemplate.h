#ifndef __dptrtemplate_h__
#define __dptrtemplate_h__

#include <qptrdict.h>

template<class Instance, class PrivateData>
class dPtrTemplate {
public:
    static PrivateData* d( const Instance* instance )
    {
        if ( !d_ptr ) {
            d_ptr = new QPtrDict<PrivateData>;
            qAddPostRoutine( cleanup_d_ptr );
        }
        PrivateData* ret = d_ptr->find( (void*) instance );
        if ( ! ret ) {
            ret = new PrivateData;
            d_ptr->replace( (void*) instance, ret );
        }
        return ret;
    }
    static void delete_d( const Instance* instance )
    {
        if ( d_ptr )
            d_ptr->remove( (void*) instance );
    }
private:
    static void cleanup_d_ptr()
    {
        delete d_ptr;
    }
    static QPtrDict<PrivateData>* d_ptr;
};

#endif
