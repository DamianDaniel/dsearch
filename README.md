# dsearch

Recursively search a directory for files by name.

## Usage

    dsearch FOLDER NAME

Searches FOLDER and all subdirectories for files whose names contain NAME.

## Example

    dsearch /home/user/documents report

Prints the full path of every file or directory whose name contains "report".

## Install

### OpenBSD

    pkg_add dsearch

### Build from source

    cc -std=c99 -Wall -o dsearch dsearch.c
    install -m 755 dsearch /usr/local/bin/
    install -m 644 dsearch.1 /usr/local/man/man1/

## License

ISC — see dsearch.c for full text.
