#include "execution/merge_join.hpp"
#include "parser/expression/comparison_expression.hpp"
#include "common/operator/comparison_operators.hpp"
#include "common/vector_operations/vector_operations.hpp"

using namespace duckdb;
using namespace std;


template <class T>
size_t MergeJoinMark::Equality::Operation(ScalarMergeInfo &l, ChunkMergeInfo &r) {
	throw NotImplementedException("Merge Join with Equality not implemented");
}

template<class T, class OP>
static size_t merge_join_mark_gt(ScalarMergeInfo &l, ChunkMergeInfo &r) {
	assert(l.sel_vector);
	auto ldata = (T *)l.v.data;
	l.pos = l.count;
	for(size_t chunk_idx = 0; chunk_idx < r.order_info.size(); chunk_idx++) {
		// we only care about the SMALLEST value in each of the RHS
		// because we want to figure out if they are greater than [or equal] to ANY value
		// get the smallest value from the RHS
		auto &rorder = r.order_info[chunk_idx];
		auto rdata = (T*) r.data_chunks.chunks[chunk_idx]->data[0].data;
		auto min_r_value = rdata[rorder.order[0]];
		// now we start from the current lpos value and check if we found a new value that is [>= OR >] the min RHS value
		while(true) {
			if (OP::Operation(ldata[l.sel_vector[l.pos - 1]], min_r_value)) {
				// found a match for lpos, set it in the found_match vector
				r.found_match[l.sel_vector[l.pos - 1]] = true;
				l.pos--;
				if (l.pos == 0) {
					// early out: we exhausted the entire LHS and they all match
					return 0;
				}
			} else {
				// we found no match: any subsequent value from the LHS we scan now will be smaller and thus also not match
				// move to the next RHS chunk
				break;
			}
		}
	}
	return 0;
}
template <class T>
size_t MergeJoinMark::GreaterThan::Operation(ScalarMergeInfo &l, ChunkMergeInfo &r) {
	return merge_join_mark_gt<T, operators::GreaterThan>(l, r);
}

template <class T>
size_t MergeJoinMark::GreaterThanEquals::Operation(ScalarMergeInfo &l, ChunkMergeInfo &r) {
	return merge_join_mark_gt<T, operators::GreaterThanEquals>(l, r);
}

template<class T, class OP>
static size_t merge_join_mark_lt(ScalarMergeInfo &l, ChunkMergeInfo &r) {
	assert(l.sel_vector);
	auto ldata = (T *)l.v.data;
	l.pos = 0;
	for(size_t chunk_idx = 0; chunk_idx < r.order_info.size(); chunk_idx++) {
		// we only care about the BIGGEST value in each of the RHS
		// because we want to figure out if they are less than [or equal] to ANY value
		// get the biggest value from the RHS
		auto &rorder = r.order_info[chunk_idx];
		auto rdata = (T*) r.data_chunks.chunks[chunk_idx]->data[0].data;
		auto max_r_value = rdata[rorder.order[rorder.count - 1]];
		// now we start from the current lpos value and check if we found a new value that is [<= OR <] the max RHS value
		while(true) {
			if (OP::Operation(ldata[l.sel_vector[l.pos]], max_r_value)) {
				// found a match for lpos, set it in the found_match vector
				r.found_match[l.sel_vector[l.pos]] = true;
				l.pos++;
				if (l.pos == l.count) {
					// early out: we exhausted the entire LHS and they all match
					return 0;
				}
			} else {
				// we found no match: any subsequent value from the LHS we scan now will be bigger and thus also not match
				// move to the next RHS chunk
				break;
			}
		}
	}
	return 0;
}

template <class T>
size_t MergeJoinMark::LessThan::Operation(ScalarMergeInfo &l, ChunkMergeInfo &r) {
	return merge_join_mark_lt<T, operators::LessThan>(l, r);
	throw NotImplementedException("Not implemented");
}

template <class T>
size_t MergeJoinMark::LessThanEquals::Operation(ScalarMergeInfo &l, ChunkMergeInfo &r) {
	return merge_join_mark_lt<T, operators::LessThanEquals>(l, r);
}

INSTANTIATE_MERGEJOIN_TEMPLATES(MergeJoinMark, Equality, ScalarMergeInfo, ChunkMergeInfo);
INSTANTIATE_MERGEJOIN_TEMPLATES(MergeJoinMark, LessThan, ScalarMergeInfo, ChunkMergeInfo);
INSTANTIATE_MERGEJOIN_TEMPLATES(MergeJoinMark, LessThanEquals, ScalarMergeInfo, ChunkMergeInfo);
INSTANTIATE_MERGEJOIN_TEMPLATES(MergeJoinMark, GreaterThan, ScalarMergeInfo, ChunkMergeInfo);
INSTANTIATE_MERGEJOIN_TEMPLATES(MergeJoinMark, GreaterThanEquals, ScalarMergeInfo, ChunkMergeInfo);