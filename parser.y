/*  Intermediate Code
 *  Homework 4
 *  CS 445, Dr. Jeffery
 *  Nov. 24, 2014
 *  Kathryn Price
 */

/*
 * Grammar for 120++, a subset of C++ used in CS 120 at University of Idaho
 *
 * Adaptation by Clinton Jeffery, with help from Matthew Brown, Ranger
 * Adams, and Shea Newton.
 *
 * Based on Sandro Sigala's transcription of the ISO C++ 1996 draft standard.
 * 
 */

/*  $Id: parser.y,v 1.3 1997/11/19 15:13:16 sandro Exp $    */

/*
 * Copyright (c) 1997 Sandro Sigala <ssigala@globalnet.it>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* 120++ parser based on
 * ISO C++ parser.
 * Based on the ISO C++ draft standard of December '96.
 */

%{
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "120++.h"
#include "rules.h"

extern int semantic_debug;
extern int parser_debug;

extern int StructKeywordSeen;
extern int namespace_found;
extern int lineno;
int yydebug=0;
/* 0= no errs, 1= lex errs, 2 = syntax errs; 3 = not supported */
int exit_num = 0; 
static void yyerror(char *s);
extern parsetree *parse_tree;
parsetree *create_node(int rule, int nkids, ...);
parsetree *create_node1(int rule, struct tree *t);
parsetree *create_node2(int rule, struct tree *t1, struct tree *t2, int splice);
parsetree *create_node3(int rule, struct tree *t1, struct tree *t2, struct tree *t3);
parsetree *create_node4(int rule, struct tree *t1, struct tree *t2, struct tree *t3, struct tree *t4);
parsetree *create_leaf(int rule, struct token *l);
parsetree *create_leaf2(int rule, struct token *l1, struct token *l2);
extern char *curfilename;
extern int lineno;
void printer(parsetree *t, int depth);
extern void insertToHash(char *s, int cat);
char *ident_tosave = 0;
int ident_cat = 0;
extern char *readable_rule(int rule);
void semantic_error(char *s);
parsetree *create_leaf2node(int rule, struct token *l, struct tree *t1, struct tree *t2);
parsetree *create_leaf1node(int rule, struct token *l, struct tree *t1);
%}

%union {
struct token *tokptr;
struct tree *treeptr;
}

%token <tokptr> IDENTIFIER INTEGER FLOATING CHARACTER STRING
%token <tokptr> TYPEDEF_NAME NAMESPACE_NAME CLASS_NAME 

%token <tokptr> COLON COLONCOLON DOTSTAR ADDEQ SUBEQ MULEQ DIVEQ MODEQ
%token <tokptr> XOREQ ANDEQ OREQ SL SR SREQ SLEQ EQ NOTEQ LTEQ GTEQ ANDAND 
%token <tokptr> OROR PLUSPLUS MINUSMINUS ARROWSTAR ARROW

%token <tokptr> AND ASM ASN AUTO BANG BOOL BREAK CASE CATCH CHAR CLASS 
%token <tokptr> CONST CONST_CAST COMMA CONTINUE
%token <tokptr> DEFAULT DELETE DIV DO DOT DOUBLE DYNAMIC_CAST ELLIPSIS ELSE 
%token <tokptr> ENUM ENUM_NAME ER EXPLICIT EXTERN EXPORT
%token <tokptr> FALSE FLOAT FOR FRIEND GOTO GT IF INLINE INT LB LC LP LONG LT 
%token <tokptr> MINUS MOD MUTABLE NAMESPACE NEW NOT
%token <tokptr> OPERATOR OR PLUS PRIVATE PROTECTED PUBLIC QUEST RB RC RP 
%token <tokptr> REGISTER REINTERPRET_CAST RETURN 
%token <tokptr> SHORT SIGNED SIZEOF SM STAR STATIC STATIC_CAST STRUCT SWITCH 
%token <tokptr> TEMPLATE TEMPLATE_NAME THIS
%token <tokptr> THROW TRUE TRY TYPEDEF TYPEID TYPENAME UNION UNSIGNED USING 
%token <tokptr> VIRTUAL VOLATILE VOID WCHAR_T WHILE

%start translation_unit

%type <treeptr> typedef_name 
%type <treeptr> namespace_name 
%type <treeptr> original_namespace_name 
%type <treeptr> class_name 
%type <treeptr> enum_name 
%type <treeptr> template_name 
%type <treeptr> identifier 
%type <treeptr> literal 
%type <treeptr> integer_literal 
%type <treeptr> character_literal 
%type <treeptr> floating_literal 
%type <treeptr> string_literal 
%type <treeptr> boolean_literal 
%type <treeptr> translation_unit 
%type <treeptr> primary_expression 
%type <treeptr> id_expression 
%type <treeptr> unqualified_id 
%type <treeptr> qualified_id 
%type <treeptr> nested_name_specifier 
%type <treeptr> postfix_expression 
%type <treeptr> expression_list 
%type <treeptr> unary_expression 
%type <treeptr> unary_operator 
%type <treeptr> new_expression 
%type <treeptr> new_placement 
%type <treeptr> new_type_id 
%type <treeptr> new_declarator 
%type <treeptr> direct_new_declarator 
%type <treeptr> new_initializer 
%type <treeptr> delete_expression 
%type <treeptr> cast_expression 
%type <treeptr> pm_expression 
%type <treeptr> multiplicative_expression 
%type <treeptr> additive_expression 
%type <treeptr> shift_expression 
%type <treeptr> relational_expression 
%type <treeptr> equality_expression 
%type <treeptr> and_expression 
%type <treeptr> exclusive_or_expression 
%type <treeptr> inclusive_or_expression 
%type <treeptr> logical_and_expression 
%type <treeptr> logical_or_expression 
%type <treeptr> conditional_expression 
%type <treeptr> assignment_expression 
%type <treeptr> assignment_operator 
%type <treeptr> expression 
%type <treeptr> constant_expression 
%type <treeptr> statement 
%type <treeptr> labeled_statement 
%type <treeptr> expression_statement 
%type <treeptr> compound_statement 
%type <treeptr> statement_seq 
%type <treeptr> selection_statement 
%type <treeptr> condition 
%type <treeptr> iteration_statement 
%type <treeptr> for_init_statement 
%type <treeptr> jump_statement 
%type <treeptr> declaration_statement 
%type <treeptr> declaration_seq 
%type <treeptr> declaration 
%type <treeptr> block_declaration 
%type <treeptr> simple_declaration 
%type <treeptr> decl_specifier 
%type <treeptr> decl_specifier_seq 
%type <treeptr> storage_class_specifier 
%type <treeptr> function_specifier 
%type <treeptr> type_specifier 
%type <treeptr> simple_type_specifier 
%type <treeptr> type_name 
%type <treeptr> elaborated_type_specifier 
%type <treeptr> enum_specifier 
%type <treeptr> enumerator_list 
%type <treeptr> enumerator_definition 
%type <treeptr> enumerator 
%type <treeptr> namespace_definition 
%type <treeptr> named_namespace_definition 
%type <treeptr> original_namespace_definition 
%type <treeptr> extension_namespace_definition 
%type <treeptr> unnamed_namespace_definition 
%type <treeptr> namespace_body 
%type <treeptr> namespace_alias_definition 
%type <treeptr> qualified_namespace_specifier 
%type <treeptr> using_declaration 
%type <treeptr> using_directive 
%type <treeptr> asm_definition 
%type <treeptr> linkage_specification 
%type <treeptr> init_declarator_list 
%type <treeptr> init_declarator 
%type <treeptr> declarator 
%type <treeptr> direct_declarator 
%type <treeptr> ptr_operator 
%type <treeptr> cv_qualifier_seq 
%type <treeptr> cv_qualifier 
%type <treeptr> declarator_id 
%type <treeptr> type_id 
%type <treeptr> type_specifier_seq 
%type <treeptr> abstract_declarator 
%type <treeptr> direct_abstract_declarator 
%type <treeptr> parameter_declaration_clause 
%type <treeptr> parameter_declaration_list 
%type <treeptr> parameter_declaration 
%type <treeptr> function_definition 
%type <treeptr> function_body 
%type <treeptr> initializer 
%type <treeptr> initializer_clause 
%type <treeptr> initializer_list 
%type <treeptr> class_specifier 
%type <treeptr> class_head 
%type <treeptr> class_key 
%type <treeptr> member_specification 
%type <treeptr> member_declaration 
%type <treeptr> member_declarator_list 
%type <treeptr> member_declarator 
%type <treeptr> pure_specifier 
%type <treeptr> constant_initializer 
%type <treeptr> base_clause 
%type <treeptr> base_specifier_list 
%type <treeptr> base_specifier 
%type <treeptr> access_specifier 
%type <treeptr> conversion_function_id 
%type <treeptr> conversion_type_id 
%type <treeptr> conversion_declarator 
%type <treeptr> ctor_initializer 
%type <treeptr> mem_initializer_list 
%type <treeptr> mem_initializer 
%type <treeptr> mem_initializer_id 
%type <treeptr> operator_function_id 
%type <treeptr> operator 
%type <treeptr> template_declaration 
%type <treeptr> template_parameter_list 
%type <treeptr> template_parameter 
%type <treeptr> type_parameter 
%type <treeptr> template_id 
%type <treeptr> template_argument_list 
%type <treeptr> template_argument 
%type <treeptr> explicit_instantiation 
%type <treeptr> explicit_specialization 
%type <treeptr> try_block 
%type <treeptr> function_try_block 
%type <treeptr> handler_seq 
%type <treeptr> handler 
%type <treeptr> exception_declaration 
%type <treeptr> throw_expression 
%type <treeptr> exception_specification 
%type <treeptr> type_id_list 
%type <treeptr> declaration_seq_opt 
%type <treeptr> nested_name_specifier_opt 
%type <treeptr> expression_list_opt 
%type <treeptr> COLONCOLON_opt 
%type <treeptr> new_placement_opt 
%type <treeptr> new_initializer_opt 
%type <treeptr> new_declarator_opt 
%type <treeptr> expression_opt 
%type <treeptr> statement_seq_opt 
%type <treeptr> condition_opt 
%type <treeptr> enumerator_list_opt 
%type <treeptr> initializer_opt 
%type <treeptr> constant_expression_opt 
%type <treeptr> abstract_declarator_opt 
%type <treeptr> type_specifier_seq_opt 
%type <treeptr> direct_abstract_declarator_opt 
%type <treeptr> ctor_initializer_opt 
%type <treeptr> COMMA_opt 
%type <treeptr> member_specification_opt 
%type <treeptr> SEMICOLON_opt 
%type <treeptr> conversion_declarator_opt 
%type <treeptr> EXPORT_opt 
%type <treeptr> handler_seq_opt 
%type <treeptr> assignment_expression_opt 
%type <treeptr> type_id_list_opt


%%

