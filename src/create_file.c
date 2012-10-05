// Copyright 2007-2008 Ben Hutchings.
// Copyright 2008 Petter Reinholdtsen.
// See the file "COPYING" for licence details.

// Operation to create a file for writing. Used by dvsink-files.c

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>

int create_file(const char * format, char ** name)
{
    time_t now;
    struct tm now_local;
    size_t name_buf_len = 200, name_len;
    char * name_buf = 0;
    int file;

    now = time(0);
    localtime_r(&now, &now_local);

    // Allocate a name buffer and generate the name in it, leaving room
    // for a suffix.
    for (;;)
    {
	name_buf = realloc(name_buf, name_buf_len);
	if (!name_buf)
	{
	    perror("realloc");
	    exit(1);
	}
	name_len = strftime(name_buf, name_buf_len - 20,
			    format, &now_local);
	if (name_len > 0)
	    break;

	// Try a bigger buffer.
	name_buf_len *= 2;
    }

    // Add ".dv" extension if missing.  Add distinguishing
    // number before it if necessary to avoid collision.
    // Create parent directories as necessary.
    int suffix_num = 0;
    if (name_len <= 3 || strcmp(name_buf + name_len - 3, ".dv") != 0)
	strcpy(name_buf + name_len, ".dv");
    else
	name_len -= 3;
    for (;;)
    {
	file = open(name_buf, O_CREAT | O_EXCL | O_WRONLY, 0666);
	if (file >= 0)
	{
	    *name = name_buf;
	    return file;
	}
	else if (errno == EEXIST)
	{
	    // Name collision; try changing the suffix
	    sprintf(name_buf + name_len, "-%d.dv", ++suffix_num);
	}
	else if (errno == ENOENT)
	{
	    // Parent directory missing
	    char * p = name_buf + 1;
	    while ((p = strchr(p, '/')))
	    {
		*p = 0;
		if (mkdir(name_buf, 0777) < 0 && errno != EEXIST)
		{
		    fprintf(stderr, "ERROR: mkdir %s: %s\n",
			    name_buf, strerror(errno));
		    exit(1);
		}
		*p++ = '/';
	    }
	}
	else
	{
	    fprintf(stderr, "ERROR: open %s: %s\n",
		    name_buf, strerror(errno));
	    exit(1);
	}
    }

    *name = name_buf;
    return file;
}
