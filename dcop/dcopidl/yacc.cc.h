typedef union
{
  long   _int;
  QString        *_str;
  unsigned short          _char;
  double _float;
} YYSTYPE;
#define	T_CHARACTER_LITERAL	258
#define	T_DOUBLE_LITERAL	259
#define	T_IDENTIFIER	260
#define	T_INTEGER_LITERAL	261
#define	T_STRING_LITERAL	262
#define	T_INCLUDE	263
#define	T_CLASS	264
#define	T_STRUCT	265
#define	T_LEFT_CURLY_BRACKET	266
#define	T_LEFT_PARANTHESIS	267
#define	T_RIGHT_CURLY_BRACKET	268
#define	T_RIGHT_PARANTHESIS	269
#define	T_COLON	270
#define	T_SEMICOLON	271
#define	T_PUBLIC	272
#define	T_PROTECTED	273
#define	T_TRIPE_DOT	274
#define	T_PRIVATE	275
#define	T_VIRTUAL	276
#define	T_CONST	277
#define	T_INLINE	278
#define	T_FRIEND	279
#define	T_RETURN	280
#define	T_SIGNAL	281
#define	T_SLOT	282
#define	T_TYPEDEF	283
#define	T_PLUS	284
#define	T_MINUS	285
#define	T_COMMA	286
#define	T_ASTERISK	287
#define	T_TILDE	288
#define	T_LESS	289
#define	T_GREATER	290
#define	T_AMPERSAND	291
#define	T_EXTERN	292
#define	T_EXTERN_C	293
#define	T_ACCESS	294
#define	T_ENUM	295
#define	T_NAMESPACE	296
#define	T_USING	297
#define	T_UNKNOWN	298
#define	T_TRIPLE_DOT	299
#define	T_TRUE	300
#define	T_FALSE	301
#define	T_STATIC	302
#define	T_EQUAL	303
#define	T_SCOPE	304
#define	T_NULL	305
#define	T_INT	306
#define	T_ARRAY_OPEN	307
#define	T_ARRAY_CLOSE	308
#define	T_CHAR	309
#define	T_DCOP	310
#define	T_DCOP_AREA	311
#define	T_SIGNED	312
#define	T_UNSIGNED	313
#define	T_LONG	314
#define	T_SHORT	315
#define	T_FUNOPERATOR	316
#define	T_MISCOPERATOR	317


extern YYSTYPE yylval;
