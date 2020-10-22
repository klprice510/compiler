/*  Intermediate Code
 *  Homework 4
 *  CS 445, Dr. Jeffery
 *  Nov. 24, 2014
 *  Kathryn Price
 */

//#include "precode.h"

/* short, long and const are all no-ops.  We ignore these key words */
enum base_type
{
    INT_TYPE,       /* 0 int */
    BOOL_TYPE,      /* 1 bool */
    FLOAT_TYPE,     /* 2 float, double */
    CHAR_TYPE,      /* 3 char */
    NULL_TYPE,      /* 4 void */
    CLASS_TYPE,     /* 5 */
    NOTUSED_TYPE1,  /* 6 */
    PTR_TYPE,       /* 7 */
    ARR_TYPE,       /* 8 */
    FUNC_TYPE,      /* 9 */
    PARAM_TYPE,     /* 10 */
    UNKNOWN_TYPE,   /* 11 */
    CLASSMEM_TYPE,  /* 12 member of the class, method or variable */
    CONSTR_TYPE,    /* 13 constructor uses the function struct in the union*/
    NAMESPACE_TYPE, /* 14 */
    STRUCT_TYPE     /* 15 */
};

/* Tells parse tree which methods/functions have what access. */
/* Is done prior to filling in sym tab and should help with that process. */
enum access_type
{
    UNKNOWN_ACCESS,
    PUBLIC_TYPE,    
    PRIVATE_TYPE
};

/* lexical data */
struct token
{
  int category;    /* the integer code returned by yylex */
  char *text;      /* the actual string, lexeme matched */
  int lineno;      /* the line number on which the token occurs */
  char *filename;  /* the source file in which the token occurs */
  int ival;        /* store int values here */
  char *sval;      /* store null terminated string value here */
  char *classname;  /* store potential classname if needed */
};

struct tokenlist
{
  struct token *t;
  struct tokenlist *next;
};

struct addr
{ /* address for variable */
    int region;
    union
    {
        int offset;
        float foffset;
        char *name;   
    } u;
    int size;
};

/* parser data */
typedef struct tree
{
  int prodrule; /* rule number of production */
  int nkids;
  char *filename;
  int line_num; /* line of filename being processed */
  struct sym_table *st; /* used for denoting scope change */
  int prototype; /* if only prototype = 1 */
  char *possible_classname;
  struct addr final;
  struct typeinfo *typ;
  struct taclist *code;
  struct tree *parent; /* pointer to my parent, if NULL then at the top of the tree */
  struct token *leaf;
  struct tree *kids[1];
} parsetree;


/* type table data */
#define TYPEHASHSIZE 211

struct typenametable_entry 
{
   char *name;
   int category;
   int key;
   struct sym_table *st; /* used for defining classes */
   struct typenametable_entry *next;
};

struct hash 
{
    struct typenametable_entry *head;
    int count;
};

struct hash *identhashTable;

/* symbol table data */
#define SYMHASHSIZE 211

struct typeinfo 
{
   enum base_type basetype;
   char *name;
   int isConst;
   struct sym_table *st; /* type "owns" this space */
   struct sym_table *i_am_defined_here;
   char *myclass;
   int is5651; /* rule flags used for type checking */
   int is5652; /* rule flags used for type checking */
   int isProto; /* used by func/class to determine if param names are valid, mem allocation */
   int isParam; /* > 0 if parameter, also doubles as parameter count from left side (begin at 1) */
   char *pcn;
   struct addr memaddr;
   union
   {
       int i;
       char c;
       float f;
       char *str;
   } const_val;
   union 
   {
      struct classinfo 
      {
        struct sym_table *st;
        struct classmember *member;
        char *classname;
      }c;
      struct ptrinfo
      {
        struct typeinfo *elemtype;   
      }p;
      struct arrayinfo 
      {
         int size;
         struct typeinfo *elemtype;
      }a;
      struct funcinfo
      {
         struct sym_table *st;
         struct typeinfo *returntype;
         struct param *parameters; 
         char *classname;
         int paramsize; /* neg param size needs to be added to typeinfo->st size */
         int nameok;
      }f;
      struct param 
      {
         struct typeinfo *type;
         struct param *next;
         int cnt;
      }plist;
      struct classmember
      {
        struct typeinfo *type;
        struct classmember *next;   
        enum access_type access;
      }cm;
   } u;
};

struct sym_table
{
    struct sym_entry *head;
    struct sym_table *parent; /* enclosing sym tab, null if none */
    int count;
    int offset;
};

struct sym_entry
{
    char *name;
    enum base_type basetype;
    struct typeinfo *t;
    struct sym_table *st;
    struct sym_entry *next;
};

struct sym_list
{
    struct sym_table *curr; /* points to sym tab */
    struct sym_list *next;
};


struct tac
{ /* instruction */
    int op;
    struct addr *address[3]; /* 0 = dest, 1 = src, 2 = src */
};

struct taclist
{ /* list of instructions */
    struct tac *instruct;
    struct taclist *next;
    struct taclist *prev;
};

