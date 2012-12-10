/*
 *   Copyright (c) 2008 Vijay Kumar B. <vijaykumar@bravegnu.org>
 *
 *   Based on testcases/kernel/syscalls/waitpid/waitpid01.c
 *   Original copyright message:
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * NAME
 *	move_pages08.c
 *
 * DESCRIPTION
 *      Failure when the no. of pages is ULONG_MAX.
 *
 * ALGORITHM
 *
 *      1. Pass ULONG_MAX pages to move_pages().
 *      2. Check if errno is set to E2BIG.
 *
 * USAGE:  <for command-line>
 *      move_pages08 [-c n] [-i n] [-I x] [-P x] [-t]
 *      where,  -c n : Run n copies concurrently.
 *              -i n : Execute test n times.
 *              -I x : Execute test for x seconds.
 *              -P x : Pause for x seconds between iterations.
 *              -t   : Turn on syscall timing.
 *
 * History
 *	05/2008 Vijay Kumar
 *		Initial Version.
 *
 * Restrictions
 *	kernel < 2.6.29
 */

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include "test.h"
#include "usctest.h"
#include "move_pages_support.h"

#define TEST_PAGES 2
#define TEST_NODES 2

static void setup(void);
static void cleanup(void);

char *TCID = "move_pages08";
int TST_TOTAL = 1;

int main(int argc, char **argv)
{
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	msg = parse_opts(argc, argv, NULL, NULL);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

#if HAVE_NUMA_MOVE_PAGES
	unsigned int i;
	int lc;			/* loop counter */
	unsigned int from_node;
	unsigned int to_node;
	int ret;

	ret = get_allowed_nodes(NH_MEMS, 2, &from_node, &to_node);
	if (ret < 0)
		tst_brkm(TBROK|TERRNO, cleanup, "get_allowed_nodes: %d", ret);

	/* check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		void *pages[TEST_PAGES] = { 0 };
		int nodes[TEST_PAGES];
		int status[TEST_PAGES];

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		ret = alloc_pages_on_node(pages, TEST_PAGES, from_node);
		if (ret == -1)
			continue;

		for (i = 0; i < TEST_PAGES; i++)
			nodes[i] = to_node;

		ret = numa_move_pages(0, ULONG_MAX, pages, nodes,
				      status, MPOL_MF_MOVE);
		TEST_ERRNO = errno;
		if (ret == -1 && errno == E2BIG)
			tst_resm(TPASS, "move_pages failed with "
				 "E2BIG as expected");
		else
			tst_resm(TFAIL, "move pages did not fail "
				 "with E2BIG");

		free_pages(pages, TEST_PAGES);
	}
#else
	tst_resm(TCONF, "move_pages support not found.");
#endif

	cleanup();
	tst_exit();

}

/*
 * setup() - performs all ONE TIME setup for this test
 */
static void setup(void)
{
	/*
	 * commit 3140a2273009c01c27d316f35ab76a37e105fdd8
	 * Author: Brice Goglin <Brice.Goglin@inria.fr>
	 * Date:   Tue Jan 6 14:38:57 2009 -0800
	 *     mm: rework do_pages_move() to work on page_sized chunks
	 *
	 * reworked do_pages_move() to work by page-sized chunks and removed E2BIG
	 */
	if ((tst_kvercmp(2, 6, 29)) >= 0)
		tst_brkm(TCONF, NULL, "move_pages: E2BIG was removed in "
			 "commit 3140a227");

	tst_sig(FORK, DEF_HANDLER, cleanup);

	check_config(TEST_NODES);

	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -c option.
	 */
	TEST_PAUSE;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at completion
 */
static void cleanup(void)
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

}
