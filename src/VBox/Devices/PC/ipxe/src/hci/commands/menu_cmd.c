/*
 * Copyright (C) 2012 Michael Brown <mbrown@fensystems.co.uk>.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

FILE_LICENCE ( GPL2_OR_LATER );

/** @file
 *
 * Menu commands
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <ipxe/menu.h>
#include <ipxe/command.h>
#include <ipxe/parseopt.h>
#include <ipxe/settings.h>
#include <ipxe/features.h>

FEATURE ( FEATURE_MISC, "Menu", DHCP_EB_FEATURE_MENU, 1 );

/** "menu" options */
struct menu_options {
	/** Name */
	const char *name;
	/** Delete */
	int delete;
};

/** "menu" option list */
static struct option_descriptor menu_opts[] = {
	OPTION_DESC ( "name", 'n', required_argument,
		      struct menu_options, name, parse_string ),
	OPTION_DESC ( "delete", 'd', no_argument,
		      struct menu_options, delete, parse_flag ),
};

/** "menu" command descriptor */
static struct command_descriptor menu_cmd =
	COMMAND_DESC ( struct menu_options, menu_opts, 0, MAX_ARGUMENTS,
		       "[--name <name>] [--delete] [<title>]" );

/**
 * The "menu" command
 *
 * @v argc		Argument count
 * @v argv		Argument list
 * @ret rc		Return status code
 */
static int menu_exec ( int argc, char **argv ) {
	struct menu_options opts;
	struct menu *menu;
	char *title;
	int rc;

	/* Parse options */
	if ( ( rc = parse_options ( argc, argv, &menu_cmd, &opts ) ) != 0 )
		goto err_parse_options;

	/* Parse title */
	title = concat_args ( &argv[optind] );
	if ( ! title ) {
		rc = -ENOMEM;
		goto err_parse_title;
	}

	/* Create menu */
	menu = create_menu ( opts.name, title );
	if ( ! menu ) {
		rc = -ENOMEM;
		goto err_create_menu;
	}

	/* Destroy menu, if applicable */
	if ( opts.delete )
		destroy_menu ( menu );

	/* Success */
	rc = 0;

 err_create_menu:
	free ( title );
 err_parse_title:
 err_parse_options:
	return rc;
}

/** "item" options */
struct item_options {
	/** Menu name */
	const char *menu;
	/** Shortcut key */
	unsigned int key;
	/** Use as default */
	int is_default;
	/** Use as a separator */
	int is_gap;
};

/** "item" option list */
static struct option_descriptor item_opts[] = {
	OPTION_DESC ( "menu", 'm', required_argument,
		      struct item_options, menu, parse_string ),
	OPTION_DESC ( "key", 'k', required_argument,
		      struct item_options, key, parse_key ),
	OPTION_DESC ( "default", 'd', no_argument,
		      struct item_options, is_default, parse_flag ),
	OPTION_DESC ( "gap", 'g', no_argument,
		      struct item_options, is_gap, parse_flag ),
};

/** "item" command descriptor */
static struct command_descriptor item_cmd =
	COMMAND_DESC ( struct item_options, item_opts, 0, MAX_ARGUMENTS,
		       "[--menu <menu>] [--key <key>] [--default] "
		       "[<label>|--gap [<text>]]" );

/**
 * The "item" command
 *
 * @v argc		Argument count
 * @v argv		Argument list
 * @ret rc		Return status code
 */
static int item_exec ( int argc, char **argv ) {
	struct item_options opts;
	struct menu *menu;
	struct menu_item *item;
	char *label = NULL;
	char *text = NULL;
	int rc;

	/* Parse options */
	if ( ( rc = parse_options ( argc, argv, &item_cmd, &opts ) ) != 0 )
		goto err_parse_options;

	/* Parse label, if present */
	if ( ! opts.is_gap )
		label = argv[optind++]; /* May be NULL */

	/* Parse text, if present */
	if ( optind < argc ) {
		text = concat_args ( &argv[optind] );
		if ( ! text ) {
			rc = -ENOMEM;
			goto err_parse_text;
		}
	}

	/* Identify menu */
	if ( ( rc = parse_menu ( opts.menu, &menu ) ) != 0 )
		goto err_parse_menu;

	/* Add menu item */
	item = add_menu_item ( menu, label, ( text ? text : "" ),
			       opts.key, opts.is_default );
	if ( ! item ) {
		rc = -ENOMEM;
		goto err_add_menu_item;
	}

	/* Success */
	rc = 0;

 err_add_menu_item:
 err_parse_menu:
	free ( text );
 err_parse_text:
 err_parse_options:
	return rc;
}

/** "choose" options */
struct choose_options {
	/** Menu name */
	const char *menu;
	/** Timeout */
	unsigned int timeout;
	/** Default selection */
	const char *select;
	/** Keep menu */
	int keep;
};

/** "choose" option list */
static struct option_descriptor choose_opts[] = {
	OPTION_DESC ( "menu", 'm', required_argument,
		      struct choose_options, menu, parse_string ),
	OPTION_DESC ( "default", 'd', required_argument,
		      struct choose_options, select, parse_string ),
	OPTION_DESC ( "timeout", 't', required_argument,
		      struct choose_options, timeout, parse_integer ),
	OPTION_DESC ( "keep", 'k', no_argument,
		      struct choose_options, keep, parse_flag ),
};

/** "choose" command descriptor */
static struct command_descriptor choose_cmd =
	COMMAND_DESC ( struct choose_options, choose_opts, 1, 1,
		       "[--menu <menu>] [--default <label>] "
		       "[--timeout <timeout>] [--keep] <setting>" );

/**
 * The "choose" command
 *
 * @v argc		Argument count
 * @v argv		Argument list
 * @ret rc		Return status code
 */
static int choose_exec ( int argc, char **argv ) {
	struct choose_options opts;
	struct menu *menu;
	struct menu_item *item;
	const char *setting;
	int rc;

	/* Parse options */
	if ( ( rc = parse_options ( argc, argv, &choose_cmd, &opts ) ) != 0 )
		goto err_parse_options;

	/* Parse setting name */
	setting = argv[optind];

	/* Identify menu */
	if ( ( rc = parse_menu ( opts.menu, &menu ) ) != 0 )
		goto err_parse_menu;

	/* Show menu */
	if ( ( rc = show_menu ( menu, opts.timeout, opts.select, &item ) ) != 0)
		goto err_show_menu;

	/* Store setting */
	if ( ( rc = storef_named_setting ( setting, item->label ) ) != 0 ) {
		printf ( "Could not store \"%s\": %s\n",
			 setting, strerror ( rc ) );
		goto err_store;
	}

	/* Success */
	rc = 0;

 err_store:
 err_show_menu:
	/* Destroy menu, if applicable */
	if ( ! opts.keep )
		destroy_menu ( menu );
 err_parse_menu:
 err_parse_options:
	return rc;
}

/** Menu commands */
struct command menu_commands[] __command = {
	{
		.name = "menu",
		.exec = menu_exec,
	},
	{
		.name = "item",
		.exec = item_exec,
	},
	{
		.name = "choose",
		.exec = choose_exec,
	},
};
