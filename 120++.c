/*  Intermediate Code
 *  Homework 4
 *  CS 445, Dr. Jeffery
 *  Nov. 24, 2014
 *  Kathryn Price
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "120++.h"
#include "parser.tab.h"
#include "rules.h"

int semantic_debug = 0;
int parser_debug = 0;

extern int searchInHash(char *s);
struct typeinfo *check_io(parsetree *n1, parsetree *n2, struct sym_table *st);
extern int lineno;
extern char *curfilename;
extern int namespace_found;
extern int included_fstream;
extern int included_string;
extern int included_iostream;
extern FILE *yyin;
extern char *yytext;
extern int newfile(char *fn);
extern struct tokenlist *tlist;
extern struct token *yytoken;
void print_list(void);
extern void print_ident_list(void);
parsetree *parse_tree; /* current parse tree being worked on */
parsetree *pa[501]; /* list of parse trees */
int pa_start = 0;
int pa_end = 0;
int trim_tree(void);
extern void printer(parsetree *t, int depth);
extern void init_rules(void);
extern int exit_num;
extern char *readable_rule(int rule);
void FindTypes(parsetree *n);
void TypesToSibs(parsetree *n, struct typeinfo *t);
void FinshClassDefines(parsetree *n);
struct typeinfo *make_room(enum base_type basetype);
struct param *next_param(void);
struct typeinfo *next_type(void);
struct sym_table *symtable(int size);
struct sym_entry *next_sym(void);
extern int hash_key(char *s);
struct sym_entry *newsym_node(struct sym_table *symt, struct typeinfo *t);
void sym_insert(struct sym_table *symt, struct typeinfo *t);
enum base_type search_sym(struct sym_table *symt, char *s, enum base_type whatiam);
struct typeinfo * search_type(struct sym_table *st, char *s);
struct classmember *next_mem(void);
void printer_debug(parsetree *t, int depth);
char *copy_name(char *s);
void FillSymTabs(parsetree *n, struct sym_table *st, int inclass);
void UndefineCheck(parsetree *n, struct sym_table *st, int dotfound);
void TypeCheck(parsetree *n, struct sym_table *st);
struct sym_table *symtab;
void prefill_libs(int lib);
extern void semantic_error(char *s);
struct typeinfo *check_asn(parsetree *n1, parsetree *n2, struct sym_table *st);
struct typeinfo *check_param(parsetree *n1, parsetree *n2, struct sym_table *st);
struct typeinfo *check_uni(parsetree *n, struct sym_table *st);
struct typeinfo *check_dot(parsetree *n, struct sym_table *st);
char *build_errstr(char *ename, char *e2);
struct typeinfo *FindLexElemNode(parsetree *n, struct sym_table *st);
struct typeinfo *scroll_search(struct sym_table *st, char *s);
struct typeinfo *get_my_type(parsetree *n, struct sym_table *st);
parsetree *FindNode(parsetree *n, struct sym_table *st, int rule);
extern void ic_gen(void);
void show_symtabs(void);
void print_symentry(struct sym_entry *entry);
extern struct sym_list *symtablist;
extern struct sym_list *mem_loc(struct sym_table *st);
int paramlist = 0; /* used to get parameter count */
int inparamlist = 0; /* flag used to see if we're in a paramlist */
void PushPrototypes(parsetree *n, struct typeinfo *t);
extern struct addr *NullAddr;
struct addr MallocAddr; 
void PushPcn(parsetree *n, struct sym_table *st);

/* Return values:
 * 1 = lexical error
 * 2 = syntax error
 * 3 = semantic or C++ operation not supported in 120++ 
 * 10 = out of memory
 * 11 = file error
 */
int main(int argc, char **argv)
{
  int i;
  /* Skip program name at index 0 */
  if(argc == 1 || argc > 500)
  { /* no file specified */
    fprintf(stderr, "ERROR: no input file specified or more that 500 files specified.\n");
    fprintf(stderr, "Usage: 120++ [f1] [f2] ...\n");
    fprintf(stderr, "\twhere [f1], [f2], etc. are files to be processed.\n");
    exit(11);
  }
  /* create type hash table */
  identhashTable = (struct hash *)calloc(TYPEHASHSIZE, sizeof(struct hash));
  if(identhashTable == NULL)
  {
    fprintf(stderr, "ERROR: out of memory\n"); 
    exit(10);       
  }        
  if(parser_debug == 1)
  { /* this is only to create readable rules */
    /* if we're not printing the parse tree don't do */
    init_rules();
  }
  /* if we need libraries, process now & put in parse tree array */
  /* we'll decide later if they need to be trimmed from the array */
  prefill_libs(ADDALLLIBS); /* this prefills iostream library */
  if(!newfile("~120lib.cpp"))
  { /* file doesn't exist */
    fprintf(stderr, "ERROR: unable to open file %s.\n", "~120lib.cpp");
    exit(11);
  }
  yyparse();
  pa[pa_end] = parse_tree;
  pa_end++;
  prefill_libs(ADDSTRLIBS); /* this prefills string library */
  if(!newfile("~120lib.cpp"))
  { /* file doesn't exist */
    fprintf(stderr, "ERROR: unable to open file %s.\n", "~120lib.cpp");
    exit(11);
  } 
  lineno = 1;
  yyparse();
  pa[pa_end] = parse_tree;
  pa_end++;
  remove("~120lib.cpp"); /* get rid of file */
  for(i = 1; i < argc; i++)
  { /* File handling */
    if(!newfile(argv[i]))
    { /* file doesn't exist */
      fprintf(stderr, "ERROR: unable to open file %s.\n", argv[i]);
      exit(11);
    } 
    fprintf(stdout, "Parsing file %s\n", argv[i]);
    curfilename = argv[i];
    lineno = 1;
    yyparse(); /* call parser */
    pa[pa_end] = parse_tree;
    pa_end++;
    /* commented out print_list for output readability */ 
    /* print linked list of tokens */
    //print_list();
    if(exit_num == 0)
    {   /* commented out prints for output readability */
        //print_ident_list();
        //printer(parse_tree, 1);
        //fprintf(stdout, "Finished parsing file: %s\n", argv[i]);
    }
  }
  /* Now we've gone through all the files, let's see if we have the
   * namespace and library includes for the libraries */
  pa_start = trim_tree();
  if(semantic_debug == 1 && parser_debug == 1)
  {
      for(i = pa_start; i < pa_end; i++)
      {
        parse_tree = pa[i];  
        printer_debug(parse_tree, 1);
      }
  }
  else
  {
      for(i = pa_start; i < pa_end; i++)
      {
          parse_tree = pa[i];
          curfilename = parse_tree->filename;
          FindTypes(parse_tree); /* Fill in types on parse tree */
          /* Push type info to kids (as determined above from siblings) */
          TypesToSibs(parse_tree, NULL);  
          FinshClassDefines(parse_tree);
      }
      symtab = symtable(SYMHASHSIZE); /* global sym tab */
      for(i = pa_start; i < pa_end; i++)
      {
          parse_tree = pa[i];
          curfilename = parse_tree->filename;
          FillSymTabs(parse_tree, symtab, 0); /* fill all sym tabs */
          PushPcn(parse_tree, symtab);
      }
      for(i = pa_start; i < pa_end; i++)
      {
          parse_tree = pa[i];
          UndefineCheck(parse_tree, symtab, 0); /* checks for undefined variables */
          /* at this point, all is good... now check that expressions have valid types */
          TypeCheck(parse_tree, symtab);
          //printer(parse_tree, 1);
      }
      symtablist = mem_loc(symtab); /* load loc regions/offset & size for all sym tab entries */
      NullAddr = (struct addr *)calloc(1, sizeof(struct addr)); /* used for blank address */
      MallocAddr.u.name = copy_name("malloc");
      if(NullAddr == NULL)
      {
        fprintf(stderr, "ERROR: out of memory\n"); 
        exit(10);       
      }
      /* intermediate code generation */
      for(i = pa_start; i < pa_end; i++)
      { /* let's do intermediate code generation */
          parse_tree = pa[i];
          curfilename = parse_tree->filename;
          ic_gen();
      }
      show_symtabs(); /* if debug then prints sym tabs' contents */
  }
  fprintf(stdout, "\nFinished processing all files. Thank you.\n\n");
  /* Returns: zero with no errors, nonzero with errors */
  return exit_num;
}

void PushPcn(parsetree *n, struct sym_table *st)
{
    int i;
    struct typeinfo *tfound;
    struct sym_table *nextst;
    nextst = st;
    if(n->typ != NULL && n->typ->st != NULL)
    {
        nextst = n->typ->st;
    }
    if(n == NULL)
    {
        return;
    }
    for(i = 0; i < n->nkids; i++)
    {
        PushPcn(n->kids[i], nextst);
    }
    if(n->typ != NULL && n->typ->pcn != NULL)
    {
        tfound = scroll_search(st, n->typ->name);
        if(tfound != NULL && tfound->pcn == NULL)
        {
            tfound->pcn = copy_name(n->typ->pcn);   
        }
    }
}
struct typeinfo *scroll_search(struct sym_table *st, char *s)
{
    struct sym_table *hereiam;
    struct typeinfo *tfound = 0;
    while(tfound == NULL && st != NULL)
    {
        hereiam = st;
        tfound = search_type(st, s);
        st = st->parent;
    }
    if(tfound != NULL)
    {
        tfound->i_am_defined_here = hereiam;   
    }
    return tfound;
}

