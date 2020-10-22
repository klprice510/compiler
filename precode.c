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
#include "precode.h"
#include "rules.h"

extern int semantic_debug;
extern int parser_debug;

void ic_gen(void);
extern parsetree *parse_tree; /* current parse tree being worked on */
extern struct sym_table *symtab; /* global sym tab */
extern struct typeinfo *scroll_search(struct sym_table *st, char *s); /* lookup: retrieve entry from a sym tab */
extern struct typeinfo *make_room(enum base_type basetype);
extern char *copy_name(char *s);
/* getvalue: retrieve value of source language literal */
/* getname: retrieve name of source language variable/operation */
struct typeinfo *newvar(enum base_type bt, struct sym_table *st); /* make new intermediate code variable */
struct tac *newlabel(void); /* newlabel: make new label for jumps */
/* trans_op: translates an operator name to the name in IL */
struct tac *make_tac(void); /* create a new tac struct for an operation */
struct taclist *add_taclist(struct tac *t, struct taclist *tl); /* add a tac onto the end of tac list */
struct taclist *new_listmem(void); /* create a new node for tac list */
struct taclist *copy_list(struct taclist *tl); /* creates a separate copy of the tac list */
struct tac *copy_tac(struct tac *t); /* creates a separate copy of tac */
struct sym_list *mem_loc(struct sym_table *st); /* mark memory and size for everything in sym tabs */
struct taclist *concat(struct taclist *t1, struct taclist *t2); /* non-destructive concatination of lists */
struct taclist *addupcode(parsetree *n); /* adds up the kids code */
struct sym_list *symtablist; /* end of symtablist */
struct sym_list *symtablist_begin; /* start of symtablist */
struct sym_list *new_symlist(struct sym_table *st);
struct sym_list *add_symlist(struct sym_list *sl, struct sym_table *st);
int set_memsize(struct typeinfo *t, struct sym_table *st); /* set memory region/offset and size */
int size_up_class(struct typeinfo *t);
int size_up_arr(struct typeinfo *t);
int size_up_func(struct typeinfo *t, int cp);
int size_up_simple(enum base_type bt);
int align_size(int size);
char *name_mangle(struct typeinfo *t);
extern char *copy_name(char *s);
void fill_const(parsetree *n, struct sym_table *st);
void point_parent(parsetree *kid_node, parsetree *parent_node);
void calc_values(parsetree *n, struct sym_table *st);
extern struct typeinfo *FindLexElemNode(parsetree *n, struct sym_table *st);
extern void sym_insert(struct sym_table *symt, struct typeinfo *t);
void print_code(struct taclist *t);
struct typeinfo *new_const(int ival, float fval, enum base_type bt);
struct tac *fill_in_tac(int operation, struct addr *dest, struct addr *src1, struct addr *src2);
struct addr *NullAddr;
struct tac *find_true(parsetree *n);
struct tac *find_false(parsetree *n);
struct tac *find_follow(parsetree *n);
extern struct typeinfo * search_type(struct sym_table *st, char *s);
extern parsetree *FindNode(parsetree *n, struct sym_table *st, int rule);
extern void semantic_error(char *s);
extern int lineno;
extern char *curfilename;
extern char *build_errstr(char *ename, char *e2);
extern struct addr MallocAddr;
struct typeinfo *find_my_class(struct typeinfo *t);
char *is_classmem(struct typeinfo *t, struct sym_table *st);
struct taclist *check_obj(struct typeinfo *t, struct sym_table *st);
struct tac *getlast(struct taclist *tl); 
struct tac *obj_lists(struct taclist *newsrc, struct addr *altaddr);
struct tac *obj_asn(struct taclist *tl, struct addr *a);
struct tac *gnexttac[100];
struct tac *nextnexttac[100];
int forgcnt = -1;
int forcnt = -1;
struct tac *find_break(parsetree *n);

void ic_gen()
{
    MallocAddr.region = R_CODE;
    fill_const(parse_tree, symtab); /* mark up const regions & values, look for class memebers */
    point_parent(parse_tree, NULL); /* fill in the parent pointers for each node */
    calc_values(parse_tree, symtab); /* calculate code for expressions at each node */
    print_code(parse_tree->code);
}

struct tac *find_follow(parsetree *n)
{ /* go up and try to find an older sibling, loop until found or end */
    parsetree *nodeparent;
    struct tac *followtac = 0;
    struct taclist *tlist;
    int i;
    int sinit=0;
    int cond=0;
    int expr=0;
    int statm=0;
    int j;

    nodeparent = n;
    while(nodeparent != NULL && nodeparent->parent != NULL)
    {
        if(nodeparent->prodrule == DECLARATORS_FUN 
         || nodeparent->prodrule == DECLARATORS_FUN2) /* 5651 5652 */
        { /* End of function... just return */
            break;
        }
        for(i = 0; i < nodeparent->parent->nkids; i++)
        { /* find n kid number */
            if(nodeparent->parent->kids[i] == nodeparent)
            { /* we want this node's older sibling for follow */
                break;
            }
        }   
        if(i < (nodeparent->parent->nkids - 1))
        { /* take the next sibling else continue up the tree */
            i++;
            nodeparent = nodeparent->parent->kids[i];
            followtac = newlabel();
            tlist = new_listmem();
            tlist->instruct = followtac;
            nodeparent->code = concat(nodeparent->code, tlist);
            break;
        }
        if(nodeparent->prodrule == STATEMENTS_ITER3) /* 4573, for */
        {
        	for(j = 0; j < nodeparent->nkids; j++)
        	{
            	if(nodeparent->kids[j]->prodrule >= 4232 && nodeparent->kids[j]->prodrule <= 4243)
            	{ /* conditional found */
	            	cond = j + 1;
            	}	
            	if(nodeparent->kids[j]->prodrule >= 4501 && nodeparent->kids[j]->prodrule <= 4507)
            	{ /* stmt found */
	            	statm = j + 1;
            	}	
            	if(nodeparent->kids[j]->prodrule == 4581)
            	{
	            	sinit = j + 1;
            	}
            	if(nodeparent->kids[j]->prodrule == 4521)
            	{
	            	expr = j+1;
            	}
            	if(expr)
            	{
	            	nodeparent = nodeparent->kids[expr-1];
	            	followtac = newlabel();
	            	tlist = new_listmem();
	            	tlist->instruct = followtac;
	            	nodeparent->code = concat(nodeparent->code, tlist);
            	}
            	else
            	{
	            	if(cond)
	            	{
		            	nodeparent = nodeparent->kids[cond-1];
		            	followtac = newlabel();
		            	tlist = new_listmem();
		            	tlist->instruct = followtac;
		            	nodeparent->code = concat(nodeparent->code, tlist);
	            	}
            	}
        	}
	        break;
        }
        nodeparent = nodeparent->parent;        
    }
    return followtac;
}

struct tac *find_false(parsetree *n)
{
    parsetree *nodeparent;
    struct tac *falsetac = 0;
    struct taclist *tlist;
    int false_found = 0;
    
    nodeparent = n->parent;
    while(nodeparent != NULL)
    {
        switch(nodeparent->prodrule)
        { /* find the first flow control and then find the follow of that */
        	case STATEMENTS_ITER3: /* 4573, for */
            case STATEMENTS_SEL: /* 4551, if */
            case STATEMENTS_ITER: /* 4551, while */
            case STATEMENTS_SEL3: /* 4553, switch */
                falsetac = find_follow(nodeparent);
                false_found++;
                break;
        }
        if(false_found)
        {
            break;
        }
        nodeparent = nodeparent->parent;   
    }
    return falsetac;
}

struct tac *find_break(parsetree *n)
{
    parsetree *nodeparent;
    struct tac *falsetac = 0;
    struct taclist *tlist;
    int false_found = 0;
    
    nodeparent = n->parent;
    while(nodeparent != NULL)
    {
        switch(nodeparent->prodrule)
        { /* find the first switch control and then find the follow of that */
            case STATEMENTS_SEL3: /* 4553, switch */
                falsetac = find_follow(nodeparent);
                false_found++;
                break;
        }
        if(false_found)
        {
            break;
        }
        nodeparent = nodeparent->parent;   
    }
    return falsetac;
}

struct tac *find_true(parsetree *n)
{
    parsetree *nodeparent;
    struct tac *truetac = 0;
    struct tac *mytac;
    struct taclist *tlist;
    struct taclist *t2list;
    struct taclist *t3list;
    int true_found = 0;
    int sinit=0;
    int cond=0;
    int expr=0;
    int statm=0;
    int i;
    parsetree *myparent;
    parsetree *orignode;
    
