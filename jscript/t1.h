typedef union
{
     int vali;
     double vald;
     char *name;
     void *ptr;
} YYSTYPE;
#define	FUNCTION	258
#define	IF	259
#define	ELSE	260
#define	IN	261
#define	WITH	262
#define	WHILE	263
#define	FOR	264
#define	SHIFT_LEFT	265
#define	SHIFT_RIGHT	266
#define	EQ	267
#define	NEQ	268
#define	OR	269
#define	AND	270
#define	THIS	271
#define	B_NULL	272
#define	FLOAT	273
#define	B_TRUE	274
#define	B_FALSE	275
#define	NEW	276
#define	DELETE	277
#define	BREAK	278
#define	CONTINUE	279
#define	RETURN	280
#define	VAR	281
#define	PP	282
#define	MM	283
#define	STRING	284
#define	LEQ	285
#define	GEQ	286
#define	MAS	287
#define	DAS	288
#define	AAS	289
#define	SAS	290
#define	PAS	291
#define	RAS	292
#define	BAAS	293
#define	BOAS	294
#define	NUM	295
#define	IDENTIFIER	296


extern YYSTYPE yylval;
