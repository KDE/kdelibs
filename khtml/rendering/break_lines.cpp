#include <break_lines.h>
#include <klibloader.h>
#include <qtextcodec.h>
#include <q3cleanuphandler.h>
#include <stdio.h>
#include <config.h>


/* If HAVE_LIBTHAI is defined, libkhtml will link against
 * libthai since compile time. Otherwise it will try to
 * dlopen at run-time
 *
 * Ott Pattara Nov 14, 2004
 */

#ifndef HAVE_LIBTHAI
typedef int (*th_brk_def)(const unsigned char*, int[], int);
static th_brk_def th_brk;
#else
#include <thai/thailib.h>
#include <thai/thbrk.h>
#endif

namespace khtml {
    struct ThaiCache
    {
        ThaiCache() {
            string = 0;
            allocated = 0x400;
            wbrpos = (int *) malloc(allocated*sizeof(int));
            numwbrpos = 0;
            numisbreakable = 0x400;
            isbreakable = (int *) malloc(numisbreakable*sizeof(int));
	    library = 0;
        }
        ~ThaiCache() {
            free(wbrpos);
            free(isbreakable);
            if (library) library->unload();
        }
        const QChar *string;
        int *wbrpos;
        int *isbreakable;
        int allocated;
        int numwbrpos,numisbreakable;
        KLibrary *library;
    };
    static ThaiCache *cache = 0;

    void cleanup_thaibreaks()
    {
        delete cache;
        cache = 0;
#ifndef HAVE_LIBTHAI
        th_brk = 0;
#endif
    }

    bool isBreakableThai( const QChar *string, const int pos, const int len)
    {
        static QTextCodec *thaiCodec = QTextCodec::codecForMib(2259);
	//printf("Entering isBreakableThai with pos = %d\n", pos);

#ifndef HAVE_LIBTHAI

	KLibrary *lib = 0;

        /* load libthai dynamically */
	if (( !th_brk ) && thaiCodec  ) {
	    printf("Try to load libthai dynamically...\n");
            KLibLoader *loader = KLibLoader::self();
            lib = loader->library("libthai");
            if (lib && lib->hasSymbol("th_brk")) {
                th_brk = (th_brk_def) lib->symbol("th_brk");
            } else {
                // indication that loading failed and we shouldn't try to load again
		printf("Error, can't load libthai...\n");
                thaiCodec = 0;
                if (lib)
                    lib->unload();
            }
        }

        if (!th_brk ) {
            return true;
        }
#endif

	if (!cache ) {
            cache = new ThaiCache;
#ifndef HAVE_LIBTHAI
            cache->library = lib;
#endif
	}

        // build up string of thai chars
        if ( string != cache->string ) {
            //fprintf(stderr,"new string found (not in cache), calling libthai\n");
            QByteArray cstr = thaiCodec->fromUnicode( QConstString(string,len).string());
            //printf("About to call libthai::th_brk with str: %s",cstr.data());

            cache->numwbrpos = th_brk((const unsigned char*) cstr.data(), cache->wbrpos, cache->allocated);
            //fprintf(stderr,"libthai returns with value %d\n",cache->numwbrpos);
            if (cache->numwbrpos > cache->allocated) {
                cache->allocated = cache->numwbrpos;
                cache->wbrpos = (int *)realloc(cache->wbrpos, cache->allocated*sizeof(int));
                cache->numwbrpos = th_brk((const unsigned char*) cstr.data(), cache->wbrpos, cache->allocated);
            }
	    if ( len > cache->numisbreakable ) {
		cache->numisbreakable=len;
                cache->isbreakable = (int *)realloc(cache->isbreakable, cache->numisbreakable*sizeof(int));
	    }
	    for (int i = 0 ; i < len ; ++i) {
		cache->isbreakable[i] = 0;
	    }
            if ( cache->numwbrpos > 0 ) {
            	for (int i = cache->numwbrpos-1; i >= 0; --i) {
                	cache->isbreakable[cache->wbrpos[i]] = 1;
		}
	    }
            cache->string = string;
        }
	//printf("Returning %d\n", cache->isbreakable[pos]);
	return cache->isbreakable[pos];
    }
}