    nodeparent = n->parent;
    while(nodeparent != NULL)
    {
        switch(nodeparent->prodrule)
        {
            case STATEMENTS_SEL: /* 4551, if */
                /* true should kid[1] if exists */
                if(nodeparent->nkids == 2)
                { /* add label to start of code */
                    nodeparent = nodeparent->kids[1];
                    truetac = newlabel();
                    tlist = new_listmem();
                    tlist->instruct = truetac;
                    /* since this hasn't been processed yet, don't add up kids' code */
                    nodeparent->code = concat(nodeparent->code, tlist);
                    true_found++;
                }
                break;
            case STATEMENTS_ITER3: /* 4573, for */
            	for(i = 0; i < nodeparent->nkids; i++)
            	{
	            	if(nodeparent->kids[i]->prodrule >= 4232 && nodeparent->kids[i]->prodrule <= 4243)
	            	{ /* conditional found */
		            	cond = i + 1;
	            	}	
	            	if(nodeparent->kids[i]->prodrule >= 4501 && nodeparent->kids[i]->prodrule <= 4507)
	            	{ /* stmt found */
		            	statm = i + 1;
		            	true_found++;
	            	}	
	            	if(nodeparent->kids[i]->prodrule == 4581)
	            	{
		            	sinit = i + 1;
	            	}
	            	if(nodeparent->kids[i]->prodrule == 4521)
	            	{
		            	expr = i+1;
	            	}
            	}
            	orignode = nodeparent;
            	if(cond)
            	{
            		myparent = orignode->kids[cond-1];
            		forcnt++;
	            	nextnexttac[forcnt] = newlabel();
	            	t3list = new_listmem();
	            	t3list->instruct = nextnexttac[forcnt];
	            	myparent->code = concat(myparent->code, t3list);
            	}
            	if(statm)
            	{
	            	nodeparent = orignode->kids[statm-1];
                    truetac = newlabel();
                    tlist = new_listmem();
                    tlist->instruct = truetac;
                    nodeparent->code = concat(nodeparent->code, tlist);
            	}
            	if(expr)
            	{
	            	myparent = orignode->kids[expr-1];	
	            	forgcnt++;
	            	gnexttac[forgcnt] = newlabel();
	            	t2list = new_listmem();
	            	t2list->instruct = gnexttac[forgcnt];
	            	myparent->code = concat(myparent->code, t2list);
            	}
            	break;
        }
        if(true_found)
        {
            break;
        }
        nodeparent = nodeparent->parent;
    }
    return truetac;
}

struct taclist *check_obj(struct typeinfo *t, struct sym_table *st)
{
    struct typeinfo *tmp1;
    struct typeinfo *tmp2;
    struct typeinfo *tmp3;
    struct typeinfo *tmp4;
    struct taclist *tlist = 0;
    struct tac *tacinstruct;
    if(t == NULL || t->myclass == NULL)
    {
        return tlist;   
    }    
    /* otherwise do pointer math with local:-8 (first param) as the this pointer */
    tmp1 = newvar(PTR_TYPE, st);
    tlist = new_listmem();
    tacinstruct = make_tac();
    tacinstruct->address[0]->region = R_LOCAL;
    tacinstruct->address[0]->u.offset = -8;
    tacinstruct->address[0]->size = 8;
    tlist->instruct = fill_in_tac(O_ADDR, &tmp1->memaddr, tacinstruct->address[0], NullAddr);
    tmp2 = new_const(t->memaddr.u.offset, 0, INT_TYPE);
    tmp3 = newvar(INT_TYPE, st);
    tacinstruct = fill_in_tac(O_ADD, &tmp3->memaddr, &tmp1->memaddr, &tmp2->memaddr);        
    tlist = add_taclist(tacinstruct, tlist);
    tmp4 = newvar(t->basetype, st);
    tacinstruct = fill_in_tac(O_LCONT, &tmp4->memaddr, &tmp3->memaddr, NullAddr);
    tlist = add_taclist(tacinstruct, tlist);        
    return tlist;
}

struct tac *getlast(struct taclist *tl)
{
    struct tac *tmp = 0;
    struct addr ra;
    while(tl->next != NULL)
    {
        tl = tl->next;
    }
    tmp = make_tac();
    tmp->address[0] = tl->instruct->address[0];
    return tmp;
}

struct tac *obj_lists(struct taclist *newsrc, struct addr *altaddr)
{
    struct tac *tmptac = 0;
    if(newsrc == NULL)
    {
        tmptac = make_tac();
        tmptac->address[0] = altaddr;
    }
    else
    {
        tmptac = getlast(newsrc);
    }
    return tmptac;
}

struct tac *obj_asn(struct taclist *tl, struct addr *a)
{
	struct tac *tmptac = 0;
	if(tl != NULL)
	{
		while(tl->prev != NULL)
		{ /* go to beginning */
			tl = tl->prev;	
		}	
	}
	while(tl != NULL)
	{
		if(tl->instruct->op == O_LCONT)
		{
			if(tl->instruct->address[0]->region == a->region && tl->instruct->address[0]->u.offset == a->u.offset)
			{ /* put the value back into the pointer area */
				tmptac = fill_in_tac(O_SCONT, tl->instruct->address[1], a, NullAddr);
				break;
			}
		}	
		tl = tl->next;
	}
	return tmptac;
}

//!!! reverse parm list

