const char *tags[] = {
	"vifm-!!",
	"vifm-%",
	"vifm-'",
	"vifm-'autochpos'",
	"vifm-'cf'",
	"vifm-'confirm'",
	"vifm-'cpo'",
	"vifm-'cpoptions'",
	"vifm-'fastrun'",
	"vifm-'followlinks'",
	"vifm-'fusehome'",
	"vifm-'gdefault'",
	"vifm-'hi'",
	"vifm-'history'",
	"vifm-'hls'",
	"vifm-'hlsearch'",
	"vifm-'ic'",
	"vifm-'iec'",
	"vifm-'ignorecase'",
	"vifm-'incsearch'",
	"vifm-'is'",
	"vifm-'laststatus'",
	"vifm-'ls'",
	"vifm-'runexec'",
	"vifm-'scb'",
	"vifm-'scrollbind'",
	"vifm-'scrolloff'",
	"vifm-'scs'",
	"vifm-'sh'",
	"vifm-'shell'",
	"vifm-'slowfs'",
	"vifm-'smartcase'",
	"vifm-'so'",
	"vifm-'sort'",
	"vifm-'sortnumbers'",
	"vifm-'sortorder'",
	"vifm-'timefmt'",
	"vifm-'timeoutlen'",
	"vifm-'tm'",
	"vifm-'trash'",
	"vifm-'ul'",
	"vifm-'undolevels'",
	"vifm-'vicmd'",
	"vifm-'vifminfo'",
	"vifm-'vimhelp'",
	"vifm-'vixcmd'",
	"vifm-'wildmenu'",
	"vifm-'wmnu'",
	"vifm-'wrap'",
	"vifm-'wrapscan'",
	"vifm-'ws'",
	"vifm-(",
	"vifm-)",
	"vifm-,",
	"vifm--+c",
	"vifm---help",
	"vifm---logging",
	"vifm---no-configs",
	"vifm---version",
	"vifm--c",
	"vifm--f",
	"vifm--h",
	"vifm--v",
	"vifm-.",
	"vifm-/",
	"vifm-:",
	"vifm-:!",
	"vifm-:!!",
	"vifm-:alink",
	"vifm-:apropos",
	"vifm-:c",
	"vifm-:cd",
	"vifm-:change",
	"vifm-:chmod",
	"vifm-:chown",
	"vifm-:clone",
	"vifm-:cm",
	"vifm-:cmap",
	"vifm-:cno",
	"vifm-:cnoremap",
	"vifm-:co",
	"vifm-:colo",
	"vifm-:colorscheme",
	"vifm-:com",
	"vifm-:comc",
	"vifm-:comclear",
	"vifm-:command",
	"vifm-:copy",
	"vifm-:cu",
	"vifm-:cunmap",
	"vifm-:d",
	"vifm-:delc",
	"vifm-:delcommand",
	"vifm-:delete",
	"vifm-:delm",
	"vifm-:delmarks",
	"vifm-:di",
	"vifm-:dirs",
	"vifm-:display",
	"vifm-:e",
	"vifm-:edit",
	"vifm-:empty",
	"vifm-:exi",
	"vifm-:exit",
	"vifm-:f",
	"vifm-:file",
	"vifm-:filet",
	"vifm-:filetype",
	"vifm-:filev",
	"vifm-:filevtype",
	"vifm-:filex",
	"vifm-:filextype",
	"vifm-:filter",
	"vifm-:fin",
	"vifm-:find",
	"vifm-:gr",
	"vifm-:grep",
	"vifm-:h",
	"vifm-:help",
	"vifm-:hi",
	"vifm-:highlight",
	"vifm-:his",
	"vifm-:history",
	"vifm-:invert",
	"vifm-:jobs",
	"vifm-:locate",
	"vifm-:ls",
	"vifm-:m",
	"vifm-:ma",
	"vifm-:map",
	"vifm-:mark",
	"vifm-:marks",
	"vifm-:mes",
	"vifm-:messages",
	"vifm-:mkdir",
	"vifm-:move",
	"vifm-:nm",
	"vifm-:nmap",
	"vifm-:nn",
	"vifm-:nnoremap",
	"vifm-:no",
	"vifm-:noh",
	"vifm-:nohlsearch",
	"vifm-:noremap",
	"vifm-:nun",
	"vifm-:nunmap",
	"vifm-:on",
	"vifm-:only",
	"vifm-:popd",
	"vifm-:pushd",
	"vifm-:pw",
	"vifm-:pwd",
	"vifm-:q",
	"vifm-:quit",
	"vifm-:range",
	"vifm-:reg",
	"vifm-:registers",
	"vifm-:rename",
	"vifm-:restart",
	"vifm-:restore",
	"vifm-:rlink",
	"vifm-:s",
	"vifm-:screen",
	"vifm-:se",
	"vifm-:set",
	"vifm-:sh",
	"vifm-:shell",
	"vifm-:so",
	"vifm-:sor",
	"vifm-:sort",
	"vifm-:source",
	"vifm-:sp",
	"vifm-:split",
	"vifm-:substitute",
	"vifm-:sync",
	"vifm-:touch",
	"vifm-:tr",
	"vifm-:undol",
	"vifm-:undolist",
	"vifm-:unm",
	"vifm-:unmap",
	"vifm-:ve",
	"vifm-:version",
	"vifm-:vie",
	"vifm-:view",
	"vifm-:vifm",
	"vifm-:vm",
	"vifm-:vmap",
	"vifm-:vn",
	"vifm-:vnoremap",
	"vifm-:volume",
	"vifm-:vsp",
	"vifm-:vsplit",
	"vifm-:vu",
	"vifm-:vunmap",
	"vifm-:w",
	"vifm-:wq",
	"vifm-:write",
	"vifm-:x",
	"vifm-:xit",
	"vifm-:y",
	"vifm-:yank",
	"vifm-;",
	"vifm-?",
	"vifm-C",
	"vifm-CTRL-A",
	"vifm-CTRL-B",
	"vifm-CTRL-D",
	"vifm-CTRL-E",
	"vifm-CTRL-F",
	"vifm-CTRL-G",
	"vifm-CTRL-I",
	"vifm-CTRL-O",
	"vifm-CTRL-R",
	"vifm-CTRL-U",
	"vifm-CTRL-W_+",
	"vifm-CTRL-W_-",
	"vifm-CTRL-W_<",
	"vifm-CTRL-W_=",
	"vifm-CTRL-W_>",
	"vifm-CTRL-W_H",
	"vifm-CTRL-W_J",
	"vifm-CTRL-W_K",
	"vifm-CTRL-W_L",
	"vifm-CTRL-W_O",
	"vifm-CTRL-W_S",
	"vifm-CTRL-W_V",
	"vifm-CTRL-W_W",
	"vifm-CTRL-W_X",
	"vifm-CTRL-W__",
	"vifm-CTRL-X",
	"vifm-CTRL-Y",
	"vifm-D",
	"vifm-DD",
	"vifm-Enter",
	"vifm-F",
	"vifm-G",
	"vifm-H",
	"vifm-L",
	"vifm-M",
	"vifm-N",
	"vifm-P",
	"vifm-PageDown",
	"vifm-PageUp",
	"vifm-SHIFT-Tab",
	"vifm-Space",
	"vifm-Tab",
	"vifm-V",
	"vifm-V_%",
	"vifm-V_/",
	"vifm-V_<",
	"vifm-V_>",
	"vifm-V_?",
	"vifm-V_ALT-<",
	"vifm-V_ALT->",
	"vifm-V_ALT-Space",
	"vifm-V_ALT-V",
	"vifm-V_CTRL-B",
	"vifm-V_CTRL-D",
	"vifm-V_CTRL-E",
	"vifm-V_CTRL-F",
	"vifm-V_CTRL-K",
	"vifm-V_CTRL-L",
	"vifm-V_CTRL-N",
	"vifm-V_CTRL-P",
	"vifm-V_CTRL-R",
	"vifm-V_CTRL-U",
	"vifm-V_CTRL-V",
	"vifm-V_CTRL-W_+",
	"vifm-V_CTRL-W_-",
	"vifm-V_CTRL-W_<",
	"vifm-V_CTRL-W_=",
	"vifm-V_CTRL-W_>",
	"vifm-V_CTRL-W_S",
	"vifm-V_CTRL-W__",
	"vifm-V_CTRL-W_v",
	"vifm-V_CTRL-Y",
	"vifm-V_Enter",
	"vifm-V_G",
	"vifm-V_N",
	"vifm-V_Q",
	"vifm-V_R",
	"vifm-V_SHIFT-Tab",
	"vifm-V_Space",
	"vifm-V_Tab",
	"vifm-V_ZZ",
	"vifm-V_b",
	"vifm-V_d",
	"vifm-V_e",
	"vifm-V_f",
	"vifm-V_g",
	"vifm-V_j",
	"vifm-V_k",
	"vifm-V_n",
	"vifm-V_p",
	"vifm-V_q",
	"vifm-V_r",
	"vifm-V_u",
	"vifm-V_v",
	"vifm-V_w",
	"vifm-V_y",
	"vifm-V_z",
	"vifm-Y",
	"vifm-ZQ",
	"vifm-ZZ",
	"vifm-[count]",
	"vifm-al",
	"vifm-cW",
	"vifm-c_ALT-B",
	"vifm-c_ALT-D",
	"vifm-c_ALT-F",
	"vifm-c_Backspace",
	"vifm-c_CTRL-A",
	"vifm-c_CTRL-B",
	"vifm-c_CTRL-C",
	"vifm-c_CTRL-D",
	"vifm-c_CTRL-E",
	"vifm-c_CTRL-F",
	"vifm-c_CTRL-H",
	"vifm-c_CTRL-I",
	"vifm-c_CTRL-K",
	"vifm-c_CTRL-M",
	"vifm-c_CTRL-N",
	"vifm-c_CTRL-P",
	"vifm-c_CTRL-U",
	"vifm-c_CTRL-W",
	"vifm-c_CTRL-_",
	"vifm-c_Delete",
	"vifm-c_Down",
	"vifm-c_End",
	"vifm-c_Enter",
	"vifm-c_Esc",
	"vifm-c_Home",
	"vifm-c_Left",
	"vifm-c_Right",
	"vifm-c_SHIFT-Tab",
	"vifm-c_Tab",
	"vifm-c_Up",
	"vifm-cg",
	"vifm-cl",
	"vifm-co",
	"vifm-command-line",
	"vifm-commands",
	"vifm-commands-bg",
	"vifm-compatibility-mode",
	"vifm-configure",
	"vifm-count",
	"vifm-cp",
	"vifm-cw",
	"vifm-d",
	"vifm-dd",
	"vifm-e",
	"vifm-f",
	"vifm-filters",
	"vifm-gA",
	"vifm-gU",
	"vifm-gUU",
	"vifm-gUgU",
	"vifm-ga",
	"vifm-general-keys",
	"vifm-gf",
	"vifm-gg",
	"vifm-gl",
	"vifm-gs",
	"vifm-gu",
	"vifm-gugu",
	"vifm-guu",
	"vifm-gv",
	"vifm-h",
	"vifm-i",
	"vifm-j",
	"vifm-k",
	"vifm-l",
	"vifm-m",
	"vifm-m_/",
	"vifm-m_:",
	"vifm-m_?",
	"vifm-m_CTRL-B",
	"vifm-m_CTRL-C",
	"vifm-m_CTRL-D",
	"vifm-m_CTRL-E",
	"vifm-m_CTRL-F",
	"vifm-m_CTRL-L",
	"vifm-m_CTRL-N",
	"vifm-m_CTRL-P",
	"vifm-m_CTRL-U",
	"vifm-m_CTRL-Y",
	"vifm-m_Enter",
	"vifm-m_Escape",
	"vifm-m_G",
	"vifm-m_H",
	"vifm-m_L",
	"vifm-m_M",
	"vifm-m_N",
	"vifm-m_gg",
	"vifm-m_j",
	"vifm-m_k",
	"vifm-m_l",
	"vifm-m_n",
	"vifm-m_zH",
	"vifm-m_zL",
	"vifm-m_zb",
	"vifm-m_zh",
	"vifm-m_zl",
	"vifm-m_zt",
	"vifm-m_zz",
	"vifm-mappings",
	"vifm-menus-and-dialogs",
	"vifm-n",
	"vifm-normal",
	"vifm-options",
	"vifm-p",
	"vifm-plugin",
	"vifm-ranges",
	"vifm-registers",
	"vifm-reserved",
	"vifm-rl",
	"vifm-selectors",
	"vifm-set-options",
	"vifm-t",
	"vifm-u",
	"vifm-v",
	"vifm-v_:",
	"vifm-v_Enter",
	"vifm-v_O",
	"vifm-v_U",
	"vifm-v_V",
	"vifm-v_gU",
	"vifm-v_gu",
	"vifm-v_gv",
	"vifm-v_o",
	"vifm-v_u",
	"vifm-v_v",
	"vifm-view",
	"vifm-visual",
	"vifm-y",
	"vifm-yy",
	"vifm-zM",
	"vifm-zO",
	"vifm-zR",
	"vifm-za",
	"vifm-zb",
	"vifm-zf",
	"vifm-zm",
	"vifm-zo",
	"vifm-zt",
	"vifm-zz",
	"vifm.txt",
	0,
};
