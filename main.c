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
static uint8_t macs[10][6] = {
    {1, 1, 1, 11, 12, 0}, // root - 0
    {1, 1, 1, 11, 12, 1}, // 1
    {1, 1, 1, 11, 12, 2}, // 2
    {1, 1, 1, 11, 12, 3}, // 3
    {1, 1, 1, 11, 12, 4}, // ..
    {1, 1, 1, 11, 12, 5},
    {1, 1, 1, 11, 12, 6},
    {1, 1, 1, 11, 12, 7},
    {1, 1, 1, 11, 12, 8},
    {1, 1, 1, 11, 12, 9}, // 9
};
static uint8_t pairs[10][2] = {
        {1,0},
        {2,0},
        {3,0}
};
// called from mesh callback MESH_EVENT_ROOT_ADDRESS
void mesh_started() {
    tree_set_root(macs[0], 0);
}

// only called when event MESH_EVENT_CHILD_CONNECTED
void mesh_child_connected(){
    child_t child;
    for(int i = 1; i < 4; i++){
        memcpy(child.mac, macs[i], sizeof(uint8_t) * 6);
        tree_child_connected(&child, true);
    }
}
// from all other nodes
void mesh_rx(){
    for (int i = 4; i < 10; i++) {
        ;
    }
    int n = 6;
    fill_child(&childs[n],&root_childs[0]);
    tree_child_connected(&childs[n], false);
    n = 3;
    fill_child(&childs[n],&root_childs[1]);
    tree_child_connected(&childs[n], false);
    n = 0;
    fill_child(&childs[n],&childs[6]);
    tree_child_connected(&childs[n], false);
    n = 1;
    fill_child(&childs[n],&childs[0]);
    tree_child_connected(&childs[n], false);
}

void fill_child(child_t *child, child_t *parent) {
    memcpy(child->parent_mac, parent->mac, sizeof(uint8_t) * 6);
    child->level = parent->level + 1;
}

int main() {
    // set root
    mesh_started();
    // add childs to root
    mesh_child_connected();
    // add childs to mediate nodes
    mesh_rx();
    // print
    tree_print();
    return 0;
}

