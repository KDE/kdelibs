typedef union
{
  long   _int;
  QString        *_str;
  ushort          _char;
  double _float;
} YYSTYPE;
#define	T_CHARACTER_LITERAL	257
#define	T_DOUBLE_LITERAL	258
#define	T_IDENTIFIER	259
#define	T_INTEGER_LITERAL	260
#define	T_CHAR_LITERAL	261
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
#define	T_PRIVATE	274
#define	T_VIRTUAL	275
#define	T_CONST	276
#define	T_RETURN	277
#define	T_SIGNAL	278
#define	T_SLOT	279
#define	T_TYPEDEF	280
#define	T_COMMA	281
#define	T_ASTERISK	282
#define	T_TILDE	283
#define	T_LESS	284
#define	T_GREATER	285
#define	T_AMPERSAND	286
#define	T_ACCESS	287
#define	T_ENUM	288
#define	T_NAMESPACE	289
#define	T_UNKNOWN	290
#define	T_TRUE	291
#define	T_FALSE	292
#define	T_STATIC	293
#define	T_EQUAL	294
#define	T_SCOPE	295
#define	T_NULL	296
#define	T_DCOP	297
#define	T_DCOP_AREA	298
#define	T_SIGNED	299
#define	T_UNSIGNED	300
#define	T_FUNOPERATOR	301
#define	T_MISCOPERATOR	302


extern YYSTYPE yylval;