void UndefineCheck(parsetree *n, struct sym_table *st, int dotfound)
{
    int i;
    struct sym_table *next_st;
    struct typeinfo *tfound = 0;
    if(n == NULL)
    {
        return;   
    }   
    switch(n->prodrule)
    {
        case CLASSES_SPEC: /* 6001 */
        case DECLARATORS_FUN: /* 5651 function */
        case DECLARATORS_FUN2: /* 5652 function */
            next_st = n->typ->st;
            break;
        case EXPRESSION_POSTFIX5: /* 4055 */
            dotfound++;
            n->kids[1]->possible_classname = copy_name(n->kids[0]->typ->name);
            next_st = st;
            break;
        case LEXICAL_IDENT: /* 1101 */
            if(n->typ->basetype == UNKNOWN_TYPE)
            { /* Undefined, need to see if in reachable sym tab */
                tfound = scroll_search(st, n->typ->name);
                if(tfound == NULL)
                {
                    if(dotfound && n->possible_classname != NULL)
                    { /* let's try to look for class.function */
                        tfound = scroll_search(st, n->possible_classname);
                        if(tfound != NULL)
                        { /* found class instance, now look for class definition */
                            /* all classes are global so search the global table */
                            if(n->typ->u.f.classname == NULL)
                            {
                                if(tfound->u.f.classname != NULL)
                                {
                                    n->typ->u.f.classname = copy_name(tfound->u.f.classname);   
                                }
                                else
                                {
                                    n->typ->u.f.classname = copy_name(tfound->pcn);   
                                }
                            }
                            tfound = scroll_search(symtab, n->typ->u.f.classname);
                            
                            if(tfound != NULL)
                            { /* we now have where the function should be defined */
                                tfound = scroll_search(tfound->st, n->typ->name);
                            }
                        }
                    }
                    if(tfound == NULL)
                    { /* if we still can't find it then error */                    
                        lineno = n->line_num;
                        curfilename = n->filename;
                        semantic_error(build_errstr(n->typ->name, " undefined!"));
                    }
                    else
                    { /* we found the type, so save */
                        n->typ = tfound;
                    }                   
                }
                else
                { /* we found the type, so save */
                    n->typ = tfound;
                }
            }
            break;
        default:
            next_st = st;
            break;
    }
    for(i = 0; i < n->nkids; i++)
    {
        UndefineCheck(n->kids[i], next_st, dotfound);   
    }
    if(n->prodrule == EXPRESSION_POSTFIX5)
    {
        dotfound--;   
    }
}

void TypeCheck(parsetree *n, struct sym_table *st)
{
    int i,j;
    struct sym_table *newst;
    struct typeinfo *tfound;
    if(n == NULL)
    {
        return;   
    }    
    /* post order bottom up type check of expressions */
    if(n->typ != NULL && n->typ->st != NULL)
    {
        newst = n->typ->st;
    }
    else
    {
        newst = st;
    }
    for(i = 0; i < n->nkids; i++)
    {
        TypeCheck(n->kids[i], newst);
    }
    if((n->prodrule >= 4000 && n->prodrule <= 4500) || n->prodrule == 5536 || n->prodrule == 5511)
    {
        switch(n->prodrule)
        {
            case DECLARATORS_DIRDEC6: /* 5536 */
                tfound = check_uni(n->kids[1], newst);
                if(tfound->basetype != INT_TYPE)
                {
                    lineno = n->line_num;
                    curfilename = n->filename;
                    semantic_error(build_errstr(" ", "Type mismatch!"));
                }
                n->typ = tfound;
                break;
            case STATEMENTS_SEL: /* 4551 */
            case STATEMENTS_SEL2: /*4552 */
                tfound = check_uni(n->kids[0], newst);
                if(tfound->basetype != INT_TYPE && tfound->basetype != BOOL_TYPE)
                {
                    lineno = n->line_num;
                    curfilename = n->filename;
                    semantic_error(build_errstr(" ", "Type mismatch!"));
                }
                n->typ = tfound;
                break;
            case STATEMENTS_SEL3: /* 4553, switch */
                tfound = check_uni(n->kids[0], newst);
                if(tfound->basetype != INT_TYPE)
                {
                    lineno = n->line_num;
                    curfilename = n->filename;
                    semantic_error(build_errstr(" ", "Type mismatch!"));
                }
                n->typ = tfound;
                break;
            case EXPRESSION_ASN2: /* 4312 */
                switch(n->kids[1]->prodrule)
                {
                    case EXPRESSION_OP: /* 4321 */
                        tfound = check_asn(n->kids[0], n->kids[2], newst);
                        break;
                    case EXPRESSION_OP2: /* 4322, *= */
                    case EXPRESSION_OP3: /* 4323, /= */
                    case EXPRESSION_OP4: /* 4324, %= */
                    case EXPRESSION_OP5: /* 4325, += */
                    case EXPRESSION_OP6: /* 4326, -= */
                        tfound = check_asn(n->kids[0], n->kids[2], newst);
                        if(tfound->basetype != INT_TYPE)
                        {
                            lineno = n->line_num;
                            curfilename = n->filename;
                            semantic_error(build_errstr(" ", "Type mismatch!"));
                        }
                        break;
                }
                n->typ = tfound;
                break;                
            case EXPRESSION_ADD2: /* 4212 */
            case EXPRESSION_ADD3: /* 4213 */
            case EXPRESSION_MULT2: /* 4202 */
            case EXPRESSION_MULT3: /* 4203 */
            case EXPRESSION_MULT4: /* 4204 */
                tfound = check_asn(n->kids[0], n->kids[1], newst);
                n->typ = tfound;
                break;
            case EXPRESSION_REL2: /* 4232 */
            case EXPRESSION_REL3: /* 4233 */
            case EXPRESSION_REL4: /* 4234 */
            case EXPRESSION_REL5: /* 4235 */
            case EXPRESSION_EQ2: /* 4242 */
            case EXPRESSION_EQ3: /* 4243 */
                tfound = check_asn(n->kids[0], n->kids[1], newst);
                n->typ = make_room(BOOL_TYPE);
                break;
            case EXPRESSION_SHIFT2: /* 4222, cout */
            case EXPRESSION_SHIFT3: /*4223, cin */
                tfound = check_io(n->kids[0], n->kids[1], newst); 
                if(tfound == NULL)
                {
                    lineno = n->line_num;
                    curfilename = n->filename;
                    semantic_error(build_errstr(" ", "type mismatch for cin/cout!"));
                }
                n->typ = tfound;
                break;
            case EXPRESSION_POSTFIX3: /* 4053 */
                if(n->nkids == 2)
                {
                    tfound = check_param(n->kids[0], n->kids[1], st);
                    if(tfound == NULL)
                    {
                        lineno = n->line_num;
                        curfilename = n->filename;
                        semantic_error(build_errstr(" ", "parameter mismatch!"));
                    }
                }
                else
                {
                    tfound = check_param(n->kids[0], NULL, st);
                    if(tfound != NULL)
                    {
                        lineno = n->line_num;
                        curfilename = n->filename;
                        semantic_error(build_errstr(" ", "parameter mismatch!"));
                    }
                }
                n->typ = tfound;
                break;
            case EXPRESSION_POSTFIX5: /* 4055 */
                tfound = check_dot(n->kids[1], newst);
                if(tfound == NULL)
                {
                    lineno = n->line_num;
                    curfilename = n->filename;
                    semantic_error(build_errstr(" ", "function/class mismatch!"));
                }
                n->typ = make_room(tfound->basetype);
                n->typ = tfound;
                break;
            case EXPRESSION_POSTFIX8: /* 4058 */
            case EXPRESSION_POSTFIX9: /* 4059 */
                tfound = check_uni(n->kids[0], newst);
                if(tfound->basetype != INT_TYPE)
                {
                    lineno = n->line_num;
                    curfilename = n->filename;
                    semantic_error(build_errstr(" ", "Type mismatch!"));
                }
                n->typ = tfound;
                break;
            case EXPRESSION_UNEX2: /* 4082 */
            case EXPRESSION_UNEX3: /* 4083 */
                //!!!?
                break;
            case EXPRESSION_LOGAND2: /* 4282 */
            case EXPRESSION_LOGOR2: /* 4292 */
                tfound = check_asn(n->kids[0], n->kids[1], newst);
                if(tfound->basetype != BOOL_TYPE && tfound->basetype != INT_TYPE)
                {
                    lineno = n->line_num;
                    curfilename = n->filename;
                    semantic_error(build_errstr(" ", "type mismatch for logical expression!"));
                }
                n->typ = tfound;
                break;
            //case EXPRESSION_NEWEX: /* 4111 */
                //!!!?
                //break;
            case EXPRESSION_DELEX: /* 4171 */
                tfound = check_uni(n->kids[0], newst);
                if(tfound->basetype != PTR_TYPE)
                {
                    lineno = n->line_num;
                    curfilename = n->filename;
                    semantic_error(build_errstr(" ", "Type mismatch!"));
                }
                n->typ = tfound;
                break;
            case DECLARATORS_INIT: /* 5511 */
                if(n->nkids == 2)
                {
                    /* assignment must be the same as declare */
                    tfound = check_asn(n->kids[0], n->kids[1], newst);
                }
                break;
            default:
                break;   
                
        }
    }
}

struct typeinfo *check_param(parsetree *n1, parsetree *n2, struct sym_table *st)
{ /* n1 has param list, n2 is a param, and st is sym tab */
    struct typeinfo *tparam;
    struct typeinfo *tfound;
    struct param *plst;
    parsetree *tnode;
    parsetree *tkid1;
    int i = 0;
    int j;
    int param_cnt = 0;
    int list_cnt = 0;
    