/*----------------------------------------------------------------------
 * Context-dependent identifiers.
 *----------------------------------------------------------------------*/

typedef_name:
    /* identifier */
    TYPEDEF_NAME
        { $$ = NULL;
          semantic_error("ERROR: typedef not supported!\n"); }
    ;

namespace_name:
    original_namespace_name
        {  $$ = $1; }
    ;

original_namespace_name:
    /* identifier */
    NAMESPACE_NAME
        { $$ = create_leaf(IDENTIFIERS_ORIG_NS, $1); }
    ;

class_name:
    /* identifier */
    CLASS_NAME
        { $$ = create_leaf(IDENTIFIERS_CLASS, $1); }
    | template_id
        { $$ = NULL;
          semantic_error("ERROR: templates not supported!\n"); 
        }
    ;

enum_name:
    /* identifier */
    ENUM_NAME
        { $$ = NULL; 
         semantic_error("ERROR:  enum keyword not supported!\n"); 
       }
    ;

template_name:
    /* identifier */
    TEMPLATE_NAME
        { $$ = NULL; 
          semantic_error("ERROR:  template keyword not supported!\n"); 
        }
    ;

/*----------------------------------------------------------------------
 * Lexical elements.
 *----------------------------------------------------------------------*/

identifier:
    IDENTIFIER
        { $$ = create_leaf(LEXICAL_IDENT, $1); }
    ;

literal:
    integer_literal
        {  $$ = $1; }
    | character_literal
        {  $$ = $1; }
    | floating_literal
        {  $$ = $1; }
    | string_literal
        {  $$ = $1; }
    | boolean_literal
        {  $$ = $1; }
    ;

integer_literal:
    INTEGER
        { $$ = create_leaf(LEXICAL_INTLIT, $1); }
    ;

character_literal:
    CHARACTER
        { $$ = create_leaf(LEXICAL_CHARLIT, $1); }
    ;

floating_literal:
    FLOATING
        { $$ = create_leaf(LEXICAL_FLOATLIT, $1); }
    ;

string_literal:
    STRING
        { $$ = create_leaf(LEXICAL_STRINGLIT, $1); }
    ;

boolean_literal:
    TRUE
        { $$ = create_leaf(LEXICAL_BOOLLIT, $1); }
    | FALSE
        { $$ = create_leaf(LEXICAL_BOOLLIT2, $1); }
    ;

/*----------------------------------------------------------------------
 * Translation unit.
 *----------------------------------------------------------------------*/

translation_unit:
    declaration_seq_opt
        {  $$ = $1; }
    ;

/*----------------------------------------------------------------------
 * Expressions.
 *----------------------------------------------------------------------*/

primary_expression:
    literal
        {  $$ = $1; }
    | THIS
        { $$ = NULL; 
          semantic_error("ERROR:  this keyword not supported!\n"); }
    | LP expression RP
        {  $$ = $2; }
    | id_expression
        {  $$ = $1; }
    ;

id_expression:
    unqualified_id
        {  $$ = $1; }
    | qualified_id
        {  $$ = $1; }
    ;

unqualified_id:
    identifier
        {  $$ = $1; }
    | operator_function_id
         {  $$ = $1; }
    | conversion_function_id
         {  $$ = $1; }
    | NOT class_name
        { $$ = NULL; 
          semantic_error("ERROR:  class feature not supported!\n"); 
        }
    ;

qualified_id:
    nested_name_specifier unqualified_id
        {  $$ = create_node2(EXPRESSION_QID, $1, $2, 0); }
    | nested_name_specifier TEMPLATE unqualified_id
        { $$ = NULL; 
          semantic_error("ERROR:  templates not supported!\n"); }
    ;

nested_name_specifier:
    class_name COLONCOLON nested_name_specifier
    namespace_name COLONCOLON nested_name_specifier
        {  $$ = NULL; 
           semantic_error("ERROR:  this use of :: not supported!\n"); }
    | class_name COLONCOLON
        { $$ = $1; }
    | namespace_name COLONCOLON
        { $$ = NULL;
          semantic_error("ERROR:  this use of :: not supported!\n"); }

    ;

postfix_expression:
    primary_expression
        {  $$ = $1; }
    | postfix_expression LB expression RB
        {  $$ = create_node2(EXPRESSION_POSTFIX2, $1, $3, 0); }
    | postfix_expression LP expression_list_opt RP
        {  $$ = create_node2(EXPRESSION_POSTFIX3, $1, $3, 0); }
    | postfix_expression DOT TEMPLATE COLONCOLON id_expression
        { $$ = NULL; 
          semantic_error("ERROR:  templates not supported!\n"); }
    | postfix_expression DOT TEMPLATE id_expression
        { $$ = NULL; 
          semantic_error("ERROR:  templates not supported!\n"); }
    | postfix_expression DOT COLONCOLON id_expression
        {  $$ = NULL; 
           semantic_error("ERROR:  this use of :: not supported!\n"); }
    | postfix_expression DOT id_expression
        {  $$ = create_node2(EXPRESSION_POSTFIX5, $1, $3, 0); }
    | postfix_expression ARROW TEMPLATE COLONCOLON id_expression
        { $$ = NULL; 
          semantic_error("ERROR:  template not supported!\n"); }
    | postfix_expression ARROW TEMPLATE id_expression
        { $$ = NULL; 
          semantic_error("ERROR:  template not supported!\n"); }
    | postfix_expression ARROW COLONCOLON id_expression
        {  $$ = NULL; 
           semantic_error("ERROR:  this use of :: not supported!\n"); }
    | postfix_expression ARROW id_expression
        {  $$ = NULL; 
           semantic_error("ERROR:  this use of -> not supported!\n"); }
    | postfix_expression PLUSPLUS
        {  $$ = create_node1(EXPRESSION_POSTFIX8, $1); }
    | postfix_expression MINUSMINUS
        {  $$ = create_node1(EXPRESSION_POSTFIX9, $1); }
    | DYNAMIC_CAST LT type_id GT LP expression RP
        { $$ = NULL; 
          semantic_error("ERROR:  dynamic cast not supported!\n"); }
    | STATIC_CAST LT type_id GT LP expression RP
        {  $$ = NULL; 
           semantic_error("ERROR:  static cast not supported!\n"); }
    | REINTERPRET_CAST LT type_id GT LP expression RP
        { $$ = NULL; 
          semantic_error("ERROR:  reinterpret cast not supported!\n"); }
    | CONST_CAST LT type_id GT LP expression RP
        { $$ = NULL; 
          semantic_error("ERROR:  const cast not supported!\n"); }
    | TYPEID LP expression RP
        { $$ = NULL; 
          semantic_error("ERROR:  typeid not supported!\n"); }
    | TYPEID LP type_id RP
        {  $$ = NULL; 
          semantic_error("ERROR:  typeid not supported!\n");  }
    ;

expression_list:
    assignment_expression
        {  $$ = $1; }
    | expression_list COMMA assignment_expression
        {  $$ = create_node2(EXPRESSION_LIST2, $1, $3, 0); }
    ;

unary_expression:
    postfix_expression
        {  $$ = $1; }
    | PLUSPLUS cast_expression
        {  $$ = create_node1(EXPRESSION_UNEX2, $2); }
    | MINUSMINUS cast_expression
        {  $$ = create_node1(EXPRESSION_UNEX3, $2); }
    | STAR cast_expression
        {  $$ = create_node1(EXPRESSION_UNEX4, $2); }
    | AND cast_expression
        {  $$ = create_node1(EXPRESSION_UNEX5, $2); }
    | unary_operator cast_expression
        {  $$ = create_node2(EXPRESSION_UNEX6, $1, $2, 0); }
    | SIZEOF unary_expression
        {  $$ = NULL; 
           semantic_error("ERROR:  sizeof keyword not supported!\n"); }
    | SIZEOF LP type_id RP
        {  $$ = NULL; 
           semantic_error("ERROR:  sizeof keyword not supported!\n"); }
    | new_expression
        {  $$ = $1; }
    | delete_expression
        {  $$ = $1; }
    ;

unary_operator:
      PLUS
        { $$ = create_leaf(EXPRESSION_UNOP, $1); }
    | MINUS
        { $$ = create_leaf(EXPRESSION_UNOP2, $1); }
    | BANG
        { $$ = create_leaf(EXPRESSION_UNOP3, $1); }
    | NOT
        { $$ = create_leaf(EXPRESSION_UNOP4, $1); }
    ;

new_expression:
      NEW new_placement_opt new_type_id new_initializer_opt
        {  $$ = create_node3(EXPRESSION_NEWEX, $2, $3, $4); }
    | COLONCOLON NEW new_placement_opt new_type_id new_initializer_opt
        {  $$ = NULL; 
           semantic_error("ERROR:  this use of :: not supported!\n"); }
    | NEW new_placement_opt LP type_id RP new_initializer_opt
        {  $$ = create_node3(EXPRESSION_NEWEX3, $2, $4, $6); }
    | COLONCOLON NEW new_placement_opt LP type_id RP new_initializer_opt
        {  $$ = NULL; 
           semantic_error("ERROR:  this use of :: not supported!\n"); }
    ;

new_placement:
    LP expression_list RP
        {  $$ = $2; }
    ;

new_type_id:
    type_specifier_seq new_declarator_opt
        {  $$ = create_node2(EXPRESSION_NEWTYPE, $1, $2, 0); }
    ;

new_declarator:
    ptr_operator new_declarator_opt
        {  $$ = create_node2(EXPRESSION_NEWDEC, $1, $2, 0); }
    | direct_new_declarator
        {  $$ = $1; }
    ;

direct_new_declarator:
    LB expression RB
        {  $$ =$2; }
    | direct_new_declarator LB constant_expression RB
        {  $$ = create_node2(EXPRESSION_DIRDEC2, $1, $3, 0); }
    ;

new_initializer:
    LP expression_list_opt RP
        {  $$ = $2; }
    ;

delete_expression:
      DELETE cast_expression
        {  $$ = create_node1(EXPRESSION_DELEX, $2); }
    | COLONCOLON DELETE cast_expression
        {  $$ = NULL; 
           semantic_error("ERROR:  this use of :: not supported!\n"); }
    | DELETE LB RB cast_expression
        {  $$ = NULL; 
           semantic_error("ERROR:  this use of delete not supported!\n");  }
    | COLONCOLON DELETE LB RB cast_expression
        {  $$ = NULL; 
           semantic_error("ERROR:  this use of :: not supported!\n"); }
    ;

cast_expression:
    unary_expression
        {  $$ = $1; }
    | LP type_id RP cast_expression
        {  $$ = create_node2(EXPRESSION_CAST2, $2, $4, 0); }
    ;

pm_expression:
    cast_expression
        {  $$ = $1; }
    | pm_expression DOTSTAR cast_expression
        {  $$ = create_node2(EXPRESSION_PM2, $1, $3, 0); }
    | pm_expression ARROWSTAR cast_expression
        {  $$ = create_node2(EXPRESSION_PM3, $1, $3, 0); }
    ;

