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
#define	T_LEFT_CURLY_BRACKET	264
#define	T_LEFT_PARANTHESIS	265
#define	T_RIGHT_CURLY_BRACKET	266
#define	T_RIGHT_PARANTHESIS	267
#define	T_COLON	268
#define	T_SEMICOLON	269
#define	T_PUBLIC	270
#define	T_VIRTUAL	271
#define	T_CONST	272
#define	T_TYPEDEF	273
#define	T_COMMA	274
#define	T_LESS	275
#define	T_GREATER	276
#define	T_AMPERSAND	277
#define	T_ENUM	278
#define	T_UNKNOWN	279
#define	T_EQUAL	280
#define	T_SCOPE	281
#define	T_NULL	282
#define	T_DCOP	283
#define	T_DCOP_AREA	284


extern YYSTYPE yylval;
