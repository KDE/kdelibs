typedef union
{
  // generic data types
  long		_int;
  char*		_str;
  unsigned short	_char;
  double	_float;

  vector<char *> *_strs;

  // types
  vector<TypeComponent *> *_typeComponentSeq;
  TypeComponent* _typeComponent;

  // enums
  vector<EnumComponent *> *_enumComponentSeq;

  // interfaces
  InterfaceDef *_interfaceDef;

  ParamDef* _paramDef;
  vector<ParamDef *> *_paramDefSeq;

  MethodDef* _methodDef;
  vector<MethodDef *> *_methodDefSeq;

  AttributeDef* _attributeDef;
  vector<AttributeDef *> *_attributeDefSeq;
} YYSTYPE;
#define	T_STRUCT	257
#define	T_ENUM	258
#define	T_INTERFACE	259
#define	T_MODULE	260
#define	T_LEFT_CURLY_BRACKET	261
#define	T_RIGHT_CURLY_BRACKET	262
#define	T_LEFT_PARANTHESIS	263
#define	T_RIGHT_PARANTHESIS	264
#define	T_LESS	265
#define	T_GREATER	266
#define	T_EQUAL	267
#define	T_SEMICOLON	268
#define	T_COLON	269
#define	T_COMMA	270
#define	T_IDENTIFIER	271
#define	T_INTEGER_LITERAL	272
#define	T_UNKNOWN	273
#define	T_BOOLEAN	274
#define	T_STRING	275
#define	T_LONG	276
#define	T_BYTE	277
#define	T_OBJECT	278
#define	T_SEQUENCE	279
#define	T_AUDIO	280
#define	T_IN	281
#define	T_OUT	282
#define	T_STREAM	283
#define	T_MULTI	284
#define	T_ATTRIBUTE	285
#define	T_READONLY	286
#define	T_ASYNC	287
#define	T_ONEWAY	288


extern YYSTYPE yylval;
