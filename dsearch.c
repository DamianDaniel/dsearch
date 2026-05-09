/*
 * Copyright (c) 2026 Damian Daiel <damian@danielovci.net>
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

#include <sys/types.h>
#include <sys/stat.h>

#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *needle = NULL;
static int        found   = 0;

static void search(const char *dirpath);

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
			puts(path);
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

	return 0;
}
