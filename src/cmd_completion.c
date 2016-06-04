/* vifm
 * Copyright (C) 2001 Ken Steen.
 * Copyright (C) 2012 xaizek.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "cmd_completion.h"

#ifdef _WIN32
#define REQUIRED_WINVER 0x0500 /* To get GetFileSizeEx() function. */
#include "utils/windefs.h"
#include <windows.h>
#include <lm.h>
#endif

#include <sys/stat.h> /* stat */
#include <dirent.h> /* DIR dirent */

#ifndef _WIN32
#include <grp.h> /* getgrent setgrent */
#include <pwd.h> /* getpwent setpwent */
#endif

#include <assert.h> /* assert() */
#include <stddef.h> /* NULL size_t */
#include <stdlib.h> /* free() */
#include <stdio.h> /* snprintf() */
#include <string.h> /* memcpy() strdup() strlen() strncasecmp() strncmp()
                       strrchr() */

#include "cfg/config.h"
#include "compat/fs_limits.h"
#include "compat/os.h"
#include "engine/abbrevs.h"
#include "engine/cmds.h"
#include "engine/completion.h"
#include "engine/functions.h"
#include "engine/options.h"
#include "engine/variables.h"
#include "int/file_magic.h"
#include "int/path_env.h"
#ifdef _WIN32
#include "menus/menus.h"
#endif
#include "modes/dialogs/msg_dialog.h"
#include "ui/color_scheme.h"
#include "ui/colors.h"
#include "ui/statusbar.h"
#include "utils/fs.h"
#include "utils/macros.h"
#include "utils/path.h"
#include "utils/str.h"
#include "utils/utils.h"
#include "bmarks.h"
#include "cmd_core.h"
#include "filelist.h"
#include "filetype.h"
#include "tags.h"

static int cmd_ends_with_space(const char *cmd);
static void complete_colorscheme(const char *str, size_t arg_num);
static void complete_selective_sync(const char str[]);
static void complete_help(const char *str);
static void complete_history(const char str[]);
static void complete_invert(const char str[]);
static int complete_chown(const char *str);
static void complete_filetype(const char *str);
static void complete_progs(const char *str, assoc_records_t records);
static void complete_highlight_groups(const char *str);
static int complete_highlight_arg(const char *str);
static void complete_envvar(const char str[]);
static void complete_winrun(const char str[]);
static void complete_from_string_list(const char str[], const char *items[][2],
		size_t item_count, int ignore_case);
static void complete_command_name(const char beginning[]);
static void filename_completion_in_dir(const char *path, const char *str,
		CompletionType type);
static void filename_completion_internal(DIR *dir, const char dirname[],
		const char filename[], CompletionType type);
static int is_dirent_targets_exec(const struct dirent *d);
#ifdef _WIN32
static void complete_with_shared(const char *server, const char *file);
#endif