void calc_values(parsetree *n, struct sym_table *st)
{ /* post order traversal to do code for expressions, stores in temps */
    int i, j;
    parsetree *tnode;
    struct typeinfo *tfound;
    struct typeinfo *tmp1;
    struct typeinfo *tmp2;
    struct typeinfo *tmp3; 
    struct typeinfo *tmp4;
    struct typeinfo *tmp5;
    struct sym_table *nextst;
    struct taclist *tlist;
    struct taclist *taddup;
    struct taclist *newlist;
    struct tac *tacinstruct;
    struct tac *nexttac;
    struct tac *tmptac1;
    struct tac *tmptac2;
    char cnstr[100]; /* for construtor name */
    int sinit=0;
    int cond=0;
    int expr=0;
    int statm=0;
    
    if(n == NULL)
    {
        return;   
    }
    nextst = st;
    if(n->typ != NULL && n->typ->st != NULL)
    {
        nextst = n->typ->st;   
    }
    for(i = 0; i < n->nkids; i++)
    {
        calc_values(n->kids[i], nextst);   
    }
    switch(n->prodrule)
    {
        case STATEMENTS_ITER3: /* 4573, for */
        	for(j = 0; j < n->nkids; j++)
        	{
            	if(n->kids[j]->prodrule >= 4232 && n->kids[j]->prodrule <= 4243)
            	{ /* conditional found */
	            	cond = j + 1;
            	}	
            	if(n->kids[j]->prodrule >= 4501 && n->kids[j]->prodrule <= 4507)
            	{ /* stmt found */
	            	statm = j + 1;
            	}	
            	if(n->kids[j]->prodrule == 4581)
            	{
	            	sinit = j + 1;
            	}
            	if(n->kids[j]->prodrule == 4521)
            	{
	            	expr = j+1;
            	}
        	}
            if(expr >0 && statm >0)
            { /* Add goto to counter increment to end of statement */
	            tacinstruct = fill_in_tac(O_GOTO, gnexttac[forgcnt]->address[0], NullAddr, NullAddr);
	            n->kids[statm-1]->code = add_taclist(tacinstruct, n->kids[statm-1]->code);
	            forgcnt--;
            }
            if(expr > 0 && cond > 0)
            { /* send back to test */
	            tacinstruct = fill_in_tac(O_GOTO, nextnexttac[forcnt]->address[0], NullAddr, NullAddr);
	            n->kids[expr-1]->code = add_taclist(tacinstruct, n->kids[expr-1]->code);
	            forcnt--;
            }
            taddup = addupcode(n);
            n->code = concat(n->code, taddup);
        	break;
        case EXPRESSION_NEWEX: /* 4111, new */
            tnode = FindNode(n, st, DECLARATORS_TYPESEQ);
            switch(tnode->typ->basetype)
            {
                case CLASS_TYPE:
                    /* malloc then constr */
                    tmp1 = search_type(symtab, tnode->kids[0]->leaf->text); /* classes should be global */
                    tmp2 = new_const(tmp1->memaddr.size, 0, INT_TYPE);
                    tlist = new_listmem();
                    /* push class size as a parameter for malloc */
                    tlist->instruct = fill_in_tac(O_PARM, &tmp2->memaddr, NullAddr, NullAddr);
                    tacinstruct = make_tac();
                    tmp3 = newvar(PTR_TYPE, nextst); /* store return here */
                    tmp3->u.p.elemtype = make_room(CLASS_TYPE);
                    tmp3->u.p.elemtype->name = copy_name(tmp1->name);
                    nexttac = make_tac();
                    nexttac->address[0]->size = 8; /* size of parameter passed to malloc */
                    tacinstruct = fill_in_tac(O_CALL, &MallocAddr, nexttac->address[0], &tmp3->memaddr);
                    tlist = add_taclist(tacinstruct, tlist);
                    /* now call construtor, ie classname__classname with pointer as a parameter */
                    tmp2 = search_type(tmp1->st, tmp1->name); /* look for the construtor */
                    tmptac1 = make_tac();
                    if(tmp2 == NULL)
                    {  /* build constructor */
                        tmptac1->address[0]->region = R_CODE;
                        sprintf(cnstr, "%s__%s", tmp1->name, tmp1->name);
                        tmptac1->address[0]->u.name = copy_name(cnstr);
                    }
                    else
                    {
                        tmptac1->address[0] = &tmp2->memaddr;
                    }
                    /* push pointer for constructor */
                    tacinstruct = fill_in_tac(O_PARM, &tmp3->memaddr, NullAddr, NullAddr);
                    tlist = add_taclist(tacinstruct, tlist);
                    /* since INT_TYPE & PTR_TYPE are same size, we can reuse nexttac as parameter size */
                    tacinstruct = fill_in_tac(O_CALL, tmptac1->address[0], nexttac->address[0], NullAddr);
                    tlist = add_taclist(tacinstruct, tlist);
                    break;
                case INT_TYPE:
                case BOOL_TYPE:
                case FLOAT_TYPE:
                case CHAR_TYPE:
                case PTR_TYPE:
                    /* malloc only, all the same size (after alignment) */
                    nexttac = make_tac();
                    nexttac->address[0]->size = 8; /* size of parameter passed to malloc */
                    tmp1 = new_const(nexttac->address[0]->size, 0, INT_TYPE);
                    tlist = new_listmem();
                    tlist->instruct = fill_in_tac(O_PARM, &tmp1->memaddr, NullAddr, NullAddr);
                    tmp3 = newvar(PTR_TYPE, nextst); /* store return here */
                    tmp3->u.p.elemtype = make_room(tnode->typ->basetype);
                    tacinstruct = fill_in_tac(O_CALL, &MallocAddr, nexttac->address[0], &tmp3->memaddr);
                    tlist = add_taclist(tacinstruct, tlist);
                    break;
                case ARR_TYPE:
                    /* malloc only */
                    nexttac = make_tac();
                    nexttac->address[0]->size = align_size(size_up_arr(tnode->typ));
                    tmp1 = new_const(nexttac->address[0]->size, 0, INT_TYPE);
                    tlist = new_listmem();
                    tlist->instruct = fill_in_tac(O_PARM, &tmp1->memaddr, NullAddr, NullAddr);
                    tmp3 = newvar(PTR_TYPE, nextst); /* store return here */
                    tmp3->u.p.elemtype = make_room(tnode->typ->basetype);
                    tacinstruct = fill_in_tac(O_CALL, &MallocAddr, nexttac->address[0], &tmp3->memaddr);
                    tlist = add_taclist(tacinstruct, tlist);
                    break;
                default:
                    lineno = n->line_num;
                    curfilename = n->filename;
                    semantic_error(build_errstr(" ", "Type mismatch!"));
                    break;
            }
            n->final = tmp3->memaddr;
            taddup = addupcode(n);
            n->code = concat(n->code, taddup);
            n->code = concat(n->code, tlist);
            break;
        case DECLARATORS_FUN: /* 5651 */
        case DECLARATORS_FUN2: /* 5652 */
            tlist = new_listmem();
            tacinstruct = make_tac();
            tmp1 = search_type(n->typ->i_am_defined_here, n->typ->name);
            if(tmp1->basetype != FUNC_TYPE)
            { /* we found class not function */
	         	tmp1 = search_type(nextst, n->typ->name);   
            }
            nexttac = make_tac();
            tacinstruct->address[0]->size = tmp1->u.f.paramsize;
            nexttac->address[0]->size = tmp1->st->offset;
            /* if this is a class function then we need to pass the this pointer in */
            /* this pointer will be the first on the stack at -8 */
            /* the function and parameter sizes already have this implicit pointer size of 8 added in */
            /* in the class instance, the parameters should be number correctly to pull starting at -(8+8) */
            tlist->instruct = fill_in_tac(D_PROC, &tmp1->memaddr, tacinstruct->address[0], nexttac->address[0]);
            n->code = concat(tlist, n->code);
            taddup = addupcode(n);
            /* if the last command was not a return, then make it so */
            if(taddup == NULL || taddup->instruct == NULL || taddup->instruct->op != O_RET)
            { /* let's put a return statment in */
                if(taddup == NULL)
                {
                    taddup = new_listmem();
                    taddup->instruct = fill_in_tac(O_RET, NullAddr, NullAddr, NullAddr);
                }
                else
                {
                    tacinstruct = fill_in_tac(O_RET, NullAddr, NullAddr, NullAddr);
                    taddup = add_taclist(tacinstruct, taddup);
                }
            }
            n->code = concat(n->code, taddup);
            tacinstruct = fill_in_tac(D_END, NullAddr, NullAddr, NullAddr);
            n->code = add_taclist(tacinstruct, n->code);
            break;
		case EXPRESSION_REL2: /* 4232, LT */
            /* find true label */
            tacinstruct = find_true(n);
            if(tacinstruct != NULL)
            { /* flow statement found, otherwise it's just a random test so skip processing */
                tlist = check_obj(n->kids[0]->typ, st);
                tmptac1 = obj_lists(tlist, &n->kids[0]->final);
                newlist = check_obj(n->kids[1]->typ, st);
                tmptac2 = obj_lists(newlist, &n->kids[1]->final);
                tlist = concat(tlist, newlist);
                nexttac = fill_in_tac(O_BLT, tmptac1->address[0], tmptac2->address[0], tacinstruct->address[0]);
                if(tlist == NULL)
                {
                    tlist = new_listmem();
                    tlist->instruct = nexttac;   
                }
                else
                {
                    tlist = add_taclist(nexttac, tlist);
                }
                tacinstruct = find_false(n); /* now find false or continue label */ 
                if(tacinstruct == NULL)
                { /* end of proc */
                    nexttac = fill_in_tac(O_RET, NullAddr, NullAddr, NullAddr);
                }
                else
                {
                    nexttac = fill_in_tac(O_GOTO, tacinstruct->address[0], NullAddr, NullAddr);
                }
                tlist = add_taclist(nexttac, tlist);      
                taddup = addupcode(n);
                n->code = concat(n->code, taddup);
                n->code = concat(n->code, tlist);
            }
            else
            {
                taddup = addupcode(n);
                n->code = concat(n->code, taddup);
            }
            break;
		case EXPRESSION_REL3: /* 4233, GT */
            /* find true label */
            tacinstruct = find_true(n);
            if(tacinstruct != NULL)
            { /* flow statement found, otherwise it's just a random test so skip processing */
                tlist = check_obj(n->kids[0]->typ, st);
                tmptac1 = obj_lists(tlist, &n->kids[0]->final);
                newlist = check_obj(n->kids[1]->typ, st);
                tmptac2 = obj_lists(newlist, &n->kids[1]->final);
                tlist = concat(tlist, newlist);
                nexttac = fill_in_tac(O_BGT, tmptac1->address[0], tmptac2->address[0], tacinstruct->address[0]);
                if(tlist == NULL)
                {
                    tlist = new_listmem();
                    tlist->instruct = nexttac;   
                }
                else
                {
                    tlist = add_taclist(nexttac, tlist);
                }
                tacinstruct = find_false(n); /* now find false or continue label */ 
                if(tacinstruct == NULL)
                { /* end of proc */
                    nexttac = fill_in_tac(O_RET, NullAddr, NullAddr, NullAddr);
                }
                else
                {
                    nexttac = fill_in_tac(O_GOTO, tacinstruct->address[0], NullAddr, NullAddr);
                }
                tlist = add_taclist(nexttac, tlist);      
                taddup = addupcode(n);
                n->code = concat(n->code, taddup);
                n->code = concat(n->code, tlist);
            }
            else
            {
                taddup = addupcode(n);
                n->code = concat(n->code, taddup);
            }
            break;
		case EXPRESSION_REL4: /* 4234, LTEQ */
            /* find true label */
            tacinstruct = find_true(n);
            if(tacinstruct != NULL)
            { /* flow statement found, otherwise it's just a random test so skip processing */
                tlist = check_obj(n->kids[0]->typ, st);
                tmptac1 = obj_lists(tlist, &n->kids[0]->final);
                newlist = check_obj(n->kids[1]->typ, st);
                tmptac2 = obj_lists(newlist, &n->kids[1]->final);
                tlist = concat(tlist, newlist);
                nexttac = fill_in_tac(O_BLE, tmptac1->address[0], tmptac2->address[0], tacinstruct->address[0]);
                if(tlist == NULL)
                {
                    tlist = new_listmem();
                    tlist->instruct = nexttac;   
                }
                else
                {
                    tlist = add_taclist(nexttac, tlist);
                }
                tacinstruct = find_false(n); /* now find false or continue label */ 
                if(tacinstruct == NULL)
                { /* end of proc */
                    nexttac = fill_in_tac(O_RET, NullAddr, NullAddr, NullAddr);
                }
                else
                {
                    nexttac = fill_in_tac(O_GOTO, tacinstruct->address[0], NullAddr, NullAddr);
                }
                tlist = add_taclist(nexttac, tlist);      
                taddup = addupcode(n);
                n->code = concat(n->code, taddup);
                n->code = concat(n->code, tlist);
            }
            else
            {
                taddup = addupcode(n);
                n->code = concat(n->code, taddup);
            }
            break;
		case EXPRESSION_REL5: /* 4235, GTEQ */
            /* find true label */
            tacinstruct = find_true(n);
            if(tacinstruct != NULL)
            { /* flow statement found, otherwise it's just a random test so skip processing */
                tlist = check_obj(n->kids[0]->typ, st);
                tmptac1 = obj_lists(tlist, &n->kids[0]->final);
                newlist = check_obj(n->kids[1]->typ, st);
                tmptac2 = obj_lists(newlist, &n->kids[1]->final);
                tlist = concat(tlist, newlist);
                nexttac = fill_in_tac(O_BGE, tmptac1->address[0], tmptac2->address[0], tacinstruct->address[0]);
                if(tlist == NULL)
                {
                    tlist = new_listmem();
                    tlist->instruct = nexttac;   
                }
                else
                {
                    tlist = add_taclist(nexttac, tlist);
                }
                tacinstruct = find_false(n); /* now find false or continue label */ 
                if(tacinstruct == NULL)
                { /* end of proc */
                    nexttac = fill_in_tac(O_RET, NullAddr, NullAddr, NullAddr);
                }
                else
                {
                    nexttac = fill_in_tac(O_GOTO, tacinstruct->address[0], NullAddr, NullAddr);
                }
                tlist = add_taclist(nexttac, tlist);      
                taddup = addupcode(n);
                n->code = concat(n->code, taddup);
                n->code = concat(n->code, tlist);
            }
            else
            {
                taddup = addupcode(n);
                n->code = concat(n->code, taddup);
            }
            break;
        case EXPRESSION_EQ3: /* 4243, != */ 
            /* find true label */
            tacinstruct = find_true(n);
            if(tacinstruct != NULL)
            { /* flow statement found, otherwise it's just a random test so skip processing */
                tlist = check_obj(n->kids[0]->typ, st);
                tmptac1 = obj_lists(tlist, &n->kids[0]->final);
                newlist = check_obj(n->kids[1]->typ, st);
                tmptac2 = obj_lists(newlist, &n->kids[1]->final);
                tlist = concat(tlist, newlist);
                nexttac = fill_in_tac(O_BNE, tmptac1->address[0], tmptac2->address[0], tacinstruct->address[0]);
                if(tlist == NULL)
                {
                    tlist = new_listmem();
                    tlist->instruct = nexttac;   
                }
                else
                {
                    tlist = add_taclist(nexttac, tlist);
                }
                tacinstruct = find_false(n); /* now find false or continue label */ 
                if(tacinstruct == NULL)
                { /* end of proc */
                    nexttac = fill_in_tac(O_RET, NullAddr, NullAddr, NullAddr);
                }
                else
                {
                    nexttac = fill_in_tac(O_GOTO, tacinstruct->address[0], NullAddr, NullAddr);
                }
                tlist = add_taclist(nexttac, tlist);      
                taddup = addupcode(n);
                n->code = concat(n->code, taddup);
                n->code = concat(n->code, tlist);
            }
            else
            {
                taddup = addupcode(n);
                n->code = concat(n->code, taddup);
            }
            break;
        case EXPRESSION_EQ2: /* 4242, == */
            /* find true label */
            tacinstruct = find_true(n);
            if(tacinstruct != NULL)
            { /* flow statement found, otherwise it's just a random test so skip processing */
                tlist = check_obj(n->kids[0]->typ, st);
                tmptac1 = obj_lists(tlist, &n->kids[0]->final);
                newlist = check_obj(n->kids[1]->typ, st);
                tmptac2 = obj_lists(newlist, &n->kids[1]->final);
                tlist = concat(tlist, newlist);
                nexttac = fill_in_tac(O_BEQ, tmptac1->address[0], tmptac2->address[0], tacinstruct->address[0]);
                if(tlist == NULL)
                {
                    tlist = new_listmem();
                    tlist->instruct = nexttac;   
                }
                else
                {
                    tlist = add_taclist(nexttac, tlist);
                }
                tacinstruct = find_false(n); /* now find false or continue label */ 
                if(tacinstruct == NULL)
                { /* end of proc */
                    nexttac = fill_in_tac(O_RET, NullAddr, NullAddr, NullAddr);
                }
                else
                {
                    nexttac = fill_in_tac(O_GOTO, tacinstruct->address[0], NullAddr, NullAddr);
                }
                tlist = add_taclist(nexttac, tlist);      
                taddup = addupcode(n);
                n->code = concat(n->code, taddup);
                n->code = concat(n->code, tlist);
            }
            else
            {
                taddup = addupcode(n);
                n->code = concat(n->code, taddup);
            }
            break;
        case EXPRESSION_POSTFIX2: /* 4052, array script, array[expr] */
            tmp1 = newvar(PTR_TYPE, st);            
            newlist = check_obj(n->kids[0]->typ, st);
            tmptac1 = obj_lists(newlist, &n->kids[0]->typ->memaddr);
            tlist = concat(tlist, newlist);
            nexttac = fill_in_tac(O_ADDR, &tmp1->memaddr, tmptac1->address[0], NullAddr);
            newlist = check_obj(n->kids[1]->typ, st);
            tmptac2 = obj_lists(newlist, &n->kids[1]->final);
            tlist = concat(tlist, newlist);
            tmp2 = newvar(INT_TYPE, st);
            if(n->kids[0]->typ->u.a.elemtype->basetype == ARR_TYPE)
            { /* for 2D arrays */
                i = size_up_arr(n->kids[0]->typ->u.a.elemtype);
            }
            else
            {
                i = size_up_simple(n->kids[0]->typ->u.a.elemtype->basetype);
            }
            tmp5 = new_const(i, 0, INT_TYPE);
            tacinstruct = fill_in_tac(O_MUL, &tmp2->memaddr, tmptac2->address[0], &tmp5->memaddr);
            tlist = add_taclist(tacinstruct, tlist);
            tmp3 = newvar(INT_TYPE, nextst);
            tacinstruct = fill_in_tac(O_ADD, &tmp3->memaddr, &tmp1->memaddr, &tmp2->memaddr);
            tlist = add_taclist(tacinstruct, tlist);
            tmp4 = newvar(n->kids[0]->typ->u.a.elemtype->basetype, nextst);
            tacinstruct = fill_in_tac(O_LCONT, &tmp4->memaddr, &tmp3->memaddr, NullAddr);
            tlist = add_taclist(tacinstruct, tlist);
            n->final = tmp4->memaddr;
            taddup = addupcode(n);
            n->code = concat(n->code, taddup);
            n->code = concat(n->code, tlist);
            break;
        case EXPRESSION_POSTFIX5: /* 4055 */
            /* Make this node point to function */
            tmp1 = scroll_search(st, n->kids[1]->typ->name);
            if(tmp1 == NULL)
            {
                tmp1 = scroll_search(nextst, n->kids[1]->typ->name);   
            }
            n->typ = make_room(tmp1->basetype);
            n->typ = tmp1;
            /* push this pointer onto stack */
            tmp2 = scroll_search(st, n->kids[0]->typ->name);
            tlist = new_listmem();
            tlist->instruct = fill_in_tac(O_PARM, &tmp2->memaddr, NullAddr, NullAddr);
            taddup = addupcode(n);
            n->code = concat(n->code, taddup);
            n->code = concat(n->code, tlist);
            break;
        case EXPRESSION_POSTFIX3: /* 4053, function call */
            i = 0;
            tlist = new_listmem();
            if(n->nkids > 1 && n->kids[1]->prodrule != EXPRESSION_LIST2)
            {
                tlist->instruct = fill_in_tac(O_PARM, &n->kids[1]->final, NullAddr, NullAddr);
                i++;
            }
            tmp2 = search_type(n->kids[0]->typ->i_am_defined_here, n->kids[0]->typ->name);
            tmptac1 = make_tac();
            if(tmp2->u.f.returntype->basetype != NULL_TYPE)
            { /* find return type and make room if needed */
                tmp1 = newvar(tmp2->u.f.returntype->basetype, nextst);
                n->final = tmp1->memaddr;
                tmptac1->address[0] = &tmp1->memaddr;
            }
            else
            {
                tmptac1->address[0] = NullAddr;   
            }
            nexttac = make_tac();
            nexttac->address[0]->size = n->kids[0]->typ->u.f.paramsize;
            tacinstruct = fill_in_tac(O_CALL, &tmp2->memaddr, nexttac->address[0], tmptac1->address[0]);
            if(i == 0)
            {
                tlist->instruct = tacinstruct;
            }
            else
            {
                tlist = add_taclist(tacinstruct, tlist);
            }
            taddup = addupcode(n);
            n->code = concat(n->code, taddup);
            n->code = concat(n->code, tlist);
            break;
        case EXPRESSION_LIST2: /* 4072, param list */
            tlist = new_listmem();
            if(n->kids[0]->prodrule != EXPRESSION_LIST2)
            { 
                tlist->instruct = fill_in_tac(O_PARM, &n->kids[0]->final, NullAddr, NullAddr);
                tacinstruct = fill_in_tac(O_PARM, &n->kids[1]->final, NullAddr, NullAddr);
                tlist = add_taclist(tacinstruct, tlist);
            }
            else
            { /* Don't process kid[0] if it's a parameter list, it's been processed lower down */
                tlist->instruct = fill_in_tac(O_PARM, &n->kids[1]->final, NullAddr, NullAddr);
            }
            taddup = addupcode(n);
            n->code = concat(n->code, taddup);
            n->code = concat(n->code, tlist);
            break;
        case EXPRESSION_ASN2: /* 4312 */
            i = 0;
            switch(n->kids[1]->prodrule)
            {
                case EXPRESSION_OP2: /* 4322, *= */
                    i = O_MUL;
                    break;
                case EXPRESSION_OP3: /* 4323, /= */
                    i = O_DIV;
                    break;
                case EXPRESSION_OP4: /* 4324, %= */
                    i = O_MOD;
                    break;
                case EXPRESSION_OP5: /* 4325, += */
                    i = O_ADD;
                    break;
                case EXPRESSION_OP6: /* 4326, -= */
                    i = O_SUB;
                    break;
            }
            tlist = new_listmem();
            if(i > 0)
            {  /* do the arthmitic & assign */
                tlist = check_obj(n->kids[0]->typ, st);
                tmptac1 = obj_lists(tlist, &n->kids[0]->final);
                newlist = check_obj(n->kids[2]->typ, st);
                tmptac2 = obj_lists(newlist, &n->kids[2]->final);
                tlist = concat(tlist, newlist);
                tmp1 = newvar(n->typ->basetype, nextst);
                tacinstruct = fill_in_tac(i, &tmp1->memaddr, tmptac1->address[0], tmptac2->address[0]);
                tlist = add_taclist(tacinstruct, tlist); 
                tacinstruct = fill_in_tac(O_ASN, tmptac1->address[0], &tmp1->memaddr, NullAddr);
                tlist = add_taclist(tacinstruct, tlist);
                tacinstruct = obj_asn(tlist, tmptac1->address[0]);
                tlist = add_taclist(tacinstruct, tlist);
            }
            else
            { /* just assign */
                tlist = check_obj(n->kids[0]->typ, st);
                tmptac1 = obj_lists(tlist, &n->kids[0]->final);
                newlist = check_obj(n->kids[2]->typ, st);
                tmptac2 = obj_lists(newlist, &n->kids[2]->final);
                tlist = concat(tlist, newlist);
                tacinstruct = fill_in_tac(O_ASN, tmptac1->address[0], tmptac2->address[0], NullAddr);
                tlist = add_taclist(tacinstruct, tlist); 
                tacinstruct = obj_asn(tlist, tmptac1->address[0]);
                tlist = add_taclist(tacinstruct, tlist);
            }                   
            n->final = *tmptac1->address[0];
            taddup = addupcode(n);
            n->code = concat(n->code, taddup);
            n->code = concat(n->code, tlist);
            break;
        case EXPRESSION_POSTFIX8: /* 4058 */
    		tmp1 = newvar(INT_TYPE, nextst);
			tmp2 = new_const(1, 0, INT_TYPE);
            tlist = check_obj(n->kids[0]->typ, st);
            tmptac1 = obj_lists(tlist, &n->kids[0]->final);
            tacinstruct = fill_in_tac(O_ADD, &tmp1->memaddr, tmptac1->address[0], &tmp2->memaddr);
            tlist = add_taclist(tacinstruct, tlist);
            tacinstruct = fill_in_tac(O_ASN, tmptac1->address[0], &tmp1->memaddr, NullAddr);
            tlist = add_taclist(tacinstruct, tlist); 
            tacinstruct = obj_asn(tlist, tmptac1->address[0]);
            tlist = add_taclist(tacinstruct, tlist);
            taddup = addupcode(n);
            n->code = concat(n->code, taddup);
            n->code = concat(n->code, tlist);
          	break;
        case EXPRESSION_POSTFIX9: /* 4059 */
    		tmp1 = newvar(INT_TYPE, nextst);
			tmp2 = new_const(1, 0, INT_TYPE);
            tlist = check_obj(n->kids[0]->typ, st);
            tmptac1 = obj_lists(tlist, &n->kids[0]->final);
            tacinstruct = fill_in_tac(O_SUB, &tmp1->memaddr, tmptac1->address[0], &tmp2->memaddr);
            tlist = add_taclist(tacinstruct, tlist);
            tacinstruct = fill_in_tac(O_ASN, tmptac1->address[0], &tmp1->memaddr, NullAddr);
            tlist = add_taclist(tacinstruct, tlist); 
            tacinstruct = obj_asn(tlist, tmptac1->address[0]);
            tlist = add_taclist(tacinstruct, tlist);
            taddup = addupcode(n);
            n->code = concat(n->code, taddup);
            n->code = concat(n->code, tlist);
          	break;
        case EXPRESSION_MULT2: /* 4202, a * b */
            tmp1 = newvar(n->typ->basetype, nextst);
            tlist = check_obj(n->kids[0]->typ, st);
            tmptac1 = obj_lists(tlist, &n->kids[0]->final);
            newlist = check_obj(n->kids[1]->typ, st);
            tmptac2 = obj_lists(newlist, &n->kids[1]->final);
            tlist = concat(tlist, newlist);
            tacinstruct = fill_in_tac(O_MUL, &tmp1->memaddr, tmptac1->address[0], tmptac2->address[0]);
            tlist = add_taclist(tacinstruct, tlist);
            n->final = tmp1->memaddr;
            taddup = addupcode(n);
            n->code = concat(n->code, taddup);
            n->code = concat(n->code, tlist);
            break;
        case EXPRESSION_MULT3: /* 4203, a / b */
            tmp1 = newvar(n->typ->basetype, nextst);
            tlist = check_obj(n->kids[0]->typ, st);
            tmptac1 = obj_lists(tlist, &n->kids[0]->final);
            newlist = check_obj(n->kids[1]->typ, st);
            tmptac2 = obj_lists(newlist, &n->kids[1]->final);
            tlist = concat(tlist, newlist);
            tacinstruct = fill_in_tac(O_DIV, &tmp1->memaddr, tmptac1->address[0], tmptac2->address[0]);
            tlist = add_taclist(tacinstruct, tlist);
            n->final = tmp1->memaddr;
            taddup = addupcode(n);
            n->code = concat(n->code, taddup);
            n->code = concat(n->code, tlist);
            break;
        case EXPRESSION_MULT4: /* 4204, a % b */
            tmp1 = newvar(n->typ->basetype, nextst);
            tlist = check_obj(n->kids[0]->typ, st);
            tmptac1 = obj_lists(tlist, &n->kids[0]->final);
            newlist = check_obj(n->kids[1]->typ, st);
            tmptac2 = obj_lists(newlist, &n->kids[1]->final);
            tlist = concat(tlist, newlist);
            tacinstruct = fill_in_tac(O_MOD, &tmp1->memaddr, tmptac1->address[0], tmptac2->address[0]);
            tlist = add_taclist(tacinstruct, tlist);
            n->final = tmp1->memaddr;
            taddup = addupcode(n);
            n->code = concat(n->code, taddup);
            n->code = concat(n->code, tlist);
            break;
        case EXPRESSION_ADD3: /* 4213, a - b */
            tmp1 = newvar(n->typ->basetype, nextst);
            tlist = check_obj(n->kids[0]->typ, st);
            tmptac1 = obj_lists(tlist, &n->kids[0]->final);
            newlist = check_obj(n->kids[1]->typ, st);
            tmptac2 = obj_lists(newlist, &n->kids[1]->final);
            tlist = concat(tlist, newlist);
            tacinstruct = fill_in_tac(O_SUB, &tmp1->memaddr, tmptac1->address[0], tmptac2->address[0]);
            tlist = add_taclist(tacinstruct, tlist);
            n->final = tmp1->memaddr;
            taddup = addupcode(n);
            n->code = concat(n->code, taddup);
            n->code = concat(n->code, tlist);
            break;
        case EXPRESSION_ADD2: /* 4212, a + b */
            tmp1 = newvar(n->typ->basetype, nextst);
            tlist = check_obj(n->kids[0]->typ, st);
            tmptac1 = obj_lists(tlist, &n->kids[0]->final);
            newlist = check_obj(n->kids[1]->typ, st);
            tmptac2 = obj_lists(newlist, &n->kids[1]->final);
            tlist = concat(tlist, newlist);
            tacinstruct = fill_in_tac(O_ADD, &tmp1->memaddr, tmptac1->address[0], tmptac2->address[0]);
            tlist = add_taclist(tacinstruct, tlist);
            n->final = tmp1->memaddr;
            taddup = addupcode(n);
            n->code = concat(n->code, taddup);
            n->code = concat(n->code, tlist);
            break;
        case STATEMENTS_JUMP: /* 4591, break */
			tacinstruct = find_break(n); /* now find false or continue label */ 
	        if(tacinstruct == NULL)
	        { /* end of proc */
	            nexttac = fill_in_tac(O_RET, NullAddr, NullAddr, NullAddr);
	        }
	        else
	        {
	            nexttac = fill_in_tac(O_GOTO, tacinstruct->address[0], NullAddr, NullAddr);
	        }
	        tlist = new_listmem();
	        tlist->instruct = nexttac;
	        taddup = addupcode(n);
	        n->code = concat(n->code, taddup);
	        n->code = concat(n->code, tlist);
        	break;
        case STATEMENTS_JUMP3: /* 4593, return */
            tlist = check_obj(n->kids[0]->typ, st);
            tmptac1 = obj_lists(tlist, &n->kids[0]->final);
            tacinstruct = fill_in_tac(O_RET, tmptac1->address[0], NullAddr, NullAddr);
            tlist = add_taclist(tacinstruct, tlist);
            taddup = addupcode(n);
            n->code = concat(n->code, taddup);
            n->code = concat(n->code, tlist);
            break;
        case STATEMENTS_EX: /* 4521 */     
        	n->final = n->kids[0]->final;
            taddup = addupcode(n);
            n->code = concat(n->code, taddup);
        	break;
        case DECLARATORS_INIT: /* 5511, declare & assign */ 
            if(n->nkids == 2)
            {
                if(n->kids[0]->typ == NULL || n->kids[0]->typ->name == NULL)
                {
                    tmptac1 = make_tac();
                    tmptac1->address[0] = &n->kids[0]->final;
                }
                else
                {
                    if(n->kids[1]->prodrule == EXPRESSION_NEWEX) /* 4111, new */
                    { /* don't do anything! it's done on 4111 */
                        n->final = n->kids[1]->final; /* address of instance pointer */
                        //!!! maybe put in myclass?
                        /* move the this pointer into the memaddress of the instance */
                        tlist = new_listmem();   
                        tmp1 = search_type(st, n->kids[0]->typ->name);
                        tlist->instruct = fill_in_tac(O_ASN, &tmp1->memaddr, &n->kids[1]->final, NullAddr);   
                    }
                    else
                    {
                        tmptac1 = make_tac();
                        tmptac2 = make_tac();
                        tmp1 = search_type(nextst, n->kids[0]->typ->name); //!!! maybe dig for lexident?
                        tmptac1->address[0] = &tmp1->memaddr;
                        if(n->kids[1]->typ == NULL || n->kids[1]->typ->name == NULL)
                        {
                            tmptac2->address[0] = &n->kids[1]->final;
                        }
                        else
                        {
                            tmp2 = search_type(nextst, n->kids[1]->typ->name);
                            tmptac2->address[0] = &tmp2->memaddr;
                        }
                        tlist = new_listmem();   
                        tlist->instruct = fill_in_tac(O_ASN, tmptac1->address[0], tmptac2->address[0], NullAddr);   
                        n->final = n->kids[0]->final;
                    }
                    taddup = addupcode(n);
                    n->code = concat(n->code, taddup);
                    n->code = concat(n->code, tlist);
                }
            }
            else
            {
                taddup = addupcode(n);
                n->code = concat(n->code, taddup);
            }
            break;
            
        default:
            taddup = addupcode(n);
            n->code = concat(n->code, taddup);
            break;
    }
    
}