multiplicative_expression:
    pm_expression
        {  $$ = $1; }
    | multiplicative_expression STAR pm_expression
        {  $$ = create_node2(EXPRESSION_MULT2, $1, $3, 0); }
    | multiplicative_expression DIV pm_expression
        {  $$ = create_node2(EXPRESSION_MULT3, $1, $3, 0); }
    | multiplicative_expression MOD pm_expression
        {  $$ = create_node2(EXPRESSION_MULT4, $1, $3, 0); }
    ;

additive_expression:
    multiplicative_expression
        {  $$ = $1; }
    | additive_expression PLUS multiplicative_expression
        {  $$ = create_node2(EXPRESSION_ADD2, $1, $3, 0); }
    | additive_expression MINUS multiplicative_expression
        {  $$ = create_node2(EXPRESSION_ADD3, $1, $3, 0); }
    ;

shift_expression:
    additive_expression
        {  $$ = $1; }
    | shift_expression SL additive_expression
        {  $$ = create_node2(EXPRESSION_SHIFT2, $1, $3, 0); }
    | shift_expression SR additive_expression
        {  $$ = create_node2(EXPRESSION_SHIFT3, $1, $3, 0); }
    ;

relational_expression:
    shift_expression
        {  $$ = $1; }
    | relational_expression LT shift_expression
        {  $$ = create_node2(EXPRESSION_REL2, $1, $3, 0); }
    | relational_expression GT shift_expression
        {  $$ = create_node2(EXPRESSION_REL3, $1, $3, 0); }
    | relational_expression LTEQ shift_expression
        {  $$ = create_node2(EXPRESSION_REL4, $1, $3, 0); }
    | relational_expression GTEQ shift_expression
        {  $$ = create_node2(EXPRESSION_REL5, $1, $3, 0); }
    ;

equality_expression:
    relational_expression
        {  $$ = $1; }
    | equality_expression EQ relational_expression
        {  $$ = create_node2(EXPRESSION_EQ2, $1, $3, 0); }
    | equality_expression NOTEQ relational_expression
        {  $$ = create_node2(EXPRESSION_EQ3, $1, $3, 0); }
    ;

and_expression:
    equality_expression
        {  $$ = $1; }
    | and_expression AND equality_expression
        {  $$ = NULL; 
           semantic_error("ERROR:  bitwise and not supported!\n"); }
    ;

exclusive_or_expression:
    and_expression
        {  $$ = $1; }
    | exclusive_or_expression ER and_expression
        {  $$ = NULL; 
           semantic_error("ERROR:  bitwise xor not supported!\n"); }
    ;

inclusive_or_expression:
    exclusive_or_expression
        {  $$ = $1; }
    | inclusive_or_expression OR exclusive_or_expression
        {  $$ = NULL; 
            semantic_error("ERROR:  bitwise or not supported!\n"); }
    ;

logical_and_expression:
    inclusive_or_expression
        {  $$ = $1; }
    | logical_and_expression ANDAND inclusive_or_expression
        {  $$ = create_node2(EXPRESSION_LOGAND2, $1, $3, 0); }
    ;

logical_or_expression:
    logical_and_expression
        {  $$ = $1; }
    | logical_or_expression OROR logical_and_expression
        {  $$ = create_node2(EXPRESSION_LOGOR2, $1, $3, 0); }
    ;

conditional_expression:
    logical_or_expression
        {  $$ = $1; }
    | logical_or_expression  QUEST expression COLON assignment_expression
        {  $$ = create_node3(EXPRESSION_COND, $1, $3, $5); }
    ;

assignment_expression:
    conditional_expression
        {  $$ = $1; }
    | logical_or_expression assignment_operator assignment_expression
        {  $$ = create_node3(EXPRESSION_ASN2, $1, $2, $3); }
    | throw_expression
        {  $$ = NULL; 
           semantic_error("ERROR:  throw exceptions not supported!\n"); }
    ;

assignment_operator:
    ASN
        { $$ = create_leaf(EXPRESSION_OP, $1); }
    | MULEQ
        { $$ = create_leaf(EXPRESSION_OP2, $1); }
    | DIVEQ
        { $$ = create_leaf(EXPRESSION_OP3, $1); }
    | MODEQ
        { $$ = create_leaf(EXPRESSION_OP4, $1); }
    | ADDEQ
        { $$ = create_leaf(EXPRESSION_OP5, $1); }
    | SUBEQ
        { $$ = create_leaf(EXPRESSION_OP6, $1); }
    | SREQ
        { $$ = NULL; 
          semantic_error("ERROR:  bitwise shift not supported!\n"); }
    | SLEQ
        { $$ = NULL; 
          semantic_error("ERROR:  bitwise shift not supported!\n"); }
    | ANDEQ
        { $$ = NULL; 
          semantic_error("ERROR:  bitwise and not supported!\n"); }
    | XOREQ
        { $$ = NULL; 
           semantic_error("ERROR:  bitwise xor not supported!\n"); }
    | OREQ
        { $$ = NULL; 
           semantic_error("ERROR:  bitwise or not supported!\n"); }
    ;

expression:
    assignment_expression
        {  $$ = $1; }
    | expression COMMA assignment_expression
        {  $$ = create_node2(EXPRESSION_EX2, $1, $3, 0); }
    ;

constant_expression:
    conditional_expression
        {  $$ = $1; }
    ;

/*----------------------------------------------------------------------
 * Statements.
 *----------------------------------------------------------------------*/

statement:
    labeled_statement
        {  $$ = create_node1(STATEMENTS_STMT, $1); }
    | expression_statement
        {  $$ = create_node1(STATEMENTS_STMT2, $1); }
    | compound_statement
        {  $$ = create_node1(STATEMENTS_STMT3, $1); }
    | selection_statement
        {  $$ = create_node1(STATEMENTS_STMT4, $1); }
    | iteration_statement
        {  $$ = create_node1(STATEMENTS_STMT5, $1); }
    | jump_statement
        {  $$ = create_node1(STATEMENTS_STMT6, $1); }
    | declaration_statement
        {  $$ = create_node1(STATEMENTS_STMT7, $1); }
    | try_block
        { $$ = NULL; 
          semantic_error("ERROR:  try/throw features not supported!\n"); }
    ;

labeled_statement:
    identifier COLON statement
        {  $$ = create_node2(STATEMENTS_LABEL, $1, $3, 0); }
    | CASE constant_expression COLON statement
        {  $$ = create_node2(STATEMENTS_LABEL2, $2, $4, 0); }
    | DEFAULT COLON statement
        {  $$ = $3; }
    ;

expression_statement:
    expression_opt SM
        {  $$ = $1; }
    ;

compound_statement:
    LC statement_seq_opt RC
        {  if($2 == NULL) 
           {
                $$ = NULL;
           }
           else
           {
                $$ = $2; 
           }
        }
    ;

statement_seq:
    statement
        {  $$ = $1; }
    | statement_seq statement
        {  $$ = create_node2(STATEMENTS_STSEQ2, $1, $2, 0); }
    ;

selection_statement:
    IF LP condition RP statement
        {  $$ = create_node2(STATEMENTS_SEL, $3, $5, 0); }
    | IF LP condition RP statement ELSE statement
        {  $$ = create_node3(STATEMENTS_SEL2, $3, $5, $7); }
    | SWITCH LP condition RP statement
        {  $$ = create_node2(STATEMENTS_SEL3, $3, $5, 0); }
    ;

condition:
    expression
        {  $$ = $1; }
    | type_specifier_seq declarator ASN assignment_expression
        {  $$ = create_node3(STATEMENTS_COND2, $1, $2, $4); }
    ;

iteration_statement:
    WHILE LP condition RP statement
        {  $$ = create_node2(STATEMENTS_ITER, $3, $5, 0); }
    | DO statement WHILE LP expression RP SM
        {  $$ = NULL; 
           semantic_error("ERROR:  do-while keyword not supported!\n"); }
    | FOR LP for_init_statement condition_opt SM expression_opt RP statement
        {  $$ = create_node4(STATEMENTS_ITER3, $3, $4, $6, $8); }
    ;

for_init_statement:
    expression_statement
        {  $$ = create_node1(STATEMENTS_FOR, $1); }
    | simple_declaration
        {  $$ = create_node1(STATEMENTS_FOR2, $1); }
    ;

jump_statement:
    BREAK SM
        { $$ = create_leaf(STATEMENTS_JUMP, $1); }
    | CONTINUE SM
        { $$ = NULL; 
           semantic_error("ERROR:  continue keyword not supported!\n"); }
    | RETURN expression_opt SM
        {  $$ = create_node1(STATEMENTS_JUMP3, $2); }
    | GOTO identifier SM
        {  $$ = NULL; 
           semantic_error("ERROR:  goto keyword not supported!\n"); }
    ;

declaration_statement:
    block_declaration
        {  $$ = $1; }
    ;

/*----------------------------------------------------------------------
 * Declarations.
 *----------------------------------------------------------------------*/

declaration_seq:
    declaration
        {  $$ = $1; }
    | declaration_seq declaration
        {  $$ = create_node2(DECLARATIONS_SEQ, $1, $2, 0); }
    ;

declaration:
    block_declaration
        {  $$ = $1; }
    | function_definition
        {  $$ = $1; }
    | template_declaration
        { $$ = NULL; 
          semantic_error("ERROR:  templates not supported!\n"); }
    | explicit_instantiation
        {  $$ = $1; }
    | explicit_specialization
        {  $$ = $1; }
    | linkage_specification
        {  $$ = $1; }
    | namespace_definition
        {  $$ = $1; }
    ;

block_declaration:
    simple_declaration
        {  $$ = $1; }
    | asm_definition
        {  $$ = NULL; 
           semantic_error("ERROR:  asm keyword not supported!\n"); }
    | namespace_alias_definition
        {  $$ = $1; }
    | using_declaration
        {  $$ = $1; }
    | using_directive
        {  $$ = $1; }
    ;

simple_declaration:
      decl_specifier_seq init_declarator_list SM
        {  $$ = create_node2(DECLARATIONS_SIMPLE, $1, $2, 0); }
    |  decl_specifier_seq SM
        {  $$ = $1; }
    ;

decl_specifier:
    storage_class_specifier
        {  $$ = NULL; 
         semantic_error("ERROR: storage class specifier not supported!\n");; }
    | type_specifier
        {  $$ = $1; }
    | function_specifier
        {  $$ = $1; }
    | FRIEND
        { $$ = NULL; 
          semantic_error("ERROR:  friend keyword not supported!\n"); 
        }
    | TYPEDEF
        { $$ = NULL;
          semantic_error("ERROR: typedef not supported!\n"); }
    ;

