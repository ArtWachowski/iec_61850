struct API {
    char* (*get_prediction)(char *buffer, int buffer_size);
    int (*release_object)(char *obj);
};

 