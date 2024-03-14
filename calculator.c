/*======================HEADER=======================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>

// #define _DEBUG_
#define V_UNUSED(x) ((void)(x))

/*======================vector=======================*/

typedef void *ObjectPtr;
typedef void (*Destructor)(ObjectPtr);
typedef void (*CopyConstructor)(ObjectPtr, ObjectPtr, size_t);

//default destructor do nothing.
void default_destructor(ObjectPtr ignored){
    V_UNUSED(ignored);
}

//default copy constructor just copy all bytes from source.
void default_copy(ObjectPtr dst, ObjectPtr src, size_t size){
    memcpy(dst,src,size);
}

/**
 * @brief a dynamic length array. \n
 * All object in vector should have the same size. \n
 * Type casting safety is NOT guaranteed.
 */
typedef struct{

    ObjectPtr data;       //data pointer
    size_t width;           //object size

    int size;            //current objects number
    size_t capacity;        //size of pre-allocated memory

    Destructor destroy;     //object's destructor
    CopyConstructor copy;   //object's copy constructor

}Vector;

typedef Vector *VectorPtr;

const int DEFAULT_CAPACITY=4;

//details see vector-impl part.

Vector create_vec(int size,Destructor destroy,CopyConstructor copy);
Vector create_vec_default(int size);
void destroy_vec(ObjectPtr object);
void copy_vec(ObjectPtr _dst, ObjectPtr _src, size_t ignored);

ObjectPtr get_addr(Vector vec, int idx);
ObjectPtr get_back(Vector vec);
void get_object(Vector vec, ObjectPtr dst, int idx);
void set_object(VectorPtr vec, ObjectPtr value, int idx);

void push_back(VectorPtr vec, ObjectPtr value);
void pop_back(VectorPtr vec, ObjectPtr dst);

void clear_vec(VectorPtr vec);
void vec_recapacity(VectorPtr vec, size_t new_size);

