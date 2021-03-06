/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

   Test that sigdelset() will remove signo to the set signal set.
   Test steps:
   1)  Initialize an empty or full signal set.
	--> if argv[1] = 0, empty
	--> if argv[1] = 1, full
   2)  Add the SIGALRM signal to the empty signal set.
   3)  Verify that SIGALRM is a member of the signal set.
   4)  Remove the SIGALRM signal from the signal set.
   5)  Verify that SIGALRM is not a member of the signal set.
 */
#include <stdio.h>
#include <signal.h>
#include "posixtest.h"

int main(int argc, char *argv[])
{
	sigset_t signalset;

	if (argc < 2) {
        	printf("Usage:  %s [0|1]\n", argv[0]);
		return PTS_UNRESOLVED;
	}
	if ('0' == argv[1][0]) {
		if (sigemptyset(&signalset) == -1) {
			perror("sigemptyset failed -- test aborted");
			return PTS_UNRESOLVED;
		}
	} else if ('1' == argv[1][0]) {
			if (sigfillset(&signalset) == -1) {
				perror("sigfillset failed -- test aborted");
				return PTS_UNRESOLVED;
			}
		} else {
			printf("Usage:  %s [0|1]\n", argv[0]);
			return PTS_UNRESOLVED;
		}

	if (sigaddset(&signalset, SIGALRM) == 0) {
		if (sigismember(&signalset, SIGALRM) == 0) {
			printf("sigaddset returned, sigismember failed\n");
			return PTS_UNRESOLVED;
		}
	} else {
		printf("sigaddset did not successfully add signal\n");
		return PTS_UNRESOLVED;
	}

	if (sigdelset(&signalset, SIGALRM) == 0) {
		if (sigismember(&signalset, SIGALRM) == 0) {
			printf("sigdelset successfully removed signal\n");
			return PTS_PASS;
		} else {
			printf("Signal is still in signal set.\n");
			return PTS_FAIL;
		}
	} else {
		printf("sigdelset() failed\n");
		return PTS_FAIL;
	}
}