typedef union
{
     char valb;
     int vali;
     double vald;
     char *name;
     void *ptr;
} YYSTYPE;
#define	NOT	257
#define	EQ	258
#define	NEQ	259
#define	LEQ	260
#define	GEQ	261
#define	LE	262
#define	GR	263
#define	OR	264
#define	AND	265
#define	IN	266
#define	EXIST	267
#define	MAX	268
#define	MIN	269
#define	BOOL	270
#define	STRING	271
#define	ID	272
#define	NUM	273
#define	FLOAT	274


extern YYSTYPE kiotraderlval;