    plst = n1->typ->u.f.parameters;
    while(plst != NULL)
    {
        tparam = plst->type;
        plst = plst->next;
        if(tparam->basetype != NULL_TYPE)
        {
            i++;   
        }
    }
    list_cnt = i; /* count of function defined params */
    if(n2 == NULL && list_cnt > 0)
    { /* function has no params, but is given some */
        tfound = n1->typ; /* expects a null return - a non null return triggers error */ 
        return tfound;   
    }
    if(n2 == NULL && list_cnt == 0)
    { /* expected none, got none. all good */
        tfound = 0;
        return tfound;
    }
    plst = n1->typ->u.f.parameters;
    tnode = n2;
    tnode = FindNode(tnode, st, EXPRESSION_LIST2);
    if(tnode == NULL && list_cnt > 1)
    { /* only 1 param given, but more expected */
        tfound = 0;
        return tfound;
    }
    while(tnode != NULL)
    { /* let's count the parameters given */
        tnode = tnode->kids[0];
        tnode = FindNode(tnode, st, EXPRESSION_LIST2);
        param_cnt++;
    }
    param_cnt++;
    if(param_cnt != list_cnt)
    { /* error */
        tfound = 0;
        return tfound;   
    }
    /* now check param types */
    tnode = n2;
    j = list_cnt;
    while(tnode->nkids == 2 && tnode->kids[0]->prodrule == EXPRESSION_LIST2)
    { /* process backwards */
        tfound = FindLexElemNode(tnode->kids[1], st);
        if(tfound == NULL)
        { /* error */
            return tfound;   
        }
        for(i = 1; i < j; i++)
        { /* go to end */
            plst = plst->next;   
        }
        tparam = plst->type;
        if(tfound->basetype != tparam->basetype || tfound->basetype == UNKNOWN_TYPE)
        {
            tfound = 0; /* error */
            return tfound;
        }
        j--;
        //tparam->name = copy_name(tfound->name);
        tnode = tnode->kids[0];
        tnode = FindNode(tnode, st, EXPRESSION_LIST2);
        plst = n1->typ->u.f.parameters; 
    } 
    if(param_cnt > 1)
    {
        plst = n1->typ->u.f.parameters;
        tfound = FindLexElemNode(tnode->kids[1], st);
        if(tfound == NULL)
        { /* error */
            return tfound;   
        }
        for(i = 1; i < j; i++)
        { /* go to end */
            plst = plst->next;   
        }
        tparam = plst->type;
        if(tfound->basetype != tparam->basetype || tfound->basetype == UNKNOWN_TYPE)
        {
            tfound = 0; /* error */
            return tfound;
        }
        //tparam->name = copy_name(tfound->name);
        tnode = tnode->kids[0];
    }
    plst = n1->typ->u.f.parameters;
    tfound = FindLexElemNode(tnode, st);
    if(tfound == NULL)
    { /* error */
        return tfound;   
    }
    tparam = plst->type;
    if(tfound->basetype != tparam->basetype || tfound->basetype == UNKNOWN_TYPE)
    {
        tfound = 0; /* error */
        return tfound;
    }
    /* All's good. Return the function return type */
    //tparam->name = copy_name(tfound->name);
    tfound = make_room(n1->typ->u.f.returntype->basetype);
    return tfound;
}

struct typeinfo *check_io(parsetree *n1, parsetree *n2, struct sym_table *st)
{
    enum base_type n2type = UNKNOWN_TYPE;
    struct typeinfo *tfound1 = 0;
    struct typeinfo *tfound2;
    int isok = 0;
    
    if(trim_tree() == 2)
    { /* cin, cout shifts not supported */
        return tfound1;
    }
    tfound1 = FindLexElemNode(n1, st); 
    if(tfound1 != NULL && tfound1->name != NULL && tfound1->u.f.classname != NULL)
    { /* only shifts allowed are for cin & cout, must be these! */
        if(tfound1->basetype == FUNC_TYPE)
        { /* must be func type, must be cin or cout */
            if(strcmp("cout", tfound1->name) == 0 || strcmp("cin", tfound1->name) == 0)
            { /* must be of class ofstream or ifstream */
                if(strcmp("ofstream", tfound1->u.f.classname) == 0
                || strcmp("ifstream", tfound1->u.f.classname) == 0)
                { /* good to go! */
                    isok = 1; 
                }  
            }
        }   
    }
    if(isok)
    {
        tfound2 = FindLexElemNode(n2, st);
        if(tfound2 != NULL)
        {
            switch(tfound2->basetype)
            {
                case FUNC_TYPE:
                    n2type = tfound2->u.f.returntype->basetype;
                    break;
                case ARR_TYPE:
                    n2type = tfound2->u.a.elemtype->basetype;
                    break;
                default:
                    n2type = tfound2->basetype;
                    break;
            }
            if(n2type != INT_TYPE && n2type != BOOL_TYPE && n2type != CHAR_TYPE)
            {
                if(n2type == CLASS_TYPE && strcmp("string", tfound2->u.f.classname) != 0)
                { /* string class is the only one we accept */
                    isok = 0;
                }
            }
        }
    }
    if(!isok)
    { /* not good */
        tfound1 = 0;
    }
    return tfound1;
}

struct typeinfo *check_uni(parsetree *n, struct sym_table *st)
{
    struct typeinfo *tfound;
    enum base_type n1type = UNKNOWN_TYPE;
    tfound = FindLexElemNode(n, st); 
    if(tfound != NULL)
    {
        switch(tfound->basetype)
        {
            case FUNC_TYPE:
                n1type = tfound->u.f.returntype->basetype;
                break;
            case ARR_TYPE:
                n1type = tfound->u.a.elemtype->basetype;
                break;
            default:
                n1type = tfound->basetype;
                break;
        }
    }
    tfound = make_room(n1type);
    return tfound;
}

struct typeinfo *check_asn(parsetree *n1, parsetree *n2, struct sym_table *st)
{
    enum base_type n1type = UNKNOWN_TYPE;
    enum base_type n2type = UNKNOWN_TYPE;
    struct typeinfo *tfound;
    struct typeinfo *tfound2;
    parsetree *tnode;

    tfound = FindLexElemNode(n1, st); 
    if(tfound != NULL)
    {
        switch(tfound->basetype)
        {
            case FUNC_TYPE:
                n1type = tfound->u.f.returntype->basetype;
                break;
            case ARR_TYPE:
                n1type = tfound->u.a.elemtype->basetype;
                break;
            default:
                n1type = tfound->basetype;
                break;
        }
    }
    if(n2->prodrule == EXPRESSION_NEWEX) /* 4111 */
    {
        tnode = FindNode(n2, st, DECLARATORS_TYPESEQ);
        if(tnode == NULL)
        {
            n2type = UNKNOWN_TYPE; /* force type mismatch */
        }
        else
        {
            tfound2 = make_room(PTR_TYPE);
            tfound2->u.p.elemtype = tnode->kids[0]->typ;
            tnode->typ = tnode->kids[0]->typ;
            n2type = PTR_TYPE;
        }
    }
    else
    {
        tfound2 = FindLexElemNode(n2, st);
        if(tfound2 != NULL)
        {
            switch(tfound2->basetype)
            {
                case FUNC_TYPE:
                    n2type = tfound2->u.f.returntype->basetype;
                    break;
                case ARR_TYPE:
                    n2type = tfound2->u.a.elemtype->basetype;
                    break;
                default:
                    n2type = tfound2->basetype;
                    break;
            }
        }
    }
    if(n1type == UNKNOWN_TYPE || n1type != n2type)
    {
            lineno = n1->line_num;
            curfilename = n1->filename;
            semantic_error(build_errstr(" ", "Type mismatch!"));
    }
    if(n1type == PTR_TYPE)
    { /* check to see if class instance */
        if(tfound2->u.p.elemtype->basetype == UNKNOWN_TYPE)
        {
            lineno = n1->line_num;
            curfilename = n1->filename;
            semantic_error(build_errstr(" ", "Type mismatch!"));
        }
        if(tfound->u.p.elemtype->basetype == FUNC_TYPE && tfound2->u.p.elemtype->basetype == CLASS_TYPE)
        { /* Check further */
            if(tfound->u.p.elemtype->u.f.returntype->basetype != tfound2->u.p.elemtype->basetype)   
            {
                lineno = n1->line_num;
                curfilename = n1->filename;
                semantic_error(build_errstr(" ", "Type mismatch!"));
            }
        }
        if(tfound->u.p.elemtype->basetype != FUNC_TYPE)
        {
            if(tfound->u.p.elemtype->basetype != tfound2->u.p.elemtype->basetype)   
            {
                lineno = n1->line_num;
                curfilename = n1->filename;
                semantic_error(build_errstr(" ", "Type mismatch!"));
            }
        }
    }
    tfound = make_room(n1type);
    return tfound;
}

parsetree *FindNode(parsetree *n, struct sym_table *st, int rule)
{
    int i;
    parsetree *tnode = 0;
    if(n == NULL || n->prodrule == rule)
    {
        return n;   
    }
    for(i = 0; i < n->nkids; i++)
    {
        tnode = FindNode(n->kids[i], st, rule);
        if(tnode != NULL)
        {
            break;
        }   
    }
    return tnode;   
}

struct typeinfo *FindLexElemNode(parsetree *n, struct sym_table *st)
{
    struct typeinfo *tfound = 0;
    struct sym_table *newst;
    int i;
    
    if(n == NULL)
    {
        return tfound;   
    }   
    if(n->prodrule < 1200) /* Lexical Element */
    {
        tfound = get_my_type(n, st);
        return tfound;
    }
    if(n->prodrule > 4000 && n->prodrule < 5000) /* expressions & statements */
    {
        if(n->typ != NULL && n->typ->basetype != UNKNOWN_TYPE)
        {
            return n->typ;   
        }   
    }
    if(n->typ != NULL && n->typ->st !=NULL)
    {
        newst = n->typ->st;
    }
    else
    {
        newst = st;
    }
    i = 0;
    if(n->prodrule == EXPRESSION_POSTFIX5) /* 4055 */
    {
        i++; /* look for function part */
    }
    while(i < n->nkids)
    {
        tfound = FindLexElemNode(n->kids[i], newst);   
        i++;
        if(tfound != NULL)
        {
            break;   
        }
    }
    return tfound;
}

