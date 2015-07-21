/*
** This source file is part of MY-BASIC
**
** For the latest info, see https://github.com/paladin-t/my_basic/
**
** Copyright (C) 2011 - 2015 W. Renxin
**
** Permission is hereby granted, free of charge, to any person obtaining a copy of
** this software and associated documentation files (the "Software"), to deal in
** the Software without restriction, including without limitation the rights to
** use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
** the Software, and to permit persons to whom the Software is furnished to do so,
** subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
** FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
** COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
** IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifdef _MSC_VER
#	ifndef _CRT_SECURE_NO_WARNINGS
#		define _CRT_SECURE_NO_WARNINGS
#	endif /* _CRT_SECURE_NO_WARNINGS */
#endif /* _MSC_VER */

#include "my_basic.h"
#ifdef _MSC_VER
#	include <conio.h>
#	include <malloc.h>
#else /* _MSC_VER */
#	include <stdint.h>
#endif /* _MSC_VER */
#include <memory.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef _MSC_VER
#	pragma warning(push)
#	pragma warning(disable : 4127)
#	pragma warning(disable : 4305)
#endif /* _MSC_VER */

#ifdef __APPLE__
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wunused-function"
#	pragma clang diagnostic ignored "-Wunused-variable"
#endif /* __APPLE__ */

#ifdef __BORLANDC__
#	pragma warn -8004
#	pragma warn -8008
#	pragma warn -8012
#endif /* __BORLANDC__ */

#ifdef MB_COMPACT_MODE
#	pragma pack(1)
#endif /* MB_COMPACT_MODE */

/*
** {========================================================
** Data type declarations
*/

/** Macros */
#define _VER_MAJOR 1
#define _VER_MINOR 1
#define _VER_REVISION 59
#define _MB_VERSION ((_VER_MAJOR * 0x01000000) + (_VER_MINOR * 0x00010000) + (_VER_REVISION))

/* Uncomment this line to treat warnings as error */
/*#define _WARING_AS_ERROR*/

/* Uncomment this line to use a comma to PRINT a new line as compatibility */
/*#define _COMMA_AS_NEWLINE*/

#define _NO_EAT_COMMA 2

/* Helper */
#ifndef sgn
#	define sgn(__v) ((__v) ? ((__v) > 0 ? 1 : -1) : (0))
#endif /* sgn */

#ifndef _countof
#	define _countof(__a) (sizeof(__a) / sizeof(*(__a)))
#endif /* _countof */

#ifndef islower
#	define islower(__c) ((__c) >= 'a' && (__c) <= 'z')
#endif /* islower */

#ifndef toupper
#	define toupper(__c) ((islower(__c)) ? ((__c) - 'a' + 'A') : (__c))
#endif /* toupper */

#define DON(__o) ((__o) ? ((_object_t*)((__o)->data)) : 0)

#define _IS_EOS(__o) (__o && ((_object_t*)(__o))->type == _DT_EOS)
#define _IS_SEP(__o, __c) (((_object_t*)(__o))->type == _DT_SEP && ((_object_t*)(__o))->data.separator == __c)
#define _IS_FUNC(__o, __f) (((_object_t*)(__o))->type == _DT_FUNC && ((_object_t*)(__o))->data.func->pointer == __f)

/* Hash table size */
#define _HT_ARRAY_SIZE_SMALL 193
#define _HT_ARRAY_SIZE_MID 1543
#define _HT_ARRAY_SIZE_BIG 12289
#define _HT_ARRAY_SIZE_DEFAULT _HT_ARRAY_SIZE_SMALL

/* Max length of a single symbol */
#define _SINGLE_SYMBOL_MAX_LENGTH 128

typedef int (* _common_compare)(void*, void*);

/* Container operation */
#define _OP_RESULT_NORMAL 0
#define _OP_RESULT_DEL_NODE -1
typedef int (* _common_operation)(void*, void*);

/** List */
typedef _common_compare _ls_compare;
typedef _common_operation _ls_operation;

typedef struct _ls_node_t {
	void* data;
	struct _ls_node_t* prev;
	struct _ls_node_t* next;
	void* extra;
} _ls_node_t;

/** Dictionary */
typedef unsigned int (* _ht_hash)(void*, void*);
typedef _common_compare _ht_compare;
typedef _common_operation _ht_operation;

typedef struct _ht_node_t {
	_ls_operation free_extra;
	_ht_compare compare;
	_ht_hash hash;
	unsigned int array_size;
	unsigned int count;
	_ls_node_t** array;
} _ht_node_t;

/** enum / struct / union / const */
/* Error description text */
static const char* _ERR_DESC[] = {
	"No error",
	/** Common */
	"Open MY-BASIC failed",
	"A function with the same name already exists",
	"A function with the name does not exists",
	/** Parsing */
	"Open file failed",
	"Symbol too long",
	"Invalid character",
	/** Running */
	"Not supported",
	"Empty program",
	"Syntax error",
	"Invalid data type",
	"Type does not match",
	"Illegal bound",
	"Too much dimensions",
	"Operation failed",
	"Dimension count out of bound",
	"Out of bound",
	"Label does not exist",
	"No return point",
	"Colon expected",
	"Comma or semicolon expected",
	"Array identifier expected",
	"Open bracket expected",
	"Close bracket expected",
	"Array subscript expected",
	"Structure not completed",
	"Function expected",
	"Variable or array identifier expected",
	"Assign operator expected",
	"String expected",
	"Number expected",
	"Integer expected",
	"ELSE statement expected",
	"TO statement expected",
	"NEXT statement expected",
	"UNTIL statement expected",
	"Loop variable expected",
	"Jump label expected",
	"Variable expected",
	"Invalid identifier usage",
	"Operator expected",
	"Calculation error",
	"Divide by zero",
	"MOD by zero",
	"Invalid expression",
	"Out of memory",
	/** Extended abort */
	"Extended abort"
};

mb_static_assert(_countof(_ERR_DESC) == SE_COUNT);

/* Data type */
typedef enum _data_e {
	_DT_NIL = -1,
	_DT_ANY = 0,
	_DT_INT,
	_DT_REAL,
	_DT_STRING,
	_DT_USERTYPE,
	_DT_FUNC,
	_DT_VAR,
	_DT_ARRAY,
	_DT_LABEL, /* Label type, used for GOTO, GOSUB statement */
	_DT_SEP, /* Separator */
	_DT_EOS /* End of statement */
} _data_e;

typedef struct _func_t {
	char* name;
	mb_func_t pointer;
} _func_t;

typedef struct _var_t {
	char* name;
	struct _object_t* data;
} _var_t;

typedef struct _array_t {
	char* name;
	_data_e type;
#ifndef MB_SIMPLE_ARRAY
	_data_e* types;
#endif /* MB_SIMPLE_ARRAY */
	void* raw;
	unsigned int count;
	int dimension_count;
	int dimensions[MB_MAX_DIMENSION_COUNT];
} _array_t;

typedef struct _label_t {
	char* name;
	_ls_node_t* node;
} _label_t;

typedef union _raw_u { int_t i; real_t r; void* p; } _raw_u;

typedef unsigned char _raw_t[sizeof(_raw_u)];

typedef struct _object_t {
	_data_e type;
	union {
		int_t integer;
		real_t float_point;
		char* string;
		void* usertype;
		_func_t* func;
		_var_t* variable;
		_array_t* array;
		_label_t* label;
		char separator;
		_raw_t raw;
	} data;
	bool_t ref;
#ifdef MB_ENABLE_SOURCE_TRACE
	int source_pos;
	unsigned short source_row;
	unsigned short source_col;
#else /* MB_ENABLE_SOURCE_TRACE */
	char source_pos;
#endif /* MB_ENABLE_SOURCE_TRACE */
} _object_t;

static const _object_t _OBJ_INT_UNIT = { _DT_INT, 1, false, 0 };
static const _object_t _OBJ_INT_ZERO = { _DT_INT, 0, false, 0 };

static _object_t* _OBJ_BOOL_TRUE = 0;
static _object_t* _OBJ_BOOL_FALSE = 0;

/* Parsing context */
typedef enum _parsing_state_e {
	_PS_NORMAL = 0,
	_PS_STRING,
	_PS_COMMENT
} _parsing_state_e;

typedef enum _symbol_state_e {
	_SS_IDENTIFIER = 0,
	_SS_OPERATOR
} _symbol_state_e;

typedef struct _parsing_context_t {
	char current_char;
	char current_symbol[_SINGLE_SYMBOL_MAX_LENGTH + 1];
	int current_symbol_nonius;
	_object_t* last_symbol;
	_parsing_state_e parsing_state;
	_symbol_state_e symbol_state;
	int parsing_pos;
	unsigned short parsing_row;
	unsigned short parsing_col;
} _parsing_context_t;

/* Running context */
typedef struct _running_context_t {
	_ls_node_t* temp_values;
	_ls_node_t* suspent_point;
	int schedule_suspend_tag;
	_ls_node_t* sub_stack;
	_var_t* next_loop_var;
	mb_value_t intermediate_value;
	int_t no_eat_comma_mark;
	_ls_node_t* skip_to_eoi;
	_ls_node_t* in_neg_expr;
} _running_context_t;

/* Expression processing */
typedef struct _tuple3_t {
	void* e1;
	void* e2;
	void* e3;
} _tuple3_t;

/* Interpreter tag */
typedef struct mb_interpreter_t {
	_ht_node_t* local_func_dict;
	_ht_node_t* global_func_dict;
	_ht_node_t* global_var_dict;
	_ls_node_t* ast;
	_parsing_context_t* parsing_context;
	_running_context_t* running_context;
	mb_error_e last_error;
	int last_error_pos;
	unsigned short last_error_row;
	unsigned short last_error_col;
	mb_debug_stepped_handler_t debug_stepped_handler;
	mb_error_handler_t error_handler;
	mb_print_func_t printer;
	mb_input_func_t inputer;
	void* userdata;
} mb_interpreter_t;

