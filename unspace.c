#define _GNU_SOURCE
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

#include "unspace.h"

#define GETDENTS_BUFSIZ 2048

struct linux_dirent64 {
    ino64_t        d_ino;    /* 64-bit inode number */
    off64_t        d_off;    /* Not an offset; see getdents() */
    unsigned short d_reclen; /* Size of this dirent */
    unsigned char  d_type;   /* File type */
    char           d_name[]; /* Filename (null-terminated) */
};

bool should_recurse_into(struct linux_dirent64 const *d)
{
    return (d->d_type == DT_DIR || d->d_type == DT_UNKNOWN)
        && strcmp(".", d->d_name) && strcmp("..", d->d_name);
}

int unspace(
    char const *pname,
    struct unspace_options *options,
    int dirfd,
    char const *filename,
    struct unspace_rec_data *rec_data
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
            fprintf(stderr,
                    "%s:%s ignoring input '%s' (no spaces)\n",
                    pname, rec_data->padding, renamebuf);
        }
        return 0;
    }
    if (!options->dry_run && renameat2(dirfd, filename, dirfd, renamebuf, RENAME_NOREPLACE)) {
        fprintf(stderr, "%s:%s error renaming '%s': %s\n",
                pname, rec_data->padding, filename, strerror(errno));
        return 1;
    }
    if (options->verbose) {
        printf("%s:%s '%s' -> '%s'\n", pname, rec_data->padding, filename, renamebuf);
    }
    return 0;
}

int unspace_rec(
    char const *pname,
    struct unspace_options *options,
    int current_dirfd,
    char const *entry,
    struct unspace_rec_data *rec_data
);
// "unspaces" a single linux_dirent64 directory entry.
int unspace_rec_direntry(
    char const *pname,
    struct unspace_options *options,
    int dirfd,
    struct linux_dirent64 *d,
    struct unspace_rec_data *rec_data
)
{
    int ret;

    if (!should_recurse_into(d)) {
        return unspace(pname, options, dirfd, d->d_name, rec_data);
    }
    if (options->verbose) {
        rec_data->padding[rec_data->depth] = ' ';
        rec_data->depth++;
        rec_data->padding[rec_data->depth] = '\0';
    }

    ret = unspace_rec(pname, options, dirfd, d->d_name, rec_data);

    if (options->verbose) {
        rec_data->padding[rec_data->depth] = ' ';
        rec_data->depth--;
        rec_data->padding[rec_data->depth] = '\0';
    }
    return ret;
}

// "unspaces" `entry` located in `current_dirfd`
// a non-directory `entry` is immediately renamed by calling `unspace`
// a directory `entry` will be opened, and its subentries are looped over:
//  each subentry is going to be fed into `unspace_rec_direntry`
// after looping on subentries, the directory itself is unspaced.
int unspace_rec(
    char const *pname,
    struct unspace_options *options,
    int current_dirfd,
    char const *entry,
    struct unspace_rec_data *rec_data
)
{
    int ret = 0;
    int subdirfd = openat(current_dirfd, entry, O_RDONLY | O_DIRECTORY | O_NOFOLLOW);
    char *getdents_buf;
    ssize_t nread;

    if (subdirfd < 0 && (errno == ENOTDIR || errno == ELOOP)) {
        return unspace(pname, options, current_dirfd, entry, rec_data);
    }
    if (subdirfd < 0) {
        fprintf(stderr,
                "%s:%s found weird or invalid entry '%s/%s' (might be an implementation mistake): '%s'\n",
                pname, rec_data->padding, get_fd_filename(current_dirfd), entry, strerror(errno));
        return 1;
    }
    if (options->verbose) {
        printf("%s:%s recursing into '%s'\n", pname, rec_data->padding, entry);
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
            unspace_rec_direntry(pname, options, subdirfd, d, rec_data);
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

    return ret | unspace(pname, options, current_dirfd, entry, rec_data);
}

int main(int argc, char **argv)
{
    struct unspace_input input = {
        .o.verbose = false,
        .o.replace = '_',
        .o.recursive = false,
        .o.dry_run = false,
        .files = NULL,
        .filec = 0,
    };
    struct unspace_rec_data rec_data = {
        .padding = {0},
        .depth = 0,
    };
    int ret = 0;

    if (read_cli(argc, argv, &input)) {
        return 1;
    }
    if (input.dump_input) {
        show_inputs(&input);
    }

    int (*unsp)(char const *, struct unspace_options *, int, char const *, struct unspace_rec_data *) = input.o.recursive
        ? &unspace_rec
        : &unspace
    ;

    for (size_t i = 0; i < input.filec; i++) {
        ret |= unsp(argv[0], &input.o, AT_FDCWD, input.files[i], &rec_data);
    }

    return ret;
}
