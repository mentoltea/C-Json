#include <stdio.h>
#include <stdlib.h>
#include "json.h"



int main() {
    init_json();
    
    FILE* fd = fopen("file.json", "r");
    if (!fd) {
        printf("Cannot open\n");
        exit(1);
    }
    json_child glob = read_json(fd);
    fclose(fd);

    json_object obj;
    obj.type = STR;
    obj.data.str = "new data";
    json_pair pair;
    pair.key = "KeYY";
    pair.value = obj;

    glob.fields = vec_add(glob.fields, &pair);

    fd = fopen("added.json", "w");
    save_json(fd, &glob);
    fclose(fd);

    destroy_pages();
    return 0;
}