//for_each loop. current element's index is `CUR_IDX`
#define FOR_RANGE(type, name, vec, l, r)\
    do{\
        type *name;\
        for(int __i = (l); __i < (r); ++__i){\
            name = (type *)get_addr(vec, __i);

#define FOR_EACH(type, name, vec) FOR_RANGE(type, name, vec, 0, vec.size)

#define END_FOR() }}while(0)

#define CUR_IDX __i

/*======================string=======================*/

/**
 * string based on vector<char>.
*/
typedef Vector String;
typedef VectorPtr StringPtr;

//see string-impl

String create_str(const char *str);
String create_str_emp();

const char *as_cstr(String str);
void push_char(StringPtr str, char c);
void push_string(StringPtr str1, const char* str2);
char char_at(String str, int idx);
bool str_eq(String str1, String str2);

/*======================Number definition=======================*/
#define NUM_SCALE 100000000

enum _numFlag{
    NUM_NINF = -2 ,NUM_NEG = -1, NUM_NAN, NUM_POS = 1, NUM_PINF
};

typedef enum _numFlag NumFlag;

//float-point number
typedef struct{
    char *digits;       //mantissa
    int place;          //significant digits
    int exp;            //exponent
    int sign;           //sign: -1 or 1
}Number;

typedef Number *NumberPtr;

void swap_numptr(NumberPtr *a, NumberPtr *b);

int get_dec_place();
int get_precision();
int get_eps();
bool get_fixed_flag();

Number create_num();
Number create_num_i(int i);
Number create_num_of_size(int size);
Number create_num_of_flag(NumFlag flag);
void destroy_num(ObjectPtr ptr);
void copy_num(ObjectPtr _dst, ObjectPtr _src, size_t size);
void move_num(NumberPtr dst, Number src);

bool num_is_inf(NumberPtr num);
bool num_is_nan(NumberPtr num);
bool num_invalid(NumberPtr num);

void num_adjust(NumberPtr num);
void num_check(NumberPtr num);
void num_rm_leading_zeros(NumberPtr num);

void num_rshift(NumberPtr num, int shift_n);
void num_lshift(NumberPtr num, int shift_n);

String num_to_str_fixed(NumberPtr num);
String num_to_str_sc(NumberPtr num);
String num_to_str(NumberPtr num);
Number str_to_num(StringPtr str);

int num_cmp_u(NumberPtr num1, NumberPtr num2);
int num_cmp(NumberPtr num1, NumberPtr num2);
int num_cmp_eps(NumberPtr num1, NumberPtr num2);
int num_cmp_zero(NumberPtr num);
bool num_is_zero(NumberPtr num);
bool num_lt(NumberPtr num1, NumberPtr num2);
bool num_gt(NumberPtr num1, NumberPtr num2);
bool num_le(NumberPtr num1, NumberPtr num2);
bool num_ge(NumberPtr num1, NumberPtr num2);
bool num_eq(NumberPtr num1, NumberPtr num2);
bool num_neq(NumberPtr num1, NumberPtr num2);

Number num_add_u(NumberPtr num1, NumberPtr num2);
Number num_sub_u(NumberPtr num1, NumberPtr num2);
Number num_add(NumberPtr num1, NumberPtr num2);
Number num_addi(NumberPtr num1, int num2);
Number num_sub(NumberPtr num1, NumberPtr num2);
Number num_subi(NumberPtr num1, int num2);
Number num_mul(NumberPtr num1, NumberPtr num2);
Number num_div(NumberPtr num1, NumberPtr num2);

Number num_floor_u(NumberPtr num);
Number num_ceil_u(NumberPtr num);
Number num_floor(NumberPtr num);
Number num_ceil(NumberPtr num);
Number num_round(NumberPtr num, int prec);

int split_int(Vector vec, char* err);


/*======================Variable definition=======================*/

enum _varType{
    VAR_FUN, VAR_NUM, VAR_REF, VAR_BOOL, VAR_VOID, VAR_ERR, VAR_ANY
};

typedef enum _varType VarType;

typedef struct{
    VarType type;
    ObjectPtr value;
}Value;

typedef Value *ValuePtr;

typedef struct{
    Value value;
    String name;
    bool is_const;
}Variable;

typedef Variable *VarPtr;
typedef Value (*BuiltinFunc)(Vector, char*);

Value create_val(Number num);
Value create_val_bool(bool val);
Value create_val_void();
Value create_val_err();
Value create_val_func(BuiltinFunc func);
void destroy_val(ObjectPtr obj);
void copy_val(ObjectPtr _dst, ObjectPtr _src, size_t size);

bool val_to_num(ValuePtr var);

void print_val(ValuePtr ptr);
VarPtr const_check(ValuePtr val, char* err);

Variable create_var(String name, bool is_const);
void copy_var(ObjectPtr _dst, ObjectPtr _src, size_t size);
void destroy_var(ObjectPtr obj);

ValuePtr var_deref(ValuePtr var);

void builtin_create_var(const char* name);
void builtin_set_var(const char* name, ValuePtr nval);
VarPtr lookup_var_byname(String name);
VarPtr lookup_var_notnull(String name);
VarPtr lookup_var(String name);

/*======================context=======================*/

enum _errorCode{
    NO_EXCEPTION,       //no error
    ERR_INVALID_TOKEN,  //incomplete or invalid token
    ERR_UNKNOWN_NAME,   //undefined variable name
    ERR_INVALID_EXPR,   //invalid expression
    ERR_ARITHMETIC,     //calculate error
    ERR_UNKNOWN         //unknown error
};
typedef enum _errorCode ErrCode;

//error description
const char *ERROR_DESC[]={
    "",
    "INVALID TOKEN",
    "UNKNOW NAME",
    "INVALID EXPRESSION",
    "ARITHMETIC ERROR",
    "UNKNOWN ERROR"
};

typedef struct _runtimeException{

    ErrCode err_code;
    String desc;

}Exception;

typedef Exception *ExceptionPtr;

void destroy_ex(ObjectPtr obj);
void copy_ex(ObjectPtr _dst, ObjectPtr _src, size_t size);
void throw_exception(ErrCode code, String desc);
String except_info(VectorPtr token, int l, int r, const char* info);
bool any_ex_happened();

struct{

    Vector ex_info;     //exceptions information
    Vector param_stk;   //calculation stack

    int scale;          //maximum/minimum of calculation
    int place;          //decimal place in calculation
    int prec;           //decimal place in output
    int eps;            //epsilon for compare

    bool flag_fix;      //output as fix-point

    jmp_buf buf;        //buffer for long jump

    Vector var_table;   //global variables
}global;

void global_init();
void global_flash();
void push_arg(ValuePtr var);
bool push_arg_num(ValuePtr var);
Value pop_arg();

/*======================tokenizer=======================*/

enum _tokenType{
    TOKEN_UNDEFINED,
    TOKEN_NUMBER,
    TOKEN_SYMBOL,
    TOKEN_NAME
};
typedef enum _tokenType TokenType;

const char *TOKEN_NAMES[]={
    "UNDEFINED",
    "NUMBER",
    "SYMBOL",
    "NAME"
};

typedef struct{

    TokenType type;
    String cont;
    String raw_cont;

    int match_pos;  //right bracket position

}Token;

typedef Token *TokenPtr;

bool cont_eq(Token token, const char *str);
void print_token(TokenPtr token);
Token create_token();
void destroy_token(ObjectPtr obj);
void copy_token(ObjectPtr _dst,ObjectPtr _src,size_t size);
bool op_append_check(const char* cont, char c);
void push_token(VectorPtr vec, TokenPtr token, int *number_flag);
void match_barket(Vector tokens);
void unary_check(Vector tokens);
Vector tokenize(int argc,char *words[]);
String recover_cont(Vector tokens, int l, int r);

enum NumberFlag{
    NUM_BEGIN,
    NUM_DOT,
    NUM_EXP,
    NUM_SIGN,
    NUM_FINISH
};

/*======================operators=======================*/

//valid operator for input.
const char *OP_LIST[]={"+","-","*","/","++","--","<",">","<=",">=","==","!=","(",")","=",","};

//valid operator in token.
const char *FULL_OP_LIST[]={"+","-","*","/","++","--","<",">","<=",">=","==","!=","(",")","=","+!","-!","++!","--!",","};

//left priority
// '()'[12] > '++','--'[8] > unary[7] > '*','/'[5] > '+','-'[3] > cmp[1] > '='[-1]
const int OP_L_LEVEL[]={3,3,5,5,8,8,1,1,1,1,1,1,12,12,-1,7,7,7,7,-2};

//right priority
// '()'[11] > '='[10] > '++','--'[9] > unary[6] > '*','/'[4] > '+','-'[2] > cmp[0]
const int OP_R_LEVEL[]={2,2,4,4,9,9,0,0,0,0,0,0,11,11,10,6,6,6,6,-2};

const int OP_COUNT = sizeof(OP_LIST) / sizeof(char*);
const int FULL_OP_COUNT = sizeof(FULL_OP_LIST) / sizeof(char*);
const int MAX_PRIORITY = 114514;

int get_l_level(Token token){
    if(token.type != TOKEN_SYMBOL)
        return MAX_PRIORITY;

    for(int i = 0; i < FULL_OP_COUNT; ++i)
        if(cont_eq(token, FULL_OP_LIST[i]))
            return OP_L_LEVEL[i];
    
    throw_exception(ERR_UNKNOWN, create_str("unknown operator"));
    return -1;
}

int get_r_level(Token token){
    if(token.type != TOKEN_SYMBOL)
        return MAX_PRIORITY - 1;

    for(int i = 0; i < FULL_OP_COUNT; ++i)
        if(cont_eq(token, FULL_OP_LIST[i]))
            return OP_R_LEVEL[i];
    
    throw_exception(ERR_UNKNOWN, create_str("unknown operator"));
    return -1;
}

Value opfunc_add(ValuePtr val1, ValuePtr val2, char* err){
    V_UNUSED(err);
    return create_val(num_add(val1->value, val2->value));
}

Value opfunc_sub(ValuePtr val1, ValuePtr val2, char* err){
    V_UNUSED(err);
    return create_val(num_sub(val1->value, val2->value));
}

Value opfunc_mul(ValuePtr val1, ValuePtr val2, char* err){
    V_UNUSED(err);
    return create_val(num_mul(val1->value, val2->value));
}

Value opfunc_div(ValuePtr val1, ValuePtr val2, char* err){
    V_UNUSED(err);
    return create_val(num_div(val1->value, val2->value));
}

Value opfunc_lt(ValuePtr val1, ValuePtr val2, char* err){
    V_UNUSED(err);
    return create_val_bool(num_lt(val1->value, val2->value));
}

Value opfunc_le(ValuePtr val1, ValuePtr val2, char* err){
    V_UNUSED(err);
    return create_val_bool(num_le(val1->value, val2->value));
}

Value opfunc_gt(ValuePtr val1, ValuePtr val2, char* err){
    V_UNUSED(err);
    return create_val_bool(num_gt(val1->value, val2->value));
}

Value opfunc_ge(ValuePtr val1, ValuePtr val2, char* err){
    V_UNUSED(err);
    return create_val_bool(num_ge(val1->value, val2->value));
}

Value opfunc_eq(ValuePtr val1, ValuePtr val2, char* err){
    V_UNUSED(err);
    return create_val_bool(num_eq(val1->value, val2->value));
}

Value opfunc_neq(ValuePtr val1, ValuePtr val2, char* err){
    V_UNUSED(err);
    return create_val_bool(num_neq(val1->value, val2->value));
}

Value opfunc_set(ValuePtr val1, ValuePtr val2, char* err){
    VarPtr ref = const_check(val1, err);
    if(ref == NULL)
        return create_val_err();
    if(val2->type == VAR_REF)
        val2 = var_deref(val2);
    destroy_val(&ref->value);

    copy_val(&ref->value, val2, sizeof(Value));
    return *val1;
}

Value opfunc_pos(ValuePtr val, char* err){
    V_UNUSED(err);
    Number res;
    copy_num(&res, val->value,sizeof(Number));
    return create_val(res);
}

Value opfunc_neg(ValuePtr val, char* err){
    V_UNUSED(err);
    Number res;
    copy_num(&res, val->value,sizeof(Number));
    res.sign *= -1;
    return create_val(res);
}

Value opfunc_inc_l(ValuePtr val, char* err){
    VarPtr ref = const_check(val, err);
    if(ref == NULL)
        return create_val_err();
    if(!val_to_num(&ref->value)){
        strcpy(err,"increment can only be applied to number variable:");
    }
    NumberPtr v = ref->value.value;
    move_num(v, num_addi(v, 1));
    return *val;
}

Value opfunc_inc_r(ValuePtr val, char* err){
    VarPtr ref = const_check(val, err);
    if(ref == NULL)
        return create_val_err();
    if(!val_to_num(&ref->value)){
        strcpy(err,"increment can only be applied to number variable:");
        return create_val_err();
    }
    NumberPtr v = ref->value.value;
    Number ret;
    copy_num(&ret, v, sizeof(Number));
    move_num(v, num_addi(v, 1));
    return create_val(ret);
}

Value opfunc_dec_l(ValuePtr val, char* err){
    VarPtr ref = const_check(val, err);
    if(ref == NULL)
        return create_val_err();
    if(!val_to_num(&ref->value)){
        strcpy(err,"decrement can only be applied to number variable:");
        return create_val_err();
    }
    NumberPtr v = ref->value.value;
    move_num(v, num_addi(v, -1));
    return *val;
}

Value opfunc_dec_r(ValuePtr val, char* err){
    VarPtr ref = const_check(val, err);
    if(ref == NULL)
        return create_val_err();
    if(!val_to_num(&ref->value)){
        strcpy(err,"decrement can only be applied to number variable:");
        return create_val_err();
    }
    NumberPtr v = ref->value.value;
    Number ret;
    copy_num(&ret, v, sizeof(Number));
    move_num(v, num_addi(v, -1));
    return create_val(ret);
}

Value opfunc_call(ValuePtr func, Vector args, char *err){
    if(func->type == VAR_REF)
        func = var_deref(func);
    if(func->type != VAR_FUN){
        strcpy(err, "cannot call a non-function variable.");
        return create_val_err();
    }
    BuiltinFunc func_ptr = func->value;
    return func_ptr(args, err);
}

Value builtin_func_max(Vector args, char* err){
    if(args.size == 0){
        strcpy(err, "expect at least 1 argument:");
        return create_val_err();
    }

    ValuePtr mx_ele = NULL;
    FOR_EACH(Value, val, args)
        if(!val_to_num(val)){
            strcpy(err, "argument should be number:");
            return create_val_err();
        }
        if(mx_ele == NULL || num_lt(mx_ele->value, val->value))
            mx_ele = val;
    END_FOR();
    Number res;
    copy_num(&res, mx_ele->value, sizeof(Number));
    return create_val(res);
}

Value builtin_func_min(Vector args, char* err){
    if(args.size == 0){
        strcpy(err, "expect at least 1 argument:");
        return create_val_err();
    }

    ValuePtr mn_ele = NULL;
    FOR_EACH(Value, val, args)
        if(!val_to_num(val)){
            strcpy(err, "argument should be number:");
            return create_val_err();
        }
        if(mn_ele == NULL || num_gt(mn_ele->value, val->value))
            mn_ele = val;
    END_FOR();
    Number res;
    copy_num(&res, mn_ele->value, sizeof(Number));
    return create_val(res);
}

Value builtin_func_print(Vector args, char* err){
    V_UNUSED(err);
    FOR_EACH(Value, val, args)
        if(CUR_IDX != 0)
            putchar(' ');
        print_val(val); 
    END_FOR();
    puts("");
    return create_val_void();
}

Value builtin_func_set_place(Vector args, char* err){
    int place = split_int(args, err);
    if(strlen(err) > 0)
        return create_val_err();
    if(place > 1000 || place < 0){
        strcpy(err, "argument out of bound [10,100000]:");
        return create_val_err();
    }
    global.place = place;
    return create_val_void();
}

Value builtin_func_set_prec(Vector args, char* err){
    int prec = split_int(args, err);
    if(strlen(err) > 0)
        return create_val_err();
    if(prec > 1000 || prec < 0){
        strcpy(err, "argument out of bound [1,1000]:");
        return create_val_err();
    }
    global.prec = prec;
    return create_val_void();
}

Value builtin_func_set_eps(Vector args, char* err){
    int eps = split_int(args, err);
    if(strlen(err) > 0)
        return create_val_err();
    if(eps > 1000 || eps < -100){
        strcpy(err, "argument out of bound [-100,1000]:");
        return create_val_err();
    }
    global.eps = eps;
    return create_val_void();
}

Value builtin_func_floor(Vector args, char *err){
    if(args.size != 1){
        strcpy(err, "expect exactly 1 argument:");
        return create_val_err();
    }
    ValuePtr val = get_addr(args, 0);
    if(!val_to_num(val)){
        strcpy(err, "argument should be number:");
        return create_val_err();
    }
    return create_val(num_floor(val->value));
}

Value builtin_func_ceil(Vector args, char *err){
    if(args.size != 1){
        strcpy(err, "expect exactly 1 argument:");
        return create_val_err();
    }
    ValuePtr val = get_addr(args, 0);
    if(!val_to_num(val)){
        strcpy(err, "argument should be number:");
        return create_val_err();
    }
    return create_val(num_ceil(val->value));
}

Value builtin_func_round(Vector args, char *err){
    if(args.size != 1){
        strcpy(err, "expect exactly 1 argument:");
        return create_val_err();
    }
    ValuePtr val = get_addr(args, 0);
    if(!val_to_num(val)){
        strcpy(err, "argument should be number:");
        return create_val_err();
    }
    return create_val(num_round(val->value, 0));
}

Value builtin_func_exit(Vector args, char *err){
    if(args.size > 0){
        strcpy(err, "expect no argument:");
        return create_val_err();
    }
    exit(0);
}

Value builtin_func_clear(Vector args, char* err){
    if(args.size > 0){
        strcpy(err, "expect no argument:");
        return create_val_err();
    }
    system("clear");
    return create_val_void();
}

Value builtin_func_set_fixflag(Vector args, char *err){
    if(args.size != 1){
        strcpy(err, "expect exactly 1 argument:");
        return create_val_err();
    }
    ValuePtr val = get_addr(args, 0);
    if(val->type == VAR_REF)
        val = var_deref(val);
    if(val->type != VAR_BOOL){
        strcpy(err, "argument should be bool:");
        return create_val_err();
    }
    global.flag_fix = *(bool *)val->value;
    return create_val_void();
}
/*======================main:evaluator=======================*/

void eval_init(Vector vec){
    match_barket(vec);
    unary_check(vec);
}

//calculate two operands and store them to val_l, val_r.
#define PREPARE_BIN_OP(type_l, type_r) \
    do{\
        bool flag = sub_eval(token, l, p - 1, type_l)\
                && sub_eval(token, p + 1, r, type_r);\
        if(!flag){\
            throw_exception(ERR_INVALID_EXPR, except_info(token, l, r, "Expect more operand in the expression: "));\
        }\
        val_r = pop_arg();\
        val_l = pop_arg();\
    }while(0)

//calculate operand and store it to val_l.
#define PREPARE_UNARY_OP(type, lp, rp)\
    do{\
        if(l == lp && rp != r - 1){\
            throw_exception(ERR_INVALID_EXPR, except_info(token, l, r, "too many arguments for unary operator: "));\
        }\
        if(!sub_eval(token, lp, rp, type))\
            throw_exception(ERR_INVALID_EXPR, except_info(token, l, r, "Expect more operand in the expression: "));\
        val_l = pop_arg();\
    }while(0)\

//destroy val_l, val_r
#define PREPARE_BIN_END\
    do{\
        destroy_val(&val_l);\
        destroy_val(&val_r);\
    }while(0)

//destroy val_l
#define PREPARE_UNARY_END destroy_val(&val_l);

//check whether an operand can be convert to number
#define NUM_CHECK(v)\
    do{\
        if(!val_to_num(&v)){\
            PREPARE_BIN_END;\
            throw_exception(ERR_INVALID_EXPR,\
             except_info(token, l, r, "Operand is not a number:"));\
        }\
    }while(0)

//setup a binary operator
#define SET_BIN_OP(sym, type_l, type_r, func)\
    do{\
        if(cont_eq(*root, sym)){\
            PREPARE_BIN_OP(type_l, type_r);\
            if(type_l == VAR_NUM)\
                NUM_CHECK(val_l);\
            if(type_r == VAR_NUM)\
                NUM_CHECK(val_r);\
            Value ret = func(&val_l, &val_r, err_info);\
            if(ret.type == VAR_ERR || strlen(err_info) > 0){\
                PREPARE_BIN_END;\
                if(strlen(err_info) > 0 && ret.type == VAR_NUM)\
                    destroy_val(&ret);\
                throw_exception(ERR_ARITHMETIC, except_info(token, l, r, err_info));\
                return true;\
            }\
            push_arg(&ret);\
            destroy_val(&ret);\
            PREPARE_BIN_END;\
            return true;\
        }\
    }while(0)

//setup a unary operator
#define SET_UNARY_OP(sym, argtype, func, lp, rp)\
    do{\
        if(cont_eq(*root,sym)){\
            PREPARE_UNARY_OP(argtype, lp, rp);\
            if(argtype == VAR_NUM)\
                NUM_CHECK(val_l);\
            Value ret = func(&val_l, err_info);\
            if(ret.type == VAR_ERR || strlen(err_info) > 0){\
                PREPARE_UNARY_END;\
                if(strlen(err_info) > 0 && ret.type == VAR_NUM)\
                    destroy_val(&ret);\
                throw_exception(ERR_ARITHMETIC, except_info(token, l, r, err_info));\
                return true;\
            }\
            push_arg(&ret);\
            destroy_val(&ret);\
            PREPARE_UNARY_END;\
            return true;\
        }\
    }while(0)\

#define SET_UNARY_LEFT_OP(sym, type, func) SET_UNARY_OP(sym, type, func, p + 1, r)
#define SET_UNARY_RIGHT_OP(sym, type, func) SET_UNARY_OP(sym, type, func, l, p - 1)

bool sub_eval(VectorPtr token, int l, int r, VarType expect_type);

Vector args_eval(VectorPtr token, int l, int r){
    Vector res = create_vec(sizeof(Value), destroy_val, copy_val);
    if(l > r)
        return res;
    int r_pos = r + 1, l_pos = r, argc = 0;
    while(r_pos >= l){
        TokenPtr cur = get_addr(*token, l_pos);
        while(l_pos >= l && !cont_eq(*cur, ",")){
            if(cur->match_pos != -1)
                l_pos = cur->match_pos;
            cur = get_addr(*token, --l_pos);
        }
        if(!sub_eval(token, l_pos + 1, r_pos - 1, VAR_REF)){
            destroy_vec(&res);
            throw_exception(ERR_INVALID_EXPR,
                except_info(token, l, r, "expect expression in argument list:"));
        }
        ++argc;
        r_pos = l_pos--;
    }
    while(argc--){
        Value val = pop_arg();
        push_back(&res, &val);
        destroy_val(&val);
    }
    return res;
}

bool sub_eval(VectorPtr token, int l, int r, VarType expect_type){

    if(l > r)
        return false;

    TokenPtr root = (TokenPtr)get_addr(*token, l);
    if(root->match_pos == r){
        if(!sub_eval(token, l + 1, r - 1, expect_type))
            throw_exception(ERR_INVALID_EXPR,create_str("expect expression inside '()'."));
        return true;
    }

    int p = l, lv = get_l_level(*root);
    for(int i = l; i <= r; ++i){
        TokenPtr cur = get_addr(*token, i);
        if(lv > get_r_level(*cur)){
            root = cur;
            p = i;
            lv = get_l_level(*cur);
        }else{
            lv = fmin(lv, get_l_level(*cur));
        }
        if(cur->match_pos != -1)
            i = cur->match_pos;
    }

    char err_info[50] = {""};
    if(root->type == TOKEN_NUMBER){
        if(p != l){
            throw_exception(ERR_INVALID_EXPR, except_info(token, l, r, "Too many expression:"));
        }
        Value val = create_val(str_to_num(&root->cont));
        push_arg_num(&val);
        destroy_val(&val);
        return true;
    }

    if(root->type == TOKEN_NAME){
        if(p != l){
            throw_exception(ERR_INVALID_EXPR, except_info(token, l, r, "Too many expression:"));
        }
        if(expect_type == VAR_REF){
            VarPtr v = lookup_var(root->cont);
            Value val = {VAR_REF, v};
            push_arg(&val);
        }else if(expect_type == VAR_ANY){
            VarPtr v = lookup_var_notnull(root->cont);
            Value val = {VAR_REF, v};
            push_arg(&val);
        }else{
            VarPtr v = lookup_var_notnull(root->cont);
            push_arg(&v->value);
        }
        return true;
    }

    Value val_l, val_r;

    SET_BIN_OP("+", VAR_NUM, VAR_NUM, opfunc_add);
    SET_BIN_OP("-", VAR_NUM, VAR_NUM, opfunc_sub);
    SET_BIN_OP("*", VAR_NUM, VAR_NUM, opfunc_mul);
    SET_BIN_OP("/", VAR_NUM, VAR_NUM, opfunc_div);
    SET_BIN_OP("<", VAR_NUM, VAR_NUM, opfunc_lt);
    SET_BIN_OP(">", VAR_NUM, VAR_NUM, opfunc_gt);
    SET_BIN_OP("<=", VAR_NUM, VAR_NUM, opfunc_le);
    SET_BIN_OP(">=", VAR_NUM, VAR_NUM, opfunc_ge);
    SET_BIN_OP("==", VAR_NUM, VAR_NUM, opfunc_eq);
    SET_BIN_OP("!=", VAR_NUM, VAR_NUM, opfunc_neq);
    SET_BIN_OP("=", VAR_REF, VAR_ANY, opfunc_set);

    SET_UNARY_LEFT_OP("+!", VAR_NUM, opfunc_pos);
    SET_UNARY_LEFT_OP("-!", VAR_NUM, opfunc_neg);
    SET_UNARY_LEFT_OP("++!", VAR_ANY, opfunc_inc_l);
    SET_UNARY_LEFT_OP("--!", VAR_ANY, opfunc_dec_l);

    SET_UNARY_RIGHT_OP("++", VAR_ANY, opfunc_inc_r);
    SET_UNARY_RIGHT_OP("--", VAR_ANY, opfunc_dec_r);

    if(cont_eq(*root,"(")){
        if(!sub_eval(token, l, p - 1, VAR_FUN))
            throw_exception(ERR_INVALID_EXPR, except_info(token, l, r,"Expect more operand in the expression: "));
        Vector args = args_eval(token, p + 1, root->match_pos - 1);
        Value func = pop_arg();
        Value ret = opfunc_call(&func, args, err_info);
        if(ret.type == VAR_ERR){
            destroy_val(&func);
            destroy_vec(&args);
            throw_exception(ERR_ARITHMETIC, except_info(token, l, r, err_info));
        }
        if(ret.type == VAR_VOID){
            if(expect_type != VAR_VOID)
                throw_exception(ERR_ARITHMETIC, except_info(token, l, r, "This function has no return value: "));
        }else{
            push_arg(&ret);
        }
        destroy_val(&ret);
        destroy_val(&func);
        destroy_vec(&args);
        return true;
    }

    throw_exception(ERR_INVALID_EXPR, except_info(token, l, r,"Unknown operator: "));
    return false;
}

#undef PREPARE_BIN_OP
#undef PREPARE_UNARY_OP
#undef PREPARE_BIN_END
#undef PREPARE_UNARY_END
#undef NUM_CHECK
#undef SET_BIN_OP
#undef SET_UNARY_LEFT_OP
#undef SET_UNARY_RIGHT_OP

/**
 * @brief evaluate tokenized expression.
 * result will bed saved in struct `global`.
 * @param token Vector of Token type.
*/
void evaluate(Vector token){
    eval_init(token);
    sub_eval(&token, 0, token.size - 1, VAR_VOID);
    if(global.param_stk.size > 0){
        builtin_set_var("last",get_back(global.param_stk));
    }
}

/**
 * @brief print result saved in `global`,
 * or show exception detail if evaluation failed. 
*/
void print_result(Vector tokens){
    Vector ex_info = global.ex_info;
    if(any_ex_happened()){
        puts("ERROR:");
        
        FOR_EACH(Exception, e, ex_info)
            printf("[%s]: %s\n",ERROR_DESC[e->err_code],as_cstr(e->desc));
        END_FOR();
        return;
    }
    if(global.param_stk.size > 0){
        String cont = recover_cont(tokens, 0 ,tokens.size - 1);
        push_string(&cont, " = ");
        printf("%s",as_cstr(cont));
        destroy_vec(&cont);
        print_val(get_back(global.param_stk));
        puts("");
    }
}

/*======================main: input=======================*/

String get_line(){
    String res = create_str_emp();
    char c = getchar();

    while(c != '\n' && c != EOF){
        push_char(&res, c);
        c=getchar();
    }

    return res;
}

//evaluate expression from std input.
void cmd_run(){
    while(1){
        String str;
        Vector tokens;
        bool init_flag = false;
        
        ErrCode err_code = setjmp(global.buf);

        if(err_code == NO_EXCEPTION){
            global_flash();
            printf(">>> ");
            str = get_line();
            char *ptr = str.data; 
            tokens = tokenize(1, &ptr);

            if(tokens.size == 0){
                destroy_vec(&tokens);
                destroy_vec(&str);
                continue;
            }

            init_flag = true;

            evaluate(tokens);
        }

        //long jump will jump to here!
        print_result(tokens);

        destroy_vec(&str);
        if(init_flag)
            destroy_vec(&tokens);
    }
}

//calculate only one line
void calc_line(int argc, char **argv){
    Vector tokens;
    ErrCode err_code = setjmp(global.buf);

    if(err_code == NO_EXCEPTION){
        tokens = tokenize(argc, argv);
        evaluate(tokens);
    }

    //long jump will jump to here!
    print_result(tokens);
}

int main(int argc, char *argv[]){
    global_init();
    if(argc > 1){
        calc_line(argc - 1, argv + 1);
    }else{
        cmd_run();
    }
}

/*======================vector impl=======================*/

Vector create_vec(int size,Destructor destroy,CopyConstructor copy){
    Vector res={
            malloc(DEFAULT_CAPACITY * size),
            size,
            0,
            DEFAULT_CAPACITY,
            destroy,
            copy
    };
    return res;
}

//create vector with default element destructor and copy constructor
Vector create_vec_default(int size){
    return create_vec(size,default_destructor,default_copy);
}

//get object's address by index
ObjectPtr get_addr(Vector vec, int idx){
    return (char *)vec.data + vec.width * idx;
}

//get the last object's address
ObjectPtr get_back(Vector vec){
    return get_addr(vec,vec.size - 1);
}

//get i-th object and copy it to destination
void get_object(Vector vec, ObjectPtr dst, int idx){
    if(idx >= vec.size)
        return;
    vec.copy(dst,get_addr(vec,idx),vec.width);
}

//set i-th object
void set_object(VectorPtr vec, ObjectPtr value, int idx){
    vec->destroy(get_addr(*vec,idx));
    vec->copy(get_addr(*vec,idx),value,vec->width);
}

//vector destructor
void destroy_vec(ObjectPtr object){
    VectorPtr vec=(VectorPtr)object;

    if(vec->data == NULL)
        return;

    for (int i = 0; i < vec->size; ++i)
        vec->destroy(get_addr(*vec,i));
    free(vec->data);

    vec->data = NULL;
}

//clear the vector (won't free the allocated memory).
void clear_vec(VectorPtr vec){
    for (int i = 0; i < vec->size; ++i)
        vec->destroy(get_addr(*vec,i));
    vec->size = 0;
}

void copy_vec(ObjectPtr _dst, ObjectPtr _src, size_t ignored){
    VectorPtr dst = (VectorPtr)_dst;
    VectorPtr src = (VectorPtr)_src;
    
    *dst = *src;
    dst->data = malloc(src->capacity * src->width);

    for(int i = 0 ; i < src->size ; ++i){
        src->copy(get_addr(*dst,i),get_addr(*src,i),src->width);
    }
    V_UNUSED(ignored);
}

/**
 * increase the allocated memory size if not enough.
 * won't free any memory.
*/
void vec_recapacity(VectorPtr vec, size_t new_size){
    size_t new_capacity = vec->capacity;

    while (new_capacity < new_size)
        new_capacity *= 2;
    
    if(new_capacity != vec->capacity){
        vec->capacity = new_capacity;
        vec->data = realloc(vec->data,new_capacity * vec->width);
    }
}

//push a copy of `value` to the vector
void push_back(VectorPtr vec, ObjectPtr value){
    int tail = vec->size++;

    if((size_t)vec->size >= vec->capacity)
        vec_recapacity(vec,vec->size);
    vec->copy(get_addr(*vec,tail),value,vec->width);
}

/**
 * @brief Pop the last element and MOVE it to the `dst`.
 * Hence the object should be destroyed outside.
*/
void pop_back(VectorPtr vec, ObjectPtr dst){
    int tail = --vec->size;
    memcpy(dst, get_addr(*vec, tail), vec->width);
}

/*======================string impl=======================*/

String create_str(const char *str){
    String res=create_vec_default(sizeof(char));
    int p=0;
    do{
        push_back(&res, (char *)str + p);
    } while (str[p++] != '\0');
    return res;
}

String create_str_emp(){
    return create_str("");
}

//view string as const char*
const char *as_cstr(String str){
    return str.data;
}

//append a char
void push_char(StringPtr str, char c){
    char val_zero = '\0';
    set_object(str,&c,str->size - 1);
    push_back(str,&val_zero);
}

//get character by index
char char_at(String str, int idx){
    return *(char *)get_addr(str,idx);
}

//append a string
void push_string(StringPtr str1, const char* str2){
    for(int i = 0; str2[i]!='\0'; ++i)
        push_char(str1, str2[i]);
}

bool str_eq(String str1, String str2){
    return strcmp(as_cstr(str1), as_cstr(str2)) == 0;
}

/*======================variable impl=======================*/

Value create_val(Number num){
    Value value={VAR_NUM, malloc(sizeof(Number))};
    *(NumberPtr)value.value = num;
    return value;
}

Value create_val_zero(){
    Value value={VAR_NUM, malloc(sizeof(Number))};
    *(NumberPtr)value.value = create_num();
    return value;
}

Value create_val_bool(bool val){
    Value value={VAR_BOOL, malloc(sizeof(bool))};
    *(bool *)value.value = val;
    return value;
}

Value create_val_void(){
    Value value = {VAR_VOID, NULL};
    return value; 
}

Value create_val_err(){
    Value value = {VAR_ERR, NULL};
    return value; 
}

Value create_val_func(BuiltinFunc func){
    Value value = {VAR_FUN, func};
    return value; 
}

Variable create_var(String name, bool is_const){
    Variable var = {create_val_zero(), create_str(as_cstr(name)), is_const};
    return var;
}

void destroy_val(ObjectPtr obj){
    ValuePtr val = (ValuePtr) obj;
    if(val->type == VAR_NUM || val->type == VAR_BOOL)
        free(val->value);
    val->value = NULL;
}

void destroy_var(ObjectPtr obj){
    VarPtr var= (VarPtr) obj;
    destroy_val(&var->value);
    destroy_vec(&var->name);
}

void copy_val(ObjectPtr _dst, ObjectPtr _src, size_t size){
    V_UNUSED(size);    
    ValuePtr dst = (ValuePtr)_dst;
    ValuePtr src = (ValuePtr)_src;
    *dst = *src;
    if(src->type == VAR_BOOL){
        dst->value = malloc(sizeof(bool));
        *(bool*) dst->value = *(bool*) src->value;
    }else if(src->type == VAR_NUM){
        dst->value = malloc(sizeof(Number));
        copy_num(dst->value, src->value, sizeof(Number));
    }
}

void copy_var(ObjectPtr _dst, ObjectPtr _src, size_t size){
    V_UNUSED(size);
    VarPtr dst = (VarPtr)_dst;
    VarPtr src = (VarPtr)_src;
    dst->is_const = src->is_const;
    copy_val(&dst->value, &src->value, sizeof(Value));
    copy_vec(&dst->name, &src->name, sizeof(Vector));
}

void print_val(ValuePtr ptr){
    if(ptr->type == VAR_BOOL){
        bool *v = ptr->value;
        printf(*v ? "true" : "false");
    }else if(ptr->type == VAR_NUM){
        NumberPtr v = ptr->value;
        String str = num_to_str(v);
        printf("%s", as_cstr(str));
        destroy_vec(&str);
    }else if(ptr->type == VAR_REF){
        VarPtr ref = ptr->value;
        print_val(&ref->value);
    }else{
        printf("function");
    }
}

//check whether a value can be modified
VarPtr const_check(ValuePtr val, char* err){
    if(val->type != VAR_REF){
        strcpy(err,"Can not modify a rvalue in expression:");
        return NULL;
    }
    VarPtr ref = val->value;
    // printf("@%s\n",as_cstr(ref->name));
    if(ref->is_const){
        strcpy(err,"Can not modify a const variable in expression:");
        return NULL;
    }
    return ref;
}

ValuePtr var_deref(ValuePtr var){
    return &(((VarPtr)(var->value))->value);
}

VarPtr lookup_var_byname(String name){
    FOR_EACH(Variable, var, global.var_table)
        if(str_eq(var->name, name))
            return var;
    END_FOR();
    return NULL;
}

VarPtr lookup_var_notnull(String name){
    VarPtr res = lookup_var_byname(name);
    if(res == NULL){
        String info = create_str("variable '");
        push_string(&info,as_cstr(name));
        push_string(&info, "' is undefined");
        throw_exception(ERR_UNKNOWN_NAME, info);
    }
    return res;
}

VarPtr lookup_var(String name){
    VarPtr res = lookup_var_byname(name);
    if(res == NULL){
        Variable var = create_var(name, false);
        push_back(&global.var_table, &var);
        destroy_var(&var);
        res = lookup_var_byname(name);
    }
    return res;
}

/**
 * @brief Check if the value can be coverted to a number type. 
 * If so, convert it to a number
 * @param var The value to be converted
 * @return true if the conversion is successful, otherwise returns false
 */
bool val_to_num(ValuePtr var){
    if(var->type == VAR_BOOL){
        //convert bool to number
        NumberPtr nval = malloc(sizeof(Number));
        bool *bval = var->value;
        *nval = create_num_i(*bval);
        free(var->value);
        var->value = nval;
        var->type = VAR_NUM;
        return true;
    }else if(var->type == VAR_REF){
        ValuePtr val = var_deref(var);
        copy_val(var, val, sizeof(Number));
        return val_to_num(var);
    }else{
        return var->type == VAR_NUM;
    }
}

//create builtin variable
void builtin_create_var(const char* name){
    String name_str = create_str(name);
    Variable var = create_var(name_str,true);
    var.value = create_val_zero();
    push_back(&global.var_table, &var);
    destroy_var(&var);
    destroy_vec(&name_str);
}


//set builtin variable
void builtin_set_var(const char* name, ValuePtr nval){
    FOR_EACH(Variable, var, global.var_table)
        if(strcmp(as_cstr(var->name),name) == 0){
            destroy_val(&var->value);
            copy_val(&var->value,nval,sizeof(Value));
            return;
        }
    END_FOR();
}

/*======================context impl=======================*/

void destroy_ex(ObjectPtr obj){
    ExceptionPtr ex = (ExceptionPtr)obj;
    destroy_vec(&ex->desc);
}

void copy_ex(ObjectPtr _dst, ObjectPtr _src, size_t size){
    V_UNUSED(size);
    ExceptionPtr dst = (ExceptionPtr)_dst;
    ExceptionPtr src = (ExceptionPtr)_src;
    *dst = *src;
    copy_vec(&dst->desc,&src->desc,sizeof(Vector));
}

void link_function(const char* name, BuiltinFunc func){
    builtin_create_var(name);
    Value func_val = create_val_func(func);
    builtin_set_var(name,&func_val);
}

void global_init(){
    global.ex_info = create_vec(sizeof(Exception), destroy_ex, copy_ex);
    global.param_stk = create_vec(sizeof(Value), destroy_val, copy_val);
    global.var_table = create_vec(sizeof(Variable), destroy_var, copy_var);
    global.place = 200;
    global.eps = 40;
    global.prec = 15;
    global.flag_fix = true;

    vec_recapacity(&global.var_table, 100);

    //set builtin variable
    builtin_create_var("last");
    builtin_create_var("true");
    builtin_create_var("false");

    link_function("max", builtin_func_max);
    link_function("min", builtin_func_min);
    link_function("print", builtin_func_print);
    link_function("set_place", builtin_func_set_place);
    link_function("set_prec", builtin_func_set_prec);
    link_function("set_eps", builtin_func_set_eps);
    link_function("ceil", builtin_func_ceil);
    link_function("floor", builtin_func_floor);
    link_function("round", builtin_func_round);
    link_function("exit", builtin_func_exit);
    link_function("clear",builtin_func_clear);
    link_function("set_fixflag", builtin_func_set_fixflag);

    Value bval = create_val_bool(true);
    builtin_set_var("true", &bval);
    *(bool *)bval.value = false;
    builtin_set_var("false", &bval);
    destroy_val(&bval);
}

void global_flash(){
    clear_vec(&global.ex_info);
    clear_vec(&global.param_stk);
}

void throw_exception(ErrCode code, String desc){
    Exception e = {code, desc};
    push_back(&global.ex_info, &e);
    destroy_vec(&desc);
    longjmp(global.buf, code);
}

//concat the tokens content and error information
String except_info(VectorPtr token, int l, int r, const char* info){
    String res = create_str(info);
    String cont = recover_cont(*token, l, r);
    push_char(&res, ' ');
    push_char(&res, '\'');
    push_string(&res, as_cstr(cont));
    push_char(&res, '\'');
    destroy_vec(&cont);
    return res;
}

bool any_ex_happened(){
    return global.ex_info.size > 0;
}

void push_arg(ValuePtr var){
    push_back(&global.param_stk, var);
}

bool push_arg_num(ValuePtr var){
    
    if(!val_to_num(var))
        return false; 

    push_arg(var);
    return true;
}

Value pop_arg(){
    Value top;
    pop_back(&global.param_stk, &top);
    return top;
}

/*======================tokenizer impl=======================*/

bool cont_eq(Token token, const char *str){
    return strcmp(as_cstr(token.cont),str) == 0;
}

void print_token(TokenPtr token){
    printf("[%s]: %s aka %s, pos = %d\n"
            ,TOKEN_NAMES[token->type]
            ,as_cstr(token->raw_cont)
            ,as_cstr(token->cont)
            ,token->match_pos);
}

Token create_token(){
    Token res;
    res.type = TOKEN_UNDEFINED;
    res.cont = create_str_emp();
    res.raw_cont = create_str_emp();
    res.match_pos = -1;
    return res;
}

void destroy_token(ObjectPtr obj){
    TokenPtr token=(TokenPtr)obj;
    destroy_vec(&token->cont);
    destroy_vec(&token->raw_cont);
}

void copy_token(ObjectPtr _dst,ObjectPtr _src,size_t size){
    V_UNUSED(size);
    TokenPtr dst = (TokenPtr)_dst;
    TokenPtr src = (TokenPtr)_src;
    *dst = *src;
    copy_vec(&dst->cont, &src->cont, sizeof(String));
    copy_vec(&dst->raw_cont, &src->raw_cont, sizeof(String));
}

bool op_append_check(const char* cont, char c){
    String str = create_str(cont);
    push_char(&str,c);

    for(int i=0;i < OP_COUNT;i++)
        if(strcmp(as_cstr(str),OP_LIST[i]) == 0){
            destroy_vec(&str);
            return true;
        }
    destroy_vec(&str);
    return false;
}

//push token to the container, and reset the token.
void push_token(VectorPtr vec, TokenPtr token, int *number_flag){
    if(token->type == TOKEN_UNDEFINED)
        return;
    if(token->type == TOKEN_NUMBER){
        if(*number_flag == NUM_BEGIN)
            push_string(&token->cont,".0E+0");
        else if(*number_flag == NUM_DOT)
            push_string(&token->cont,"0E+0");
    }
    push_back(vec, token);
    destroy_token(token);
    *token = create_token();
    *number_flag = NUM_BEGIN;
    return;
}

void match_barket(Vector tokens){
    Vector stk=create_vec_default(sizeof(int));

    FOR_EACH(Token, token, tokens)
        if(cont_eq(*token,"(")){
            push_back(&stk, &CUR_IDX);
        }else if(cont_eq(*token,")")){
            int top;
            if(stk.size == 0)
                goto error;
            pop_back(&stk, &top);
            TokenPtr left_bk = (TokenPtr) get_addr(tokens, top);
            left_bk->match_pos = CUR_IDX;
            token->match_pos = top;
        }
    END_FOR();

    if(stk.size > 0)
        goto error;

    destroy_vec(&stk);
    return;

error:
    destroy_vec(&stk);
    throw_exception(ERR_INVALID_EXPR, create_str("barket mismatch"));
}

/**
 * @brief decide whether an operator is binary or unary.
 * An operator is unary iff:
 *   it's the first operator;
 *   it follows ')' or '++'/'--'
*/
void unary_check(Vector tokens){
    bool last_flag = true;
    FOR_EACH(Token, token, tokens)
        if(token->type == TOKEN_SYMBOL){
            if(cont_eq(*token, "+") || cont_eq(*token, "-") 
             ||cont_eq(*token, "++") || cont_eq(*token, "--"))
                if(last_flag)
                    push_char(&token->cont, '!');
            last_flag = !cont_eq(*token,")") && !cont_eq(*token, "++") && !cont_eq(*token, "--");
        }else{
            last_flag = false;
        }
    END_FOR();
}

Vector tokenize(int argc,char *words[]){
    Vector res = create_vec(sizeof(Token), destroy_token, copy_token);

    for(int i = 0; i < argc; ++i){
        String str = create_str(words[i]);
        Token token = create_token();
        int number_flag = NUM_BEGIN;

        for(int p = 0; p < str.size - 1;){
            char c = char_at(str, p);
            if(token.type == TOKEN_UNDEFINED){
                bool append_flag = true;
                if(isdigit(c)){
                    token.type = TOKEN_NUMBER;
                    push_char(&token.cont, c);
                }else if(c == ' ' || c == '\t'){
                    append_flag = false;
                }else if(c == '.'){
                    token.type = TOKEN_NUMBER;
                    number_flag = NUM_DOT;
                    push_string(&token.cont,"0.");
                }else if(isalpha(c) || c == '_'){
                    token.type = TOKEN_NAME;
                    push_char(&token.cont, c);
                }else if(op_append_check("",c)){
                    push_char(&token.cont, c);
                    token.type = TOKEN_SYMBOL;
                }else{
                    push_char(&token.raw_cont, c);
                    goto error; //error: unknown symbol
                }
                if(append_flag)
                    push_char(&token.raw_cont, c);
            }else if(token.type == TOKEN_NUMBER){

                if(number_flag == NUM_BEGIN){
                    if(isdigit(c)){
                        push_char(&token.cont, c);
                    }else if(c == '.'){
                        number_flag = NUM_DOT;
                        push_char(&token.cont, c);
                    }else if(c == 'e' || c == 'E'){
                        number_flag = NUM_EXP;
                        push_string(&token.cont, ".0E");
                    }else{
                        push_token(&res, &token, &number_flag);
                        continue;
                    }
                }else if(number_flag == NUM_DOT){
                    if(isdigit(c)){
                        push_char(&token.cont, c);
                    }else if(c == 'e' || c == 'E'){
                        number_flag = NUM_EXP;
                        push_string(&token.cont, "0E");
                    }else{
                        push_token(&res, &token, &number_flag);
                        continue;
                    }
                }else if(number_flag == NUM_EXP){
                    if(isdigit(c)){
                        number_flag = NUM_FINISH;
                        push_char(&token.cont, '+');
                        push_char(&token.cont, c);
                    }else if(c == '+' || c == '-'){
                        number_flag = NUM_SIGN;
                        push_char(&token.cont, c);
                    }else{
                        goto error; // incomplete number
                    }
                }else if(number_flag == NUM_SIGN){
                    if(isdigit(c)){
                        number_flag = NUM_FINISH;
                        push_char(&token.cont, c);
                    }else{
                        goto error; // incomplete number
                    }
                }else if(number_flag == NUM_FINISH){
                    if(isdigit(c)){
                        push_char(&token.cont, c);
                    }else{
                        push_token(&res, &token, &number_flag);
                        continue;
                    }
                }
                push_char(&token.raw_cont, c);
            }else if(token.type == TOKEN_NAME){

                if(isalnum(c) || c == '_'){
                    push_char(&token.cont, c);
                    push_char(&token.raw_cont, c);
                }else{
                    push_token(&res, &token, &number_flag);
                    continue;
                }

            }else if(token.type == TOKEN_SYMBOL){
                if(op_append_check(as_cstr(token.cont), c)){
                    push_char(&token.cont, c);
                    push_char(&token.raw_cont, c);
                }else{
                    push_token(&res,&token,&number_flag);
                    continue;
                }
            }
            ++p;
        }

        if(token.type == TOKEN_NUMBER)
            if(number_flag == NUM_SIGN || number_flag == NUM_EXP)
                goto error; // incomplete number
        push_token(&res, &token, &number_flag);

        destroy_token(&token);
        destroy_vec(&str);

        continue;

    error:
        destroy_vec(&str);
        destroy_vec(&res);
        destroy_vec(&token.cont);
        throw_exception(ERR_INVALID_TOKEN, token.raw_cont);
    }
    return res;
}

//recover the original text of tokens.
String recover_cont(Vector tokens, int l, int r){
    String res = create_str_emp();
    bool space_flag = false;
    FOR_RANGE(Token, token, tokens, l, r + 1)
        if(space_flag && token->type != TOKEN_SYMBOL)
            push_char(&res, ' ');
        push_string(&res, as_cstr(token->raw_cont));
        if(token->type != TOKEN_SYMBOL || cont_eq(*token, ","))
            space_flag = true;
        else space_flag = false;
    END_FOR();
    return res;
}

/*======================big-number impl=======================*/

bool get_fixed_flag(){
    return global.flag_fix;
}

int get_dec_place(){
    return global.place;
}

int get_precision(){
    return global.prec;
}

int get_eps(){
    return global.eps;
}

void swap_numptr(NumberPtr *a, NumberPtr *b){
    NumberPtr tmp = *a;
    *a = *b;
    *b = tmp;
}

Number create_num_of_size(int size){
    Number res = {
        calloc(size, sizeof(char)),
        size,
        0,
        1
    };
    return res;
}

Number create_num_of_flag(NumFlag flag){
    Number res = create_num();
    res.sign = flag;
    return res;
}

Number create_num(){
    return create_num_of_size(get_dec_place());
}

Number create_num_i(int i){
    char buf[20];
    bool flag = false;
    if(i < 0){
        flag = true;
        i = -i;
    }
    sprintf(buf, "%d.0E+0",i);
    String str = create_str(buf);
    Number res = str_to_num(&str);
    if(flag)
        res.sign = -1;
    destroy_vec(&str);
    return res;
}

void destroy_num(ObjectPtr ptr){
    NumberPtr num = ptr;
    if(num->digits != NULL)
        free(num->digits);
    num->digits = NULL;
}

void copy_num(ObjectPtr _dst, ObjectPtr _src, size_t size){
    V_UNUSED(size);
    NumberPtr dst = _dst;
    NumberPtr src = _src;
    *dst = *src;
    dst->digits = malloc(src->place * sizeof(char));
    memcpy(dst->digits, src->digits, src->place * sizeof(char));
}

void move_num(NumberPtr dst, Number src){
    destroy_num(dst);
    *dst = src;
    src.digits = NULL;
}

void num_adjust(NumberPtr num){
    if(num_invalid(num))
        return;
    int place = get_dec_place();
    if(place != num->place){
        num->digits = realloc(num->digits, place);
        if(place > num->place)
            memset(num->digits + num->place, 0, sizeof(char) * (place - num->place));
        num->place = place;
    }
}

bool num_is_inf(NumberPtr num){
    return num->sign == NUM_PINF || num->sign == NUM_NINF;
}

bool num_is_nan(NumberPtr num){
    return num->sign == NUM_NAN;
}

bool num_invalid(NumberPtr num){
    return num_is_inf(num) || num_is_nan(num);
}

void num_check(NumberPtr num){
    //check out of bound error
    if(num_invalid(num))
        return;
    if(num->exp > NUM_SCALE){
        if(num->sign == NUM_NEG)
            num->sign = NUM_NINF;
        else num->sign = NUM_PINF;
        return;
    }
    //reset zero
    if(num_is_zero(num)){
        num->sign = 1;
        num->exp = 0;
    }
}

void num_rm_leading_zeros(NumberPtr num){
    int p = 0;
    while(p < num->place && num->digits[p] == 0)
        ++p;
    if(p == num->place){
        num->sign = 1;
        num->exp = 0;
        return;
    }
    num_lshift(num, p);
    num->exp -= p;
}

void num_lshift(NumberPtr num, int shift_n){
    if(shift_n == 0)
        return;
    if(shift_n >= num->place){
        memset(num->digits, 0, sizeof(char) * num->place);
        return;
    }
    memmove(num->digits, num->digits + shift_n, sizeof(char) * (num->place - shift_n));
    memset(num->digits + num->place - shift_n, 0, sizeof(char) * shift_n);
}

void num_rshift(NumberPtr num, int shift_n){
    if(shift_n == 0)
        return;
    if(shift_n >= num->place){
        memset(num->digits, 0, sizeof(char) * num->place);
        return;
    }
    memmove(num->digits + shift_n, num->digits, sizeof(char) * (num->place - shift_n));
    memset(num->digits, 0, sizeof(char) * shift_n);
}

String num_to_str_fixed(NumberPtr num){
    String res = create_str_emp();
    int prec = get_precision();

    if(num->sign < 0)
        push_char(&res, '-');    

    if(num_is_inf(num)){
        push_string(&res, "Infinity");
        return res;
    }

    if(num_is_nan(num)){
        push_string(&res, "Not a Number");
        return res;
    }

    if(num->exp < 0){
        push_char(&res, '0');
        if(prec == 0)
            return res;
        push_char(&res, '.');
        int exp = -num->exp - 1;
        while(exp > 0 && prec > 0){
            push_char(&res, '0');
            exp--;
            prec--;
        }
        for(int i = 0; i < num->place && prec > 0; ++i, --prec)
            push_char(&res, num->digits[i] + '0');
    }else{
        int exp = num->exp + 1;
        for(int i = 0; i < num->place && prec >= 0; ++i){
            if(exp == 0)
                push_char(&res, '.');
            push_char(&res, num->digits[i] + '0');
            exp--;
            if(exp <= 0)
                prec--;
        }

        if(exp > 0){
            while(exp > 0){
                push_char(&res, '0');
                exp--;
            }
            if(prec > 0){
                push_char(&res, '.');
                while(prec > 0){
                    push_char(&res, '0');
                    prec--;
                }
            }
        }
    }
    return res;
}

String num_to_str_sc(NumberPtr num){
    String res = create_str_emp();

    if(num->sign < 0)
        push_char(&res, '-');    

    if(num_is_inf(num)){
        push_string(&res, "Infinity");
        return res;
    }

    if(num_is_nan(num)){
        push_string(&res, "Not a Number");
        return res;
    }

    push_char(&res, num->digits[0] + '0');
    
    int prec = get_precision();
    if(prec > 0){
        push_char(&res, '.');
        for(int i = 1; i < num->place && prec > 0; ++i, --prec)
            push_char(&res, num->digits[i] + '0');
    }

    char buf[20];
    sprintf(buf, "e%d", num->exp);
    push_string(&res, buf);
    return res;
}

Number str_to_num(StringPtr str){
    int exp = -1, slen = strlen(as_cstr(*str));
    String int_str = create_str_emp();
    String exp_str = create_str_emp();
    Number res = create_num();
    
    int p, sign = 1;
    for(p = 0; p < slen; ++p){
        char c = char_at(*str, p);
        if(c == '.')
            break;
        push_char(&int_str, c);
        ++exp;
    }

    for(++p; p < slen; ++p){
        char c = char_at(*str, p);
        if(c == 'E')
            break;
        push_char(&int_str, c);
    }

    sign = ((char_at(*str, ++p) == '-') ? -1 : 1);
    for(++p; p < slen; ++p){
        push_char(&exp_str, char_at(*str, p));
    }

    int len = strlen(as_cstr(int_str));
    for(p = 0; p < len; ++p, --exp)
        if(char_at(int_str, p) != '0')
            break;

    for(int i = 0; i < res.place && i + p < len; ++i){
        res.digits[i] = char_at(int_str, i + p) - '0';
    }

    if(strlen(as_cstr(exp_str)) > 8){
        if(sign == -1)
            move_num(&res, create_num());
        else
            move_num(&res, create_num_of_flag(NUM_PINF));
    }else{
        res.exp = exp + sign * atoi(as_cstr(exp_str));
    }

    num_check(&res);
    destroy_vec(&int_str);
    destroy_vec(&exp_str);
    return res;
}

String num_to_str(NumberPtr num){
    num_adjust(num);
    String str;
    Number tmp;
    if(get_fixed_flag() && num->exp < 5000 && num->exp > -5000){
        tmp = num_round(num, get_precision());
        str = num_to_str_fixed(&tmp);
    }else{
        copy_num(&tmp, num ,sizeof(Number));
        tmp.exp = 0;
        move_num(&tmp, num_round(&tmp, get_precision()));
        tmp.exp += num->exp;
        str = num_to_str_sc(&tmp);
    }
    destroy_num(&tmp);
    return str;
}

int num_cmp_u(NumberPtr num1, NumberPtr num2){
    if(num1->exp != num2->exp)
        return num1->exp - num2->exp;
    for(int i = 0; i < num1->place; ++i)
        if(num1->digits[i] != num2->digits[i])
            return num1->digits[i] - num2->digits[i];
    return 0;
}

int num_cmp(NumberPtr num1, NumberPtr num2){
    if(num_invalid(num1) || num_invalid(num2))
        return num1->sign - num2->sign;
    if(num1->sign == num2->sign)
        return num1->sign * num_cmp_u(num1, num2);
    else return num1->sign - num2->sign;
}

bool num_is_zero(NumberPtr num){
    if(num_invalid(num))
        return false;
    return num->digits[0] == 0;
}

Number num_add_u(NumberPtr num1, NumberPtr num2){
    Number res = create_num();

    if(num1->exp < num2->exp)
        swap_numptr(&num1, &num2);
    res.exp = num1->exp;

    int carry = 0, delta = num1->exp - num2->exp, place = num1->place;
    //rounding correction
    if(delta > 0 && delta <= place)
        carry = num2->digits[place - delta] < 5 ? 0 : 1;
    for(int i = place - 1; i >= 0; --i){
        carry += num1->digits[i];
        if(i >= delta)
            carry += num2->digits[i - delta];
        res.digits[i] = carry % 10;
        carry /= 10;
    }
    if(carry){
        num_rshift(&res, 1);
        res.digits[0] = carry;
        res.exp++;
    }
    return res;
}

Number num_sub_u(NumberPtr num1, NumberPtr num2){
    Number res = create_num();
    int cmp = num_cmp_u(num1, num2);
    if(cmp == 0)
        return res;
    if(cmp < 0){
        swap_numptr(&num1, &num2);
        res.sign = -1;
    }
    res.exp = num1->exp;

    int borrow = 0, delta = num1->exp - num2->exp, place = num1->place;
    //rounding correction
    if(delta > 0 && delta <= place)
        borrow = num2->digits[place - delta] < 5 ? 0 : 1;

    for(int i = place - 1; i >= 0; --i){
        res.digits[i] = num1->digits[i] - borrow;
        if(i >= delta)
            res.digits[i] -= num2->digits[i - delta];
        if(res.digits[i] < 0){
            res.digits[i] += 10;
            borrow = 1;
        }else{
            borrow = 0;
        }
    }
    num_rm_leading_zeros(&res);
    return res;
}

Number num_add(NumberPtr num1, NumberPtr num2){
    Number res;
    if(num_is_nan(num1) || num_is_nan(num2))
        return create_num_of_flag(NUM_NAN);
    if(num_is_inf(num1) || num_is_inf(num2)){
        if(num1->sign == num2->sign)
            return create_num_of_flag(num1->sign);
        else return create_num_of_flag(NUM_NAN);
    }
    num_adjust(num1);
    num_adjust(num2);
    if(num1->sign == num2->sign)
        res = num_add_u(num1, num2);
    else res = num_sub_u(num1, num2);
    res.sign *= num1->sign;
    num_check(&res);
    return res;
}

Number num_addi(NumberPtr num1, int num2){
    Number res = create_num_i(num2);
    move_num(&res, num_add(num1, &res));
    return res;
}

Number num_sub(NumberPtr num1, NumberPtr num2){
    Number res;
    if(num_is_nan(num1) || num_is_nan(num2))
        return create_num_of_flag(NUM_NAN);
    if(num_is_inf(num1) || num_is_inf(num2)){
        if(num1->sign != num2->sign)
            return create_num_of_flag(num1->sign);
        else return create_num_of_flag(NUM_NAN);
    }
    num_adjust(num1);
    num_adjust(num2);
    if(num1->sign == num2->sign)
        res = num_sub_u(num1, num2);
    else res = num_add_u(num1, num2);
    res.sign *= num1->sign;
    num_check(&res);
    return res;
}

Number num_subi(NumberPtr num1, int num2){
    Number res = create_num_i(num2);
    move_num(&res, num_sub(num1, &res));
    return res;
}

//num should be valid.
int num_cmp_zero(NumberPtr num){
    if(num_is_zero(num) || num->exp < -get_eps())
        return 0;
    return num->sign;
}

//num should not be NaN
int num_cmp_eps(NumberPtr num1, NumberPtr num2){
    if(num_is_inf(num1) || num_is_inf(num2))
        return num1->sign - num2->sign;

    Number delta = num_sub(num1, num2);
    int ret = num_cmp_zero(&delta);
    destroy_num(&delta);
    return ret;
}

bool num_lt(NumberPtr num1, NumberPtr num2){
    if(num_is_nan(num1) || num_is_nan(num2))
        return false;
    return num_cmp_eps(num1, num2) < 0;
}

bool num_gt(NumberPtr num1, NumberPtr num2){
    if(num_is_nan(num1) || num_is_nan(num2))
        return false;
    return num_cmp_eps(num1, num2) > 0;
}

bool num_le(NumberPtr num1, NumberPtr num2){
    if(num_is_nan(num1) || num_is_nan(num2))
        return false;
    return num_cmp_eps(num1, num2) <= 0;
}

bool num_ge(NumberPtr num1, NumberPtr num2){
    if(num_is_nan(num1) || num_is_nan(num2))
        return false;
    return num_cmp_eps(num1, num2) >= 0;
}

bool num_eq(NumberPtr num1, NumberPtr num2){
    if(num_is_nan(num1) || num_is_nan(num2))
        return false;
    return num_cmp_eps(num1, num2) == 0;
}

bool num_neq(NumberPtr num1, NumberPtr num2){
    if(num_is_nan(num1) || num_is_nan(num2))
        return false;
    return num_cmp_eps(num1, num2) != 0;
}

Number num_mul(NumberPtr num1, NumberPtr num2){
    Number res = create_num();

    if(num_is_nan(num1) || num_is_nan(num2))
        return create_num_of_flag(NUM_NAN);

    if(num_is_inf(num1) || num_is_inf(num2)){
        if(num_is_zero(num1) || num_is_zero(num2))
            return create_num_of_flag(NUM_NAN);
        if(num1->sign != num2->sign)
            return create_num_of_flag(NUM_NINF);
        else return create_num_of_flag(NUM_PINF);
    }

    num_adjust(num1);
    num_adjust(num2);

    int place = get_dec_place();
    res.sign = num1->sign * num2->sign;
    res.exp += num1->exp + num2->exp;

    int *digits = calloc(place * 2, sizeof(int));

    Vector idx = create_vec_default(sizeof(int));
    for(int j = 0; j < place; ++j)
        if(num2->digits[j] != 0)
            push_back(&idx, &j);

    
    for(int i = 0; i < place; i++)
        if(num1->digits[i] != 0)
            FOR_EACH(int, j, idx)
                digits[i + *j] += num1->digits[i] * num2->digits[*j];
            END_FOR();

    int carry = 0;
    for(int i = place * 2 - 1; i > place; i--)
        carry = (carry + digits[i]) / 10;
    
    //rounding correction
    carry += digits[place];
    carry = (carry / 10) + (carry % 10 >= 5);
    
    for(int i = place - 1; i >= 0; i--){
        carry +=  digits[i];
        res.digits[i] = carry % 10;
        carry /= 10;
    }

    while(carry > 0){
        num_rshift(&res, 1);
        res.digits[0] = carry % 10;
        res.exp++;
        carry /= 10;
    }

    destroy_vec(&idx);
    free(digits);
    num_check(&res);
    return res;
}

Number num_div(NumberPtr num1, NumberPtr num2){
    Number res;

    if(num_is_nan(num1) || num_is_nan(num2))
        return create_num_of_flag(NUM_NAN);
    
    if(num_is_inf(num1) && num_is_inf(num2))
        return create_num_of_flag(NUM_NAN);
    
    if(num_is_inf(num1))
        return create_num_of_flag(num2->sign * num1->sign);
    
    if(num_is_inf(num2))
        return create_num();
    
    if(num_is_zero(num2)){
        if(num_is_zero(num1))
            return create_num_of_flag(NUM_NAN);
        return create_num_of_flag(NUM_PINF * num1->sign);
    }

    num_adjust(num1);
    num_adjust(num2);

    int place = num1->place;
    
    char *tmp = calloc(place * 2 + 1, sizeof(char));
    char *digits = calloc(place + 1 , sizeof(char));
    memcpy(tmp, num1->digits, place * sizeof(char));

    res = create_num();

    for(int i = 0; i <= place; i++){
        while(true){
            bool flag = true;
            for(int j = 0; j < place; j++){
                if(tmp[i + j] != num2->digits[j]){
                    flag = tmp[i + j] > num2->digits[j];
                    break;
                }
            }
            if(!flag)
                break;
            digits[i]++;
            for(int j = place - 1; j >= 0; --j){
                tmp[i + j] -= num2->digits[j];
                if(tmp[i + j] < 0){
                    tmp[i + j] += 10;
                    tmp[i + j - 1]--;
                }
            }
        }
        tmp[i + 1] += tmp[i] * 10;
    }

    if(digits[0] == 0){
        res.exp--;
        memcpy(res.digits, digits + 1, place * sizeof(char));
    }else{
        memcpy(res.digits, digits, place * sizeof(char));
    }

    res.sign = num1->sign * num2->sign;
    res.exp += num1->exp - num2->exp;
    num_check(&res);
    free(digits);
    free(tmp);
    return res;
}

Number num_floor_u(NumberPtr num){
    Number res;
    if(num_invalid(num)){
        copy_num(&res, num, sizeof(Number));
        return res;
    }
    if(num->exp < 0)
        return create_num();

    copy_num(&res, num, sizeof(Number));

    if(num->exp >= num->place - 1)
        return res;
    memset(res.digits + num->exp + 1, 0, (num->place - num->exp - 1) * sizeof(char));
    return res;
}

Number num_ceil_u(NumberPtr num){
    Number res;
    if(num_invalid(num)){
        copy_num(&res, num, sizeof(Number));
        return res;
    }
    if(num->exp < 0)
        create_num_i(1);

    copy_num(&res, num, sizeof(Number));

    if(num->exp >= num->place - 1)
        return res;
    bool flag = false;
    for(int i = num->exp + 1; i < num->place; i++){
        if(res.digits[i] != 0)
            flag = true;
        res.digits[i] = 0;
    }
    if(flag){
        Number one = create_num_i(1);
        move_num(&res, num_add_u(&res, &one));
        res.sign = num->sign;
        destroy_num(&one);
    }
    return res;
}

Number num_floor(NumberPtr num){
    num_adjust(num);
    if(num->sign < 0)
        return num_ceil_u(num);
    else 
        return num_floor_u(num);
}

Number num_ceil(NumberPtr num){
    num_adjust(num);
    if(num->sign < 0)
        return num_floor_u(num);
    else 
        return num_ceil_u(num);
}

//convert argument pack to a single integer.
int split_int(Vector vec, char* err){
    if(vec.size != 1){
        strcpy(err, "expect exactly 1 argument:");
        return 0;
    }
    ValuePtr val = get_addr(vec, 0);
    if(val->type != VAR_NUM){
        strcpy(err, "expect number argument:");
        return 0;
    }
    NumberPtr num = val->value;
    if(num_invalid(num)){
        strcpy(err, "invalid number argument:");
        return 0;
    }

    if(num->exp < 0)
        return 0;
    
    if(num->exp > 8){
        strcpy(err, "value out of bound:");
        return 0;
    }

    int res = 0;
    for(int i = 0; i <= num->exp; i++)
        res = res * 10 + num->digits[i];

    return res * num->sign;
}

Number num_round(NumberPtr num, int prec){
    Number num_u, half = create_num_i(5);
    copy_num(&num_u, num, sizeof(Number));
    Number res = create_num_i(1);
    res.exp = prec;
    half.exp = -1;
    num_u.sign = 1;
    move_num(&res, num_mul(&res, &num_u));
    move_num(&res, num_add(&res, &half));
    move_num(&res, num_floor(&res));
    res.exp -= prec;
    res.sign = num->sign;
    destroy_num(&num_u);
    destroy_num(&half);
    return res;
}