typedef union
{
  long   _int;
  QString        *_str;
  ushort          _char;
  double _float;
} YYSTYPE;
#define	T_CHARACTER_LITERAL	258
#define	T_DOUBLE_LITERAL	259
#define	T_IDENTIFIER	260
#define	T_INTEGER_LITERAL	261
#define	T_STRING_LITERAL	262
#define	T_INCLUDE	263
#define	T_CLASS	264
#define	T_LEFT_CURLY_BRACKET	265
#define	T_LEFT_PARANTHESIS	266
#define	T_RIGHT_CURLY_BRACKET	267
#define	T_RIGHT_PARANTHESIS	268
#define	T_COLON	269
#define	T_SEMICOLON	270
#define	T_PUBLIC	271
#define	T_VIRTUAL	272
#define	T_CONST	273
#define	T_TYPEDEF	274
#define	T_COMMA	275
#define	T_LESS	276
#define	T_GREATER	277
#define	T_AMPERSAND	278
#define	T_ENUM	279
#define	T_UNKNOWN	280
#define	T_EQUAL	281
#define	T_SCOPE	282
#define	T_NULL	283
#define	T_DCOP	284


extern YYSTYPE yylval;