int
complete_args(int id, const cmd_info_t *cmd_info, int arg_pos, void *extra_arg)
{
	/* TODO: Refactor this function complete_args() */

	const CompletionPreProcessing cpp = (CompletionPreProcessing)extra_arg;

	const char *const args = cmd_info->args;
	int argc = cmd_info->argc;
	char **const argv = cmd_info->argv;

	const char *arg = after_last(args, ' ');
	const char *start = arg;
	const char *slash = strrchr(args + arg_pos, '/');
	const char *dollar = strrchr(arg, '$');
	const char *ampersand = strrchr(arg, '&');

	if(id == COM_SET || id == COM_SETLOCAL)
	{
		complete_options(args, &start, (id == COM_SET) ? OPT_GLOBAL : OPT_LOCAL);
	}
	else if(id == COM_CABBR)
	{
		vle_abbr_complete(args);
		start = args;
	}
	else if(command_accepts_expr(id))
	{
		if(ampersand > dollar)
		{
			OPT_SCOPE scope = OPT_GLOBAL;
			start = ampersand + 1;

			if(starts_with_lit(start, "l:"))
			{
				scope = OPT_LOCAL;
				start += 2;
			}
			else if(starts_with_lit(start, "g:"))
			{
				start += 2;
			}

			complete_real_option_names(start, scope);
		}
		else if(dollar == NULL)
		{
			function_complete_name(arg, &start);
		}
		else
		{
			complete_variables((dollar > arg) ? dollar : arg, &start);
		}
	}
	else if(id == COM_UNLET)
		complete_variables(arg, &start);
	else if(id == COM_HELP)
		complete_help(args);
	else if(id == COM_HISTORY)
	{
		complete_history(args);
		start = args;
	}
	else if(id == COM_INVERT)
	{
		complete_invert(args);
		start = args;
	}
	else if(id == COM_CHOWN)
		start += complete_chown(args);
	else if(id == COM_FILE)
		complete_filetype(args);
	else if(id == COM_HIGHLIGHT)
	{
		if(argc == 0 || (argc == 1 && !cmd_ends_with_space(args)))
			complete_highlight_groups(args);
		else
			start += complete_highlight_arg(arg);
	}
	else if((id == COM_CD || id == COM_PUSHD || id == COM_EXECUTE ||
			id == COM_SOURCE || id == COM_EDIT) && dollar != NULL && dollar > slash)
	{
		start = dollar + 1;
		complete_envvar(start);
	}
	else if(id == COM_SELECT)
	{
		cmd_info_t exec_info = *cmd_info;
		char *exec_argv[exec_info.argc];

		/* Make sure that it's a filter-argument. */
		if(args[0] != '!' || char_is_one_of("/{", cmd_info->args[1]))
		{
			return start - args;
		}

		/* Fake !-command completion by hiding "!" in front and calling this
		 * function again. */
		++exec_info.args;
		if(exec_info.args[0] == '\0')
		{
			exec_info.argc = 0;
		}

		exec_info.argv = exec_argv;
		memcpy(exec_argv, argv, sizeof(exec_argv));
		++exec_argv[0];

		if(arg_pos != 0)
		{
			--arg_pos;
		}

		return complete_args(COM_EXECUTE, &exec_info, arg_pos, extra_arg) + 1;
	}
	else if(id == COM_WINDO)
		;
	else if(id == COM_WINRUN)
	{
		if(argc == 0)
			complete_winrun(args);
	}
	else if(id == COM_AUTOCMD)
	{
		/* Complete only first argument. */
		if(argc <= 1 && !cmd_ends_with_space(args))
		{
			static const char *events[][2] = {
				{ "DirEnter", "occurs on directory change" },
			};
			complete_from_string_list(args, events, ARRAY_LEN(events), 1);
		}
	}
	else
	{
		char *free_me = NULL;
		size_t arg_num = argc;
		start = slash;
		if(start == NULL)
			start = args + arg_pos;
		else
			start++;

		if(argc > 0 && !cmd_ends_with_space(args))
		{
			if(ends_with(args, "\""))
			{
				return start - args;
			}
			if(ends_with(args, "'"))
			{
				return start - args;
			}
			arg_num = argc - 1;
			arg = argv[arg_num];
		}

		/* Pre-process input with requested method. */
		if(cpp != CPP_NONE)
		{
			if(cpp != CPP_PERCENT_UNESCAPE)
			{
				arg = args + arg_pos + 1;
				start = (slash == NULL) ? arg : (slash + 1);
			}

			free_me = strdup(arg);
			arg = free_me;

			switch(cpp)
			{
				case CPP_PERCENT_UNESCAPE: expand_percent_escaping(free_me); break;
				case CPP_SQUOTES_UNESCAPE: expand_squotes_escaping(free_me); break;
				case CPP_DQUOTES_UNESCAPE: expand_dquotes_escaping(free_me); break;

				default:
					assert(0 && "Unhandled preprocessing type.");
					break;
			};
		}

		if(id == COM_COLORSCHEME)
		{
			complete_colorscheme(arg, arg_num);
		}
		else if(id == COM_SYNC)
		{
			if(cmd_info->emark)
			{
				complete_selective_sync(arg);
			}
			else
			{
				filename_completion(arg, CT_DIRONLY);
			}
		}
		else if(id == COM_BMARKS)
		{
			if(cmd_info->emark && argc == 1)
			{
				filename_completion(arg, CT_ALL);
			}
			else
			{
				bmarks_complete(argc, argv, arg);
			}
		}
		else if(id == COM_DELBMARKS)
		{
			if(cmd_info->emark)
			{
				filename_completion(arg, CT_ALL);
			}
			else
			{
				bmarks_complete(argc, argv, arg);
			}
		}
		else if(id == COM_CD || id == COM_PUSHD || id == COM_MKDIR)
		{
			filename_completion(arg, CT_DIRONLY);
		}
		else if(id == COM_COPY || id == COM_MOVE || id == COM_ALINK ||
				id == COM_RLINK)
		{
			filename_completion_in_dir(other_view->curr_dir, arg, CT_ALL);
		}
		else if(id == COM_SPLIT || id == COM_VSPLIT)
		{
			filename_completion_in_dir(flist_get_dir(curr_view), arg, CT_DIRONLY);
		}
		else if(id == COM_FIND)
		{
			if(argc == 1 && !cmd_ends_with_space(args))
				filename_completion(arg, CT_DIRONLY);
		}
		else if(id == COM_EXECUTE)
		{
			if(argc == 0 || (argc == 1 && !cmd_ends_with_space(args)))
			{
				if(*arg == '.' || *arg == '~' || is_path_absolute(arg))
					filename_completion(arg, CT_DIREXEC);
				else
					complete_command_name(arg);
			}
			else
				filename_completion(arg, CT_ALL);
		}
		else if(id == COM_TOUCH || id == COM_RENAME)
		{
			filename_completion(arg, CT_ALL_WOS);
		}
		else
		{
			filename_completion(arg, CT_ALL);
		}

		free(free_me);
	}

	return start - args;
}

