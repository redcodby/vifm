/* vifm
 * Copyright (C) 2001 Ken Steen.
 * Copyright (C) 2011 xaizek.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#ifndef _WIN32
#include <grp.h>
#include <pwd.h>
#endif
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../config.h"

#include "config.h"
#include "filelist.h"
#include "file_magic.h"
#include "menus.h"
#include "modes.h"
#include "status.h"
#include "ui.h"
#include "utils.h"

/* Returns increment for curr_y */
static int
show_file_type(FileView *view, int curr_y)
{
	int x, y;
	int old_curr_y = curr_y;
	getmaxyx(menu_win, y, x);

	mvwaddstr(menu_win, curr_y, 2, "Type: ");
	if(view->dir_entry[view->list_pos].type == LINK)
	{
		char linkto[PATH_MAX + NAME_MAX];
		char *filename = view->dir_entry[view->list_pos].name;

		mvwaddstr(menu_win, curr_y, 8, "Link");
		curr_y += 2;
		mvwaddstr(menu_win, curr_y, 2, "Link To: ");

		if(get_link_target(filename, linkto, sizeof(linkto)) == 0)
		{
			mvwaddnstr(menu_win, curr_y, 11, linkto, x - 11);

			if(access(linkto, F_OK) != 0)
				mvwaddstr(menu_win, curr_y - 2, 12, " (BROKEN)");
		}
		else
		{
			mvwaddstr(menu_win, curr_y, 11, "Couldn't Resolve Link");
		}
	}
	else if(view->dir_entry[view->list_pos].type == EXECUTABLE ||
			view->dir_entry[view->list_pos].type == REGULAR)
	{
#ifdef HAVE_FILE_PROG
		FILE *pipe;
		char command[1024];
		char buf[NAME_MAX];

		/* Use the file command to get file information */
		snprintf(command, sizeof(command), "file \"%s\" -b",
				view->dir_entry[view->list_pos].name);

		if((pipe = popen(command, "r")) == NULL)
		{
			mvwaddstr(menu_win, curr_y, 8, "Unable to open pipe to read file");
			return 2;
		}

		if(fgets(buf, sizeof(buf), pipe) != buf)
			strcpy(buf, "Pipe read error");

		pclose(pipe);

		mvwaddnstr(menu_win, curr_y, 8, buf, x - 9);
		if(strlen(buf) > x - 9)
			mvwaddnstr(menu_win, curr_y + 1, 8, buf + x - 9, x - 9);
#else /* #ifdef HAVE_FILE_PROG */
		if(view->dir_entry[view->list_pos].type == EXECUTABLE)
			mvwaddstr(menu_win, curr_y, 8, "Executable");
		else
			mvwaddstr(menu_win, curr_y, 8, "Regular File");
#endif /* #ifdef HAVE_FILE_PROG */
	}
	else if(view->dir_entry[view->list_pos].type == DIRECTORY)
	{
	  mvwaddstr(menu_win, curr_y, 8, "Directory");
	}
	else if(S_ISCHR(view->dir_entry[view->list_pos].mode))
	{
	  mvwaddstr(menu_win, curr_y, 8, "Character Device");
	}
	else if(S_ISBLK(view->dir_entry[view->list_pos].mode))
	{
	  mvwaddstr(menu_win, curr_y, 8, "Block Device");
	}
	else if(view->dir_entry[view->list_pos].type == FIFO)
	{
	  mvwaddstr(menu_win, curr_y, 8, "Fifo Pipe");
	}
#ifndef _WIN32
	else if(S_ISSOCK(view->dir_entry[view->list_pos].mode))
	{
	  mvwaddstr(menu_win, curr_y, 8, "Socket");
	}
#endif
	else
	{
	  mvwaddstr(menu_win, curr_y, 8, "Unknown");
	}
	curr_y += 2;

	return curr_y - old_curr_y;
}

/* Returns increment for curr_y */
static int
show_mime_type(FileView *view, int curr_y)
{
	const char* mimetype = NULL;

	mimetype = get_mimetype(get_current_file_name(view));

	mvwaddstr(menu_win, curr_y, 2, "Mime Type: ");

	if(mimetype == NULL)
		mimetype = "Unknown";

	mvwaddstr(menu_win, curr_y, 13, mimetype);

	return 2;
}