struct tac *fill_in_tac(int operation, struct addr *dest, struct addr *src1, struct addr *src2)
{
    struct tac *tacinstruct;
    tacinstruct = make_tac();
    tacinstruct->op = operation;
    tacinstruct->address[0] = dest;
    tacinstruct->address[1] = src1;
    tacinstruct->address[2] = src2;
    return tacinstruct;
}

struct taclist *addupcode(parsetree *n)
{
    int i;
    struct taclist *tlist = 0;
    for(i = 0; i < n->nkids; i++)
    {
        tlist = concat(tlist, n->kids[i]->code);
    }    
    return tlist;
}

void print_code(struct taclist *tmp)
{
    struct taclist *t;
    t = tmp;
    int i;
    while(t->prev != NULL)
    {
        t = t->prev;   
    }
    while(t != NULL)
    {
        for(i = 0; i < 3; i++)
        {
            if(t->instruct->address[i] == NULL || t->instruct->op == 0)
            { /* done, no more addresses used */
                break;
            }
            if(i == 0)
            { /* print operation */
                switch(t->instruct->op)
                {
                    case O_ADD: /* 9001 */
                        fprintf(stdout, "\t%s", "ADD");
                        break;
                    case O_SUB: /* 9002 */
                        fprintf(stdout, "\t%s", "SUB");
                        break;
                    case O_MUL: /* 9003 */
                        fprintf(stdout, "\t%s", "MUL");
                        break;
                    case O_DIV: /* 9004 */
                        fprintf(stdout, "\t%s", "DIV");
                        break;
                    case O_NEG: /* 9005 */
                        fprintf(stdout, "\t%s", "NEG");
                        break;
                    case O_ASN: /* 9006 */
                        fprintf(stdout, "\t%s", "ASN");
                        break;
                    case O_ADDR: /* 9007 */
                        fprintf(stdout, "\t%s", "ADDR");
                        break;
                    case O_LCONT: /* 9008 */
                        fprintf(stdout, "\t%s", "LCONT");
                        break;
                    case O_SCONT: /* 9009 */
                        fprintf(stdout, "\t%s", "SCONT");
                        break;
                    case O_GOTO: /* 9010 */
                        fprintf(stdout, "\t%s", "GOTO");
                        break;
                    case O_BLT: /* 9011 */
                        fprintf(stdout, "\t%s", "BLT");
                        break;
                    case O_BLE: /* 9012 */
                        fprintf(stdout, "\t%s", "BLE");
                        break;
                    case O_BGT: /* 9013 */
                        fprintf(stdout, "\t%s", "BGT");
                        break;
                    case O_BGE: /* 9014 */
                        fprintf(stdout, "\t%s", "BGE");
                        break;
                    case O_BEQ: /* 9015 */
                        fprintf(stdout, "\t%s", "BEQ");
                        break;
                    case O_BNE: /* 9016 */
                        fprintf(stdout, "\t%s", "BNE");
                        break;
                    case O_BIF: /* 9017 */
                        fprintf(stdout, "\t%s", "BIF");
                        break;
                    case O_BNIF: /* 9018 */
                        fprintf(stdout, "\t%s", "BNIF");
                        break;
                    case O_PARM: /* 9019 */
                        fprintf(stdout, "\t%s", "PARM");
                        break;
                    case O_CALL: /* 9020 */
                        fprintf(stdout, "\t%s", "CALL");
                        break;
                    case O_RET: /* 9021 */
                        fprintf(stdout, "\t%s", "RET");
                        break;
                    /* declarations/pseudo instructions */
                    case D_GLOB: /* 9051 */
                        fprintf(stdout, "%s", "GLOB");
                        break;
                    case D_PROC: /* 9052 */
                        fprintf(stdout, "%s", "PROC");
                        break;
                    case D_LOCAL: /* 9053 */
                        fprintf(stdout, "%s", "LOCAL");
                        break;
                    case D_LABEL: /* 9054 */
                        fprintf(stdout, "%s", "LABEL");
                        break;
                    case D_END: /* 9055 */
                        fprintf(stdout, "%s", "END");
                        break;
                }
            }
            switch(t->instruct->address[i]->region)
            {
                case R_GLOBAL: /* 8001 */
                    fprintf(stdout, ",\t%s:", "global");
                    break;
                case R_LOCAL: /*  8002 */
                    fprintf(stdout, ",\t%s:", "local");
                    break;
                case R_CLASS: /*  8003 */
                    fprintf(stdout, ",\t%s:", "class");
                    break;
                case R_LABEL: /*  8004 */
                    fprintf(stdout, ",\t%s:", "label");
                    break;
                case R_CONST: /*  8005 */
                    fprintf(stdout, ",\t%s:", "const");
                    break;
                case R_CODE: /*   8006 */
                    fprintf(stdout, "\t%s:", "code");
                    break;
                case R_SCONST: /* 8007 */
                    fprintf(stdout, ",\t%s:", "sconst");
                    break;
            }
            switch(t->instruct->address[i]->region)
            {
                case R_CODE:
                case R_LABEL:
                case R_CLASS:
                    if(t->instruct->address[i]->u.name != NULL)
                    {
                        fprintf(stdout, "%s", t->instruct->address[i]->u.name);
                    }
                    break;
                default:
                    if(t->instruct->address[i]->region > 0)
                    {
                        fprintf(stdout, "%d", t->instruct->address[i]->u.offset);
                    }
                    break;
            }
            if(t->instruct->op == D_PROC && i > 0)
            {
                fprintf(stdout, ",\t%d", t->instruct->address[i]->size);
            }
            if(t->instruct->op == O_CALL && i == 1)
            {
                fprintf(stdout, ",\t%d", t->instruct->address[i]->size);
            }
        }
        fprintf(stdout, "\n");
        t = t->next;
    }
}

