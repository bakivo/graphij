//
// Created by Vladimir Bakaev on 28/07/21.
//

#ifndef GRAPHIJ_TREES_H
#define GRAPHIJ_TREES_H
#include <libc.h>
#include <stdbool.h>

#define MAX_NODES 20

typedef struct {
    uint8_t parent_mac[6];
    uint8_t mac[6];
    uint8_t level;
} child_t;

void tree_node_lookup(uint8_t *mac);
void tree_print_levels();
void tree_set_root(uint8_t *root_mac, uint8_t level);
void tree_child_connected(const child_t *child, bool is_connected_to_root);
void tree_child_lost(child_t *lost_child);

#endif //GRAPHIJ_TREES_H
