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
#define	T_STRING_LITERAL	261
#define	T_INCLUDE	262
#define	T_CLASS	263
#define	T_STRUCT	264
#define	T_LEFT_CURLY_BRACKET	265
#define	T_LEFT_PARANTHESIS	266
#define	T_RIGHT_CURLY_BRACKET	267
#define	T_RIGHT_PARANTHESIS	268
#define	T_COLON	269
#define	T_SEMICOLON	270
#define	T_PUBLIC	271
#define	T_PROTECTED	272
#define	T_PRIVATE	273
#define	T_VIRTUAL	274
#define	T_CONST	275
#define	T_RETURN	276
#define	T_SIGNAL	277
#define	T_SLOT	278
#define	T_TYPEDEF	279
#define	T_COMMA	280
#define	T_ASTERISK	281
#define	T_TILDE	282
#define	T_LESS	283
#define	T_GREATER	284
#define	T_AMPERSAND	285
#define	T_ACCESS	286
#define	T_ENUM	287
#define	T_UNKNOWN	288
#define	T_TRUE	289
#define	T_FALSE	290
#define	T_STATIC	291
#define	T_EQUAL	292
#define	T_SCOPE	293
#define	T_NULL	294
#define	T_DCOP	295
#define	T_DCOP_AREA	296
#define	T_SIGNED	297
#define	T_UNSIGNED	298


extern YYSTYPE yylval;
