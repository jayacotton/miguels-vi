/*	te_ui.c

	Text editor.

	User interface.

	Copyright (c) 2015-2021 Miguel Garcia / FloppySoftware

	This program is free software; you can redistribute it and/or modify it
	under the terms of the GNU General Public License as published by the
	Free Software Foundation; either version 2, or (at your option) any
	later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

	Changes:

	30 Jan 2018 : Extracted from te.c.
	22 Feb 2018 : Ask for confirmation only if changes were not saved. INTRO equals Y on confirmation.
	16 Jan 2019 : Modified Refresh() to show block selection. Added RefreshBlock().
	19 Jan 2019 : Added ShowFilename().
	23 Jan 2019 : Refactorized MenuHelp().
	30 Jan 2019 : Added putchrx().
	24 Dec 2019 : Modified some text messages. SysLineKey() is now SysLineCont(). Added support for numbered lines.
	26 Dec 2019 : Now K_INTRO is K_CR. Add SysLineWait(), SysLineBack().
	28 Feb 2020 : Minor changes.
	08 Mar 2020 : Support for CRT_LONG in menu, about and (ejem) help options.
	31 Dec 2020 : Use NUM_SEP instead of space to separate line numbers from text.
	04 Jan 2021 : Use configuration variables.
	22 Feb 2021 : Removed CRT_ROWS, CRT_COLS.
	04 Apr 2021 : Remove customized key names. Use key bindings from configuration.
	05 Apr 2021 : Adapt HELP to last changes.
	06 Apr 2021 : Use special screen characters from configuration instead of macros.
	08 Apr 2021 : Get adaptation (port) name from configuration.
	05 Jul 2021 : Support for OPT_Z80.
	25 Sep 2021 : Added SysLineEdit(). Fix URLs.
	01 Nov 2021 : Added menu option when macros are enabled: insert file.
*/
extern void CrtOut(int);

/* Read character from keyboard
   ----------------------------
*/
/*
int getchr()
{
	return GetKey();
}
*/
/* Print character on screen
   -------------------------
*/
/* **************************** SEE #define
putchr(ch)
int ch;
{
	CrtOut(ch);
}
******************************* */

/* Print character on screen X times
   ---------------------------------
*/

void putchrx(ch, n)
int ch, n;
{
    TRACE("putchrx");
    while (n--) {
	CrtOut(ch);
    }
}

/* Print string on screen
   ----------------------
*/
#if OPT_Z80
/* putstr(s) */
/* char *s;  */
// *INDENT-OFF*
#asm
putstr:
	pop bc 
	pop hl 
	push hl 
	push bc 
putstr1: 
	ld a, (hl)
     	or a
       ret z
       inc hl
       push hl 
	ld h, 0 
	ld l, a 
	push hl 
	call CrtOut 
	pop de 
	pop hl 
	jp putstr1
#endasm
// *INDENT-ON*
#else
void putstr(s)
char *s;
{
    TRACE("putstr");
    while (*s)
	CrtOut(*s++);
}
#endif

/* Print string + '\n' on screen
   -----------------------------
*/
void putln(s)
char *s;
{
    TRACE("putln");
    putstr(s);
    CrtOut('\n');
}

/* Print number on screen
   ----------------------
*/
void putint(format, value)
char *format;
int value;
{
    char r[7];			/* -12345 + ZERO */

    TRACE("putint");
    sprintf(r, format, value);

    putstr(r);
}

/* Print program layout
   --------------------
*/
void Layout()
{
    int i, k, w;

    TRACE("Layout");
    /* Clear screen */
    CrtClear();

    /* Header */
    putstr("te:");

    /* Information layout */
    CrtLocate(PS_ROW, PS_INF);
    putstr(PS_TXT);

    /* Max. # of lines */
    CrtLocate(PS_ROW, PS_LIN_MAX);
    putint("%04d", cf_mx_lines);

    /* # of columns */
    CrtLocate(PS_ROW, PS_COL_MAX);
    putint("%02d", 1 + ln_max);

    /* Ruler */
#if CRT_LONG
    CrtLocate(BOX_ROW - 1, cf_num);

    w = cf_cols - cf_num;

    for (i = k = 0; i < w; ++i) {
	if (k++) {
	    CrtOut(cf_rul_chr);

	    if (k == cf_tab_cols)
		k = 0;
	} else
	    CrtOut(cf_rul_tab);
    }

    /* System line separator */
    CrtLocate(cf_rows - 2, 0);

    putchrx(cf_horz_chr, cf_cols);
#endif
}