static int
cmd_ends_with_space(const char *cmd)
{
	while(cmd[0] != '\0' && cmd[1] != '\0')
	{
		if(cmd[0] == '\\')
			cmd++;
		cmd++;
	}
	return cmd[0] == ' ';
}

static void
complete_colorscheme(const char *str, size_t arg_num)
{
	if(arg_num == 0)
	{
		cs_complete(str);
	}
	else if(arg_num == 1)
	{
		filename_completion(str, CT_DIRONLY);
	}
}

/* Completes properties for selective synchronization. */
static void
complete_selective_sync(const char str[])
{
	static const char *lines[][2] = {
		{ "location",  "current directory" },
		{ "cursorpos", "cursor position" },
		{ "localopts", "values of view-specific options" },
		{ "filters",   "all filters" },
		{ "filelist",  "list of files for custom views" },
		{ "all",       "as much as possible" },
	};

	complete_from_string_list(str, lines, ARRAY_LEN(lines), 0);
}

static void
complete_help(const char *str)
{
	int i;

	if(!cfg.use_vim_help)
	{
		return;
	}

	for(i = 0; tags[i] != NULL; i++)
	{
		if(strstr(tags[i], str) != NULL)
		{
			vle_compl_add_match(tags[i], "");
		}
	}
	vle_compl_finish_group();
	vle_compl_add_last_match(str);
}

/* Completes available kinds of histories. */
static void
complete_history(const char str[])
{
	static const char *lines[][2] = {
		{ ".",       "directory visit history" },
		{ "dir",     "directory visit history" },

		{ "@",       "prompt input (e.g. file name)" },
		{ "input",   "prompt input (e.g. file name)" },

		{ "/",       "search patterns (forward)" },
		{ "search",  "search patterns (forward)" },
		{ "fsearch", "search patterns (forward)" },

		{ "?",       "search patterns (backward)" },
		{ "bsearch", "search patterns (backward)" },

		{ ":",       "command-line commands" },
		{ "cmd",     "command-line commands" },

		{ "=",       "local filter patterns" },
		{ "filter",  "local filter patterns" },
	};
	complete_from_string_list(str, lines, ARRAY_LEN(lines), 0);
}