struct typeinfo *check_dot(parsetree *n, struct sym_table *st)
{
    struct typeinfo *tfound;
    tfound = FindLexElemNode(n, st);
    return tfound;
}

struct typeinfo *get_my_type(parsetree *n, struct sym_table *st)
{
    struct typeinfo * tfound = 0;
    if(n->typ->basetype != UNKNOWN_TYPE)
    {
        return n->typ;
    }
    if(n->typ->name != NULL)
    {
        tfound = scroll_search(st, n->typ->name); 
        if(tfound == NULL && n->typ->i_am_defined_here != NULL)
        {
            tfound = scroll_search(n->typ->i_am_defined_here, n->typ->name);   
        }        
        if(tfound == NULL && n->possible_classname != NULL)
        {
            tfound = scroll_search(st, n->possible_classname);
            if(tfound != NULL)
            { /* found class instance, now look for class definition */
                /* all classes are global so search the global table */
                tfound = scroll_search(symtab, tfound->u.f.classname);
                if(tfound != NULL)
                { /* we now have where the function should be defined */
                    tfound = scroll_search(tfound->st, n->typ->name);
                }
            }        
        }
    }
    return tfound;
}

int trim_tree()
{
    int idx = 2;  /* we keep nothing */
    if((included_fstream || included_iostream) && namespace_found)
    {
       idx = 0;  /* we keep all  */
    }
    else
    {
        if(included_string && namespace_found)
        {
            idx = 1;  /* we keep strings only */
        }   
    }
    return idx;
}

void prefill_libs(int libs)
{    
    /* this is hard code to prefill global symbol 
     * table with endl, cin, cout, and string if
     * namespace and libs are included */
    
     /* Sample Classes */
     /*
        class ifstream{
            public:
                void close();
                bool eof();
                void ignore();
                void open(char *);
            };
         class ofstream{
             public:
                void close();
                void open(char *);
            };
         ifstream cin;
         ofstream cout;   
            
         class string{
             public:
                char *c_str();
            };
     */
     //!!! what about getline(ifstream, string&)?
    FILE *f;
    f = fopen("~120lib.cpp", "w");
    if(!f)
    {  
        fprintf(stderr, "ERROR: unable to open file %s.\n", "~120lib.cpp");
        exit(11);
    }
    switch(libs)
    {
        case ADDALLLIBS:
            fprintf(f, "class ifstream{\n\tpublic:\n\t\tvoid close();\n");
            fprintf(f, "\t\tbool eof();\n\t\tvoid ignore();\n");
            fprintf(f, "\t\tvoid open(char *);\n\t};\n");
            
            fprintf(f, "class ofstream{\n\tpublic:\n\t\tvoid close();\n");
            fprintf(f, "\t\tvoid open(char *);\n\t};\n");
            fprintf(f, "ofstream cout;\nifstream cin;\n");
            fprintf(f, "const char endl = '\\n';\n");
            break;
        case ADDSTRLIBS:
            fprintf(f, "class string{\n\tpublic:\n\t\tchar *c_str();\n\t};\n\n");
            break;
    }
    fflush(f);
    fclose(f);
}

char *build_errstr(char *ename, char *e2)
{
    int i;
    char *e1;
    char *e3;
    e1 = copy_name("ERROR: ");
    i = strlen(e1) + strlen(e2) + strlen(ename);
    e3 = (char *)calloc(i, sizeof(char)+1);
    if(e3 == NULL)
    {
        fprintf(stderr, "ERROR: out of memory\n"); 
        exit(10);
    }
    strcpy(e3, e1);
    strcat(e3, ename);
    strcat(e3, e2);
    return e3;
}