/* Print filename
   --------------
*/
void ShowFilename()
{
    char *s;
    TRACE("ShowFilename");

    CrtLocate(PS_ROW, PS_FNAME);

    putstr((s = (char *) CurrentFile()));

    putchrx(' ', FILENAME_LEN - strlen(s) - 1);
}

/* Print message on system line
   ----------------------------
   Message can be NULL == blank line / clear system line.
*/
void SysLine(s)
char *s;
{
    TRACE("SysLine");
    CrtClearLine(cf_rows - 1);

    if (s)
	putstr(s);

    /* Set flag for Loop() */
    sysln = 1;
}

/* Print message when editing
   --------------------------
*/
void SysLineEdit()
{
    TRACE("SysLineEdit");
    SysLine(GetKeyName(K_ESC));
    putstr(" = menu");
}

/* Print message on system line and wait
   for CR and / or ESC key press
   -------------------------------------
   Message can be NULL. Returns NZ if CR, else Z.
*/
SysLineWait(s, cr, esc)
char *s, *cr, *esc;
{
    int ch;

    TRACE("SysLineWait");
    SysLine(s);

    if (s)
	putstr(" (");

    if (cr) {
	putstr(GetKeyName(K_CR));
	putstr(" = ");
	putstr(cr);
	putstr(", ");
    }

    if (esc) {
	putstr(GetKeyName(K_ESC));
	putstr(" = ");
	putstr(esc);
    }

    if (s)
	CrtOut(')');

    putstr(": ");

    for (;;) {
	ch = GetKey();

	if (cr && ch == K_CR)
	    break;

	if (esc && ch == K_ESC)
	    break;
    }

    SysLine(NULL);

    return (ch == K_CR);
}


/* Print message on system line and wait
   for ESC key press to CONTINUE
   -------------------------------------
   Message can be NULL.
*/
void SysLineCont(s)
char *s;
{
    TRACE("SysLineCont");
    SysLineWait(s, NULL, "continue");
}

/* Print message on system line and wait
   for ESC key press to COMEBACK
   -------------------------------------
   Message can be NULL.
*/
void SysLineBack(s)
char *s;
{
    TRACE("SysLineBack");
    SysLineWait(s, NULL, "back");
}

/* Print message on system line and wait
   for CONFIRMATION
   -------------------------------------
   Message can be NULL. Returns NZ if YES, else Z.
*/
SysLineConf(s)
char *s;
{
    TRACE("SysLineConf");
    return SysLineWait(s, "continue", "cancel");
}

/* Ask for a string
   ----------------
   Return NZ if entered, else Z.
*/
SysLineStr(what, buf, maxlen)
char *what, *buf;
int maxlen;
{
    int ch;

    TRACE("SysLineStr");
    SysLine(what);
    putstr(" (");
    putstr(GetKeyName(K_ESC));
    putstr(" = cancel): ");

    ch = ReadLine(buf, maxlen);

    SysLine(NULL);

    if (ch == K_CR && *buf)
	return 1;

    return 0;
}

/* Ask for a filename
   ------------------
   Return NZ if entered, else Z.
*/
SysLineFile(fn)
char *fn;
{
    TRACE("SysLineFile");
    return SysLineStr("Filename", fn, FILENAME_LEN - 1);
}

/* Ask for confirmation on changes not saved
   -----------------------------------------
   Returns NZ if YES, else Z.
*/
SysLineChanges()
{
    TRACE("SysLineChanges");
    return SysLineConf("Changes will be lost!");
}

/* Read simple line
   ----------------
   Returns last character entered: INTRO or ESC.
*/
int ReadLine(buf, width)
char *buf;
int width;
{
    int len;
    int ch;

    TRACE("ReadLine");
    putstr(buf);
    len = strlen(buf);

    while (1) {
	ch = GetKey();
	switch (ch) {
	case K_LDEL:
	    if (len) {
		CrtOut('\b');
		CrtOut(' ');
		CrtOut('\b');

		--len;
	    }
	    break;
	case K_CR:
	case K_ESC:
	    buf[len] = 0;
	    return ch;
	default:
	    if (len < width && ch >= ' ')
		CrtOut(buf[len++] = ch);
	    break;
	}
    }
    return 0;
}

/* Return name of current file
   ---------------------------
*/
CurrentFile()
{
    TRACE("CurrentFile");
    return (file_name[0] ? file_name : "-");
}

/* Clear the editor box
   --------------------
*/
void ClearBox()
{
    int i;

    TRACE("ClearBox");
    for (i = 0; i < box_rows; ++i)
	CrtClearLine(BOX_ROW + i);
}

/* Print centered text on the screen
   ---------------------------------
*/
void CenterText(row, txt)
int row;
char *txt;
{
    TRACE("CenterText");
    CrtLocate(row, (cf_cols - strlen(txt)) / 2);

    putstr(txt);
}