decl_specifier_seq:
      decl_specifier
        {  $$ = $1; }
    | decl_specifier_seq decl_specifier
        {  $$ = create_node2(DECLARATIONS_DECLSEQ, $1, $2, 0); }
    ;

storage_class_specifier:
    AUTO
        { $$ = NULL; 
          semantic_error("ERROR:  auto keyword not supported!\n"); }
    | REGISTER
        {$$ = NULL; 
         semantic_error("ERROR:  register keyword not supported!\n"); }
    | STATIC
        { $$ = NULL; 
         semantic_error("ERROR:  static keyword not supported!\n"); }
    | EXTERN
        { $$ = NULL; 
         semantic_error("ERROR:  extern keyword not supported!\n"); }
    | MUTABLE
        { $$ = NULL; 
          semantic_error("ERROR:  mutable keyword not supported!\n");}
    ;

function_specifier:
    INLINE
        { $$ = NULL; 
          semantic_error("ERROR: inline keyword not supported! \n"); }
    | VIRTUAL
        { $$ = NULL; 
          semantic_error("ERROR:  virtual keyword not supported!\n"); }
    | EXPLICIT
        { $$ = NULL; 
          semantic_error("ERROR: explicit keyword not supported!\n"); }
    ;

type_specifier:
    simple_type_specifier
        {  $$ = $1; }
    | class_specifier
        {  $$ = $1; }
    | enum_specifier
        { $$ = NULL; 
          semantic_error("ERROR:  enum not supported!\n"); }
    | elaborated_type_specifier
        {  $$ = $1; }
    | cv_qualifier
        {  $$ = $1; }
    ;

simple_type_specifier:
      type_name
        {  $$ = $1; }
    | nested_name_specifier type_name
        {  $$ = create_node2(DECLARATIONS_SIMPLETP2, $1, $2, 0); }
    | COLONCOLON nested_name_specifier_opt type_name
        {  $$ = NULL; 
           semantic_error("ERROR:  this use of :: not supported!\n"); }
    | CHAR
        { $$ = create_leaf(DECLARATIONS_SIMPLETP4, $1); }
    | WCHAR_T
        { $$ = NULL; 
          semantic_error("ERROR:  wchar_t keyword not supported!\n"); }
    | BOOL
        { $$ = create_leaf(DECLARATIONS_SIMPLETP6, $1); }
    | SHORT
        { $$ = create_leaf(DECLARATIONS_SIMPLETP7, $1); }
    | INT
        { $$ = create_leaf(DECLARATIONS_SIMPLETP8, $1); }
    | LONG
        { $$ = create_leaf(DECLARATIONS_SIMPLETP9, $1); }
    | SIGNED
        { $$ = NULL; 
           semantic_error("ERROR: signed keyword not supported!\n"); }
    | UNSIGNED
        { $$ = NULL; 
           semantic_error("ERROR: unsigned keyword not supported!\n"); }
    | FLOAT
        { $$ = create_leaf(DECLARATIONS_SIMPLETP12, $1); }
    | DOUBLE
        { $$ = create_leaf(DECLARATIONS_SIMPLETP13, $1); }
    | VOID
        { $$ = create_leaf(DECLARATIONS_SIMPLETP14, $1); }
    ;

type_name:
    class_name
        {  $$ = $1; }
    | enum_name
        { $$ = NULL; 
          semantic_error("ERROR:  enum not supported!\n"); 
        }
    | typedef_name
        {  $$ = NULL; 
          semantic_error("ERROR:  typedef not supported!\n"); }
    ;

elaborated_type_specifier:
      class_key COLONCOLON nested_name_specifier identifier
        {  $$ = NULL; 
           semantic_error("ERROR:  this use of :: not supported!\n"); }
    | class_key COLONCOLON identifier
        {  $$ = create_node2(DECLARATIONS_ETYPE2, $1, $3, 0); }
    | ENUM COLONCOLON nested_name_specifier identifier
        { $$ = NULL; 
          semantic_error("ERROR:  enum not supported!\n"); }
    | ENUM COLONCOLON identifier
        { $$ = NULL; 
          semantic_error("ERROR:  enum not supported!\n"); }
    | ENUM nested_name_specifier identifier
        { $$ = NULL; 
          semantic_error("ERROR:  enum not supported!\n"); }
    | TYPENAME COLONCOLON_opt nested_name_specifier identifier
        {  $$ = NULL; 
           semantic_error("ERROR:  this use of :: not supported!\n"); }
    | TYPENAME COLONCOLON_opt nested_name_specifier 
        identifier LT template_argument_list GT
        { $$ = NULL; 
          semantic_error("ERROR:  this use of :: not supported!\n"); }
    ;

enum_specifier:
    ENUM identifier LC enumerator_list_opt RC
        { $$ = NULL; 
          semantic_error("ERROR:  enum not supported!\n"); }
    ;

enumerator_list:
    enumerator_definition
        { $$ = NULL; 
          semantic_error("ERROR:  enum not supported!\n"); }
    | enumerator_list COMMA enumerator_definition
        { $$ = NULL; 
          semantic_error("ERROR:  enum not supported!\n"); }
    ;

enumerator_definition:
    enumerator
        { $$ = NULL; 
          semantic_error("ERROR:  enum not supported!\n"); }
    | enumerator ASN constant_expression
        { $$ = NULL; 
          semantic_error("ERROR:  enum not supported!\n"); }
    ;

enumerator:
    identifier
        { $$ = NULL; 
          semantic_error("ERROR:  enum not supported!\n"); }
    ;

namespace_definition:
    named_namespace_definition
        {  $$ = $1; }
    | unnamed_namespace_definition
        {  $$ = $1; }
    ;

named_namespace_definition:
    original_namespace_definition
        {  $$ = $1; }
    | extension_namespace_definition
        {  $$ = $1; }
    ;

original_namespace_definition:
    NAMESPACE identifier LC namespace_body RC
        {  $$ = NULL; 
           semantic_error("ERROR:  this use of namespace not supported!\n"); }
    ;

extension_namespace_definition:
    NAMESPACE original_namespace_name LC namespace_body RC
        {  $$ = NULL; 
           semantic_error("ERROR:  this use of namespace not supported!\n"); }
    ;

unnamed_namespace_definition:
    NAMESPACE LC namespace_body RC
        {  $$ = NULL; 
           semantic_error("ERROR:  this use of namespace not supported!\n"); }
    ;

namespace_body:
    declaration_seq_opt
        {  $$ = $1; }
    ;

namespace_alias_definition:
    NAMESPACE identifier ASN qualified_namespace_specifier SM
        {  $$ = NULL; 
           semantic_error("ERROR:  this use of namespace not supported!\n"); }
    ;

qualified_namespace_specifier:
      COLONCOLON nested_name_specifier namespace_name
        {  $$ = NULL; 
           semantic_error("ERROR:  this use of :: not supported!\n"); }
    | COLONCOLON namespace_name
        {  $$ = NULL; 
           semantic_error("ERROR:  this use of :: not supported!\n"); }
    | nested_name_specifier namespace_name
        {  $$ = NULL; 
           semantic_error("ERROR:  this use of namespace not supported!\n"); }
    | namespace_name
        {  $$ = $1; }
    ;

using_declaration:
      USING TYPENAME COLONCOLON nested_name_specifier unqualified_id SM
        {  $$ = NULL; 
           semantic_error("ERROR:  this use of :: not supported!\n"); }
    | USING TYPENAME nested_name_specifier unqualified_id SM
        {  $$ = NULL; 
           semantic_error("ERROR:  this use of using keyword not supported!\n"); }
    | USING COLONCOLON nested_name_specifier unqualified_id SM
        {  $$ = NULL; 
           semantic_error("ERROR:  this use of :: not supported!\n"); }
    | USING nested_name_specifier unqualified_id SM
        {  $$ = NULL; 
           semantic_error("ERROR:  this use of using keyword not supported!\n"); }
    | USING COLONCOLON unqualified_id SM
        {  $$ = NULL; 
           semantic_error("ERROR:  this use of :: not supported!\n"); }
    ;

using_directive:
    USING NAMESPACE COLONCOLON nested_name_specifier namespace_name SM
        { $$ = NULL; 
          semantic_error("ERROR:  this use of :: not supported!\n");}
    | USING NAMESPACE COLONCOLON namespace_name SM
        { $$ = NULL; 
          semantic_error("ERROR:  this use of :: not supported!\n"); }
    | USING NAMESPACE nested_name_specifier namespace_name SM
        { $$ = NULL; 
          semantic_error("ERROR:  this use of namespace not supported!\n"); }
    | USING NAMESPACE namespace_name SM
        {  $$ = $3; 
           namespace_found = 1;}
    ;

asm_definition:
    ASM LP string_literal RP SM
        {  $$ = NULL; 
           semantic_error("ERROR:  asm keyword not supported!\n");
        }
    ;

linkage_specification:
    EXTERN string_literal LC declaration_seq_opt RC
        {  $$ = NULL; 
           semantic_error("ERROR: this use of extern keyword not supported!\n"); }
    | EXTERN string_literal declaration
        {  $$ = NULL; 
           semantic_error("ERROR: this use of extern keyword not supported!\n"); }
    ;

/*----------------------------------------------------------------------
 * Declarators.
 *----------------------------------------------------------------------*/

init_declarator_list:
    init_declarator
        {  $$ = $1; }
    | init_declarator_list COMMA init_declarator
        {  $$ = create_node2(DECLARATORS_INITLIST2, $1, $3, 0); }
    ;

init_declarator:
    declarator initializer_opt
        {  $$ = create_node2(DECLARATORS_INIT, $1, $2, 0);; }
    ;

declarator:
    direct_declarator
        {  $$ = $1; }
    | ptr_operator declarator
        {  $$ = create_node2(DECLARATORS_DECL2, $1, $2, 0); }
    ;

direct_declarator:
      declarator_id
        {  $$ = $1; }
    | direct_declarator LP parameter_declaration_clause RP 
      cv_qualifier_seq exception_specification
        {  $$ = NULL; 
          semantic_error("ERROR:  exception catching not supported!\n"); }
    | direct_declarator LP parameter_declaration_clause RP cv_qualifier_seq
        {  $$ = NULL; 
          semantic_error("ERROR: this use of const not supported!\n"); }
    | direct_declarator LP parameter_declaration_clause RP 
      exception_specification
    {   $$ = NULL; 
          semantic_error("ERROR:  exception catching not supported!\n"); }
    | direct_declarator LP parameter_declaration_clause RP
        { $$ = create_node2(DECLARATORS_DIRDEC5, $1, $3, 0);  }
    | direct_declarator LB constant_expression_opt RB
        {  $$ = create_node2(DECLARATORS_DIRDEC6, $1, $3, 0); }
    | LP declarator RP
        {  $$ = $2; }
    | CLASS_NAME LP parameter_declaration_clause RP
        {  $$ = $3; }
    | CLASS_NAME COLONCOLON declarator_id LP parameter_declaration_clause RP
        {  /* $$ = create_node2(DECLARATORS_DIRDEC9, $3, $5, 0); */
            if($3 == NULL)
            {
                $$ = create_leaf1node(DECLARATORS_DIRDEC9, $1, $5);  
            }
            if($5 == NULL)
            {
                $$ = create_leaf1node(DECLARATORS_DIRDEC9, $1, $3);  
            }
            if($3 != NULL && $5 != NULL)
            {
                $$ = create_leaf2node(DECLARATORS_DIRDEC9, $1, $3, $5);  
            }
        }
    | CLASS_NAME COLONCOLON CLASS_NAME LP parameter_declaration_clause RP
        { $$ = $5; }
    | CLASS_NAME COLONCOLON CLASS_NAME LP RP
        { $$ = create_leaf2(DECLARATORS_DIRDEC2, $1, $3); }
    ;

