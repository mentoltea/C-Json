#include <stdio.h>
#include <stdlib.h>
#include "json.h"



int main() {
    init_json();
    
    FILE* fd = fopen("build.json", "r");
    if (!fd) {
        printf("Cannot open\n");
        exit(1);
    }
    json_child glob = read_json(fd);
    fclose(fd);

    // fprintchild(stdout, &glob, 0);

    // json_child ch;
    // ch.fields = new_vec(sizeof(json_pair), 10);
    // json_pair pr;
    // json_object ob;
    // for (int i=0; i<5; i++) {
    //     pr.key = memloc(3*sizeof(char));
    //     sprintf(pr.key, "%d", i+1);
    //     ob.type = INT;
    //     ob.data.num = i;
    //     pr.value = ob;
    //     ch.fields = vec_add(ch.fields, &pr);
    // }

    // json_object obj;
    // obj.type = CHILD;
    // obj.data.child = ch;

    // json_pair pair;
    // pair.key = "CH ADD";
    // pair.value = obj;

    // glob.fields = vec_add(glob.fields, &pair);

    fd = fopen("copy.json", "w");
    save_json(fd, &glob);
    fclose(fd);

    page_info(0);
    destroy_pages();
    printf("\nsucces\n");
    return 0;
}