void FillSymTabs(parsetree *n, struct sym_table *st, int inclass)
{
    int i;
    struct sym_table *next_st;
    enum base_type symtype;
    struct typeinfo *tmp = 0;
    struct typeinfo *tmp2 = 0;
    struct param *tmpnode;
    if(n == NULL)
    {
        return;
    }
    next_st = st; /* next scope is same as current scope */
    switch(n->prodrule)
    {
        case DECLARATIONS_SEQ: /* 5001 */
            if(n->kids[0]->prodrule == DECLARATIONS_SIMPLE && n->kids[0]->typ->basetype == FUNC_TYPE)
            {
               n->kids[0]->prototype++; 
               n->kids[0]->typ->isProto++;
               tmp = n->kids[0]->typ;
               i = 0;
                if(tmp != NULL)
                {   /* insert globally defined function */
                    if(search_type(symtab, tmp->name) == NULL)
                    {
                        sym_insert(symtab, tmp);
                        st->count++;
                        next_st = symtable(SYMHASHSIZE);
                        next_st->parent = symtab; /* new scope */
                        tmp->st = next_st; /* i own this, the same if not a class or function */
                        tmp->i_am_defined_here = symtab;
                        n->kids[i]->st = symtab; /* i'm defined here */
                        if(semantic_debug == 1)
                        {
                            printf("%d\t%s\t\t%d\t%p\t%p\n",n->kids[0]->prodrule,tmp->name,tmp->basetype,st,next_st);
                        }
                        PushPrototypes(n, NULL);
                    }
                    else
                    {
                        lineno = n->line_num;
                        curfilename = n->filename;
                        semantic_error(build_errstr(tmp->name, " redefined!"));
                    }
                }
            }
            if(n->kids[1] != NULL)
            {
                if(n->kids[1]->prodrule == DECLARATIONS_SIMPLE && n->kids[1]->typ->basetype == FUNC_TYPE)
                {
                    n->kids[1]->prototype++; 
                    n->kids[1]->typ->isProto++;
                    tmp = n->kids[1]->typ;
                    i = 1;
                    if(tmp != NULL)
                    {   /* insert globally defined function */
                        if(search_type(symtab, tmp->name) == NULL)
                        {
                            sym_insert(symtab, tmp);
                            st->count++;
                            next_st = symtable(SYMHASHSIZE);
                            next_st->parent = symtab; /* new scope */
                            tmp->st = next_st; /* i own this, the same if not a class or function */
                            tmp->i_am_defined_here = symtab;
                            n->kids[i]->st = symtab; /* i'm defined here */
                            if(semantic_debug == 1)
                            {
                                printf("%d\t%s\t\t%d\t%p\t%p\n",n->kids[1]->prodrule,tmp->name,tmp->basetype,st,next_st);
                            }
                            PushPrototypes(n, NULL);
                        }
                        else
                        {
                            lineno = n->line_num;
                            curfilename = n->filename;
                            semantic_error(build_errstr(tmp->name, " redefined!"));
                        }
                    }
                }
            }
            next_st = symtab; /* this is a prototype so don't pass down sym tab */
            break;
        case CLASSES_SPEC: /* 6001 */
            symtype = search_sym(st, n->typ->name, UNKNOWN_TYPE);
            if(symtype == UNKNOWN_TYPE)
            { /* put in curr scope & then create new scope sym tab */  
                sym_insert(st, n->typ);
                st->count++;
                next_st = symtable(SYMHASHSIZE);
                next_st->parent = st; /* new scope */
                n->typ->st = next_st; /* i own this, the same if not a class or function */
                n->typ->i_am_defined_here = st;
                n->st = st; /* i'm defined here */
                if(semantic_debug == 1)
                {
                    printf("%d\t%s\t\t%d\t%p\t%p\n",n->prodrule,n->typ->name,n->typ->basetype,st,next_st);
                    //printf("\t\tn->typ = %p\n", n->typ);
                }
                inclass++;
            }
            else
            {
                lineno = n->line_num;
                curfilename = n->filename;
                semantic_error(build_errstr(n->typ->name, " class already defined!"));                    
            }
            if(semantic_debug == 1)
            {
                printf("CLASS %s in scope %p owns scope %p\n", n->typ->name,st,next_st);            
            }
            break;
            
        case DECLARATORS_PARAMLS2: /* 5632 */
        case DECLARATORS_PARAMDC: /* 5641 */
        case DECLARATORS_PARAMDC3: /* 5643 */
            /* we have a list of params, they all should be numbered */
            inparamlist++;
            if(inparamlist == 1)
            { /* reset param count */
                paramlist = 1;
            }
            break;
        case DECLARATORS_FUN: /* 5651 function */
        case DECLARATORS_FUN2: /* 5652 function */
            if(n->prodrule == 5652)
            {
                n->typ->is5652++;   
            }
            if(n->prodrule == 5651)
            {
                n->typ->is5651++;
            }
            tmp = search_type(symtab, n->typ->name); /* search global first */
            if(tmp == NULL)
            { /* now see if it's in a class */
                if(n->typ->u.f.classname != NULL)
                {
                    tmp = search_type(symtab, n->typ->u.f.classname);
                    if(tmp == NULL)
                    { /* class undefined */         
                        lineno = n->line_num;
                        curfilename = n->filename;
                        semantic_error(build_errstr(n->typ->u.f.classname, " enclosing class undefined!"));                    
                    }
                    next_st = tmp->st;
                    tmp = search_type(next_st, n->typ->name); /* find the function's scope */
                    if(tmp == NULL)
                    { /* put into class scope */
                        sym_insert(next_st, n->typ);
                        st->count++;
                        st = next_st;
                        next_st = symtable(SYMHASHSIZE);
                        next_st->parent = st; /* new scope */
                        n->typ->st = next_st; /* i own this, the same if not a class or function */
                        n->typ->i_am_defined_here = st;
                        n->st = st; /* i'm defined here */
                        if(semantic_debug == 1)
                        {
                            printf("%d\t%s\t\t%d\t%p\t%p\n",n->prodrule,n->typ->name,n->typ->basetype,st,next_st);
                            //printf("\t\tn->typ = %p\n", n->typ);
                        }
                    }
                    else
                    {
                        n->typ->st = tmp->st;
                        n->typ->i_am_defined_here = tmp->st->parent;
                        n->st = tmp->st->parent;
                        next_st = tmp->st;   
                    }
                }
                else
                { /* put in global symtab because functions are global unless in a class */
                    sym_insert(symtab, n->typ);
                    st->count++;
                    next_st = symtable(SYMHASHSIZE);
                    next_st->parent = st; /* new scope */
                    n->typ->st = next_st; /* i own this, the same if not a class or function */
                    n->typ->i_am_defined_here = st;
                    n->st = st; /* i'm defined here */
                    if(semantic_debug == 1)
                    {
                        printf("%d\t%s\t\t%d\t%p\t%p\n",n->prodrule,n->typ->name,n->typ->basetype,st,next_st);
                        //printf("\t\tn->typ = %p\n", n->typ);
                    }
                }
            }
            else
            {
                if(n->prodrule == 5652)
                {
                   tmp->is5652++;   
                }
                if(n->prodrule == 5651)
                {
                    tmp->is5651++;
                }
                if(tmp->is5652 > 1 || tmp->is5651 > 1)
                {
                    lineno = n->line_num;
                    curfilename = n->filename;
                    semantic_error(build_errstr(n->typ->name, " redefined!"));
                }
                n->typ->st = tmp->st;
                n->st = tmp->st->parent;
                n->typ->i_am_defined_here = tmp->st->parent;
                next_st = tmp->st;
            }
            break;
        case DECLARATORS_DIRDEC2: /* 5532 */
            /* constructor, need to add classname to constructor */
            tmp = search_type(symtab, n->typ->u.f.classname);
            /* ok now I have correct sym table to find construct */
            tmp2 = search_type(tmp->st, n->typ->name);
            tmp2->u.f.classname = n->typ->u.f.classname;
            tmp2->u.f.returntype = n->typ->u.f.returntype;
            tmp2->u.f.returntype->name = copy_name(n->typ->u.f.returntype->name);
            break;
        case LEXICAL_IDENT: /* 1101 */
            /* only do this if type != unknown */
            if(n->typ->basetype != UNKNOWN_TYPE && n->typ->basetype != CLASS_TYPE)
            { /* class has been inserted on node 6001 */
                if(n->typ->basetype == FUNC_TYPE)
                {
                    if(inclass > 0)
                    { /* we're setting up functions inside class */
                        symtype = search_sym(st, n->typ->name, UNKNOWN_TYPE);
                        if(symtype == UNKNOWN_TYPE)
                        { /* add to class sym tab, make func sym tab, but don't pass down */  
                            sym_insert(st, n->typ);
                            st->count++;
                            next_st = symtable(SYMHASHSIZE); /* new scope */
                            next_st->parent = st; /* enclosing scope */
                            n->typ->st = next_st; /* i own this, the same if not a class or function */
                            n->st = st; /* i'm defined here */
                            n->typ->i_am_defined_here = st;
                            if(semantic_debug == 1)
                            {
                                printf("%d\t%s\t\t%d\t%p\t%p\n",n->prodrule,n->typ->name,n->typ->basetype,st,next_st);
                                //printf("\t\tn->typ = %p\n", n->typ);
                            }
                            next_st = st; /* we don't want to pass the new sym tab down */
                        }
                        else
                        { 
                            lineno = n->line_num;
                            curfilename = n->filename;
                            semantic_error(build_errstr(n->typ->name, " redefined!"));
                        }
                    }
                    else
                    { /* are we defined globally? yes = func, no = class func */
                        symtype = search_sym(symtab, n->typ->name, UNKNOWN_TYPE);
                        if(symtype == UNKNOWN_TYPE)
                        {
                            if(n->typ->u.f.classname != NULL)
                            { /* the classname should be global */
                                tmp = search_type(symtab, n->typ->u.f.classname);
                                if(tmp == NULL)
                                {   /* error, enclosing class undefined */
                                    lineno = n->line_num;
                                    curfilename = n->filename;
                                    semantic_error(build_errstr(n->typ->u.f.classname, " enclosing class undefined!"));
                                }
                                else
                                { /* if the returntype = class, then we're just defining an instance */
                                    i = 0;
                                    if(n->typ->u.f.returntype->basetype == CLASS_TYPE)
                                    { /* insert into current sym tab */
                                        i++;
                                        sym_insert(st, n->typ);
                                        if(semantic_debug == 1)
                                        {
                                            printf("%d\t%s\t\t%d\t%p\t%p\n",n->prodrule,n->typ->name,n->typ->basetype,st,next_st);
                                            //printf("\t\tn->typ = %p\n", n->typ);
                                        }
                                        st->count++;
                                        n->st = st;
                                    } 
                                    if(i == 0) /* return type != class */
                                    { /* is the function defined in the class */
                                        //symtype = search_sym(tmp->st, n->typ->name, UNKNOWN_TYPE);
                                        tmp2 = search_type(tmp->st, n->typ->name);
                                        //if(symtype == UNKNOWN_TYPE)
                                        if(tmp2 == NULL)
                                        { /* error, using a class function that is undefined */   
                                            lineno = n->line_num;
                                            curfilename = n->filename;
                                            semantic_error(build_errstr(n->typ->name, " undefined class function!"));
                                        } // error, function not defined in class
                                        else
                                        { /* we found defined function inside class, mark save type with classname */
                                            if(tmp2->u.f.classname == NULL)
                                            {
                                                tmp2->u.f.classname = copy_name(tmp->name);  
                                            }
                                        }
                                    } // fucntion not an instance of class
                                } // function associated with class
                            }
                            else
                            {
                                if(st != symtab)
                                {
                                    lineno = n->line_num;
                                    curfilename = n->filename;
                                    semantic_error(build_errstr(n->typ->name, " undefined global function!"));
                                }
                            } // function not associated with class & not global so error
                        }
                        /* else we're defined globally, e.g. like main */
                    } // are we in a class definition?
                }
                else
                {
                    if(n->typ->isProto == 0) /* don't insert if prototype */
                    {
                        symtype = search_sym(st, n->typ->name, n->typ->basetype);
                        if(symtype == UNKNOWN_TYPE)
                        { /* add to sym tab */  
                            if(n->typ->basetype == PARAM_TYPE && inclass == 0)
                            { /* we need to get the param numbers into the symtab */
                                tmp = make_room(n->typ->u.plist.type->basetype);
                                tmp->isParam = paramlist;
                                n->typ->u.plist.type->isParam = paramlist;
                                paramlist++;
                                tmp->name = copy_name(n->typ->name);
                                sym_insert(st, tmp);
                                if(semantic_debug == 1)
                                {
                                    printf("%d\t%s\t\t%d\t%p\t%p\n",n->prodrule,tmp->name,tmp->basetype,st,next_st);
                                    //printf("\t\tn->typ = %p\n", tmp);
                                }
                                st->count++;
                                tmpnode = n->typ->u.plist.next;
                                while(tmpnode != NULL)   
                                {
                                    tmp = make_room(tmpnode->type->basetype);
                                    tmp->name = copy_name(n->typ->name);
                                    tmp->isParam = paramlist;
                                    tmpnode->type->isParam = paramlist;
                                    paramlist++;
                                    sym_insert(st, tmp);
                                    if(semantic_debug == 1)
                                    {
                                        printf("%d\t%s\t\t%d\t%p\t%p\n",n->prodrule,tmp->name,tmp->basetype,st,next_st);
                                        //printf("\t\tn->typ = %p\n", tmp);
                                    }
                                    st->count++;
                                    tmpnode = tmpnode->next;
                                }
                                n->st = st;
                                n->typ->i_am_defined_here = st;
                            }
                            else
                            {
                                sym_insert(st, n->typ);
                                if(semantic_debug == 1)
                                {
                                    printf("%d\t%s\t\t%d\t%p\t%p\n",n->prodrule,n->typ->name,n->typ->basetype,st,next_st);
                                    //printf("\t\tn->typ = %p\n", n->typ);
                                }
                                st->count++;
                                n->st = st;
                            }
                        }
                    }
                    else
                    { 
                        if(n->typ->isProto == 0)
                        { /* if not prototype then it's redefined! */
                            lineno = n->line_num;
                            curfilename = n->filename;
                            semantic_error(build_errstr(n->typ->name, " already defined!"));
                        }
                    }
                }
            }
            break; 
    }
    for(i = 0; i < n->nkids; i++) /* mother done, so now do her kids */
    {
        FillSymTabs(n->kids[i], next_st, inclass);
    } 
    switch(n->prodrule)
    {
        case DECLARATORS_PARAMLS2: /* 5632 */
        case DECLARATORS_PARAMDC: /* 5641 */
        case DECLARATORS_PARAMDC3: /* 5643 */
            inparamlist--;
            if(inparamlist == 0)
            { /* reset paramlist */
                paramlist = 0;
            }
            break;
        case CLASSES_SPEC: /* 6001 */
            inclass--; /* we're out of class now */
            break;
    }
}

