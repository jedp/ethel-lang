#include <assert.h>
#include "../inc/type.h"
#include "../inc/mem.h"
#include "../inc/heap.h"

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

void mark_traceable(void *obj, type_t type, flags_t flags) {
  gc_header_t *hdr = (gc_header_t*) obj;
  assert(type > TYPE_ERR_DO_NOT_USE && type < TYPE_MAX);

  hdr->type = type;
  hdr->flags = flags;

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
      hdr->children = 0;
      break;

    // Fancy types have one pointer to the child structure.
    case TYPE_LIST:
    case TYPE_DICT:
    case TYPE_BYTEARRAY:
    case TYPE_STRING:
    case TYPE_FUNCTION:
    case TYPE_RANGE:
    case TYPE_ITERATOR:
      hdr->children = 1;
      break;

    // Various child data structures.
    case TYPE_BYTEARRAY_DATA: hdr->children = 0; break;
    case TYPE_RANGE_DATA: hdr->children = 0; break;
    case TYPE_VARIABLE_ARGS: hdr->children = 2; break;
    case TYPE_LIST_DATA: hdr->children = 1; break;
    case TYPE_LIST_ELEM_DATA: hdr->children = 2; break;
    case TYPE_DICT_DATA: hdr->children = 1; break;
    case TYPE_DICT_KV_DATA: hdr->children = 3; break;
    case TYPE_FUNCTION_PTR_DATA: hdr->children = 2; break;
    case TYPE_RETURN_VAL: hdr->children = 1; break;
    case TYPE_ITERATOR_DATA: hdr->children = 2; break;

    // Environment.
    case ENV_SYM: hdr->children = 4; break;

    // AST.
    // Basic types and control words.
    case AST_EMPTY:
    case AST_INT:
    case AST_FLOAT:
    case AST_BOOLEAN:
    case AST_BYTE:
    case AST_BREAK:
    case AST_CONTINUE:
      hdr->children = 0;
      break;

    case AST_IDENT: hdr->children = 1; break;
    case AST_STRING: hdr->children = 1; break;
    case AST_FUNCTION_RETURN: hdr->children = 1; break;

    // Unary operators.
    case AST_DELETE:
    case AST_NEGATE:
    case AST_NOT:
    case AST_BITWISE_NOT:
      hdr->children = 1;
      break;

    // Unary operator data.
    case AST_UNARY_ARG: hdr->children = 1; break;

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
      hdr->children = 1;
      break;

    // Binop args.
    case AST_BINOP_ARGS: hdr->children = 2; break;

    case AST_RANGE: hdr->children = 1; break;
    case AST_RANGE_ARGS: hdr->children = 3; break;
    case AST_CAST: hdr->children = 1; break;
    case AST_CAST_ARGS: hdr->children = 3; break;

    // Compound objects and expressions.
    case AST_BLOCK: hdr->children = 1; break;
    case AST_LIST: hdr->children = 1; break;
    case AST_LIST_ELEMS: hdr->children = 1; break;
    case AST_DICT: hdr->children = 1; break;
    case AST_DICT_KVS: hdr->children = 1; break;
    case AST_DICT_KV: hdr->children = 3; break;
    case AST_BYTEARRAY_DECL: hdr->children = 1; break;
    case AST_BYTEARRAY_DECL_DATA: hdr->children = 1; break;
    case AST_APPLY: hdr->children = 1; break;
    case AST_APPLY_DATA: hdr->children = 3; break;
    case AST_EXPR_LIST: hdr->children = 1; break;
    case AST_METHOD_CALL: hdr->children = 1; break;
    case AST_METHOD_CALL_DATA: hdr->children = 2; break;
    case AST_FUNCTION_DEF: hdr->children = 1; break;
    case AST_FUNCTION_DEF_DATA: hdr->children = 2; break;
    case AST_FUNCTION_CALL: hdr->children = 1; break;
    case AST_FUNCTION_CALL_DATA: hdr->children = 2; break;
    case AST_RESERVED_CALLABLE: hdr->children = 1; break;
    case AST_RESERVED_CALLABLE_DATA: hdr->children = 1; break;
    case AST_IF_THEN: hdr->children = 1; break;
    case AST_IF_THEN_DATA: hdr->children = 2; break;
    case AST_IF_THEN_ELSE: hdr->children = 1; break;
    case AST_IF_THEN_ELSE_DATA: hdr->children = 3; break;
    case AST_DO_WHILE_LOOP: hdr->children = 1; break;
    case AST_DO_WHILE_LOOP_DATA: hdr->children = 2; break;
    case AST_WHILE_LOOP: hdr->children = 1; break;
    case AST_WHILE_LOOP_DATA: hdr->children = 2; break;
    case AST_FOR_LOOP: hdr->children = 1; break;
    case AST_FOR_LOOP_DATA: hdr->children = 3; break;

    default:
      printf("Unhandled GC type. Object will be untraceable: %s\n", type_names[TYPEOF(obj)]);
      assert(0);
      break;
  }
}

