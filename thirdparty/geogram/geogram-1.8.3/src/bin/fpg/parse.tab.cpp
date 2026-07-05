/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 9 "parse.y" /* yacc.c:339  */

#define YYERROR_VERBOSE

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <sstream>
#include <FPG/AST.h>
#include <FPG/MSG.h>
#include <FPG/Declarator.h>
#include <FPG/SymbolEnvironment.h>

extern int lineno;
extern AST::TranslationUnit *translation_unit;

typedef std::list< std::string > StringContainer;
typedef std::list< Declarator* > DeclaratorList;

// function forward declarations
extern int yylex();

void yyerror (char *s);

void yyerror (const std::string& s);
void yywarning( const std::string& s );


AST::Expression*
makeFunctionCall( const std::string &id, AST::ExpressionList *l = nullptr );

static Type *base_type = nullptr;

static Type            *init_type   = nullptr;
static AST::Expression *init_target = nullptr;
static EnumType* current_enum;
static bool is_inline = false;
static bool is_extern = false;
static bool is_exact  = false;

struct Group_rep {
    unsigned int           degree;
    std::list<std::string> variables;
};

#line 112 "parse.tab.c" /* yacc.c:339  */

# ifndef YY_nullptrPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_nullptrPTR nullptr
#  else
#   define YY_nullptrPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "parse.tab.h".  */
#ifndef YY_YY_PARSE_TAB_H_INCLUDED
# define YY_YY_PARSE_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    PTR_OP = 258,
    INC_OP = 259,
    DEC_OP = 260,
    LEFT_OP = 261,
    RIGHT_OP = 262,
    LE_OP = 263,
    GE_OP = 264,
    EQ_OP = 265,
    NE_OP = 266,
    AND_OP = 267,
    OR_OP = 268,
    MUL_ASSIGN = 269,
    DIV_ASSIGN = 270,
    MOD_ASSIGN = 271,
    ADD_ASSIGN = 272,
    SUB_ASSIGN = 273,
    LEFT_ASSIGN = 274,
    RIGHT_ASSIGN = 275,
    AND_ASSIGN = 276,
    XOR_ASSIGN = 277,
    OR_ASSIGN = 278,
    INT = 279,
    FLOAT = 280,
    DOUBLE = 281,
    VOID = 282,
    TYPEDEF = 283,
    INLINE = 284,
    BOOL = 285,
    IF = 286,
    ELSE = 287,
    RETURN = 288,
    SIGN = 289,
    ABS = 290,
    SQRT = 291,
    COMPARE = 292,
    ENUM = 293,
    EXTERN = 294,
    EXACT = 295,
    GROUP = 296,
    DEGREE = 297,
    INT_CONSTANT = 298,
    FLOAT_CONSTANT = 299,
    IDENTIFIER = 300,
    STRING_LITERAL = 301,
    USER_TYPE = 302
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 56 "parse.y" /* yacc.c:355  */

  AST::Expression* exp;
  AST::Statement* stm;
  AST::CompoundStatement* compound_stm;
  AST::ExpressionStatement* exp_stm;
  AST::StatementList* statements;
  AST::VariableDeclaration *var_decl;
  AST::StatementList* stm_list;


  //AST::SwitchStatement* switch_stm;
  //AST::WhileLoop* while_loop;
  //AST::DoLoop* do_loop;
  //AST::ForLoop* for_loop;
  //AST::LabelContext* label_context;

  Declarator *declarator;
  FunctionDeclarator::ParameterDeclList *param_decl_list;
  FunctionDeclarator::ParameterDecl *param_decl;
  std::list< Declarator* > *decl_list;
  AST::ExpressionList *exp_list;

  AST::BinaryExpression::Kind binexp_kind;
  AST::UnaryExpression::Kind unexp_kind;
  AST::AssignmentExpression::Kind ass_exp_kind;
  AST::FunctionDefinition *fun_def;
  //AST::SwitchStatement::SwitchCase *switch_case;
  //AST::SwitchStatement::CaseContainer *case_container;
  FunctionType *fun_type;
  //StructType *struct_type;

  Type* type;
  int int_const;
  double float_const;
  char* string_const;

  Group_rep* single_group;
  std::list< Group_rep* > *group_list;

