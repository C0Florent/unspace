#define _GNU_SOURCE
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

struct unspace_input {
    // Actual runtime options (affect the behaviour within the renaming loops)
    struct unspace_options {
        bool verbose;
        char replace;
    } o;

    // "Control"-like arguments
    bool dump_input;
    bool recursive;
    char **files;
    size_t filec;
};

int read_cli(int argc, char **argv, struct unspace_input *input)
{
    int opt;

    while ((opt = getopt(argc, argv, "vrdc:")) != -1) {
        switch (opt) {
        case 'v':
            input->o.verbose = true;
            break;
        case 'r':
            input->recursive = true;
            break;
        case 'd':
            input->dump_input = true;
            break;
        case 'c':
            if (strlen(optarg) != 1) {
                fprintf(stderr, "-c option expects a single character\n");
                return 1;
            }
            input->o.replace = optarg[0];
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
    printf("verbose: %i\n", input->o.verbose);
    printf("recursive: %i\n", input->recursive);
    printf("dump_input: %i\n", input->dump_input);
    printf("replace: %c\n", input->o.replace);
    printf("files:\n");
    for (size_t i = 0; i < input->filec; i++) {
        printf("- %s\n", input->files[i]);
    }
}

int unspace(
    char const *pname,
    struct unspace_options *options,
    int dirfd,
    char const *filename
)
{
    static char renamebuf[PATH_MAX] = {0};
    bool renamed = false;

    strcpy(renamebuf, filename);
    for (size_t j = 0; renamebuf[j]; j++) {
        if (renamebuf[j] == ' ') {
            renamebuf[j] = options->replace;
            renamed = true;
        }
    }
    if (!renamed) {
        fprintf(stderr, "%s: ignoring input '%s' (no spaces)\n", pname, renamebuf);
        return 0;
    }
    if (renameat2(dirfd, filename, dirfd, renamebuf, RENAME_NOREPLACE)) {
        fprintf(stderr, "%s: error renaming '%s': %s\n",
                pname, filename, strerror(errno));
        return 1;
    }
    if (options->verbose) {
        printf("%s: '%s' -> '%s'\n", pname, filename, renamebuf);
    }
    return 0;
}

int main(int argc, char **argv)
{
    struct unspace_input input = {
        .o.verbose = false,
        .o.replace = '_',
        .recursive = false,
        .files = NULL,
        .filec = 0,
    };
    int ret = 0;

    if (read_cli(argc, argv, &input)) {
        return 1;
    }
    if (input.dump_input) {
        show_inputs(&input);
    }

    for (size_t i = 0; i < input.filec; i++) {
        if (input.recursive) {
            fprintf(stderr, "recursive unimplemented, renaming non-recursively instead\n");
            ret = 1;
        }
        ret |= unspace(argv[0], &input.o, AT_FDCWD, input.files[i]);
    }

    return ret;
}
