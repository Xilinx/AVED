// SPDX-License-Identifier: GPL-2.0-only
/*
 * amiapp.c - This file contains the command line application for the AMI API
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

/* Standard includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* API includes */
#include "ami.h"
#include "ami_version.h"

/* App includes */
#include "amiapp.h"
#include "apputils.h"
#include "commands.h"

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

/* First element in cmd map is for empty commands. */
#define EMPTY_CMD_HANDLER	(0)
#define CMD_INDEX		(1)

/*****************************************************************************/
/* Local function declarations                                               */
/*****************************************************************************/

/**
 * do_cmd_none() - Global command callback when no CLI command is specified.
 * @options:  Ordered list of options passed in at the command line
 * @num_args:  Number of non-option arguments (excluding command)
 * @args:  List of non-option arguments (excluding command)
 * 
 * `args` may be an invalid pointer. It is the function's responsibility
 * to validate the `num_args` parameter.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE
 */
static int do_cmd_none(struct app_option *options, int num_args, char **args);

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/

/*
 * The program help/usage string.
 */
static const char help_msg[] = \
	APP_NAME " - command line tool for the AMI driver API\r\n"
	"\r\nUsage:\r\n"
	"\t" APP_NAME " {command} [arguments]\r\n"
	"\t" APP_NAME " {command} -h | --help\r\n"
	"\t" APP_NAME " -h | --help\r\n"
	"\t" APP_NAME " --version\r\n"
	"\r\nOptions:\r\n"
	"\t-h --help          Show this screen\r\n"
	"\t--version          Show version\r\n"
	"\r\nCommands:\r\n"
	"\tsensors            Show sensor information\r\n"
	"\tcfgmem_program     Program a device\r\n"
	"\tcfgmem_fpt         Program a device and update the FPT\r\n"
	"\tcfgmem_copy        Copy one partition to another\r\n"
	"\tcfgmem_info        Show partition information\r\n"
	"\tdevice_boot        Set boot partition\r\n"
	"\tmfg_info           View manufacturing information\r\n"
	"\tbar_wr             Write to PCI BAR memory\r\n"
	"\tbar_rd             Read from PCI BAR memory\r\n"
	"\toverview           Show basic AMI/device information\r\n"
	"\tpcieinfo           View PCI-related information\r\n"
	"\treload             Reload a device/devices\r\n"
	"\teeprom_rd          Read data from the device EEPROM\r\n"
	"\teeprom_wr          Write data to the device EEPROM\r\n"
	"\tmodule_byte_rd     Read data from a QSFP module\r\n"
	"\tmodule_byte_wr     Write data to a QSFP module\r\n"
;

/*
 * The program version information string.
 */
static const char api_version_str[] = \
	"API Version     |  %d.%d.%d%c (%d)\r\n"
	"API Branch      |  " GIT_BRANCH "\r\n"
	"API Hash        |  " GIT_HASH   "\r\n"
	"API Hash Date   |  " GIT_DATE   "\r\n"
;

static const char driver_version_str[] = \
	"Driver Version  |  %d.%d.%d%c (%d)\r\n"
;

/*
 * List of short options supported when no command is specified.
 * Short options may correspond to long options but this is not always the case.
 *
 * :  means that the option takes a required argument
 * :: means that the option takes an optional argument
 *
 * h: Show help screen.
 */
static const char short_options[] = "h";

/*
 * List of long options supported when no command is specified.
 * These may or may not correspond to a short counterpart.
 */
static const struct option long_options[] = {
	{ "version", no_argument,       NULL, 'V' },  /* program version */
	{ "help",    no_argument,       NULL, 'h' },  /* help screen */
	{ },
};

static struct app_cmd cmd_none = {
	.callback      = &do_cmd_none,
	.short_options = short_options,
	.long_options  = long_options,
	.root_required = false,
	.help_msg      = help_msg
};

/*
 * List of supported commands.
 */
static const struct app_cmd_map commands[] = {
	{ "",               &cmd_none           },
	{ "sensors",        &cmd_sensors        },
	{ "cfgmem_program", &cmd_cfgmem_program },
	{ "cfgmem_copy",    &cmd_cfgmem_copy    },
	{ "cfgmem_info",    &cmd_cfgmem_info    },
	{ "bar_rd",         &cmd_bar_rd         },
	{ "bar_wr",         &cmd_bar_wr         },
	{ "overview",       &cmd_overview       },
	{ "pcieinfo",       &cmd_pcieinfo       },
	{ "reload",         &cmd_reload         },
	{ "device_boot",    &cmd_device_boot    },
	{ "mfg_info",       &cmd_mfg_info       },
	{ "eeprom_rd",      &cmd_eeprom_rd      },
	{ "eeprom_wr",      &cmd_eeprom_wr      },
	{ "cfgmem_fpt",     &cmd_cfgmem_fpt     },
	{ "module_byte_rd", &cmd_module_byte_rd },
	{ "module_byte_wr", &cmd_module_byte_wr },
};

/*****************************************************************************/
/* Public function definitions                                               */
/*****************************************************************************/

/*
 * Find a CLI option.
 */
struct app_option* find_app_option(const int val, struct app_option *options)
{
	struct app_option *opt = options;

	while (opt) {
		if (opt->val == val) {
			return opt;
		}

		opt = opt->next;
	}

