#pragma once

#include <iostream>
#include <vector>
#include <handlegraph/types.hpp>
#include <handlegraph/util.hpp>
#include <handlegraph/path_handle_graph.hpp>
#include "ips4o.hpp"
#include "BooPHF.h"

namespace odgi {

namespace algorithms {

using namespace handlegraph;

struct step_handle_hasher_t {
	// the class should have operator () with this signature :
	// BBhash will use the 'seed' paramater to generate two different hash values form this key.
    //then it will generate internally a sequence of hash values using xorshifts, using these two first hash values as starting point.
	uint64_t operator() (step_handle_t step, uint64_t seed=0) const {
        uint64_t k1 = as_integers(step)[0];
		k1 ^= k1 >> 33;
		k1 *= 0xff51afd7ed558ccd;
		k1 ^= k1 >> 33;
		k1 *= 0xc4ceb9fe1a85ec53;
		k1 ^= k1 >> 33;
        uint64_t k2 = as_integers(step)[1];
		k2 ^= k2 >> 33;
		k2 *= 0xff51afd7ed558ccd;
		k2 ^= k2 >> 33;
		k2 *= 0xc4ceb9fe1a85ec53;
		k2 ^= k2 >> 33;
        // Reciprocal of the golden ratio helps spread entropy and handles duplicates.
        // See Mike Seymour in magic-numbers-in-boosthash-combine: http://stackoverflow.com/questions/4948780
        k1 ^= k2 + 0x9e3779b9 + (k1<<6) + (k1>>2);

		k1 ^= seed;
		return k1;
	}
};

typedef boomphf::mphf<step_handle_t, step_handle_hasher_t> boophf_step_t;
typedef boomphf::mphf<uint64_t, boomphf::SingleHashFunctor<uint64_t>> boophf_uint64_t;

struct step_index_t {
    step_index_t(const PathHandleGraph& graph,
                 const std::vector<path_handle_t>& paths,
                 const uint64_t& nthreads,
                 const bool progress);
    ~step_index_t(void);
    const uint64_t& get_position(const step_handle_t& step) const;
    // map from step to position in its path
    boophf_step_t* step_mphf = nullptr;
    std::vector<uint64_t> pos;
};

// index of a single path's steps designed for efficient iteration
// over steps on a single handle
// in practice
struct path_step_index_t {
    path_step_index_t(const PathHandleGraph& graph,
                      const path_handle_t& paths,
                      const uint64_t& nthreads);
    ~path_step_index_t(void);
    // map from node id in the path to an index in node_offsets
    boophf_uint64_t* node_mphf = nullptr;
    // map to the beginning of a range in node_steps
    std::vector<uint64_t> node_offset;
    // record the steps in positional order by node (index given in node_offset)
    std::vector<step_handle_t> node_steps;
    // map from step to an index in step_offset
    boophf_step_t* step_mphf = nullptr;
    // index in handle_steps for the given step
    std::vector<uint64_t> step_offset;
    uint64_t node_count = 0;
    uint64_t step_count = 0;
    // get the idx of a node
    uint64_t get_node_idx(const nid_t& id) const;
    // get the idx of a step
    uint64_t get_step_idx(const step_handle_t& step) const;
    // compute how many steps we have on the given node
    uint64_t n_steps_on_node(const nid_t& id) const;
    // these functions require, but do not check, that our step is in the indexed path
    // next step on node (sorted by position in path), (false, _) if there is no next step
    std::pair<bool, step_handle_t> get_next_step_on_node(const nid_t& id, const step_handle_t& step) const;
    // prev step on node (sorted by position in path), (false, _) if there is no next step
    std::pair<bool, step_handle_t> get_prev_step_on_node(const nid_t& id, const step_handle_t& step) const;
};

}

}
