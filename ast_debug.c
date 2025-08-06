/*
 * AST Debug Tool - Shows in-memory AST structure instead of generating C code
 * Based on codegen.c but modified to output debug information
 */

#include <isl/ast.h>
#include <isl/ast_build.h>
#include <isl/options.h>
#include <isl/schedule_node.h>
#include <isl/set.h>
#include <isl/space.h>
#include <isl/stream.h>
#include <isl/union_map.h>
#include <isl/union_set.h>
#include <stdio.h>
#include <stdlib.h>

/* Set default build options.
 */
static __isl_give isl_ast_build *set_options(__isl_take isl_ast_build *build,
                                             __isl_take isl_union_map *opt) {
  build = isl_ast_build_set_options(build, opt);
  return build;
}

/* Construct an AST in case the schedule is specified by a union map.
 */
static __isl_give isl_ast_node *
construct_ast_from_union_map(__isl_take isl_union_map *schedule,
                             __isl_keep isl_stream *s) {
  isl_set *context;
  isl_union_map *options_map;
  isl_ast_build *build;
  isl_ast_node *tree;

  context = isl_stream_read_set(s);
  options_map = isl_stream_read_union_map(s);

  build = isl_ast_build_from_context(context);
  build = set_options(build, options_map);
  tree = isl_ast_build_node_from_schedule_map(build, schedule);
  isl_ast_build_free(build);

  return tree;
}

/* Read an object from a file and construct AST for debug output
 */
int main(int argc, char **argv) {
  isl_ctx *ctx;
  isl_stream *s;
  isl_ast_node *tree = NULL;
  struct isl_obj obj;
  int r = EXIT_SUCCESS;
  FILE *input_file;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
    return EXIT_FAILURE;
  }

  input_file = fopen(argv[1], "r");
  if (!input_file) {
    fprintf(stderr, "Error: Cannot open file %s\n", argv[1]);
    return EXIT_FAILURE;
  }

  ctx = isl_ctx_alloc();

  s = isl_stream_new_file(ctx, input_file);
  obj = isl_stream_read_obj(s);
  if (obj.v == NULL) {
    r = EXIT_FAILURE;
  } else if (obj.type == isl_obj_map) {
    isl_union_map *umap;

    umap = isl_union_map_from_map(obj.v);
    tree = construct_ast_from_union_map(umap, s);
  } else if (obj.type == isl_obj_union_map) {
    tree = construct_ast_from_union_map(obj.v, s);
  } else {
    obj.type->free(obj.v);
    isl_die(ctx, isl_error_invalid, "unknown input", r = EXIT_FAILURE);
  }
  isl_stream_free(s);
  fclose(input_file);

  if (tree) {
    printf("=== AST Debug Information ===\n");
    printf("AST structure dump:\n");
    isl_ast_node_dump(tree);
    printf("\n=== End AST Debug Information ===\n");
  }

  isl_ast_node_free(tree);
  isl_ctx_free(ctx);
  return r;
}
