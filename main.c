#include <stdio.h>
#include <libc.h>
#include <stdbool.h>
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
typedef int16_t my_err;
static uint8_t mac_bytes = sizeof(uint8_t) * 6;
typedef struct {
    uint8_t parent_mac[6];
    uint8_t mac[6];
    uint8_t level;
} child_t;
#define ROOT_CHILDS_SIZE (3)
#define CHILDS_SIZE (9)
static child_t root_childs[ROOT_CHILDS_SIZE] = {
        {.mac = {1, 1, 1, 11, 12, 1},.level = 0},
        {.mac = {1, 1, 1, 11, 12, 2},.level = 0},
        {.mac = {1, 1, 1, 11, 12, 3},.level = 0}
};
static child_t childs[CHILDS_SIZE] = {
        {.mac = {1, 1, 1, 11, 12, 26},.level = 1},
        {.mac = {1, 1, 1, 11, 12, 27},.level = 1},
        {.mac = {1, 1, 1, 11, 12, 28},.level = 1},
        {.mac = {1, 1, 1, 11, 12, 42},.level = 1},
        {.mac = {1, 1, 1, 11, 12, 43},.level = 1},
        {.mac = {1, 1, 1, 11, 12, 44},.level = 1},
        {.mac = {1, 1, 1, 11, 12, 58},.level = 1},
        {.mac = {1, 1, 1, 11, 12, 59},.level = 1},
        {.mac = {1, 1, 1, 11, 12, 60},.level = 1},
};
typedef struct node {
    struct node *root;
    struct node *first_child;
    struct node *next_sibling;
    uint8_t mac[6];
    uint8_t level;
} node_t;
static uint8_t last_index;
#define MAX_NODES 20
static node_t nodes[MAX_NODES];
static uint8_t levels[6];
static node_t root = {.mac = {1,1,1,11,12,0}};

void fill_child(child_t *child, child_t *parent) {
    memcpy(child->parent_mac, parent->mac, mac_bytes);
}

// called from mesh callback MESH_STARTED
void mesh_started() {
    root.root = NULL;
}
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
    if(parent->first_child != NULL) {
        new->next_sibling = parent->first_child;
    }
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
//void print_leaf()

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
void child_lost(child_t *lost_child) {
    node_t *lost_node = get_node(lost_child);
    if(lost_node == NULL) {
        printf("lost node is not found in array\n");
        return;
    }
    remove_branch(lost_node);
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
    //fill_child(&childs[6],&root_childs[2]);
    //child_connected(&childs[6], false);
    //fill_child(&childs[7],&root_childs[2]);
    //child_connected(&childs[7], false);
}
void visit_nodes(node_t *node) {
    node_t *next = node;
    while(next != NULL) {
        printf(""MACSTR"\n", MAC2STR(next->mac));
        if(next->first_child != NULL) {
            visit_nodes(next->first_child);
        }
        next = next->next_sibling;
    }
}

void print_all_nodes(){
    for (int i = 0; i < MAX_NODES; i++) {
        if (*nodes[i].mac != 0) {
            printf(""MACSTR"\n", MAC2STR(nodes[i].mac));
            levels[nodes[i].level]++;
            //printf(""MACSTR"\n", MAC2STR(nodes[i].first_child->mac));
            //printf("child %d %p\n",i, &nodes[i]);
            //printf("next sibling %p\n", nodes[i].next_sibling);
        }
    }
    printf("1 - %d\n2 - %d\n3 - %d\n4 - %d\n5 - %d\n6 - %d\n", levels[0],levels[1],levels[2],levels[3],levels[4],levels[5]);


    visit_nodes(&root);
    printf("\n");
    visit_nodes(root.first_child);
    printf("\n");
    visit_nodes(root.first_child->next_sibling);
    printf("\n");
    visit_nodes(root.first_child->next_sibling->next_sibling);

}
void main_fun(){
    mesh_started();
    mesh_child_connected();
    mesh_rx();
}
void test(){
    int i = 3;
    printf(""MACSTR"\n", MAC2STR(childs[i].mac));
    printf(""MACSTR"\n", MAC2STR(childs[i].parent_mac));
    fill_child(&childs[i],&root_childs[1]);
    printf(""MACSTR"\n", MAC2STR(childs[i].mac));
    printf(""MACSTR"\n", MAC2STR(childs[i].parent_mac));
}
int main() {
    main_fun();
    print_all_nodes();
    return 0;
}