#if OPT_BLOCK

/* Refresh block selection in editor box
   -------------------------------------
   Set 'sel' to NZ for reverse print, else Z for normal print.
*/
void RefreshBlock(row, sel)
int row, sel;
{
    int i, line;

    TRACE("RefreshBlock");
    line = GetFirstLine() + row;

    for (i = row; i < box_rows; ++i) {
	if (line >= blk_start) {
	    if (line <= blk_end) {
#if CRT_CAN_REV
		CrtLocate(BOX_ROW + i, cf_num);
		CrtClearEol();

		if (sel) {
		    CrtReverse(1);
		}

		putstr(lp_arr[line]);
		CrtOut(' ');

		if (sel) {

		    CrtReverse(0);
		}
#else
		CrtLocate(BOX_ROW + i, cf_cols - 1);
		CrtOut(sel ? BLOCK_CHR : ' ');
#endif
	    } else {
		break;
	    }
	}

	++line;
    }
}

#endif

/* Refresh editor box
   ------------------
   Starting from box row 'row', line 'line'.
*/
void Refresh(row, line)
int row, line;
{
    int i;
    char *format;

    TRACE("Refresh");
#if OPT_BLOCK

    int blk, sel;

    blk = (blk_count && blk_start <= GetLastLine()
	   && blk_end >= GetFirstLine());
    sel = 0;

#endif

    if (cf_num) {
	format = "%?d";
	format[1] = '0' + cf_num - 1;
    }

    for (i = row; i < box_rows; ++i) {
	CrtClearLine(BOX_ROW + i);

	if (line < lp_now) {

	    if (cf_num) {
		putint(format, line + 1);
		CrtOut(cf_lnum_chr);
	    }
#if OPT_BLOCK

	    if (blk) {
		if (line >= blk_start) {
		    if (line <= blk_end) {
#if CRT_CAN_REV
			CrtReverse((sel = 1));
#else
			sel = 1;
#endif
		    }
		}
	    }
#endif

	    putstr(lp_arr[line++]);

#if OPT_BLOCK

	    if (sel) {
#if CRT_CAN_REV
		CrtOut(' ');

		CrtReverse((sel = 0));
#else
		sel = 0;

		CrtLocate(BOX_ROW + i, cf_cols - 1);
		CrtOut(BLOCK_CHR);
#endif
	    }
#endif

	}
    }
}

/* Refresh editor box
   ------------------
*/
void RefreshAll()
{
    TRACE("RefreshAll");
    Refresh(0, lp_cur - box_shr);
}

/* Show the menu
   -------------
   Return NZ to quit program.
*/
Menu()
{
    int run, row, stay, menu, ask;
    TRACE("Menu");

    /* Setup some things */
    run = stay = menu = ask = 1;

    /* Loop */
    while (run) {
	/* Show the menu */
	if (menu) {
	    row = BOX_ROW + 1;

	    ClearBox();

	    CenterText(row++, "OPTIONS");
	    row++;
#if CRT_LONG
	    CenterText(row++, "New");
	    CenterText(row++, "Open");
	    CenterText(row++, "Save");
	    CenterText(row++, "save As");
#if OPT_MACRO
	    CenterText(row++, "Insert");
#endif
	    CenterText(row++, "Help");
	    CenterText(row++, "aBout te");
	    CenterText(row, "eXit te");
#else
#if OPT_MACRO
	    CenterText(row++, "New     Open  Save      Save As");
	    CenterText(row++, "Insert  Help  aBout te  eXit te");
#else
	    CenterText(row++, "New   Open      Save     Save As");
	    CenterText(row++, "Help  aBout te  eXit te         ");
#endif
#endif
	    menu = 0;
	}

	/* Ask for option */
	if (ask) {
	    SysLine("Option (");
	    putstr(GetKeyName(K_ESC));
	    putstr(" = back): ");
	} else {
	    ask = 1;
	}

	/* Do it */
	switch (toupper(GetKey())) {
	case 'N':
	    run = MenuNew();
	    break;
	case 'O':
	    run = MenuOpen();
	    break;
	case 'S':
	    run = MenuSave();
	    break;
	case 'A':
	    run = MenuSaveAs();
	    break;
#if OPT_MACRO
	case 'I':
	    run = MenuInsert();
	    break;
#endif
	case 'B':
	    MenuAbout();
	    menu = 1;
	    break;
	case 'H':
	    MenuHelp();
	    menu = 1;
	    break;
	case 'X':
	    run = stay = MenuExit();
	    break;
	case K_ESC:
	    run = 0;
	    break;
	default:
	    ask = 0;
	    break;
	}
    }

    /* Clear editor box */
    ClearBox();

    SysLine(NULL);

    /* Return NZ to quit the program */
    return !stay;
}