#line 240 "parse.tab.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSE_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 257 "parse.tab.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  31
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   270

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  66
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  54
/* YYNRULES -- Number of rules.  */
#define YYNRULES  121
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  193

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   302

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    53,     2,     2,     2,     2,     2,     2,
      48,    49,    54,    51,    50,    52,     2,    55,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    59,    61,
      56,    60,    57,    58,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    64,     2,    65,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    62,     2,    63,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   139,   139,   150,   151,   153,   155,   159,   161,   163,
     165,   167,   169,   171,   184,   189,   197,   198,   218,   219,
     220,   225,   232,   233,   238,   239,   244,   245,   250,   251,
     255,   256,   261,   262,   263,   264,   268,   269,   271,   276,
     277,   282,   283,   289,   290,   295,   296,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   313,   318,   319,   320,
     321,   322,   326,   328,   333,   338,   341,   340,   351,   353,
     358,   359,   363,   365,   375,   380,   384,   386,   391,   393,
     392,   411,   421,   422,   423,   424,   425,   426,   431,   430,
     441,   442,   446,   452,   462,   490,   494,   495,   496,   498,
     504,   509,   513,   522,   523,   528,   532,   539,   546,   550,
     557,   558,   563,   562,   622,   623,   627,   628,   629,   630,
     631,   636
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "PTR_OP", "INC_OP", "DEC_OP", "LEFT_OP",
  "RIGHT_OP", "LE_OP", "GE_OP", "EQ_OP", "NE_OP", "AND_OP", "OR_OP",
  "MUL_ASSIGN", "DIV_ASSIGN", "MOD_ASSIGN", "ADD_ASSIGN", "SUB_ASSIGN",
  "LEFT_ASSIGN", "RIGHT_ASSIGN", "AND_ASSIGN", "XOR_ASSIGN", "OR_ASSIGN",
  "INT", "FLOAT", "DOUBLE", "VOID", "TYPEDEF", "INLINE", "BOOL", "IF",
  "ELSE", "RETURN", "SIGN", "ABS", "SQRT", "COMPARE", "ENUM", "EXTERN",
  "EXACT", "GROUP", "DEGREE", "INT_CONSTANT", "FLOAT_CONSTANT",
  "IDENTIFIER", "STRING_LITERAL", "USER_TYPE", "'('", "')'", "','", "'+'",
  "'-'", "'!'", "'*'", "'/'", "'<'", "'>'", "'?'", "':'", "'='", "';'",
  "'{'", "'}'", "'['", "']'", "$accept", "identifier_expression",
  "primary_expression", "postfix_expression", "argument_expression_list",
  "unary_expression", "unary_operator", "cast_expression",
  "multiplicative_expression", "binary_op_kind_mul", "additive_expression",
  "binary_op_kind_add", "relational_expression", "binary_op_kind_rel",
  "equality_expression", "logical_and_expression", "logical_or_expression",
  "conditional_expression", "assignment_expression", "assignment_operator",
  "expression", "statement", "statement_list", "expression_statement",
  "compound_statement", "$@1", "conditional_statement", "jump_statement",
  "declaration", "declaration_specifiers", "init_declarator_list",
  "init_declarator", "$@2", "initializer", "type_specifier",
  "enum_specifier", "$@3", "enumerator_list", "enumerator", "declarator",
  "declarator2", "direct_declarator", "parameter_list",
  "parameter_declaration", "group_options", "group_var_list",
  "single_group", "group_definition2", "group_definition",
  "function_definition", "@4", "translation_unit", "external_declaration",
  "program", YY_nullptrPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,    40,    41,
      44,    43,    45,    33,    42,    47,    60,    62,    63,    58,
      61,    59,   123,   125,    91,    93
};
# endif

#define YYPACT_NINF -151

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-151)))