void point_parent(parsetree *kid_node, parsetree *parent_node)
{ /* pre-oprder traversal to fill in parent node pointers */
    int i;
    kid_node->parent = parent_node;
    for(i = 0; i < kid_node->nkids; i++)
    {
        point_parent(kid_node->kids[i], kid_node);   
    }  
}

void fill_const(parsetree *n, struct sym_table *st)
{ /* post order traversal to fill const values */
    int i;
    struct sym_table *nextst;
    struct typeinfo *tmp;
    if(n == NULL)
    {
        return;   
    }
    nextst = st;
    if(n->typ != NULL && n->typ->st != NULL)
    {
        nextst = n->typ->st;   
    }
    for(i = 0; i < n->nkids; i++)
    {
        fill_const(n->kids[i], nextst);   
    }
    switch(n->prodrule)
    {
        case LEXICAL_INTLIT: /*1121 */
            n->typ->memaddr.region = R_CONST;
            n->typ->memaddr.u.offset = n->leaf->ival;
            n->typ->memaddr.size = 8;
            n->final = n->typ->memaddr;
            break;
        case LEXICAL_CHARLIT: /* 1131 */
            n->typ->memaddr.region = R_SCONST;
            n->typ->memaddr.size = 1; /* size = 1 in R_SCONST indicates char */
            n->typ->memaddr.u.offset = SconstOffset;
            SconstOffset += n->typ->memaddr.size;
            n->final = n->typ->memaddr;
            break;
        case LEXICAL_FLOATLIT: /* 1141 */
            n->typ->memaddr.region = R_CONST;
            n->typ->memaddr.u.foffset = atof(n->leaf->text);
            n->typ->memaddr.size = 8;
            n->final = n->typ->memaddr;
            break;
        case LEXICAL_STRINGLIT: /* 1151 */
            n->typ->memaddr.region = R_SCONST;
            n->typ->memaddr.size = strlen(n->leaf->sval) + 1;
            n->typ->memaddr.u.offset = SconstOffset;
            SconstOffset += n->typ->memaddr.size;
            n->final = n->typ->memaddr;
            break;
        case LEXICAL_BOOLLIT: /* 1161 */
            n->typ->memaddr.region = R_CONST;
            n->typ->memaddr.u.offset = 1;
            n->typ->memaddr.size = 1; /* size 1 in R_CONST indicates bool */
            n->final = n->typ->memaddr;
            break;
        case LEXICAL_BOOLLIT2: /* 1162 */
            n->typ->memaddr.region = R_CONST;
            n->typ->memaddr.u.offset = 0;
            n->typ->memaddr.size = 1;
            n->final = n->typ->memaddr;
            break;
        case LEXICAL_IDENT: /* 1101 */
            tmp = scroll_search(st, n->typ->name);
            if(tmp == NULL)
            {
                tmp = scroll_search(nextst, n->typ->name);
            }
            n->final = tmp->memaddr;
            break;
    }
}

