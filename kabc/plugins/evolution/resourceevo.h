#include "resource.h"

namespace Evolution {
    class DBWrapper;
}

namespace KABC {
    class ResourceEvolution : public Resource {
    public:
        ResourceEvolution( const KConfig* config );
        ~ResourceEvolution();

        bool doOpen();
        void doClose();
        Ticket* requestSaveTicket();
        bool load();
        bool save( Ticket* ticket );
        void removeAddressee( const Addressee& );
    private:
        Evolution::DBWrapper *mWrap;
    };
}
