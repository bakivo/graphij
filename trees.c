//
// Created by Nina Bakaeva on 28/07/21.
//
#include "include/trees.h"
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
typedef int16_t my_err;
// main memory for storing tree's nodes
typedef struct node {
    struct node *first_child;
    struct node *next_sibling;
    uint8_t mac[6];
    uint8_t level;
} node_t;
static node_t nodes[MAX_NODES];
static node_t buff[MAX_NODES];
static node_t root;
static node_t *ptrs[20];
static uint8_t ptrs_index;
static uint8_t new_index;          // very important global variable indexing the last node in array {nodes}
static uint8_t levels[6];           // distribution of nodes on mesh levels
// function's definitions
node_t *get_node(const uint8_t *);
node_t *find_parent(const child_t *child);
my_err append_child(node_t *parent, const child_t *child);
void remove_branch(node_t *node);
void visit_nodes(node_t *);
void print_visited_nodes();
void adjust_array();
void delete_marked_nodes();

void tree_set_root(uint8_t *root_mac, uint8_t level) {
    memcpy(root.mac, root_mac, 6 * sizeof(uint8_t));
    root.level = level;
}

void tree_print_levels(){
    for (int i = 0; i < MAX_NODES; i++) {
        if (*nodes[i].mac != 0) {
            printf(""MACSTR"\n", MAC2STR(nodes[i].mac));
            levels[nodes[i].level]++;
        }
        else printf("node\n");
    }
    printf("0 - %d\n1 - %d\n2 - %d\n3 - %d\n4 - %d\n5 - %d\n", 1,levels[1],levels[2],levels[3],levels[4],levels[5]);
}

// called from mesh callback CHILD_CONNECTED or mesh_rx function
void tree_child_connected(const child_t *child, bool is_connected_to_root) {
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
    if(res == 0) new_index++;
    else printf("error of adding a child: %d\n", res);
}

void tree_child_lost(child_t *lost_child) {
    node_t *lost_node = get_node(lost_child->mac);
    if(lost_node == NULL) {
        printf("lost node is not found in array\n");
        return;
    }
    printf("Remove branch of: "MACSTR"\n", MAC2STR(lost_node->mac));
    remove_branch(lost_node);
}

void tree_node_lookup(uint8_t *mac) {
    node_t *node = get_node(mac);
    if (node == NULL) return;
    printf(""MACSTR"\n", MAC2STR(mac));
    if (node->first_child == NULL) return;
    visit_nodes(node->first_child);
    print_visited_nodes();
}

//private functions **********************************************
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
node_t *get_node(const uint8_t *mac) {
    if (is_same(root.mac, mac)) return &root;

    for (int i = 0; i < MAX_NODES; i++) {
        if(is_same(nodes[i].mac, mac)) return &nodes[i];
    }
    printf(""MACSTR" node is not found\n", MAC2STR(mac));
    return NULL;
}
// function works with global static array of nodes
my_err append_child(node_t *parent, const child_t *child) {
    my_err res = -1;
    if (new_index >= MAX_NODES || new_index < 0) return res;
    node_t *new = &nodes[new_index];
    new->level = parent->level + 1;
    memcpy(new->mac, child->mac, sizeof(uint8_t) * 6);
    // new child is always pushed into place of the existing first child.
    if(parent->first_child != NULL) {
        new->next_sibling = parent->first_child;
    }
    // update parent's pointer to the first child
    parent->first_child = new;
    return 0;
}
void remove_branch(node_t *node) {
    node_t *parent = NULL;
    node_t *left = NULL;
    // check if the node being removed is a first child of its parent;
    for (int i = 0; i < MAX_NODES; i++) {
        if(is_same(nodes[i].first_child->mac, node->mac))
            parent = &nodes[i];
            break;
    }
    if (parent != NULL) {
        printf("Parent of node removed: "MACSTR"\n", MAC2STR(parent->mac));
        if (node->next_sibling != NULL) {
            parent->first_child = node->next_sibling;
        } else {
            parent->first_child = NULL;
        }
    } // if not so, find the left sibling of it
    else {
        printf("wrong\n");
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
    // relay node to recursively mark all it's childs to delete.

    ptrs[ptrs_index++] = node;
    if (node->first_child != NULL) {
        visit_nodes(node->first_child);
    }
    delete_marked_nodes();
}

void visit_nodes(node_t *node) {
    node_t *next = node;
    while(next != NULL) {
        //printf(""MACSTR"\n", MAC2STR(next->mac));
        ptrs[ptrs_index++] = next;
        if(next->first_child != NULL) {
            visit_nodes(next->first_child);
        }
        next = next->next_sibling;
    }
}
void print_visited_nodes(){
    for (int i = 0; i < ptrs_index; i++) {
        printf(""MACSTR"\n", MAC2STR(ptrs[i]->mac));
        ptrs[i] = NULL;
    }
    ptrs_index = 0;
}

void delete_marked_nodes() {
    for (int i = 0; i < ptrs_index; i++) {
        memset(ptrs[i], 0, sizeof(node_t));
        ptrs[i] = NULL;
    }
    ptrs_index = 0;
    adjust_array();
}

void adjust_array() {
    // clear buff
    memset(buff, 0, sizeof(node_t) * MAX_NODES );
    uint8_t j = 0; // will indicate how many non-empty nodes to copy
    for (int i = 0; i < MAX_NODES; i++) {
        if (nodes->mac[0] != 0) {
            buff[j++] = nodes[i];
        }
    }
    // now buff array contains all non-empty nodes
    // clear nodes array and copy buff array to it
    memset(nodes, 0, sizeof(node_t) * MAX_NODES);
    memcpy(nodes, buff, sizeof(node_t) * (j + 1) );
    new_index = j + 1;
}