#include <stdio.h>
#include <libc.h>
#include "include/trees.h"

#define ROOT_CHILDS_SIZE (3)
#define CHILDS_SIZE (9)
void fill_child(child_t *child, child_t *parent);
static child_t root_childs[ROOT_CHILDS_SIZE] = {
        {.mac = {1, 1, 1, 11, 12, 1}},
        {.mac = {1, 1, 1, 11, 12, 2}},
        {.mac = {1, 1, 1, 11, 12, 3}}
};
static child_t childs[CHILDS_SIZE] = {
        {.mac = {1, 1, 1, 11, 12, 26}},
        {.mac = {1, 1, 1, 11, 12, 27}},
        {.mac = {1, 1, 1, 11, 12, 28}},
        {.mac = {1, 1, 1, 11, 12, 42}},
        {.mac = {1, 1, 1, 11, 12, 43}},
        {.mac = {1, 1, 1, 11, 12, 44}},
        {.mac = {1, 1, 1, 11, 12, 58}},
        {.mac = {1, 1, 1, 11, 12, 59}},
        {.mac = {1, 1, 1, 11, 12, 60}}
};

// called from mesh callback MESH_STARTED
void mesh_started() {
}

// only root
void mesh_child_connected(){
    child_t root_child;
    for(int i = 0; i < ROOT_CHILDS_SIZE; i++){
        memcpy(root_child.mac, root_childs[i].mac, sizeof(uint8_t) * 6);
        child_connected(&root_child, true);
    }
}
// from all other nodes
void mesh_rx(){
    int n = 6;
    fill_child(&childs[n],&root_childs[0]);
    child_connected(&childs[n], false);
    n = 3;
    fill_child(&childs[n],&root_childs[1]);
    child_connected(&childs[n], false);
    n = 0;
    fill_child(&childs[n],&childs[6]);
    child_connected(&childs[n], false);
    n = 1;
    fill_child(&childs[n],&childs[0]);
    child_connected(&childs[n], false);
}

void fill_child(child_t *child, child_t *parent) {
    memcpy(child->parent_mac, parent->mac, mac_bytes);
    child->level = parent->level + 1;
}

int main() {
    mesh_started();
    mesh_child_connected();
    mesh_rx();
    print_all_nodes();
    return 0;
}

