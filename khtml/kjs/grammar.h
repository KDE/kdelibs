typedef union {
  int                 ival;
  double              dval;
  CString             *cstr;
  UString             *ustr;
  Node                *node;
  StatementNode       *stat;
  ParameterNode       *param;
  FuncDeclNode        *func;
  ProgramNode         *prog;
  AssignExprNode      *init;
  SourceElementNode   *src;
  SourceElementsNode  *srcs;
  StatListNode        *slist;
  ArgumentsNode       *args;
  ArgumentListNode    *alist;
  VarDeclNode         *decl;
  VarDeclListNode     *vlist;
  Operator            op;
} YYSTYPE;
#define	LF	258
#define	NULLTOKEN	259
#define	TRUETOKEN	260
#define	FALSETOKEN	261
#define	STRING	262
#define	DECIMAL	263
#define	INTEGER	264
#define	BREAK	265
#define	FOR	266
#define	NEW	267
#define	VAR	268
#define	CONTINUE	269
#define	FUNCTION	270
#define	RETURN	271
#define	VOID	272
#define	DELETE	273
#define	IF	274
#define	THIS	275
#define	WHILE	276
#define	ELSE	277
#define	IN	278
#define	TYPEOF	279
#define	WITH	280
#define	RESERVED	281
#define	PRINT	282
#define	ALERT	283
#define	EQEQ	284
#define	NE	285
#define	LE	286
#define	GE	287
#define	OR	288
#define	AND	289
#define	PLUSPLUS	290
#define	MINUSMINUS	291
#define	LSHIFT	292
#define	RSHIFT	293
#define	URSHIFT	294
#define	PLUSEQUAL	295
#define	MINUSEQUAL	296
#define	MULTEQUAL	297
#define	DIVEQUAL	298
#define	LSHIFTEQUAL	299
#define	RSHIFTEQUAL	300
#define	URSHIFTEQUAL	301
#define	ANDEQUAL	302
#define	MODEQUAL	303
#define	XOREQUAL	304
#define	OREQUAL	305
#define	DOUBLE	306
#define	IDENT	307


extern YYSTYPE yylval;
