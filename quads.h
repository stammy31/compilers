#ifndef __QUADS_H__
#define __QUADS_H__

#include "symbol.h"
#define MAX_QUAD_NUM 1000

typedef enum {               /***********/
	PLUS_QUAD,               /*    +    */
	MINUS_QUAD,              /*    -    */
	MULT_QUAD,               /*    *    */
	DIV_QUAD,                /*    /    */
	MOD_QUAD,                /*    %    */
	EQ_QUAD,                 /*    =    */
	NE_QUAD,                 /*    <>   */
	GT_QUAD,                 /*    >    */
	LT_QUAD,                 /*    <    */
	GE_QUAD,                 /*    >=   */
	LE_QUAD,                 /*    <=   */
	ASSIGN_QUAD,             /*    :=   */
	JMP_QUAD,                /*   jump  */
	RET_QUAD,                /*   ret   */
    RETV_QUAD,               /*   retv  */
	ARRAY_QUAD,              /*   array */
    LIST_QUAD,               /*   list  */
    HEAD_QUAD,               /*   head  */
    TAIL_QUAD,               /*   tail  */
    ISNIL_QUAD,              /*   nil?  */
	UNIT_QUAD,               /*   unit  */
	ENDU_QUAD,               /*   endu  */
	PAR_QUAD,                /*   par   */
	CALL_QUAD,               /*   call  */
	IFB_QUAD,                /*   ifb   */
	PTR_QUAD,                /* ^points */
} QuadType;                  /***********/

typedef struct
{
	char * pm;
	char * type;
	char * nesting;
	char * kind;
	char * offset;
} Quad_req;

typedef struct {                 /*********/
	QuadType type;           /*  op   */
	char *arg1, *arg2;       /* x, y  */
	char *dest;              /*   z   */
	Quad_req arg1_req, arg2_req;
	Quad_req dest_req;
} Quad;                          /*********/

typedef struct label_list_struct {
	long label;
	struct label_list_struct * next;
} label_list_t;

typedef label_list_t * label_list;

/* -------------------------------------------------------------------------------------------------------
   ------------------------ Οι συναρτήσεις μας -----------------------------------------------------------
   ------------------------------------------------------------------------------------------------------- */

extern Quad quad_array[MAX_QUAD_NUM];            /* an array storing quads                                 */

extern long nextquad;                            /* returns next's quad label                              */


long GenQuad(QuadType q, SymbolEntry * x,        /* generates next quad                                    */
	SymbolEntry * y, SymbolEntry * z);
label_list emptylist();                          /* generates an empty quad labels' list                   */
label_list make_list(long val);                  /* generates quad label's list containing label: val      */
label_list merge(label_list a, label_list b);    /* merges two quad labels' lists                          */
void backpatch(label_list list, long val);       /* replace unknown labels of quads of list l with label z */

char * print_quad(int i);                        /* printable version of quad type                         */
/*void FreeQuads(void);                             Should we use it ?!? */
char *strdup(const char *str);
void print_all_quads();                          /* prints all quad_array                                  */
char * outp(char *inp);
long GenQuad2(QuadType q, SymbolEntry * x, SymbolEntry * y, char * z);
long GenQuad3(QuadType q, char * x, char * y, char * z);
long GenQuad4(QuadType q, SymbolEntry * x, char * y, char * z);
char * symbol_type (SymbolEntry * p);
char * symbol_pm (SymbolEntry * p);
char * symbol_kind (SymbolEntry * p);
char * symbol_offset (SymbolEntry * p);

#endif
