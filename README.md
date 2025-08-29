# `unspace`

Simple fix for when your coworkers send you files with spaces in the names, and it's annoying to write in CLIs.  

Just `unspace` them.

## Features

- Will never overwrite files (fails to rename instead)
- Very basic CLI, no complicated options
- Rename several files in a single invocation
- Rename recursively in directories
- Select a custom replacement character (underscore by default)

## Usage

```sh
$ unspace 'file with spaces' # renames to file_with_spaces
$ unspace 'a a a' 'b b b' 'c c c' # several files at once
$ unspace -v 'a a' 'b b' # verbose: print renames
unspace: 'a a' -> 'a_a'
unspace: 'b b' -> 'b_b'
$ unspace -rv 'some directory' # rename in directories recursively
unspace: 'some directory/sub dir/a b c' -> 'some directory/sub dir/a_b_c'
unspace: 'some directory/sub dir/1 2 2' -> 'some directory/sub dir/1_2_3'
unspace: 'some directory/sub dir' -> 'some directory/sub_dir'
unspace: 'some directory/other element' -> 'some directory/other_element'
unspace: 'some directory' -> 'some_directory'
$ unspace -c - 'rename with dashes' # renames to 'rename-with-dashes'
$ unspace -d file1 -r file2 file3 -c- file4 # dump config options before processing
verbose: 0
recursive: 1
dump_options: 1
replace: -
files:
- file1
- file2
- file3
- file4
```
