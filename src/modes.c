#include <stdlib.h>

#include "background.h"
#include "file_info.h"
#include "filelist.h"
#include "keys.h"
#include "keys_buildin_c.h"
#include "keys_buildin_m.h"
#include "keys_buildin_n.h"
#include "keys_buildin_s.h"
#include "keys_buildin_v.h"
#include "status.h"
#include "ui.h"

#include "modes.h"

static int mode = NORMAL_MODE;

static int mode_flags[] = {
    MF_USES_REGS | MF_USES_COUNT,
    MF_USES_INPUT,
    MF_USES_COUNT,
    MF_USES_COUNT,
    MF_USES_COUNT
};

void
init_modes(void)
{
	init_keys(MODES_COUNT, &mode, (int*)&mode_flags);
	init_buildin_c_keys(&mode);
	init_buildin_m_keys(&mode);
	init_buildin_n_keys(&mode);
	init_buildin_s_keys(&mode);
	init_buildin_v_keys(&mode);
}

void
modes_pre(void)
{
	if(mode == CMDLINE_MODE)
	{
		wrefresh(status_bar);
		return;
	}
	else if(mode == SORT_MODE)
		return;
	else if(mode == MENU_MODE)
	{
		menu_pre();
		return;
	}

	check_if_filelists_have_changed(curr_view);
	check_background_jobs();

	if(!curr_stats.save_msg)
	{
		clean_status_bar();
		wrefresh(status_bar);
	}
}

void
modes_post(void)
{
	char status_buf[64] = "";

	if(mode == CMDLINE_MODE)
		return;
	else if(mode == SORT_MODE)
		return;
	else if(mode == MENU_MODE)
	{
		menu_post();
		return;
	}

	if(curr_stats.show_full)
		show_full_file_properties(curr_view);
	else
		update_stat_window(curr_view);

	if(curr_view->selected_files)
	{
		static int number = 0;
		if(number != curr_view->selected_files)
		{
			snprintf(status_buf, sizeof(status_buf), "%d %s Selected",
					curr_view->selected_files, curr_view->selected_files == 1 ? "File" :
					"Files");
			status_bar_message(status_buf);
			curr_stats.save_msg = 1;
		}
	}
	else if(!curr_stats.save_msg)
		clean_status_bar();

	if(curr_stats.need_redraw)
		redraw_window();

	update_all_windows();
}

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab : */
