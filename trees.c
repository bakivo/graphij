//
// Created by Nina Bakaeva on 28/07/21.
//
#include "include/trees.h"

static char buff[20];
static char* buff_ptr = buff;
static uint8_t last_index;
static node_t nodes[MAX_NODES];
static uint8_t levels[6];
static node_t root = {.mac = {1,1,1,11,12,0},.level = 0};
node_t *get_node(const child_t *node);
void remove_branch(node_t *node);
void visit_nodes(node_t *);

void print_all_nodes(){
    for (int i = 0; i < MAX_NODES; i++) {
        if (*nodes[i].mac != 0) {
            printf(""MACSTR"\n", MAC2STR(nodes[i].mac));
            levels[nodes[i].level]++;
        }
    }
    printf("1 - %d\n2 - %d\n3 - %d\n4 - %d\n5 - %d\n6 - %d\n", levels[0],levels[1],levels[2],levels[3],levels[4],levels[5]);


    visit_nodes(&root);
    printf("\n");
    /*visit_nodes(root.first_child);
    printf("\n");
    visit_nodes(root.first_child->next_sibling);
    printf("\n");
    visit_nodes(root.first_child->next_sibling->next_sibling);*/
    for (int i = 0; i < 20; i++) {
        printf("%c",buff[i]);
    }
}

void visit_nodes(node_t *node) {
    *(buff_ptr++) = ']';
    node_t *next = node;
    while(next != NULL) {
        *(buff_ptr++) = '>';
        printf(""MACSTR"\n", MAC2STR(next->mac));
        if(next->first_child != NULL) {
            visit_nodes(next->first_child);
        }
        next = next->next_sibling;
    }
}



void child_lost(child_t *lost_child) {
    node_t *lost_node = get_node(lost_child);
    if(lost_node == NULL) {
        printf("lost node is not found in array\n");
        return;
    }
    remove_branch(lost_node);
}
//private functions
bool is_same(const uint8_t *node1, const uint8_t *node2) {
    if (*(node1+3)*100 + *(node1+4)*10 + *(node1+5) == *(node2+3)*100 + *(node2+4)*10 + *(node2+5)) return true;
    return false;
}

node_t *find_parent(const child_t *child) {
    for (int i = 0; i < MAX_NODES; i++) {
        if(is_same(nodes[i].mac, child->parent_mac)) return &nodes[i];
    }
    return NULL;
}
node_t *get_node(const child_t *node) {
    for (int i = 0; i < MAX_NODES; i++) {
        if(is_same(nodes[i].mac, node->mac)) return &nodes[i];
    }
    return NULL;
}

// function works with global static array of nodes
my_err append_child(node_t *parent, const child_t *child) {
    my_err res = -1;
    if (last_index >= MAX_NODES || last_index < 0) return res;
    node_t *new = &nodes[last_index];
    new->level = child->level;
    memcpy(new->mac, child->mac, mac_bytes);
    // new child is always pushed into place of the existing first child.
    if(parent->first_child != NULL) {
        new->next_sibling = parent->first_child;
    }
    // update parent's pointer to the first child
    parent->first_child = new;
    return 0;
}

// called from mesh callback CHILD_CONNECTED or mesh_rx function
void child_connected(const child_t *child, bool is_connected_to_root) {
    my_err res;
    if (is_connected_to_root) {
        res = append_child(&root, child);
    } else {
        node_t *current_parent = find_parent(child);
        if(current_parent == NULL) {
            printf("parent_mac not found\n");
            return;
        }
        res = append_child(current_parent,child);
    }
    if(res == 0) last_index++;
    else printf("error of adding a child: %d\n", res);
}

void remove_branch(node_t *node) {
    node_t *parent = NULL, *left = NULL;
    // check if the node being removed is a first child of its parent;
    for (int i = 0; i < MAX_NODES; i++) {
        if(is_same(nodes[i].first_child->mac, node->mac))
            parent = &nodes[i];
    }
    if (parent != NULL) {
        if (node->next_sibling != NULL) {
            parent->first_child = node->next_sibling;
        } else {
            parent->first_child = NULL;
        }
    } // if not so, find the left sibling of it
    else {
        for (int i = 0; i < MAX_NODES; i++) {
            if(is_same(nodes[i].next_sibling->mac, node->mac))
                left = &nodes[i];
        }
        if (node->next_sibling != NULL) {
            left->next_sibling = node->next_sibling;
        } else {
            left->next_sibling = NULL;
        }
    }
    // to this point all references must be updated
    // relay node to recursively delete itself and all the childs.

}