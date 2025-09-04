#include <getopt.h>
#include <string.h>
#include <stdio.h>

#include "unspace.h"

static int check_filename_lengths(char const *pname, struct unspace_input const *input)
{
    int ret = 0;

    for (size_t i = 0; i < input->filec; i++) {
        if (strlen(input->files[i]) > PATH_MAX - 1) {
            fprintf(stderr, "%s: pathname too long: '%s'\n", pname, input->files[i]);
            ret = 1;
        }
    }
    return ret;
}

int read_cli(int argc, char **argv, struct unspace_input *input)
{
    int opt;

    while ((opt = getopt(argc, argv, "Vvrdnc:")) != -1) {
        switch (opt) {
        case 'v':
            input->o.verbose = true;
            break;
        case 'r':
            input->o.recursive = true;
            break;
        case 'd':
            input->dump_input = true;
            break;
        case 'n':
            input->o.dry_run = true;
            input->o.verbose = true;
            break;
        case 'c':
            if (strlen(optarg) != 1) {
                fprintf(stderr, "-c option expects a single character\n");
                return 1;
            }
            input->o.replace = optarg[0];
            break;
        case 'V':
            input->show_version = true;
            break;
        default:
            fprintf(stderr, "Usage: %s [-vrd] [-c replacechar] file...\n", argv[0]);
            return 1;
        }
    }
    if (optind >= argc && !input->show_version) {
        fprintf(stderr, "%s: Expected at least one file argument\n", argv[0]);
        return 1;
    }
    input->files = &argv[optind];
    input->filec = argc - optind;
    return check_filename_lengths(argv[0], input);
}