ptr_operator:
    STAR
        { $$ = create_leaf(DECLARATORS_PTR, $1); }
    | STAR cv_qualifier_seq
        { $$ = NULL; 
          semantic_error("ERROR: const keyword for pointers not supported!\n"); }
    | AND
        { $$ = create_leaf(DECLARATORS_PTR3, $1); }
    | nested_name_specifier STAR
        { $$ = NULL; 
          semantic_error("ERROR:  this use of :: not supported!\n"); }
    | nested_name_specifier STAR cv_qualifier_seq
        { $$ = NULL; 
          semantic_error("ERROR:  this use of :: not supported!\n"); }
    | COLONCOLON nested_name_specifier STAR
        { $$ = NULL; 
          semantic_error("ERROR:  this use of :: not supported!\n"); }
    | COLONCOLON nested_name_specifier STAR cv_qualifier_seq
        { $$ = NULL; 
          semantic_error("ERROR:  this use of :: not supported!\n"); }
    ;

cv_qualifier_seq:
    cv_qualifier
        { $$ = $1; }
    | cv_qualifier cv_qualifier_seq
        { $$ = NULL; 
          semantic_error("ERROR:  this use of const not supported!\n"); }
    ;

cv_qualifier:
    CONST
        { $$ = create_leaf(DECLARATORS_CV, $1); }
    | VOLATILE
        { $$ = NULL; 
          semantic_error("ERROR:  this use of volatile not supported!\n"); }
    ;

declarator_id:
      id_expression
        { $$ = $1; }
    | COLONCOLON id_expression
        { $$ = NULL; 
          semantic_error("ERROR:  this use of :: not supported!\n"); }
    | COLONCOLON nested_name_specifier type_name
        { $$ = NULL; 
          semantic_error("ERROR:  this use of :: not supported!\n"); }
    | COLONCOLON type_name
        { $$ = NULL; 
          semantic_error("ERROR:  this use of :: not supported!\n"); }
    ;

type_id:
    type_specifier_seq abstract_declarator_opt
        { $$ = create_node2(DECLARATORS_TYPE, $1, $2, 0); }
    ;

type_specifier_seq:
    type_specifier type_specifier_seq_opt
        { $$ = create_node2(DECLARATORS_TYPESEQ, $1, $2, 0); }
    ;

abstract_declarator:
    ptr_operator abstract_declarator_opt
        { $$ = create_node2(DECLARATORS_ABST, $1, $2, 0); }
    | direct_abstract_declarator
        { $$ = $1; }
    ;

direct_abstract_declarator:
      direct_abstract_declarator_opt LP parameter_declaration_clause RP 
      cv_qualifier_seq exception_specification
        { $$ = NULL; 
          semantic_error("ERROR:  this use of exception cathing not supported!\n"); }
    | direct_abstract_declarator_opt LP parameter_declaration_clause RP 
      cv_qualifier_seq
        { $$ = NULL; 
          semantic_error("ERROR:  this use of const not supported!\n"); }
    | direct_abstract_declarator_opt LP parameter_declaration_clause RP 
      exception_specification
        { $$ = NULL; 
          semantic_error("ERROR:  this use of exception cathing not supported!\n"); }
    | direct_abstract_declarator_opt LP parameter_declaration_clause RP
        { $$ = create_node2(DECLARATORS_DIRAB4, $1, $3, 0); }
    | direct_abstract_declarator_opt LB constant_expression_opt RB
        { $$ = create_node2(DECLARATORS_DIRAB5, $1, $3, 0); }
    | LP abstract_declarator RP
        { $$ = $2; }
    ;

parameter_declaration_clause:
      parameter_declaration_list ELLIPSIS
        { $$ = NULL; 
          semantic_error("ERROR:  this use of ... not supported!\n"); }
    | parameter_declaration_list
        { $$ = $1; }
    | ELLIPSIS
        { $$ = NULL; 
          semantic_error("ERROR:  this use of ... not supported!\n"); }
    |   {  $$ = NULL; }
    | parameter_declaration_list COMMA ELLIPSIS
        { $$ = NULL; 
          semantic_error("ERROR:  this use of ... not supported!\n"); }
    ;

parameter_declaration_list:
    parameter_declaration
        { $$ = $1; }
    | parameter_declaration_list COMMA parameter_declaration
        { $$ = create_node2(DECLARATORS_PARAMLS2, $1, $3, 0); }
    ;

parameter_declaration:
    decl_specifier_seq declarator
        { $$ = create_node2(DECLARATORS_PARAMDC, $1, $2, 0); }
    | decl_specifier_seq declarator ASN assignment_expression
        { $$ = NULL; 
          semantic_error("ERROR:  assignment not supported in parameter list!\n"); }
    | decl_specifier_seq abstract_declarator_opt
        { $$ = create_node2(DECLARATORS_PARAMDC3, $1, $2, 0); }
    | decl_specifier_seq abstract_declarator_opt ASN assignment_expression
        { $$ = NULL; 
          semantic_error("ERROR:  assignment not supported in parameter list!\n"); }
    ;

function_definition:
      declarator ctor_initializer_opt function_body
        { $$ = create_node3(DECLARATORS_FUN, $1, $2, $3); }
    | decl_specifier_seq declarator function_body
        {$$ = create_node3(DECLARATORS_FUN2, $1, $2, $3); }
    | decl_specifier_seq declarator ctor_initializer function_body
        { $$ = create_node4(DECLARATORS_FUN3, $1, $2, $3, $4); }
    | declarator function_try_block
        { $$ = NULL; 
          semantic_error("ERROR:  this use of try not supported!\n"); }
    | decl_specifier_seq declarator function_try_block
        { $$ = NULL; 
          semantic_error("ERROR:  this use of try not supported!\n"); }
    ;

function_body:
    compound_statement
        { $$ = $1; }
    ;

initializer:
    ASN initializer_clause
        { $$ = $2; }
    | LP expression_list RP
        { $$ = $2; }
    ;

initializer_clause:
    assignment_expression
        { $$ = $1; }
    | LC initializer_list COMMA_opt RC
        { $$ = create_node2(DECLARATORS_INIZC2, $2, $3, 0); }
    | LC RC
        { $$ = NULL; }
    ;

initializer_list:
    initializer_clause
        { $$ = $1; }
    | initializer_list COMMA initializer_clause
        { $$ = create_node2(DECLARATORS_INIZL, $1, $3, 0); }
    ;

/*----------------------------------------------------------------------
 * Classes.
 *----------------------------------------------------------------------*/

class_specifier:
    class_head LC member_specification_opt RC
        { $$ = create_node2(CLASSES_SPEC, $1, $3, 0); 
          if(ident_cat != 0) 
          {
              insertToHash(ident_tosave, ident_cat);
              free(ident_tosave);
              ident_cat = 0;
          }
        }
    ;

class_head:
      class_key identifier
        { $$ = create_node2(CLASSES_HEAD, $1, $2, 0);
          if(!(ident_tosave = strdup($2->leaf->text)))
          {// !!!
              $$ = NULL;
              exit_num = 10;
              yyerror("ERROR: out of memory!\n");
          }
        }
    | class_key identifier base_clause
        { $$ = create_node2(CLASSES_HEAD2, $1, $2, 0); }
    | class_key nested_name_specifier identifier
        { $$ = create_node3(CLASSES_HEAD3, $1, $2, $3); }
    | class_key nested_name_specifier identifier base_clause
        { $$ = create_node4(CLASSES_HEAD4, $1, $2, $3, $4); }
    ;

class_key:
    CLASS
        { $$ = create_leaf(CLASSES_KEY, $1);
          ident_cat = CLASS_NAME; 
        }
    | STRUCT
        { $$ = NULL; 
          semantic_error("ERROR:  struct keyword not supported!\n"); 
        }
    | UNION
        { $$ = NULL; 
          semantic_error("ERROR:  this use of union not supported!\n"); }
    ;

member_specification:
    member_declaration member_specification_opt
    { 
      $$ = create_node2(CLASSES_MEMSPEC, $1, $2, 0); 
    }
    | access_specifier COLON member_specification_opt
        { $$ = create_node2(CLASSES_MEMSPEC2, $1, $3, 0); }
    ;

member_declaration:
      decl_specifier_seq member_declarator_list SM
        { $$ = create_node2(CLASSES_MEMDEC, $1, $2, 0); }
    | decl_specifier_seq SM
        { $$ = $1; }
    | member_declarator_list SM
        { $$ = $1; }
    | SM
        { $$ = NULL; }
    | function_definition SEMICOLON_opt
        { $$ = create_node2(CLASSES_MEMDEC5, $1, $2, 0); }
    | qualified_id SM
        { $$ = $1; }
    | using_declaration
        { $$ = $1; }
    | template_declaration
        { $$ = NULL; 
          semantic_error("ERROR:  this use of template not supported!\n"); }
    ;

member_declarator_list:
    member_declarator
        { $$ = $1; }
    | member_declarator_list COMMA member_declarator
        { $$ = create_node2(CLASSES_MEMLS2, $1, $3, 0); }
    ;

member_declarator:
        { $$ = NULL; }
    | declarator
        { $$ = $1; }
    | declarator pure_specifier
        { $$ = create_node2(CLASSES_MEMDC, $1, $2, 0); }
    | declarator constant_initializer
        { $$ = create_node2(CLASSES_MEMDC2, $1, $2, 0); }
    | identifier COLON constant_expression
        { $$ = create_node2(CLASSES_MEMDC3, $1, $3, 0); }
    ;

/*
 * This rule need a hack for working around the ``= 0'' pure specifier.
 * 0 is returned as an ``INTEGER'' by the lexical analyzer but in this
 * context is different.
 */
pure_specifier:
    ASN '0'
        { $$ = NULL; }
    ;

constant_initializer:
    ASN constant_expression
        { $$ = $2; }
    ;

/*----------------------------------------------------------------------
 * Derived classes.
 *----------------------------------------------------------------------*/

