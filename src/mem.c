#include <assert.h>
#include "../inc/ast.h"
#include "../inc/obj.h"
#include "../inc/eval.h"
#include "../inc/env.h"
#include "../inc/type.h"
#include "../inc/mem.h"
#include "../inc/heap.h"

#define HDR_ALLOC(t, y, c) \
  hdr = mem_alloc(sizeof(t)); \
  hdr->type = y; \
  hdr->children = c

void* mem_alloc(size_t size) {
  return ealloc(size);
}

void* mem_realloc(void *b, size_t size) {
  return erealloc(b, size);
}

void mem_free(void *b) {
  return efree(b);
}

void mem_init(unsigned char initval) {
  heap_init(initval);
}

gc_header_t *alloc_type(type_t type, flags_t flags) {
  assert(type > TYPE_ERR_DO_NOT_USE && type < TYPE_MAX);

  gc_header_t *hdr;

  switch(type) {
    // Primitive and simple types with no children.
    case TYPE_UNKNOWN:
    case TYPE_NOTHING:
    case TYPE_UNDEF:
    case TYPE_NIL:
    case TYPE_ERROR:
    case TYPE_INT:
    case TYPE_FLOAT:
    case TYPE_BYTE:
    case TYPE_BOOLEAN:
    case TYPE_BREAK:
    case TYPE_CONTINUE:
      HDR_ALLOC(obj_t, type, 0);
      break;

    // Fancy types have one pointer to the child structure.
    case TYPE_LIST:
    case TYPE_DICT:
    case TYPE_BYTEARRAY:
    case TYPE_STRING:
    case TYPE_FUNCTION:
    case TYPE_RANGE:
    case TYPE_ITERATOR:
      HDR_ALLOC(obj_t, type, 1);
      break;

    // Various child data structures.
    // TYPE_BYTEARRAY_DATA gets special handling in str.c.
    case TYPE_RANGE_DATA:        HDR_ALLOC(obj_range_t, type, 0);        break;
    case TYPE_VARIABLE_ARGS:     HDR_ALLOC(obj_varargs_t, type, 2);      break;
    case TYPE_LIST_DATA:         HDR_ALLOC(obj_list_t, type, 1);         break;
    case TYPE_LIST_ELEM_DATA:    HDR_ALLOC(obj_list_element_t, type, 2); break;
    case TYPE_DICT_DATA:         HDR_ALLOC(obj_dict_t, type, 0);         break;
    case TYPE_DICT_KV_DATA:      HDR_ALLOC(dict_node_t, type, 3);        break;
    case TYPE_FUNCTION_PTR_DATA: HDR_ALLOC(obj_func_def_t, type, 2);     break;
    case TYPE_RETURN_VAL:        HDR_ALLOC(obj_t, type, 1);              break;
    case TYPE_ITERATOR_DATA:     HDR_ALLOC(obj_iter_t, type, 2);         break;

    // Environment.
    case ENV_SYM:                HDR_ALLOC(env_sym_t, type, 2);          break;
    case ENV_SYM_ELEM:           HDR_ALLOC(env_sym_elem_t, type, 3);     break;

    // Eval Result
    case EVAL_RESULT:            HDR_ALLOC(eval_result_t, type, 1);      break;

    // AST.
    // Basic types and control words.
    case AST_EMPTY:
    case AST_INT:
    case AST_FLOAT:
    case AST_BOOLEAN:
    case AST_BYTE:
    case AST_BREAK:
    case AST_CONTINUE:
      HDR_ALLOC(ast_expr_t, type, 0);
      break;

    case AST_IDENT:
    case AST_STRING:
    case AST_FUNCTION_RETURN:
      HDR_ALLOC(ast_expr_t, type, 1);
      break;

    // Unary operators.
    case AST_DELETE:
    case AST_NEGATE:
    case AST_NOT:
    case AST_BITWISE_NOT:
      HDR_ALLOC(ast_expr_t, type, 1);
      break;

    // Unary operator data.
    case AST_UNARY_ARG:
      HDR_ALLOC(ast_unary_arg_t, type, 1);
      break;

    // Binop nodes.
    case AST_ADD:
    case AST_SUB:
    case AST_MUL:
    case AST_DIV:
    case AST_MOD:
    case AST_AND:
    case AST_OR:
    case AST_BITWISE_OR:
    case AST_BITWISE_XOR:
    case AST_BITWISE_AND:
    case AST_BITWISE_SHL:
    case AST_BITWISE_SHR:
    case AST_GT:
    case AST_GE:
    case AST_LT:
    case AST_LE:
    case AST_EQ:
    case AST_NE:
    case AST_IS:
    case AST_IN:
    case AST_SUBSCRIPT:
    case AST_MAPS_TO:
    case AST_ASSIGN:
      HDR_ALLOC(ast_expr_t, type, 1);
      break;

    // Binop args.
    case AST_BINOP_ARGS:
      HDR_ALLOC(ast_op_args_t, type, 2);
      break;

    case AST_RANGE:              HDR_ALLOC(ast_expr_t, type, 1);         break;
    case AST_RANGE_ARGS:         HDR_ALLOC(ast_range_args_t, type, 3);   break;
    case AST_CAST:               HDR_ALLOC(ast_expr_t, type, 1);         break;
    case AST_CAST_ARGS:          HDR_ALLOC(ast_cast_args_t, type, 2);    break;

    // Compound objects and expressions.
    case AST_BLOCK:              HDR_ALLOC(ast_expr_t, type, 1);         break;
    case AST_LIST:               HDR_ALLOC(ast_expr_t, type, 1);         break;
    case AST_LIST_ELEMS:         HDR_ALLOC(ast_list_t, type, 1);         break;
    case AST_DICT:               HDR_ALLOC(ast_expr_t, type, 1);         break;
    case AST_DICT_KVS:           HDR_ALLOC(ast_dict_t, type, 1);         break;
    case AST_DICT_KV:            HDR_ALLOC(ast_expr_kv_list_t, type, 3); break;
    case AST_BYTEARRAY_DECL:     HDR_ALLOC(ast_expr_t, type, 1);         break;
    case AST_BYTEARRAY_DECL_DATA:HDR_ALLOC(bytearray_t, type, 0);        break;
    case AST_APPLY:              HDR_ALLOC(ast_expr_t, type, 1);         break;
    case AST_APPLY_DATA:         HDR_ALLOC(ast_apply_t, type, 3);        break;
    case AST_EXPR_LIST:          HDR_ALLOC(ast_expr_list_t, type, 2);    break;
    case AST_METHOD_CALL:        HDR_ALLOC(ast_expr_t, type, 1);         break;
    case AST_METHOD_CALL_DATA:   HDR_ALLOC(ast_method_t, type, 2);       break;
    case AST_FUNCTION_DEF:       HDR_ALLOC(ast_expr_t, type, 1);         break;
    case AST_FUNCTION_DEF_DATA:  HDR_ALLOC(ast_func_def_t, type, 2);     break;
    case AST_FUNCTION_DEF_ARGS:  HDR_ALLOC(ast_fn_arg_decl_t, type, 2);  break;
    case AST_FUNCTION_CALL:      HDR_ALLOC(ast_expr_t, type, 1);         break;
    case AST_FUNCTION_CALL_DATA: HDR_ALLOC(ast_func_call_t, type, 2);    break;
    case AST_RESERVED_CALLABLE:  HDR_ALLOC(ast_expr_t, type, 1);         break;
    case AST_RESERVED_CALLABLE_DATA:
        HDR_ALLOC(ast_reserved_callable_t, type, 1);                     break;
    case AST_IF_THEN:            HDR_ALLOC(ast_expr_t, type, 1);         break;
    case AST_IF_THEN_DATA:       HDR_ALLOC(ast_if_then_args_t, type, 2); break;
    case AST_IF_THEN_ELSE:       HDR_ALLOC(ast_expr_t, type, 1);         break;
    case AST_IF_THEN_ELSE_DATA:
        HDR_ALLOC(ast_if_then_else_args_t, type, 3);                     break;
    case AST_DO_WHILE_LOOP:      HDR_ALLOC(ast_expr_t, type, 1);         break;
    case AST_DO_WHILE_LOOP_DATA: HDR_ALLOC(ast_do_while_loop_t, type, 2);break;
    case AST_WHILE_LOOP:         HDR_ALLOC(ast_expr_t, type, 1);         break;
    case AST_WHILE_LOOP_DATA:    HDR_ALLOC(ast_while_loop_t, type, 2);   break;
    case AST_FOR_LOOP:           HDR_ALLOC(ast_expr_t, type, 1);         break;
    case AST_FOR_LOOP_DATA:      HDR_ALLOC(ast_for_loop_t, type, 3);     break;

    default:
      printf("Unhandled GC type. Object will be untraceable: %s\n", type_names[type]);
      assert(0);
      break;
  }

  hdr->flags = flags;

  return hdr;
}

void assert_valid_typed_node(gc_header_t* node) {
  assert(node->type > TYPE_ERR_DO_NOT_USE && node->type < TYPE_MAX);
  assert(node->children >= 0 && node->children <= 4);

  // Verify child node pointers are sane.
  for (int i = 0; i < node->children; ++i) {
    size_t offset = sizeof(gc_header_t) + (i * sizeof(void*));
    void **child = (void*) node + offset;
    if (*child != NULL) assert_valid_data_ptr(*child);
  }
}