struct sym_list *mem_loc(struct sym_table *stbeg)
{ /* load loc regions/offset & size for all sym tab entries */
    int i, inlist;
    struct sym_list *write;
    struct sym_list *read;  
    struct sym_list *tmp;
    struct sym_entry *entry;
    struct sym_table *st;
    write = new_symlist(stbeg); /* start with given sym tab */
    read = write;
    symtablist_begin = write;
    while(read != NULL)
    {
        for(i = 0; i < SYMHASHSIZE; i++)
        {
            st = read->curr;
            entry = st[i].head;
            while(entry != NULL)
            {
                if(entry->t->memaddr.region == 0)
                { /* we haven't processed this yet */
                    inlist = set_memsize(entry->t, st);
                }
                if(entry->t->st != st && entry->t->st != NULL)
                {
                    tmp = symtablist_begin;
                    inlist = 0;
                    while(tmp != NULL)
                    {  
                        if(tmp->curr == entry->t->st)
                        {
                            inlist++;
                            break; /* already in list */   
                        }
                        tmp = tmp->next;
                    }
                    if(!inlist)
                    { /* add to list */
                        write = add_symlist(write, entry->t->st);
                    }
                }
                entry = entry->next;   
            }
        }
        read = read->next;
    }
    return write;
}

char *is_classmem(struct typeinfo *t, struct sym_table *st)
{
    struct typeinfo *tmp;
    struct typeinfo *tmpcl;
    struct classmember *cm;
    char *cn = 0;
    tmp = scroll_search(st, t->name); /* look for my definition */
    /* look for a class that "owns" tmp->i_am_defined_here */
    tmpcl = find_my_class(t);
    if(tmpcl != NULL)
    { /* Check to make sure I'm a member */
        cm = tmpcl->u.c.member;
        while(cm != NULL)
        {
            if(strcmp(cm->type->name, tmp->name) == 0)
            { /* we found this! */
                cn = copy_name(tmpcl->name);
                break;
            }
            cm = cm->next;
        }
    }
    return cn;
}

