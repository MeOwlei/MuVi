#ifndef MUZK_H_
#define MUZK_H_ 

#define LIST_OF_FUNC    \
    X(muzk_init, void, const char* file_path)      \
    X(muzk_update, void, void)      \
    X(muzk_pre_reload, void*, void)  \
    X(muzk_post_reload, void, void*)

#define X(name, ret, ...) typedef ret (name##_t)(__VA_ARGS__);
LIST_OF_FUNC
#undef X

#endif // MUZK_H_


