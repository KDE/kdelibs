typedef union
{
     char *name;
} YYSTYPE;
#define	STRING	258
#define	REF	259
#define	PATH	260
#define	DSLASH	261


extern YYSTYPE kurllval;

void kurl_initFlex( char *_code );