void PushPrototypes(parsetree *n, struct typeinfo *t)
{ /* pre order traverse pushes prototype flag to unknown kids */
    int i, j;
    if(n == NULL)
    {
        return;   
    }
    if(t != NULL)
    {
        switch(n->prodrule)
        {
            case LEXICAL_IDENT: /* 1101 */
                if(t != NULL)
                { /* if t is null then this isn't a decl */
                    n->typ->isProto = t->isProto;
                }
                break;
            case DECLARATORS_INITLIST2: /* 5502, decl list */
                n->typ->isProto = t->isProto;
                break;
            case DECLARATORS_DECL2: /* 5522 */
            case DECLARATIONS_SIMPLE: /* 5031 */
            case DECLARATORS_INIT: /* 5511 */
                n->typ->isProto = t->isProto;
                break;
            case DECLARATORS_DIRDEC2: /* 5532 */
            case DECLARATORS_DIRDEC5: /* 5535 */
            case DECLARATORS_DIRDEC9: /* 5539 */
                n->typ->isProto = t->isProto;
                break;
            case DECLARATORS_PARAMLS2: /* 5632 */
            case DECLARATORS_PARAMDC: /* 5641 */
            case DECLARATORS_PARAMDC3: /* 5643 */
                n->typ->isProto = t->isProto;
                break;
            case DECLARATORS_DIRDEC6: /* 5536 */
                n->typ->isProto = t->isProto;
                break;
        }
    }
    for(i = 0; i < n->nkids; i++)
    {
        PushPrototypes(n->kids[i], n->typ);
    }
}

void TypesToSibs(parsetree *n, struct typeinfo *t)
{ /* pre order traverse pushes types to unknown kids */
    int i, j;
    struct typeinfo *save_type;
    if(n == NULL)
    {
        return;   
    }
    switch(n->prodrule)
    {
        case LEXICAL_IDENT: /* 1101 */
            if(t != NULL)
            { /* if t is null then this isn't a decl */
                save_type = n->typ; /* save off type info */
                n->typ = make_room(t->basetype);
                n->typ->u = t->u; /* copy union info */
                n->typ->name = save_type->name;
                n->typ->isConst = save_type->isConst;
                n->typ->const_val = save_type->const_val;
                n->typ->isProto = t->isProto;
            }
            break;
        case DECLARATORS_INITLIST2: /* 5502, decl list */
        case DECLARATORS_INIT: /* 5511 */
            if(n->typ->basetype == FUNC_TYPE)
            {
                n->typ->u.f.returntype == t;
            }
            else
            {
                n->typ = t;
                if(n->possible_classname != NULL)
                {
                    n->typ->pcn = copy_name(n->possible_classname);
                }
            }
            break;
        case DECLARATORS_DECL2: /* 5522 */
            /* what type of pointer? */
            if(t->basetype == PTR_TYPE)
            { /* we pulled pointer type up on rule 5641 */
                n->typ->u.p.elemtype = t->u.p.elemtype;
            }
            else
            { /* we need to pull type down from rule 6041 */
                n->typ->u.p.elemtype = t; 
            }
            n->typ->pcn = copy_name(t->pcn);
            break;
        case DECLARATORS_DIRDEC2: /* 5532 */
        case DECLARATORS_DIRDEC5: /* 5535 */
        case DECLARATORS_DIRDEC9: /* 5539 */
            /* function needs return type */
            if(t != NULL && t->u.f.returntype != NULL 
            && (n->typ->u.f.returntype == NULL || n->typ->u.f.returntype->basetype == UNKNOWN_TYPE))
            {
                n->typ->u.f.returntype = t->u.f.returntype;   
            }
            else
            {
                if(n->typ->u.f.returntype == NULL)
                {
                    n->typ->u.f.returntype = make_room(NULL_TYPE);  
                } 
            }
            if(n->possible_classname != NULL && n->typ->u.f.classname == NULL)
            {
                n->typ->u.f.classname = copy_name(n->possible_classname);   
            }
            else
            {
                if(t != NULL && n->typ->u.f.classname == NULL && t->u.f.classname != NULL)
                {
                    n->typ->u.f.classname = t->u.f.classname;   
                }
            }
            if(t != NULL)
            {
                n->typ->isProto = t->isProto;
            }
            break;
        case DECLARATORS_PARAMLS2: /* 5632 */
        case DECLARATORS_PARAMDC: /* 5641 */
        case DECLARATORS_PARAMDC3: /* 5643 */
            n->typ->isProto = t->isProto;
            break;
        case DECLARATORS_DIRDEC6: /* 5536 */
            n->typ->u.a.elemtype = t; /* array */
            n->typ->isProto = t->isProto;
            break;
        case DECLARATORS_FUN: /* 5651 function */
        case DECLARATORS_FUN2: /* 5652 function */
            for(j = 0; j < n->nkids; j++)
            { /* push down class info */   
                if(n->kids[j]->prodrule < 5000)
                {
                    n->kids[j]->possible_classname = copy_name(n->typ->u.f.classname);
                }
            }
            break;
        default:
            if(n->possible_classname != NULL)
            {
                for(j = 0; j < n->nkids; j++)
                { /* push down class info */   
                    if(n->kids[j]->prodrule < 5000)
                    {
                        n->kids[j]->possible_classname = copy_name(n->possible_classname);
                    }
                }
            }
            break;
    }
    for(i = 0; i < n->nkids; i++)
    {
        TypesToSibs(n->kids[i], n->typ);
    }
}