static const char _PRECEDE_TABLE[19][19] = {
	/* +    -    *    /    MOD  ^    (    )    =    >    <    >=   <=   ==   <>   AND  OR   NOT  NEG */
	{ '>', '>', '<', '<', '<', '<', '<', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>' }, /* + */
	{ '>', '>', '<', '<', '<', '<', '<', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>' }, /* - */
	{ '>', '>', '>', '>', '>', '<', '<', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>' }, /* * */
	{ '>', '>', '>', '>', '>', '<', '<', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>' }, /* / */
	{ '>', '>', '<', '<', '>', '<', '<', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>' }, /* MOD */
	{ '>', '>', '>', '>', '>', '>', '<', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>' }, /* ^ */
	{ '<', '<', '<', '<', '<', '<', '<', '=', ' ', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<' }, /* ( */
	{ '>', '>', '>', '>', '>', '>', ' ', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>' }, /* ) */
	{ '<', '<', '<', '<', '<', '<', '<', ' ', '=', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<' }, /* = */
	{ '<', '<', '<', '<', '<', '<', '<', '>', '>', ' ', ' ', ' ', ' ', ' ', ' ', '>', '>', '>', '>' }, /* > */
	{ '<', '<', '<', '<', '<', '<', '<', '>', '>', ' ', ' ', ' ', ' ', ' ', ' ', '>', '>', '>', '>' }, /* < */
	{ '<', '<', '<', '<', '<', '<', '<', '>', '>', ' ', ' ', ' ', ' ', ' ', ' ', '>', '>', '>', '>' }, /* >= */
	{ '<', '<', '<', '<', '<', '<', '<', '>', '>', ' ', ' ', ' ', ' ', ' ', ' ', '>', '>', '>', '>' }, /* <= */
	{ '<', '<', '<', '<', '<', '<', '<', '>', '>', ' ', ' ', ' ', ' ', ' ', ' ', '>', '>', '>', '>' }, /* == */
	{ '<', '<', '<', '<', '<', '<', '<', '>', '>', ' ', ' ', ' ', ' ', ' ', ' ', '>', '>', '>', '>' }, /* <> */
	{ '<', '<', '<', '<', '<', '<', '<', '>', '>', '<', '<', '<', '<', '<', '<', '>', '>', '<', '>' }, /* AND */
	{ '<', '<', '<', '<', '<', '<', '<', '>', '>', '<', '<', '<', '<', '<', '<', '>', '>', '<', '>' }, /* OR */
	{ '<', '<', '<', '<', '<', '<', '<', '>', '>', '<', '<', '<', '<', '<', '<', '>', '>', '>', '>' }, /* NOT */
	{ '<', '<', '<', '<', '<', '<', '<', '>', '>', '<', '<', '<', '<', '<', '<', '<', '<', '<', '=' }  /* NEG */
};

static _object_t* _exp_assign = 0;

#define _set_real_with_hex(__r, __i) \
	do { \
		if(sizeof(__r) == sizeof(unsigned char)) { \
			unsigned char __b = __i; \
			memcpy(&__r, &__b, sizeof(__r)); \
		} else if(sizeof(__r) == sizeof(unsigned short)) { \
			unsigned short __b = __i; \
			memcpy(&__r, &__b, sizeof(__r)); \
		} else if(sizeof(__r) == sizeof(unsigned)) { \
			unsigned __b = __i; \
			memcpy(&__r, &__b, sizeof(__r)); \
		} else if(sizeof(__r) == sizeof(unsigned long)) { \
			unsigned long __b = __i; \
			memcpy(&__r, &__b, sizeof(__r)); \
		} else if(sizeof(__r) == sizeof(unsigned long long)) { \
			unsigned long long __b = __i; \
			memcpy(&__r, &__b, sizeof(__r)); \
		} else { \
			mb_assert(0 && "Invalid real number precision."); \
		} \
	} while(0)

#define _instruct_common(__tuple) \
	_object_t opndv1; \
	_object_t opndv2; \
	_tuple3_t* tpptr = (_tuple3_t*)(*__tuple); \
	_object_t* opnd1 = (_object_t*)(tpptr->e1); \
	_object_t* opnd2 = (_object_t*)(tpptr->e2); \
	_object_t* val = (_object_t*)(tpptr->e3); \
	opndv1.type = (opnd1->type == _DT_INT || (opnd1->type == _DT_VAR && opnd1->data.variable->data->type == _DT_INT)) ? \
		_DT_INT : _DT_REAL; \
	opndv1.data = opnd1->type == _DT_VAR ? opnd1->data.variable->data->data : opnd1->data; \
	opndv2.type = (opnd2->type == _DT_INT || (opnd2->type == _DT_VAR && opnd2->data.variable->data->type == _DT_INT)) ? \
		_DT_INT : _DT_REAL; \
	opndv2.data = opnd2->type == _DT_VAR ? opnd2->data.variable->data->data : opnd2->data;
#define _instruct_fun_num_num(__optr, __tuple) \
	do { \
		_instruct_common(__tuple) \
		if(opndv1.type == _DT_INT && opndv2.type == _DT_INT) { \
			val->type = _DT_REAL; \
			val->data.float_point = (real_t)__optr((real_t)opndv1.data.integer, (real_t)opndv2.data.integer); \
		} else { \
			val->type = _DT_REAL; \
			val->data.float_point = (real_t)__optr( \
				opndv1.type == _DT_INT ? opndv1.data.integer : opndv1.data.float_point, \
				opndv2.type == _DT_INT ? opndv2.data.integer : opndv2.data.float_point); \
		} \
		if(val->type == _DT_REAL && (real_t)(int_t)val->data.float_point == val->data.float_point) { \
			val->type = _DT_INT; \
			val->data.integer = (int_t)val->data.float_point; \
		} \
	} while(0)
#define _instruct_num_op_num(__optr, __tuple) \
	do { \
		_instruct_common(__tuple) \
		if(opndv1.type == _DT_INT && opndv2.type == _DT_INT) { \
			if((real_t)(opndv1.data.integer __optr opndv2.data.integer) == (real_t)opndv1.data.integer __optr (real_t)opndv2.data.integer) { \
				val->type = _DT_INT; \
				val->data.integer = opndv1.data.integer __optr opndv2.data.integer; \
			} else { \
				val->type = _DT_REAL; \
				val->data.float_point = (real_t)((real_t)opndv1.data.integer __optr (real_t)opndv2.data.integer); \
			} \
		} else { \
			val->type = _DT_REAL; \
			val->data.float_point = (real_t) \
				((opndv1.type == _DT_INT ? opndv1.data.integer : opndv1.data.float_point) __optr \
				(opndv2.type == _DT_INT ? opndv2.data.integer : opndv2.data.float_point)); \
		} \
		if(val->type == _DT_REAL && (real_t)(int_t)val->data.float_point == val->data.float_point) { \
			val->type = _DT_INT; \
			val->data.integer = (int_t)val->data.float_point; \
		} \
	} while(0)
#define _instruct_int_op_int(__optr, __tuple) \
	do { \
		_instruct_common(__tuple) \
		if(opndv1.type == _DT_INT && opndv2.type == _DT_INT) { \
			val->type = _DT_INT; \
			val->data.integer = opndv1.data.integer __optr opndv2.data.integer; \
		} else { \
			val->type = _DT_INT; \
			val->data.integer = \
				((opndv1.type == _DT_INT ? opndv1.data.integer : (int_t)(opndv1.data.float_point)) __optr \
				(opndv2.type == _DT_INT ? opndv2.data.integer : (int_t)(opndv2.data.float_point))); \
		} \
	} while(0)
#define _instruct_connect_strings(__tuple) \
	do { \
		char* _str1 = 0; \
		char* _str2 = 0; \
		_tuple3_t* tpptr = (_tuple3_t*)(*__tuple); \
		_object_t* opnd1 = (_object_t*)(tpptr->e1); \
		_object_t* opnd2 = (_object_t*)(tpptr->e2); \
		_object_t* val = (_object_t*)(tpptr->e3); \
		val->type = _DT_STRING; \
		if(val->data.string) { \
			safe_free(val->data.string); \
		} \
		_str1 = _extract_string(opnd1); \
		_str2 = _extract_string(opnd2); \
		val->data.string = (char*)mb_malloc(strlen(_str1) + strlen(_str2) + 1); \
		memset(val->data.string, 0, strlen(_str1) + strlen(_str2) + 1); \
		strcat(val->data.string, _str1); \
		strcat(val->data.string, _str2); \
	} while(0)
#define _instruct_compare_strings(__optr, __tuple) \
	do { \
		char* _str1 = 0; \
		char* _str2 = 0; \
		_tuple3_t* tpptr = (_tuple3_t*)(*__tuple); \
		_object_t* opnd1 = (_object_t*)(tpptr->e1); \
		_object_t* opnd2 = (_object_t*)(tpptr->e2); \
		_object_t* val = (_object_t*)(tpptr->e3); \
		val->type = _DT_INT; \
		_str1 = _extract_string(opnd1); \
		_str2 = _extract_string(opnd2); \
		val->data.integer = strcmp(_str1, _str2) __optr 0; \
	} while(0)
#define _proc_div_by_zero(__s, __tuple, __exit, __result, __kind) \
	do { \
		_instruct_common(__tuple) \
		if((opndv2.type == _DT_INT && opndv2.data.integer == 0) || (opndv2.type == _DT_REAL && opndv2.data.float_point == 0.0f)) { \
			if((opndv1.type == _DT_INT && opndv1.data.integer == 0) || (opndv1.type == _DT_REAL && opndv1.data.float_point == 0.0f)) { \
				val->type = _DT_REAL; \
				_set_real_with_hex(val->data.float_point, MB_FNAN); \
			} else { \
				val->type = _DT_REAL; \
				_set_real_with_hex(val->data.float_point, MB_FINF); \
			} \
			_handle_error_on_obj((__s), __kind, ((__tuple) && *(__tuple)) ? ((_object_t*)(((_tuple3_t*)(*(__tuple)))->e1)) : 0, MB_FUNC_WARNING, __exit, __result); \
		} \
	} while(0)

#define _set_tuple3_result(__l, __r) \
	do { \
		_object_t* val = (_object_t*)(((_tuple3_t*)(*(__l)))->e3); \
		val->type = _DT_INT; \
		val->data.integer = __r; \
	} while(0)

/* ========================================================} */

/*
** {========================================================
** Private function declarations
*/

/** List */
static int _ls_cmp_data(void* node, void* info);
static int _ls_cmp_extra(void* node, void* info);
static int _ls_cmp_extra_string(void* node, void* info);

static _ls_node_t* _ls_create_node(void* data);
static _ls_node_t* _ls_create(void);
static _ls_node_t* _ls_back(_ls_node_t* node);
static _ls_node_t* _ls_pushback(_ls_node_t* list, void* data);
static void* _ls_popback(_ls_node_t* list);
static unsigned int _ls_try_remove(_ls_node_t* list, void* info, _ls_compare cmp, _ls_operation op);
static unsigned int _ls_foreach(_ls_node_t* list, _ls_operation op);
static bool_t _ls_empty(_ls_node_t* list);
static void _ls_clear(_ls_node_t* list);
static void _ls_destroy(_ls_node_t* list);
static int _ls_free_extra(void* data, void* extra);

#define _LS_FOREACH(L, O, P, E) \
	do { \
		_ls_node_t* __lst = L; \
		int __opresult = _OP_RESULT_NORMAL; \
		_ls_node_t* __tmp = 0; \
		mb_assert(L && O); \
		__lst = __lst->next; \
		while(__lst) { \
			if(P != 0) { \
				P(__lst->data, __lst->extra, E); \
			} \
			__opresult = O(__lst->data, __lst->extra); \
			__tmp = __lst; \
			__lst = __lst->next; \
			if(_OP_RESULT_DEL_NODE == __opresult) { \
				__tmp->prev->next = __lst; \
				if(__lst) { \
					__lst->prev = __tmp->prev; \
				} \
				safe_free(__tmp); \
			} \
		} \
	} while(0)

/** Dictionary */
static unsigned int _ht_hash_string(void* ht, void* d);
static unsigned int _ht_hash_int(void* ht, void* d);

static int _ht_cmp_string(void* d1, void* d2);
static int _ht_cmp_int(void* d1, void* d2);

static _ht_node_t* _ht_create(unsigned int size, _ht_compare cmp, _ht_hash hs, _ls_operation freeextra);
static _ls_node_t* _ht_find(_ht_node_t* ht, void* key);
static unsigned int _ht_set_or_insert(_ht_node_t* ht, void* key, void* value);
static unsigned int _ht_remove(_ht_node_t* ht, void* key, _ls_compare cmp);
static unsigned int _ht_foreach(_ht_node_t* ht, _ht_operation op);
static void _ht_clear(_ht_node_t* ht);
static void _ht_destroy(_ht_node_t* ht);

/** Memory operations */
#define _MB_POINTER_SIZE (sizeof(intptr_t))
#define _MB_WRITE_CHUNK_SIZE(t, s) (*((size_t*)((char*)(t) - _MB_POINTER_SIZE)) = s)
#define _MB_READ_CHUNK_SIZE(t) (*((size_t*)((char*)(t) - _MB_POINTER_SIZE)))

#ifdef MB_ENABLE_ALLOC_STAT
static volatile size_t _mb_allocated = 0;
#else /* MB_ENABLE_ALLOC_STAT */
static volatile size_t _mb_allocated = (size_t)(~0);
#endif /* MB_ENABLE_ALLOC_STAT */

static void* mb_malloc(size_t s);
static void mb_free(void* p);

static size_t mb_memtest(void*p, size_t s);

static char* mb_strupr(char* s);

#define safe_free(__p) do { if(__p) { mb_free(__p); __p = 0; } else { mb_assert(0 && "Memory already released"); } } while(0)

/** Expression processing */
static bool_t _is_operator(mb_func_t op);
static bool_t _is_flow(mb_func_t op);
static char _get_priority(mb_func_t op1, mb_func_t op2);
static int _get_priority_index(mb_func_t op);
static _object_t* _operate_operand(mb_interpreter_t* s, _object_t* optr, _object_t* opnd1, _object_t* opnd2, int* status);
static bool_t _is_expression_terminal(mb_interpreter_t* s, _object_t* obj);
static int _calc_expression(mb_interpreter_t* s, _ls_node_t** l, _object_t** val);
static bool_t _is_print_terminal(mb_interpreter_t* s, _object_t* obj);

/** Others */
#ifdef _WARING_AS_ERROR
#	define _handle_error(__s, __err, __pos, __row, __col, __ret, __exit, __result) \
		do { \
			_set_current_error(__s, __err); \
			_set_error_pos(__s, __pos, __row, __col); \
			__result = __ret; \
			goto __exit; \
		} while(0)
#else /* _WARING_AS_ERROR */
#	define _handle_error(__s, __err, __pos, __row, __col, __ret, __exit, __result) \
		do { \
			_set_current_error(__s, __err); \
			_set_error_pos(__s, __pos, __row, __col); \
			if(__ret != MB_FUNC_WARNING) { \
				__result = __ret; \
			} \
			goto __exit; \
		} while(0)
#endif /* _WARING_AS_ERROR */
#ifdef MB_ENABLE_SOURCE_TRACE
#	define _HANDLE_ERROR(__s, __err, __obj, __ret, __exit, __result) _handle_error(__s, __err, (__obj)->source_pos, (__obj)->source_row, (__obj)->source_col, __ret, __exit, __result)
#else /* MB_ENABLE_SOURCE_TRACE */
#	define _HANDLE_ERROR(__s, __err, __obj, __ret, __exit, __result) _handle_error(__s, __err, 0, 0, 0, __ret, __exit, __result)
#endif /* MB_ENABLE_SOURCE_TRACE */
#define _handle_error_on_obj(__s, __err, __obj, __ret, __exit, __result) \
	do { \
		if(__obj) { \
			_HANDLE_ERROR(__s, __err, __obj, __ret, __exit, __result); \
		} else { \
			_handle_error(__s, __err, 0, 0, 0, __ret, __exit, __result); \
		} \
	} while(0)

static void _set_current_error(mb_interpreter_t* s, mb_error_e err);

static mb_print_func_t _get_printer(mb_interpreter_t* s);
static mb_input_func_t _get_inputer(mb_interpreter_t* s);

static bool_t _is_blank(char c);
static bool_t _is_newline(char c);
static bool_t _is_separator(char c);
static bool_t _is_bracket(char c);
static bool_t _is_quotation_mark(char c);
static bool_t _is_comment(char c);
static bool_t _is_identifier_char(char c);
static bool_t _is_operator_char(char c);

static int _append_char_to_symbol(mb_interpreter_t* s, char c);
static int _cut_symbol(mb_interpreter_t* s, int pos, unsigned short row, unsigned short col);
static int _append_symbol(mb_interpreter_t* s, char* sym, bool_t* delsym, int pos, unsigned short row, unsigned short col);
static int _create_symbol(mb_interpreter_t* s, _ls_node_t* l, char* sym, _object_t** obj, _ls_node_t*** asgn, bool_t* delsym);
static _data_e _get_symbol_type(mb_interpreter_t* s, char* sym, _raw_t* value);

static int _parse_char(mb_interpreter_t* s, char c, int pos, unsigned short row, unsigned short col);
static void _set_error_pos(mb_interpreter_t* s, int pos, unsigned short row, unsigned short col);

static int_t _get_size_of(_data_e type);
static bool_t _try_get_value(_object_t* obj, mb_value_u* val, _data_e expected);

static int _get_array_pos(struct mb_interpreter_t* s, _array_t* arr, int* d, int c);
static int _get_array_index(mb_interpreter_t* s, _ls_node_t** l, unsigned int* index);
static bool_t _get_array_elem(mb_interpreter_t* s, _array_t* arr, unsigned int index, mb_value_u* val, _data_e* type);
static int _set_array_elem(mb_interpreter_t* s, _ls_node_t* ast, _array_t* arr, unsigned int index, mb_value_u* val, _data_e* type);

static void _init_array(_array_t* arr);
static void _clear_array(_array_t* arr);
static void _destroy_array(_array_t* arr);
static bool_t _is_array(void* obj);
static bool_t _is_string(void* obj);
static char* _extract_string(_object_t* obj);
static bool_t _is_internal_object(_object_t* obj);
static int _dispose_object(_object_t* obj);
static int _destroy_object(void* data, void* extra);
static int _destroy_object_non_syntax(void* data, void* extra);
static int _remove_source_object(void* data, void* extra);
static int _compare_numbers(const _object_t* first, const _object_t* second);
static _data_e _public_type_to_internal_type(mb_data_e t);
static mb_data_e _internal_type_to_public_type(_data_e t);
static int _public_value_to_internal_object(mb_value_t* pbl, _object_t* itn);
static int _internal_object_to_public_value(_object_t* itn, mb_value_t* pbl);
static void _try_clear_intermediate_value(void* data, void* extra, mb_interpreter_t* s);

static void _stepped(mb_interpreter_t* s, _ls_node_t* ast);
static int _execute_statement(mb_interpreter_t* s, _ls_node_t** l);
static int _skip_to(mb_interpreter_t* s, _ls_node_t** l, mb_func_t f, _data_e t);
static int _skip_if_chunk(mb_interpreter_t* s, _ls_node_t** l);
static int _skip_struct(mb_interpreter_t* s, _ls_node_t** l, mb_func_t open_func, mb_func_t close_func);

static _parsing_context_t* _reset_parsing_context(_parsing_context_t* context);

static int _register_func(mb_interpreter_t* s, const char* n, mb_func_t f, bool_t local);
static int _remove_func(mb_interpreter_t* s, const char* n, bool_t local);

static int _open_constant(mb_interpreter_t* s);
static int _close_constant(mb_interpreter_t* s);
static int _open_core_lib(mb_interpreter_t* s);
static int _close_core_lib(mb_interpreter_t* s);
static int _open_std_lib(mb_interpreter_t* s);
static int _close_std_lib(mb_interpreter_t* s);

/* ========================================================} */

/*
** {========================================================
** Lib declarations
*/

/** Macro */
#ifdef _MSC_VER
#	if _MSC_VER < 1300
#		define _do_nothing do { static int i = 0; ++i; printf("Unaccessable function called %d times\n", i); } while(0)
#	else /* _MSC_VER < 1300 */
#		define _do_nothing do { printf("Unaccessable function: %s\n", __FUNCTION__); } while(0)
#	endif /* _MSC_VER < 1300 */
#elif defined __BORLANDC__
#	define _do_nothing do { printf("Unaccessable function: %s\n", __FUNC__); } while(0)
#elif defined __POCC__
#	define _do_nothing do { printf("Unaccessable function: %s\n", __func__); } while(0)
#else /* _MSC_VER */
#	define _do_nothing do { printf("Unaccessable function: %s\n", __FUNCTION__); } while(0)
#endif /* _MSC_VER */

/** Core lib */
static int _core_dummy_assign(mb_interpreter_t* s, void** l);
static int _core_add(mb_interpreter_t* s, void** l);
static int _core_min(mb_interpreter_t* s, void** l);
static int _core_mul(mb_interpreter_t* s, void** l);
static int _core_div(mb_interpreter_t* s, void** l);
static int _core_mod(mb_interpreter_t* s, void** l);
static int _core_pow(mb_interpreter_t* s, void** l);
static int _core_open_bracket(mb_interpreter_t* s, void** l);
static int _core_close_bracket(mb_interpreter_t* s, void** l);
static int _core_neg(mb_interpreter_t* s, void** l);
static int _core_equal(mb_interpreter_t* s, void** l);
static int _core_less(mb_interpreter_t* s, void** l);
static int _core_greater(mb_interpreter_t* s, void** l);
static int _core_less_equal(mb_interpreter_t* s, void** l);
static int _core_greater_equal(mb_interpreter_t* s, void** l);
static int _core_not_equal(mb_interpreter_t* s, void** l);
static int _core_and(mb_interpreter_t* s, void** l);
static int _core_or(mb_interpreter_t* s, void** l);
static int _core_not(mb_interpreter_t* s, void** l);
static int _core_let(mb_interpreter_t* s, void** l);
static int _core_dim(mb_interpreter_t* s, void** l);
static int _core_if(mb_interpreter_t* s, void** l);
static int _core_then(mb_interpreter_t* s, void** l);
static int _core_elseif(mb_interpreter_t* s, void** l);
static int _core_else(mb_interpreter_t* s, void** l);
static int _core_endif(mb_interpreter_t* s, void** l);
static int _core_for(mb_interpreter_t* s, void** l);
static int _core_to(mb_interpreter_t* s, void** l);
static int _core_step(mb_interpreter_t* s, void** l);
static int _core_next(mb_interpreter_t* s, void** l);
static int _core_while(mb_interpreter_t* s, void** l);
static int _core_wend(mb_interpreter_t* s, void** l);
static int _core_do(mb_interpreter_t* s, void** l);
static int _core_until(mb_interpreter_t* s, void** l);
static int _core_exit(mb_interpreter_t* s, void** l);
static int _core_goto(mb_interpreter_t* s, void** l);
static int _core_gosub(mb_interpreter_t* s, void** l);
static int _core_return(mb_interpreter_t* s, void** l);
#ifdef MB_ENABLE_ALLOC_STAT
static int _core_mem(mb_interpreter_t* s, void** l);
#endif /* MB_ENABLE_ALLOC_STAT */
static int _core_end(mb_interpreter_t* s, void** l);

/** Std lib */
static int _std_abs(mb_interpreter_t* s, void** l);
static int _std_sgn(mb_interpreter_t* s, void** l);
static int _std_sqr(mb_interpreter_t* s, void** l);
static int _std_floor(mb_interpreter_t* s, void** l);
static int _std_ceil(mb_interpreter_t* s, void** l);
static int _std_fix(mb_interpreter_t* s, void** l);
static int _std_round(mb_interpreter_t* s, void** l);
static int _std_rnd(mb_interpreter_t* s, void** l);
static int _std_sin(mb_interpreter_t* s, void** l);
static int _std_cos(mb_interpreter_t* s, void** l);
static int _std_tan(mb_interpreter_t* s, void** l);
static int _std_asin(mb_interpreter_t* s, void** l);
static int _std_acos(mb_interpreter_t* s, void** l);
static int _std_atan(mb_interpreter_t* s, void** l);
static int _std_exp(mb_interpreter_t* s, void** l);
static int _std_log(mb_interpreter_t* s, void** l);
static int _std_asc(mb_interpreter_t* s, void** l);
static int _std_chr(mb_interpreter_t* s, void** l);
static int _std_left(mb_interpreter_t* s, void** l);
static int _std_len(mb_interpreter_t* s, void** l);
static int _std_mid(mb_interpreter_t* s, void** l);
static int _std_right(mb_interpreter_t* s, void** l);
static int _std_str(mb_interpreter_t* s, void** l);
static int _std_val(mb_interpreter_t* s, void** l);
static int _std_print(mb_interpreter_t* s, void** l);
static int _std_input(mb_interpreter_t* s, void** l);

/** Lib information */
static const _func_t _core_libs[] = {
	{ "#", _core_dummy_assign },
	{ "+", _core_add },
	{ "-", _core_min },
	{ "*", _core_mul },
	{ "/", _core_div },
	{ "MOD", _core_mod },
	{ "^", _core_pow },
	{ "(", _core_open_bracket },
	{ ")", _core_close_bracket },
	{ 0, _core_neg },

	{ "=", _core_equal },
	{ "<", _core_less },
	{ ">", _core_greater },
	{ "<=", _core_less_equal },
	{ ">=", _core_greater_equal },
	{ "<>", _core_not_equal },

	{ "AND", _core_and },
	{ "OR", _core_or },
	{ "NOT", _core_not },

	{ "LET", _core_let },
	{ "DIM", _core_dim },

	{ "IF", _core_if },
	{ "THEN", _core_then },
	{ "ELSEIF", _core_elseif },
	{ "ELSE", _core_else },
	{ "ENDIF", _core_endif },

	{ "FOR", _core_for },
	{ "TO", _core_to },
	{ "STEP", _core_step },
	{ "NEXT", _core_next },
	{ "WHILE", _core_while },
	{ "WEND", _core_wend },
	{ "DO", _core_do },
	{ "UNTIL", _core_until },

	{ "EXIT", _core_exit },
	{ "GOTO", _core_goto },
	{ "GOSUB", _core_gosub },
	{ "RETURN", _core_return },

#ifdef MB_ENABLE_ALLOC_STAT
	{ "MEM", _core_mem },
#endif /* MB_ENABLE_ALLOC_STAT */

	{ "END", _core_end }
};

static const _func_t _std_libs[] = {
	{ "ABS", _std_abs },
	{ "SGN", _std_sgn },
	{ "SQR", _std_sqr },
	{ "FLOOR", _std_floor },
	{ "CEIL", _std_ceil },
	{ "FIX", _std_fix },
	{ "ROUND", _std_round },
	{ "RND", _std_rnd },
	{ "SIN", _std_sin },
	{ "COS", _std_cos },
	{ "TAN", _std_tan },
	{ "ASIN", _std_asin },
	{ "ACOS", _std_acos },
	{ "ATAN", _std_atan },
	{ "EXP", _std_exp },
	{ "LOG", _std_log },

	{ "ASC", _std_asc },
	{ "CHR", _std_chr },
	{ "LEFT", _std_left },
	{ "LEN", _std_len },
	{ "MID", _std_mid },
	{ "RIGHT", _std_right },
	{ "STR", _std_str },
	{ "VAL", _std_val },

	{ "PRINT", _std_print },
	{ "INPUT", _std_input }
};

/* ========================================================} */

/*
** {========================================================
** Private function definitions
*/

/** List */
int _ls_cmp_data(void* node, void* info) {
	_ls_node_t* n = (_ls_node_t*)node;

	return (n->data == info) ? 0 : 1;
}

int _ls_cmp_extra(void* node, void* info) {
	_ls_node_t* n = (_ls_node_t*)node;

	return (n->extra == info) ? 0 : 1;
}

int _ls_cmp_extra_string(void* node, void* info) {
	_ls_node_t* n = (_ls_node_t*)node;
	char* s1 = (char*)n->extra;
	char* s2 = (char*)info;

	return strcmp(s1, s2);
}

_ls_node_t* _ls_create_node(void* data) {
	_ls_node_t* result = 0;

	result = (_ls_node_t*)mb_malloc(sizeof(_ls_node_t));
	mb_assert(result);
	memset(result, 0, sizeof(_ls_node_t));
	result->data = data;

	return result;
}

_ls_node_t* _ls_create(void) {
	_ls_node_t* result = 0;

	result = _ls_create_node(0);

	return result;
}

_ls_node_t* _ls_back(_ls_node_t* node) {
	_ls_node_t* result = node;

	result = result->prev;

	return result;
}

_ls_node_t* _ls_pushback(_ls_node_t* list, void* data) {
	_ls_node_t* result = 0;
	_ls_node_t* tmp = 0;

	mb_assert(list);

	result = _ls_create_node(data);

	tmp = _ls_back(list);
	if(!tmp)
		tmp = list;
	tmp->next = result;
	result->prev = tmp;
	list->prev = result;

	return result;
}

void* _ls_popback(_ls_node_t* list) {
	void* result = 0;
	_ls_node_t* tmp = 0;

	mb_assert(list);

	tmp = _ls_back(list);
	if(tmp) {
		result = tmp->data;
		if(list != tmp->prev)
			list->prev = tmp->prev;
		else
			list->prev = 0;
		tmp->prev->next = 0;
		safe_free(tmp);
	}

	return result;
}

unsigned int _ls_try_remove(_ls_node_t* list, void* info, _ls_compare cmp, _ls_operation op) {
	unsigned int result = 0;
	_ls_node_t* tmp = 0;

	mb_assert(list && cmp);

	tmp = list->next;
	while(tmp) {
		if(cmp(tmp, info) == 0) {
			if(tmp->prev)
				tmp->prev->next = tmp->next;
			if(tmp->next)
				tmp->next->prev = tmp->prev;
			if(list->prev == tmp)
				list->prev = 0;
			if(op)
				op(tmp->data, tmp->extra);
			safe_free(tmp);
			++result;

			break;
		}
		tmp = tmp->next;
	}

	return result;
}

unsigned int _ls_foreach(_ls_node_t* list, _ls_operation op) {
	unsigned int idx = 0;
	int opresult = _OP_RESULT_NORMAL;
	_ls_node_t* tmp = 0;

	mb_assert(list && op);

	list = list->next;
	while(list) {
		opresult = op(list->data, list->extra);
		++idx;
		tmp = list;
		list = list->next;

		if(_OP_RESULT_NORMAL == opresult) {
			/* Do nothing */
		} else if(_OP_RESULT_DEL_NODE == opresult) {
			tmp->prev->next = list;
			if(list)
				list->prev = tmp->prev;
			safe_free(tmp);
		} else {
			/* Do nothing */
		}
	}

	return idx;
}

bool_t _ls_empty(_ls_node_t* list) {
	bool_t result = false;

	mb_assert(list);

	result = 0 == list->next;

	return result;
}

void _ls_clear(_ls_node_t* list) {
	_ls_node_t* tmp = 0;

	mb_assert(list);

	tmp = list;
	list = list->next;
	tmp->next = 0;
	tmp->prev = 0;

	while(list) {
		tmp = list;
		list = list->next;
		safe_free(tmp);
	}
}

void _ls_destroy(_ls_node_t* list) {
	_ls_clear(list);
	safe_free(list);
}

int _ls_free_extra(void* data, void* extra) {
	int result = _OP_RESULT_NORMAL;
	mb_unrefvar(data);

	mb_assert(extra);

	safe_free(extra);

	result = _OP_RESULT_DEL_NODE;

	return result;
}

/** Dictionary */
unsigned int _ht_hash_string(void* ht, void* d) {
	unsigned int result = 0;
	_ht_node_t* self = (_ht_node_t*)ht;
	char* s = (char*)d;
	unsigned int h = 0;

	mb_assert(ht);

	for( ; *s; ++s)
		h = 5 * h + *s;
	result = h % self->array_size;

	return result;
}

unsigned int _ht_hash_int(void* ht, void* d) {
	unsigned int result = 0;
	_ht_node_t* self = (_ht_node_t*)ht;
	int_t i = *(int_t*)d;

	mb_assert(ht);

	result = (unsigned int)i;
	result %= self->array_size;

	return result;
}

int _ht_cmp_string(void* d1, void* d2) {
	char* s1 = (char*)d1;
	char* s2 = (char*)d2;

	return strcmp(s1, s2);
}

int _ht_cmp_int(void* d1, void* d2) {
	int_t i1 = *(int_t*)d1;
	int_t i2 = *(int_t*)d2;
	int_t i = i1 - i2;
	int result = 0;
	if(i < 0)
		result = -1;
	else if(i > 0)
		result = 1;

	return result;
}

_ht_node_t* _ht_create(unsigned int size, _ht_compare cmp, _ht_hash hs, _ls_operation freeextra) {
	const unsigned int array_size = size ? size : _HT_ARRAY_SIZE_DEFAULT;
	_ht_node_t* result = 0;
	unsigned int ul = 0;

	if(!cmp)
		cmp = _ht_cmp_int;
	if(!hs)
		hs = _ht_hash_int;

	result = (_ht_node_t*)mb_malloc(sizeof(_ht_node_t));
	result->free_extra = freeextra;
	result->compare = cmp;
	result->hash = hs;
	result->array_size = array_size;
	result->count = 0;
	result->array = (_ls_node_t**)mb_malloc(sizeof(_ls_node_t*) * result->array_size);
	for(ul = 0; ul < result->array_size; ++ul)
		result->array[ul] = _ls_create();

	return result;
}

_ls_node_t* _ht_find(_ht_node_t* ht, void* key) {
	_ls_node_t* result = 0;
	_ls_node_t* bucket = 0;
	unsigned int hash_code = 0;

	mb_assert(ht && key);

	hash_code = ht->hash(ht, key);
	bucket = ht->array[hash_code];
	bucket = bucket->next;
	while(bucket) {
		if(ht->compare(bucket->extra, key) == 0) {
			result = bucket;

			break;
		}
		bucket = bucket->next;
	}

	return result;
}

unsigned int _ht_set_or_insert(_ht_node_t* ht, void* key, void* value) {
	unsigned int result = 0;
	_ls_node_t* bucket = 0;
	unsigned int hash_code = 0;

	mb_assert(ht && key);

	bucket = _ht_find(ht, key);
	if(bucket) { /* Update */
		bucket->data = value;
		++result;
	} else { /* Insert */
		hash_code = ht->hash(ht, key);
		bucket = ht->array[hash_code];
		bucket = _ls_pushback(bucket, value);
		mb_assert(bucket);
		bucket->extra = key;
		++ht->count;
		++result;
	}

	return result;
}

unsigned int _ht_remove(_ht_node_t* ht, void* key, _ls_compare cmp) {
	unsigned int result = 0;
	unsigned int hash_code = 0;
	_ls_node_t* bucket = 0;

	mb_assert(ht && key);

	if(!cmp)
		cmp = _ls_cmp_extra;

	bucket = _ht_find(ht, key);
	hash_code = ht->hash(ht, key);
	bucket = ht->array[hash_code];
	result = _ls_try_remove(bucket, key, cmp, ht->free_extra);
	ht->count -= result;

	return result;
}

unsigned int _ht_foreach(_ht_node_t* ht, _ht_operation op) {
	unsigned int result = 0;
	_ls_node_t* bucket = 0;
	unsigned int ul = 0;

	for(ul = 0; ul < ht->array_size; ++ul) {
		bucket = ht->array[ul];
		if(bucket)
			result += _ls_foreach(bucket, op);
	}

	return result;
}

void _ht_clear(_ht_node_t* ht) {
	unsigned int ul = 0;

	mb_assert(ht && ht->array);

	for(ul = 0; ul < ht->array_size; ++ul)
		_ls_clear(ht->array[ul]);
	ht->count = 0;
}

void _ht_destroy(_ht_node_t* ht) {
	unsigned int ul = 0;

	mb_assert(ht && ht->array);

	if(ht->free_extra)
		_ht_foreach(ht, ht->free_extra);

	for(ul = 0; ul < ht->array_size; ++ul)
		_ls_destroy(ht->array[ul]);

	safe_free(ht->array);
	safe_free(ht);
}

/** Memory operations */
void* mb_malloc(size_t s) {
	char* ret = NULL;
	size_t rs = s;
#ifdef MB_ENABLE_ALLOC_STAT
	rs += _MB_POINTER_SIZE;
#endif /* MB_ENABLE_ALLOC_STAT */
	ret = (char*)malloc(rs);
	mb_assert(ret);
#ifdef MB_ENABLE_ALLOC_STAT
	_mb_allocated += s;
	ret += _MB_POINTER_SIZE;
	_MB_WRITE_CHUNK_SIZE(ret, s);
#endif /* MB_ENABLE_ALLOC_STAT */

	return (void*)ret;
}

void mb_free(void* p) {
	mb_assert(p);

#ifdef MB_ENABLE_ALLOC_STAT
	do {
		size_t os = _MB_READ_CHUNK_SIZE(p);
		_mb_allocated -= os;
		p = (char*)p - _MB_POINTER_SIZE;
	} while(0);
#endif /* MB_ENABLE_ALLOC_STAT */

	free(p);
}

size_t mb_memtest(void*p, size_t s) {
	size_t result = 0;
	size_t i = 0;
	for(i = 0; i < s; i++)
		result += ((unsigned char*)p)[i];

	return result;
}

char* mb_strupr(char* s) {
	char* t = s;

	while(*s) {
		*s = toupper(*s);
		++s;
	}

	return t;
}

/** Expression processing */
bool_t _is_operator(mb_func_t op) {
	/* Determine whether a function is an operator */
	bool_t result = false;

	result =
		(op == _core_dummy_assign) ||
		(op == _core_add) ||
		(op == _core_min) ||
		(op == _core_mul) ||
		(op == _core_div) ||
		(op == _core_mod) ||
		(op == _core_pow) ||
		(op == _core_open_bracket) ||
		(op == _core_close_bracket) ||
		(op == _core_equal) ||
		(op == _core_less) ||
		(op == _core_greater) ||
		(op == _core_less_equal) ||
		(op == _core_greater_equal) ||
		(op == _core_not_equal) ||
		(op == _core_and) ||
		(op == _core_or);

	return result;
}

bool_t _is_flow(mb_func_t op) {
	/* Determine whether a function is for flow control */
	bool_t result = false;

	result =
		(op == _core_if) ||
		(op == _core_then) ||
		(op == _core_elseif) ||
		(op == _core_else) ||
		(op == _core_endif) ||
		(op == _core_for) ||
		(op == _core_to) ||
		(op == _core_step) ||
		(op == _core_next) ||
		(op == _core_while) ||
		(op == _core_wend) ||
		(op == _core_do) ||
		(op == _core_until) ||
		(op == _core_exit) ||
		(op == _core_goto) ||
		(op == _core_gosub) ||
		(op == _core_return) ||
		(op == _core_end);

	return result;
}

char _get_priority(mb_func_t op1, mb_func_t op2) {
	/* Get the priority of two operators */
	char result = '\0';
	int idx1 = 0;
	int idx2 = 0;

	mb_assert(op1 && op2);

	idx1 = _get_priority_index(op1);
	idx2 = _get_priority_index(op2);
	mb_assert(idx1 < _countof(_PRECEDE_TABLE) && idx2 < _countof(_PRECEDE_TABLE[0]));
	result = _PRECEDE_TABLE[idx1][idx2];

	return result;
}

int _get_priority_index(mb_func_t op) {
	/* Get the index of an operator in the priority table */
	int result = 0;

	mb_assert(op);

	if(op == _core_dummy_assign) {
		result = 8;
	} else if(op == _core_add) {
		result = 0;
	} else if(op == _core_min) {
		result = 1;
	} else if(op == _core_mul) {
		result = 2;
	} else if(op == _core_div) {
		result = 3;
	} else if(op == _core_mod) {
		result = 4;
	} else if(op == _core_pow) {
		result = 5;
	} else if(op == _core_open_bracket) {
		result = 6;
	} else if(op == _core_close_bracket) {
		result = 7;
	} else if(op == _core_equal) {
		result = 13;
	} else if(op == _core_greater) {
		result = 9;
	} else if(op == _core_less) {
		result = 10;
	} else if(op == _core_greater_equal) {
		result = 11;
	} else if(op == _core_less_equal) {
		result = 12;
	} else if(op == _core_not_equal) {
		result = 14;
	} else if(op == _core_and) {
		result = 15;
	} else if(op == _core_or) {
		result = 16;
	} else if(op == _core_not) {
		result = 17;
	} else if(op == _core_neg) {
		result = 18;
	} else {
		mb_assert(0 && "Unknown operator");
	}

	return result;
}

_object_t* _operate_operand(mb_interpreter_t* s, _object_t* optr, _object_t* opnd1, _object_t* opnd2, int* status) {
	/* Operate two operands */
	_object_t* result = 0;
	_tuple3_t tp;
	_tuple3_t* tpptr = 0;
	int _status = 0;

	mb_assert(s && optr);
	mb_assert(optr->type == _DT_FUNC);

	if(!opnd1)
		return result;

	result = (_object_t*)mb_malloc(sizeof(_object_t));
	memset(result, 0, sizeof(_object_t));

	memset(&tp, 0, sizeof(_tuple3_t));
	tp.e1 = opnd1;
	tp.e2 = opnd2;
	tp.e3 = result;
	tpptr = &tp;

	_status = (optr->data.func->pointer)(s, (void**)(&tpptr));
	if(status)
		*status = _status;
	if(_status != MB_FUNC_OK) {
		if(_status != MB_FUNC_WARNING) {
			safe_free(result);
			result = 0;
		}
		_set_current_error(s, SE_RN_OPERATION_FAILED);
#ifdef MB_ENABLE_SOURCE_TRACE
		_set_error_pos(s, optr->source_pos, optr->source_row, optr->source_col);
#else /* MB_ENABLE_SOURCE_TRACE */
		_set_error_pos(s, 0, 0, 0);
#endif /* MB_ENABLE_SOURCE_TRACE */
	}

	return result;
}

bool_t _is_expression_terminal(mb_interpreter_t* s, _object_t* obj) {
	/* Determine whether an object is an expression termination */
	bool_t result = false;

	mb_assert(s && obj);

	result =
		(obj->type == _DT_EOS) ||
		(obj->type == _DT_SEP) ||
		(obj->type == _DT_FUNC &&
			(obj->data.func->pointer == _core_then ||
			obj->data.func->pointer == _core_elseif ||
			obj->data.func->pointer == _core_else ||
			obj->data.func->pointer == _core_endif ||
			obj->data.func->pointer == _core_to ||
			obj->data.func->pointer == _core_step));

	return result;
}

int _calc_expression(mb_interpreter_t* s, _ls_node_t** l, _object_t** val) {
	/* Calculate an expression */
	int result = 0;
	_ls_node_t* ast = 0;
	_running_context_t* running = 0;
	_ls_node_t* garbage = 0;
	_ls_node_t* optr = 0;
	_ls_node_t* opnd = 0;
	_object_t* c = 0;
	_object_t* x = 0;
	_object_t* a = 0;
	_object_t* b = 0;
	_object_t* r = 0;
	_object_t* theta = 0;
	char pri = '\0';
	int* inep = 0;
	int f = 0;

	unsigned int arr_idx = 0;
	mb_value_u arr_val;
	_data_e arr_type;
	_object_t* arr_elem = 0;

	_object_t* guard_val = 0;
	int bracket_count = 0;
	bool_t hack = false;
	_ls_node_t* errn = 0;

	mb_assert(s && l);

	running = s->running_context;
	ast = *l;
	garbage = _ls_create();
	optr = _ls_create();
	opnd = _ls_create();

	inep = (int*)mb_malloc(sizeof(int));
	*inep = 0;
	_ls_pushback(running->in_neg_expr, inep);

	c = (_object_t*)(ast->data);
	do {
		if(c->type == _DT_STRING) {
			if(ast->next) {
				_object_t* _fsn = (_object_t*)ast->next->data;
				if(_IS_FUNC(_fsn, _core_add))
					break;
			}

			(*val)->type = _DT_STRING;
			(*val)->data.string = c->data.string;
			(*val)->ref = true;
			ast = ast->next;

			goto _exit;
		}
	} while(0);
	guard_val = c;
	ast = ast->next;
	_ls_pushback(optr, _exp_assign);
	while(
		!(c->type == _DT_FUNC && strcmp(c->data.func->name, "#") == 0) ||
		!(((_object_t*)(_ls_back(optr)->data))->type == _DT_FUNC && strcmp(((_object_t*)(_ls_back(optr)->data))->data.func->name, "#") == 0)) {
		if(!hack) {
			if(_IS_FUNC(c, _core_open_bracket)) {
				++bracket_count;
			} else if(_IS_FUNC(c, _core_close_bracket)) {
				--bracket_count;
				if(bracket_count < 0) {
					c = _exp_assign;
					ast = ast->prev;

					continue;
				}
			}
		}
		hack = false;
		if(!(c->type == _DT_FUNC && _is_operator(c->data.func->pointer))) {
			if(_is_expression_terminal(s, c)) {
				c = _exp_assign;
				if(ast)
					ast = ast->prev;
				if(bracket_count) {
					_object_t _cb;
					_func_t _cbf;
					memset(&_cb, 0, sizeof(_object_t));
					_cb.type = _DT_FUNC;
					_cb.data.func = &_cbf;
					_cb.data.func->name = ")";
					_cb.data.func->pointer = _core_close_bracket;
					while(bracket_count) {
						_ls_pushback(optr, &_cb);
						bracket_count--;
						f = 0;
					}
					errn = ast;
				}
			} else {
				if(c->type == _DT_ARRAY) {
					ast = ast->prev;
					result = _get_array_index(s, &ast, &arr_idx);
					if(result != MB_FUNC_OK) {
						_handle_error_on_obj(s, SE_RN_CALCULATION_ERROR, DON(ast), MB_FUNC_ERR, _exit, result);
					}
					ast = ast->next;
					_get_array_elem(s, c->data.array, arr_idx, &arr_val, &arr_type);
					arr_elem = (_object_t*)mb_malloc(sizeof(_object_t));
					memset(arr_elem, 0, sizeof(_object_t));
					_ls_pushback(garbage, arr_elem);
					arr_elem->type = arr_type;
					arr_elem->ref = true;
					if(arr_type == _DT_INT) {
						arr_elem->data.integer = arr_val.integer;
					} else if(arr_type == _DT_REAL) {
						arr_elem->data.float_point = arr_val.float_point;
					} else if(arr_type == _DT_STRING) {
						arr_elem->data.string = arr_val.string;
					} else if(arr_type == _DT_USERTYPE) {
						arr_elem->data.usertype = arr_val.usertype;
					} else {
						mb_assert(0 && "Unsupported");
					}
					if(f) {
						_handle_error_on_obj(s, SE_RN_OPERATOR_EXPECTED, DON(ast), MB_FUNC_ERR, _exit, result);
					}
					_ls_pushback(opnd, arr_elem);
					f++;
				} else if(c->type == _DT_FUNC) {
					ast = ast->prev;
					result = (c->data.func->pointer)(s, (void**)(&ast));
					if(result != MB_FUNC_OK) {
						_handle_error_on_obj(s, SE_RN_CALCULATION_ERROR, DON(ast), MB_FUNC_ERR, _exit, result);
					}
					c = (_object_t*)mb_malloc(sizeof(_object_t));
					memset(c, 0, sizeof(_object_t));
					_ls_pushback(garbage, c);
					result = _public_value_to_internal_object(&running->intermediate_value, c);
					if(result != MB_FUNC_OK)
						goto _exit;
					if(f) {
						_handle_error_on_obj(s, SE_RN_OPERATOR_EXPECTED, DON(ast), MB_FUNC_ERR, _exit, result);
					}
					_ls_pushback(opnd, c);
					f++;
				} else if(c->type == _DT_VAR && c->data.variable->data->type == _DT_ARRAY) {
					ast = ast->prev;
					result = _get_array_index(s, &ast, &arr_idx);
					if(result != MB_FUNC_OK) {
						_handle_error_on_obj(s, SE_RN_CALCULATION_ERROR, DON(ast), MB_FUNC_ERR, _exit, result);
					}
					ast = ast->next;
					_get_array_elem(s, c->data.variable->data->data.array, arr_idx, &arr_val, &arr_type);
					arr_elem = (_object_t*)mb_malloc(sizeof(_object_t));
					memset(arr_elem, 0, sizeof(_object_t));
					_ls_pushback(garbage, arr_elem);
					arr_elem->type = arr_type;
					arr_elem->ref = true;
					if(arr_type == _DT_INT) {
						arr_elem->data.integer = arr_val.integer;
					} else if(arr_type == _DT_REAL) {
						arr_elem->data.float_point = arr_val.float_point;
					} else if(arr_type == _DT_STRING) {
						arr_elem->data.string = arr_val.string;
					} else if(arr_type == _DT_USERTYPE) {
						arr_elem->data.usertype = arr_val.usertype;
					} else {
						mb_assert(0 && "Unsupported");
					}
					if(f) {
						_handle_error_on_obj(s, SE_RN_OPERATOR_EXPECTED, DON(ast), MB_FUNC_ERR, _exit, result);
					}
					_ls_pushback(opnd, arr_elem);
					f++;
				} else {
					if(c->type == _DT_VAR && ast) {
						_object_t* _err_var = (_object_t*)(ast->data);
						if(_IS_FUNC(_err_var, _core_open_bracket)) {
							_handle_error_on_obj(s, SE_RN_INVALID_ID_USAGE, DON(ast), MB_FUNC_ERR, _exit, result);
						}
					}
					if(f) {
						_handle_error_on_obj(s, SE_RN_OPERATOR_EXPECTED, DON(ast), MB_FUNC_ERR, _exit, result);
					}
					_ls_pushback(opnd, c);
					f++;
				}
				if(ast) {
					c = (_object_t*)(ast->data);
					if(c->type == _DT_FUNC && !_is_operator(c->data.func->pointer) && !_is_flow(c->data.func->pointer)) {
						_ls_foreach(opnd, _remove_source_object);

						_handle_error_on_obj(s, SE_RN_COLON_EXPECTED, DON(ast), MB_FUNC_ERR, _exit, result);
					}
					ast = ast->next;
				} else {
					c = _exp_assign;
				}
			}
		} else {
			pri = _get_priority(((_object_t*)(_ls_back(optr)->data))->data.func->pointer, c->data.func->pointer);
			switch(pri) {
			case '<':
				_ls_pushback(optr, c);
				c = (_object_t*)(ast->data);
				ast = ast->next;
				f = 0;

				break;
			case '=':
				x = (_object_t*)_ls_popback(optr);
				c = (_object_t*)(ast->data);
				ast = ast->next;

				break;
			case '>':
				theta = (_object_t*)_ls_popback(optr);
				b = (_object_t*)_ls_popback(opnd);
				a = (_object_t*)_ls_popback(opnd);
				r = _operate_operand(s, theta, a, b, &result);
				if(!r) {
					_ls_clear(optr);
					_handle_error_on_obj(s, SE_RN_OPERATION_FAILED, DON(errn), MB_FUNC_ERR, _exit, result);
				}
				_ls_pushback(opnd, r);
				_ls_pushback(garbage, r);
				if(_IS_FUNC(c, _core_close_bracket))
					hack = true;

				break;
			}
		}
	}

	if(errn) {
		_handle_error_on_obj(s, SE_RN_CLOSE_BRACKET_EXPECTED, DON(errn), MB_FUNC_ERR, _exit, result);
	}

	c = (_object_t*)(_ls_popback(opnd));
	if(!c || !(c->type == _DT_INT || c->type == _DT_REAL || c->type == _DT_STRING || c->type == _DT_VAR || c->type == _DT_USERTYPE || c->type == _DT_ARRAY)) {
		_set_current_error(s, SE_RN_INVALID_DATA_TYPE);
		result = MB_FUNC_ERR;

		goto _exit;
	}
	if(c->type == _DT_VAR) {
		(*val)->type = c->data.variable->data->type;
		(*val)->data = c->data.variable->data->data;
		if(_is_string(c))
			(*val)->ref = true;
	} else {
		(*val)->type = c->type;
		if(_is_string(c)) {
			char* _str = _extract_string(c);
			size_t _sl = strlen(_str);
			(*val)->data.string = (char*)mb_malloc(_sl + 1);
			(*val)->data.string[_sl] = '\0';
			memcpy((*val)->data.string, _str, _sl + 1);
		} else if(c->type == _DT_ARRAY) {
			(*val)->data = c->data;
			c->type = _DT_NIL;
		} else {
			(*val)->data = c->data;
		}
	}
	if(guard_val != c && _ls_try_remove(garbage, c, _ls_cmp_data, NULL)) {
		_try_clear_intermediate_value(c, 0, s);

		_destroy_object(c, 0);
	}

_exit:
	_LS_FOREACH(garbage, _destroy_object, _try_clear_intermediate_value, s);
	_ls_destroy(garbage);
	_ls_foreach(optr, _destroy_object_non_syntax);
	_ls_foreach(opnd, _destroy_object_non_syntax);
	_ls_destroy(optr);
	_ls_destroy(opnd);
	*l = ast;
	mb_free(_ls_popback(running->in_neg_expr));

	return result;
}

bool_t _is_print_terminal(mb_interpreter_t* s, _object_t* obj) {
	/* Determine whether an object is a PRINT termination */
	bool_t result = false;

	mb_assert(s && obj);

	result = _IS_EOS(obj) ||
		_IS_SEP(obj, ':') ||
		_IS_FUNC(obj, _core_elseif) ||
		_IS_FUNC(obj, _core_else) ||
		_IS_FUNC(obj, _core_endif);

	return result;
}

/** Others */
void _set_current_error(mb_interpreter_t* s, mb_error_e err) {
	/* Set current error information */
	mb_assert(s && err >= 0 && err < _countof(_ERR_DESC));

	if(s->last_error == SE_NO_ERR)
		s->last_error = err;
}

mb_print_func_t _get_printer(mb_interpreter_t* s) {
	/* Get a print functor according to an interpreter */
	mb_assert(s);

	if(s->printer)
		return s->printer;

	return printf;
}

mb_input_func_t _get_inputer(mb_interpreter_t* s) {
	/* Get an input functor according to an interpreter */
	mb_assert(s);

	if(s->inputer)
		return s->inputer;

	return mb_gets;
}

bool_t _is_blank(char c) {
	/* Determine whether a char is a blank */
	return (' ' == c) || ('\t' == c);
}

bool_t _is_newline(char c) {
	/* Determine whether a char is a newline */
	return ('\r' == c) || ('\n' == c) || (EOF == c);
}

bool_t _is_separator(char c) {
	/* Determine whether a char is a separator */
	return (',' == c) || (';' == c) || (':' == c);
}

bool_t _is_bracket(char c) {
	/* Determine whether a char is a bracket */
	return ('(' == c) || (')' == c);
}

bool_t _is_quotation_mark(char c) {
	/* Determine whether a char is a quotation mark */
	return ('"' == c);
}

bool_t _is_comment(char c) {
	/* Determine whether a char is a comment mark */
	return ('\'' == c);
}

bool_t _is_identifier_char(char c) {
	/* Determine whether a char is an identifier char */
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
		(c == '_') ||
		(c >= '0' && c <= '9') ||
		(c == '$') ||
		(c == '.');
}

bool_t _is_operator_char(char c) {
	/* Determine whether a char is an operator char */
	return (c == '+') || (c == '-') || (c == '*') || (c == '/') ||
		(c == '^') ||
		(c == '(') || (c == ')') ||
		(c == '=') ||
		(c == '>') || (c == '<');
}

int _append_char_to_symbol(mb_interpreter_t* s, char c) {
	/* Parse a char and append it to current parsing symbol */
	int result = MB_FUNC_OK;
	_parsing_context_t* context = 0;

	mb_assert(s);

	context = s->parsing_context;

	if(context->current_symbol_nonius + 1 >= _SINGLE_SYMBOL_MAX_LENGTH) {
		_set_current_error(s, SE_PS_SYMBOL_TOO_LONG);

		++result;
	} else {
		context->current_symbol[context->current_symbol_nonius] = c;
		++context->current_symbol_nonius;
	}

	return result;
}

int _cut_symbol(mb_interpreter_t* s, int pos, unsigned short row, unsigned short col) {
	/* Current symbol parsing done and cut it */
	int result = MB_FUNC_OK;
	_parsing_context_t* context = 0;
	char* sym = 0;
	int status = 0;
	bool_t delsym = false;

	mb_assert(s);

	context = s->parsing_context;
	if(context->current_symbol_nonius && context->current_symbol[0] != '\0') {
		sym = (char*)mb_malloc(context->current_symbol_nonius + 1);
		memcpy(sym, context->current_symbol, context->current_symbol_nonius + 1);

		status = _append_symbol(s, sym, &delsym, pos, row, col);
		if(status || delsym) {
			safe_free(sym);
		}
		result = status;
	}
	memset(context->current_symbol, 0, sizeof(context->current_symbol));
	context->current_symbol_nonius = 0;

	return result;
}

int _append_symbol(mb_interpreter_t* s, char* sym, bool_t* delsym, int pos, unsigned short row, unsigned short col) {
	/* Append cut current symbol to AST list */
	int result = MB_FUNC_OK;
	_ls_node_t* ast = 0;
	_object_t* obj = 0;
	_ls_node_t** assign = 0;
	_ls_node_t* node = 0;
	_parsing_context_t* context = 0;

	mb_assert(s && sym);

	ast = s->ast;
	result = _create_symbol(s, ast, sym, &obj, &assign, delsym);
	if(obj) {
#ifdef MB_ENABLE_SOURCE_TRACE
		obj->source_pos = pos;
		obj->source_row = row;
		obj->source_col = col;
#else /* MB_ENABLE_SOURCE_TRACE */
		mb_unrefvar(row);
		mb_unrefvar(col);
		obj->source_pos = (char)pos;
#endif /* MB_ENABLE_SOURCE_TRACE */

		node = _ls_pushback(ast, obj);
		if(assign)
			*assign = node;

		context = s->parsing_context;
		context->last_symbol = obj;
	}

	return result;
}

int _create_symbol(mb_interpreter_t* s, _ls_node_t* l, char* sym, _object_t** obj, _ls_node_t*** asgn, bool_t* delsym) {
	/* Create a syntax symbol */
	int result = MB_FUNC_OK;
	_data_e type;
	union { _func_t* func; _array_t* array; _var_t* var; _label_t* label; real_t float_point; int_t integer; _raw_t any; } tmp;
	_raw_t value;
	unsigned int ul = 0;
	_parsing_context_t* context = 0;
	_ls_node_t* glbsyminscope = 0;
	mb_unrefvar(l);

	mb_assert(s && sym && obj);

	memset(value, 0, sizeof(_raw_t));

	context = s->parsing_context;

	*obj = (_object_t*)mb_malloc(sizeof(_object_t));
	memset(*obj, 0, sizeof(_object_t));
#ifdef MB_ENABLE_SOURCE_TRACE
	(*obj)->source_pos = -1;
	(*obj)->source_row = (*obj)->source_col = 0xffff;
#else /* MB_ENABLE_SOURCE_TRACE */
	(*obj)->source_pos = -1;
#endif /* MB_ENABLE_SOURCE_TRACE */

	type = _get_symbol_type(s, sym, &value);
	(*obj)->type = type;
	switch(type) {
	case _DT_NIL:
		safe_free(*obj);
		*obj = 0;
		safe_free(sym);

		break;
	case _DT_INT:
		memcpy(tmp.any, value, sizeof(_raw_t));
		(*obj)->data.integer = tmp.integer;
		safe_free(sym);

		break;
	case _DT_REAL:
		memcpy(tmp.any, value, sizeof(_raw_t));
		(*obj)->data.float_point = tmp.float_point;
		safe_free(sym);

		break;
	case _DT_STRING: {
			size_t _sl = strlen(sym);
			(*obj)->data.string = (char*)mb_malloc(_sl - 2 + 1);
			memcpy((*obj)->data.string, sym + sizeof(char), _sl - 2);
			(*obj)->data.string[_sl - 2] = '\0';
			*delsym = true;
		}

		break;
	case _DT_FUNC:
		tmp.func = (_func_t*)mb_malloc(sizeof(_func_t));
		memset(tmp.func, 0, sizeof(_func_t));
		tmp.func->name = sym;
		memcpy(&tmp.func->pointer, value, sizeof(tmp.func->pointer));
		(*obj)->data.func = tmp.func;

		break;
	case _DT_ARRAY:
		glbsyminscope = _ht_find(s->global_var_dict, sym);
		if(glbsyminscope && ((_object_t*)(glbsyminscope->data))->type == _DT_ARRAY) {
			(*obj)->data.array = ((_object_t*)(glbsyminscope->data))->data.array;
			(*obj)->ref = true;
			*delsym = true;
		} else {
			tmp.array = (_array_t*)mb_malloc(sizeof(_array_t));
			memset(tmp.array, 0, sizeof(_array_t));
			tmp.array->name = sym;
			memcpy(&tmp.array->type, value, sizeof(tmp.array->type));
			(*obj)->data.array = tmp.array;

			ul = _ht_set_or_insert(s->global_var_dict, sym, *obj);
			mb_assert(ul);

			*obj = (_object_t*)mb_malloc(sizeof(_object_t));
			memset(*obj, 0, sizeof(_object_t));
			(*obj)->type = type;
			(*obj)->data.array = tmp.array;
			(*obj)->ref = true;
		}

		break;
	case _DT_VAR:
		glbsyminscope = _ht_find(s->global_var_dict, sym);
		if(glbsyminscope && ((_object_t*)(glbsyminscope->data))->type == _DT_VAR) {
			(*obj)->data.variable = ((_object_t*)(glbsyminscope->data))->data.variable;
			(*obj)->ref = true;
			*delsym = true;
		} else {
			tmp.var = (_var_t*)mb_malloc(sizeof(_var_t));
			memset(tmp.var, 0, sizeof(_var_t));
			tmp.var->name = sym;
			tmp.var->data = (_object_t*)mb_malloc(sizeof(_object_t));
			memset(tmp.var->data, 0, sizeof(_object_t));
			tmp.var->data->type = (sym[strlen(sym) - 1] == '$') ? _DT_STRING : _DT_INT;
			tmp.var->data->data.integer = 0;
			(*obj)->data.variable = tmp.var;

			ul = _ht_set_or_insert(s->global_var_dict, sym, *obj);
			mb_assert(ul);

			*obj = (_object_t*)mb_malloc(sizeof(_object_t));
			memset(*obj, 0, sizeof(_object_t));
			(*obj)->type = type;
			(*obj)->data.variable = tmp.var;
			(*obj)->ref = true;
		}

		break;
	case _DT_LABEL:
		if(context->current_char == ':') {
			if(mb_memtest(value, sizeof(_raw_t))) {
				memcpy(&((*obj)->data.label), value, sizeof((*obj)->data.label));
				(*obj)->ref = true;
				*delsym = true;
			} else {
				tmp.label = (_label_t*)mb_malloc(sizeof(_label_t));
				memset(tmp.label, 0, sizeof(_label_t));
				tmp.label->name = sym;
				*asgn = &(tmp.label->node);
				(*obj)->data.label = tmp.label;

				ul = _ht_set_or_insert(s->global_var_dict, sym, *obj);
				mb_assert(ul);

				*obj = (_object_t*)mb_malloc(sizeof(_object_t));
				memset(*obj, 0, sizeof(_object_t));
				(*obj)->type = type;
				(*obj)->data.label = tmp.label;
				(*obj)->ref = true;
			}
		} else {
			(*obj)->data.label = (_label_t*)mb_malloc(sizeof(_label_t));
			memset((*obj)->data.label, 0, sizeof(_label_t));
			(*obj)->data.label->name = sym;
		}

		break;
	case _DT_SEP:
		(*obj)->data.separator = sym[0];
		safe_free(sym);

		break;
	case _DT_EOS:
		safe_free(sym);

		break;
	default:
		break;
	}

	return result;
}

_data_e _get_symbol_type(mb_interpreter_t* s, char* sym, _raw_t* value) {
	/* Get the type of a syntax symbol */
	_data_e result = _DT_NIL;
	union { real_t float_point; int_t integer; _object_t* obj; _raw_t any; } tmp;
	char* conv_suc = 0;
	_parsing_context_t* context = 0;
	_ls_node_t* lclsyminscope = 0;
	_ls_node_t* glbsyminscope = 0;
	size_t _sl = 0;
	_data_e en = _DT_ANY;
	intptr_t ptr = 0;

	mb_assert(s && sym);
	_sl = strlen(sym);
	mb_assert(_sl > 0);

	context = s->parsing_context;

	/* int_t */
	tmp.integer = (int_t)mb_strtol(sym, &conv_suc, 0);
	if(*conv_suc == '\0') {
		memcpy(*value, tmp.any, sizeof(_raw_t));

		result = _DT_INT;

		goto _exit;
	}
	/* real_t */
	tmp.float_point = (real_t)mb_strtod(sym, &conv_suc);
	if(*conv_suc == '\0') {
		memcpy(*value, tmp.any, sizeof(_raw_t));

		result = _DT_REAL;

		goto _exit;
	}
	/* string */
	if(sym[0] == '"' && sym[_sl - 1] == '"' && _sl >= 2) {
		result = _DT_STRING;

		goto _exit;
	}
	/* _array_t */
	glbsyminscope = _ht_find(s->global_var_dict, sym);
	if(glbsyminscope && ((_object_t*)(glbsyminscope->data))->type == _DT_ARRAY) {
		tmp.obj = (_object_t*)(glbsyminscope->data);
		memcpy(*value, &(tmp.obj->data.array->type), sizeof(tmp.obj->data.array->type));

		result = _DT_ARRAY;

		goto _exit;
	}
	if(context->last_symbol && context->last_symbol->type == _DT_FUNC) {
		if(strcmp("DIM", context->last_symbol->data.func->name) == 0) {
#ifdef MB_SIMPLE_ARRAY
			en = (sym[_sl - 1] == '$' ? _DT_STRING : _DT_REAL);
#else /* MB_SIMPLE_ARRAY */
			en = _DT_REAL;
#endif /* MB_SIMPLE_ARRAY */
			memcpy(*value, &en, sizeof(en));

			result = _DT_ARRAY;

			goto _exit;
		}
	}
	/* _func_t */
	if(!context->last_symbol ||
		(context->last_symbol && ((context->last_symbol->type == _DT_FUNC && context->last_symbol->data.func->pointer != _core_close_bracket) ||
		context->last_symbol->type == _DT_SEP))) {
		if(strcmp("-", sym) == 0) {
			ptr = (intptr_t)_core_neg;
			memcpy(*value, &ptr, sizeof(intptr_t));

			result = _DT_FUNC;

			goto _exit;
		}
	}
	lclsyminscope = _ht_find(s->local_func_dict, sym);
	glbsyminscope = _ht_find(s->global_func_dict, sym);
	if(lclsyminscope || glbsyminscope) {
		ptr = lclsyminscope ? (intptr_t)lclsyminscope->data : (intptr_t)glbsyminscope->data;
		memcpy(*value, &ptr, sizeof(intptr_t));

		result = _DT_FUNC;

		goto _exit;
	}
	/* MB_EOS */
	if(_sl == 1 && sym[0] == MB_EOS) {
		if(_IS_EOS(context->last_symbol))
			result = _DT_NIL;
		else
			result = _DT_EOS;

		goto _exit;
	}
	/* separator */
	if(_sl == 1 && _is_separator(sym[0])) {
		result = _DT_SEP;

		goto _exit;
	}
	/* _var_t */
	glbsyminscope = _ht_find(s->global_var_dict, sym);
	if(glbsyminscope) {
		if(((_object_t*)glbsyminscope->data)->type != _DT_LABEL) {
			memcpy(*value, &glbsyminscope->data, sizeof(glbsyminscope->data));

			result = _DT_VAR;

			goto _exit;
		}
	}
	/* _label_t */
	if(context->current_char == ':') {
		if(!context->last_symbol || _IS_EOS(context->last_symbol)) {
			glbsyminscope = _ht_find(s->global_var_dict, sym);
			if(glbsyminscope) {
				memcpy(*value, &glbsyminscope->data, sizeof(glbsyminscope->data));
			}

			result = _DT_LABEL;

			goto _exit;
		}
	}
	if(context->last_symbol && context->last_symbol->type == _DT_FUNC) {
		if(context->last_symbol->data.func->pointer == _core_goto || context->last_symbol->data.func->pointer == _core_gosub) {
			result = _DT_LABEL;

			goto _exit;
		}
	}
	/* else */
	result = _DT_VAR;

_exit:
	return result;
}

int _parse_char(mb_interpreter_t* s, char c, int pos, unsigned short row, unsigned short col) {
	/* Parse a char */
	int result = MB_FUNC_OK;
	_parsing_context_t* context = 0;
	char last_char = '\0';

	mb_assert(s && s->parsing_context);

	context = s->parsing_context;

	last_char = context->current_char;
	context->current_char = c;

	if(context->parsing_state == _PS_NORMAL) {
		if(c >= 'a' && c <= 'z')
			c += 'A' - 'a';

		if(_is_blank(c)) { /* \t ' ' */
			result += _cut_symbol(s, pos, row, col);
		} else if(_is_newline(c)) { /* \r \n EOF */
			result += _cut_symbol(s, pos, row, col);
			result += _append_char_to_symbol(s, MB_EOS);
			result += _cut_symbol(s, pos, row, col);
		} else if(_is_separator(c) || _is_bracket(c)) { /* , ; : ( ) */
			result += _cut_symbol(s, pos, row, col);
			result += _append_char_to_symbol(s, c);
			result += _cut_symbol(s, pos, row, col);
		} else if(_is_quotation_mark(c)) { /* " */
			result += _cut_symbol(s, pos, row, col);
			result += _append_char_to_symbol(s, c);
			context->parsing_state = _PS_STRING;
		} else if(_is_comment(c)) { /* ' */
			result += _cut_symbol(s, pos, row, col);
			result += _append_char_to_symbol(s, MB_EOS);
			result += _cut_symbol(s, pos, row, col);
			context->parsing_state = _PS_COMMENT;
		} else {
			if(context->symbol_state == _SS_IDENTIFIER) {
				if(_is_identifier_char(c)) {
					result += _append_char_to_symbol(s, c);
				} else if(_is_operator_char(c)) {
					if((last_char == 'e' || last_char == 'E') && c == '-') {
						result += _append_char_to_symbol(s, c);
					} else {
						context->symbol_state = _SS_OPERATOR;
						result += _cut_symbol(s, pos, row, col);
						result += _append_char_to_symbol(s, c);
					}
				} else {
					_handle_error(s, SE_PS_INVALID_CHAR, pos, row, col, MB_FUNC_ERR, _exit, result);
				}
			} else if(context->symbol_state == _SS_OPERATOR) {
				if(_is_identifier_char(c)) {
					context->symbol_state = _SS_IDENTIFIER;
					result += _cut_symbol(s, pos, row, col);
					result += _append_char_to_symbol(s, c);
				} else if(_is_operator_char(c)) {
					if(c == '-')
						result += _cut_symbol(s, pos, row, col);
					result += _append_char_to_symbol(s, c);
				} else {
					_handle_error(s, SE_PS_INVALID_CHAR, pos, row, col, MB_FUNC_ERR, _exit, result);
				}
			} else {
				mb_assert(0 && "Impossible here");
			}
		}
	} else if(context->parsing_state == _PS_STRING) {
		if(_is_quotation_mark(c)) { /* " */
			result += _append_char_to_symbol(s, c);
			result += _cut_symbol(s, pos, row, col);
			context->parsing_state = _PS_NORMAL;
		} else {
			result += _append_char_to_symbol(s, c);
		}
	} else if(context->parsing_state == _PS_COMMENT) {
		if(_is_newline(c)) { /* \r \n EOF */
			context->parsing_state = _PS_NORMAL;
		} else {
			/* Do nothing */
		}
	} else {
		mb_assert(0 && "Unknown parsing state");
	}

_exit:
	return result;
}

void _set_error_pos(mb_interpreter_t* s, int pos, unsigned short row, unsigned short col) {
	/* Set the position of a parsing error */
	mb_assert(s);

	s->last_error_pos = pos;
	s->last_error_row = row;
	s->last_error_col = col;
}

int_t _get_size_of(_data_e type) {
	/* Get the size of a data type */
	int_t result = 0;

#ifdef MB_SIMPLE_ARRAY
	if(type == _DT_INT) {
		result = sizeof(int_t);
	} else if(type == _DT_REAL) {
		result = sizeof(real_t);
	} else if(type == _DT_STRING) {
		result = sizeof(char*);
	} else {
		mb_assert(0 && "Unsupported");
	}
#else /* MB_SIMPLE_ARRAY */
	mb_unrefvar(type);
	result = sizeof(_raw_u);
#endif /* MB_SIMPLE_ARRAY */

	return result;
}

bool_t _try_get_value(_object_t* obj, mb_value_u* val, _data_e expected) {
	/* Try to get a value(typed as int_t, real_t or char*) */
	bool_t result = false;

	mb_assert(obj && val);

	if(obj->type == _DT_INT && (expected == _DT_ANY || expected == _DT_INT)) {
		val->integer = obj->data.integer;
		result = true;
	} else if(obj->type == _DT_REAL && (expected == _DT_ANY || expected == _DT_REAL)) {
		val->float_point = obj->data.float_point;
		result = true;
	} else if(obj->type == _DT_VAR) {
		result = _try_get_value(obj->data.variable->data, val, expected);
	}

	return result;
}

int _get_array_pos(struct mb_interpreter_t* s, _array_t* arr, int* d, int c) {
	/* Calculate the index, used when interactive with host */
	int result = 0;
	int i = 0;
	int n = 0;

	mb_assert(s && arr && d);

	if(c < 0 || c > arr->dimension_count) {
		result = -1;

		goto _exit;
	}
	for(i = 0; i < c; i++) {
		n = d[i];
		if(n < 0 || n >= arr->dimensions[i]) {
			result = -1;

			goto _exit;
		}
		if(result)
			result *= n;
		else
			result += n;
	}

_exit:
	return result;
}

int _get_array_index(mb_interpreter_t* s, _ls_node_t** l, unsigned int* index) {
	/* Calculate the index, used when walking through an AST */
	int result = MB_FUNC_OK;
	_ls_node_t* ast = 0;
	_object_t* arr = 0;
	_object_t* len = 0;
	_object_t subscript;
	_object_t* subscript_ptr = 0;
	mb_value_u val;
	int dcount = 0;
	unsigned int idx = 0;

	mb_assert(s && l && index);

	subscript_ptr = &subscript;

	/* Array name */
	ast = (_ls_node_t*)(*l);
	if(!ast || !_is_array(ast->data)) {
		_handle_error_on_obj(s, SE_RN_ARRAY_IDENTIFIER_EXPECTED, DON(ast), MB_FUNC_ERR, _exit, result);
	}
	if(((_object_t*)(ast->data))->type == _DT_ARRAY)
		arr = (_object_t*)(ast->data);
	else
		arr = ((_object_t*)(ast->data))->data.variable->data;
	/* ( */
	if(!ast->next || ((_object_t*)(ast->next->data))->type != _DT_FUNC || ((_object_t*)(ast->next->data))->data.func->pointer != _core_open_bracket) {
		_handle_error_on_obj(s, SE_RN_OPEN_BRACKET_EXPECTED,
			(ast && ast->next) ? ((_object_t*)(ast->next->data)) : 0,
			MB_FUNC_ERR, _exit, result);
	}
	ast = ast->next;
	/* Array subscript */
	if(!ast->next) {
		_handle_error_on_obj(s, SE_RN_ARRAY_SUBSCRIPT_EXPECTED, DON(ast), MB_FUNC_ERR, _exit, result);
	}
	ast = ast->next;
	while(((_object_t*)(ast->data))->type != _DT_FUNC || ((_object_t*)(ast->data))->data.func->pointer != _core_close_bracket) {
		/* Calculate an integer value */
		result = _calc_expression(s, &ast, &subscript_ptr);
		if(result != MB_FUNC_OK)
			goto _exit;
		len = subscript_ptr;
		if(!_try_get_value(len, &val, _DT_INT)) {
			_handle_error_on_obj(s, SE_RN_TYPE_NOT_MATCH, DON(ast), MB_FUNC_ERR, _exit, result);
		}
		if(val.integer < 0) {
			_handle_error_on_obj(s, SE_RN_ILLEGAL_BOUND, DON(ast), MB_FUNC_ERR, _exit, result);
		}
		if(dcount + 1 > arr->data.array->dimension_count) {
			_handle_error_on_obj(s, SE_RN_DIMENSION_OUT_OF_BOUND, DON(ast), MB_FUNC_ERR, _exit, result);
		}
		if(val.integer >= arr->data.array->dimensions[dcount]) {
			_handle_error_on_obj(s, SE_RN_ARRAY_OUT_OF_BOUND, DON(ast), MB_FUNC_ERR, _exit, result);
		}
		if(idx)
			idx *= (unsigned int)val.integer;
		else
			idx += (unsigned int)val.integer;
		/* Comma? */
		if(_IS_SEP(ast->data, ','))
			ast = ast->next;

		++dcount;
	}
	*index = idx;

_exit:
	*l = ast;

	return result;
}

bool_t _get_array_elem(mb_interpreter_t* s, _array_t* arr, unsigned int index, mb_value_u* val, _data_e* type) {
	/* Get the value of an element in an array */
	bool_t result = true;
	int_t elemsize = 0;
	unsigned int pos = 0;
	void* rawptr = 0;

	mb_assert(s && arr && val && type);

	mb_assert(index < arr->count);
	elemsize = _get_size_of(arr->type);
	pos = (unsigned int)(elemsize * index);
	rawptr = (void*)((intptr_t)arr->raw + pos);
	if(arr->type == _DT_REAL) {
#ifdef MB_SIMPLE_ARRAY
		val->float_point = *((real_t*)rawptr);
		*type = _DT_REAL;
#else /* MB_SIMPLE_ARRAY */
		if(arr->types[index] == _DT_REAL) {
			val->float_point = *((real_t*)rawptr);
			*type = _DT_REAL;
		} else if(arr->types[index] == _DT_INT) {
			val->integer = *((int_t*)rawptr);
			*type = _DT_INT;
		} else if(arr->types[index] == _DT_USERTYPE) {
			val->usertype = *((void**)rawptr);
			*type = _DT_USERTYPE;
		} else if(arr->types[index] == _DT_STRING) {
			val->string = *((char**)rawptr);
			*type = _DT_STRING;
		} else {
			mb_assert(0 && "Unsupported");
		}
#endif /* MB_SIMPLE_ARRAY */
	} else if(arr->type == _DT_STRING) {
		val->string = *((char**)rawptr);
		*type = _DT_STRING;
	} else {
		mb_assert(0 && "Unsupported");
	}

	return result;
}

int _set_array_elem(mb_interpreter_t* s, _ls_node_t* ast, _array_t* arr, unsigned int index, mb_value_u* val, _data_e* type) {
	/* Set the value of an element in an array */
	int result = MB_FUNC_OK;
	int_t elemsize = 0;
	unsigned int pos = 0;
	void* rawptr = 0;
	mb_unrefvar(ast);

	mb_assert(s && arr && val);

	mb_assert(index < arr->count);
	elemsize = _get_size_of(arr->type);
	pos = (unsigned int)(elemsize * index);
	rawptr = (void*)((intptr_t)arr->raw + pos);
	if(*type == _DT_INT) {
#ifdef MB_SIMPLE_ARRAY
		*((real_t*)rawptr) = (real_t)val->integer;
#else /* MB_SIMPLE_ARRAY */
		*((int_t*)rawptr) = val->integer;
		arr->types[index] = _DT_INT;
#endif /* MB_SIMPLE_ARRAY */
	} else if(*type == _DT_REAL) {
		*((real_t*)rawptr) = val->float_point;
#ifndef MB_SIMPLE_ARRAY
		arr->types[index] = _DT_REAL;
#endif /* MB_SIMPLE_ARRAY */
	} else if(*type == _DT_STRING) {
		size_t _sl = 0;
#ifndef MB_SIMPLE_ARRAY
		arr->types[index] = _DT_STRING;
#else /* MB_SIMPLE_ARRAY */
		if(arr->type != _DT_STRING) {
			_handle_error_on_obj(s, SE_RN_STRING_EXPECTED, DON(ast), MB_FUNC_ERR, _exit, result);
		}
#endif /* MB_SIMPLE_ARRAY */
		_sl = strlen(val->string);
		*((char**)rawptr) = (char*)mb_malloc(_sl + 1);
		memcpy(*((char**)rawptr), val->string, _sl + 1);
#ifndef MB_SIMPLE_ARRAY
	} else if(*type == _DT_USERTYPE) {
		*((void**)rawptr) = val->usertype;
		arr->types[index] = _DT_USERTYPE;
#endif /* MB_SIMPLE_ARRAY */
	} else {
		mb_assert(0 && "Unsupported");
	}

	goto _exit; /* Avoid an unreferenced warning */

_exit:
	return result;
}

void _init_array(_array_t* arr) {
	/* Initialize an array */
	int elemsize = 0;

	mb_assert(arr);

#ifdef MB_SIMPLE_ARRAY
	elemsize = (int)_get_size_of(arr->type);
#else /* MB_SIMPLE_ARRAY */
	elemsize = (int)_get_size_of(_DT_ANY);
#endif /* MB_SIMPLE_ARRAY */
	mb_assert(arr->count > 0);
	mb_assert(!arr->raw);
	arr->raw = (void*)mb_malloc(elemsize * arr->count);
	if(arr->raw) {
		memset(arr->raw, 0, elemsize * arr->count);
	}
#ifndef MB_SIMPLE_ARRAY
	arr->types = (_data_e*)mb_malloc(sizeof(_data_e) * arr->count);
	if(arr->types) {
		unsigned int ul = 0;
		for(ul = 0; ul < arr->count; ++ul) {
			arr->types[ul] = _DT_INT;
		}
	}
#endif /* MB_SIMPLE_ARRAY */
}

void _clear_array(_array_t* arr) {
	/* Clear an array */
	char* str = 0;
	int_t elemsize = 0;
	unsigned int pos = 0;
	void* rawptr = 0;
	unsigned int ul = 0;

	mb_assert(arr);

	if(arr->raw) {
#ifndef MB_SIMPLE_ARRAY
		if(arr->type == _DT_REAL) {
			for(ul = 0; ul < arr->count; ++ul) {
				if(arr->types[ul] == _DT_STRING) {
					elemsize = _get_size_of(arr->type);
					pos = (unsigned int)(elemsize * ul);
					rawptr = (void*)((intptr_t)arr->raw + pos);
					str = *((char**)rawptr);
					if(str) {
						safe_free(str);
					}
				}
			}
		}
#endif /* MB_SIMPLE_ARRAY */
		if(arr->type == _DT_STRING) {
			for(ul = 0; ul < arr->count; ++ul) {
				elemsize = _get_size_of(arr->type);
				pos = (unsigned int)(elemsize * ul);
				rawptr = (void*)((intptr_t)arr->raw + pos);
				str = *((char**)rawptr);
				if(str) {
					safe_free(str);
				}
			}
		}
		safe_free(arr->raw);
		arr->raw = 0;
	}
}

void _destroy_array(_array_t* arr) {
	/* Destroy an array */
	mb_assert(arr);

	_clear_array(arr);
	if(arr->name) {
		safe_free(arr->name);
	}
#ifndef MB_SIMPLE_ARRAY
	if(arr->types) {
		safe_free(arr->types);
	}
#endif /* MB_SIMPLE_ARRAY */
	safe_free(arr);
}

bool_t _is_array(void* obj) {
	/* Determine whether an object is an array value or an array variable */
	bool_t result = false;
	_object_t* o = 0;

	mb_assert(obj);

	o = (_object_t*)obj;
	if(o->type == _DT_ARRAY)
		result = true;
	else if(o->type == _DT_VAR)
		result = o->data.variable->data->type == _DT_ARRAY;

	return result;
}

bool_t _is_string(void* obj) {
	/* Determine whether an object is a string value or a string variable */
	bool_t result = false;
	_object_t* o = 0;

	mb_assert(obj);

	o = (_object_t*)obj;
	if(o->type == _DT_STRING)
		result = true;
	else if(o->type == _DT_VAR)
		result = o->data.variable->data->type == _DT_STRING;

	return result;
}

char* _extract_string(_object_t* obj) {
	/* Extract a string inside an object */
	char* result = 0;

	mb_assert(obj);

	if(obj->type == _DT_STRING)
		result = obj->data.string;
	else if(obj->type == _DT_VAR && obj->data.variable->data->type == _DT_STRING)
		result = obj->data.variable->data->data.string;

	if(!result)
		result = MB_NULL_STRING;

	return result;
}

bool_t _is_internal_object(_object_t* obj) {
	/* Determine whether an object is an internal one */
	bool_t result = false;

	mb_assert(obj);

	result = (_exp_assign == obj) ||
		(_OBJ_BOOL_TRUE == obj) || (_OBJ_BOOL_FALSE == obj);

	return result;
}

int _dispose_object(_object_t* obj) {
	/* Dispose a syntax object */
	int result = 0;
	_var_t* var = 0;

	mb_assert(obj);

	if(_is_internal_object(obj))
		goto _exit;
	switch(obj->type) {
	case _DT_VAR:
		if(!obj->ref) {
			var = (_var_t*)(obj->data.variable);
			safe_free(var->name);
			mb_assert(var->data->type != _DT_VAR);
			_destroy_object(var->data, 0);
			safe_free(var);
		}

		break;
	case _DT_STRING:
		if(!obj->ref) {
			if(obj->data.string) {
				safe_free(obj->data.string);
			}
		}

		break;
	case _DT_FUNC:
		safe_free(obj->data.func->name);
		safe_free(obj->data.func);

		break;
	case _DT_ARRAY:
		if(!obj->ref)
			_destroy_array(obj->data.array);

		break;
	case _DT_LABEL:
		if(!obj->ref) {
			safe_free(obj->data.label->name);
			safe_free(obj->data.label);
		}

		break;
	case _DT_NIL: /* Fall through */
	case _DT_INT: /* Fall through */
	case _DT_REAL: /* Fall through */
	case _DT_SEP: /* Fall through */
	case _DT_EOS: /* Fall through */
	case _DT_USERTYPE: /* Do nothing */
		break;
	default:
		mb_assert(0 && "Invalid type");

		break;
	}
	obj->ref = false;
	obj->type = _DT_NIL;
	memset(&obj->data, 0, sizeof(obj->data));
#ifdef MB_ENABLE_SOURCE_TRACE
	obj->source_pos = 0;
	obj->source_row = 0;
	obj->source_col = 0;
#else /* MB_ENABLE_SOURCE_TRACE */
	obj->source_pos = 0;
#endif /* MB_ENABLE_SOURCE_TRACE */
	++result;

_exit:
	return result;
}

int _destroy_object(void* data, void* extra) {
	/* Destroy a syntax object */
	int result = _OP_RESULT_NORMAL;
	_object_t* obj = 0;
	mb_unrefvar(extra);

	mb_assert(data);

	obj = (_object_t*)data;
	if(!_dispose_object(obj))
		goto _exit;
	safe_free(obj);

_exit:
	result = _OP_RESULT_DEL_NODE;

	return result;
}

int _destroy_object_non_syntax(void* data, void* extra) {
	/* Destroy a non syntax object */
	int result = _OP_RESULT_NORMAL;
	_object_t* obj = 0;
	mb_unrefvar(extra);

	mb_assert(data);

	obj = (_object_t*)data;
	if(!obj->source_pos) {
		if(!_dispose_object(obj))
			goto _exit;
		safe_free(obj);
	}

_exit:
	result = _OP_RESULT_DEL_NODE;

	return result;
}

int _remove_source_object(void* data, void* extra) {
	/* Remove an object referenced from source code */
	int result = _OP_RESULT_DEL_NODE;
	mb_unrefvar(extra);

	mb_assert(data);

	return result;
}

int _compare_numbers(const _object_t* first, const _object_t* second) {
	/* Compare two numbers inside two _object_t */
	int result = 0;

	mb_assert(first && second);
	mb_assert((first->type == _DT_INT || first->type == _DT_REAL) && (second->type == _DT_INT || second->type == _DT_REAL));

	if(first->type == _DT_INT && second->type == _DT_INT) {
		if(first->data.integer > second->data.integer)
			result = 1;
		else if(first->data.integer < second->data.integer)
			result = -1;
	} else if(first->type == _DT_REAL && second->type == _DT_REAL) {
		if(first->data.float_point > second->data.float_point)
			result = 1;
		else if(first->data.float_point < second->data.float_point)
			result = -1;
	} else {
		if((first->type == _DT_INT ? (real_t)first->data.integer : first->data.float_point) > (second->type == _DT_INT ? (real_t)second->data.integer : second->data.float_point))
			result = 1;
		else if((first->type == _DT_INT ? (real_t)first->data.integer : first->data.float_point) > (second->type == _DT_INT ? (real_t)second->data.integer : second->data.float_point))
			result = -1;
	}

	return result;
}

_data_e _public_type_to_internal_type(mb_data_e t) {
	/* Convert a public mb_data_e type to an internal _data_e */
	switch(t) {
	case MB_DT_INT:
		return _DT_INT;
	case MB_DT_REAL:
		return _DT_REAL;
	case MB_DT_STRING:
		return _DT_STRING;
	case MB_DT_USERTYPE:
		return _DT_USERTYPE;
	default:
		return _DT_NIL;
	}
}

mb_data_e _internal_type_to_public_type(_data_e t) {
	/* Convert an internal mb_data_e type to a public _data_e */
	switch(t) {
	case _DT_INT:
		return MB_DT_INT;
	case _DT_REAL:
		return MB_DT_REAL;
	case _DT_STRING:
		return MB_DT_STRING;
	case _DT_USERTYPE:
		return MB_DT_USERTYPE;
	default:
		return MB_DT_NIL;
	}
}

int _public_value_to_internal_object(mb_value_t* pbl, _object_t* itn) {
	/* Assign a public mb_value_t to an internal _object_t */
	int result = MB_FUNC_OK;

	mb_assert(pbl && itn);

	switch(pbl->type) {
	case MB_DT_INT:
		itn->type = _DT_INT;
		itn->data.integer = pbl->value.integer;

		break;
	case MB_DT_REAL:
		itn->type = _DT_REAL;
		itn->data.float_point = pbl->value.float_point;

		break;
	case MB_DT_STRING:
		itn->type = _DT_STRING;
		itn->data.string = pbl->value.string;

		break;
	case MB_DT_USERTYPE:
		itn->type = _DT_USERTYPE;
		itn->data.usertype = pbl->value.usertype;

		break;
	case MB_DT_ARRAY:
		itn->type = _DT_ARRAY;
		itn->data.array = pbl->value.array;

		break;
	default:
		result = MB_FUNC_ERR;

		break;
	}

	return result;
}

int _internal_object_to_public_value(_object_t* itn, mb_value_t* pbl) {
	/* Assign an internal _object_t to a public mb_value_t */
	int result = MB_FUNC_OK;

	mb_assert(pbl && itn);

	switch(itn->type) {
	case _DT_INT:
		pbl->type = MB_DT_INT;
		pbl->value.integer = itn->data.integer;

		break;
	case _DT_REAL:
		pbl->type = MB_DT_REAL;
		pbl->value.float_point = itn->data.float_point;

		break;
	case _DT_STRING:
		pbl->type = MB_DT_STRING;
		pbl->value.string = itn->data.string;

		break;
	case _DT_USERTYPE:
		pbl->type = MB_DT_USERTYPE;
		pbl->value.usertype = itn->data.usertype;

		break;
	case _DT_ARRAY:
		pbl->type = MB_DT_ARRAY;
		pbl->value.array = itn->data.array;

		break;
	default:
		result = MB_FUNC_ERR;

		break;
	}

	return result;
}

void _try_clear_intermediate_value(void* data, void* extra, mb_interpreter_t* s) {
	/* Try clear the intermediate value when destroying an object */
	_object_t* obj = 0;
	_running_context_t* running = 0;
	mb_unrefvar(extra);

	mb_assert(s);

	if(data == 0)
		return;

	obj = (_object_t*)data;
	running = s->running_context;

	if(obj->type == _DT_STRING && running->intermediate_value.type == MB_DT_STRING && obj->data.string == running->intermediate_value.value.string)
		running->intermediate_value.type = MB_DT_NIL;
}

void _stepped(mb_interpreter_t* s, _ls_node_t* ast) {
	/* Called each step */
	_object_t* obj = 0;

	mb_assert(s);

	if(s->debug_stepped_handler) {
		if(ast && ast->data) {
			obj = (_object_t*)ast->data;
#ifdef MB_ENABLE_SOURCE_TRACE
			s->debug_stepped_handler(s, obj->source_pos, obj->source_row, obj->source_col);
#else /* MB_ENABLE_SOURCE_TRACE */
			s->debug_stepped_handler(s, obj->source_pos, 0, 0);
#endif /* MB_ENABLE_SOURCE_TRACE */
		} else {
			s->debug_stepped_handler(s, -1, 0, 0);
		}
	}
}

int _execute_statement(mb_interpreter_t* s, _ls_node_t** l) {
	/* Execute the ast, core execution function */
	int result = MB_FUNC_OK;
	_ls_node_t* ast = 0;
	_object_t* obj = 0;
	_running_context_t* running = 0;
	bool_t skip_to_eoi = true;

	mb_assert(s && l);

	running = s->running_context;

	ast = *l;

	obj = (_object_t*)(ast->data);
	switch(obj->type) {
	case _DT_FUNC:
		result = (obj->data.func->pointer)(s, (void**)(&ast));

		break;
	case _DT_VAR: /* Fall through */
	case _DT_ARRAY:
		result = _core_let(s, (void**)(&ast));

		break;
	case _DT_INT: /* Fall through */
	case _DT_REAL: /* Fall through */
	case _DT_STRING:
		_handle_error_on_obj(s, SE_RN_INVALID_EXPRESSION, DON(ast), MB_FUNC_ERR, _exit, result);

		break;
	default:
		break;
	}

	if(running->schedule_suspend_tag) {
		if(running->schedule_suspend_tag == MB_FUNC_SUSPEND)
			mb_suspend(s, (void**)(&ast));
		result = running->schedule_suspend_tag;
		running->schedule_suspend_tag = 0;
	}

	if(result != MB_FUNC_OK && result != MB_FUNC_SUSPEND && result != MB_SUB_RETURN)
		goto _exit;

	if(ast) {
		obj = (_object_t*)(ast->data);
		if(_IS_EOS(obj)) {
			ast = ast->next;
		} else if(_IS_SEP(obj, ':')) {
			skip_to_eoi = false;
			ast = ast->next;
		} else if(obj && obj->type == _DT_VAR) {
			_handle_error_on_obj(s, SE_RN_COLON_EXPECTED, DON(ast), MB_FUNC_ERR, _exit, result);
		} else if((obj && obj->type != _DT_FUNC) || (obj && obj->type == _DT_FUNC && (_is_operator(obj->data.func->pointer) || _is_flow(obj->data.func->pointer)))) {
			ast = ast->next;
		} else {
			_handle_error_on_obj(s, SE_RN_COLON_EXPECTED, DON(ast), MB_FUNC_ERR, _exit, result);
		}
	}

	if(skip_to_eoi && running->skip_to_eoi && running->skip_to_eoi == _ls_back(running->sub_stack)) {
		running->skip_to_eoi = 0;
		obj = (_object_t*)(ast->data);
		if(obj->type != _DT_EOS) {
			result = _skip_to(s, &ast, 0, _DT_EOS);
			if(result != MB_FUNC_OK)
				goto _exit;
		}
	}

_exit:
	*l = ast;

	_stepped(s, ast);

	return result;
}

int _skip_to(mb_interpreter_t* s, _ls_node_t** l, mb_func_t f, _data_e t) {
	/* Skip current execution flow to a specific function */
	int result = MB_FUNC_OK;
	_ls_node_t* ast = 0;
	_ls_node_t* tmp = 0;
	_object_t* obj = 0;

	mb_assert(s && l);

	ast = *l;
	mb_assert(ast && ast->prev);
	do {
		if(!ast) {
			_handle_error_on_obj(s, SE_RN_SYNTAX, DON(tmp), MB_FUNC_ERR, _exit, result);
		}
		tmp = ast;
		obj = (_object_t*)(ast->data);
		*l = ast;
		ast = ast->next;
	} while(!(obj->type == _DT_FUNC && obj->data.func->pointer == f) && obj->type != t);

_exit:
	return result;
}

int _skip_if_chunk(mb_interpreter_t* s, _ls_node_t** l) {
	/* Skip current IF execution flow to next chunk */
	int result = MB_FUNC_OK;
	_ls_node_t* ast = 0;
	_ls_node_t* tmp = 0;
	_object_t* obj = 0;

	mb_assert(s && l);

	ast = *l;
	mb_assert(ast && ast->prev);
	do {
		if(!ast) {
			_handle_error_on_obj(s, SE_RN_SYNTAX, DON(tmp), MB_FUNC_ERR, _exit, result);
		}
		tmp = ast;
		obj = (_object_t*)(ast->data);
		*l = ast;
		ast = ast->next;
	} while(!_IS_FUNC(obj, _core_if) && !_IS_FUNC(obj, _core_elseif) && !_IS_FUNC(obj, _core_else));

_exit:
	return result;
}

int _skip_struct(mb_interpreter_t* s, _ls_node_t** l, mb_func_t open_func, mb_func_t close_func) {
	/* Skip current structure */
	int result = MB_FUNC_OK;
	int count = 0;
	_ls_node_t* ast = 0;
	_object_t* obj = 0;
	_object_t* obj_prev = 0;

	mb_assert(s && l && open_func && close_func);

	ast = (_ls_node_t*)(*l);

	count = 1;
	do {
		if(!ast->next) {
			_handle_error_on_obj(s, SE_RN_STRUCTURE_NOT_COMPLETED, DON(ast), MB_FUNC_ERR, _exit, result);
		}
		obj_prev = (_object_t*)(ast->data);
		ast = ast->next;
		obj = (_object_t*)(ast->data);
		if(_IS_FUNC(obj, open_func))
			++count;
		else if(_IS_FUNC(obj, close_func) && _IS_EOS(obj_prev))
			--count;
	} while(count);

_exit:
	*l = ast;

	return result;
}

_parsing_context_t* _reset_parsing_context(_parsing_context_t* context) {
	if(!context)
		context = (_parsing_context_t*)mb_malloc(sizeof(_parsing_context_t));
	memset(context, 0, sizeof(_parsing_context_t));
	context->parsing_row = 1;

	return context;
}

int _register_func(mb_interpreter_t* s, const char* n, mb_func_t f, bool_t local) {
	/* Register a function to a MY-BASIC environment */
	int result = 0;
	_ht_node_t* scope = 0;
	_ls_node_t* exists = 0;
	char* name = 0;

	mb_assert(s);

	if(!n)
		return result;

	scope = local ? s->local_func_dict : s->global_func_dict;
	exists = _ht_find(scope, (void*)n);
	if(!exists) {
		size_t _sl = strlen(n);
		name = (char*)mb_malloc(_sl + 1);
		memcpy(name, n, _sl + 1);
		mb_strupr(name);
		result += _ht_set_or_insert(scope, (void*)name, (void*)(intptr_t)f);
	} else {
		_set_current_error(s, SE_CM_FUNC_EXISTS);
	}

	return result;
}

int _remove_func(mb_interpreter_t* s, const char* n, bool_t local) {
	/* Remove a function from a MY-BASIC environment */
	int result = 0;
	_ht_node_t* scope = 0;
	_ls_node_t* exists = 0;
	char* name = 0;

	mb_assert(s);

	if(!n)
		return result;

	scope = local ? s->local_func_dict : s->global_func_dict;
	exists = _ht_find(scope, (void*)n);
	if(exists) {
		size_t _sl = strlen(n);
		name = (char*)mb_malloc(_sl + 1);
		memcpy(name, n, _sl + 1);
		mb_strupr(name);
		result += _ht_remove(scope, (void*)name, _ls_cmp_extra_string);
		safe_free(name);
	} else {
		_set_current_error(s, SE_CM_FUNC_NOT_EXISTS);
	}

	return result;
}

int _open_constant(mb_interpreter_t* s) {
	/* Open global constant */
	int result = MB_FUNC_OK;
	unsigned long ul = 0;

	mb_assert(s);

	ul = _ht_set_or_insert(s->global_var_dict, "TRUE", _OBJ_BOOL_TRUE);
	mb_assert(ul);
	ul = _ht_set_or_insert(s->global_var_dict, "FALSE", _OBJ_BOOL_FALSE);
	mb_assert(ul);

	return result;
}

int _close_constant(mb_interpreter_t* s) {
	/* Close global constant */
	int result = MB_FUNC_OK;

	mb_assert(s);

	return result;
}

int _open_core_lib(mb_interpreter_t* s) {
	/* Open the core functional libraries */
	int result = 0;
	int i = 0;

	mb_assert(s);

	for(i = 0; i < _countof(_core_libs); ++i)
		result += _register_func(s, _core_libs[i].name, _core_libs[i].pointer, true);

	return result;
}

int _close_core_lib(mb_interpreter_t* s) {
	/* Close the core functional libraries */
	int result = 0;
	int i = 0;

	mb_assert(s);

	for(i = 0; i < _countof(_core_libs); ++i)
		result += _remove_func(s, _core_libs[i].name, true);

	return result;
}

int _open_std_lib(mb_interpreter_t* s) {
	/* Open the standard functional libraries */
	int result = 0;
	int i = 0;

	mb_assert(s);

	for(i = 0; i < _countof(_std_libs); ++i)
		result += _register_func(s, _std_libs[i].name, _std_libs[i].pointer, true);

	return result;
}

int _close_std_lib(mb_interpreter_t* s) {
	/* Close the standard functional libraries */
	int result = 0;
	int i = 0;

	mb_assert(s);

	for(i = 0; i < _countof(_std_libs); ++i)
		result += _remove_func(s, _std_libs[i].name, true);

	return result;
}

/* ========================================================} */

/*
** {========================================================
** Public functions definitions
*/

unsigned int mb_ver(void) {
	/* Get the version number of this MY-BASIC system */
	return _MB_VERSION;
}

const char* mb_ver_string(void) {
	/* Get the version text of this MY-BASIC system */
	static char buf[32] = { '\0' };
	if(!buf[0])
		sprintf(buf, "%d.%d.%04d", _VER_MAJOR, _VER_MINOR, _VER_REVISION);

	return buf;
}

int mb_init(void) {
	/* Initialize the MY-BASIC system */
	int result = MB_FUNC_OK;

	mb_assert(!_exp_assign);
	_exp_assign = (_object_t*)mb_malloc(sizeof(_object_t));
	memset(_exp_assign, 0, sizeof(_object_t));
	_exp_assign->type = _DT_FUNC;
	_exp_assign->data.func = (_func_t*)mb_malloc(sizeof(_func_t));
	memset(_exp_assign->data.func, 0, sizeof(_func_t));
	_exp_assign->data.func->name = (char*)mb_malloc(strlen("#") + 1);
	memcpy(_exp_assign->data.func->name, "#\0", strlen("#") + 1);
	_exp_assign->data.func->pointer = _core_dummy_assign;

	mb_assert(!_OBJ_BOOL_TRUE);
	if(!_OBJ_BOOL_TRUE) {
		_OBJ_BOOL_TRUE = (_object_t*)mb_malloc(sizeof(_object_t));
		memset(_OBJ_BOOL_TRUE, 0, sizeof(_object_t));

		_OBJ_BOOL_TRUE->type = _DT_VAR;
		_OBJ_BOOL_TRUE->data.variable = (_var_t*)mb_malloc(sizeof(_var_t));
		memset(_OBJ_BOOL_TRUE->data.variable, 0, sizeof(_var_t));
		_OBJ_BOOL_TRUE->data.variable->name = (char*)mb_malloc(strlen("TRUE") + 1);
		memset(_OBJ_BOOL_TRUE->data.variable->name, 0, strlen("TRUE") + 1);
		strcpy(_OBJ_BOOL_TRUE->data.variable->name, "TRUE");

		_OBJ_BOOL_TRUE->data.variable->data = (_object_t*)mb_malloc(sizeof(_object_t));
		memset(_OBJ_BOOL_TRUE->data.variable->data, 0, sizeof(_object_t));
		_OBJ_BOOL_TRUE->data.variable->data->type = _DT_INT;
		_OBJ_BOOL_TRUE->data.variable->data->data.integer = 1;
	}
	mb_assert(!_OBJ_BOOL_FALSE);
	if(!_OBJ_BOOL_FALSE) {
		_OBJ_BOOL_FALSE = (_object_t*)mb_malloc(sizeof(_object_t));
		memset(_OBJ_BOOL_FALSE, 0, sizeof(_object_t));

		_OBJ_BOOL_FALSE->type = _DT_VAR;
		_OBJ_BOOL_FALSE->data.variable = (_var_t*)mb_malloc(sizeof(_var_t));
		memset(_OBJ_BOOL_FALSE->data.variable, 0, sizeof(_var_t));
		_OBJ_BOOL_FALSE->data.variable->name = (char*)mb_malloc(strlen("FALSE") + 1);
		memset(_OBJ_BOOL_FALSE->data.variable->name, 0, strlen("FALSE") + 1);
		strcpy(_OBJ_BOOL_FALSE->data.variable->name, "FALSE");

		_OBJ_BOOL_FALSE->data.variable->data = (_object_t*)mb_malloc(sizeof(_object_t));
		memset(_OBJ_BOOL_FALSE->data.variable->data, 0, sizeof(_object_t));
		_OBJ_BOOL_FALSE->data.variable->data->type = _DT_INT;
		_OBJ_BOOL_FALSE->data.variable->data->data.integer = 0;
	}

	return result;
}

int mb_dispose(void) {
	/* Close the MY-BASIC system */
	int result = MB_FUNC_OK;

	mb_assert(_exp_assign);
	safe_free(_exp_assign->data.func->name);
	safe_free(_exp_assign->data.func);
	safe_free(_exp_assign);
	_exp_assign = 0;

	mb_assert(_OBJ_BOOL_TRUE);
	if(_OBJ_BOOL_TRUE) {
		safe_free(_OBJ_BOOL_TRUE->data.variable->data);
		safe_free(_OBJ_BOOL_TRUE->data.variable->name);
		safe_free(_OBJ_BOOL_TRUE->data.variable);
		safe_free(_OBJ_BOOL_TRUE);
		_OBJ_BOOL_TRUE = 0;
	}
	mb_assert(_OBJ_BOOL_FALSE);
	if(_OBJ_BOOL_FALSE) {
		safe_free(_OBJ_BOOL_FALSE->data.variable->data);
		safe_free(_OBJ_BOOL_FALSE->data.variable->name);
		safe_free(_OBJ_BOOL_FALSE->data.variable);
		safe_free(_OBJ_BOOL_FALSE);
		_OBJ_BOOL_FALSE = 0;
	}

	return result;
}

int mb_open(struct mb_interpreter_t** s) {
	/* Initialize a MY-BASIC environment */
	int result = MB_FUNC_OK;
	_ht_node_t* local_scope = 0;
	_ht_node_t* global_scope = 0;
	_ls_node_t* ast = 0;
	_parsing_context_t* context = 0;
	_running_context_t* running = 0;

	*s = (mb_interpreter_t*)mb_malloc(sizeof(mb_interpreter_t));
	memset(*s, 0, sizeof(mb_interpreter_t));

	local_scope = _ht_create(0, _ht_cmp_string, _ht_hash_string, _ls_free_extra);
	(*s)->local_func_dict = local_scope;

	global_scope = _ht_create(0, _ht_cmp_string, _ht_hash_string, _ls_free_extra);
	(*s)->global_func_dict = global_scope;

	global_scope = _ht_create(0, _ht_cmp_string, _ht_hash_string, 0);
	(*s)->global_var_dict = global_scope;

	ast = _ls_create();
	(*s)->ast = ast;

	(*s)->parsing_context = context = _reset_parsing_context((*s)->parsing_context);

	running = (_running_context_t*)mb_malloc(sizeof(_running_context_t));
	memset(running, 0, sizeof(_running_context_t));

	running->temp_values = _ls_create();
	running->in_neg_expr = _ls_create();

	running->sub_stack = _ls_create();
	(*s)->running_context = running;

	_open_core_lib(*s);
	_open_std_lib(*s);

	result = _open_constant(*s);
	mb_assert(MB_FUNC_OK == result);

	return result;
}

int mb_close(struct mb_interpreter_t** s) {
	/* Close a MY-BASIC environment */
	int result = MB_FUNC_OK;
	_ht_node_t* local_scope = 0;
	_ht_node_t* global_scope = 0;
	_ls_node_t* ast;
	_parsing_context_t* context = 0;
	_running_context_t* running = 0;

	mb_assert(s);

	_close_std_lib(*s);
	_close_core_lib(*s);

	running = (*s)->running_context;

	mb_dispose_value(*s, running->intermediate_value);
	_ls_foreach(running->temp_values, _destroy_object);
	_ls_destroy(running->temp_values);
	_ls_destroy(running->in_neg_expr);

	_ls_destroy(running->sub_stack);
	safe_free(running);

	context = (*s)->parsing_context;
	if(context) {
		safe_free(context);
	}

	ast = (*s)->ast;
	_ls_foreach(ast, _destroy_object);
	_ls_destroy(ast);

	global_scope = (*s)->global_var_dict;
	_ht_foreach(global_scope, _destroy_object);
	_ht_destroy(global_scope);

	global_scope = (*s)->global_func_dict;
	_ht_foreach(global_scope, _ls_free_extra);
	_ht_destroy(global_scope);

	local_scope = (*s)->local_func_dict;
	_ht_foreach(local_scope, _ls_free_extra);
	_ht_destroy(local_scope);

	_close_constant(*s);

	safe_free(*s);
	*s = 0;

	return result;
}

int mb_reset(struct mb_interpreter_t** s, bool_t clrf/* = false*/) {
	/* Reset a MY-BASIC environment */
	int result = MB_FUNC_OK;
	_ht_node_t* global_scope = 0;
	_ls_node_t* ast;
	_parsing_context_t* context = 0;
	_running_context_t* running = 0;

	mb_assert(s);

	(*s)->last_error = SE_NO_ERR;

	running = (*s)->running_context;
	_ls_clear(running->sub_stack);
	running->suspent_point = 0;
	running->next_loop_var = 0;
	running->no_eat_comma_mark = 0;
	memset(&(running->intermediate_value), 0, sizeof(mb_value_t));

	(*s)->parsing_context = context = _reset_parsing_context((*s)->parsing_context);

	ast = (*s)->ast;
	_ls_foreach(ast, _destroy_object);
	_ls_clear(ast);

	global_scope = (*s)->global_var_dict;
	_ht_foreach(global_scope, _destroy_object);
	_ht_clear(global_scope);

	if(clrf) {
		global_scope = (*s)->global_func_dict;
		_ht_foreach(global_scope, _ls_free_extra);
		_ht_clear(global_scope);
	}

	result = _open_constant(*s);
	mb_assert(MB_FUNC_OK == result);

	return result;
}

int mb_register_func(struct mb_interpreter_t* s, const char* n, mb_func_t f) {
	/* Register a remote function to a MY-BASIC environment */
	return _register_func(s, n, f, false);
}

int mb_remove_func(struct mb_interpreter_t* s, const char* n) {
	/* Remove a remote function from a MY-BASIC environment */
	return _remove_func(s, n, false);
}

int mb_remove_reserved_func(struct mb_interpreter_t* s, const char* n) {
	/* Remove a reserved remote function from a MY-BASIC environment */
	return _remove_func(s, n, true);
}

int mb_attempt_func_begin(struct mb_interpreter_t* s, void** l) {
	/* Try attempting to begin a function */
	int result = MB_FUNC_OK;
	_ls_node_t* ast = 0;
	_object_t* obj = 0;
	_running_context_t* running = 0;

	mb_assert(s && l);

	ast = (_ls_node_t*)(*l);
	obj = (_object_t*)(ast->data);
	if(!(obj->type == _DT_FUNC)) {
		_handle_error_on_obj(s, SE_RN_STRUCTURE_NOT_COMPLETED, DON(ast), MB_FUNC_ERR, _exit, result);
	}
	ast = ast->next;

	running = s->running_context;
	++running->no_eat_comma_mark;

_exit:
	*l = ast;

	return result;
}

int mb_attempt_func_end(struct mb_interpreter_t* s, void** l) {
	/* Try attempting to end a function */
	int result = MB_FUNC_OK;
	_running_context_t* running = 0;

	mb_assert(s && l);

	running = s->running_context;
	--running->no_eat_comma_mark;

	return result;
}

int mb_attempt_open_bracket(struct mb_interpreter_t* s, void** l) {
	/* Try attempting an open bracket */
	int result = MB_FUNC_OK;
	_ls_node_t* ast = 0;
	_object_t* obj = 0;

	mb_assert(s && l);

	ast = (_ls_node_t*)(*l);
	ast = ast->next;
	obj = (_object_t*)(ast->data);
	if(!_IS_FUNC(obj, _core_open_bracket)) {
		_handle_error_on_obj(s, SE_RN_OPEN_BRACKET_EXPECTED, DON(ast), MB_FUNC_ERR, _exit, result);
	}
	ast = ast->next;

_exit:
	*l = ast;

	return result;
}

int mb_attempt_close_bracket(struct mb_interpreter_t* s, void** l) {
	/* Try attempting a close bracket */
	int result = MB_FUNC_OK;
	_ls_node_t* ast = 0;
	_object_t* obj = 0;

	mb_assert(s && l);

	ast = (_ls_node_t*)(*l);
	if(!ast) {
		_handle_error_on_obj(s, SE_RN_CLOSE_BRACKET_EXPECTED, DON(ast), MB_FUNC_ERR, _exit, result);
	}
	obj = (_object_t*)(ast->data);
	if(!_IS_FUNC(obj, _core_close_bracket)) {
		_handle_error_on_obj(s, SE_RN_CLOSE_BRACKET_EXPECTED, DON(ast), MB_FUNC_ERR, _exit, result);
	}
	ast = ast->next;

_exit:
	*l = ast;

	return result;
}

int mb_has_arg(struct mb_interpreter_t* s, void** l) {
	/* Detect whether there is any more argument */
	int result = 0;
	_ls_node_t* ast = 0;
	_object_t* obj = 0;

	mb_assert(s && l);

	ast = (_ls_node_t*)(*l);
	if(ast) {
		obj = (_object_t*)(ast->data);
		if(!_IS_FUNC(obj, _core_close_bracket) && obj->type != _DT_EOS)
			result = obj->data.integer;
	}

	return result;
}

int mb_pop_int(struct mb_interpreter_t* s, void** l, int_t* val) {
	/* Pop an integer argument */
	int result = MB_FUNC_OK;
	mb_value_t arg;
	int_t tmp = 0;

	mb_assert(s && l && val);

	mb_check(mb_pop_value(s, l, &arg));

	switch(arg.type) {
	case MB_DT_INT:
		tmp = arg.value.integer;

		break;
	case MB_DT_REAL:
		tmp = (int_t)(arg.value.float_point);

		break;
	default:
		result = MB_FUNC_ERR;

		goto _exit;
	}

	*val = tmp;

_exit:
	return result;
}

int mb_pop_real(struct mb_interpreter_t* s, void** l, real_t* val) {
	/* Pop a float point argument */
	int result = MB_FUNC_OK;
	mb_value_t arg;
	real_t tmp = 0;

	mb_assert(s && l && val);

	mb_check(mb_pop_value(s, l, &arg));

	switch(arg.type) {
	case MB_DT_INT:
		tmp = (real_t)(arg.value.integer);

		break;
	case MB_DT_REAL:
		tmp = arg.value.float_point;

		break;
	default:
		result = MB_FUNC_ERR;

		goto _exit;
	}

	*val = tmp;

_exit:
	return result;
}

int mb_pop_string(struct mb_interpreter_t* s, void** l, char** val) {
	/* Pop a string argument */
	int result = MB_FUNC_OK;
	mb_value_t arg;
	char* tmp = 0;

	mb_assert(s && l && val);

	mb_check(mb_pop_value(s, l, &arg));

	switch(arg.type) {
	case MB_DT_STRING:
		tmp = arg.value.string;

		break;
	default:
		result = MB_FUNC_ERR;

		goto _exit;
	}

	*val = tmp;

_exit:
	return result;
}

int mb_pop_usertype(struct mb_interpreter_t* s, void** l, void** val) {
	/* Pop a usertype argument */
	int result = MB_FUNC_OK;
	mb_value_t arg;
	void* tmp = 0;

	mb_assert(s && l && val);

	mb_check(mb_pop_value(s, l, &arg));

	switch(arg.type) {
	case MB_DT_USERTYPE:
		tmp = arg.value.usertype;

		break;
	default:
		result = MB_FUNC_ERR;

		goto _exit;
	}

	*val = tmp;

_exit:
	return result;
}

int mb_pop_value(struct mb_interpreter_t* s, void** l, mb_value_t* val) {
	/* Pop an argument */
	int result = MB_FUNC_OK;
	_ls_node_t* ast = 0;
	_object_t val_obj;
	_object_t* val_ptr = 0;
	_running_context_t* running = 0;
	int* inep = 0;

	mb_assert(s && l && val);

	running = s->running_context;

	if(!_ls_empty(running->in_neg_expr))
		inep = (int*)_ls_back(running->in_neg_expr)->data;

	val_ptr = &val_obj;
	memset(val_ptr, 0, sizeof(_object_t));

	ast = (_ls_node_t*)(*l);
	result = _calc_expression(s, &ast, &val_ptr);
	if(result != MB_FUNC_OK)
		goto _exit;

	if(val_ptr->type == _DT_STRING && !val_ptr->ref) {
		val_ptr = (_object_t*)mb_malloc(sizeof(_object_t));
		memcpy(val_ptr, &val_obj, sizeof(_object_t));
		_ls_pushback(running->temp_values, val_ptr);
	}

	if(running->no_eat_comma_mark < _NO_EAT_COMMA && (!inep || (inep && !(*inep)))) {
		if(ast && _IS_SEP(ast->data, ','))
			ast = ast->next;
	}

	result = _internal_object_to_public_value(val_ptr, val);
	if(result != MB_FUNC_OK)
		goto _exit;

_exit:
	*l = ast;

	return result;
}

int mb_push_int(struct mb_interpreter_t* s, void** l, int_t val) {
	/* Push an integer argument */
	int result = MB_FUNC_OK;
	mb_value_t arg;

	mb_assert(s && l);

	arg.type = MB_DT_INT;
	arg.value.integer = val;
	mb_check(mb_push_value(s, l, arg));

	return result;
}

int mb_push_real(struct mb_interpreter_t* s, void** l, real_t val) {
	/* Push a float point argument */
	int result = MB_FUNC_OK;
	mb_value_t arg;

	mb_assert(s && l);

	arg.type = MB_DT_REAL;
	arg.value.float_point = val;
	mb_check(mb_push_value(s, l, arg));

	return result;
}

int mb_push_string(struct mb_interpreter_t* s, void** l, char* val) {
	/* Push a string argument */
	int result = MB_FUNC_OK;
	mb_value_t arg;

	mb_assert(s && l);

	arg.type = MB_DT_STRING;
	arg.value.string = val;
	mb_check(mb_push_value(s, l, arg));

	return result;
}

int mb_push_usertype(struct mb_interpreter_t* s, void** l, void* val) {
	/* Push a usertype argument */
	int result = MB_FUNC_OK;
	mb_value_t arg;

	mb_assert(s && l);

	arg.type = MB_DT_USERTYPE;
	arg.value.usertype = val;
	mb_check(mb_push_value(s, l, arg));

	return result;
}

int mb_push_value(struct mb_interpreter_t* s, void** l, mb_value_t val) {
	/* Push an argument */
	int result = MB_FUNC_OK;
	_running_context_t* running = 0;

	mb_assert(s && l);

	running = s->running_context;
	running->intermediate_value = val;

	return result;
}

int mb_init_array(struct mb_interpreter_t* s, void** l, mb_data_e t, int* d, int c, void** a) {
	/* Create an array */
	int result = MB_FUNC_OK;
	_array_t* arr = 0;
	_data_e type = _DT_NIL;
	int j = 0;
	int n = 0;

	mb_assert(s && l && d && a);

	*a = 0;
#ifdef MB_SIMPLE_ARRAY
	if(t == MB_DT_REAL) {
		type = _DT_REAL;
	} else if(t == MB_DT_STRING) {
		type = _DT_STRING;
	} else {
		result = MB_NEED_COMPLEX_ARRAY;

		goto _exit;
	}
#else /* MB_SIMPLE_ARRAY */
	type = _DT_REAL;
#endif /* MB_SIMPLE_ARRAY */

	arr = (_array_t*)mb_malloc(sizeof(_array_t));
	memset(arr, 0, sizeof(_array_t));
	arr->type = type;
	arr->name = 0;
	for(j = 0; j < c; j++) {
		n = d[j];
		arr->dimensions[arr->dimension_count++] = n;
		if(arr->count)
			arr->count *= n;
		else
			arr->count += n;
	}
	_init_array(arr);
	if(!arr->raw) {
		arr->dimension_count = 0;
		arr->dimensions[0] = 0;
		arr->count = 0;
	}
	*a = arr;

_exit:
	return result;
}

int mb_get_array_len(struct mb_interpreter_t* s, void** l, void* a, int r, int* i) {
	/* Get the length of an array */
	int result = MB_FUNC_OK;
	_array_t* arr = 0;

	mb_assert(s && l);
	arr = (_array_t*)a;
	if(r < 0 || r >= arr->dimension_count) {
		result = MB_RANK_OUT_OF_BOUNDS;

		goto _exit;
	}
	if(i)
		*i = arr->dimensions[r];

_exit:
	return result;
}

int mb_get_array_elem(struct mb_interpreter_t* s, void** l, void* a, int* d, int c, mb_value_t* val) {
	/* Get an element of an array with a given index */
	int result = MB_FUNC_OK;
	_array_t* arr = 0;
	int index = 0;
	_data_e type = _DT_NIL;

	mb_assert(s && l);
	arr = (_array_t*)a;
	if(c < 0 || c > arr->dimension_count) {
		result = MB_RANK_OUT_OF_BOUNDS;

		goto _exit;
	}
	if(!val)
		goto _exit;

	index = _get_array_pos(s, arr, d, c);
	if(index < 0) {
		result = MB_INDEX_OUT_OF_BOUNDS;

		goto _exit;
	}

	_get_array_elem(s, arr, index, &val->value, &type);
	val->type = _internal_type_to_public_type(type);

_exit:
	return result;
}

int mb_set_array_elem(struct mb_interpreter_t* s, void** l, void* a, int* d, int c, mb_value_t val) {
	/* Set an element of an array with a given index */
	int result = MB_FUNC_OK;
	_array_t* arr = 0;
	int index = 0;
	_data_e type = _DT_NIL;

	mb_assert(s && l);
	arr = (_array_t*)a;
	if(c < 0 || c > arr->dimension_count) {
		result = MB_RANK_OUT_OF_BOUNDS;

		goto _exit;
	}

	index = _get_array_pos(s, arr, d, c);
	if(index < 0) {
		result = MB_INDEX_OUT_OF_BOUNDS;

		goto _exit;
	}

	type = _public_type_to_internal_type(val.type);
	_set_array_elem(s, 0, arr, (unsigned int)index, &val.value, &type);

_exit:
	return result;
}

int mb_dispose_value(struct mb_interpreter_t* s, mb_value_t val) {
	/* Dispose a value */
	int result = MB_FUNC_OK;

	mb_assert(s);

	if(val.type == MB_DT_STRING)
		safe_free(val.value.string);

	return result;
}

int mb_load_string(struct mb_interpreter_t* s, const char* l) {
	/* Load a script string */
	int result = MB_FUNC_OK;
	char ch = 0;
	int status = 0;
	int i = 0;
	unsigned short _row = 0;
	unsigned short _col = 0;
	char wrapped = '\0';
	_parsing_context_t* context = 0;

	mb_assert(s && s->parsing_context);

	context = s->parsing_context;

	while(l[i]) {
		ch = l[i];
		if((ch == '\n' || ch == '\r') && (!wrapped || wrapped == ch)) {
			wrapped = ch;
			++context->parsing_row;
			context->parsing_col = 0;
		} else {
			wrapped = '\0';
			++context->parsing_col;
		}
		status = _parse_char(s, ch, context->parsing_pos, _row, _col);
		result = status;
		if(status) {
			_set_error_pos(s, context->parsing_pos, _row, _col);
			if(s->error_handler) {
				(s->error_handler)(s, s->last_error, (char*)mb_get_error_desc(s->last_error),
					s->last_error_pos,
					s->last_error_row,
					s->last_error_col,
					result);
			}

			goto _exit;
		}
		_row = context->parsing_row;
		_col = context->parsing_col;
		++i;
		++context->parsing_pos;
	};
	status = _parse_char(s, MB_EOS, context->parsing_pos, context->parsing_row, context->parsing_col);

_exit:
	context->parsing_state = _PS_NORMAL;

	return result;
}

int mb_load_file(struct mb_interpreter_t* s, const char* f) {
	/* Load a script file */
	int result = MB_FUNC_OK;
	FILE* fp = 0;
	char* buf = 0;
	long curpos = 0;
	long l = 0;
	_parsing_context_t* context = 0;

	mb_assert(s);

	context = s->parsing_context;

	s->parsing_context = context = _reset_parsing_context(s->parsing_context);

	fp = fopen(f, "rb");
	if(fp) {
		curpos = ftell(fp);
		fseek(fp, 0L, SEEK_END);
		l = ftell(fp);
		fseek(fp, curpos, SEEK_SET);
		buf = (char*)mb_malloc((size_t)(l + 1));
		mb_assert(buf);
		fread(buf, 1, l, fp);
		fclose(fp);
		buf[l] = '\0';

		result = mb_load_string(s, buf);
		safe_free(buf);

		if(result)
			goto _exit;
	} else {
		_set_current_error(s, SE_PS_FILE_OPEN_FAILED);

		++result;
	}

_exit:
	context->parsing_state = _PS_NORMAL;

	return result;
}

int mb_run(struct mb_interpreter_t* s) {
	/* Run loaded and parsed script */
	int result = MB_FUNC_OK;
	_ls_node_t* ast = 0;
	_running_context_t* running = 0;

	running = s->running_context;

	if(s->parsing_context)
		safe_free(s->parsing_context);

	if(running->suspent_point) {
		ast = running->suspent_point;
		ast = ast->next;
		running->suspent_point = 0;
	} else {
		mb_assert(!running->no_eat_comma_mark);
		ast = s->ast;
		ast = ast->next;
		if(!ast) {
			_set_current_error(s, SE_RN_EMPTY_PROGRAM);
			_set_error_pos(s, 0, 0, 0);
			result = MB_FUNC_ERR;
			(s->error_handler)(s, s->last_error, (char*)mb_get_error_desc(s->last_error),
				s->last_error_pos,
				s->last_error_row,
				s->last_error_col,
				result);

			goto _exit;
		}
	}

	do {
		result = _execute_statement(s, &ast);
		if(result != MB_FUNC_OK && result != MB_SUB_RETURN) {
			if(result != MB_FUNC_SUSPEND && s->error_handler) {
				if(result >= MB_EXTENDED_ABORT)
					s->last_error = SE_EA_EXTENDED_ABORT;
				(s->error_handler)(s, s->last_error, (char*)mb_get_error_desc(s->last_error),
					s->last_error_pos,
					s->last_error_row,
					s->last_error_col,
					result);
			}

			goto _exit;
		}
	} while(ast);

_exit:
	_ls_foreach(running->temp_values, _destroy_object);
	_ls_clear(running->temp_values);

	return result;
}

int mb_suspend(struct mb_interpreter_t* s, void** l) {
	/* Suspend current execution and save the context */
	int result = MB_FUNC_OK;
	_ls_node_t* ast = 0;

	mb_assert(s && l && *l);

	ast = (_ls_node_t*)(*l);
	s->running_context->suspent_point = ast;

	return result;
}

int mb_schedule_suspend(struct mb_interpreter_t* s, int t) {
	/* Schedule to suspend current execution and will save the context */
	int result = MB_FUNC_OK;

	mb_assert(s);

	if(t == MB_FUNC_OK)
		t = MB_FUNC_SUSPEND;
	s->running_context->schedule_suspend_tag = t;

	return result;
}

int mb_debug_get(struct mb_interpreter_t* s, const char* n, mb_value_t* val) {
	/* Get the value of an identifier */
	int result = MB_FUNC_OK;
	_ls_node_t* v = 0;
	_object_t* obj = 0;
	mb_value_t tmp;

	mb_assert(s && n);

	v = _ht_find(s->global_var_dict, (void*)n);
	if(v) {
		obj = (_object_t*)(v->data);
		mb_assert(obj->type == _DT_VAR);
		if(val)
			result = _internal_object_to_public_value(obj->data.variable->data, val);
		else
			result = _internal_object_to_public_value(obj->data.variable->data, &tmp);
	} else {
		if(val)
			val->type = MB_DT_NIL;
		result = MB_DEBUG_ID_NOT_FOUND;
	}

	return result;
}

int mb_debug_set(struct mb_interpreter_t* s, const char* n, mb_value_t val) {
	/* Set the value of an identifier */
	int result = MB_FUNC_OK;
	_ls_node_t* v = 0;
	_object_t* obj = 0;

	mb_assert(s && n);

	v = _ht_find(s->global_var_dict, (void*)n);
	if(v) {
		obj = (_object_t*)(v->data);
		mb_assert(obj->type == _DT_VAR);
		result = _public_value_to_internal_object(&val, obj->data.variable->data);
	} else {
		result = MB_DEBUG_ID_NOT_FOUND;
	}

	return result;
}

int mb_debug_set_stepped_handler(struct mb_interpreter_t* s, mb_debug_stepped_handler_t h) {
	/* Set a stepped handler to an interpreter instance */
	int result = MB_FUNC_OK;

	mb_assert(s);

	s->debug_stepped_handler = h;

	return result;
}

mb_error_e mb_get_last_error(struct mb_interpreter_t* s) {
	/* Get last error information */
	mb_error_e result = SE_NO_ERR;

	mb_assert(s);

	result = s->last_error;
	s->last_error = SE_NO_ERR;

	return result;
}

const char* mb_get_error_desc(mb_error_e err) {
	/* Get error description text */
	mb_assert(err >= 0 && err < _countof(_ERR_DESC));

	return _ERR_DESC[err];
}

int mb_set_error_handler(struct mb_interpreter_t* s, mb_error_handler_t h) {
	/* Set an error handler to an interpreter instance */
	int result = MB_FUNC_OK;

	mb_assert(s);

	s->error_handler = h;

	return result;
}

int mb_set_printer(struct mb_interpreter_t* s, mb_print_func_t p) {
	/* Set a print functor to an interpreter instance */
	int result = MB_FUNC_OK;

	mb_assert(s);

	s->printer = p;

	return result;
}

int mb_set_inputer(struct mb_interpreter_t* s, mb_input_func_t p) {
	/* Set an input functor to an interpreter instance */
	int result = MB_FUNC_OK;

	mb_assert(s);

	s->inputer = p;

	return result;
}

int mb_gets(char* buf, int s) {
	/* Safe stdin reader function */
	int result = 0;
	if(fgets(buf, s, stdin) == 0) {
		fprintf(stderr, "Error reading.\n");
		exit(1);
	}
	result = (int)strlen(buf);
	if(buf[result - 1] == '\n')
		buf[result - 1] = '\0';

	return result;
}

char* mb_memdup(char* val, unsigned size) {
	/* Duplicate a string for internal use */
	char* result = 0;

	if(val != 0) {
		result = (char*)mb_malloc(size);
		memcpy(result, val, size);
	}

	return result;
}

/* ========================================================} */

/*
** {========================================================
** Lib definitions
*/

/** Core lib */
int _core_dummy_assign(mb_interpreter_t* s, void** l) {
	/* Operator #, dummy assignment */
	int result = MB_FUNC_OK;
	mb_unrefvar(s);
	mb_unrefvar(l);

	mb_assert(0 && "Do nothing, impossible here");
	_do_nothing;

	return result;
}

int _core_add(mb_interpreter_t* s, void** l) {
	/* Operator + */
	int result = MB_FUNC_OK;

	mb_assert(s && l);

	if(_is_string(((_tuple3_t*)(*l))->e1) || _is_string(((_tuple3_t*)(*l))->e2)) {
		if(_is_string(((_tuple3_t*)(*l))->e1) && _is_string(((_tuple3_t*)(*l))->e2)) {
			_instruct_connect_strings(l);
		} else {
			_handle_error_on_obj(s, SE_RN_STRING_EXPECTED, (l && *l) ? ((_object_t*)(((_tuple3_t*)(*l))->e1)) : 0, MB_FUNC_ERR, _exit, result);
		}
	} else {
		_instruct_num_op_num(+, l);
	}

_exit:
	return result;
}

int _core_min(mb_interpreter_t* s, void** l) {
	/* Operator - */
	int result = MB_FUNC_OK;

	mb_assert(s && l);

	_instruct_num_op_num(-, l);

	return result;
}

int _core_mul(mb_interpreter_t* s, void** l) {
	/* Operator * */
	int result = MB_FUNC_OK;

	mb_assert(s && l);

	_instruct_num_op_num(*, l);

	return result;
}

int _core_div(mb_interpreter_t* s, void** l) {
	/* Operator / */
	int result = MB_FUNC_OK;

	mb_assert(s && l);

	_proc_div_by_zero(s, l, _exit, result, SE_RN_DIVIDE_BY_ZERO);
	_instruct_num_op_num(/, l);

_exit:
	return result;
}

int _core_mod(mb_interpreter_t* s, void** l) {
	/* Operator MOD */
	int result = MB_FUNC_OK;

	mb_assert(s && l);

	_proc_div_by_zero(s, l, _exit, result, SE_RN_MOD_BY_ZERO);
	_instruct_int_op_int(%, l);

_exit:
	return result;
}

int _core_pow(mb_interpreter_t* s, void** l) {
	/* Operator ^ */
	int result = MB_FUNC_OK;

	mb_assert(s && l);

	_instruct_fun_num_num(pow, l);

	return result;
}

int _core_open_bracket(mb_interpreter_t* s, void** l) {
	/* Operator ( */
	int result = MB_FUNC_OK;
	mb_unrefvar(s);
	mb_unrefvar(l);

	mb_assert(0 && "Do nothing, impossible here");
	_do_nothing;

	return result;
}

int _core_close_bracket(mb_interpreter_t* s, void** l) {
	/* Operator ) */
	int result = MB_FUNC_OK;
	mb_unrefvar(s);
	mb_unrefvar(l);

	mb_assert(0 && "Do nothing, impossible here");
	_do_nothing;

	return result;
}

int _core_neg(mb_interpreter_t* s, void** l) {
	/* Operator - (negative) */
	int result = MB_FUNC_OK;
	mb_value_t arg;
	_running_context_t* running = 0;
	int* inep = 0;

	mb_assert(s && l);

	running = s->running_context;

	if(!_ls_empty(running->in_neg_expr))
		inep = (int*)_ls_back(running->in_neg_expr)->data;

	if(inep)
		(*inep)++;

	mb_check(mb_attempt_func_begin(s, l));

	mb_check(mb_pop_value(s, l, &arg));

	mb_check(mb_attempt_func_end(s, l));

	if(inep)
		(*inep)--;

	switch(arg.type) {
	case MB_DT_INT:
		arg.value.integer = -(arg.value.integer);

		break;
	case MB_DT_REAL:
		arg.value.float_point = -(arg.value.float_point);

		break;
	default:
		_handle_error_on_obj(s, SE_RN_NUMBER_EXPECTED, (l && *l) ? ((_object_t*)(((_tuple3_t*)(*l))->e1)) : 0, MB_FUNC_WARNING, _exit, result);

		break;
	}
	mb_check(mb_push_value(s, l, arg));

_exit:
	return result;
}

int _core_equal(mb_interpreter_t* s, void** l) {
	/* Operator = */
	int result = MB_FUNC_OK;
	_tuple3_t* tpr = 0;

	mb_assert(s && l);

	if(_is_string(((_tuple3_t*)(*l))->e1) || _is_string(((_tuple3_t*)(*l))->e2)) {
		if(_is_string(((_tuple3_t*)(*l))->e1) && _is_string(((_tuple3_t*)(*l))->e2)) {
			_instruct_compare_strings(==, l);
		} else {
			_set_tuple3_result(l, 0);
			_handle_error_on_obj(s, SE_RN_STRING_EXPECTED, (l && *l) ? ((_object_t*)(((_tuple3_t*)(*l))->e1)) : 0, MB_FUNC_WARNING, _exit, result);
		}
	} else {
		_instruct_num_op_num(==, l);
		tpr = (_tuple3_t*)(*l);
		if(((_object_t*)(tpr->e3))->type != _DT_INT) {
			((_object_t*)(tpr->e3))->type = _DT_INT;
			((_object_t*)(tpr->e3))->data.integer = ((_object_t*)(tpr->e3))->data.float_point != 0.0f;
		}
	}

_exit:
	return result;
}

int _core_less(mb_interpreter_t* s, void** l) {
	/* Operator < */
	int result = MB_FUNC_OK;
	_tuple3_t* tpr = 0;

	mb_assert(s && l);

	if(_is_string(((_tuple3_t*)(*l))->e1) || _is_string(((_tuple3_t*)(*l))->e2)) {
		if(_is_string(((_tuple3_t*)(*l))->e1) && _is_string(((_tuple3_t*)(*l))->e2)) {
			_instruct_compare_strings(<, l);
		} else {
			if(_is_string(((_tuple3_t*)(*l))->e1)) {
				_set_tuple3_result(l, 0);
			} else {
				_set_tuple3_result(l, 1);
			}
			_handle_error_on_obj(s, SE_RN_STRING_EXPECTED, (l && *l) ? ((_object_t*)(((_tuple3_t*)(*l))->e1)) : 0, MB_FUNC_WARNING, _exit, result);
		}
	} else {
		_instruct_num_op_num(<, l);
		tpr = (_tuple3_t*)(*l);
		if(((_object_t*)(tpr->e3))->type != _DT_INT) {
			((_object_t*)(tpr->e3))->type = _DT_INT;
			((_object_t*)(tpr->e3))->data.integer = ((_object_t*)(tpr->e3))->data.float_point != 0.0f;
		}
	}

_exit:
	return result;
}

int _core_greater(mb_interpreter_t* s, void** l) {
	/* Operator > */
	int result = MB_FUNC_OK;
	_tuple3_t* tpr = 0;

	mb_assert(s && l);

	if(_is_string(((_tuple3_t*)(*l))->e1) || _is_string(((_tuple3_t*)(*l))->e2)) {
		if(_is_string(((_tuple3_t*)(*l))->e1) && _is_string(((_tuple3_t*)(*l))->e2)) {
			_instruct_compare_strings(>, l);
		} else {
			if(_is_string(((_tuple3_t*)(*l))->e1)) {
				_set_tuple3_result(l, 1);
			} else {
				_set_tuple3_result(l, 0);
			}
			_handle_error_on_obj(s, SE_RN_STRING_EXPECTED, (l && *l) ? ((_object_t*)(((_tuple3_t*)(*l))->e1)) : 0, MB_FUNC_WARNING, _exit, result);
		}
	} else {
		_instruct_num_op_num(>, l);
		tpr = (_tuple3_t*)(*l);
		if(((_object_t*)(tpr->e3))->type != _DT_INT) {
			((_object_t*)(tpr->e3))->type = _DT_INT;
			((_object_t*)(tpr->e3))->data.integer = ((_object_t*)(tpr->e3))->data.float_point != 0.0f;
		}
	}

_exit:
	return result;
}

int _core_less_equal(mb_interpreter_t* s, void** l) {
	/* Operator <= */
	int result = MB_FUNC_OK;
	_tuple3_t* tpr = 0;

	mb_assert(s && l);

	if(_is_string(((_tuple3_t*)(*l))->e1) || _is_string(((_tuple3_t*)(*l))->e2)) {
		if(_is_string(((_tuple3_t*)(*l))->e1) && _is_string(((_tuple3_t*)(*l))->e2)) {
			_instruct_compare_strings(<=, l);
		} else {
			if(_is_string(((_tuple3_t*)(*l))->e1)) {
				_set_tuple3_result(l, 0);
			} else {
				_set_tuple3_result(l, 1);
			}
			_handle_error_on_obj(s, SE_RN_STRING_EXPECTED, (l && *l) ? ((_object_t*)(((_tuple3_t*)(*l))->e1)) : 0, MB_FUNC_WARNING, _exit, result);
		}
	} else {
		_instruct_num_op_num(<=, l);
		tpr = (_tuple3_t*)(*l);
		if(((_object_t*)(tpr->e3))->type != _DT_INT) {
			((_object_t*)(tpr->e3))->type = _DT_INT;
			((_object_t*)(tpr->e3))->data.integer = ((_object_t*)(tpr->e3))->data.float_point != 0.0f;
		}
	}

_exit:
	return result;
}

int _core_greater_equal(mb_interpreter_t* s, void** l) {
	/* Operator >= */
	int result = MB_FUNC_OK;
	_tuple3_t* tpr = 0;

	mb_assert(s && l);

	if(_is_string(((_tuple3_t*)(*l))->e1) || _is_string(((_tuple3_t*)(*l))->e2)) {
		if(_is_string(((_tuple3_t*)(*l))->e1) && _is_string(((_tuple3_t*)(*l))->e2)) {
			_instruct_compare_strings(>=, l);
		} else {
			if(_is_string(((_tuple3_t*)(*l))->e1)) {
				_set_tuple3_result(l, 1);
			} else {
				_set_tuple3_result(l, 0);
			}
			_handle_error_on_obj(s, SE_RN_STRING_EXPECTED, (l && *l) ? ((_object_t*)(((_tuple3_t*)(*l))->e1)) : 0, MB_FUNC_WARNING, _exit, result);
		}
	} else {
		_instruct_num_op_num(>=, l);
		tpr = (_tuple3_t*)(*l);
		if(((_object_t*)(tpr->e3))->type != _DT_INT) {
			((_object_t*)(tpr->e3))->type = _DT_INT;
			((_object_t*)(tpr->e3))->data.integer = ((_object_t*)(tpr->e3))->data.float_point != 0.0f;
		}
	}

_exit:
	return result;
}

int _core_not_equal(mb_interpreter_t* s, void** l) {
	/* Operator <> */
	int result = MB_FUNC_OK;
	_tuple3_t* tpr = 0;

	mb_assert(s && l);

	if(_is_string(((_tuple3_t*)(*l))->e1) || _is_string(((_tuple3_t*)(*l))->e2)) {
		if(_is_string(((_tuple3_t*)(*l))->e1) && _is_string(((_tuple3_t*)(*l))->e2)) {
			_instruct_compare_strings(!=, l);
		} else {
			_set_tuple3_result(l, 1);
			_handle_error_on_obj(s, SE_RN_STRING_EXPECTED, (l && *l) ? ((_object_t*)(((_tuple3_t*)(*l))->e1)) : 0, MB_FUNC_WARNING, _exit, result);
		}
	} else {
		_instruct_num_op_num(!=, l);
		tpr = (_tuple3_t*)(*l);
		if(((_object_t*)(tpr->e3))->type != _DT_INT) {
			((_object_t*)(tpr->e3))->type = _DT_INT;
			((_object_t*)(tpr->e3))->data.integer = ((_object_t*)(tpr->e3))->data.float_point != 0.0f;
		}
	}

_exit:
	return result;
}

int _core_and(mb_interpreter_t* s, void** l) {
	/* Operator AND */
	int result = MB_FUNC_OK;

	mb_assert(s && l);

	_instruct_num_op_num(&&, l);

	return result;
}

int _core_or(mb_interpreter_t* s, void** l) {
	/* Operator OR */
	int result = MB_FUNC_OK;

	mb_assert(s && l);

	_instruct_num_op_num(||, l);

	return result;
}

int _core_not(mb_interpreter_t* s, void** l) {
	/* Operator NOT */
	int result = MB_FUNC_OK;
	mb_value_t arg;

	mb_assert(s && l);

	mb_check(mb_attempt_func_begin(s, l));

	mb_check(mb_pop_value(s, l, &arg));

	mb_check(mb_attempt_func_end(s, l));

	switch(arg.type) {
	case MB_DT_INT:
		arg.value.integer = (int_t)(!arg.value.integer);

		break;
	case MB_DT_REAL:
		arg.value.integer = (int_t)(!((int_t)arg.value.float_point));
		arg.type = MB_DT_INT;

		break;
	default:
		break;
	}
	mb_check(mb_push_int(s, l, arg.value.integer));

	return result;
}

int _core_let(mb_interpreter_t* s, void** l) {
	/* LET statement */
	int result = MB_FUNC_OK;
	_ls_node_t* ast = 0;
	_object_t* obj = 0;
	_var_t* var = 0;
	_array_t* arr = 0;
	unsigned int arr_idx = 0;
	_object_t* val = 0;

	mb_assert(s && l);

	ast = (_ls_node_t*)(*l);
	obj = (_object_t*)(ast->data);
	if(obj->type == _DT_FUNC)
		ast = ast->next;
	if(!ast || !ast->data) {
		_handle_error_on_obj(s, SE_RN_SYNTAX, DON(ast), MB_FUNC_ERR, _exit, result);
	}
	obj = (_object_t*)(ast->data);
	if(obj->type == _DT_VAR) {
		var = obj->data.variable;
	} else if(obj->type == _DT_ARRAY) {
		arr = obj->data.array;
		result = _get_array_index(s, &ast, &arr_idx);
		if(result != MB_FUNC_OK)
			goto _exit;
	} else {
		_handle_error_on_obj(s, SE_RN_VAR_OR_ARRAY_EXPECTED, DON(ast), MB_FUNC_ERR, _exit, result);
	}

	ast = ast->next;
	if(!ast || !ast->data) {
		_handle_error_on_obj(s, SE_RN_SYNTAX, DON(ast), MB_FUNC_ERR, _exit, result);
	}
	obj = (_object_t*)(ast->data);
	if(obj->type != _DT_FUNC || strcmp(obj->data.func->name, "=") != 0) {
		_handle_error_on_obj(s, SE_RN_ASSIGN_OPERATOR_EXPECTED, DON(ast), MB_FUNC_ERR, _exit, result);
	}

	ast = ast->next;
	val = (_object_t*)mb_malloc(sizeof(_object_t));
	memset(val, 0, sizeof(_object_t));
	result = _calc_expression(s, &ast, &val);

	if(var) {
		if(val->type != _DT_ANY) {
			_dispose_object(var->data);
			var->data->type = val->type;
			var->data->data = val->data;
			var->data->ref = val->ref;
		}
	} else if(arr) {
		mb_value_u _val;
		switch(val->type) {
		case _DT_INT:
			_val.integer = val->data.integer;

			break;
		case _DT_REAL:
			_val.float_point = val->data.float_point;

			break;
		case _DT_STRING:
			_val.string = val->data.string;

			break;
		case _DT_USERTYPE:
			_val.usertype = val->data.usertype;

			break;
		default:
			mb_assert(0 && "Unsupported");

			break;
		}
		result = _set_array_elem(s, ast, arr, arr_idx, &_val, &val->type);
		if(result != MB_FUNC_OK)
			goto _exit;
		if(val->type == _DT_STRING && !val->ref) {
			safe_free(val->data.string);
		}
	}
	safe_free(val);

_exit:
	*l = ast;

	return result;
}

int _core_dim(mb_interpreter_t* s, void** l) {
	/* DIM statement */
	int result = MB_FUNC_OK;
	_ls_node_t* ast = 0;
	_object_t* arr = 0;
	_object_t* len = 0;
	mb_value_u val;
	_array_t dummy;

	mb_assert(s && l);

	/* Array name */
	ast = (_ls_node_t*)(*l);
	if(!ast->next || ((_object_t*)(ast->next->data))->type != _DT_ARRAY) {
		_handle_error_on_obj(s, SE_RN_ARRAY_IDENTIFIER_EXPECTED, (ast && ast->next) ? ((_object_t*)(ast->next->data)) : 0, MB_FUNC_ERR, _exit, result);
	}
	ast = ast->next;
	arr = (_object_t*)(ast->data);
	memset(&dummy, 0, sizeof(_array_t));
	dummy.type = arr->data.array->type;
	dummy.name = arr->data.array->name;
	/* ( */
	if(!ast->next || ((_object_t*)(ast->next->data))->type != _DT_FUNC || ((_object_t*)(ast->next->data))->data.func->pointer != _core_open_bracket) {
		_handle_error_on_obj(s, SE_RN_OPEN_BRACKET_EXPECTED, (ast && ast->next) ? ((_object_t*)(ast->next->data)) : 0, MB_FUNC_ERR, _exit, result);
	}
	ast = ast->next;
	/* Array subscript */
	if(!ast->next) {
		_handle_error_on_obj(s, SE_RN_ARRAY_SUBSCRIPT_EXPECTED, (ast && ast->next) ? ((_object_t*)(ast->next->data)) : 0, MB_FUNC_ERR, _exit, result);
	}
	ast = ast->next;
	while(((_object_t*)(ast->data))->type != _DT_FUNC || ((_object_t*)(ast->data))->data.func->pointer != _core_close_bracket) {
		/* Get an integer value */
		len = (_object_t*)(ast->data);
		if(!_try_get_value(len, &val, _DT_INT)) {
			_handle_error_on_obj(s, SE_RN_TYPE_NOT_MATCH, DON(ast), MB_FUNC_ERR, _exit, result);
		}
		if(val.integer <= 0) {
			_handle_error_on_obj(s, SE_RN_ILLEGAL_BOUND, DON(ast), MB_FUNC_ERR, _exit, result);
		}
		if(dummy.dimension_count >= MB_MAX_DIMENSION_COUNT) {
			_handle_error_on_obj(s, SE_RN_DIMENSION_TOO_MUCH, DON(ast), MB_FUNC_ERR, _exit, result);
		}
		dummy.dimensions[dummy.dimension_count++] = (int)val.integer;
		if(dummy.count)
			dummy.count *= (unsigned int)val.integer;
		else
			dummy.count += (unsigned int)val.integer;
		ast = ast->next;
		/* Comma? */
		if(_IS_SEP(ast->data, ','))
			ast = ast->next;
	}
	/* Create or modify raw data */
	_clear_array(arr->data.array);
	*(arr->data.array) = dummy;
	_init_array(arr->data.array);
	if(!arr->data.array->raw) {
		arr->data.array->dimension_count = 0;
		arr->data.array->dimensions[0] = 0;
		arr->data.array->count = 0;
		_handle_error_on_obj(s, SE_RN_OUT_OF_MEMORY, DON(ast), MB_FUNC_ERR, _exit, result);
	}

_exit:
	*l = ast;

	return result;
}

int _core_if(mb_interpreter_t* s, void** l) {
	/* IF statement */
	int result = MB_FUNC_OK;
	_ls_node_t* ast = 0;
	_object_t* val = 0;
	_object_t* obj = 0;
	bool_t multi_line = false;
	_running_context_t* running = 0;

	mb_assert(s && l);

	running = s->running_context;

	ast = (_ls_node_t*)(*l);
	ast = ast->next;

	val = (_object_t*)mb_malloc(sizeof(_object_t));

_elseif:
	memset(val, 0, sizeof(_object_t));
	result = _calc_expression(s, &ast, &val);
	if(result != MB_FUNC_OK)
		goto _exit;
	mb_assert(val->type == _DT_INT);

	obj = (_object_t*)(ast->data);
	if(val->data.integer) {
		if(!_IS_FUNC(obj, _core_then)) {
			_handle_error_on_obj(s, SE_RN_INTEGER_EXPECTED, DON(ast), MB_FUNC_ERR, _exit, result);
		}

		if(ast && ast->next && _IS_EOS(ast->next->data))
			multi_line = true;

		running->skip_to_eoi = _ls_back(running->sub_stack);
		do {
			ast = ast->next;
			result = _execute_statement(s, &ast);
			if(result != MB_FUNC_OK)
				goto _exit;
			if(ast)
				ast = ast->prev;
		} while(ast && (
				(!multi_line && _IS_SEP(ast->data, ':')) || (
					multi_line && ast->next && (
						!_IS_FUNC(ast->next->data, _core_elseif) &&
						!_IS_FUNC(ast->next->data, _core_else) &&
						!_IS_FUNC(ast->next->data, _core_endif)
					)
				)
			)
		);

		if(!ast)
			goto _exit;

		obj = (_object_t*)(ast->data);
		if(obj->type != _DT_EOS) {
			running->skip_to_eoi = 0;
			result = _skip_to(s, &ast, 0, _DT_EOS);
			if(result != MB_FUNC_OK)
				goto _exit;
		}
	} else {
		if(ast && ast->next && _IS_EOS(ast->next->data)) {
			multi_line = true;

			_skip_if_chunk(s, &ast);
		}
		if(multi_line && ast && _IS_FUNC(ast->data, _core_elseif)) {
			if(ast) ast = ast->next;

			goto _elseif;
		}

		result = _skip_to(s, &ast, _core_else, _DT_EOS);
		if(result != MB_FUNC_OK)
			goto _exit;

		obj = (_object_t*)(ast->data);
		if(obj->type != _DT_EOS) {
			if(!_IS_FUNC(obj, _core_else)) {
				_handle_error_on_obj(s, SE_RN_ELSE_EXPECTED, DON(ast), MB_FUNC_ERR, _exit, result);
			}

			do {
				ast = ast->next;
				while(_IS_EOS(ast->data))
					ast = ast->next;
				result = _execute_statement(s, &ast);
				if(result != MB_FUNC_OK)
					goto _exit;
				if(ast)
					ast = ast->prev;
			} while(ast && (
					(!multi_line && _IS_SEP(ast->data, ':')) ||
					(multi_line && !_IS_FUNC(ast->next->data, _core_endif))
				)
			);
		}
	}

_exit:
	if(multi_line)
		result = _skip_to(s, &ast, _core_endif, _DT_NIL);

	_destroy_object(val, 0);

	*l = ast;

	return result;
}

int _core_then(mb_interpreter_t* s, void** l) {
	/* THEN statement */
	int result = MB_FUNC_OK;
	mb_unrefvar(s);
	mb_unrefvar(l);

	mb_assert(0 && "Do nothing, impossible here");
	_do_nothing;

	return result;
}

int _core_elseif(mb_interpreter_t* s, void** l) {
	/* ELSEIF statement */
	int result = MB_FUNC_OK;
	mb_unrefvar(s);
	mb_unrefvar(l);

	mb_assert(0 && "Do nothing, impossible here");
	_do_nothing;

	return result;
}

int _core_else(mb_interpreter_t* s, void** l) {
	/* ELSE statement */
	int result = MB_FUNC_OK;
	mb_unrefvar(s);
	mb_unrefvar(l);

	mb_assert(0 && "Do nothing, impossible here");
	_do_nothing;

	return result;
}

int _core_endif(mb_interpreter_t* s, void** l) {
	/* ENDIF statement */
	int result = MB_FUNC_OK;
	mb_unrefvar(s);
	mb_unrefvar(l);

	mb_assert(0 && "Do nothing, impossible here");
	_do_nothing;

	return result;
}

int _core_for(mb_interpreter_t* s, void** l) {
	/* FOR statement */
	int result = MB_FUNC_OK;
	_ls_node_t* ast = 0;
	_ls_node_t* to_node = 0;
	_object_t* obj = 0;
	_object_t to_val;
	_object_t step_val;
	_object_t* to_val_ptr = 0;
	_object_t* step_val_ptr = 0;
	_var_t* var_loop = 0;
	_tuple3_t ass_tuple3;
	_tuple3_t* ass_tuple3_ptr = 0;
	_running_context_t* running = 0;

	mb_assert(s && l);

	running = s->running_context;
	ast = (_ls_node_t*)(*l);
	ast = ast->next;

	to_val_ptr = &to_val;
	step_val_ptr = &step_val;
	ass_tuple3_ptr = &ass_tuple3;

	obj = (_object_t*)(ast->data);
	if(obj->type != _DT_VAR) {
		_handle_error_on_obj(s, SE_RN_LOOP_VAR_EXPECTED, DON(ast), MB_FUNC_ERR, _exit, result);
	}
	var_loop = obj->data.variable;

	result = _execute_statement(s, &ast);
	if(result != MB_FUNC_OK)
		goto _exit;
	ast = ast->prev;

	obj = (_object_t*)(ast->data);
	if(!_IS_FUNC(obj, _core_to)) {
		_handle_error_on_obj(s, SE_RN_TO_EXPECTED, DON(ast), MB_FUNC_ERR, _exit, result);
	}

	ast = ast->next;
	if(!ast) {
		_handle_error_on_obj(s, SE_RN_SYNTAX, DON(ast), MB_FUNC_ERR, _exit, result);
	}
	to_node = ast;

_to:
	ast = to_node;

	result = _calc_expression(s, &ast, &to_val_ptr);
	if(result != MB_FUNC_OK)
		goto _exit;

	obj = (_object_t*)(ast->data);
	if(!_IS_FUNC(obj, _core_step)) {
		step_val = _OBJ_INT_UNIT;
	} else {
		ast = ast->next;
		if(!ast) {
			_handle_error_on_obj(s, SE_RN_SYNTAX, DON(ast), MB_FUNC_ERR, _exit, result);
		}

		result = _calc_expression(s, &ast, &step_val_ptr);
		if(result != MB_FUNC_OK)
			goto _exit;
	}

	if((_compare_numbers(step_val_ptr, &_OBJ_INT_ZERO) == 1 && _compare_numbers(var_loop->data, to_val_ptr) == 1) ||
		(_compare_numbers(step_val_ptr, &_OBJ_INT_ZERO) == -1 && _compare_numbers(var_loop->data, to_val_ptr) == -1)) {
		/* End looping */
		if(_skip_struct(s, &ast, _core_for, _core_next) != MB_FUNC_OK)
			goto _exit;
		_skip_to(s, &ast, 0, _DT_EOS);

		goto _exit;
	} else {
		/* Keep looping */
		obj = (_object_t*)(ast->data);
		while(!_IS_FUNC(obj, _core_next)) {
			result = _execute_statement(s, &ast);
			if(result == MB_LOOP_CONTINUE) { /* NEXT */
				if(!running->next_loop_var || running->next_loop_var == var_loop) { /* This loop */
					running->next_loop_var = 0;
					result = MB_FUNC_OK;

					break;
				} else { /* Not this loop */
					if(_skip_struct(s, &ast, _core_for, _core_next) != MB_FUNC_OK)
						goto _exit;
					_skip_to(s, &ast, 0, _DT_EOS);

					goto _exit;
				}
			} else if(result == MB_LOOP_BREAK) { /* EXIT */
				if(_skip_struct(s, &ast, _core_for, _core_next) != MB_FUNC_OK)
					goto _exit;
				_skip_to(s, &ast, 0, _DT_EOS);
				result = MB_FUNC_OK;

				goto _exit;
			} else if(result != MB_FUNC_OK && result != MB_SUB_RETURN) { /* Normally */
				goto _exit;
			}

			if(!ast) {
				_handle_error_on_obj(s, SE_RN_NEXT_EXPECTED, DON(ast), MB_FUNC_ERR, _exit, result);
			}
			obj = (_object_t*)(ast->data);
		}

		ass_tuple3.e1 = var_loop->data;
		ass_tuple3.e2 = step_val_ptr;
		ass_tuple3.e3 = var_loop->data;
		_instruct_num_op_num(+, &ass_tuple3_ptr);

		goto _to;
	}

_exit:
	*l = ast;

	return result;
}

int _core_to(mb_interpreter_t* s, void** l) {
	/* TO statement */
	int result = MB_FUNC_OK;
	mb_unrefvar(s);
	mb_unrefvar(l);

	mb_assert(0 && "Do nothing, impossible here");
	_do_nothing;

	return result;
}

int _core_step(mb_interpreter_t* s, void** l) {
	/* STEP statement */
	int result = MB_FUNC_OK;
	mb_unrefvar(s);
	mb_unrefvar(l);

	mb_assert(0 && "Do nothing, impossible here");
	_do_nothing;

	return result;
}

int _core_next(mb_interpreter_t* s, void** l) {
	/* NEXT statement */
	int result = MB_FUNC_OK;
	_ls_node_t* ast = 0;
	_object_t* obj = 0;
	_running_context_t* running = 0;

	mb_assert(s && l);

	running = s->running_context;
	ast = (_ls_node_t*)(*l);

	result = MB_LOOP_CONTINUE;

	ast = ast->next;
	if(ast && ((_object_t*)(ast->data))->type == _DT_VAR) {
		obj = (_object_t*)(ast->data);
		running->next_loop_var = obj->data.variable;
	}

	*l = ast;

	return result;
}

int _core_while(mb_interpreter_t* s, void** l) {
	/* WHILE statement */
	int result = MB_FUNC_OK;
	_ls_node_t* ast = 0;
	_ls_node_t* loop_begin_node = 0;
	_object_t* obj = 0;
	_object_t loop_cond;
	_object_t* loop_cond_ptr = 0;

	mb_assert(s && l);

	ast = (_ls_node_t*)(*l);
	ast = ast->next;

	loop_cond_ptr = &loop_cond;

	loop_begin_node = ast;

_loop_begin:
	ast = loop_begin_node;

	result = _calc_expression(s, &ast, &loop_cond_ptr);
	if(result != MB_FUNC_OK)
		goto _exit;
	mb_assert(loop_cond_ptr->type == _DT_INT);

	if(loop_cond_ptr->data.integer) {
		/* Keep looping */
		obj = (_object_t*)(ast->data);
		while(!_IS_FUNC(obj, _core_wend)) {
			result = _execute_statement(s, &ast);
			if(result == MB_LOOP_BREAK) { /* EXIT */
				if(_skip_struct(s, &ast, _core_while, _core_wend) != MB_FUNC_OK)
					goto _exit;
				_skip_to(s, &ast, 0, _DT_EOS);
				result = MB_FUNC_OK;

				goto _exit;
			} else if(result != MB_FUNC_OK && result != MB_SUB_RETURN) { /* Normally */
				goto _exit;
			}

			obj = (_object_t*)(ast->data);
		}

		goto _loop_begin;
	} else {
		/* End looping */
		if(_skip_struct(s, &ast, _core_while, _core_wend) != MB_FUNC_OK)
			goto _exit;
		_skip_to(s, &ast, 0, _DT_EOS);

		goto _exit;
	}

_exit:
	*l = ast;

	return result;
}

int _core_wend(mb_interpreter_t* s, void** l) {
	/* WEND statement */
	int result = MB_FUNC_OK;
	mb_unrefvar(s);
	mb_unrefvar(l);

	mb_assert(0 && "Do nothing, impossible here");
	_do_nothing;

	return result;
}

int _core_do(mb_interpreter_t* s, void** l) {
	/* DO statement */
	int result = MB_FUNC_OK;
	_ls_node_t* ast = 0;
	_ls_node_t* loop_begin_node = 0;
	_object_t* obj = 0;
	_object_t loop_cond;
	_object_t* loop_cond_ptr = 0;

	mb_assert(s && l);

	ast = (_ls_node_t*)(*l);
	ast = ast->next;

	obj = (_object_t*)(ast->data);
	if(!_IS_EOS(obj)) {
		_handle_error_on_obj(s, SE_RN_SYNTAX, DON(ast), MB_FUNC_ERR, _exit, result);
	}
	ast = ast->next;

	loop_cond_ptr = &loop_cond;

	loop_begin_node = ast;

_loop_begin:
	ast = loop_begin_node;

	obj = (_object_t*)(ast->data);
	while(!_IS_FUNC(obj, _core_until)) {
		result = _execute_statement(s, &ast);
		if(result == MB_LOOP_BREAK) { /* EXIT */
			if(_skip_struct(s, &ast, _core_do, _core_until) != MB_FUNC_OK)
				goto _exit;
			_skip_to(s, &ast, 0, _DT_EOS);
			result = MB_FUNC_OK;

			goto _exit;
		} else if(result != MB_FUNC_OK && result != MB_SUB_RETURN) { /* Normally */
			goto _exit;
		}

		obj = (_object_t*)(ast->data);
	}

	obj = (_object_t*)(ast->data);
	if(!_IS_FUNC(obj, _core_until)) {
		_handle_error_on_obj(s, SE_RN_UNTIL_EXPECTED, DON(ast), MB_FUNC_ERR, _exit, result);
	}
	ast = ast->next;

	result = _calc_expression(s, &ast, &loop_cond_ptr);
	if(result != MB_FUNC_OK)
		goto _exit;
	mb_assert(loop_cond_ptr->type == _DT_INT);

	if(loop_cond_ptr->data.integer) {
		/* End looping */
		_skip_to(s, &ast, 0, _DT_EOS);

		goto _exit;
	} else {
		/* Keep looping */
		goto _loop_begin;
	}

_exit:
	*l = ast;

	return result;
}

int _core_until(mb_interpreter_t* s, void** l) {
	/* UNTIL statement */
	int result = MB_FUNC_OK;
	mb_unrefvar(s);
	mb_unrefvar(l);

	mb_assert(0 && "Do nothing, impossible here");
	_do_nothing;

	return result;
}

int _core_exit(mb_interpreter_t* s, void** l) {
	/* EXIT statement */
	int result = MB_FUNC_OK;

	mb_assert(s && l);

	result = MB_LOOP_BREAK;

	return result;
}

int _core_goto(mb_interpreter_t* s, void** l) {
	/* GOTO statement */
	int result = MB_FUNC_OK;
	_ls_node_t* ast = 0;
	_object_t* obj = 0;
	_label_t* label = 0;
	_ls_node_t* glbsyminscope = 0;

	mb_assert(s && l);

	ast = (_ls_node_t*)(*l);
	ast = ast->next;

	obj = (_object_t*)(ast->data);
	if(obj->type != _DT_LABEL) {
		_handle_error_on_obj(s, SE_RN_JUMP_LABEL_EXPECTED, DON(ast), MB_FUNC_ERR, _exit, result);
	}

	label = (_label_t*)(obj->data.label);
	if(!label->node) {
		glbsyminscope = _ht_find(s->global_var_dict, label->name);
		if(!(glbsyminscope && ((_object_t*)(glbsyminscope->data))->type == _DT_LABEL)) {
			_handle_error_on_obj(s, SE_RN_LABEL_NOT_EXISTS, DON(ast), MB_FUNC_ERR, _exit, result);
		}
		label->node = ((_object_t*)(glbsyminscope->data))->data.label->node;
	}

	mb_assert(label->node && label->node->prev);
	ast = label->node->prev;

_exit:
	*l = ast;

	return result;
}

int _core_gosub(mb_interpreter_t* s, void** l) {
	/* GOSUB statement */
	int result = MB_FUNC_OK;
	_ls_node_t* ast = 0;
	_running_context_t* running = 0;

	mb_assert(s && l);

	running = s->running_context;
	ast = (_ls_node_t*)(*l);
	result = _core_goto(s, l);
	if(result == MB_FUNC_OK)
		_ls_pushback(running->sub_stack, ast);

	return result;
}

int _core_return(mb_interpreter_t* s, void** l) {
	/* RETURN statement */
	int result = MB_SUB_RETURN;
	_ls_node_t* ast = 0;
	_running_context_t* running = 0;

	mb_assert(s && l);

	running = s->running_context;
	ast = (_ls_node_t*)_ls_popback(running->sub_stack);
	if(!ast) {
		_handle_error_on_obj(s, SE_RN_NO_RETURN_POINT, DON(ast), MB_FUNC_ERR, _exit, result);
	}
	*l = ast;

_exit:
	return result;
}

#ifdef MB_ENABLE_ALLOC_STAT
int _core_mem(mb_interpreter_t* s, void** l) {
	/* MEM statement */
	int result = MB_FUNC_OK;

	mb_assert(s && l);

	mb_check(mb_attempt_func_begin(s, l));
	mb_check(mb_attempt_func_end(s, l));

	mb_check(mb_push_int(s, l, (int_t)_mb_allocated));

	return result;
}
#endif /* MB_ENABLE_ALLOC_STAT */

int _core_end(mb_interpreter_t* s, void** l) {
	/* END statement */
	int result = MB_FUNC_OK;

	mb_assert(s && l);

	result = MB_FUNC_END;

	return result;
}

/** Std lib */
int _std_abs(mb_interpreter_t* s, void** l) {
	/* Get the absolute value of a number */
	int result = MB_FUNC_OK;
	mb_value_t arg;

	mb_assert(s && l);

	mb_check(mb_attempt_open_bracket(s, l));

	mb_check(mb_pop_value(s, l, &arg));

	mb_check(mb_attempt_close_bracket(s, l));

	switch(arg.type) {
	case MB_DT_INT:
		arg.value.integer = (int_t)abs(arg.value.integer);

		break;
	case MB_DT_REAL:
		arg.value.float_point = (real_t)fabs(arg.value.float_point);

		break;
	default:
		_handle_error_on_obj(s, SE_RN_NUMBER_EXPECTED, (l && *l) ? ((_object_t*)(((_tuple3_t*)(*l))->e1)) : 0, MB_FUNC_WARNING, _exit, result);

		break;
	}
	mb_check(mb_push_value(s, l, arg));

_exit:
	return result;
}

int _std_sgn(mb_interpreter_t* s, void** l) {
	/* Get the sign of a number */
	int result = MB_FUNC_OK;
	mb_value_t arg;

	mb_assert(s && l);

	mb_check(mb_attempt_open_bracket(s, l));

	mb_check(mb_pop_value(s, l, &arg));

	mb_check(mb_attempt_close_bracket(s, l));

	switch(arg.type) {
	case MB_DT_INT:
		arg.value.integer = sgn(arg.value.integer);

		break;
	case MB_DT_REAL:
		arg.value.integer = sgn(arg.value.float_point);
		arg.type = MB_DT_INT;

		break;
	default:
		_handle_error_on_obj(s, SE_RN_NUMBER_EXPECTED, (l && *l) ? ((_object_t*)(((_tuple3_t*)(*l))->e1)) : 0, MB_FUNC_WARNING, _exit, result);

		break;
	}
	mb_check(mb_push_int(s, l, arg.value.integer));

_exit:
	return result;
}

int _std_sqr(mb_interpreter_t* s, void** l) {
	/* Get the square root of a number */
	int result = MB_FUNC_OK;
	mb_value_t arg;

	mb_assert(s && l);

	mb_check(mb_attempt_open_bracket(s, l));

	mb_check(mb_pop_value(s, l, &arg));

	mb_check(mb_attempt_close_bracket(s, l));

	switch(arg.type) {
	case MB_DT_INT:
		arg.value.float_point = (real_t)sqrt((real_t)arg.value.integer);
		arg.type = MB_DT_REAL;

		break;
	case MB_DT_REAL:
		arg.value.float_point = (real_t)sqrt(arg.value.float_point);

		break;
	default:
		_handle_error_on_obj(s, SE_RN_NUMBER_EXPECTED, (l && *l) ? ((_object_t*)(((_tuple3_t*)(*l))->e1)) : 0, MB_FUNC_WARNING, _exit, result);

		break;
	}
	mb_check(mb_push_value(s, l, arg));

_exit:
	return result;
}

int _std_floor(mb_interpreter_t* s, void** l) {
	/* Get the greatest integer not greater than a number */
	int result = MB_FUNC_OK;
	mb_value_t arg;

	mb_assert(s && l);

	mb_check(mb_attempt_open_bracket(s, l));

	mb_check(mb_pop_value(s, l, &arg));

	mb_check(mb_attempt_close_bracket(s, l));

	switch(arg.type) {
	case MB_DT_INT:
		arg.value.integer = (int_t)(arg.value.integer);

		break;
	case MB_DT_REAL:
		arg.value.integer = (int_t)floor(arg.value.float_point);
		arg.type = MB_DT_INT;

		break;
	default:
		_handle_error_on_obj(s, SE_RN_NUMBER_EXPECTED, (l && *l) ? ((_object_t*)(((_tuple3_t*)(*l))->e1)) : 0, MB_FUNC_WARNING, _exit, result);

		break;
	}
	mb_check(mb_push_int(s, l, arg.value.integer));

_exit:
	return result;
}

int _std_ceil(mb_interpreter_t* s, void** l) {
	/* Get the least integer not less than a number */
	int result = MB_FUNC_OK;
	mb_value_t arg;

	mb_assert(s && l);

	mb_check(mb_attempt_open_bracket(s, l));

	mb_check(mb_pop_value(s, l, &arg));

	mb_check(mb_attempt_close_bracket(s, l));

	switch(arg.type) {
	case MB_DT_INT:
		arg.value.integer = (int_t)(arg.value.integer);

		break;
	case MB_DT_REAL:
		arg.value.integer = (int_t)ceil(arg.value.float_point);
		arg.type = MB_DT_INT;

		break;
	default:
		_handle_error_on_obj(s, SE_RN_NUMBER_EXPECTED, (l && *l) ? ((_object_t*)(((_tuple3_t*)(*l))->e1)) : 0, MB_FUNC_WARNING, _exit, result);

		break;
	}
	mb_check(mb_push_int(s, l, arg.value.integer));

_exit:
	return result;
}

int _std_fix(mb_interpreter_t* s, void** l) {
	/* Get the integer format of a number */
	int result = MB_FUNC_OK;
	mb_value_t arg;

	mb_assert(s && l);

	mb_check(mb_attempt_open_bracket(s, l));

	mb_check(mb_pop_value(s, l, &arg));

	mb_check(mb_attempt_close_bracket(s, l));

	switch(arg.type) {
	case MB_DT_INT:
		arg.value.integer = (int_t)(arg.value.integer);

		break;
	case MB_DT_REAL:
		arg.value.integer = (int_t)(arg.value.float_point);
		arg.type = MB_DT_INT;

		break;
	default:
		_handle_error_on_obj(s, SE_RN_NUMBER_EXPECTED, (l && *l) ? ((_object_t*)(((_tuple3_t*)(*l))->e1)) : 0, MB_FUNC_WARNING, _exit, result);

		break;
	}
	mb_check(mb_push_int(s, l, arg.value.integer));

_exit:
	return result;
}

int _std_round(mb_interpreter_t* s, void** l) {
	/* Get the rounded integer of a number */
	int result = MB_FUNC_OK;
	mb_value_t arg;

	mb_assert(s && l);

	mb_check(mb_attempt_open_bracket(s, l));

	mb_check(mb_pop_value(s, l, &arg));

	mb_check(mb_attempt_close_bracket(s, l));

	switch(arg.type) {
	case MB_DT_INT:
		arg.value.integer = (int_t)(arg.value.integer);

		break;
	case MB_DT_REAL:
		arg.value.integer = (int_t)(arg.value.float_point + (real_t)0.5f);
		arg.type = MB_DT_INT;

		break;
	default:
		_handle_error_on_obj(s, SE_RN_NUMBER_EXPECTED, (l && *l) ? ((_object_t*)(((_tuple3_t*)(*l))->e1)) : 0, MB_FUNC_WARNING, _exit, result);

		break;
	}
	mb_check(mb_push_int(s, l, arg.value.integer));

_exit:
	return result;
}

int _std_rnd(mb_interpreter_t* s, void** l) {
	/* Get a random value among 0 ~ 1 */
	int result = MB_FUNC_OK;
	real_t rnd = (real_t)0.0f;

	mb_assert(s && l);

	mb_check(mb_attempt_func_begin(s, l));
	mb_check(mb_attempt_func_end(s, l));

	rnd = (real_t)(((real_t)(rand() % 101)) / 100.0f);
	mb_check(mb_push_real(s, l, rnd));

	return result;
}

int _std_sin(mb_interpreter_t* s, void** l) {
	/* Get the sin value of a number */
	int result = MB_FUNC_OK;
	mb_value_t arg;

	mb_assert(s && l);

	mb_check(mb_attempt_open_bracket(s, l));

	mb_check(mb_pop_value(s, l, &arg));

	mb_check(mb_attempt_close_bracket(s, l));

	switch(arg.type) {
	case MB_DT_INT:
		arg.value.float_point = (real_t)sin((real_t)arg.value.integer);
		arg.type = MB_DT_REAL;

		break;
	case MB_DT_REAL:
		arg.value.float_point = (real_t)sin(arg.value.float_point);

		break;
	default:
		_handle_error_on_obj(s, SE_RN_NUMBER_EXPECTED, (l && *l) ? ((_object_t*)(((_tuple3_t*)(*l))->e1)) : 0, MB_FUNC_WARNING, _exit, result);

		break;
	}
	mb_check(mb_push_value(s, l, arg));

_exit:
	return result;
}

int _std_cos(mb_interpreter_t* s, void** l) {
	/* Get the cos value of a number */
	int result = MB_FUNC_OK;
	mb_value_t arg;

	mb_assert(s && l);

	mb_check(mb_attempt_open_bracket(s, l));

	mb_check(mb_pop_value(s, l, &arg));

	mb_check(mb_attempt_close_bracket(s, l));

	switch(arg.type) {
	case MB_DT_INT:
		arg.value.float_point = (real_t)cos((real_t)arg.value.integer);
		arg.type = MB_DT_REAL;

		break;
	case MB_DT_REAL:
		arg.value.float_point = (real_t)cos(arg.value.float_point);

		break;
	default:
		_handle_error_on_obj(s, SE_RN_NUMBER_EXPECTED, (l && *l) ? ((_object_t*)(((_tuple3_t*)(*l))->e1)) : 0, MB_FUNC_WARNING, _exit, result);

		break;
	}
	mb_check(mb_push_value(s, l, arg));

_exit:
	return result;
}

int _std_tan(mb_interpreter_t* s, void** l) {
	/* Get the tan value of a number */
	int result = MB_FUNC_OK;
	mb_value_t arg;

	mb_assert(s && l);

	mb_check(mb_attempt_open_bracket(s, l));

	mb_check(mb_pop_value(s, l, &arg));

	mb_check(mb_attempt_close_bracket(s, l));

	switch(arg.type) {
	case MB_DT_INT:
		arg.value.float_point = (real_t)tan((real_t)arg.value.integer);
		arg.type = MB_DT_REAL;

		break;
	case MB_DT_REAL:
		arg.value.float_point = (real_t)tan(arg.value.float_point);

		break;
	default:
		_handle_error_on_obj(s, SE_RN_NUMBER_EXPECTED, (l && *l) ? ((_object_t*)(((_tuple3_t*)(*l))->e1)) : 0, MB_FUNC_WARNING, _exit, result);

		break;
	}
	mb_check(mb_push_value(s, l, arg));

_exit:
	return result;
}

int _std_asin(mb_interpreter_t* s, void** l) {
	/* Get the asin value of a number */
	int result = MB_FUNC_OK;
	mb_value_t arg;

	mb_assert(s && l);

	mb_check(mb_attempt_open_bracket(s, l));

	mb_check(mb_pop_value(s, l, &arg));

	mb_check(mb_attempt_close_bracket(s, l));

	switch(arg.type) {
	case MB_DT_INT:
		arg.value.float_point = (real_t)asin((real_t)arg.value.integer);
		arg.type = MB_DT_REAL;

		break;
	case MB_DT_REAL:
		arg.value.float_point = (real_t)asin(arg.value.float_point);

		break;
	default:
		_handle_error_on_obj(s, SE_RN_NUMBER_EXPECTED, (l && *l) ? ((_object_t*)(((_tuple3_t*)(*l))->e1)) : 0, MB_FUNC_WARNING, _exit, result);

		break;
	}
	mb_check(mb_push_value(s, l, arg));

_exit:
	return result;
}

int _std_acos(mb_interpreter_t* s, void** l) {
	/* Get the acos value of a number */
	int result = MB_FUNC_OK;
	mb_value_t arg;

	mb_assert(s && l);

	mb_check(mb_attempt_open_bracket(s, l));

	mb_check(mb_pop_value(s, l, &arg));

	mb_check(mb_attempt_close_bracket(s, l));

	switch(arg.type) {
	case MB_DT_INT:
		arg.value.float_point = (real_t)acos((real_t)arg.value.integer);
		arg.type = MB_DT_REAL;

		break;
	case MB_DT_REAL:
		arg.value.float_point = (real_t)acos(arg.value.float_point);

		break;
	default:
		_handle_error_on_obj(s, SE_RN_NUMBER_EXPECTED, (l && *l) ? ((_object_t*)(((_tuple3_t*)(*l))->e1)) : 0, MB_FUNC_WARNING, _exit, result);

		break;
	}
	mb_check(mb_push_value(s, l, arg));

_exit:
	return result;
}

int _std_atan(mb_interpreter_t* s, void** l) {
	/* Get the atan value of a number */
	int result = MB_FUNC_OK;
	mb_value_t arg;

	mb_assert(s && l);

	mb_check(mb_attempt_open_bracket(s, l));

	mb_check(mb_pop_value(s, l, &arg));

	mb_check(mb_attempt_close_bracket(s, l));

	switch(arg.type) {
	case MB_DT_INT:
		arg.value.float_point = (real_t)atan((real_t)arg.value.integer);
		arg.type = MB_DT_REAL;

		break;
	case MB_DT_REAL:
		arg.value.float_point = (real_t)atan(arg.value.float_point);

		break;
	default:
		_handle_error_on_obj(s, SE_RN_NUMBER_EXPECTED, (l && *l) ? ((_object_t*)(((_tuple3_t*)(*l))->e1)) : 0, MB_FUNC_WARNING, _exit, result);

		break;
	}
	mb_check(mb_push_value(s, l, arg));

_exit:
	return result;
}

int _std_exp(mb_interpreter_t* s, void** l) {
	/* Get the exp value of a number */
	int result = MB_FUNC_OK;
	mb_value_t arg;

	mb_assert(s && l);

	mb_check(mb_attempt_open_bracket(s, l));

	mb_check(mb_pop_value(s, l, &arg));

	mb_check(mb_attempt_close_bracket(s, l));

	switch(arg.type) {
	case MB_DT_INT:
		arg.value.float_point = (real_t)exp((real_t)arg.value.integer);
		arg.type = MB_DT_REAL;

		break;
	case MB_DT_REAL:
		arg.value.float_point = (real_t)exp(arg.value.float_point);

		break;
	default:
		_handle_error_on_obj(s, SE_RN_NUMBER_EXPECTED, (l && *l) ? ((_object_t*)(((_tuple3_t*)(*l))->e1)) : 0, MB_FUNC_WARNING, _exit, result);

		break;
	}
	mb_check(mb_push_value(s, l, arg));

_exit:
	return result;
}

int _std_log(mb_interpreter_t* s, void** l) {
	/* Get the log value of a number */
	int result = MB_FUNC_OK;
	mb_value_t arg;

	mb_assert(s && l);

	mb_check(mb_attempt_open_bracket(s, l));

	mb_check(mb_pop_value(s, l, &arg));

	mb_check(mb_attempt_close_bracket(s, l));

	switch(arg.type) {
	case MB_DT_INT:
		arg.value.float_point = (real_t)log((real_t)arg.value.integer);
		arg.type = MB_DT_REAL;

		break;
	case MB_DT_REAL:
		arg.value.float_point = (real_t)log(arg.value.float_point);

		break;
	default:
		_handle_error_on_obj(s, SE_RN_NUMBER_EXPECTED, (l && *l) ? ((_object_t*)(((_tuple3_t*)(*l))->e1)) : 0, MB_FUNC_WARNING, _exit, result);

		break;
	}
	mb_check(mb_push_value(s, l, arg));

_exit:
	return result;
}

int _std_asc(mb_interpreter_t* s, void** l) {
	/* Get the ASCII code of a character */
	int result = MB_FUNC_OK;
	char* arg = 0;

	mb_assert(s && l);

	mb_check(mb_attempt_open_bracket(s, l));

	mb_check(mb_pop_string(s, l, &arg));

	mb_check(mb_attempt_close_bracket(s, l));

	if(arg[0] == '\0') {
		result = MB_FUNC_ERR;

		goto _exit;
	}
	mb_check(mb_push_int(s, l, (int_t)arg[0]));

_exit:
	return result;
}

int _std_chr(mb_interpreter_t* s, void** l) {
	/* Get the character of an ASCII code */
	int result = MB_FUNC_OK;
	int_t arg = 0;
	char* chr = 0;

	mb_assert(s && l);

	mb_check(mb_attempt_open_bracket(s, l));

	mb_check(mb_pop_int(s, l, &arg));

	mb_check(mb_attempt_close_bracket(s, l));

	chr = (char*)mb_malloc(2);
	memset(chr, 0, 2);
	chr[0] = (char)arg;
	mb_check(mb_push_string(s, l, chr));

	return result;
}

int _std_left(mb_interpreter_t* s, void** l) {
	/* Get a number of characters from the left of a string */
	int result = MB_FUNC_OK;
	char* arg = 0;
	int_t count = 0;
	char* sub = 0;

	mb_assert(s && l);

	mb_check(mb_attempt_open_bracket(s, l));

	mb_check(mb_pop_string(s, l, &arg));
	mb_check(mb_pop_int(s, l, &count));

	mb_check(mb_attempt_close_bracket(s, l));

	if(count <= 0) {
		result = MB_FUNC_ERR;

		goto _exit;
	}

	sub = (char*)mb_malloc(count + 1);
	memcpy(sub, arg, count);
	sub[count] = '\0';
	mb_check(mb_push_string(s, l, sub));

_exit:
	return result;
}

int _std_len(mb_interpreter_t* s, void** l) {
	/* Get the length of a string */
	int result = MB_FUNC_OK;
	char* arg = 0;

	mb_assert(s && l);

	mb_check(mb_attempt_open_bracket(s, l));

	mb_check(mb_pop_string(s, l, &arg));

	mb_check(mb_attempt_close_bracket(s, l));

	mb_check(mb_push_int(s, l, (int_t)strlen(arg)));

	return result;
}

int _std_mid(mb_interpreter_t* s, void** l) {
	/* Get a number of characters from a given position of a string */
	int result = MB_FUNC_OK;
	char* arg = 0;
	int_t start = 0;
	int_t count = 0;
	char* sub = 0;

	mb_assert(s && l);

	mb_check(mb_attempt_open_bracket(s, l));

	mb_check(mb_pop_string(s, l, &arg));
	mb_check(mb_pop_int(s, l, &start));
	mb_check(mb_pop_int(s, l, &count));

	mb_check(mb_attempt_close_bracket(s, l));

	if(count <= 0 || start < 0 || start >= (int_t)strlen(arg)) {
		result = MB_FUNC_ERR;

		goto _exit;
	}

	sub = (char*)mb_malloc(count + 1);
	memcpy(sub, arg + start, count);
	sub[count] = '\0';
	mb_check(mb_push_string(s, l, sub));

_exit:
	return result;
}

int _std_right(mb_interpreter_t* s, void** l) {
	/* Get a number of characters from the right of a string */
	int result = MB_FUNC_OK;
	char* arg = 0;
	int_t count = 0;
	char* sub = 0;

	mb_assert(s && l);

	mb_check(mb_attempt_open_bracket(s, l));

	mb_check(mb_pop_string(s, l, &arg));
	mb_check(mb_pop_int(s, l, &count));

	mb_check(mb_attempt_close_bracket(s, l));

	if(count <= 0) {
		result = MB_FUNC_ERR;

		goto _exit;
	}

	sub = (char*)mb_malloc(count + 1);
	memcpy(sub, arg + (strlen(arg) - count), count);
	sub[count] = '\0';
	mb_check(mb_push_string(s, l, sub));

_exit:
	return result;
}

int _std_str(mb_interpreter_t* s, void** l) {
	/* Get the string format of a number */
	int result = MB_FUNC_OK;
	mb_value_t arg;
	char* chr = 0;

	mb_assert(s && l);

	mb_check(mb_attempt_open_bracket(s, l));

	mb_check(mb_pop_value(s, l, &arg));

	mb_check(mb_attempt_close_bracket(s, l));

	chr = (char*)mb_malloc(32);
	memset(chr, 0, 32);
	if(arg.type == MB_DT_INT) {
		sprintf(chr, MB_INT_FMT, arg.value.integer);
	} else if(arg.type == MB_DT_REAL) {
		sprintf(chr, MB_REAL_FMT, arg.value.float_point);
	} else {
		result = MB_FUNC_ERR;

		goto _exit;
	}
	mb_check(mb_push_string(s, l, chr));

_exit:
	return result;
}

int _std_val(mb_interpreter_t* s, void** l) {
	/* Get the number format of a string */
	int result = MB_FUNC_OK;
	char* conv_suc = 0;
	mb_value_t val;
	char* arg = 0;

	mb_assert(s && l);

	mb_check(mb_attempt_open_bracket(s, l));

	mb_check(mb_pop_string(s, l, &arg));

	mb_check(mb_attempt_close_bracket(s, l));

	val.value.integer = (int_t)mb_strtol(arg, &conv_suc, 0);
	if(*conv_suc == '\0') {
		val.type = MB_DT_INT;
		mb_check(mb_push_value(s, l, val));

		goto _exit;
	}
	val.value.float_point = (real_t)mb_strtod(arg, &conv_suc);
	if(*conv_suc == '\0') {
		val.type = MB_DT_REAL;
		mb_check(mb_push_value(s, l, val));

		goto _exit;
	}
	result = MB_FUNC_ERR;

_exit:
	return result;
}

int _std_print(mb_interpreter_t* s, void** l) {
	/* PRINT statement */
	int result = MB_FUNC_OK;
	_ls_node_t* ast = 0;
	_object_t* obj = 0;
	_running_context_t* running = 0;

	_object_t val_obj;
	_object_t* val_ptr = 0;

	mb_assert(s && l);

	val_ptr = &val_obj;
	memset(val_ptr, 0, sizeof(_object_t));

	running = s->running_context;
	++running->no_eat_comma_mark;
	ast = (_ls_node_t*)(*l);
	ast = ast->next;
	if(!ast || !ast->data) {
		_handle_error_on_obj(s, SE_RN_SYNTAX, DON(ast), MB_FUNC_ERR, _exit, result);
	}

	obj = (_object_t*)(ast->data);
	do {
		switch(obj->type) {
		case _DT_INT: /* Fall through */
		case _DT_REAL: /* Fall through */
		case _DT_STRING: /* Fall through */
		case _DT_VAR: /* Fall through */
		case _DT_ARRAY: /* Fall through */
		case _DT_FUNC:
			result = _calc_expression(s, &ast, &val_ptr);
			if(val_ptr->type == _DT_INT) {
				_get_printer(s)(MB_INT_FMT, val_ptr->data.integer);
			} else if(val_ptr->type == _DT_REAL) {
				_get_printer(s)(MB_REAL_FMT, val_ptr->data.float_point);
			} else if(val_ptr->type == _DT_STRING) {
				_get_printer(s)("%s", (val_ptr->data.string ? val_ptr->data.string : MB_NULL_STRING));
				if(!val_ptr->ref && val_ptr->data.string) {
					safe_free(val_ptr->data.string);
				}
			}
			if(result != MB_FUNC_OK)
				goto _exit;
			/* Fall through */
		case _DT_SEP:
			if(!ast)
				break;
			obj = (_object_t*)(ast->data);
#ifdef _COMMA_AS_NEWLINE
			if(obj->data.separator == ',') {
#else /* _COMMA_AS_NEWLINE */
			if(obj->data.separator == ';') {
#endif /* _COMMA_AS_NEWLINE */
				_get_printer(s)("\n");
			}

			break;
		default:
			_handle_error_on_obj(s, SE_RN_NOT_SUPPORTED, DON(ast), MB_FUNC_ERR, _exit, result);

			break;
		}

		if(!ast)
			break;
		obj = (_object_t*)(ast->data);
		if(_is_print_terminal(s, obj))
			break;
		if(_IS_SEP(obj, ',') || _IS_SEP(obj, ';')) {
			ast = ast->next;
			obj = (_object_t*)(ast->data);
		} else {
			_handle_error_on_obj(s, SE_RN_COMMA_OR_SEMICOLON_EXPECTED, DON(ast), MB_FUNC_ERR, _exit, result);
		}
	} while(ast && !_IS_SEP(obj, ':') && (obj->type == _DT_SEP || !_is_expression_terminal(s, obj)));

_exit:
	--running->no_eat_comma_mark;

	*l = ast;
	if(result != MB_FUNC_OK)
		_get_printer(s)("\n");

	return result;
}

int _std_input(mb_interpreter_t* s, void** l) {
	/* INPUT statement */
	int result = MB_FUNC_OK;
	_ls_node_t* ast = 0;
	_object_t* obj = 0;
	char line[256];
	char* conv_suc = 0;

	mb_assert(s && l);

	mb_check(mb_attempt_func_begin(s, l));
	mb_check(mb_attempt_func_end(s, l));

	ast = (_ls_node_t*)(*l);
	obj = (_object_t*)(ast->data);

	if(!obj || obj->type != _DT_VAR) {
		_handle_error_on_obj(s, SE_RN_VARIABLE_EXPECTED, DON(ast), MB_FUNC_ERR, _exit, result);
	}
	if(obj->data.variable->data->type == _DT_INT || obj->data.variable->data->type == _DT_REAL) {
		_get_inputer(s)(line, sizeof(line));
		obj->data.variable->data->type = _DT_INT;
		obj->data.variable->data->data.integer = (int_t)mb_strtol(line, &conv_suc, 0);
		if(*conv_suc != '\0') {
			obj->data.variable->data->type = _DT_REAL;
			obj->data.variable->data->data.float_point = (real_t)mb_strtod(line, &conv_suc);
			if(*conv_suc != '\0') {
				result = MB_FUNC_ERR;

				goto _exit;
			}
		}
		ast = ast->next;
	} else if(obj->data.variable->data->type == _DT_STRING) {
		if(obj->data.variable->data->data.string) {
			safe_free(obj->data.variable->data->data.string);
		}
		obj->data.variable->data->data.string = (char*)mb_malloc(256);
		memset(obj->data.variable->data->data.string, 0, 256);
		_get_inputer(s)(line, sizeof(line));
		strcpy(obj->data.variable->data->data.string, line);
		ast = ast->next;
	} else {
		result = MB_FUNC_ERR;

		goto _exit;
	}

_exit:
	*l = ast;

	return result;
}

/* ========================================================} */

#ifdef MB_COMPACT_MODE
#	pragma pack()
#endif /* MB_COMPACT_MODE */

#ifdef __BORLANDC__
#	pragma warn .8004
#	pragma warn .8008
#	pragma warn .8012
#endif /* __BORLANDC__ */

#ifdef __APPLE__
#	pragma clang diagnostic pop
#endif /* __APPLE__ */

#ifdef _MSC_VER
#	pragma warning(pop)
#endif /* _MSC_VER */

#ifdef __cplusplus
}
#endif /* __cplusplus */
