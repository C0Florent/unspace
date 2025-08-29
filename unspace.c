#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

struct unspace_input {
    // Flags/options
    bool verbose;
    bool recursive;
    bool dump_options;
    char replace;

    // File arguments
    char **files;
    size_t filec;
};

int read_cli(int argc, char **argv, struct unspace_input *input)
{
    int opt;

    while ((opt = getopt(argc, argv, "vrdc:")) != -1) {
        switch (opt) {
        case 'v':
            input->verbose = true;
            break;
        case 'r':
            input->recursive = true;
            break;
        case 'd':
            input->dump_options = true;
            break;
        case 'c':
            if (strlen(optarg) != 1) {
                fprintf(stderr, "-c option expects a single character\n");
                return 1;
            }
            input->replace = optarg[0];
            break;
        default:
            fprintf(stderr, "Usage: %s [-vrd] [-c replacechar] file...\n", argv[0]);
            return 1;
        }
    }
    if (optind >= argc) {
        fprintf(stderr, "%s: Expected at least one file argument\n", argv[0]);
        return 1;
    }
    input->files = &argv[optind];
    input->filec = argc - optind;
    return 0;
}

void show_inputs(struct unspace_input const *input)
{
    printf("verbose: %i\n", input->verbose);
    printf("recursive: %i\n", input->recursive);
    printf("dump_options: %i\n", input->dump_options);
    printf("replace: %c\n", input->replace);
    printf("files:\n");
    for (size_t i = 0; i < input->filec; i++) {
        printf("- %s\n", input->files[i]);
    }
}

int main(int argc, char **argv)
{
    struct unspace_input input = {
        .verbose = false,
        .recursive = false,
        .replace = '_',
        .files = NULL,
        .filec = 0,
    };

    if (read_cli(argc, argv, &input)) {
        return 1;
    }
    if (input.dump_options) {
        show_inputs(&input);
    }
}