/* Completes available inversion kinds. */
static void
complete_invert(const char str[])
{
	static const char *lines[][2] = {
		{ "f", "filter state" },
		{ "s", "file selection" },
		{ "o", "primary sorting key" },
	};
	complete_from_string_list(str, lines, ARRAY_LEN(lines), 0);
}

/* Completes either user or group name for the :chown command. */
static int
complete_chown(const char *str)
{
#ifndef _WIN32
	char *colon = strchr(str, ':');
	if(colon == NULL)
	{
		complete_user_name(str);
		return 0;
	}
	else
	{
		complete_group_name(colon + 1);
		return colon - str + 1;
	}
#else
	vle_compl_add_last_match(str);
	return 0;
#endif
}

static void
complete_filetype(const char *str)
{
	char *const typed_fname = get_typed_entry_fpath(get_current_entry(curr_view));
	assoc_records_t ft = ft_get_all_programs(typed_fname);

	complete_progs(str, ft);
	ft_assoc_records_free(&ft);

	complete_progs(str, get_magic_handlers(typed_fname));

	vle_compl_finish_group();
	vle_compl_add_last_match(str);

	free(typed_fname);
}

static void
complete_progs(const char *str, assoc_records_t records)
{
	int i;
	const size_t len = strlen(str);

	for(i = 0; i < records.count; i++)
	{
		const assoc_record_t *const assoc = &records.list[i];
		char cmd[NAME_MAX];

		(void)extract_cmd_name(assoc->command, 1, sizeof(cmd), cmd);

		if(strnoscmp(cmd, str, len) == 0)
		{
			vle_compl_put_match(escape_chars(cmd, "|"), assoc->description);
		}
	}
}

/* Completes highlight groups and subcommand "clear" for :highlight command. */
static void
complete_highlight_groups(const char *str)
{
	int i;
	const size_t len = strlen(str);
	for(i = 0; i < MAXNUM_COLOR; ++i)
	{
		if(strncasecmp(str, HI_GROUPS[i], len) == 0)
		{
			vle_compl_add_match(HI_GROUPS[i], HI_GROUPS_DESCR[i]);
		}
	}
	if(strncmp(str, "clear", len) == 0)
	{
		vle_compl_add_match("clear", "restore default colors");
	}
	vle_compl_finish_group();
	vle_compl_add_last_match(str);
}

static int
complete_highlight_arg(const char *str)
{
	/* TODO: Refactor this function complete_highlight_arg() */
	const char *equal = strchr(str, '=');
	int result = (equal == NULL) ? 0 : (equal - str + 1);
	size_t len = strlen((equal == NULL) ? str : ++equal);
	if(equal == NULL)
	{
		static const char *const args[][2] = {
			{ "cterm",   "text attributes" },
			{ "ctermfg", "foreground color" },
			{ "ctermbg", "background color" },
		};

		size_t i;

		for(i = 0U; i < ARRAY_LEN(args); ++i)
		{
			if(strncmp(str, args[i][0], len) == 0)
			{
				vle_compl_add_match(args[i][0], args[i][1]);
			}
		}
	}
	else
	{
		if(strncmp(str, "cterm", equal - str - 1) == 0)
		{
			static const char *const STYLES[][2] = {
				{ "bold",      "bold text, lighter color" },
				{ "underline", "underlined text" },
				{ "reverse",   "reversed colors" },
				{ "inverse",   "reversed colors" },
				{ "standout",  "like bold or similar to it" },
				{ "none",      "no attributes" },
			};

			size_t i;

			const char *const comma = strrchr(equal, ',');
			if(comma != NULL)
			{
				result += comma - equal + 1;
				equal = comma + 1;
				len = strlen(equal);
			}

			for(i = 0U; i < ARRAY_LEN(STYLES); ++i)
			{
				if(strncasecmp(equal, STYLES[i][0], len) == 0)
				{
					vle_compl_add_match(STYLES[i][0], STYLES[i][1]);
				}
			}
		}
		else
		{
			size_t i;

			if(strncasecmp(equal, "default", len) == 0)
			{
				vle_compl_add_match("default", "default or transparent color");
			}
			if(strncasecmp(equal, "none", len) == 0)
			{
				vle_compl_add_match("none", "no specific attributes");
			}

			for(i = 0U; i < ARRAY_LEN(XTERM256_COLOR_NAMES); ++i)
			{
				if(strncasecmp(equal, XTERM256_COLOR_NAMES[i], len) == 0)
				{
					vle_compl_add_match(XTERM256_COLOR_NAMES[i], "");
				}
			}
			for(i = 0U; i < ARRAY_LEN(LIGHT_COLOR_NAMES); ++i)
			{
				if(strncasecmp(equal, LIGHT_COLOR_NAMES[i], len) == 0)
				{
					vle_compl_add_match(LIGHT_COLOR_NAMES[i], "");
				}
			}
		}
	}
	vle_compl_finish_group();
	vle_compl_add_last_match((equal == NULL) ? str : equal);
	return result;
}

