/*
* Copyright (c) Bull S.A.  2007 All Rights Reserved.
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of version 2 of the GNU General Public License as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it would be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* Further, this software is distributed without any warranty that it is
* free of the rightful claim of any third person regarding infringement
* or the like.  Any license provided herein, whether implied or
* otherwise, applies only to this software file.  Patent licenses, if
* any, provided herein do not apply to combinations of this program with
* other software, or any other product whatsoever.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write the Free Software Foundation, Inc., 59
* Temple Place - Suite 330, Boston MA 02111-1307, USA.
*
* History:
* Created by: Cyril Lacabanne (Cyril.Lacabanne@bull.net)
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <tirpc/rpc/rpc.h>

//Standard define
#define INTPROCNUM 10
#define DBLPROCNUM 20
#define LNGPROCNUM 30
#define STRPROCNUM 40
#define VERSNUM 1

int main(int argn, char *argc[])
{
	//Program parameters : argc[1] : HostName or Host IP
	//					   argc[2] : Server Program Number
	//					   other arguments depend on test case

	//run_mode can switch into stand alone program or program launch by shell script
	//1 : stand alone, debug mode, more screen information
	//0 : launch by shell script as test case, only one printf -> result status
	int run_mode = 0;
	int test_status = 0; //Default test result set to PASSED
	int progNum = atoi(argc[2]);
	char nettype[16] = "visible";
	CLIENT *clnt = NULL;
	enum clnt_stat rslt;
    struct timeval total_timeout;
	//Sent variables
	int intSnd;
	double dblSnd;
	long lngSnd;
	char *strSnd;
	//Received variables
	int intRec;
	double dblRec;
	long lngRec;
	char *strRec;

	//Test initialization
	total_timeout.tv_sec = 1;
	total_timeout.tv_usec = 1;/**/

	//First of all, create client using top level API
	clnt = clnt_create(argc[1], progNum, VERSNUM, nettype);

	if (clnt == NULL)
	{
		printf("5\n");
		return 5;
	}

	//Call tested procedure several times
	//Int test : call INTPROCNUM RPC
	intSnd = -65536;

	clnt_call((CLIENT *)clnt, INTPROCNUM,
						    (xdrproc_t)xdr_int, (char *)&intSnd, // xdr_in
                    		(xdrproc_t)xdr_int, (char *)&intRec, // xdr_out
						    total_timeout);

	if (intSnd != intRec)
		test_status = 1;
	if (run_mode == 1)
		printf("Send (int) : %d, Received : %d\n", intSnd, intRec);

	//Test positive number
	intSnd = 16777216;

	clnt_call((CLIENT *)clnt, INTPROCNUM,
						    (xdrproc_t)xdr_int, (char *)&intSnd, // xdr_in
                    		(xdrproc_t)xdr_int, (char *)&intRec, // xdr_out
						    total_timeout);

	if (intSnd != intRec)
		test_status = 1;
	if (run_mode == 1)
		printf("Send (int) : %d, Received : %d\n", intSnd, intRec);

	//Long test : call LNGPROCNUM RPC
	lngSnd = -430000;

	clnt_call((CLIENT *)clnt, LNGPROCNUM,
						    (xdrproc_t)xdr_long, (char *)&lngSnd, // xdr_in
                    		(xdrproc_t)xdr_long, (char *)&lngRec, // xdr_out
						    total_timeout);

	if (lngSnd != lngRec)
		test_status = 1;
	if (run_mode == 1)
		printf("Send (long) : %ld, Received : %ld\n", lngSnd, lngRec);

	//Double test : call DBLPROCNUM RPC
	dblSnd = -1735.63000f;

	clnt_call((CLIENT *)clnt, DBLPROCNUM,
						    (xdrproc_t)xdr_double, (char *)&dblSnd, // xdr_in
                    		(xdrproc_t)xdr_double, (char *)&dblRec, // xdr_out
						    total_timeout);

	if (dblSnd != dblRec)
		test_status = 1;
	if (run_mode == 1)
		printf("Send (double) : %lf, Received : %lf\n", dblSnd, dblRec);

	//String test : call STRPROCNUM RPC
	strSnd = "text to send.";
	strRec = (char *)malloc(64 * sizeof(char));

	clnt_call((CLIENT *)clnt, STRPROCNUM,
				(xdrproc_t)xdr_wrapstring, (char *)&strSnd, // xdr_in
                (xdrproc_t)xdr_wrapstring, (char *)&strRec, // xdr_out
				total_timeout);

	if (strcmp(strSnd, strRec))
		test_status = 1;
	if (run_mode == 1)
		printf("Send (string) : %s, Received : %s\n", strSnd, strRec);

	//This last printf gives the result status to the tests suite
	//normally should be 0: test has passed or 1: test has failed
	printf("%d\n", test_status);

	return test_status;
}