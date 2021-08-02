#include <stdio.h>
#include <libc.h>
#include "include/trees.h"
#include "include/tests.h"

void create_pair(uint8_t child_mac[6], uint8_t parent_mac[6], child_t *child);
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
#define PAIRS_NUM 10
static uint8_t pairs[PAIRS_NUM][2] = {
        {4,3},
        {5,3},
        {6,2},
        {7,4},
        {8,4}
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
    child_t new_child;
    uint8_t pairs_num = 0;
    for(int i = 0; i < PAIRS_NUM; i++) {
        if (pairs[i][0] == 0) break;
        pairs_num++;
    }
    //printf("pairs number = %d\n", pairs_num);
    for (int i = 0; i < pairs_num; i++) {
        uint8_t child_index = pairs[i][0];
        uint8_t parent_index = pairs[i][1];
        create_pair(macs[child_index], macs[parent_index], &new_child);
        tree_child_connected(&new_child, false);
        //printf("new\n");
    }
}
void create_pair(uint8_t child_mac[6], uint8_t parent_mac[6], child_t *child) {
    memcpy(child->parent_mac, parent_mac, sizeof(uint8_t) * 6);
    memcpy(child->mac, child_mac, sizeof(uint8_t) * 6);

}

static bool is_test = false;

void main_tests() {
    test2();
}
void main_functions(){
    mesh_started();
    mesh_child_connected();
    mesh_rx();
    //tree_print_levels();
    //tree_node_lookup(macs[3]);
    //printf("\n\n");
    //child_t child1;
    //memcpy(child1.mac, macs[5], sizeof(uint8_t)*6);
    //tree_child_lost(&child1);
    //tree_print_levels();
    tree_node_lookup(macs[3]);

}
int main() {
    if (is_test) {
        main_tests();
    } else {
        main_functions();
    }
    return 0;
}