struct typeinfo *find_my_class(struct typeinfo *t)
{
    struct typeinfo *rett = 0;
    int i;
    struct sym_table *st;
    struct sym_entry *entry;
    st = t->i_am_defined_here;
    while(st->parent != NULL)
    {
        st = st->parent;
        for(i = 0; i < SYMHASHSIZE; i++)
        {
            entry = st[i].head;
            while(entry != NULL)
            {
                if(entry->basetype == CLASS_TYPE)
                { /* Check to see if it owns the space */
                    if(entry->t->st != NULL && entry->t->st == t->i_am_defined_here)
                    { /* we're done */
                        rett = entry->t;
                        return rett;
                    }
                }
                entry = entry->next;   
            }
        } 
    }
    return rett;
}

int set_memsize(struct typeinfo *t, struct sym_table *st)
{ /* set memory offset/region and byte size if possible (e.g. can't set mem loc for 
   * classes that don't have an instance) */
   
    /* Param offset should be negative since the space is allocated from the
     * calling region. The function typeinfo has total param space 
     * specified & the isParam member on the parameter typeinfo gives
     * the order number of the param. */
    struct typeinfo *tfound;
    char* namemangle;
    int classpointer = 0;
    switch(t->basetype)
    {
        case INT_TYPE:       /* 0 int */
        case FLOAT_TYPE:     /* 2 float, double */
        case BOOL_TYPE:      /* 1 bool */
        case CHAR_TYPE:      /* 3 char */
        case PTR_TYPE:       /* 7 */
            t->memaddr.size = align_size(size_up_simple(t->basetype));
            t->myclass = is_classmem(t, st);
            break;
        case CLASS_TYPE:     /* 5 */
            if(t->isProto == 1) 
            { /* prototype, calc up size for later use on instances */
                t->memaddr.size = size_up_class(t);
                t->memaddr.region = R_CLASS;
            }
            break;
        case ARR_TYPE:       /* 8 */
            t->memaddr.size = align_size(size_up_arr(t));
            t->myclass = is_classmem(t, st);
            break;
        case FUNC_TYPE:      /* 9 */
            namemangle = name_mangle(t); /* check to see if we need to name mangle it for class stuff */
            if(namemangle != NULL)
            { /* this is 8 so we can pass the this point as the first param */
                classpointer = 8;   
            }
            if(t->u.f.returntype->basetype == CLASS_TYPE 
            && (t->u.f.returntype->name == NULL || strcmp(t->u.f.classname, t->u.f.returntype->name) != 0))
            { /* class instance, 120++ doesn't have functions that return classes? */
                t->memaddr.region = R_CLASS;
                /* find class info, all classes defined globally */
                tfound = scroll_search(symtab, t->u.f.classname);
                t->memaddr.size = tfound->memaddr.size;
                /* now add 8 bytes for this pointer */
                t->memaddr.size += 8;
            }
            else
            {
                t->memaddr.region = R_CODE;
                t->memaddr.size = size_up_func(t, classpointer);
            }
            if(namemangle != NULL)
            {
                t->memaddr.u.name = copy_name(namemangle);
            }
            else
            {
                t->memaddr.u.name = copy_name(t->name);   
            }
            break;
    }
    if(t->memaddr.region == 0) 
    { /* if we haven't already set the region, do so now */
        if(st == symtab)
        { /* global region */
            t->memaddr.region = R_GLOBAL;
        }
        else
        {
            t->memaddr.region = R_LOCAL;
        }
    }
    if(t->basetype != CLASS_TYPE)
    { /* manage offsets */
        if(t->memaddr.u.name == NULL)
        { /* only do if the name hasn't been used as an offset */
            if(st == symtab)
            {
                t->memaddr.u.offset = GlobalOffset;
                GlobalOffset += t->memaddr.size;
            }
            else
            {
                t->memaddr.u.offset = st->offset;   
            }
            st->offset += t->memaddr.size; 
        }
        t->st->offset += classpointer; /* if this is a class instance... add 8 to the sym tab for this pointer */
    }
    return t->memaddr.size;
}

int align_size(int size)
{
    while((size % 8) != 0)
    {
        size++;   
    }  
    return size;  
}

int size_up_class(struct typeinfo *t)
{ /* scroll thru the class members and only add up members, i.e. don't add functions */
    int size = 0;
    struct classmember *cm;
    struct typeinfo *ct;
    cm = t->u.c.member;
    while(cm != NULL)
    {
        ct = cm->type;
        switch(ct->basetype)
        {
            case INT_TYPE:       /* 0 int */
            case FLOAT_TYPE:     /* 2 float, double */
            case BOOL_TYPE:      /* 1 bool */
            case CHAR_TYPE:      /* 3 char */
            case PTR_TYPE:       /* 7 */
                size += align_size(size_up_simple(ct->basetype));
                break;
            case ARR_TYPE:       /* 8 */
                size += align_size(size_up_arr(ct));            
                break;
        }
        cm = cm->next;   
    }
    return size;
}

int size_up_simple(enum base_type bt)
{   int size = 0;
    switch(bt)
    {
        case INT_TYPE:
        case FLOAT_TYPE:
        case PTR_TYPE: 
            size = 8;
            break;
        case CHAR_TYPE:
        case BOOL_TYPE:
            size = 1;
            break;
    }
    return size;
}

int size_up_arr(struct typeinfo *t)
{ 
    int mysize = 0;
    if(t == NULL)
    {
        return mysize;   
    }
    switch(t->u.a.elemtype->basetype)
    {
        case INT_TYPE:
        case FLOAT_TYPE:
        case CHAR_TYPE:
        case BOOL_TYPE:
        case PTR_TYPE:      
            mysize = size_up_simple(t->u.a.elemtype->basetype);
            break;
        case ARR_TYPE:
            mysize = size_up_arr(t->u.a.elemtype);
            break;
        case FUNC_TYPE: //stores param sizes
            //!!! error out
            break;
        case CLASS_TYPE:
            // !! error out
            break;
    }
    mysize *= t->u.a.size;
    return mysize;
}

