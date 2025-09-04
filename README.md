# `unspace`

Simple fix for when your coworkers send you files with spaces in the names, and it's annoying to write in CLIs.  

Just `unspace` them.

## Features

- Will never overwrite files (fails to rename instead)
- Very basic CLI, no complicated options
- Rename several files in a single invocation
- Rename recursively in directories
- Select a custom replacement character (underscore by default)
- Dry-run mode: print what would be done, but don't rename anything

## Usage

```sh
$ unspace 'file with spaces' # renames to file_with_spaces
$ unspace 'a a a' 'b b b' 'c c c' # several files at once
$ unspace -v 'a a' 'b b' # verbose: print renames
unspace: 'a a' -> 'a_a'
unspace: 'b b' -> 'b_b'
$ unspace -rv 'some directory' # rename in directories recursively, verbose log
unspace: recursing into 'some directory/'
unspace:  'other element' -> 'other_element'
unspace:  recursing into 'sub dir'
unspace:   'a b c' -> 'a_b_c'
unspace:   '1 2 3' -> '1_2_3'
unspace:  'sub dir' -> 'sub_dir'
unspace: 'some directory' -> some_directory
$ unspace -c - 'rename with dashes' # renames to 'rename-with-dashes'
$ unspace -d file1 -r file2 file3 -c- file4 # dump config options before processing
verbose: 0
recursive: 1
dump_input: 1
dry_run: 0
replace: -
files:
- file1
- file2
- file3
- file4
```

## Options

- `-r`: recursive mode: rename recursively in directory arguments
- `-v`: verbose mode: print each rename being made; tree-like log with `-rv`
- `-n`: dry-run: turn on `-v` and don't actually rename anything
- `-d`: dump CLI parsing before proceeding to rename arguments; use `-dn` to debug what renames will be done
- `-c <char>`: change the replacement character to `<char>` (instead of default `_`), must be single 1-byte character