	return NULL;
}

/*
 * Find a command by its name.
 */
int find_app_command(const char *name)
{
	int cmd = 0;

	if (!name)
		return APP_INVALID_INDEX;

	for (cmd = 0; cmd < ARRAY_SIZE(commands); cmd++) {
		if (strcmp(commands[cmd].name, name) == 0) {
			return cmd;
		}
	}

	return APP_INVALID_INDEX;
}

/*****************************************************************************/
/* Local function definitions                                                */
/*****************************************************************************/

/*
 * Empty command callback.
 */
static int do_cmd_none(struct app_option *options, int num_args, char **args)
{
	bool parse_options = true;
	int ret = EXIT_FAILURE;
	struct app_option *next_opt = options;

	/* There should be no positional arguments. */
	if (num_args == 0) {
		if (!options) {
			APP_USER_ERROR("not enough arguments", help_msg);
		} else {
			while (next_opt && parse_options) {
				switch (next_opt->val)
				{
				/* help is handled by top level parser */
				
				case 'V':
				{
					struct ami_version driver_ver = { 0 };

					printf(
						api_version_str,
						GIT_TAG_VER_MAJOR,
						GIT_TAG_VER_MINOR,
						GIT_TAG_VER_PATCH,
						(GIT_STATUS == 0) ? (' ') : ('*'),
						GIT_TAG_VER_DEV_COMMITS
					);

					if (ami_get_driver_version(&driver_ver) == AMI_STATUS_OK) {
						printf(
							driver_version_str,
							driver_ver.major,
							driver_ver.minor,
							driver_ver.patch,
							(driver_ver.status == 0) ? (' ') : ('*'),
							driver_ver.dev_commits
						);

						ret = EXIT_SUCCESS;
					} else {
						APP_API_ERROR("unable to retrieve driver version");
					}

					parse_options = false;
					break;
				}

				default:
					APP_USER_ERROR("invalid options", help_msg);
					parse_options = false;
					break;
				}

				next_opt = next_opt->next;
			}
		}
	} else {
		APP_USER_ERROR("too many arguments", help_msg);
	}

	return ret;
}

/*****************************************************************************/

int main(int argc, char *argv[])
{
	int ret = EXIT_FAILURE;
	int long_ind = 0;
	int opt = AMI_LINUX_STATUS_ERROR;

	int n_args = 0;
	int cmd_ind = APP_INVALID_INDEX;
	struct app_cmd *cmd = NULL;

	struct app_option *options_head = NULL;
	struct app_option *options_tail = NULL;

	/*
	 * Check if user specified a command;
	 * `argc` must be enough to have an element at `CMD_INDEX`
	 * and the element at `CMD_INDEX` must not be an option (i.e., must not
	 * start with a '-' character). Positional arguments are not allowed when
	 * a command is not specified as the argument could be confused for a command.
	 */
	if ((argc < (CMD_INDEX + 1)) || (argv[CMD_INDEX][0] == '-')) {
		cmd_ind = EMPTY_CMD_HANDLER;
	} else {
		cmd_ind = find_app_command(argv[CMD_INDEX]);
	}

	if (cmd_ind == APP_INVALID_INDEX) {
		APP_USER_ERROR("unrecognised command", help_msg);
		exit(EXIT_FAILURE);
	}

	cmd = commands[cmd_ind].command;

	/*
	 * Parse options for the identified command (or no command).
	 */
	while (AMI_LINUX_STATUS_ERROR != (opt = getopt_long(argc, argv, \
			cmd->short_options, cmd->long_options, &long_ind)))
	{
		switch (opt)
		{
		/* Error. */
		case '?':
		case ':':
			APP_USER_ERROR("invalid arguments", cmd->help_msg);
			exit(EXIT_FAILURE);
			break;

		/* All other options. */
		default:
		{
			struct app_option *option = \
				(struct app_option*)calloc(1, sizeof *option);

			option->long_ind = long_ind;
			option->val = opt;
			option->arg = optarg;
			option->next = NULL;
			option->handled = false;

			if (!options_head) {
				options_head = option;
				options_tail = option;
			} else {
				options_tail->next = option;
				options_tail = option;
			}

			break;
		}
		}
	}

	if (cmd_ind != EMPTY_CMD_HANDLER) {
		if ((optind + 1) < argc)  /* +1 to exclude command */
			n_args = argc - optind - 1;
	}

	/* Check if help was requested */
	if (NULL != find_app_option('h', options_head)) {
		if (cmd->help_msg)
			printf("%s", cmd->help_msg);
		else
			printf("Error. No help available for command '%s'\r\n", commands[cmd_ind].name);

		ret = EXIT_SUCCESS;
	} else {
		/* Check permissions. */
		if (cmd->root_required && (geteuid() != 0))
			APP_WARN("this command requires elevated permissions but you are not running as root!\r\n");

		/* Execute command callback. */
		ret = cmd->callback(
			options_head,
			n_args,
			((n_args != 0) ? (&argv[optind + 1]) : (NULL))
		);
	}

	/* Cleanup. */
	struct app_option *next_opt = NULL;
	options_tail = options_head;

	while (options_tail) {
		next_opt = options_tail->next;
		free(options_tail);
		options_tail = next_opt;
	}

	return ret;
}