base_clause:
    COLON base_specifier_list
        { $$ = $2; }
    ;

base_specifier_list:
    base_specifier
        { $$ = $1; }
    | base_specifier_list COMMA base_specifier
        { $$ = create_node2(DCLASSES_BASESPL2, $1, $3, 0); }
    ;

base_specifier:
      COLONCOLON nested_name_specifier class_name
        { $$ = NULL; 
          semantic_error("ERROR:  this use of :: not supported!\n"); }
    | COLONCOLON class_name
        { $$ = NULL; 
          semantic_error("ERROR:  this use of :: not supported!\n"); }
    | nested_name_specifier class_name
        { $$ = create_node2(DCLASSES_BASESP3, $1, $2, 0); }
    | class_name
        { $$ = $1; }
    | VIRTUAL access_specifier COLONCOLON nested_name_specifier_opt class_name
        { $$ = NULL; 
          semantic_error("ERROR:  this use of virtual not supported!\n"); }
    | VIRTUAL access_specifier nested_name_specifier_opt class_name
        { $$ = NULL; 
          semantic_error("ERROR:  this use of virtual not supported!\n"); }
    | VIRTUAL COLONCOLON nested_name_specifier_opt class_name
        { $$ = NULL; 
          semantic_error("ERROR:  this use of virtual not supported!\n"); }
    | VIRTUAL nested_name_specifier_opt class_name
        { $$ = NULL; 
          semantic_error("ERROR:  this use of virtual not supported!\n"); }
    | access_specifier VIRTUAL COLONCOLON nested_name_specifier_opt class_name
        { $$ = NULL; 
          semantic_error("ERROR:  this use of virtual not supported!\n"); }
    | access_specifier VIRTUAL nested_name_specifier_opt class_name
        { $$ = NULL; 
          semantic_error("ERROR:  this use of virtual not supported!\n"); }
    | access_specifier COLONCOLON nested_name_specifier_opt class_name
        { $$ = create_node3(DCLASSES_BASESP5, $1, $3, $4); }
    | access_specifier nested_name_specifier_opt class_name
        { $$ = create_node3(DCLASSES_BASESP6, $1, $2, $3); }
    ;

access_specifier:
    PRIVATE
        { $$ = create_leaf(DCLASSES_ACCESS, $1); }
    | PROTECTED
        { $$ = NULL; 
          semantic_error("ERROR:  this use of protected not supported!\n"); }
    | PUBLIC
        { $$ = create_leaf(DCLASSES_ACCESS2, $1); }
    ;

/*----------------------------------------------------------------------
 * Special member functions.
 *----------------------------------------------------------------------*/

conversion_function_id:
    OPERATOR conversion_type_id
        { $$ = create_node1(SPCMEM_CONVFUN, $2); }
    ;

conversion_type_id:
    type_specifier_seq conversion_declarator_opt
        { $$ = create_node2(SPCMEM_CONVID, $1, $2, 0); }
    ;

conversion_declarator:
    ptr_operator conversion_declarator_opt
        { $$ = create_node2(SPCMEM_CONVDC, $1, $2, 0); }
    ;

ctor_initializer:
    COLON mem_initializer_list
        { $$ = $2; }
    ;

mem_initializer_list:
    mem_initializer
        { $$ = $1; }
    | mem_initializer COMMA mem_initializer_list
        { $$ = create_node2(SPCMEM_MEMLIST2, $1, $3, 0); }
    ;

mem_initializer:
    mem_initializer_id LP expression_list_opt RP
        { $$ = create_node2(SPCMEM_MEMINIT, $1, $3, 0); }
    ;

mem_initializer_id:
      COLONCOLON nested_name_specifier class_name
        { $$ = NULL; 
          semantic_error("ERROR:  this use of :: not supported!\n"); }
    | COLONCOLON class_name
        { $$ = NULL; 
          semantic_error("ERROR:  this use of :: not supported!\n"); }
    | nested_name_specifier class_name
        { $$ = create_node2(SPCMEM_MEMID3, $1, $2, 0); }
    | class_name
        { $$ = $1; }
    | identifier
        { $$ = $1; }
    ;

/*----------------------------------------------------------------------
 * Overloading.
 *----------------------------------------------------------------------*/

operator_function_id:
    OPERATOR operator
        { $$ = create_node1(OVERLOAD_FUNID, $2); }
    ;

operator:
    NEW
        { $$ = create_leaf(OVERLOAD_OP, $1); }
    | DELETE
        { $$ = create_leaf(OVERLOAD_OP2, $1); }
    | NEW LB RB
        { $$ = NULL; 
          semantic_error("ERROR:  this use of new not supported!\n"); }
    | DELETE LB RB
        { $$ = NULL; 
          semantic_error("ERROR:  this use of delete not supported!\n"); }
    | PLUS
        { $$ = create_leaf(OVERLOAD_OP3, $1); }
    | STAR
        { $$ = create_leaf(OVERLOAD_OP4, $1); }
    | DIV
        { $$ = create_leaf(OVERLOAD_OP5, $1); }
    | MOD
        { $$ = create_leaf(OVERLOAD_OP6, $1); }
    | ER
        { $$ = create_leaf(OVERLOAD_OP7, $1); }
    | AND
        { $$ = create_leaf(OVERLOAD_OP8, $1); }
    | OR
        { $$ = create_leaf(OVERLOAD_OP9, $1); }
    | NOT
        { $$ = create_leaf(OVERLOAD_OP10, $1); }
    | BANG
        { $$ = create_leaf(OVERLOAD_OP11, $1); }
    | ASN
        { $$ = create_leaf(OVERLOAD_OP12, $1); }
    | LT
        { $$ = create_leaf(OVERLOAD_OP13, $1); }
    | GT
        { $$ = create_leaf(OVERLOAD_OP14, $1); }
    | ADDEQ
        { $$ = create_leaf(OVERLOAD_OP15, $1); }
    | SUBEQ
        { $$ = create_leaf(OVERLOAD_OP16, $1); }
    | MULEQ
        { $$ = create_leaf(OVERLOAD_OP17, $1); }
    | DIVEQ
        { $$ = create_leaf(OVERLOAD_OP18, $1); }
    | MODEQ
        { $$ = create_leaf(OVERLOAD_OP19, $1); }
    | XOREQ
        { $$ = NULL; 
          semantic_error("ERROR: this use of bitwise xor not supported!\n"); }
    | ANDEQ
        { $$ = NULL; 
          semantic_error("ERROR: this use of bitwise and not supported!\n"); }
    | OREQ
        { $$ = NULL; 
          semantic_error("ERROR: this use of bitwise or not supported!\n"); }
    | SL
        { $$ = create_leaf(OVERLOAD_OP23, $1); }
    | SR
        { $$ = create_leaf(OVERLOAD_OP24, $1); }
    | SREQ
        { $$ = create_leaf(OVERLOAD_OP25, $1); }
    | SLEQ
        { $$ = create_leaf(OVERLOAD_OP26, $1); }
    | EQ
        { $$ = create_leaf(OVERLOAD_OP27, $1); }
    | NOTEQ
        { $$ = create_leaf(OVERLOAD_OP28, $1); }
    | LTEQ
        { $$ = create_leaf(OVERLOAD_OP29, $1); }
    | GTEQ
        { $$ = create_leaf(OVERLOAD_OP30, $1); }
    | ANDAND
        { $$ = create_leaf(OVERLOAD_OP31, $1); }
    | OROR
        { $$ = create_leaf(OVERLOAD_OP32, $1); }
    | PLUSPLUS
        { $$ = create_leaf(OVERLOAD_OP33, $1); }
    | MINUSMINUS
        { $$ = create_leaf(OVERLOAD_OP34, $1); }
    | COMMA
        { $$ = create_leaf(OVERLOAD_OP35, $1); }
    | ARROWSTAR
        { $$ = create_leaf(OVERLOAD_OP36, $1); }
    | ARROW
        { $$ = create_leaf(OVERLOAD_OP37, $1); }
    | LP RP
        { $$ = NULL; }
    | LB RB
        { $$ = NULL; }
    ;

/*----------------------------------------------------------------------
 * Templates.
 *----------------------------------------------------------------------*/

template_declaration:
    EXPORT_opt TEMPLATE LT template_parameter_list GT declaration
        { $$ = NULL; 
          semantic_error("ERROR:  this use of template not supported!\n"); }
    ;

template_parameter_list:
    template_parameter
        { $$ = NULL; 
          semantic_error("ERROR:  this use of template not supported!\n"); }
    | template_parameter_list COMMA template_parameter
         { $$ = NULL; 
           semantic_error("ERROR:  this use of template not supported!\n"); }
   ;

template_parameter:
    type_parameter
        { $$ = NULL; 
          semantic_error("ERROR:  this use of template not supported!\n"); }
    | parameter_declaration
        { $$ = NULL; 
          semantic_error("ERROR:  this use of template not supported!\n"); }
    ;

type_parameter:
      CLASS identifier
        { $$ = NULL; 
          semantic_error("ERROR:  this use of template not supported!\n"); }
    | CLASS identifier ASN type_id
        { $$ = NULL; 
          semantic_error("ERROR:  this use of template not supported!\n"); }
    | TYPENAME identifier
        { $$ = NULL; 
          semantic_error("ERROR:  this use of template not supported!\n"); }
    | TYPENAME identifier ASN type_id
        { $$ = NULL; 
          semantic_error("ERROR:  this use of template not supported!\n"); }
    | TEMPLATE LT template_parameter_list GT CLASS identifier
        { $$ = NULL; 
          semantic_error("ERROR:  this use of template not supported!\n"); }
    | TEMPLATE LT template_parameter_list GT CLASS identifier 
      ASN template_name
        { $$ = NULL; 
          semantic_error("ERROR:  this use of template not supported!\n"); }
    ;

template_id:
    template_name LT template_argument_list GT
        { $$ = NULL; 
          semantic_error("ERROR:  this use of template not supported!\n"); }
    ;

template_argument_list:
    template_argument
        { $$ = NULL; 
          semantic_error("ERROR:  this use of template not supported!\n"); }
    | template_argument_list COMMA template_argument
        { $$ = NULL; 
          semantic_error("ERROR:  this use of template not supported!\n"); }
    ;

template_argument:
    assignment_expression
        { $$ = NULL; 
          semantic_error("ERROR:  this use of template not supported!\n"); }
    | type_id
        { $$ = NULL; 
          semantic_error("ERROR:  this use of template not supported!\n"); }
    | template_name
        { $$ = NULL; 
          semantic_error("ERROR:  this use of template not supported!\n"); }
    ;

explicit_instantiation:
    TEMPLATE declaration
        { $$ = NULL; 
          semantic_error("ERROR:  this use of template not supported!\n"); }
    ;

explicit_specialization:
    TEMPLATE LT GT declaration
        { $$ = NULL; 
          semantic_error("ERROR:  this use of template not supported!\n"); }
    ;

