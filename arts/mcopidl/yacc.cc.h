typedef union
{
  // generic data types
  long		_int;
  char*		_str;
  unsigned short	_char;
  double	_float;

  vector<char*> *_strs;

  // types
  vector<TypeComponent> *_typeComponentSeq;
  TypeComponent* _typeComponent;

  // enums
  vector<EnumComponent> *_enumComponentSeq;

  // interfaces
  InterfaceDef *_interfaceDef;

  ParamDef* _paramDef;
  vector<ParamDef> *_paramDefSeq;

  MethodDef* _methodDef;
  vector<MethodDef> *_methodDefSeq;

  AttributeDef* _attributeDef;
  vector<AttributeDef> *_attributeDefSeq;
} YYSTYPE;
#define	T_STRUCT	257
#define	T_ENUM	258
#define	T_INTERFACE	259
#define	T_MODULE	260
#define	T_VOID	261
#define	T_LEFT_CURLY_BRACKET	262
#define	T_RIGHT_CURLY_BRACKET	263
#define	T_LEFT_PARANTHESIS	264
#define	T_RIGHT_PARANTHESIS	265
#define	T_LESS	266
#define	T_GREATER	267
#define	T_EQUAL	268
#define	T_SEMICOLON	269
#define	T_COLON	270
#define	T_COMMA	271
#define	T_IDENTIFIER	272
#define	T_QUALIFIED_IDENTIFIER	273
#define	T_INTEGER_LITERAL	274
#define	T_UNKNOWN	275
#define	T_BOOLEAN	276
#define	T_STRING	277
#define	T_LONG	278
#define	T_BYTE	279
#define	T_OBJECT	280
#define	T_SEQUENCE	281
#define	T_AUDIO	282
#define	T_FLOAT	283
#define	T_IN	284
#define	T_OUT	285
#define	T_STREAM	286
#define	T_MULTI	287
#define	T_ATTRIBUTE	288
#define	T_READONLY	289
#define	T_ASYNC	290
#define	T_ONEWAY	291
#define	T_DEFAULT	292


extern YYSTYPE yylval;