/* Menu option: New
   ----------------
   Return Z to quit the menu.
*/
MenuNew()
{
    TRACE("MenuNew");
    if (lp_chg) {
	if (!SysLineChanges())
	    return 1;
    }

    NewFile();

    return 0;
}

/* Menu option: Open
   -----------------
   Return Z to quit the menu.
*/
MenuOpen()
{
    char fn[FILENAME_LEN];

    TRACE("MenuOpen");
    if (lp_chg) {
	if (!SysLineChanges())
	    return 1;
    }

    fn[0] = 0;

    if (SysLineFile(fn)) {
	if (ReadFile(fn))
	    NewFile();
	else
	    strcpy(file_name, fn);

	return 0;
    }

    return 1;
}

/* Menu option: Save
   -----------------
   Return Z to quit the menu.
*/
MenuSave()
{
    TRACE("MenuSave");
    if (!file_name[0])
	return MenuSaveAs();

    WriteFile(file_name);

    return 1;
}

/* Menu option: Save as
   --------------------
   Return Z to quit the menu.
*/
MenuSaveAs()
{
    char fn[FILENAME_LEN];

    TRACE("MenuSaveAs");
    strcpy(fn, file_name);

    if (SysLineFile(fn)) {
	if (!WriteFile(fn))
	    strcpy(file_name, fn);

	return 0;
    }

    return 1;
}

#if OPT_MACRO

/* Menu option: Insert
   -------------------
   Return Z to quit the menu.
*/
MenuInsert()
{
    char fn[FILENAME_LEN];

    TRACE("MenuInsert");
    fn[0] = 0;

    if (SysLineFile(fn)) {
	return MacroRunFile(fn, 1);
    }

    return 1;
}

#endif

/* Menu option: Help
   -----------------
*/
void MenuHelp()
{
    int i, k;
    char *s;

    TRACE("MenuHelp");
    ClearBox();

    CrtLocate(BOX_ROW + 1, 0);

    putln("HELP:\n");

#if CRT_LONG

    for (i = 0; help_items[i] != -1; ++i) {

	// 123456789012345 (15 characters)
	// BlkEnd     ^B^E

	if ((k = help_items[i])) {
	    if (*(s = GetKeyWhat(k)) == '?') {
		k = 0;
	    }
	}

	if (k) {
	    putstr(s);
	    putchrx(' ', 11 - strlen(s));

	    k -= 1000;

	    MenuHelpCh(cf_keys[k]);
	    MenuHelpCh(cf_keys_ex[k]);
	} else {
	    putchrx(' ', 15);
	}

	if ((i + 1) % 3) {
	    CrtOut(' ');
	    CrtOut(cf_vert_chr);
	    CrtOut(' ');
	} else {
	    CrtOut('\n');
	}
    }

#else

    putstr("Sorry, no help is available.");

#endif

    SysLineBack(NULL);
}

void MenuHelpCh(ch)
int ch;
{
    TRACE("MenuHelpCH");
    if (ch) {
	if (ch < 32 || ch == 0x7F) {
	    CrtOut('^');
	    CrtOut(ch != 0x7F ? '@' + ch : '?');
	} else {
	    CrtOut(ch);
	    CrtOut(' ');
	}
    } else {
	CrtOut(' ');
	CrtOut(' ');
    }
}

/* Menu option: About
   ------------------
*/
void MenuAbout()
{
    int row;

    TRACE("MenuAbout");
#if CRT_LONG
    row = BOX_ROW + 1;

    ClearBox();

    CenterText(row++, "te - Text Editor");
    row++;
    CenterText(row++, VERSION);
    row++;
    CenterText(row++, "Configured for");
    CenterText(row++, cf_name);
    row++;
    CenterText(row++, COPYRIGHT);
    row++;
    CenterText(row++, "http://www.floppysoftware.es");
    CenterText(row++, "https://cpm-connections.blogspot.com");
    CenterText(row, "floppysoftware@gmail.com");
#else
    row = BOX_ROW;

    ClearBox();

    CenterText(row++, "te - Text Editor");
    CenterText(row++, VERSION);
    CenterText(row++, "Configured for");
    CenterText(row++, cf_name);
    CenterText(row++, COPYRIGHT);
    CenterText(row++, "www.floppysoftware.es");
#endif

    SysLineBack(NULL);
}

/* Menu option: Quit program
   -------------------------
*/
MenuExit()
{
    TRACE("MenuExit");
    if (lp_chg) {
	return !SysLineChanges();
    }

    /* Quit program */
    return 0;
}