void FindTypes(parsetree *n)
{
    int i;
    struct param *param_list = 0;
    struct param *param_next = 0;
    if(n == NULL)
    {
        return;   
    }
    for(i = 0; i < n->nkids; i++)
    {
        FindTypes(n->kids[i]);   
    }

    switch(n->prodrule)
    {
        case IDENTIFIERS_ORIG_NS: /* 1021 */
            n->typ = make_room(NAMESPACE_TYPE);
            break;
        case IDENTIFIERS_CLASS: /* 1031 */
            n->typ = make_room(CLASS_TYPE);
            n->typ->name = copy_name(n->leaf->text);
            break;
        case LEXICAL_IDENT: /* 1101 */
            n->typ = make_room(UNKNOWN_TYPE);
            n->typ->name = copy_name(n->leaf->text);
            break;
        case LEXICAL_INTLIT: /*1121 */
            n->typ = make_room(INT_TYPE);
            break;
        case LEXICAL_CHARLIT: /* 1131 */
            n->typ = make_room(CHAR_TYPE);
            break;
        case LEXICAL_FLOATLIT: /* 1141 */
            n->typ = make_room(FLOAT_TYPE);
            break;
        case LEXICAL_STRINGLIT: /* 1151 */
            //!!!! wrong!
            n->typ = make_room(PTR_TYPE);
            n->typ->u.p.elemtype->basetype = CHAR_TYPE;
            break;
        case LEXICAL_BOOLLIT: /* 1161 */
        case LEXICAL_BOOLLIT2: /* 1162 */
            n->typ = make_room(BOOL_TYPE);
            break;
        case EXPRESSION_NEWEX: /* 4111 */
        case EXPRESSION_NEWTYPE: /* 4131 */
            n->typ = make_room(UNKNOWN_TYPE);
            n->typ = n->kids[0]->typ;
            break;
        case DECLARATIONS_SIMPLE: /* 5031 */
            if(n->kids[1]->typ->basetype == FUNC_TYPE)
            {
                n->typ = n->kids[1]->typ;
                n->typ->u.f.returntype = n->kids[0]->typ;
                if(searchInHash(n->kids[0]->leaf->text) == CLASS_NAME)
                { /* we have a class type */
                    n->possible_classname = copy_name(n->kids[0]->leaf->text);
                    n->typ->u.f.classname = copy_name(n->kids[0]->leaf->text);
                }
                n->typ->name = copy_name(n->kids[1]->typ->name);
            }
            else
            {
                if(n->kids[0]->typ->basetype == CLASS_TYPE)
                { /* could be globally defined class, make a function */
                    n->typ = make_room(FUNC_TYPE);
                    n->typ->u.f.returntype = n->kids[0]->typ;
                    n->typ->name = copy_name(n->kids[1]->typ->name);
                    n->typ->u.f.classname = copy_name(n->kids[0]->leaf->text);
                }
                else
                {
                    /* let's just use this to push return type down */
                    n->typ = n->kids[0]->typ;
                    n->typ->name = copy_name(n->kids[1]->typ->name);
                }
            }
            break;
        case DECLARATIONS_DECLSEQ: /* 5051 */
            /* declare with static|extern, ignore static|extern */
            n->typ = n->kids[1]->typ;
            if(n->kids[0]->prodrule = DECLARATORS_CV)
            {
                n->typ->isConst = 1;   
            }
            break;
        //case DECLARATIONS_SIMPLETP2: /* 5092 */
        case DECLARATIONS_SIMPLETP4: /* 5094 */
            n->typ = make_room(CHAR_TYPE);
            break;
        case DECLARATIONS_SIMPLETP6: /* 5096 */
            n->typ = make_room(BOOL_TYPE);
            break;
        case DECLARATIONS_SIMPLETP7: /* 5097 */
        case DECLARATIONS_SIMPLETP8: /* 5098 */
        case DECLARATIONS_SIMPLETP9: /* 5099 */
            n->typ = make_room(INT_TYPE);
            break;
        case DECLARATIONS_SIMPLETP12: /* 5102 */
        case DECLARATIONS_SIMPLETP13: /* 5103 */
            n->typ = make_room(FLOAT_TYPE);
            break;
        case DECLARATIONS_SIMPLETP14: /* 5104 */
            n->typ = make_room(NULL_TYPE);
            break;
        //case DECLARATIONS_ETYPE2: /* 5122 */
        case DECLARATORS_INITLIST2: /* 5502 */
            n->typ = make_room(UNKNOWN_TYPE);
            n->typ->name = copy_name(n->kids[1]->typ->name);
            break;
        case DECLARATORS_INIT: /* 5511 */
            if(n->kids[0]->typ->basetype == FUNC_TYPE)
            {
                n->typ = n->kids[0]->typ;
            }
            else
            {
                n->typ = make_room(UNKNOWN_TYPE);
            }
            n->typ->name = copy_name(n->kids[0]->typ->name);
            if(n->nkids > 1 && n->kids[1]->prodrule == 4111)
            {
                n->possible_classname = copy_name(n->kids[1]->typ->name);   
            }
            break;
        case DECLARATORS_DECL2: /* 5522 */
            n->typ = n->kids[0]->typ; /* ptr, we'll get elemtype in top-down push */
            n->typ->name = copy_name(n->kids[1]->typ->name);
            break;
        case DECLARATORS_DIRDEC2: /* 5532 */
            n->typ = make_room(FUNC_TYPE);
            n->typ->u.f.returntype->basetype = CLASS_TYPE;  
            /* this is the name of the class to which this constructor function belongs */
            /* all classes are global */
            n->typ->u.f.returntype->name = copy_name(n->leaf->text); 
            n->typ->u.f.parameters = next_param(); 
            n->typ->u.f.parameters->type = make_room(NULL_TYPE);
            n->typ->name = copy_name(n->leaf->text);
            n->typ->u.f.classname = copy_name(n->leaf->classname);
            break;
        case DECLARATORS_DIRDEC5: /* 5535 */
            n->typ = make_room(FUNC_TYPE);
            n->typ->u.f.parameters = next_param();
            if(n->nkids == 2)
            {
                n->typ->u.f.parameters = &n->kids[1]->typ->u.plist;
            }
            else
            {
                /* empty parans = void parameter */              
                n->typ->u.f.parameters->type = make_room(NULL_TYPE);
            }
            n->typ->name = copy_name(n->kids[0]->leaf->text);
            break;
        case DECLARATORS_DIRDEC6: /* 5536 */
            n->typ = make_room(ARR_TYPE);
            n->typ->name = copy_name(n->kids[0]->typ->name);
            if(n->nkids == 2)
            {
                n->typ->u.a.size = n->kids[1]->leaf->ival;
            }
            break;
        case DECLARATORS_DIRDEC9: /* 5539 */
            n->typ = make_room(FUNC_TYPE);
            n->typ->name = copy_name(n->kids[0]->typ->name);
            if(n->nkids == 2)
            {
                /* there are params! */
                n->typ->u.f.parameters = &n->kids[1]->typ->u.plist;    
            }
            n->typ->u.f.classname = copy_name(n->leaf->text);
            break;
        //case DECLARATORS_DIRDEC10: /* 5540 */
        case DECLARATORS_PTR: /* 5541, contents */
        case DECLARATORS_PTR3: /* 5543, address */
            n->typ = make_room(PTR_TYPE);
            break;
        case DECLARATORS_CV: /* 5561 */
            /* const, no-op */
            n->typ = make_room(UNKNOWN_TYPE);
            break;
        case DECLARATORS_TYPESEQ: /* 5591 */
            n->typ = make_room(n->kids[0]->typ->basetype);
            n->typ->name = copy_name(n->kids[0]->typ->name);
            break;
        case DECLARATORS_ABST: /* 5601 */
            n->typ = make_room(PTR_TYPE);
            break;
        case DECLARATORS_PARAMLS2: /* 5632 */
            n->typ = make_room(PARAM_TYPE);
            param_list = next_param();
            /* only param lists allowed */
            n->typ->u.plist.type = n->kids[0]->typ->u.plist.type;
            n->typ->u.plist.next = n->kids[0]->typ->u.plist.next;
            /* now find end of param linked list */
            param_next = &n->typ->u.plist;
            while(param_next != NULL)
            {
                /* find the end of the list and make room for kids[1] */
                if(param_next->next == NULL)
                {
                    break;   
                }
                param_next = param_next->next;
            }
            param_list->type = n->kids[1]->typ->u.plist.type;
            param_next->next = param_list;
            param_list = 0; /* reset temp pointers to null */
            param_next = 0;
            break;
        case DECLARATORS_PARAMDC:  /* 5641 */
            n->typ = make_room(PARAM_TYPE);
            if(n->kids[1]->typ->basetype == ARR_TYPE)
            {
                n->typ->u.plist.type = n->kids[1]->typ;
            }
            else
            {
                if(n->kids[1]->typ->basetype == PTR_TYPE)
                {
                    /* pointer has been specified in nodes below */
                    n->typ->u.plist.type = n->kids[1]->typ;
                    n->typ->u.plist.type->u.p.elemtype = n->kids[0]->typ;
                }
                else
                {
                    if(n->kids[1]->leaf->category == IDENTIFIER)
                    {
                        /* Simple declaration */
                        n->typ->u.plist.type = n->kids[0]->typ;
                    }   
                }
            }
            break;
        case DECLARATORS_PARAMDC3: /* 5643 */
            n->typ = make_room(PARAM_TYPE);
            if(n->nkids == 1)
            {
                n->typ->u.plist.type = n->kids[0]->typ;   
            }
            else
            {
                if(n->kids[1]->typ->basetype == PTR_TYPE)
                {
                    n->typ->u.plist.type = n->kids[1]->typ;
                    n->typ->u.plist.type->u.p.elemtype = n->kids[0]->typ;
                }
                else
                {
                    if(n->kids[1]->typ->basetype == ARR_TYPE)
                    {
                        n->typ = n->kids[1]->typ;
                        n->typ->u.a.elemtype = n->kids[0]->typ;
                    }
                    else
                    {
                        n->typ->u.plist.type = n->kids[0]->typ;
                    }
                }
            }
            break;
        case DECLARATORS_FUN: /* 5651 */
            /* no return type specified on function, assume void */
            if(n->kids[0]->prodrule = DECLARATORS_DIRDEC2)
            {
                n->typ = n->kids[0]->typ;
                n->typ->name = copy_name(n->kids[0]->typ->name);
            }
            else
            {
                n->typ = make_room(FUNC_TYPE);
                n->typ->u.f.returntype->basetype = NULL_TYPE;
                n->typ->u.f.parameters = &n->kids[0]->typ->u.plist;
                if(n->nkids > 1)
                {
                    n->typ->name = copy_name(n->kids[1]->typ->name);   
                }
                else
                {
                    n->typ->name = copy_name(n->kids[0]->typ->name);
                }
            }
            break;
        case DECLARATORS_FUN2: /* 5652 */
            /* function return type specified */
            n->typ = n->kids[1]->typ;
            n->typ->u.f.returntype = n->kids[0]->typ;
            break;
        case CLASSES_SPEC: /* 6001 */
            n->typ = n->kids[0]->typ;
            break;
        case CLASSES_HEAD: /* 6011 */
            /* kid 0 = class, kid 1 = ID */
            n->typ = n->kids[0]->typ;
            n->typ->name = copy_name(n->kids[1]->typ->name);
            n->typ->u.c.classname = copy_name(n->typ->name);
            n->typ->isProto = 1;
            break;
        case CLASSES_KEY: /* 6021 */
            n->typ = make_room(CLASS_TYPE);
            n->prototype = 1; /* class declared */
            break;
        case CLASSES_MEMDEC: /* 6041 */
            if(n->kids[0]->typ->basetype == CLASS_TYPE)
            { /* make kid[1] a function */
                n->typ = make_room(FUNC_TYPE);
                n->typ->u.f.returntype = n->kids[0]->typ;
                n->typ->name = copy_name(n->kids[1]->typ->name);
                n->typ->u.f.classname = copy_name(n->kids[0]->leaf->text);
            }
            else
            {
                if(n->kids[1]->typ->basetype == FUNC_TYPE)
                {
                    n->typ = n->kids[1]->typ;
                    n->typ->u.f.returntype = n->kids[0]->typ;
                }
                else
                {
                    n->typ = n->kids[0]->typ;
                    n->typ->name = copy_name(n->kids[1]->typ->name);
                }
            }
            break;
        //case DCLASSES_ACCESS: /* 6131 */
        //case DCLASSES_ACCESS2: /* 6132 */
            /* assume that variables are private, functions are public */
         
     }
}

void FinshClassDefines(parsetree *n)
{
    int i;
    struct typeinfo *tmp;
    struct classmember *clist = 0;
    struct classmember *cnext = 0;
    if(n == NULL)
    {
        return;
    }
    for(i = 0; i < n->nkids; i++)   
    {
        FinshClassDefines(n->kids[i]);
    }
    switch(n->prodrule)
    {
        case CLASSES_SPEC: /* 6001 */
            /* add member list */
            n->typ->u.c.member = &n->kids[1]->typ->u.cm;
            break;
        case CLASSES_MEMSPEC: /* 6031 */
            n->typ = make_room(CLASSMEM_TYPE);
            if(n->nkids == 2)
            { /* we already have a linked list */
                cnext = next_mem();
                n->typ->u.cm.type = n->kids[0]->typ;
                cnext = &n->kids[1]->typ->u.cm;
                n->typ->u.cm.next = cnext;
            }
            else
            { /* we need to start a linked list */
                n->typ->u.cm.type = n->kids[0]->typ;
            }
            break;
        case CLASSES_MEMSPEC2: /* 6032 */
            n->typ = make_room(CLASSMEM_TYPE);
            cnext = next_mem();
            cnext->next = n->kids[1]->typ->u.cm.next;
            n->typ->u.cm.type = n->kids[1]->typ->u.cm.type;
            n->typ->u.cm.next = cnext->next;   
            break;
        case CLASSES_MEMDEC: /* 6041 */
            tmp = n->typ;
            n->typ = n->kids[1]->typ;
            if(n->typ->basetype == FUNC_TYPE)
            {
                n->typ->u.f.returntype = tmp;   
            }
            break;
    }
}

char *copy_name(char *s)
{
    char *retp = 0;
    if(s != NULL)
    {
        retp = strdup(s);
        if(retp == NULL)
        {
          fprintf(stderr, "ERROR: out of memory\n");
          exit(10);   
        }
    }
    return retp;
}

struct sym_table *symtable(int size)
{
    struct sym_table *tmp;
    tmp = (struct sym_table *)calloc(size, sizeof(struct sym_table));
    if(tmp == NULL)
    {
      fprintf(stderr, "ERROR: out of memory\n");
      exit(10);   
    }
    return tmp; 
}

