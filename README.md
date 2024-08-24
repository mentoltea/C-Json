# Purpose
To work with JSON-files from C/C++. Provides ability to read, modify and create ones.

# General types
* ## `json_object`
  Consists of `type` identifier and `data` correlating to that type.
  Available types and their data:
  * `STR` - `char* str`
  * `INT` - `int num`
  * `FLOAT` - `float dec`
  * `ARRAY` - `json_object* array`  [vector](README.md#note-about-vectors) of objects
  * `CHILD` - `json_child child`
  
  Types are represented with `enum`, data with `union`.

* ## `json_pair`
  Consists of a `char* key` and a `json_object value`.

* ## `json_child`
  Represents `{ }` objects. Note that the whole file is treated as global `json_child`.
  Consists of [vector](README.md#note-about-vectors) `json_pair* fields`.

## Note about vectors
Vectors used in this project are old-style C vectors. Information about the vector is stored BEFORE the pointer to the beginning, so vectors can be used as arrays.
If you want to get that information, you cast to `vector_metainfo` and do -1 to your pointer. Or you use `vec_meta(vector)` func to get it.
You can find vector repo [here](https://github.com/mentoltea/C-Vector) to get extra info about its usage.

# Usage
## Important
Since project uses [CMemManager](https://github.com/mentoltea/CMemManager), there MUST be `destroy_pages()` at the end of your programm, if you dont want memory to leak.

## Functions 
* ## `void init_json(void)`
  Inits [C-Vector](https://github.com/mentoltea/C-Vector) with functions from [CMemManager](https://github.com/mentoltea/CMemManager).
  Is very recomended to be called at the beginning, especially if you dont dealloc object memory at the end.
* ## `json_child read_json(FILE* fd)`
  Reads a json-file from a file descriptor or another stream. Returns [json_child](README.md#json_child) if succes. Exits with pages deallocation on error.
* ## `void save_json(FILE* fd, json_child* child)`
  Writes a json-file to a file descriptor or another stream. Uses underhood `void fprintchild(FILE *fd, json_child *child, size_t tabs)`.
* ## `void dealloc_json(json_child* child)`
  Deallocates all memory used for child. Makes sense only if lifetime of json object in your programm is less than lifetime of programm, since at the end [all pages must be deallocated](README.md#important),
  or if you are not using `init_json()` at the beginning. Uses underhood `void dealloc_json_child(json_child* child)`.

Other functions are not desinged to be used by user, but you can use them if have a reason to. Descriptons for these funcs are not provided, but their names describes themselves.  
```
json_object read_object(char*, size_t);
json_object *read_array(char*, size_t);
json_child read_child(char*, size_t);

void dealloc_json_child(json_child* child);
void dealloc_json_pair(json_pair *pair);
void dealloc_json_object(json_object *obj);
void dealloc_json_array(json_object* array);

void fprintchild(FILE *fd, json_child *child, size_t tabs);
void fprintarray(FILE *fd, json_object *array, size_t tabs);
```

## Example of usage
This code reads json-file from "build.json", adds a new field named "CH ADD" of child, that consists of fields named (n+1) with a value n for n=0..4, and saves it to file "added.json".  
Another example of usage you can see [here](https://github.com/mentoltea/C-Builder).
```
int main() {
    init_json();
    
    FILE* fd = fopen("build.json", "r");
    if (!fd) {
        printf("Cannot open\n");
        exit(1);
    }
    json_child glob = read_json(fd);
    fclose(fd);

    json_child ch;
    ch.fields = new_vec(sizeof(json_pair), 10);
    json_pair pr;
    json_object ob;
    for (int i=0; i<5; i++) {
        pr.key = memloc(3*sizeof(char));
        sprintf(pr.key, "%d", i+1);
        ob.type = INT;
        ob.data.num = i;
        pr.value = ob;
        ch.fields = vec_add(ch.fields, &pr);
    }

    json_object obj;
    obj.type = CHILD;
    obj.data.child = ch;

    json_pair pair;
    pair.key = "CH ADD";
    pair.value = obj;

    glob.fields = vec_add(glob.fields, &pair);
    
    fd = fopen("added.json", "w");
    save_json(fd, &glob);
    fclose(fd);

    dealloc_json(&glob);
    destroy_pages();
    return 0;
}
```