int size_up_func(struct typeinfo *t, int cp)
{ 
    /* t->basetype == FUNC_TYPE
     * t->st is where my variables are defined
     * t->i_am_defined_here is where I am defined
     * t->u.f.parameters is my linked list of parameters
     *
     * The function definition has it's own sym tab that will be added up
     * as well. This procedure adds up param size so we know how much
     * of the function sym tab space belongs to the params. 
     * The total params size will be a pos number stored in:
     *  t->u.f.paramsize
     * The individual param size will be a pos number stored in:
     *  t->u.f.parameters->type->memaddr->size
     * The individual param offset will be a neg number stored in:
     *  t->u.f.parameters->type->memaddr->offset
     * The total function size will be returned.
     *
     * If the function is called within another function, e.g.
     *   main { foo(parameter); }
     * then the function size does not add into the calling function size.
     */
     
    struct param *p;
    int mysize, i;
    int totsize = cp;
    int finalsize = cp;
    struct typeinfo *ap[SYMHASHSIZE]; /* to be used for sym tab params */
    struct sym_entry *entry;
    struct sym_table *st;
    struct typeinfo *tfound;
    
    p = t->u.f.parameters;
    while(p != NULL)
    {   
        if(p->type->basetype == NULL_TYPE)
        {
            break;
        }
        mysize = 0;
        switch(p->type->basetype)
        {
            case INT_TYPE:
            case FLOAT_TYPE:
            case CHAR_TYPE:
            case BOOL_TYPE:
            case PTR_TYPE:      
                mysize = align_size(size_up_simple(p->type->basetype));
                break;
            case ARR_TYPE:
                mysize = align_size(size_up_arr(p->type));
                break;
            case FUNC_TYPE:
                mysize = size_up_func(p->type, cp);
                break;
        }
        p->type->memaddr.size = mysize;
        totsize += mysize;
        p->type->memaddr.u.offset = -1 * totsize;
        p = p->next;
    }
    t->u.f.paramsize = totsize; /* will be used to allocate stack space from calling function */
    /* now look through function's local table and size stuff up so we can get parameter's done correctly */
    if(t->st != t->i_am_defined_here)
    {
        st = t->st;
        for(i = 0; i < SYMHASHSIZE; i++)
        {
            ap[i] = 0; /* null out pointer array */   
        }
        for(i = 0; i < SYMHASHSIZE; i++)
        {
            entry = st[i].head;
            while(entry != NULL)
            { /* scopes are not allowed inside this scope, so no need to scope check */
                if(entry->t->isParam > 0)
                { /* will process last */
                    ap[entry->t->isParam] = entry->t;   
                }
                else
                { /* process other declarations */
                    if(entry->t->memaddr.region == 0)
                    { /* we haven't processed this yet */
                        finalsize += set_memsize(entry->t, st);
                    }
                    else
                    { /* just add in size */
                        finalsize += entry->t->memaddr.size;
                    }
                }
                entry = entry->next;   
            }
        }
        totsize = cp;
        for(i = 1; i < SYMHASHSIZE; i++)
        {
            mysize = 0;
            if(ap[i] != NULL)
            {
                switch(ap[i]->basetype)
                {
                    case INT_TYPE:
                    case FLOAT_TYPE:
                    case CHAR_TYPE:
                    case BOOL_TYPE:
                    case PTR_TYPE:      
                        mysize = align_size(size_up_simple(ap[i]->basetype));
                        break;
                    case ARR_TYPE:
                        mysize = align_size(size_up_arr(ap[i]));
                        break;
                }
                ap[i]->memaddr.size = mysize;
                totsize += mysize;
                ap[i]->memaddr.u.offset = -1 * totsize;
                ap[i]->memaddr.region = R_LOCAL;
                finalsize += mysize;
            }
        }
    }
    else
    { /* class instance? */
        if(t->u.f.classname != NULL)
        {
            tfound = scroll_search(symtab, t->u.f.classname);
            finalsize = tfound->memaddr.size + cp;
        }
    }
    return finalsize;
}

char *name_mangle(struct typeinfo *t)
{
    char *newname = 0;
    int size;
    if(t->u.f.nameok)
    {
        return newname;
    }
    if(t->u.f.classname != NULL)
    { /* create new class name */
      size = strlen(t->u.f.classname) + strlen("__") + strlen(t->name) + 1;
      newname = (char *)calloc(size, sizeof(char));
      newname = strcpy(newname, t->u.f.classname);
      newname = strcat(newname, "__");
      newname = strcat(newname, t->name);
    }
    t->u.f.nameok++;
    return newname;
}

struct sym_list *add_symlist(struct sym_list *sl, struct sym_table *st)
{
    struct sym_list * tmp;
    tmp = new_symlist(st);
    sl->next = tmp;
    return tmp;
}

struct sym_list *new_symlist(struct sym_table *st)
{
    struct sym_list *tmp;
    tmp = (struct sym_list *)calloc(1, sizeof(struct sym_list));
    if(tmp == NULL)
    {
        fprintf(stderr, "ERROR: out of memory\n"); 
        exit(10);       
    }   
    tmp->curr = st;
    return tmp;
}

struct typeinfo *new_const(int ival, float fval, enum base_type bt)
{
    struct typeinfo *tmp;
    tmp = (struct typeinfo *)calloc(1, sizeof(struct typeinfo));
    if(tmp == NULL)
    {
        fprintf(stderr, "ERROR: out of memory\n"); 
        exit(10);       
    }
    tmp->memaddr.size = size_up_simple(bt);
    tmp->memaddr.region = R_CONST;
    switch(bt)
    {
        case INT_TYPE:      
        case BOOL_TYPE:
            tmp->memaddr.u.offset = ival;
            break;
        case FLOAT_TYPE:
            tmp->memaddr.u.offset = fval;
            break;
    }
    return tmp;    
}

struct typeinfo *newvar(enum base_type bt, struct sym_table *st)
{ /* we need a new temp variable so let's make one */
    struct typeinfo *tmp;
    char *tmp_name;
    tmp_name = (char *)calloc(10,sizeof(char)); /* allow for (99,999,999 + 1) temps */
    if(tmp_name == NULL)
    {
        fprintf(stderr, "ERROR: out of memory\n"); 
        exit(10);       
    }
    strcpy(tmp_name, "t");
    sprintf(&tmp_name[1], "%d", TempCount);
    TempCount++;
    tmp = make_room(bt);
    tmp->name = tmp_name;
    tmp->i_am_defined_here = st;
    tmp->memaddr.size = align_size(size_up_simple(bt));
    if(st == symtab)
    {
        tmp->memaddr.region = R_GLOBAL;
        tmp->memaddr.u.offset = GlobalOffset;
        GlobalOffset += tmp->memaddr.size;
    }
    else
    {
        tmp->memaddr.region = R_LOCAL;
        tmp->memaddr.u.offset = st->offset;
    }
    
    st->offset += tmp->memaddr.size;
    sym_insert(st, tmp);
    st->count++;
    return tmp;
}

struct tac *make_tac()
{ /* make a new space for a tac instruction */
    struct tac *tmp;
    int i;
    tmp = (struct tac *)calloc(1, sizeof(struct tac));
    if(tmp == NULL)
    {
        fprintf(stderr, "ERROR: out of memory\n"); 
        exit(10);       
    }   
    for(i = 0; i < 3; i++)
    {
        tmp->address[i] = (struct addr *)calloc(1, sizeof(struct addr));
        if(tmp->address[i] == NULL)
        {
            fprintf(stderr, "ERROR: out of memory\n"); 
            exit(10);       
        }   
    }
    return tmp;
}

struct tac *newlabel()
{ /* create a new tac instruction space and fill with new label */
    struct tac *tmp;
    char *tmp_name;
    tmp = make_tac();
    tmp->op = D_LABEL;
    tmp->address[0]->region = R_LABEL;
    tmp_name = (char *)calloc(10,sizeof(char)); /* allow for (999,999 + 1) labels */
    if(tmp_name == NULL)
    {
        fprintf(stderr, "ERROR: out of memory\n"); 
        exit(10);       
    }
    strcpy(tmp_name, "lab");
    sprintf(&tmp_name[3], "%d", LabelCount);
    tmp->address[0]->u.name = copy_name(tmp_name);
    LabelCount++;
    return tmp;
}

struct taclist *add_taclist(struct tac *t, struct taclist *tl)
{ /* add a tac instruction to the end of the tac instruction list */
    struct taclist *tmp = 0;
    if(t != NULL)
    {
	    tmp = new_listmem();
	    if(tl != NULL)
	    {
		    while(tl->next != NULL)
		    { /* proceed to the end of the list */
		        tl = tl->next;
		    }
		    tl->next = tmp;
		    tmp->prev = tl;
		}
	    /* now add tac to taclist */
	    tmp->instruct = t;
    }
    else
    {
	 	tmp = tl;   
    }
    return tmp;
}

struct taclist *new_listmem(void)
{ /* create a new node for tac instruction list */
    struct taclist *tmp;
    tmp = (struct taclist *)calloc(1, sizeof(struct taclist));
    if(tmp == NULL)
    {
        fprintf(stderr, "ERROR: out of memory\n"); 
        exit(10);       
    }   
    return tmp;
}

struct tac *copy_tac(struct tac *t)
{ /* creates a separate copy of the tac... non-destructive */
    struct tac *tmp;
    tmp = make_tac();
    tmp->op = t->op;
    tmp->address[0]->region = t->address[0]->region;
    tmp->address[0]->u = t->address[0]->u;
    tmp->address[0]->size = t->address[0]->size;
    tmp->address[1]->region = t->address[1]->region;
    tmp->address[1]->u = t->address[1]->u;
    tmp->address[1]->size = t->address[1]->size;
    tmp->address[2]->region = t->address[2]->region;
    tmp->address[2]->u = t->address[2]->u;
    tmp->address[2]->size = t->address[2]->size;
    return tmp;
}

struct taclist *concat(struct taclist *t1, struct taclist *t2)
{ /* non distructive concatination */
    struct taclist *newlist;
    struct taclist *tmplist;
    if(t1 == NULL && t2 == NULL)
    {
        return NULL;   
    }
    if(t1 == NULL)
    {
        return copy_list(t2);
    }
    if(t2 == NULL)
    {
        return copy_list(t1);
    }
    newlist = copy_list(t1); /* non-destructive concat */
    tmplist = copy_list(t2);
    while(newlist->next != NULL)
    { /* go to end of list 1 */
        newlist = newlist->next;
    }
    while(tmplist->prev != NULL)
    { /* go to beginning of list 2 */
        tmplist = tmplist->prev;
    }
    newlist->next = tmplist;
    tmplist->prev = newlist;
    return newlist;
}

struct taclist *copy_list(struct taclist *tl)
{ /* creates a separate copy of the list... non-destructive for list concatenation */
    struct taclist *tmplist;
    struct taclist *newlist = 0;
    while(tl->prev != NULL)
    { /* go to head of list */
        tl = tl->prev;   
    }
    if(tl != NULL)
    { /* start new list */
        tmplist = new_listmem();
        tmplist->instruct = copy_tac(tl->instruct);
        newlist = tmplist; /* in case there is only one member */
        tl = tl->next;
    }
    while(tl != NULL)
    { /* start copying */
        newlist = new_listmem();
        newlist->instruct = copy_tac(tl->instruct);
        tmplist->next = newlist;
        newlist->prev = tmplist;
        tmplist = newlist;
        tl = tl->next;
    }
    return newlist;
}