struct sym_entry *next_sym()
{
    struct sym_entry *tmp;
    tmp = (struct sym_entry *)calloc(1, sizeof(struct sym_entry));
    if(tmp == NULL)
    {
        fprintf(stderr, "ERROR: out of memory\n");
        exit(10);   
    }   
    return tmp;
}

struct sym_entry *newsym_node(struct sym_table *symt, struct typeinfo *t) 
{
    struct sym_entry *newnode;
    newnode = next_sym();
    newnode->t = t;
    newnode->st = symt;
    newnode->name = copy_name(t->name);
    newnode->basetype = t->basetype;
    t->st = symt; 
    return newnode;
}

void sym_insert(struct sym_table *symt, struct typeinfo *t) 
{
    unsigned int hashIndex;
    hashIndex = hash_key(t->name);
    hashIndex %= SYMHASHSIZE;
    struct sym_entry *newnode =  newsym_node(symt, t);
    /* head of list for the bucket with index hashIndex */
    if (!symt[hashIndex].head) 
    {
        /* first entry for the bucket! */
        symt[hashIndex].head = newnode;
        symt[hashIndex].count = 1;
        return;
    }
    /* adding new node to the list */
    newnode->next = (symt[hashIndex].head);
    /* update the head of the list and no of
     * nodes in the current bucket */
    symt[hashIndex].head = newnode;
    symt[hashIndex].count++;
    return;
}

enum base_type search_sym(struct sym_table *symt, char *s, enum base_type whatiam) 
{
    unsigned int hashIndex;
    enum base_type return_bt;
    hashIndex = hash_key(s);
    hashIndex %= SYMHASHSIZE;
    struct sym_entry *myNode;
    return_bt = UNKNOWN_TYPE;
    myNode = symt[hashIndex].head;
    if(myNode) 
    {
        while (myNode != NULL) 
        {
            if(!strcmp(myNode->name, s)) 
            { 
                if(whatiam != UNKNOWN_TYPE)
                { /* check to see if whatiam definition is there, constructor */
                    if(myNode->basetype == whatiam)
                    {
                        return_bt = myNode->basetype;   
                    }
                }
                else
                { /* only 1 definition allowed */
                    return_bt = myNode->basetype;
                }
            }
            myNode = myNode->next;
        }
    }
    return return_bt;
}

struct typeinfo * search_type(struct sym_table *symt, char *s) 
{
    unsigned int hashIndex;
    struct typeinfo *returntype = 0;
    hashIndex = hash_key(s);
    hashIndex %= SYMHASHSIZE;
    struct sym_entry *myNode;
    myNode = symt[hashIndex].head;
    if(myNode) 
    {
        while (myNode != NULL) 
        {
            if(!strcmp(myNode->name, s)) 
            {
                returntype = myNode->t;
                break;
            }
            myNode = myNode->next;
        }
    }
    return returntype;
}

struct param *next_param()
{
    struct param *tmp;
    tmp = (struct param *)calloc(1, sizeof(struct param));
    if(tmp == NULL)
    {
        fprintf(stderr, "ERROR: out of memory\n");
        exit(10);
    }
    return tmp;
}

struct classmember *next_mem()
{
    struct classmember *tmp;
    tmp = (struct classmember *)calloc(1, sizeof(struct classmember));
    if(tmp == NULL)
    {
        fprintf(stderr, "ERROR: out of memory\n");
        exit(10);
    }
    return tmp;
}

struct typeinfo *next_type()
{
    struct typeinfo *tmp;
    tmp = (struct typeinfo *)calloc(1, sizeof(struct typeinfo));
    if(tmp == NULL)
    {
        fprintf(stderr, "ERROR: out of memory\n"); 
        exit(10);       
    }
    tmp->basetype = UNKNOWN_TYPE;
    return tmp;
}

struct typeinfo *make_room(enum base_type basetype)
{
    struct typeinfo *ptr;
    ptr = next_type();
    ptr->basetype = basetype;
    switch(basetype)
    {
        //case CLASS_TYPE:
            //ptr->u.c.st = symtable();
            //break;
        case ARR_TYPE:  
            ptr->u.a.elemtype = next_type();
            break;
        case PTR_TYPE:
            ptr->u.p.elemtype = next_type();
            break;
        case CLASSMEM_TYPE:
            ptr->u.cm.type = next_type();
            break;
        case PARAM_TYPE:
            ptr->u.plist.type = next_type();
            break;
        case FUNC_TYPE:
            //ptr->u.f.st = symtable();
            ptr->u.f.returntype = next_type();
            break;
    }   
    return ptr;
}

void print_list()
{
  struct tokenlist *lp;
  struct token *t;
  fprintf(stdout, "\nCategory\tText\t\tLineno\tFilename\tIval/Sval\n");
  fprintf(stdout, "-------------------------------------------------");
  fprintf(stdout, "--------------\n");
  lp = tlist; 
  while(lp)
  {
    /* step thru list & print */
    t = lp->t;
    fprintf(stdout, "%d\t\t%s\t\t%d\t%s\n", t->category, t->text
    , t->lineno, t->filename);
    lp = lp->next;
  }
}

void printer_debug(parsetree *t, int depth)
{
    /* code adopted from Dr. Jeffery class notes */
    int i;
    if(t==NULL)
    {
        return;
    }
    if(t->nkids > 0)
    {
        fprintf(stdout, "%*sproduction rule %d: kids %d: line %d\n"
        , depth*2, " ", t->prodrule, t->nkids, t->line_num);
        fprintf(stdout, "%*s(%s)\n", depth*2, " ", readable_rule(t->prodrule));
        for(i = 0; i < t->nkids; i++)
        {
            printer_debug(t->kids[i], depth+1);
        }
    }
    else
    {
        fprintf(stdout, "%*sproduction rule %d: token %s: line %d\n"
        , depth*2, " ", t->prodrule, t->leaf->text, t->line_num);
        fprintf(stdout, "%*s(%s)\n", depth*2, " "
        , readable_rule(t->prodrule));
    }
}

void show_symtabs()
{
    int i, k, m, n;
    struct sym_table *sym_array[SYMHASHSIZE];
    struct sym_entry *entry;
    struct sym_table *workingsymtab;
    
    if(!semantic_debug)
    { /* only do in debug mode */
        return;   
    }
    for(i = 0; i < SYMHASHSIZE; i++)
    { /* zero out pointer array */
        sym_array[i] = 0;
    }
    printf("\nGLOBAL SYM TABLE = %p with %d entries\n", symtab, symtab->count);
    workingsymtab = symtab; /* start with global */
    k = 0;
    for(i = 0; i < SYMHASHSIZE; i++)
    { /* print & fill in array of other sym tabs */
        entry = workingsymtab[i].head;
        while(entry != NULL)
        {
            print_symentry(entry);
            if(entry->t->st != workingsymtab && entry->t->st != NULL)
            { /* does it have a defined sym table of its own */
                for(m = 0; m < SYMHASHSIZE; m++)
                {
                    if(sym_array[m] == entry->t->st)
                    { /* already in array */
                        break;   
                    }
                }
                if(m >= SYMHASHSIZE && k < SYMHASHSIZE)
                { /* record in array */
                    sym_array[k] = entry->t->st;
                    k++;
                }
            }
            entry = entry->next;
        }
    }
    for(n = 0; n < SYMHASHSIZE; n++)
    { /* now do all the other found sym tabs */
        if(sym_array[n] != NULL)
        {
            workingsymtab = sym_array[n];   
            printf("\nLOCAL SYM TABLE = %p with %d entries\n", workingsymtab, workingsymtab->count);
            for(i = 0; i < SYMHASHSIZE; i++)
            { /* print & fill in array if other sym tabs */
                entry = workingsymtab[i].head;
                while(entry != NULL)
                {
                    print_symentry(entry);
                    if(entry->t->st != workingsymtab && entry->t->st != NULL)
                    { /* does it have a defined sym table of its own */
                        for(m = 0; m < SYMHASHSIZE; m++)
                        {
                            if(sym_array[m] == entry->t->st)
                            { /* already in array */
                                break;   
                            }
                        }
                        if(m >= SYMHASHSIZE && k < SYMHASHSIZE)
                        { /* record in array */
                            sym_array[k] = entry->t->st;
                            k++;
                        }
                    }
                    entry = entry->next;
                }
            }
        }
    }
}

void print_symentry(struct sym_entry *entry)
{
    char *typename[16] = {"INT_TYPE","BOOL_TYPE","FLOAT_TYPE","CHAR_TYPE","NULL_TYPE"
                         ,"CLASS_TYPE","not used","PTR_TYPE","ARR_TYPE","FUNC_TYPE"
                         ,"PARAM_TYPE","UNKNOWN_TYPE","CLASSMEM_TYPE","CONSTR_TYPE"
                         ,"NAMESPACE_TYPE","STRUCT_TYPE"};
    if(!semantic_debug)
    {
        return;
    }
    if(entry->basetype == FUNC_TYPE)
    {
        printf("%s is type %s with offset = %s, isParam = %d, isProto = %d, Size = %d, Paramsize = %d\n", 
        entry->name, typename[entry->basetype], entry->t->memaddr.u.name, entry->t->isParam, entry->t->isProto, 
        entry->t->memaddr.size, entry->t->u.f.paramsize); 
        printf("\tclassname = %s, returntype = %s\n",entry->t->u.f.classname, typename[entry->t->u.f.returntype->basetype]);
    }
    else
    {
        printf("%s is type %s with offset = %d, isParam = %d, isProto = %d, Size = %d\n", 
        entry->name, typename[entry->basetype], entry->t->memaddr.u.offset, entry->t->isParam,
        entry->t->isProto, entry->t->memaddr.size); 
    }
    //printf("\t\tn->typ = %p\n", entry->t);
}