/*----------------------------------------------------------------------
 * Exception handling.
 *----------------------------------------------------------------------*/

try_block:
    TRY compound_statement handler_seq
        { $$ = NULL; 
          semantic_error("ERROR:  this use of try not supported!\n"); }
    ;

function_try_block:
    TRY ctor_initializer_opt function_body handler_seq
        { $$ = NULL; 
          semantic_error("ERROR:  this use of try not supported!\n"); }
    ;

handler_seq:
    handler handler_seq_opt
        { $$ = NULL; 
          semantic_error("ERROR:  this use of catch not supported!\n");  }
    ;

handler:
    CATCH LP exception_declaration RP compound_statement
        { $$ = NULL; 
          semantic_error("ERROR:  this use of catch not supported!\n"); }
    ;

exception_declaration:
    type_specifier_seq declarator
        { $$ = create_node2(EXCEPT_DECL, $1, $2, 0); }
    | type_specifier_seq abstract_declarator
        { $$ = create_node2(EXCEPT_DECL2, $1, $2, 0); }
    | type_specifier_seq
        { $$ = $1; }
    | ELLIPSIS
        { $$ = NULL; 
          semantic_error("ERROR:  this use of ... not supported!\n"); }
    ;

throw_expression:
    THROW assignment_expression_opt
        { $$ = NULL; 
          semantic_error("ERROR:  this use of throw not supported!\n"); }
    ;

exception_specification:
    THROW LP type_id_list_opt RP
        { $$ = NULL; 
          semantic_error("ERROR:  this use of throw not supported!\n"); }
    ;

type_id_list:
    type_id
        { $$ = $1; }
    | type_id_list COMMA type_id
        { $$ = create_node2(EXCEPT_TYPEID2, $1, $3, 0); }
    ;

/*----------------------------------------------------------------------
 * Epsilon (optional) definitions.
 *----------------------------------------------------------------------*/

declaration_seq_opt:
    /* epsilon */
        { $$ = NULL; }
    | declaration_seq
        { $$ = $1; }
    ;

nested_name_specifier_opt:
    /* epsilon */
        { $$ = NULL; }
    | nested_name_specifier
        { $$ = $1; }
    ;

expression_list_opt:
    /* epsilon */
        { $$ = NULL; }
    | expression_list
        { $$ = $1; }
    ;

COLONCOLON_opt:
    /* epsilon */
        { $$ = NULL; }
    | COLONCOLON
        { $$ = NULL; }
    ;

new_placement_opt:
    /* epsilon */
        { $$ = NULL; }
    | new_placement
        { $$ = $1; }
    ;

new_initializer_opt:
    /* epsilon */
        { $$ = NULL; }
    | new_initializer
        { $$ = $1; }
    ;

new_declarator_opt:
    /* epsilon */
        { $$ = NULL; }
    | new_declarator
        { $$ = $1; }
    ;

expression_opt:
    /* epsilon */
        { $$ = NULL; }
    | expression
        { $$ = create_node1(STATEMENTS_EX, $1); }
    ;

statement_seq_opt:
    /* epsilon */
        { $$ = NULL; }
    | statement_seq
        { $$ = $1; }
    ;

condition_opt:
    /* epsilon */
        { $$ = NULL; }
    | condition
        { $$ = $1; }
    ;

enumerator_list_opt:
    /* epsilon */
        { $$ = NULL; }
    | enumerator_list
        { $$ = NULL; 
          semantic_error("ERROR:  this use of enum not supported!\n"); }
    ;

initializer_opt:
    /* epsilon */
        { $$ = NULL; }
    | initializer
        { $$ = $1; }
    ;

constant_expression_opt:
    /* epsilon */
        { $$ = NULL; }
    | constant_expression
        { $$ = $1; }
    ;

abstract_declarator_opt:
    /* epsilon */
        { $$ = NULL; }
    | abstract_declarator
        { $$ = $1; }
    ;

type_specifier_seq_opt:
    /* epsilon */
        { $$ = NULL; }
    | type_specifier_seq
        { $$ = $1; }
    ;

direct_abstract_declarator_opt:
    /* epsilon */
        { $$ = NULL; }
    | direct_abstract_declarator
        { $$ = $1; }
    ;

ctor_initializer_opt:
    /* epsilon */
        { $$ = NULL; }
    | ctor_initializer
        { $$ = $1; }
    ;

COMMA_opt:
    /* epsilon */
        { $$ = NULL; }
    | COMMA
        { $$ = NULL; }
    ;

member_specification_opt:
    /* epsilon */
        { $$ = NULL; }
    | member_specification
        { $$ = $1; }
    ;

SEMICOLON_opt:
    /* epsilon */
        { $$ = NULL; }
    | SM
        { $$ = NULL; }
    ;

conversion_declarator_opt:
    /* epsilon */
        { $$ = NULL; }
    | conversion_declarator
        { $$ = $1; }
    ;

EXPORT_opt:
    /* epsilon */
        { $$ = NULL; }
    | EXPORT
        { $$ = NULL; 
          semantic_error("ERROR:  this use of export not supported!\n"); }
    ;

handler_seq_opt:
    /* epsilon */
        { $$ = NULL; }
    | handler_seq
        { $$ = $1; }
    ;

assignment_expression_opt:
    /* epsilon */
        { $$ = NULL; }
    | assignment_expression
        { $$ = $1; }
    ;

type_id_list_opt:
    /* epsilon */
        { $$ = NULL;}
    | type_id_list
         { $$ = $1; }
   ;

%%

static void
yyerror(char *s)
{
    fprintf(stderr, "%d: %s\nFile %s\n", lineno, s, curfilename);
    exit_num = 2;
    exit(exit_num);
}

void semantic_error(char *s)
{
  fprintf(stderr, "%s\nLine %d, File %s\n", s, lineno, curfilename);
  exit_num = 3;
  exit(exit_num);
}

parsetree *create_leaf(int rule, struct token *l)
{
    parsetree *ptr = (parsetree *)malloc(sizeof(parsetree));
    if(ptr == NULL)
    {
        fprintf(stderr, "create_leaf out of memory\n");
        exit(10);
    }
    ptr->prodrule = rule;
    ptr->line_num = lineno; 
    ptr->filename = strdup(curfilename); 
    if(ptr->filename == NULL)
    {
        fprintf(stderr, "create_leaf out of memory\n");
        exit(10);    
    }
    ptr->nkids = 0;
    ptr->leaf = l; 
    parse_tree = ptr;
    return ptr;
}

parsetree *create_leaf2(int rule, struct token *l1, struct token *l2)
{ /* want to save name from l2  */
    parsetree *ptr = (parsetree *)malloc(sizeof(parsetree));
    if(ptr == NULL)
    {
        fprintf(stderr, "create_leaf out of memory\n");
        exit(10);
    }
    ptr->prodrule = rule;
    ptr->line_num = lineno; 
    ptr->filename = strdup(curfilename); 
    if(ptr->filename == NULL)
    {
        fprintf(stderr, "create_leaf out of memory\n");
        exit(10);    
    }
    ptr->nkids = 0;
    l2->classname = strdup(l1->text);
    if(l2->classname == NULL)
    {
        fprintf(stderr, "create_leaf out of memory\n");
        exit(10);
    }
    ptr->leaf = l2; 
    parse_tree = ptr;
    return ptr;
}

/* create a node with leaf & one kid */
parsetree *create_leaf1node(int rule, struct token *l, struct tree *t1)
{
    parsetree *ptr = (parsetree *)malloc(sizeof(parsetree) + (1)*sizeof(parsetree));
    if(ptr == NULL)
    {
        fprintf(stderr, "create_leaf out of memory\n");
        exit(10);
    }
    ptr->prodrule = rule;
    ptr->line_num = lineno; 
    ptr->filename = strdup(curfilename); 
    if(ptr->filename == NULL)
    {
        fprintf(stderr, "create_leaf out of memory\n");
        exit(10);    
    }
    ptr->nkids = 1;
    ptr->leaf = l; 
    ptr->kids[0] = t1;
    parse_tree = ptr;
    return ptr;
}

/* create a node with leaf & two kids */
parsetree *create_leaf2node(int rule, struct token *l, struct tree *t1, struct tree *t2)
{
    parsetree *ptr = (parsetree *)malloc(sizeof(parsetree) + (2)*sizeof(parsetree));
    if(ptr == NULL)
    {
        fprintf(stderr, "create_leaf out of memory\n");
        exit(10);
    }
    ptr->prodrule = rule;
    ptr->line_num = lineno; 
    ptr->filename = strdup(curfilename); 
    if(ptr->filename == NULL)
    {
        fprintf(stderr, "create_leaf out of memory\n");
        exit(10);    
    }
    ptr->nkids = 2;
    ptr->leaf = l; 
    ptr->kids[0] = t1;
    ptr->kids[1] = t2;
    parse_tree = ptr;
    return ptr;
}

parsetree *create_node(int rule, int nkids, ...)
{
    /* this function is no longer used... i thot it might be a bug source,
     * but this is not the case */
    /* code adopted from Dr. Jeffery class notes */
    int i, j;
    /* this function is only for treenodes, i.e. leaf is not used  */
    va_list ap;
    parsetree *ptr = (parsetree *)malloc(sizeof(parsetree) 
    + (nkids - 1)*sizeof(parsetree));
    if(ptr == NULL)
    {
        fprintf(stderr, "create_node out of memory\n");
        exit(10);
    }
    ptr->prodrule = rule;
    ptr->line_num = lineno; 
    ptr->filename = strdup(curfilename); 
    if(ptr->filename == NULL)
    {
        fprintf(stderr, "create_node out of memory\n");
        exit(10);    
    }
    ptr->nkids = nkids;
    va_start(ap, nkids);
    j = nkids;
    i = 0;
    while(i < j)
    {
      ptr->kids[i] = va_arg(ap, parsetree *);
      if(ptr->kids[i] == NULL)
      {
        //somehow the kid is null!
        j--;
      }
      else
      {
        //all is well
        i++;
      }
    }
    ptr->nkids = j;
    va_end(ap);
    parse_tree = ptr;
    return ptr;
}

parsetree *create_node1(int rule, struct tree *t)
{
    parsetree *ptr = 0;
    /* this function is only for one treenode, i.e. leaf is not used  */
    /* check that t is not null */
    if(t != NULL)
    {
        ptr = (parsetree *)malloc(sizeof(parsetree));
        if(ptr == NULL)
        {
            fprintf(stderr, "create_node out of memory\n");
            exit(10);
        }
        ptr->prodrule = rule;
        ptr->line_num = lineno; 
        ptr->filename = strdup(curfilename); 
        if(ptr->filename == NULL)
        {
            fprintf(stderr, "create_node out of memory\n");
            exit(10);    
        }
        ptr->nkids = 1;
        ptr->kids[0] = t;
        parse_tree = ptr;
    }
    else
    {
        ptr = parse_tree;
    }
    return ptr;
}

