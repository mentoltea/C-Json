#include <stdio.h>
#include <stdlib.h>
#include "json.h"

void printarray(json_object *array, size_t tabs) {
    size_t iter;

    // iter = tabs;
    // while (iter>0) {
    //     printf("\t");
    //     iter--;
    // } 
    printf("[\n");

    // tabs++;
    vector_metainfo meta = vec_meta(array);
    // printf("size %d\n", meta.length);
    json_object temp;
    // for (int i=0; i<meta.length; i++) {
    //     printf("%d ", array[i].type);
    // }
    for (int i=0; i<meta.length; i++) {
        temp = array[i];
        iter = tabs;
        while (iter>0) {
            printf("\t");
            iter--;
        } 
        switch (temp.type) {
            case STR:
                printf("\"%s\"", temp.data.str);
                break;
            case INT:
                printf("%d", temp.data.num);
                break;
            case FLOAT:
                printf("%f", temp.data.dec);
                break;
            case CHILD:
                printchild(temp.data.child, tabs);
                break;
            case ARRAY:
                printarray(temp.data.array, tabs+1);
                break;

            default:
                break;
        }
        if (i!=meta.length-1) {
            printf(",");
        }
        //printf(" | %d", temp.type);
        printf("\n");
    }
    tabs--;
    iter = tabs;
    while (iter>0) {
        printf("\t");
        iter--;
    } printf("]");
}

void printchild(json_child *child, size_t tabs) {
    size_t iter;

    // iter = tabs;
    // while (iter>0) {
    //     printf("\t");
    //     iter--;
    // }  
    printf("{\n");

    tabs++;
    vector_metainfo meta = vec_meta(child->fields);
    // printf("size %d\n", meta.length);
    json_pair temp;
    for (int i=0; i<meta.length; i++) {
        temp = child->fields[i];
        iter = tabs;
        while (iter>0) {
            printf("\t");
            iter--;
        } 
        printf("\"%s\" : ", temp.key);
        switch (temp.value.type) {
            case STR:
                printf("\"%s\"", temp.value.data.str);
                break;
            case INT:
                printf("%d", temp.value.data.num);
                break;
            case FLOAT:
                printf("%f", temp.value.data.dec);
                break;
            case CHILD:
                printchild(temp.value.data.child, tabs);
                break;
            case ARRAY:
                printarray(temp.value.data.array, tabs+1);
                break;

            default:
                break;
        }
        if (i!=meta.length-1) {
            printf(",");
        }
        //printf(" | %d", temp.value.type);
        printf("\n");
    }
    tabs--;
    iter = tabs;
    while (iter>0) {
        printf("\t");
        iter--;
    } printf("}");
    if (tabs == 0) {
        printf("\n");
    }
}

int main() {
    init_json();
    
    FILE* fd = fopen("file.json", "r");
    if (!fd) {
        printf("Cannot open\n");
        exit(1);
    }
    json_child glob = read_json(fd);
    printchild(&glob, 0);

    destroy_pages();
    return 0;
}