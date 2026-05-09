/*
 * Copyright (c) 2026 Damian Daniel <damian@danielovci.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * dsearch - recursive file name search
 * Usage: dsearch FOLDER NAME
 *
 * Searches FOLDER and all subdirectories for files whose names
 * contain NAME (case-sensitive substring match).
 *
 * Compiles cleanly on OpenBSD, macOS, and Linux:
 *   cc -std=c99 -Wall -Wextra -o dsearch dsearch.c
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ANSI_YELLOW "\033[1;33m"
#define ANSI_RESET  "\033[0m"

static const char *needle = NULL;
static int        found   = 0;

/* Forward declaration */
static void search(const char *dirpath);

/*
 * Print the path with the matching substring highlighted in yellow.
 */
static void
print_match(const char *path)
{
	size_t      needlelen = strlen(needle);
	const char *p         = path;
	const char *m;

	while ((m = strstr(p, needle)) != NULL) {
		fwrite(p, 1, (size_t)(m - p), stdout);
		fputs(ANSI_YELLOW, stdout);
		fwrite(m, 1, needlelen, stdout);
		fputs(ANSI_RESET, stdout);
		p = m + needlelen;
	}
	fputs(p, stdout);
	fputc('\n', stdout);
}

static void
search(const char *dirpath)
{
	DIR           *dp;
	struct dirent *de;
	struct stat    st;
	char           path[4096];
	size_t         dirlen;

	dp = opendir(dirpath);
	if (dp == NULL) {
		if (errno == EACCES)
			fprintf(stderr, "dsearch: permission denied: %s\n",
			        dirpath);
		else
			warn("opendir: %s", dirpath);
		return;
	}

	dirlen = strlen(dirpath);

	while ((de = readdir(dp)) != NULL) {
		if (strcmp(de->d_name, ".") == 0 ||
		    strcmp(de->d_name, "..") == 0)
			continue;

		if (dirlen + 1 + strlen(de->d_name) + 1 > sizeof(path)) {
			warnx("path too long, skipping: %s/%s",
			      dirpath, de->d_name);
			continue;
		}
		snprintf(path, sizeof(path), "%s/%s", dirpath, de->d_name);

		if (strstr(de->d_name, needle) != NULL) {
			print_match(path);
			found++;
		}

		if (stat(path, &st) == -1) {
			warn("stat: %s", path);
			continue;
		}
		if (S_ISDIR(st.st_mode))
			search(path);
	}

	closedir(dp);
}

int
main(int argc, char *argv[])
{
	if (argc != 3) {
		fprintf(stderr, "usage: dsearch folder name\n");
		return 1;
	}

	const char *folder = argv[1];
	needle             = argv[2];

	struct stat st;
	if (stat(folder, &st) == -1)
		err(1, "%s", folder);
	if (!S_ISDIR(st.st_mode))
		errx(1, "%s: not a directory", folder);

	search(folder);

	if (found == 0) {
		fprintf(stderr, "dsearch: no matches found for '%s' in '%s'\n",
		        needle, folder);
		return 1;
	}

	fprintf(stderr, "\ndsearch: %d match%s found\n",
	        found, found == 1 ? "" : "es");
	return 0;
}