parsetree *checkd_node2(int rule, struct tree *t1, struct tree *t2, int splice)
{
    /* same as create_node2 except we're sure t1 and t2 are not null */
    parsetree *ptr = 0;
    ptr = (parsetree *)malloc(sizeof(parsetree) + (2 - 1)*sizeof(parsetree));
    if(ptr == NULL)
    {
        fprintf(stderr, "create_node out of memory\n");
        exit(10);
    }
    ptr->prodrule = rule;
    ptr->line_num = lineno; 
    ptr->filename = strdup(curfilename); 
    if(ptr->filename == NULL)
    {
        fprintf(stderr, "create_node out of memory\n");
        exit(10);    
    }
    ptr->nkids = 2;
    ptr->kids[0] = t1;
    ptr->kids[1] = t2;
    if(splice == 0)
    {
        parse_tree = ptr;
    }
    return ptr;
}
parsetree *create_node2(int rule, struct tree *t1, struct tree *t2, int splice)
{
    parsetree *ptr = 0;
    int nulls = 0; /* 0 = all good, 1 = t1 null, 2 = t2 null, 1000 = all null */
    /* this function is only for 2 treenodes, i.e. leaf is not used  */
    if(t1 == NULL && t2 != NULL)
    {
        nulls = 1;
    }
    if(t1 != NULL && t2 == NULL)
    {
        nulls = 2;
    }
    if(t1 == NULL && t2 == NULL)
    {
        nulls = 10;
    }
    switch(nulls)
    {
        case 0:
            ptr = checkd_node2(rule, t1, t2, splice);
            break;
        case 1:
            ptr = create_node1(rule, t2);
            break;
        case 2:
            ptr = create_node1(rule, t1);
            break;
        default:
            ptr = parse_tree;
            break;
    }
    return ptr;
}

parsetree *checkd_node3(int rule, struct tree *t1, struct tree *t2, struct tree *t3)
{
    /* same as create_node3 except we're sure t1-3 are not null */
    parsetree *ptr = 0;
    ptr = (parsetree *)malloc(sizeof(parsetree) + (3 - 1)*sizeof(parsetree));
    if(ptr == NULL)
    {
        fprintf(stderr, "create_node out of memory\n");
        exit(10);
    }
    ptr->prodrule = rule;
    ptr->line_num = lineno; 
    ptr->filename = strdup(curfilename); 
    if(ptr->filename == NULL)
    {
        fprintf(stderr, "create_node out of memory\n");
        exit(10);    
    }
    ptr->nkids = 3;
    ptr->kids[0] = t1;
    ptr->kids[1] = t2;
    ptr->kids[2] = t3;
    parse_tree = ptr;
    return ptr;
}

parsetree *create_node3(int rule, struct tree *t1, struct tree *t2, struct tree *t3)
{
    parsetree *ptr = 0;
    struct tree *tptr = 0;
    struct tree *tptr2 = 0;
    int nulls = 0;
    /* this function is only for 3 treenodes, i.e. leaf is not used  */
    /* 0 = all good, 1 = t1 null, 2 = t2 null, 3 = t3 null, 
     * 4 = t1&2 null, 5 = t1&3 null, 6 = t2&3 null, 1000 = all null */
    if(t1 == NULL && t2 != NULL && t3 != NULL)
    {
        nulls = 1;
        tptr = t2;
        tptr2 = t3;
    }
    if(t1 != NULL && t2 == NULL && t3 != NULL)
    {
        nulls = 2;
        tptr = t1;
        tptr2 = t3;
    }
    if(t1 != NULL && t2 != NULL && t3 == NULL)
    {
        nulls = 3;
        tptr = t1;
        tptr2 = t2;
    }
    if(t1 == NULL && t2 == NULL)
    {
        if(t3 == NULL)
        {
            nulls = 1000;
        }
        else
        {
            nulls = 4;
            tptr = t3;
        }
    }
    if(t1 == NULL && t2 != NULL && t3 == NULL)
    {
        nulls = 5;
        tptr = t2;
    }
    if(t1 != NULL && t2 == NULL && t3 == NULL)
    {
        nulls = 6;
        tptr = t1;
    }
    switch(nulls)
    {
        case 0:
            ptr = checkd_node3(rule, t1, t2, t3);
            break;
        case 1:
        case 2:
        case 3:
            ptr = checkd_node2(rule, tptr, tptr2, 0);
            break;
        case 4:
        case 5:
        case 6:
            ptr = create_node1(rule, tptr);
            break; 
        default:
            ptr = parse_tree;
            break;
    }
    return ptr;
}

parsetree *checkd_node4(int rule, struct tree *t1, struct tree *t2, struct tree *t3, struct tree *t4)
{
    /* this function is only for 4 treenodes, i.e. leaf is not used  */
    /* t1-4 have been check are not null */
    parsetree *ptr = (parsetree *)malloc(sizeof(parsetree) + (4 - 1)*sizeof(parsetree));
    if(ptr == NULL)
    {
        fprintf(stderr, "create_node out of memory\n");
        exit(10);
    }
    ptr->prodrule = rule;
    ptr->line_num = lineno; 
    ptr->filename = strdup(curfilename); 
    if(ptr->filename == NULL)
    {
        fprintf(stderr, "create_node out of memory\n");
        exit(10);    
    }
    ptr->nkids = 4;
    ptr->kids[0] = t1;
    ptr->kids[1] = t2;
    ptr->kids[2] = t3;
    ptr->kids[3] = t4;
    parse_tree = ptr;
    return ptr;
}

parsetree *create_node4(int rule, struct tree *t1, struct tree *t2, struct tree *t3, struct tree *t4)
{
    parsetree *ptr = 0;
    struct tree *tptr1 = 0;
    struct tree *tptr2 = 0;
    struct tree *tptr3 = 0; 
    int nulls = 0;
    /* this function is only for 4 treenodes, i.e. leaf is not used  */
    /* 0 = all good, 1 = 1 null, 2 = 2 null, 3 = 3 null, 4 = 4 null 
     * 5 = 12 null, 6 = 13 null, 7 = 14 null, 8 = 23 null, 9 = 24 null, 10 =34 null 
     * 11 = 123 null, 12 = 124 null, 13 = 134 null, 14 = 234 null, 
     * 1000 = all null */
    if(t1 == NULL)
    {
        /* cases 1,5,6,7,11,12,13 */
        if(t2 == NULL)
        {
            /* cases 5,11,12 */
            nulls = 5;
            tptr1 = t3;
            tptr2 = t4;
            if(t3 == NULL && t4 == NULL)
            {
                nulls = 1000;
                tptr1 = 0;
                tptr2 = 0;
            }
            if(t3 != NULL && t4 == NULL)
            {
                nulls = 12;
                tptr2 = 0;
            }
            if(t3 == NULL && t4 != NULL)
            {
                nulls = 11;
                tptr1 = t4;
                tptr2 = 0;
            }
        }
        else
        {
            /* cases 1,6,7,13 */
            
            if(t3 == NULL)
            {
                if(t4 != NULL)
                {
                    nulls = 6;
                    tptr1 = t2;
                    tptr2 = t4;
                }
                else
                {
                    nulls = 13;
                    tptr1 = t2;
                }
            }
            else
            {
                if(t4 == NULL)
                {
                    nulls = 7;
                    tptr1 = t2;
                    tptr2 = t3;
                }
                else
                {
                    nulls = 1;
                    tptr1 = t2;
                    tptr2 = t3;
                    tptr3 = t4;
                }
            }
        }
    }
    else
    {
        if(t2 == NULL)
        {
            /* cases 2,8,9,14 */
            if(t3 == NULL)
            {
                /* cases 8, 14 */
                if(t4 != NULL)
                {
                    nulls = 8;
                    tptr1 = t1;
                    tptr2 = t4;
                }
                else
                {
                    nulls = 14;
                    tptr1 = t1;
                }
            }
            else
            {
                /* cases 2, 9 */
                if(t4 == NULL)
                {
                    nulls = 9;
                    tptr1 = t1;
                    tptr2 = t3;
                }
                else
                {
                    nulls = 2;
                    tptr1 = t1;
                    tptr2 = t3;
                    tptr3 = t4;
                }
            }
        }
        else
        {
            /* cases 3, 4, 10 */
            if(t3 == NULL)
            {
                /* 3, 10 */
                if(t4 == NULL)
                {
                    nulls = 10;
                    tptr1 = t1;
                    tptr2 = t2;
                }
                else
                {
                    nulls = 3;
                    tptr1 = t1;
                    tptr2 = t2;
                    tptr3 = t4;
                
                }
            }
            else
            {
            	if(t4 == NULL)
            	{
                	nulls = 4;
                	tptr1 = t1;
                	tptr2 = t2;
                	tptr3 = t3;
                }
            }
        }
    }
    switch(nulls)
    {
        case 0:
            ptr = checkd_node4(rule, t1, t2, t3, t4);
            break;
        case 1:
        case 2:
        case 3:
        case 4:
            ptr = checkd_node3(rule, tptr1, tptr2, tptr3);
            break;
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
            ptr = checkd_node2(rule, tptr1, tptr2, 0);
            break; 
        case 11:
        case 12:
        case 13:
        case 14:
            ptr = create_node1(rule, tptr1);
        default:
            ptr = parse_tree;
            break;
    }
    return ptr;
}

void printer(parsetree *t, int depth)
{
    /* code adopted from Dr. Jeffery class notes */
    int i;
    enum base_type bt;
    char *pname;
    char *noname = "<none>";
    pname = noname; 
    if(t==NULL)
    {
        return;
    }
    if(t->typ == NULL)
    {
        bt = UNKNOWN_TYPE;
    }
    else
    {
        bt = t->typ->basetype;
        if(t->typ->name != NULL)
        {
          pname = t->typ->name;
        }
    }
    if(t->nkids > 0)
    {
        fprintf(stdout, "%*sproduction rule %d: kids %d: basetype %d: name %s: line %d,\n"
        , depth*2, " ", t->prodrule, t->nkids, bt, pname, t->line_num);
        if(parser_debug == 1)
        {
            fprintf(stdout, "%*s(%s)\n", depth*2, " "
            , readable_rule(t->prodrule));
        }
        for(i = 0; i < t->nkids; i++)
        {
            printer(t->kids[i], depth+1);
        }
    }
    if(t->leaf != NULL)
    {
        fprintf(stdout, "%*sproduction rule %d: token %s: basetype %d: name %s: line %d\n"
        , depth*2, " ", t->prodrule, t->leaf->text, bt, pname, t->line_num);
        if(parser_debug == 1)
        {
            fprintf(stdout, "%*s(%s)\n", depth*2, " "
            , readable_rule(t->prodrule));
        }
    }
}
