/**
 * @file atomspace.cpp
 * @brief AtomSpace - Hypergraph knowledge representation
 * 
 * Implements OpenCog AtomSpace as GGML tensor hypergraph.
 */

#include "aichat/cognitive.h"
#include "aichat/kernel.h"
#include <ggml.h>
#include <cstring>
#include <cstdlib>
#include <map>
#include <string>
#include <vector>

#define MAX_ATOMS 8192

/* Atom structure */
struct atom_t {
    atom_type_t type;
    char* name;
    std::vector<atom_handle_t> outgoing;  /* For links */
    struct ggml_tensor* tensor;
    uint64_t handle;
    bool active;
};

/* AtomSpace state */
static struct {
    struct ggml_context* ctx;
    atom_t atoms[MAX_ATOMS];
    std::map<std::string, atom_handle_t> name_index;
    size_t atom_count;
    uint64_t next_handle;
    bool initialized;
} atomspace = {nullptr, {}, {}, 0, 1, false};

/**
 * Initialize AtomSpace
 */
extern "C" int atomspace_init(struct ggml_context* ctx) {
    if (atomspace.initialized) {
        return 0;
    }
    
    if (!ctx) {
        return -1;
    }
    
    atomspace.ctx = ctx;
    atomspace.atom_count = 0;
    atomspace.next_handle = 1;
    atomspace.initialized = true;
    
    return 0;
}

/**
 * Allocate a new atom
 */
extern "C" atom_handle_t cog_atom_alloc(atom_type_t type, const char* name) {
    if (!atomspace.initialized) {
        return 0;
    }
    
    if (atomspace.atom_count >= MAX_ATOMS) {
        return 0;
    }
    
    /* Check if named atom already exists */
    if (name) {
        auto it = atomspace.name_index.find(name);
        if (it != atomspace.name_index.end()) {
            return it->second;
        }
    }
    
    /* Find free slot */
    size_t idx = 0;
    for (idx = 0; idx < MAX_ATOMS; idx++) {
        if (!atomspace.atoms[idx].active) {
            break;
        }
    }
    
    if (idx >= MAX_ATOMS) {
        return 0;
    }
    
    /* Create tensor for atom (embed as 512-dim vector) */
    struct ggml_tensor* t = ggml_new_tensor_1d(atomspace.ctx, GGML_TYPE_F32, 512);
    if (!t) {
        return 0;
    }
    
    /* Initialize atom */
    atom_t* atom = &atomspace.atoms[idx];
    atom->type = type;
    atom->name = name ? strdup(name) : nullptr;
    atom->outgoing.clear();
    atom->tensor = t;
    atom->handle = atomspace.next_handle++;
    atom->active = true;
    
    atomspace.atom_count++;
    
    /* Add to name index */
    if (name) {
        atomspace.name_index[name] = atom->handle;
    }
    
    return atom->handle;
}

/**
 * Create link between atoms
 */
extern "C" atom_handle_t cog_link_create(atom_type_t type, atom_handle_t* atoms, size_t n_atoms) {
    if (!atomspace.initialized) {
        return 0;
    }
    
    /* Create link atom */
    atom_handle_t link = cog_atom_alloc(type, nullptr);
    if (!link) {
        return 0;
    }
    
    /* Find link atom */
    atom_t* link_atom = nullptr;
    for (size_t i = 0; i < MAX_ATOMS; i++) {
        if (atomspace.atoms[i].active && atomspace.atoms[i].handle == link) {
            link_atom = &atomspace.atoms[i];
            break;
        }
    }
    
    if (!link_atom) {
        return 0;
    }
    
    /* Add outgoing atoms */
    for (size_t i = 0; i < n_atoms; i++) {
        link_atom->outgoing.push_back(atoms[i]);
        
        /* Create hypergraph edge */
        atom_t* target = nullptr;
        for (size_t j = 0; j < MAX_ATOMS; j++) {
            if (atomspace.atoms[j].active && atomspace.atoms[j].handle == atoms[i]) {
                target = &atomspace.atoms[j];
                break;
            }
        }
        
        if (target) {
            hgfs_edge(link_atom->tensor->data, target->tensor->data, 1.0f);
        }
    }
    
    return link;
}