void
redraw_full_file_properties(FileView *v)
{
	static FileView *view;

	char name_buf[NAME_MAX];
	char perm_buf[26];
	char size_buf[56];
	char uid_buf[26];
	char buf[256];
#ifndef _WIN32
	struct passwd *pwd_buf;
	struct group *grp_buf;
#endif
	struct tm *tm_ptr;
	int x, y;
	int curr_y;
	unsigned long long size;

	if(v != NULL)
		view = v;

	assert(view != NULL);

	setup_menu();

	getmaxyx(menu_win, y, x);
	wclear(menu_win);

	snprintf(name_buf, sizeof(name_buf), "%s",
			view->dir_entry[view->list_pos].name);

	size = 0;
	if(view->dir_entry[view->list_pos].type == DIRECTORY)
		tree_get_data(curr_stats.dirsize_cache,
				view->dir_entry[view->list_pos].name, &size);

	if(size == 0)
		size = view->dir_entry[view->list_pos].size;

	friendly_size_notation(size, sizeof(size_buf), size_buf);

#ifndef _WIN32
	if((pwd_buf = getpwuid(view->dir_entry[view->list_pos].uid)) == NULL)
	{
		snprintf(uid_buf, sizeof(uid_buf), "%d",
				(int) view->dir_entry[view->list_pos].uid);
	}
	else
	{
		snprintf(uid_buf, sizeof(uid_buf), "%s", pwd_buf->pw_name);
	}
#else
	snprintf(uid_buf, sizeof(uid_buf), "NOT AVAILABLE ON WINDOWS");
#endif
	get_perm_string(perm_buf, sizeof(perm_buf),
			view->dir_entry[view->list_pos].mode);

	curr_y = 2;
	mvwaddstr(menu_win, curr_y, 2, "File: ");
	mvwaddnstr(menu_win, curr_y, 8, name_buf, x - 8);
	curr_y += 2;
	mvwaddstr(menu_win, curr_y, 2, "Size: ");
	mvwaddstr(menu_win, curr_y, 8, size_buf);
	curr_y += 2;

	curr_y += show_file_type(view, curr_y);
	curr_y += show_mime_type(view, curr_y);

#ifndef _WIN32
	mvwaddstr(menu_win, curr_y, 2, "Permissions: ");
	mvwaddstr(menu_win, curr_y, 15, perm_buf);
	curr_y += 2;
#endif

	mvwaddstr(menu_win, curr_y, 2, "Modified: ");
	tm_ptr = localtime(&view->dir_entry[view->list_pos].mtime);
	strftime(buf, sizeof (buf), "%a %b %d %I:%M %p", tm_ptr);
	mvwaddstr(menu_win, curr_y, 13, buf);
	curr_y += 2;

	mvwaddstr(menu_win, curr_y, 2, "Accessed: ");
	tm_ptr = localtime(&view->dir_entry[view->list_pos].atime);
	strftime (buf, sizeof (buf), "%a %b %d %I:%M %p", tm_ptr);
	mvwaddstr(menu_win, curr_y, 13, buf);
	curr_y += 2;

	mvwaddstr(menu_win, curr_y, 2, "Changed: ");
	tm_ptr = localtime(&view->dir_entry[view->list_pos].ctime);
	strftime (buf, sizeof (buf), "%a %b %d %I:%M %p", tm_ptr);
	mvwaddstr(menu_win, curr_y, 13, buf);
	curr_y += 2;

#ifndef _WIN32
	mvwaddstr(menu_win, curr_y, 2, "Owner: ");
	mvwaddstr(menu_win, curr_y, 10, uid_buf);
	curr_y += 2;

	mvwaddstr(menu_win, curr_y, 2, "Group: ");
	if((grp_buf = getgrgid(view->dir_entry[view->list_pos].gid)) != NULL)
		mvwaddstr(menu_win, curr_y, 10, grp_buf->gr_name);
	mvwaddstr(menu_win, curr_y, 10, "NOT AVAILABLE ON WINDOWS");
#endif

	wnoutrefresh(menu_win);

	box(menu_win, 0, 0);
	wrefresh(menu_win);
}

void
show_full_file_properties(FileView *view)
{
	int key;

	curr_stats.show_full = 1;

	redraw_full_file_properties(view);

#ifdef ENABLE_EXTENDED_KEYS
	keypad(menu_win, TRUE);
#endif /* ENABLE_EXTENDED_KEYS */
  /* wait for Return or Ctrl-c or Esc or error */
	do
		key = wgetch(menu_win);
	while(key != 13 && key != 3 && key != 27 && key != ERR && key != 'q');

	werase(menu_win);

	curr_stats.show_full = 0;

	modes_redraw();
}

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab cinoptions-=(0 : */
/* vim: set cinoptions+=t0 : */