#define YYTABLE_NINF -111

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     167,  -151,  -151,  -151,  -151,   185,  -151,  -151,    10,  -151,
    -151,  -151,  -151,   -18,  -151,   -33,  -151,   167,  -151,    35,
      23,  -151,    27,   -18,   -10,  -151,    14,   -28,  -151,  -151,
    -151,  -151,    15,    17,    20,    28,   -18,  -151,  -151,    18,
    -151,  -151,    40,  -151,    39,  -151,   -18,   -17,  -151,  -151,
    -151,  -151,   202,    45,    43,  -151,    29,    30,   -26,  -151,
    -151,  -151,   185,    41,    44,    50,    53,  -151,  -151,    60,
     202,  -151,  -151,  -151,  -151,  -151,  -151,     0,   222,  -151,
      -6,     2,     1,    51,    97,    -2,  -151,  -151,  -151,    55,
    -151,   -25,    56,  -151,    75,    39,  -151,  -151,   202,   202,
     202,   202,   182,  -151,  -151,    76,  -151,  -151,  -151,  -151,
    -151,  -151,  -151,  -151,  -151,   202,  -151,  -151,  -151,   202,
    -151,  -151,   202,  -151,  -151,  -151,  -151,   202,   202,   202,
     202,   202,   202,    81,  -151,  -151,  -151,   117,  -151,  -151,
      77,    78,    79,    80,  -151,    21,  -151,  -151,  -151,  -151,
      -6,     2,     1,     1,    51,    97,    70,    68,    86,   137,
      74,  -151,    69,  -151,  -151,  -151,  -151,  -151,   -18,  -151,
    -151,  -151,   202,  -151,   202,   202,  -151,   202,  -151,    85,
    -151,  -151,  -151,    87,  -151,  -151,   100,  -151,  -151,   117,
     124,   117,  -151
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    83,    84,    85,    82,     0,   118,    86,     0,   119,
     120,    87,   117,     0,    75,     0,   116,   121,   114,     0,
       0,    88,    96,     0,     0,    76,    78,    94,    95,    74,
     115,     1,     0,     0,     0,     0,     0,    72,    79,   103,
     108,   111,   112,    73,     0,    99,     0,     0,   100,    97,
      77,    94,     0,     0,     0,   109,     0,    92,     0,    90,
     102,    98,     0,     0,     0,     0,     0,     4,     5,     2,
       0,    18,    19,    20,     3,     7,    16,    21,     0,    22,
      26,    30,    36,    39,    41,    43,    45,    81,    80,     0,
     105,     0,    66,   113,     0,     0,    89,   101,     0,     0,
       0,     0,     0,    21,    56,     0,    48,    49,    50,    51,
      52,    53,    54,    55,    47,     0,    17,    24,    25,     0,
      28,    29,     0,    34,    35,    32,    33,     0,     0,     0,
       0,     0,     0,     0,   106,   107,    65,     0,    93,    91,
       0,     0,     0,     0,    11,     0,    14,     6,    46,    23,
      27,    31,    37,    38,    40,    42,     0,     0,     0,     0,
       0,    62,     0,    60,    57,    59,    61,    58,     0,     9,
       8,    10,     0,    12,     0,     0,   104,     0,    70,     0,
      64,    67,    63,     0,    15,    44,     0,    71,    13,     0,
      68,     0,    69
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -151,  -151,  -151,    88,  -151,   -51,  -151,    38,    36,  -151,
      32,  -151,   -56,  -151,    33,    46,  -151,   -52,   -49,  -151,
     -94,  -150,  -151,  -151,   111,  -151,  -151,  -151,    25,    26,
    -151,   139,  -151,  -151,    -3,  -151,  -151,  -151,    83,  -151,
       6,  -151,  -151,   114,  -151,  -151,   141,  -151,  -151,  -151,
    -151,  -151,   169,  -151
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    74,    75,    76,   145,   103,    78,    79,    80,   119,
      81,   122,    82,   127,    83,    84,    85,   104,   160,   115,
     105,   161,   162,   163,   164,   137,   165,   166,   167,   168,
      24,    25,    52,    88,    14,    15,    33,    58,    59,    26,
      51,    28,    47,    48,    54,    91,    40,    41,    42,    16,
      56,    17,    18,    19
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      86,    77,    20,    87,   140,   141,   142,   143,   146,   123,
     124,   131,   182,    39,   106,   107,   108,   109,   110,    27,
     134,   111,   112,   113,    95,    12,    13,    22,    29,    35,
      23,    46,    61,    62,  -110,    31,   135,    96,   156,   190,
      36,   192,    12,    13,     1,     2,     3,     4,   117,   118,
       7,    37,    60,   120,   121,    21,   132,   125,   126,    46,
     114,   128,   129,    86,    77,   179,   148,    11,    32,    45,
     173,   174,   152,   153,    38,    34,    43,    49,   183,    44,
     184,    39,    53,   186,    57,    86,    77,    89,    90,    98,
      94,    92,    99,     1,     2,     3,     4,     5,   100,     7,
     158,   101,   159,    63,    64,    65,    66,     8,   102,   130,
      86,    77,    67,    68,    69,   133,    11,    70,   138,   136,
      71,    72,    73,   185,   157,   147,   169,   170,   171,   175,
     172,    92,   181,   176,   177,   180,   188,    86,    77,    86,
      77,     1,     2,     3,     4,     5,   187,     7,   158,   189,
     159,    63,    64,    65,    66,     8,   191,   149,   150,   151,
      67,    68,    69,   154,    11,    70,   116,    93,    71,    72,
      73,    63,    64,    65,    66,    50,    97,   155,   139,    92,
      67,    68,    69,    55,     0,    70,    30,     0,    71,    72,
      73,     1,     2,     3,     4,     5,     6,     7,   178,     0,
       0,     0,     0,     0,     0,     8,     9,    10,     0,     1,
       2,     3,     4,     0,    11,     7,    63,    64,    65,    66,
       0,     0,     0,     0,     0,    67,    68,    69,     0,     0,
      70,   144,    11,    71,    72,    73,    63,    64,    65,    66,
       0,     0,     0,     0,     0,    67,    68,    69,     0,     0,
      70,     0,     0,    71,    72,    73,    63,    64,    65,    66,
       0,     0,     0,     0,     0,    67,    68,    69,     0,     0,
      70
};

