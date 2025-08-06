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
	isl_space *domain_space, *range_space, *map_space;
	isl_local_space *local_space;
	isl_constraint *c;
	isl_basic_map *basic_map;
	isl_map *map;
	isl_union_map *union_map;
	isl_val *val;

	/* Create domain space S[row,col,ct,slot] */
	domain_space = isl_space_set_alloc(ctx, 0, 4);
	domain_space = isl_space_set_tuple_name(domain_space, isl_dim_set, "S");

	/* Create range space [ct,slot] */
	range_space = isl_space_set_alloc(ctx, 0, 2);

	/* Create map space S[row,col,ct,slot] -> [ct,slot] */
	map_space = isl_space_map_from_domain_and_range(
		isl_space_copy(domain_space), range_space);

	/* Create local space for constraints */
	local_space = isl_local_space_from_space(isl_space_copy(map_space));

	/* Start with universe basic map */
	basic_map = isl_basic_map_universe(isl_space_copy(map_space));

	/* Add constraint: 0 <= row < 4 */
	/* 0 <= row  =>  row >= 0 */
	c = isl_constraint_alloc_inequality(isl_local_space_copy(local_space));
	c = isl_constraint_set_coefficient_si(c, isl_dim_in, 0, 1); /* row coeff = 1 */
	basic_map = isl_basic_map_add_constraint(basic_map, c);

	/* row < 4  =>  -row + 3 >= 0 */
	c = isl_constraint_alloc_inequality(isl_local_space_copy(local_space));
	c = isl_constraint_set_coefficient_si(c, isl_dim_in, 0, -1); /* row coeff = -1 */
	c = isl_constraint_set_constant_si(c, 3); /* constant = 3 */
	basic_map = isl_basic_map_add_constraint(basic_map, c);

	/* Add constraint: 0 <= col < 8 */
	/* 0 <= col  =>  col >= 0 */
	c = isl_constraint_alloc_inequality(isl_local_space_copy(local_space));
	c = isl_constraint_set_coefficient_si(c, isl_dim_in, 1, 1); /* col coeff = 1 */
	basic_map = isl_basic_map_add_constraint(basic_map, c);

	/* col < 8  =>  -col + 7 >= 0 */
	c = isl_constraint_alloc_inequality(isl_local_space_copy(local_space));
	c = isl_constraint_set_coefficient_si(c, isl_dim_in, 1, -1); /* col coeff = -1 */
	c = isl_constraint_set_constant_si(c, 7); /* constant = 7 */
	basic_map = isl_basic_map_add_constraint(basic_map, c);

	/* Add constraint: 0 <= ct < 4 */
	/* 0 <= ct  =>  ct >= 0 */
	c = isl_constraint_alloc_inequality(isl_local_space_copy(local_space));
	c = isl_constraint_set_coefficient_si(c, isl_dim_in, 2, 1); /* ct coeff = 1 */
	basic_map = isl_basic_map_add_constraint(basic_map, c);

	/* ct < 4  =>  -ct + 3 >= 0 */
	c = isl_constraint_alloc_inequality(isl_local_space_copy(local_space));
	c = isl_constraint_set_coefficient_si(c, isl_dim_in, 2, -1); /* ct coeff = -1 */
	c = isl_constraint_set_constant_si(c, 3); /* constant = 3 */
	basic_map = isl_basic_map_add_constraint(basic_map, c);

	/* Add constraint: 0 <= slot < 32 */
	/* 0 <= slot  =>  slot >= 0 */
	c = isl_constraint_alloc_inequality(isl_local_space_copy(local_space));
	c = isl_constraint_set_coefficient_si(c, isl_dim_in, 3, 1); /* slot coeff = 1 */
	basic_map = isl_basic_map_add_constraint(basic_map, c);

	/* slot < 32  =>  -slot + 31 >= 0 */
	c = isl_constraint_alloc_inequality(isl_local_space_copy(local_space));
	c = isl_constraint_set_coefficient_si(c, isl_dim_in, 3, -1); /* slot coeff = -1 */
	c = isl_constraint_set_constant_si(c, 31); /* constant = 31 */
	basic_map = isl_basic_map_add_constraint(basic_map, c);

	/* Add output constraints: ct -> ct, slot -> slot */
	/* ct_out = ct_in  =>  ct_out - ct_in = 0 */
	c = isl_constraint_alloc_equality(isl_local_space_copy(local_space));
	c = isl_constraint_set_coefficient_si(c, isl_dim_in, 2, -1); /* ct_in coeff = -1 */
	c = isl_constraint_set_coefficient_si(c, isl_dim_out, 0, 1); /* ct_out coeff = 1 */
	basic_map = isl_basic_map_add_constraint(basic_map, c);

	/* slot_out = slot_in  =>  slot_out - slot_in = 0 */
	c = isl_constraint_alloc_equality(isl_local_space_copy(local_space));
	c = isl_constraint_set_coefficient_si(c, isl_dim_in, 3, -1); /* slot_in coeff = -1 */
	c = isl_constraint_set_coefficient_si(c, isl_dim_out, 1, 1); /* slot_out coeff = 1 */
	basic_map = isl_basic_map_add_constraint(basic_map, c);

	/* Add modular constraints using ISL's mod operation support */
	/* (-row + slot) % 4 = 0  =>  -row + slot = 4*k for some integer k */
	/* This is implemented as: slot - row = 4*k, which ISL handles via */
	/* stride constraints or by creating multiple basic maps for k values */

	/* For now, let's use the constraint that slot - row is divisible by 4 */
	/* We can add this as: (slot - row) mod 4 = 0 */
	/* ISL represents this through stride information or explicit enumeration */

	/* (-col + ct + slot) % 8 = 0  =>  -col + ct + slot = 8*k */
	/* Similarly: ct + slot - col = 8*k */

	isl_space_free(domain_space);
	isl_space_free(map_space);
	isl_local_space_free(local_space);

	/* Convert basic_map to map */
	map = isl_map_from_basic_map(basic_map);

	/* For the modular constraints, we need to intersect with the appropriate sets */
	/* This is complex to do purely with API calls, so we'll use a hybrid approach */
	/* Create the modular constraint parts using string parsing for precision */
	const char *mod_constraints = "{ S[row,col,ct,slot] -> [ct,slot] : "
		"((-row + slot) % 4) = 0 and (-col + ct + slot) % 8 = 0 }";
	isl_union_map *mod_umap = isl_union_map_read_from_str(ctx, mod_constraints);
	isl_map *mod_map = isl_map_from_union_map(mod_umap);

	/* Intersect our manually created map with the modular constraints */
	map = isl_map_intersect(map, mod_map);

	/* Convert to union map */
	union_map = isl_union_map_from_map(map);

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
