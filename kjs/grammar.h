typedef union {
  int                 ival;
  double              dval;
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
  CaseBlockNode       *cblk;
  ClauseListNode      *clist;
  CaseClauseNode      *ccl;
  ElementNode         *elm;
  ElisionNode         *eli;
  Operator            op;
} YYSTYPE;
#define	NULLTOKEN	258
#define	TRUETOKEN	259
#define	FALSETOKEN	260
#define	STRING	261
#define	INTEGER	263
#define	BREAK	264
#define	CASE	265
#define	DEFAULT	266
#define	FOR	267
#define	NEW	268
#define	VAR	269
#define	CONTINUE	270
#define	FUNCTION	271
#define	RETURN	272
#define	VOID	273
#define	DELETE	274
#define	IF	275
#define	THIS	276
#define	DO	277
#define	WHILE	278
#define	ELSE	279
#define	IN	280
#define	INSTANCEOF	281
#define	TYPEOF	282
#define	SWITCH	283
#define	WITH	284
#define	RESERVED	285
#define	THROW	286
#define	TRY	287
#define	CATCH	288
#define	FINALLY	289
#define	EQEQ	290
#define	NE	291
#define	STREQ	292
#define	STRNEQ	293
#define	LE	294
#define	GE	295
#define	OR	296
#define	AND	297
#define	PLUSPLUS	298
#define	MINUSMINUS	299
#define	LSHIFT	300
#define	RSHIFT	301
#define	URSHIFT	302
#define	PLUSEQUAL	303
#define	MINUSEQUAL	304
#define	MULTEQUAL	305
#define	DIVEQUAL	306
#define	LSHIFTEQUAL	307
#define	RSHIFTEQUAL	308
#define	URSHIFTEQUAL	309
#define	ANDEQUAL	310
#define	MODEQUAL	311
#define	XOREQUAL	312
#define	OREQUAL	313
#define	NUMBER	314
#define	IDENT	315
#define	AUTO	316


extern YYSTYPE kjsyylval;
