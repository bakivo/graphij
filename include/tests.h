//
// Created by Nina Bakaeva on 1/08/21.
//

#ifndef GRAPHIJ_TESTS_H
#define GRAPHIJ_TESTS_H
#include "trees.h"
typedef struct dog {
    int age;
    char name[10];
} dog_t;

void test1() {
    uint8_t num = 5;
    child_t childs1[num];
    child_t childs2[num];
}

void test2() {
    dog_t dog1 = {4,{"barky"}}, dog2 = {5,{"corgi"}};
    printf("%s: %d\n", dog1.name, dog1.age);
    printf("%s: %d\n", dog2.name, dog2.age);
    dog2 = dog1;
    printf("%s: %d\n", dog1.name, dog1.age);
    printf("%s: %d\n", dog2.name, dog2.age);
}


#endif //GRAPHIJ_TESTS_H
