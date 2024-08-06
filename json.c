#include "json.h"

void init_json() {
    set_funcs(memloc, memfree, memcpy);
    prealloc(PAGE_SIZE*4);
}

void exit_on_error(int code) {
    page_info(0);
    destroy_pages();
    exit(code);
}

// reads the entire file
char* read_from_file(FILE* fd, size_t size) {
    char *str = memloc(size);
    fread(str, sizeof(char), size, fd);
    return str;
}

char* string_from_ptrs(char* start, char* end) {
    char *str = memloc(sizeof(char)*(end-start+1));
    memcpy(str, start, sizeof(char)*(end-start+1));
    str[end-start] = '\0';
    return str;
}

char* remove_spaces(char* str, size_t size) {
    while (size!=0 && (*str <=32)) {
        str++; size--;
    }
    while (size!=0 && (*(str+size-1) <= 32)) {
        size--;
    }
    char* res;
    if (size==0) return NULL;
    res = string_from_ptrs(str, str+size);
    return res;
}

size_t count_of(char *start, char *end, char symb) {
    char* ptr = start;
    size_t size = 0;
    while (ptr && ptr<end) {
        ptr = strchr(ptr, symb);
        if (!ptr || ptr>=end) break;
        size++;
        ptr++;
    }
    return size;
}

// splits everything into pairs
// @return vector of tokens
Token* split_into_pairs(char *str, size_t size, int array) {
    char *ptr, *start;
    char *end = str+size;
    char stopsymb = '}';
    if (array) stopsymb = ']';
    while (size!=0 && *end != stopsymb) {
        end--; size--;
    }
    if (size==0) {
        printf("Not found global %c\n", stopsymb);
        exit_on_error(1);
    }
    
    Token* vec = new_vec(sizeof(Token), 10);
    Token temp;
    for (ptr = str+1; ptr != end; ptr++) {
        if (*ptr == ',') {
            ptr++;
        }
        if (*ptr == '{' || *ptr == '}' || *ptr=='\n' || *ptr=='\t') {
            continue;
        }
        start = ptr;
        ptr = strchr(ptr, ',');
        if (!ptr) ptr = end-1;

        temp.start = start; temp.end = ptr; temp.parent = NULL;
        vec = vec_add(vec, &temp);
    }
    vector_metainfo *meta = vec_metaptr(vec);
    Token* pairs = new_vec(sizeof(Token), 10);
    
    for (int i=0; i<meta->length; i++) {
        temp = vec[i];
        if (count_of(temp.start, temp.end, '"')%2 == 1) {
            if (i!=meta->length-1) {
                vec[i+1].start = temp.start;
                continue;
            }
            printf("Syntax error with \"");
            printf("%s\n", string_from_ptrs(temp.start, temp.end));
            exit_on_error(1);
        }
        if (count_of(temp.start, temp.end, '[') != count_of(temp.start, temp.end, ']')) {
            if (i!=meta->length-1) {
                vec[i+1].start = temp.start;
                continue;
            }
            printf("Syntax error with []");
            printf("%s\n", string_from_ptrs(temp.start, temp.end));
            exit_on_error(1);
        } 
        if (count_of(temp.start, temp.end, '{') != count_of(temp.start, temp.end, '}')) {
            if (i!=meta->length-1) {
                vec[i+1].start = temp.start;
                continue;
            }
            printf("Syntax error with {}");
            printf("%s\n", string_from_ptrs(temp.start, temp.end));
            exit_on_error(1);
        }
        pairs = vec_add(pairs, &temp);
    }
    delete_vec(vec);
    return pairs;
}

json_pair parse_pair(Token *tk) {
    json_pair pair;
    char* ptr = strchr(tk->start, ':');
    if (!ptr || ptr>=tk->end) {
        printf("Cannot parse pair\n%s\n", string_from_ptrs(tk->start, tk->end));
        exit_on_error(1);
    }
    char* key = remove_spaces(tk->start+1, ptr-tk->start-1);
    char* value = remove_spaces(ptr+1, tk->end-ptr-1);
    size_t keysize = strlen(key);
    if (*key=='"') {
        key++; keysize--;
    }
    if (*(key+keysize-1)=='"'){
        key[keysize-1] = '\0';
    }
    pair.key = key;
    // printf(key);
    //printf("SIZE OF %s %d\n", value, strlen(value));
    pair.value = read_object(value, strlen(value)+1);
    return pair;   
}

json_object_types object_type(char* value, size_t size) {
    json_object_types type;
    switch (*value) {
        case '[':
            type = ARRAY;
            break;
        case '{':
            type = CHILD;
            break;
        case '"':
            type = STR;
            break;
        default:
            type = INT;
            break;
    }
    char* ptr;
    if (type == INT) {
        ptr = strchr(value, '.');
        if (ptr && ptr<value+size) {
            type = FLOAT;
        }
    }

    return type;
}

json_object read_object(char* value, size_t size) {
    json_object_types type = object_type(value, size);
    json_object obj; obj.type = type;
    json_child *child = memloc(sizeof(json_child));
    switch (type) {
        case STR:
            obj.data.str = string_from_ptrs(value+1, value+size-2);
            // printf("str | %s\n", obj.data.str);
            break;
        case INT:
            obj.data.num = strtol(value, NULL, 0);
            // printf("num | %d\n", obj.data.num);
            break;
        case FLOAT:
            obj.data.dec = strtof(value, NULL);
            // printf("dec | %f\n", obj.data.dec);
            break;
        case ARRAY:
            obj.data.array = read_array(value, size);
            // printf("array | %s\n", value);
            break;
        case CHILD:
            // printf("child | %s\n", value);
            *child = read_child(value, size);
            obj.data.child = child;
            break;
        default:
            break;
    }
    return obj;
}

// @return vector of objects
json_object *read_array(char *value, size_t size) {
    Token* pairs = split_into_pairs(value, size, 1); // vector
    vector_metainfo meta = vec_meta(pairs);
    json_object *array = new_vec(sizeof(json_object), meta.length);
    json_object temp;
    for (int i=0; i<meta.length; i++) {
        // printf(remove_spaces(pairs[i].start, pairs[i].end - pairs[i].start));
        // printf("##\n");
        temp = read_object(remove_spaces(pairs[i].start, pairs[i].end - pairs[i].start),
                pairs[i].end - pairs[i].start);
        // printf("%d\n", temp.type);
        array = vec_add(array, &temp);
    }
    return array;
}

json_child read_child(char *str, size_t size) {
    Token* pairs = split_into_pairs(str, size, 0); // vector
    vector_metainfo meta = vec_meta(pairs);
    json_child child;
    child.fields = new_vec(sizeof(json_pair), meta.length);
    json_pair temp;
    for (int i=0; i<meta.length; i++) {
        temp = parse_pair(pairs+i);
        child.fields = vec_add(child.fields, &temp);
    }
    return child;
}

json_child read_json(FILE* fd) {
    fseek(fd, 0, SEEK_END);
    size_t size = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    char* str = read_from_file(fd, size);
    return read_child(str, size);
}