/* Completes name of the environment variables. */
static void
complete_envvar(const char str[])
{
	extern char **environ;
	char **p = environ;
	const size_t len = strlen(str);

	while(*p != NULL)
	{
		if(strncmp(*p, str, len) == 0)
		{
			char *const equal = strchr(*p, '=');
			/* Actually equal shouldn't be NULL unless environ content is corrupted.
			 * But the check below won't harm. */
			if(equal != NULL)
			{
				*equal = '\0';
				vle_compl_add_match(*p, equal + 1);
				*equal = '=';
			}
		}
		p++;
	}

	vle_compl_finish_group();
	vle_compl_add_last_match(str);
}

/* Completes first :winrun argument. */
static void
complete_winrun(const char str[])
{
	static const char *win_marks[][2] = {
		{ "^", "left/top view" },
		{ "$", "right/bottom view" },
		{ "%", "both views" },
		{ ".", "active view" },
		{ ",", "inactive view" },
	};
	complete_from_string_list(str, win_marks, ARRAY_LEN(win_marks), 0);
}

/* Performs str completion using items in the list of length item_count. */
static void
complete_from_string_list(const char str[], const char *items[][2],
		size_t item_count, int ignore_case)
{
	size_t i;
	const size_t prefix_len = strlen(str);
	for(i = 0U; i < item_count; ++i)
	{
		const int cmp = ignore_case
		              ? strncasecmp(str, items[i][0], prefix_len)
		              : strncmp(str, items[i][0], prefix_len);

		if(cmp == 0)
		{
			vle_compl_add_match(items[i][0], items[i][1]);
		}
	}
	vle_compl_finish_group();
	vle_compl_add_last_match(str);
}

char *
fast_run_complete(const char cmd[])
{
	char *result = NULL;
	const char *args;
	char command[NAME_MAX];
	char *completed;

	args = extract_cmd_name(cmd, 0, sizeof(command), command);

	if(is_path_absolute(command))
	{
		return strdup(cmd);
	}

	vle_compl_reset();
	complete_command_name(command);
	vle_compl_unite_groups();
	completed = vle_compl_next();

	if(vle_compl_get_count() > 2)
	{
		int c = vle_compl_get_count() - 1;
		while(c-- > 0)
		{
			if(stroscmp(command, completed) == 0)
			{
				result = strdup(cmd);
				break;
			}
			else
			{
				free(completed);
				completed = vle_compl_next();
			}
		}

		if(result == NULL)
		{
			status_bar_error("Command beginning is ambiguous");
		}
	}
	else
	{
		free(completed);
		completed = vle_compl_next();
		result = format_str("%s %s", completed, args);
	}
	free(completed);

	return result;
}

