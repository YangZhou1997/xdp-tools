/* SPDX-License-Identifier: GPL-2.0 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>

#include <locale.h>
#include <unistd.h>
#include <time.h>

#include <bpf/bpf.h>
#include <bpf/libbpf.h>
#include <arpa/inet.h>

#include <net/if.h>
#include <linux/if_ether.h>
#include <linux/if_link.h> /* depend on kernel-headers installed */

#include "params.h"
#include "common_kern_user.h"
#include "prog_features.h"


struct installopt {
	bool help;
	char *devname;
	int features;
};

struct flag_val install_features[] = {
	{"tcp", FEAT_TCP},
	{"udp", FEAT_UDP},
	{"ipv6", FEAT_IPV6},
	{"ipv4", FEAT_IPV4},
	{"ethernet", FEAT_ETHERNET},
	{"all", FEAT_ALL},
	{}
};

static char *find_progname(__u32 features)
{
	struct prog_feature *feat;

	if (!features)
		return NULL;

	for (feat = prog_features; feat->prog_name; feat++) {
		if ((ntohl(feat->features) & features) == features)
			return feat->prog_name;
	}
	return NULL;
}

static struct option_wrapper install_options[] = {
	DEFINE_OPTION('h', "help", no_argument, false, OPT_HELP, NULL,
		      "Show help", "",
		      struct installopt, help),
	DEFINE_OPTION('d', "dev", required_argument, true, OPT_STRING, NULL,
		      "Install on device <ifname>", "<ifname>",
		      struct installopt, devname),
	DEFINE_OPTION('f', "features", optional_argument, true,
		      OPT_FLAGS, install_features,
		      "Enable features <feats>", "<feats>",
		      struct installopt, features),
	END_OPTIONS
};

int do_install(int argc, char **argv)
{
	struct installopt opt = {};

	/* Cmdline options can change progsec */
	parse_cmdline_args(argc, argv, install_options, &opt,
			   "xdp-filter install",
			   "Install xdp-filter on an interface");

	printf("help: %d dev %s feats %d\n", opt.help, opt.devname, opt.features);

	printf("Found prog for requested features: %s\n", find_progname(opt.features));

	return EXIT_SUCCESS;
}

int do_add_port(int argc, char **argv)
{
	return EXIT_FAILURE;
}

int do_add_ip(int argc, char **argv)
{
	return EXIT_FAILURE;
}

int do_add_ether(int argc, char **argv)
{
	return EXIT_FAILURE;
}

int do_status(int argc, char **argv)
{
	return EXIT_FAILURE;
}

int do_help(int argc, char **argv)
{
	fprintf(stderr,
		"Usage: xdp-filter { COMMAND | help } [OPTIONS]\n"
		"\n"
		"COMMAND can be one of:\n"
		"       install     - install xdp-filter on an interface\n"
		"       port        - add a port to the blacklist\n"
		"       ip          - add an IP address to the blacklist\n"
		"       ether       - add an Ethernet MAC address to the blacklist\n"
		"       status      - show current xdp-filter status\n"
		"\n"
		"Use 'xdp-filter <COMMAND> --help' to see options for each command\n");
	exit(-1);
}


static const struct cmd {
	const char *cmd;
	int (*func)(int argc, char **argv);
} cmds[] = {
	{ "install",	do_install },
	{ "port",	do_add_port },
	{ "ip",	do_add_ip },
	{ "ether",	do_add_ether },
	{ "status",	do_status },
	{ "help",	do_help },
	{ 0 }
};

static int do_cmd(const char *argv0, int argc, char **argv)
{
	const struct cmd *c;

	for (c = cmds; c->cmd; ++c) {
		if (is_prefix(argv0, c->cmd))
			return -(c->func(argc, argv));
	}

	fprintf(stderr, "Object \"%s\" is unknown, try \"xdp-filt help\".\n", argv0);
	return EXIT_FAILURE;
}

const char *pin_basedir =  "/sys/fs/bpf";

int main(int argc, char **argv)
{
	if (argc > 1)
		return do_cmd(argv[1], argc-1, argv+1);
	return EXIT_FAILURE;
}
