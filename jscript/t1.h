typedef union
{
     int vali;
     double vald;
     char *name;
     void *ptr;
} YYSTYPE;
#define	FUNCTION	257
#define	IF	258
#define	ELSE	259
#define	IN	260
#define	WITH	261
#define	WHILE	262
#define	FOR	263
#define	SHIFT_LEFT	264
#define	SHIFT_RIGHT	265
#define	EQ	266
#define	NEQ	267
#define	OR	268
#define	AND	269
#define	THIS	270
#define	B_NULL	271
#define	FLOAT	272
#define	B_TRUE	273
#define	B_FALSE	274
#define	NEW	275
#define	DELETE	276
#define	BREAK	277
#define	CONTINUE	278
#define	RETURN	279
#define	VAR	280
#define	PP	281
#define	MM	282
#define	STRING	283
#define	LEQ	284
#define	GEQ	285
#define	MAS	286
#define	DAS	287
#define	AAS	288
#define	SAS	289
#define	PAS	290
#define	RAS	291
#define	BAAS	292
#define	BOAS	293
#define	NUM	294
#define	IDENTIFIER	295


extern YYSTYPE yyjscriptlval;