/* Fills list of completions with executables in $PATH. */
static void
complete_command_name(const char beginning[])
{
	size_t i;
	char ** paths;
	size_t paths_count;
	char *const cwd = save_cwd();

	paths = get_paths(&paths_count);
	for(i = 0U; i < paths_count; ++i)
	{
		if(vifm_chdir(paths[i]) == 0)
		{
			filename_completion(beginning, CT_EXECONLY);
		}
	}
	vle_compl_add_last_path_match(beginning);

	restore_cwd(cwd);
}

static void
filename_completion_in_dir(const char *path, const char *str,
		CompletionType type)
{
	char buf[PATH_MAX];
	if(is_root_dir(str))
	{
		snprintf(buf, sizeof(buf), "%s", str);
	}
	else
	{
		snprintf(buf, sizeof(buf), "%s/%s", path, str);
	}
	filename_completion(buf, type);
}

/*
 * type: CT_*
 */
void
filename_completion(const char *str, CompletionType type)
{
	/* TODO refactor filename_completion(...) function */
	DIR *dir;
	char *dirname;
	char *filename;
	char *temp;
	char *cwd;

	if(str[0] == '~' && strchr(str, '/') == NULL)
	{
		vle_compl_put_path_match(expand_tilde(str));
		return;
	}

	dirname = expand_tilde(str);
	filename = strdup(dirname);

	temp = cmds_expand_envvars(dirname);
	free(dirname);
	dirname = temp;

	temp = strrchr(dirname, '/');
	if(temp != NULL && type != CT_FILE && type != CT_FILE_WOE)
	{
		strcpy(filename, ++temp);
		*temp = '\0';
	}
	else if(replace_string(&dirname, ".") != 0)
	{
		free(filename);
		free(dirname);
		return;
	}

#ifdef _WIN32
	if(is_unc_root(dirname) ||
			(stroscmp(dirname, ".") == 0 && is_unc_root(curr_view->curr_dir)) ||
			(stroscmp(dirname, "/") == 0 && is_unc_path(curr_view->curr_dir)))
	{
		char buf[PATH_MAX];
		if(!is_unc_root(dirname))
			snprintf(buf,
					strchr(curr_view->curr_dir + 2, '/') - curr_view->curr_dir + 1, "%s",
					curr_view->curr_dir);
		else
			snprintf(buf, sizeof(buf), "%s", dirname);

		complete_with_shared(buf, filename);
		free(filename);
		free(dirname);
		return;
	}
	if(is_unc_path(curr_view->curr_dir))
	{
		char buf[PATH_MAX];
		if(is_path_absolute(dirname) && !is_unc_root(curr_view->curr_dir))
			snprintf(buf,
					strchr(curr_view->curr_dir + 2, '/') - curr_view->curr_dir + 2, "%s",
					curr_view->curr_dir);
		else
			snprintf(buf, sizeof(buf), "%s", curr_view->curr_dir);
		strcat(buf, dirname);
		chosp(buf);
		(void)replace_string(&dirname, buf);
	}
#endif

	dir = os_opendir(dirname);

	cwd = save_cwd();

	if(dir == NULL || vifm_chdir(dirname) != 0)
	{
		vle_compl_add_path_match(filename);
	}
	else
	{
		filename_completion_internal(dir, dirname, filename, type);
		(void)vifm_chdir(flist_get_dir(curr_view));
	}

	free(filename);
	free(dirname);

	if(dir != NULL)
	{
		os_closedir(dir);
	}

	restore_cwd(cwd);
}

