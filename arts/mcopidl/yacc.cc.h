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
#define	T_STRUCT	258
#define	T_ENUM	259
#define	T_INTERFACE	260
#define	T_MODULE	261
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
#define	T_INTEGER_LITERAL	273
#define	T_UNKNOWN	274
#define	T_BOOLEAN	275
#define	T_STRING	276
#define	T_LONG	277
#define	T_BYTE	278
#define	T_OBJECT	279
#define	T_SEQUENCE	280
#define	T_AUDIO	281
#define	T_IN	282
#define	T_OUT	283
#define	T_STREAM	284
#define	T_MULTI	285
#define	T_ATTRIBUTE	286
#define	T_READONLY	287
#define	T_ASYNC	288
#define	T_ONEWAY	289
#define	T_DEFAULT	290


extern YYSTYPE yylval;
