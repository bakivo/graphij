//
// Created by Vladimir Bakaev on 28/07/21.
//

#ifndef GRAPHIJ_TREES_H
#define GRAPHIJ_TREES_H
#include <libc.h>
#include <stdbool.h>

#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MAX_NODES 20
static uint8_t mac_bytes = sizeof(uint8_t) * 6;

typedef int16_t my_err;
typedef struct {
    uint8_t parent_mac[6];
    uint8_t mac[6];
    uint8_t level;
} child_t;
typedef struct node {
    struct node *first_child;
    struct node *next_sibling;
    uint8_t mac[6];
    uint8_t level;
} node_t;
void print_all_nodes();
void child_connected(const child_t *child, bool is_connected_to_root);
void child_lost(child_t *lost_child);

#endif //GRAPHIJ_TREES_H
