#ifndef UNSPACE_H
    #define UNSPACE_H

    #define _GNU_SOURCE // Enables GNU extensions
    #include <dirent.h> // Required for PATH_MAX

    #include <stdbool.h>
    #include <stddef.h>

struct unspace_input {
    // Actual runtime options (affect the behaviour within the renaming loops)
    struct unspace_options {
        bool verbose;
        char replace;
        bool recursive;
    } o;

    // "Control"-like arguments
    bool dump_input;
    char **files;
    size_t filec;
};

// data shared between recursive calls of unspace_rec
struct unspace_rec_data {
    char padding[PATH_MAX];
    size_t depth; // aka the index of the first \0 in `padding`
};


int read_cli(int argc, char **argv, struct unspace_input *input);

const char *get_fd_filename(int fd);
void show_inputs(struct unspace_input const *input);

#endif /* UNSPACE_H */
