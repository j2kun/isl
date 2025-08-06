/*
 * Manual HS AST Debug Tool - Constructs hs.in data manually using ISL API
 * Shows in-memory AST structure for the Halevi-Shoup transformation
 */

#include <stdio.h>
#include <stdlib.h>
#include <isl/ast.h>
#include <isl/ast_build.h>
#include <isl/ctx.h>
#include <isl/constraint.h>
#include <isl/local_space.h>
#include <isl/map.h>
#include <isl/set.h>
#include <isl/space.h>
#include <isl/stream.h>
#include <isl/union_map.h>
#include <isl/union_set.h>
#include <isl/val.h>

/* Construct the schedule map manually using ISL API */
static __isl_give isl_union_map *construct_hs_schedule(isl_ctx *ctx)
{
  isl_union_map *umap = isl_union_map_read_from_str(ctx,
    "{ S[row,col,ct,slot] -> [ct,slot] : "
    "0 <= row,ct < 4 and 0 <= col < 8 and 0 <= slot < 32 and "
    "((-row + slot) % 4) = 0 and (-col + ct + slot) % 8 = 0 }");

	return union_map;
}

/* Construct AST from manually created schedule */
static __isl_give isl_ast_node *construct_hs_ast(isl_ctx *ctx)
{
	isl_union_map *schedule;
	isl_set *context;
	isl_union_map *options;
	isl_ast_build *build;
	isl_ast_node *tree;

	/* Create the schedule map */
	schedule = construct_hs_schedule(ctx);

	/* Create empty context (universe) */
	context = isl_set_universe(isl_space_params_alloc(ctx, 0));

	/* Create empty options map */
	options = isl_union_map_empty(isl_space_params_alloc(ctx, 0));

	/* Build the AST */
	build = isl_ast_build_from_context(context);
	build = isl_ast_build_set_options(build, options);
	tree = isl_ast_build_node_from_schedule_map(build, schedule);
	isl_ast_build_free(build);

	return tree;
}

int main(void)
{
	isl_ctx *ctx;
	isl_ast_node *tree;

	ctx = isl_ctx_alloc();

	tree = construct_hs_ast(ctx);

	if (tree) {
		printf("=== Manual HS AST Debug Information ===\n");
		printf("AST structure dump:\n");
		isl_ast_node_dump(tree);
		printf("\n=== End Manual HS AST Debug Information ===\n");
	} else {
		fprintf(stderr, "Failed to construct AST\n");
	}

	isl_ast_node_free(tree);
	isl_ctx_free(ctx);
	return tree ? EXIT_SUCCESS : EXIT_FAILURE;
}
