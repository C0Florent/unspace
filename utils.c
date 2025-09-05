#include <stdio.h>
#include <stddef.h>
#include <unistd.h>

#include "unspace.h"

void show_inputs(struct unspace_input const *input)
{
    printf("verbose: %i\n", input->o.verbose);
    printf("recursive: %i\n", input->o.recursive);
    printf("dump_input: %i\n", input->dump_input);
    printf("dry_run: %i\n", input->o.dry_run);
    printf("replace: %c\n", input->o.replace);
    printf("%zu files:\n", input->filec);
    for (size_t i = 0; i < input->filec; i++) {
        printf("- %s\n", input->files[i]);
    }
}

const char *get_fd_filename(int fd)
{
    static char ret[PATH_MAX];
    char searchpath[128];
    ssize_t retlen;

    sprintf(searchpath, "/proc/self/fd/%i", fd);
    retlen = readlink(searchpath, ret, PATH_MAX);

    if (retlen < 0) {
        return "<unknown>";
    } else {
        ret[retlen] = '\0';
        return ret;
    }
}
