#define _GNU_SOURCE
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

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

size_t get_max_len(char *const *files, size_t filec)
{
    size_t ret = 0;
    size_t len;

    for (size_t i = 0; i < filec; i++) {
        len = strlen(files[i]);
        if (len > ret) {
            ret = len;
        }
    }
    return ret;
}

int unspace(char const *pname, struct unspace_input *input)
{
    char renamebuf[get_max_len(input->files, input->filec) + 1] = {};
    int ret = 0;

    for (size_t i = 0; i < input->filec; i++) {
        bool renamed = false;

        strcpy(renamebuf, input->files[i]);
        for (size_t j = 0; renamebuf[j]; j++) {
            if (renamebuf[j] == ' ') {
                renamebuf[j] = input->replace;
                renamed = true;
            }
        }
        if (!renamed) {
            fprintf(stderr, "%s: ignoring input '%s' (no spaces)\n", pname, renamebuf);
            continue;
        }
        if (renameat2(AT_FDCWD, input->files[i], AT_FDCWD, renamebuf, RENAME_NOREPLACE)) {
            fprintf(stderr, "%s: error renaming '%s': %s\n",
                    pname, input->files[i], strerror(errno));
            ret = 1;
            continue;
        }
        if (input->verbose) {
            printf("%s: '%s' -> '%s'\n", pname, input->files[i], renamebuf);
        }
    }
    return ret;
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

    return unspace(argv[0], &input);
}