static const yytype_int16 yycheck[] =
{
      52,    52,     5,    52,    98,    99,   100,   101,   102,     8,
       9,    13,   162,    41,    14,    15,    16,    17,    18,    13,
      45,    21,    22,    23,    50,     0,     0,    45,    61,    23,
      48,    34,    49,    50,    62,     0,    61,    63,   132,   189,
      50,   191,    17,    17,    24,    25,    26,    27,    54,    55,
      30,    61,    46,    51,    52,    45,    58,    56,    57,    62,
      60,    10,    11,   115,   115,   159,   115,    47,    45,    49,
      49,    50,   128,   129,    60,    48,    61,    49,   172,    62,
     174,    41,    64,   177,    45,   137,   137,    42,    45,    48,
      60,    62,    48,    24,    25,    26,    27,    28,    48,    30,
      31,    48,    33,    34,    35,    36,    37,    38,    48,    12,
     162,   162,    43,    44,    45,    60,    47,    48,    43,    63,
      51,    52,    53,   175,    43,    49,    49,    49,    49,    59,
      50,    62,    63,    65,    48,    61,    49,   189,   189,   191,
     191,    24,    25,    26,    27,    28,    61,    30,    31,    49,
      33,    34,    35,    36,    37,    38,    32,   119,   122,   127,
      43,    44,    45,   130,    47,    48,    78,    56,    51,    52,
      53,    34,    35,    36,    37,    36,    62,   131,    95,    62,
      43,    44,    45,    42,    -1,    48,    17,    -1,    51,    52,
      53,    24,    25,    26,    27,    28,    29,    30,    61,    -1,
      -1,    -1,    -1,    -1,    -1,    38,    39,    40,    -1,    24,
      25,    26,    27,    -1,    47,    30,    34,    35,    36,    37,
      -1,    -1,    -1,    -1,    -1,    43,    44,    45,    -1,    -1,
      48,    49,    47,    51,    52,    53,    34,    35,    36,    37,
      -1,    -1,    -1,    -1,    -1,    43,    44,    45,    -1,    -1,
      48,    -1,    -1,    51,    52,    53,    34,    35,    36,    37,
      -1,    -1,    -1,    -1,    -1,    43,    44,    45,    -1,    -1,
      48
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    24,    25,    26,    27,    28,    29,    30,    38,    39,
      40,    47,    94,    95,   100,   101,   115,   117,   118,   119,
     100,    45,    45,    48,    96,    97,   105,   106,   107,    61,
     118,     0,    45,   102,    48,   106,    50,    61,    60,    41,
     112,   113,   114,    61,    62,    49,   100,   108,   109,    49,
      97,   106,    98,    64,   110,   112,   116,    45,   103,   104,
     106,    49,    50,    34,    35,    36,    37,    43,    44,    45,
      48,    51,    52,    53,    67,    68,    69,    71,    72,    73,
      74,    76,    78,    80,    81,    82,    83,    84,    99,    42,
      45,   111,    62,    90,    60,    50,    63,   109,    48,    48,
      48,    48,    48,    71,    83,    86,    14,    15,    16,    17,
      18,    21,    22,    23,    60,    85,    69,    54,    55,    75,
      51,    52,    77,     8,     9,    56,    57,    79,    10,    11,
      12,    13,    58,    60,    45,    61,    63,    91,    43,   104,
      86,    86,    86,    86,    49,    70,    86,    49,    84,    73,
      74,    76,    78,    78,    80,    81,    86,    43,    31,    33,
      84,    87,    88,    89,    90,    92,    93,    94,    95,    49,
      49,    49,    50,    49,    50,    59,    65,    48,    61,    86,
      61,    63,    87,    86,    86,    83,    86,    61,    49,    49,
      87,    32,    87
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    66,    67,    68,    68,    68,    68,    69,    69,    69,
      69,    69,    69,    69,    70,    70,    71,    71,    72,    72,
      72,    73,    74,    74,    75,    75,    76,    76,    77,    77,
      78,    78,    79,    79,    79,    79,    80,    80,    80,    81,
      81,    82,    82,    83,    83,    84,    84,    85,    85,    85,
      85,    85,    85,    85,    85,    85,    86,    87,    87,    87,
      87,    87,    88,    88,    89,    90,    91,    90,    92,    92,
      93,    93,    94,    94,    94,    95,    96,    96,    97,    98,
      97,    99,   100,   100,   100,   100,   100,   100,   102,   101,
     103,   103,   104,   104,   105,   106,   107,   107,   107,   107,
     108,   108,   109,   110,   110,   111,   111,   112,   113,   113,
     114,   114,   116,   115,   117,   117,   118,   118,   118,   118,
     118,   119
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     1,     3,     1,     4,     4,
       4,     3,     4,     6,     1,     3,     1,     2,     1,     1,
       1,     1,     1,     3,     1,     1,     1,     3,     1,     1,
       1,     3,     1,     1,     1,     1,     1,     3,     3,     1,
       3,     1,     3,     1,     5,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     2,     2,     0,     4,     5,     7,
       2,     3,     3,     4,     2,     1,     1,     3,     1,     0,
       4,     1,     1,     1,     1,     1,     1,     1,     0,     6,
       1,     3,     1,     3,     1,     1,     1,     3,     4,     3,
       1,     3,     2,     0,     5,     1,     2,     4,     1,     2,
       0,     1,     0,     5,     1,     2,     1,     1,     1,     1,
       1,     1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_nullptrPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_nullptrPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_nullptrPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 140 "parse.y" /* yacc.c:1646  */
    {
          Variable *var = symbol_env.findVariable( (yyvsp[0].string_const) );
          if( var == nullptr )
              yyerror( "variable " + std::string((yyvsp[0].string_const)) + " not declared" );

          (yyval.exp) = new AST::IdentifierExpression( var );
        }
#line 1520 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 152 "parse.y" /* yacc.c:1646  */
    { (yyval.exp) = new AST::LiteralExpression((yyvsp[0].int_const)); }
#line 1526 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 154 "parse.y" /* yacc.c:1646  */
    { (yyval.exp) = new AST::LiteralExpression((yyvsp[0].float_const)); }
#line 1532 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 155 "parse.y" /* yacc.c:1646  */
    { (yyval.exp) = (yyvsp[-1].exp); }
#line 1538 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 162 "parse.y" /* yacc.c:1646  */
    { (yyval.exp) = new AST::UnaryFunction( (yyvsp[-1].exp), AST::UnaryFunction::XABS ); }
#line 1544 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 164 "parse.y" /* yacc.c:1646  */
    { (yyval.exp) = new AST::UnaryFunction( (yyvsp[-1].exp), AST::UnaryFunction::XSIGN ); }
#line 1550 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 166 "parse.y" /* yacc.c:1646  */
    { (yyval.exp) = new AST::UnaryFunction( (yyvsp[-1].exp), AST::UnaryFunction::XSQRT ); }
#line 1556 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 168 "parse.y" /* yacc.c:1646  */
    { (yyval.exp) = makeFunctionCall( (yyvsp[-2].string_const) ); }
#line 1562 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 170 "parse.y" /* yacc.c:1646  */
    { (yyval.exp) = makeFunctionCall( (yyvsp[-3].string_const), (yyvsp[-1].exp_list) ); }
#line 1568 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 172 "parse.y" /* yacc.c:1646  */
    { (yyval.exp) = new AST::UnaryFunction(
                  new AST::BinaryExpression( (yyvsp[-3].exp), (yyvsp[-1].exp), AST::BinaryExpression::SUB ),
                  AST::UnaryFunction::XSIGN
                );
        }
#line 1578 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 185 "parse.y" /* yacc.c:1646  */
    {   AST::ExpressionList *l = new AST::ExpressionList();
            l->push_back( (yyvsp[0].exp) );
            (yyval.exp_list) = l;
        }
#line 1587 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 190 "parse.y" /* yacc.c:1646  */
    {   (yyvsp[-2].exp_list)->push_back( (yyvsp[0].exp) );
            (yyval.exp_list) = (yyvsp[-2].exp_list);
        }
#line 1595 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 199 "parse.y" /* yacc.c:1646  */
    { (yyval.exp) = new AST::UnaryExpression((yyvsp[0].exp),(yyvsp[-1].unexp_kind)); }
#line 1601 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 218 "parse.y" /* yacc.c:1646  */
    { (yyval.unexp_kind)= AST::UnaryExpression::PLUS; }
#line 1607 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 219 "parse.y" /* yacc.c:1646  */
    { (yyval.unexp_kind)= AST::UnaryExpression::NEG;  }
#line 1613 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 220 "parse.y" /* yacc.c:1646  */
    { (yyval.unexp_kind) = AST::UnaryExpression::NOT; }
#line 1619 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 234 "parse.y" /* yacc.c:1646  */
    { (yyval.exp) = new AST::BinaryExpression((yyvsp[-2].exp),(yyvsp[0].exp),(yyvsp[-1].binexp_kind)); }
#line 1625 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 238 "parse.y" /* yacc.c:1646  */
    { (yyval.binexp_kind) = AST::BinaryExpression::MUL; }
#line 1631 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 239 "parse.y" /* yacc.c:1646  */
    { (yyval.binexp_kind) = AST::BinaryExpression::DIV; }
#line 1637 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 246 "parse.y" /* yacc.c:1646  */
    { (yyval.exp) = new AST::BinaryExpression((yyvsp[-2].exp),(yyvsp[0].exp),(yyvsp[-1].binexp_kind)); }
#line 1643 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 250 "parse.y" /* yacc.c:1646  */
    { (yyval.binexp_kind) = AST::BinaryExpression::ADD; }
#line 1649 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 251 "parse.y" /* yacc.c:1646  */
    { (yyval.binexp_kind) = AST::BinaryExpression::SUB; }
#line 1655 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 257 "parse.y" /* yacc.c:1646  */
    { (yyval.exp) = new AST::BinaryExpression((yyvsp[-2].exp),(yyvsp[0].exp),(yyvsp[-1].binexp_kind)); }
#line 1661 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 261 "parse.y" /* yacc.c:1646  */
    { (yyval.binexp_kind) = AST::BinaryExpression::LE;  }
#line 1667 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 262 "parse.y" /* yacc.c:1646  */
    { (yyval.binexp_kind) = AST::BinaryExpression::GR;  }
#line 1673 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 263 "parse.y" /* yacc.c:1646  */
    { (yyval.binexp_kind) = AST::BinaryExpression::LEQ; }
#line 1679 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 264 "parse.y" /* yacc.c:1646  */
    { (yyval.binexp_kind) = AST::BinaryExpression::GEQ; }
#line 1685 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 270 "parse.y" /* yacc.c:1646  */
    { (yyval.exp) = new AST::BinaryExpression((yyvsp[-2].exp),(yyvsp[0].exp),AST::BinaryExpression::EQ); }
#line 1691 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 272 "parse.y" /* yacc.c:1646  */
    { (yyval.exp) = new AST::BinaryExpression((yyvsp[-2].exp),(yyvsp[0].exp),AST::BinaryExpression::NEQ); }
#line 1697 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 278 "parse.y" /* yacc.c:1646  */
    { (yyval.exp) = AST::makeBinaryExpression( (yyvsp[-2].exp), (yyvsp[0].exp), AST::BinaryExpression::AND ); }
#line 1703 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 284 "parse.y" /* yacc.c:1646  */
    { (yyval.exp) = AST::makeBinaryExpression( (yyvsp[-2].exp), (yyvsp[0].exp), AST::BinaryExpression::OR ); }
#line 1709 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 291 "parse.y" /* yacc.c:1646  */
    { (yyval.exp) = new AST::ConditionalExpression( (yyvsp[-4].exp), (yyvsp[-2].exp), (yyvsp[0].exp) ); }
#line 1715 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 297 "parse.y" /* yacc.c:1646  */
    { (yyval.exp) = new AST::AssignmentExpression((yyvsp[-2].exp),(yyvsp[0].exp),(yyvsp[-1].ass_exp_kind)); }
#line 1721 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 301 "parse.y" /* yacc.c:1646  */
    { (yyval.ass_exp_kind) = AST::AssignmentExpression::PLAIN; }
#line 1727 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 302 "parse.y" /* yacc.c:1646  */
    { (yyval.ass_exp_kind) = AST::AssignmentExpression::MUL;   }
#line 1733 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 303 "parse.y" /* yacc.c:1646  */
    { (yyval.ass_exp_kind) = AST::AssignmentExpression::DIV;   }
#line 1739 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 304 "parse.y" /* yacc.c:1646  */
    { (yyval.ass_exp_kind) = AST::AssignmentExpression::MOD;   }
#line 1745 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 305 "parse.y" /* yacc.c:1646  */
    { (yyval.ass_exp_kind) = AST::AssignmentExpression::ADD;   }
#line 1751 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 306 "parse.y" /* yacc.c:1646  */
    { (yyval.ass_exp_kind) = AST::AssignmentExpression::SUB;   }
#line 1757 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 307 "parse.y" /* yacc.c:1646  */
    { (yyval.ass_exp_kind) = AST::AssignmentExpression::AND;   }
#line 1763 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 308 "parse.y" /* yacc.c:1646  */
    { (yyval.ass_exp_kind) = AST::AssignmentExpression::XOR;   }
#line 1769 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 309 "parse.y" /* yacc.c:1646  */
    { (yyval.ass_exp_kind) = AST::AssignmentExpression::OR;    }
#line 1775 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 314 "parse.y" /* yacc.c:1646  */
    { (yyval.exp) = (yyvsp[0].exp); }
#line 1781 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 318 "parse.y" /* yacc.c:1646  */
    { (yyval.stm) = (yyvsp[0].compound_stm); }
#line 1787 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 321 "parse.y" /* yacc.c:1646  */
    { (yyval.stm)=dynamic_cast<AST::Statement*>((yyvsp[0].exp_stm)); }
#line 1793 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 327 "parse.y" /* yacc.c:1646  */
    { (yyval.statements) = new AST::StatementList(); (yyval.statements)->add((yyvsp[0].stm)); }
#line 1799 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 329 "parse.y" /* yacc.c:1646  */
    { (yyvsp[-1].statements)->add((yyvsp[0].stm)); (yyval.statements) = (yyvsp[-1].statements); }
#line 1805 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 334 "parse.y" /* yacc.c:1646  */
    { (yyval.exp_stm) = new AST::ExpressionStatement((yyvsp[-1].exp)); }
#line 1811 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 339 "parse.y" /* yacc.c:1646  */
    { (yyval.compound_stm) = new AST::CompoundStatement(new AST::StatementList()); }
#line 1817 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 341 "parse.y" /* yacc.c:1646  */
    {   symbol_env.enterBlock();  }
#line 1823 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 343 "parse.y" /* yacc.c:1646  */
    {
            symbol_env.leaveBlock();
            (yyval.compound_stm) = new AST::CompoundStatement((yyvsp[-1].statements));
        }
#line 1832 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 352 "parse.y" /* yacc.c:1646  */
    { (yyval.stm) = new AST::ConditionalStatement((yyvsp[-2].exp),(yyvsp[0].stm),nullptr); }
#line 1838 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 354 "parse.y" /* yacc.c:1646  */
    { (yyval.stm) = new AST::ConditionalStatement((yyvsp[-4].exp),(yyvsp[-2].stm),(yyvsp[0].stm)); }
#line 1844 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 358 "parse.y" /* yacc.c:1646  */
    { (yyval.stm) = new AST::Return(nullptr); }
#line 1850 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 359 "parse.y" /* yacc.c:1646  */
    { (yyval.stm) = new AST::Return((yyvsp[-1].exp));   }
#line 1856 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 364 "parse.y" /* yacc.c:1646  */
    { (yyval.stm) = (yyvsp[-1].statements);  }
#line 1862 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 366 "parse.y" /* yacc.c:1646  */
    {
            Type *t = (yyvsp[-2].type);
            AliasType *at = dynamic_cast<AliasType*>(t);
            if( at != nullptr )
                t = at->base_type;
            symbol_env.add( new AliasType( (yyvsp[-1].string_const), t ) );
            //$$ = new AST::PlainText( std::string("typedef ") + $2->name() + " " + $3 + ";" );
            (yyval.stm) = new AST::EmptyStatement();
        }
#line 1876 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 376 "parse.y" /* yacc.c:1646  */
    { (yyval.stm) = new AST::EmptyStatement(); }
#line 1882 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 380 "parse.y" /* yacc.c:1646  */
    { base_type = (yyvsp[0].type); }
#line 1888 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 385 "parse.y" /* yacc.c:1646  */
    { (yyval.statements) = new AST::StatementList(); (yyval.statements)->add( (yyvsp[0].stm) ); }
#line 1894 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 387 "parse.y" /* yacc.c:1646  */
    { (yyvsp[-2].statements)->add( (yyvsp[0].stm) ); (yyval.statements) = (yyvsp[-2].statements); }
#line 1900 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 393 "parse.y" /* yacc.c:1646  */
    {
            Variable* var = dynamic_cast<AST::VariableDeclaration*>((yyvsp[-1].stm))->var;
            init_type   = var->type;
            init_target = new AST::IdentifierExpression(var);
        }
#line 1910 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 399 "parse.y" /* yacc.c:1646  */
    {
            AST::StatementList *l = new AST::StatementList();
            l->add( (yyvsp[-3].stm) );
            l->add( (yyvsp[0].stm) );
            (yyval.stm) = l;
        }
#line 1921 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 412 "parse.y" /* yacc.c:1646  */
    {
            AST::AssignmentExpression *e = new AST::AssignmentExpression(init_target,(yyvsp[0].exp));
            (yyval.stm) = new AST::ExpressionStatement(e);
            init_type = nullptr;
            init_target = nullptr;
        }
#line 1932 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 421 "parse.y" /* yacc.c:1646  */
    { (yyval.type) = type_void;  }
#line 1938 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 422 "parse.y" /* yacc.c:1646  */
    { (yyval.type) = type_int;   }
#line 1944 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 423 "parse.y" /* yacc.c:1646  */
    { (yyval.type) = type_float; }
#line 1950 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 424 "parse.y" /* yacc.c:1646  */
    { (yyval.type) = type_float; }
#line 1956 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 425 "parse.y" /* yacc.c:1646  */
    { (yyval.type) = type_bool;  }
#line 1962 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 426 "parse.y" /* yacc.c:1646  */
    { (yyval.type) = (yyvsp[0].type);      }
#line 1968 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 431 "parse.y" /* yacc.c:1646  */
    {
           current_enum = new EnumType((yyvsp[0].string_const));
        }
#line 1976 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 435 "parse.y" /* yacc.c:1646  */
    {
            symbol_env.add( new AliasType( current_enum->id, type_int ) );
        }
#line 1984 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 447 "parse.y" /* yacc.c:1646  */
    {
            Variable *v = new Variable( (yyvsp[0].string_const), type_int );
            v->make_constant( current_enum->add_value((yyvsp[0].string_const)) );
            symbol_env.add( v );
        }
#line 1994 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 453 "parse.y" /* yacc.c:1646  */
    {
            Variable *v = new Variable( (yyvsp[-2].string_const), type_int );
            v->make_constant( current_enum->add_value((yyvsp[-2].string_const),(yyvsp[0].int_const)) );
            symbol_env.add( v );
        }
#line 2004 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 463 "parse.y" /* yacc.c:1646  */
    {
            std::string id;
            Type *type = (yyvsp[0].declarator)->declare( base_type, id );
            FunctionType* fun_type = dynamic_cast<FunctionType*>(type);
            if( fun_type != nullptr ) {
                if( symbol_env.findFunction( type->id, (unsigned int)(fun_type->parameters.size()) ) != nullptr )
                    yyerror( std::string( "function " ) + type->id + std::string( " already declared" ) );
                else {
                    if( is_extern && is_float(fun_type->return_type) && !is_exact )
                        yyerror( std::string("external functions with floating point result type need to be marked \"exact\"" ) );
                    fun_type->is_inline = is_inline;
                    fun_type->is_extern = is_extern;
                    is_inline = false;
                    is_extern = false;
                    is_exact  = false;
                    symbol_env.add( fun_type );
                }
                (yyval.stm) = new AST::EmptyStatement;
            } else {
                Variable *var = new Variable( id, type );
                symbol_env.add( var );
                (yyval.stm) = new AST::VariableDeclaration( var );
            }
        }
#line 2033 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 494 "parse.y" /* yacc.c:1646  */
    { (yyval.declarator) = new IdentifierDeclarator( (yyvsp[0].string_const) ); }
#line 2039 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 495 "parse.y" /* yacc.c:1646  */
    { (yyval.declarator) = (yyvsp[-1].declarator); }
#line 2045 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 497 "parse.y" /* yacc.c:1646  */
    { (yyval.declarator) = new FunctionDeclarator( new IdentifierDeclarator( (yyvsp[-3].string_const) ), (yyvsp[-1].param_decl_list) ); }
#line 2051 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 499 "parse.y" /* yacc.c:1646  */
    { (yyval.declarator) = new FunctionDeclarator( new IdentifierDeclarator( (yyvsp[-2].string_const) ) ); }
#line 2057 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 505 "parse.y" /* yacc.c:1646  */
    {
            (yyval.param_decl_list) = new FunctionDeclarator::ParameterDeclList();
            (yyval.param_decl_list)->push_back( (yyvsp[0].param_decl) );
        }
#line 2066 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 509 "parse.y" /* yacc.c:1646  */
    { (yyvsp[-2].param_decl_list)->push_back( (yyvsp[0].param_decl) ); (yyval.param_decl_list) = (yyvsp[-2].param_decl_list); }
#line 2072 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 514 "parse.y" /* yacc.c:1646  */
    { (yyval.param_decl) = new FunctionDeclarator::ParameterDecl( (yyvsp[-1].type), (yyvsp[0].declarator) ); }
#line 2078 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 522 "parse.y" /* yacc.c:1646  */
    { (yyval.int_const) = 1; }
#line 2084 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 524 "parse.y" /* yacc.c:1646  */
    { (yyval.int_const) = (yyvsp[-1].int_const); }
#line 2090 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 528 "parse.y" /* yacc.c:1646  */
    {
          (yyval.single_group) = new Group_rep;
          (yyval.single_group)->variables.push_back( (yyvsp[0].string_const) );
      }
#line 2099 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 532 "parse.y" /* yacc.c:1646  */
    {
          (yyvsp[-1].single_group)->variables.push_back( (yyvsp[0].string_const) );
          (yyval.single_group) = (yyvsp[-1].single_group);
      }
#line 2108 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 539 "parse.y" /* yacc.c:1646  */
    {
        (yyval.single_group) = (yyvsp[-1].single_group);
        (yyval.single_group)->degree = (unsigned int)((yyvsp[-2].int_const));
      }
#line 2117 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 546 "parse.y" /* yacc.c:1646  */
    {
          (yyval.group_list) = new std::list< Group_rep* >;
          (yyval.group_list)->push_back( (yyvsp[0].single_group) );
      }
#line 2126 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 550 "parse.y" /* yacc.c:1646  */
    {
          (yyvsp[-1].group_list)->push_back( (yyvsp[0].single_group) );
          (yyval.group_list) = (yyvsp[-1].group_list);
      }
#line 2135 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 557 "parse.y" /* yacc.c:1646  */
    { (yyval.group_list) = nullptr; }
#line 2141 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 558 "parse.y" /* yacc.c:1646  */
    { (yyval.group_list) = (yyvsp[0].group_list);   }
#line 2147 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 563 "parse.y" /* yacc.c:1646  */
    {
            std::string id;
            Type *type = (yyvsp[-1].declarator)->declare( (yyvsp[-2].type), id );
            FunctionType *fun_type = dynamic_cast<FunctionType*>(type);
            if( fun_type ) {
                // "replace_function_type" is
                // similar to "add", but doesnt care if fun_type was already declared.
                // important: previous declarations might have different variable names!
                // which do not match the function definition body. so, we prefer
                // the most recent type, which has the correct variable names
                symbol_env.replace_function_type( fun_type );
            } else
                yyerror( std::string("internal error: invalid function definition!" ) );
            fun_type->is_inline = is_inline;
            is_inline = false;
            if( is_extern )
                yyerror( std::string("external function definition does not make sense" ) );
            (yyval.fun_type) = fun_type;

            symbol_env.enterBlock();
            FOREACH_PARAMETER(fun_type) {
                Variable *var = *it;
                // array arguments are supplied by reference.

                symbol_env.add( var );
            }
            if( (yyvsp[0].group_list) != nullptr ) {
                unsigned int group_index = 1;
                std::list< Group_rep* >::iterator it;
                for( it  = (yyvsp[0].group_list)->begin(); it != (yyvsp[0].group_list)->end(); ++it, ++group_index )
                {
                    unsigned int degree = (*it)->degree;
                    for( std::list<std::string>::iterator
                        it2  = (*it)->variables.begin();
                        it2 != (*it)->variables.end();
                        ++it2)
                    {
                        //std::cout << *it2 << " ";
                        Variable *var = symbol_env.findVariable( *it2 );
                        if( var == nullptr )
                            yyerror( std::string("undefined variable ") + *it2 );
                        var->group_index = group_index;
                        var->degree      = degree;
                    }
                    //std::cout << std::endl;
                }
            }

        }
#line 2201 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 613 "parse.y" /* yacc.c:1646  */
    {
            symbol_env.leaveBlock();
            AST::FunctionDefinition *fun_def = new AST::FunctionDefinition( (yyvsp[-1].fun_type), (yyvsp[0].compound_stm) );
            symbol_env.add_fun_def( fun_def );
            (yyval.fun_def) = fun_def;
        }
#line 2212 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 627 "parse.y" /* yacc.c:1646  */
    { translation_unit->add( (yyvsp[0].fun_def) ); }
#line 2218 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 628 "parse.y" /* yacc.c:1646  */
    { translation_unit->add( (yyvsp[0].stm) ); }
#line 2224 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 629 "parse.y" /* yacc.c:1646  */
    { is_inline = true; }
#line 2230 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 630 "parse.y" /* yacc.c:1646  */
    { is_extern = true; }
#line 2236 "parse.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 631 "parse.y" /* yacc.c:1646  */
    { is_exact  = true; }
#line 2242 "parse.tab.c" /* yacc.c:1646  */
    break;


#line 2246 "parse.tab.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (std::string("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 639 "parse.y" /* yacc.c:1906  */


extern char yytext[];
extern int column;


void yyerror(char *s)
{
  throw ParseError (s, AST::current_location);
}

void yyerror(const std::string& s) {
  throw ParseError (s, AST::current_location);
}

void yywarning( const std::string& s ) {
  std::cerr << "warning: " << AST::current_location.toString() << " " << s << std::endl;
}

inline
AST::Expression*
makeFunctionCall( const std::string &id, AST::ExpressionList *l ) {
    if( l == nullptr )
        l = new AST::ExpressionList();
    FunctionType *fun_type = symbol_env.findFunction( id, (unsigned int)(l->size()) );
    if( fun_type == nullptr ) {
        yyerror( std::string("function ") + id + " not declared!" );
    }
    return new AST::FunctionCall( fun_type, l );
}


