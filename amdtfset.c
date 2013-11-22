/*
 * amdtfset  Copyright (C) 2013  Sebastian Thorarensen <sebth@naju.se>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <dlfcn.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#define ADL_OK 0
#define ADL_TRUE 1
#define ADL_FALSE 0
#define	ADL_ADAPTER_TEAR_FREE_ON 1
#define ADL_ADAPTER_TEAR_FREE_NOTENOUGHMEM -1
#define ADL_ADAPTER_TEAR_FREE_OFF_ERR_QUADBUFFERSTEREO -2
#define ADL_ADAPTER_TEAR_FREE_OFF_ERR_MGPUSLD -3
#define ADL_ADAPTER_TEAR_FREE_OFF 0

typedef void *(ADL_MAIN_MALLOC_CALLBACK)(size_t);
typedef int (*ADL_MAIN_CONTROL_CREATE)(ADL_MAIN_MALLOC_CALLBACK, int);
typedef int (*ADL_ADAPTER_TEAR_FREE_SET)(int, int, int *);
typedef int (*ADL_ADAPTER_TEAR_FREE_GET)(int, int *, int *, int *);

static char *progname;
static ADL_MAIN_CONTROL_CREATE ADL_Main_Control_Create;
static ADL_ADAPTER_TEAR_FREE_SET ADL_Adapter_Tear_Free_Set;
static ADL_ADAPTER_TEAR_FREE_GET ADL_Adapter_Tear_Free_Get;

static const char *tf_strstatus(int status)
{
	static char s[21 + sizeof(int) * CHAR_BIT / 3 + 2 + 1];

	switch (status) {
		case ADL_ADAPTER_TEAR_FREE_ON:
			return "Tear free desktop is enabled.";
		case ADL_ADAPTER_TEAR_FREE_NOTENOUGHMEM:
			return "Tear free desktop is disabled due to a lack "
					"of graphic adapter memory.";
		case ADL_ADAPTER_TEAR_FREE_OFF_ERR_QUADBUFFERSTEREO:
			return "Tear free desktop is disabled due to quad "
					"buffer stereo being enabled.";
		case ADL_ADAPTER_TEAR_FREE_OFF_ERR_MGPUSLD:
			return "Tear free desktop is disabled due to MGPU-SLS "
					"being enabled.";
		case ADL_ADAPTER_TEAR_FREE_OFF:
			return "Tear free desktop is disabled.";
	}

	snprintf(s, sizeof(s), "Unknown status code %i", status);
	return s;
}

static int tf_get(int quiet)
{
	int retval;
	int def;
	int requested;
	int status;

	if ((retval = ADL_Adapter_Tear_Free_Get(0, &def, &requested,
			&status)) != ADL_OK) {
		fprintf(stderr, "%s: ADL_Adapter_Tear_Free_Get error: "
				"ADL error code %i\n", progname, retval);
		exit(EXIT_FAILURE);
	}

	if (!quiet)
		puts(tf_strstatus(status));
	if (status < 0)
		return ADL_ADAPTER_TEAR_FREE_OFF;
	return status;
}

static void tf_set(int requested)
{
	int retval;
	int status;

	if ((retval = ADL_Adapter_Tear_Free_Set(0, requested, &status))
			!= ADL_OK) {
		fprintf(stderr, "%s: ADL_Adapter_Tear_Free_Set error: "
				"ADL error code %i\n", progname, retval);
		exit(EXIT_FAILURE);
	}
}

static void adl_init(void)
{
	int retval;
	if ((retval = ADL_Main_Control_Create(malloc, 1)) != ADL_OK) {
		fprintf(stderr, "%s: ADL_Main_Control_Create error: "
				"ADL error code %i\n", progname, retval);
		exit(EXIT_FAILURE);
	}
}

static void handle_dlerror(const char *s)
{
	char *errstr = dlerror();
	if (errstr)
		fprintf(stderr, "%s: %s\n", progname, errstr);
	else
		fprintf(stderr, "%s: %s error\n", progname, s);
	exit(EXIT_FAILURE);
}

static void load_and_link(void)
{
	void *handle;

	if (!(handle = dlopen("libatiadlxx.so", RTLD_LAZY)))
		handle_dlerror("dlopen");

	if (!(*(void **)(&ADL_Main_Control_Create) =
			dlsym(handle, "ADL_Main_Control_Create")))
		handle_dlerror("dlsym");
	if (!(*(void **)(&ADL_Adapter_Tear_Free_Set) =
			dlsym(handle, "ADL_Adapter_Tear_Free_Set")))
		handle_dlerror("dlsym");
	if (!(*(void **)(&ADL_Adapter_Tear_Free_Get) =
			dlsym(handle, "ADL_Adapter_Tear_Free_Get")))
		handle_dlerror("dlsym");
}

static void handle_usage(void)
{
	fprintf(stderr, "usage: %s [-q] [on | off]\n", progname);
	exit(EX_USAGE);
}

int main(int argc, char *argv[])
{
	int opt;
	int quiet = 0;
	progname = argv[0];

	while ((opt = getopt(argc, argv, "q")) != -1) {
		if (opt == 'q')
			quiet = 1;
		else
			handle_usage();
	}

	load_and_link();
	adl_init();

	if (argc > optind + 1) {
		fprintf(stderr, "%s: too many arguments\n", progname);
		handle_usage();
	}

	if (argc > optind) {
		if (strcmp(argv[optind], "on") == 0) {
			tf_set(ADL_TRUE);
		} else if (strcmp(argv[optind], "off") == 0) {
			tf_set(ADL_FALSE);
		} else {
			fprintf(stderr, "%s: invalid argument -- '%s'\n",
					progname, argv[optind]);
			handle_usage();
		}
	} else {
		return !tf_get(quiet);
	}

	return EXIT_SUCCESS;
}
