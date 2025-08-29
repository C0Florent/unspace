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
#include <sys/stat.h>
#include <fcntl.h>

#define GETDENTS_BUFSIZ 2048

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

struct linux_dirent64 {
    ino64_t        d_ino;    /* 64-bit inode number */
    off64_t        d_off;    /* Not an offset; see getdents() */
    unsigned short d_reclen; /* Size of this dirent */
    unsigned char  d_type;   /* File type */
    char           d_name[]; /* Filename (null-terminated) */
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
            input->o.recursive = true;
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
    printf("recursive: %i\n", input->o.recursive);
    printf("dump_input: %i\n", input->dump_input);
    printf("replace: %c\n", input->o.replace);
    printf("files:\n");
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
        if (!options->recursive) {
            fprintf(stderr, "%s: ignoring input '%s' (no spaces)\n", pname, renamebuf);
        }
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

int unspace_rec(
    char const *pname,
    struct unspace_options *options,
    int current_dirfd,
    char const *entry
)
{
    int ret = 0;
    int subdirfd = openat(current_dirfd, entry, O_RDONLY | O_DIRECTORY | O_NOFOLLOW);
    char *getdents_buf;
    ssize_t nread;

    if (subdirfd < 0 && (errno == ENOTDIR || errno == ELOOP)) {
        return unspace(pname, options, current_dirfd, entry);
    }
    if (subdirfd < 0) {
        fprintf(stderr,
                "%s: found weird or invalid entry '%s/%s' (might be an implementation mistake): '%s'\n",
                pname, get_fd_filename(current_dirfd), entry, strerror(errno));
        return 1;
    }
    getdents_buf = malloc(sizeof(char) * GETDENTS_BUFSIZ);
    if (!getdents_buf) {
        fprintf(stderr, "%s: failed allocation: %s\n", pname, strerror(errno));
        close(subdirfd);
        return 1;
    }
    while ((nread = getdents64(subdirfd, getdents_buf, GETDENTS_BUFSIZ)) > 0) {
        struct linux_dirent64 *d;

        for (ssize_t bpos = 0; bpos < nread;) {
            d = (struct linux_dirent64 *)&getdents_buf[bpos];

            if ((d->d_type == DT_DIR || d->d_type == DT_UNKNOWN)
            && strcmp(".", d->d_name) && strcmp("..", d->d_name)) {
                ret |= unspace_rec(pname, options, subdirfd, d->d_name);
            }
            ret |= unspace(pname, options, subdirfd, d->d_name);

            bpos += d->d_reclen;
        }
    }
    if (nread < 0) {
        fprintf(stderr, "%s: Failed reading directory entries for '%s': %s\n",
                pname, get_fd_filename(subdirfd), strerror(errno));
        close(subdirfd);
        free(getdents_buf);

        return 1;
    }
    close(subdirfd);
    free(getdents_buf);
    return ret;
}

int main(int argc, char **argv)
{
    struct unspace_input input = {
        .o.verbose = false,
        .o.replace = '_',
        .o.recursive = false,
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
        if (input.o.recursive) {
            unspace_rec(argv[0], &input.o, AT_FDCWD, input.files[i]);
        }
        ret |= unspace(argv[0], &input.o, AT_FDCWD, input.files[i]);
    }

    return ret;
}