static void
filename_completion_internal(DIR *dir, const char dirname[],
		const char filename[], CompletionType type)
{
	struct dirent *d;

	size_t filename_len = strlen(filename);
	while((d = os_readdir(dir)) != NULL)
	{
		if(filename[0] == '\0' && d->d_name[0] == '.')
			continue;
		if(strnoscmp(d->d_name, filename, filename_len) != 0)
			continue;

		if(type == CT_DIRONLY && !is_dirent_targets_dir(d))
			continue;
		else if(type == CT_EXECONLY && !is_dirent_targets_exec(d))
			continue;
		else if(type == CT_DIREXEC && !is_dirent_targets_dir(d) &&
				!is_dirent_targets_exec(d))
			continue;

		if(is_dirent_targets_dir(d) && type != CT_ALL_WOS)
		{
			vle_compl_put_path_match(format_str("%s/", d->d_name));
		}
		else
		{
			vle_compl_add_path_match(d->d_name);
		}
	}

	vle_compl_finish_group();
	if(type != CT_EXECONLY)
	{
		vle_compl_add_last_path_match(filename);
	}
}

/* Uses dentry to check file type.  Returns non-zero for directories,
 * otherwise zero is returned.  Symbolic links are dereferenced. */
static int
is_dirent_targets_exec(const struct dirent *d)
{
#ifndef _WIN32
	if(d->d_type == DT_DIR)
		return 0;
	if(d->d_type == DT_LNK && get_symlink_type(d->d_name) != SLT_UNKNOWN)
		return 0;
	return os_access(d->d_name, X_OK) == 0;
#else
	return is_win_executable(d->d_name);
#endif
}

#ifndef _WIN32

void
complete_user_name(const char *str)
{
	struct passwd *pw;
	size_t len;

	len = strlen(str);
	setpwent();
	while((pw = getpwent()) != NULL)
	{
		if(strncmp(pw->pw_name, str, len) == 0)
		{
			vle_compl_add_match(pw->pw_name, "");
		}
	}
	vle_compl_finish_group();
	vle_compl_add_last_match(str);
}

void
complete_group_name(const char *str)
{
	struct group *gr;
	size_t len = strlen(str);

	setgrent();
	while((gr = getgrent()) != NULL)
	{
		if(strncmp(gr->gr_name, str, len) == 0)
		{
			vle_compl_add_match(gr->gr_name, "");
		}
	}
	vle_compl_finish_group();
	vle_compl_add_last_match(str);
}

#else

static void
complete_with_shared(const char *server, const char *file)
{
	NET_API_STATUS res;
	size_t len = strlen(file);

	do
	{
		PSHARE_INFO_502 buf_ptr;
		DWORD er = 0, tr = 0, resume = 0;
		wchar_t *wserver = to_wide(server + 2);

		if(wserver == NULL)
		{
			show_error_msg("Memory Error", "Unable to allocate enough memory");
			return;
		}

		res = NetShareEnum(wserver, 502, (LPBYTE *)&buf_ptr, -1, &er, &tr, &resume);
		free(wserver);
		if(res == ERROR_SUCCESS || res == ERROR_MORE_DATA)
		{
			PSHARE_INFO_502 p;
			DWORD i;

			p = buf_ptr;
			for(i = 1; i <= er; i++)
			{
				char buf[512];
				WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)p->shi502_netname, -1, buf,
						sizeof(buf), NULL, NULL);
				strcat(buf, "/");
				if(strnoscmp(buf, file, len) == 0)
				{
					vle_compl_put_match(shell_like_escape(buf, 1), "");
				}
				p++;
			}
			NetApiBufferFree(buf_ptr);
		}
	}
	while(res == ERROR_MORE_DATA);
}

#endif

int
external_command_exists(const char cmd[])
{
	char path[PATH_MAX];

	if(get_cmd_path(cmd, sizeof(path), path) == 0)
	{
		return executable_exists(path);
	}
	return 0;
}

int
get_cmd_path(const char cmd[], size_t path_len, char path[])
{
	if(starts_with(cmd, "!!"))
	{
		cmd += 2;
	}

	if(contains_slash(cmd))
	{
		copy_str(path, path_len, cmd);
		return 0;
	}
	else
	{
		return find_cmd_in_path(cmd, path_len, path);
	}
}

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab cinoptions-=(0 : */
/* vim: set cinoptions+=t0 filetype=c : */
