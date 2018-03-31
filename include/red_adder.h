#ifndef _RED_ADDER_H
#define _RED_ADDER_H

#ifndef EXTRA_PARAMS
# error "EXTRA_PARAMS is not defined, e.g.: ', double amp_leech_scaled'"
#endif

#ifndef RED_INIT_EXPR
# error "RED_INIT_EXPR is not defined, e.g.: 'gem_init(ARG, 1, 0, 0)'"
#endif

#ifndef CFR_EXPR
# error "CFR_EXPR is not defined, e.g.: 'gem_cfr_power(ARG, amp_leech_scaled)'"
#endif

gem* gem_putred(gem* pool, int pool_length, gem** gem_array EXTRA_PARAMS)
{
	double best_pow = 0;
	gem* best_gem = NULL;
	gem* best_array = NULL;
	
	for (int i = 0; i < pool_length; ++i) {
		int depth = gem_getdepth(pool + i);
		gem* new_array = malloc(depth*sizeof(gem));
		RED_INIT_EXPR(new_array);
		
		gem** stack = malloc(depth*sizeof(gem*));
		stack[0] = pool + i;
		int stack_length = 1;
		
		while (1) { // loop over subgems
			
			gem* curr_place = new_array;
			gem* next_place = new_array + 1;
			
			// the stack contains only parents up to root, so that we can recombine them up
			while (stack[stack_length - 1]->father != NULL) {
				stack[stack_length] = stack[stack_length - 1]->father;
				stack_length++;
			}
			
			// the stack has a g1 and all its ancestors
			// combine them up from the bottom and build a candidate
			for (int level = stack_length - 1; level > 0; level--, curr_place++, next_place++) {
				if (stack[level] == stack[level - 1]->father)
					gem_combine(curr_place, stack[level - 1]->mother, next_place);
				else
					gem_combine(stack[level - 1]->father, curr_place, next_place);
			}
			
			double new_pow = CFR_EXPR(*curr_place);
			if (new_pow > best_pow) {
				best_pow = new_pow;
				best_gem = curr_place;
				
				free(best_array);
				best_array = new_array;
				new_array = malloc(depth*sizeof(gem));
				RED_INIT_EXPR(new_array);
			}
			
			pop:
			if (stack[stack_length - 1] == stack[stack_length - 2]->mother) {
				/* we've done the mother, pop recursively
				 * this check may capture the father at the first round, but
				 * if it does it means the father was equal to the mother,
				 * so we're actually skipping useless work
				 */
				stack_length--;
				if (stack_length == 1)
					break;
				else
					goto pop;
			}
			else {
				// we've done the father, but not the mother, push the mother
				stack[stack_length - 1] = stack[stack_length - 2]->mother;
			}
		}
		free(new_array);
		free(stack);
	}
	(*gem_array) = best_array;
	return best_gem;
}


#endif // _RED_ADDER_H
