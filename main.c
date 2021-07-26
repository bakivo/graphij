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
        {.mac = {1, 1, 1, 11, 12, 11},.level = 1},
        {.mac = {1, 1, 1, 11, 12, 12},.level = 1},
        {.mac = {1, 1, 1, 11, 12, 13},.level = 1},
        {.mac = {1, 1, 1, 11, 12, 21},.level = 1},
        {.mac = {1, 1, 1, 11, 12, 22},.level = 1},
        {.mac = {1, 1, 1, 11, 12, 23},.level = 1},
        {.mac = {1, 1, 1, 11, 12, 31},.level = 1},
        {.mac = {1, 1, 1, 11, 12, 32},.level = 1},
        {.mac = {1, 1, 1, 11, 12, 33},.level = 1},
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
void remove_branch(node_t *node) {
    node_t *node_to_remove = NULL, *parent_of_node = NULL, *left_sibling = NULL;
    for (int i = 0; i < MAX_NODES; i++) {
        if(is_same(nodes[i].next_sibling->mac, node->mac))
            left_sibling = &nodes[i];
    }
    if (left_sibling == NULL) {
        for (int i = 0; i < MAX_NODES; i++) {
            if(is_same(nodes[i].first_child->mac, node->mac))
                parent_of_node = &nodes[i];
        }
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
    fill_child(&childs[0],&root_childs[1]);
    fill_child(&childs[1],&root_childs[1]);
    child_connected(&childs[0], false);
    child_connected(&childs[1], false);
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
    printf("level 1: %d level 2: %d level 3: %d level 4: %d level 5: %d level 6: %d\n", levels[0],levels[1],levels[2],levels[3],levels[4],levels[5]);
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
void test2(){
    uint8_t mac[6] = {1,1,1,11,12,32};
    printf("%d\n", is_same(childs[8].mac, mac));
}
int main() {
    main_fun();
    print_all_nodes();
    return 0;
}

