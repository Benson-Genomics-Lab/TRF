/*
Tandem Repeats Finder
Copyright (C) 1999-2020 Gary Benson

This file is part of the Tandem Repeats Finder (TRF) program.

TRF is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of
the License, or (at your option) any later version.

TRF is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public
License along with TRF.  If not, see <https://www.gnu.org/licenses/>.
*/


/**************************************************************
 * trf.c :   Command line interface to the Tandem Repeats Finder
 *
 ***************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h> // ERANGE
#include <limits.h> // LONG_MIN, LONG_MAX
#include "trfrun.h" // paramset struct (via tr30dat.h), others

const char *usage = "\n\nPlease use: %s File Match Mismatch Delta PM PI Minscore MaxPeriod [options]\n"
"\nWhere: (all weights, penalties, and scores are positive)"
"\n  File = sequences input file"
"\n  Match  = matching weight"
"\n  Mismatch  = mismatching penalty"
"\n  Delta = indel penalty"
"\n  PM = match probability (whole number)"
"\n  PI = indel probability (whole number)"
"\n  Minscore = minimum alignment score to report"
"\n  MaxPeriod = maximum period size to report. Must be between 1 and 2000, inclusive"
"\n  [options] = one or more of the following:"
"\n        -m        masked sequence file"
"\n        -f        flanking sequence"
"\n        -d        data file"
"\n        -h        suppress html output"
"\n        -r        no redundancy elimination"
"\n        -l <n>    maximum TR length expected (in millions) (eg, -l 3 or -l=3 for 3 million)"
"\n                  Human genome HG38 would need -l 6"
#if (defined(UNIXGUI)+defined(UNIXCONSOLE))>=1
"\n        -ngs      more compact .dat output on multisequence files, returns 0 on success."
"\n                  Output is printed to the screen, not a file. You may pipe input in with"
"\n                  this option using - for file name. Short 50 flanks are appended to .dat"
"\n                  output."
#endif
"\n"
"\nSee more information on the TRF Unix Help web page: https://tandem.bu.edu/trf/trf.unix.help.html"
"\n"
"\nNote the sequence file should be in FASTA format:"
"\n"
"\n>Name of sequence"
"\naggaaacctgccatggcctcctggtgagctgtcctcatccactgctcgctgcctctccag"
"\natactctgacccatggatcccctgggtgcagccaagccacaatggccatggcgccgctgt"
"\nactcccacccgccccaccctcctgatcctgctatggacatggcctttccacatccctgtg"
"\n"
"\n"
"\nTandem Repeats Finder"
"\nCopyright (C) 1999-2020 Gary Benson"
"\n"
"\nThis program is free software: you can redistribute it and/or modify"
"\nit under the terms of the GNU Affero General Public License as"
"\npublished by the Free Software Foundation, either version 3 of"
"\nthe License, or (at your option) any later version."
"\n";

char* GetNamePartAddress(char* name);
void PrintBanner(void);
static int ParseInt(const char *str, int *dest);
static int ParseUInt(const char *str, unsigned int *dest);

int main(int ac, char** av)
{
	/* Handle a lone -v argument ourselves */
	if ( (ac == 2) && ((strcmp(av[1], "-v") == 0) || (strcmp(av[1], "-V") == 0)) ) {
		PrintBanner();
		exit(0);
	}

	/* Expects exactly 8 non-option arguments */
	if (ac < 9) {
		fprintf(stderr, usage, av[0]);
		exit(1);
	}

	/* set option defaults */
	paramset.use_stdin = 0;
	paramset.datafile = 0;
	paramset.maskedfile = 0;
	paramset.flankingsequence = 0;
	paramset.flankinglength = 500; /* Currently not user-configurable */
	paramset.HTMLoff = 0;
	paramset.redundoff = 0;
	paramset.maxwraplength = 2000000;
	paramset.ngs = 0; /* this is for unix systems only */
	paramset.guihandle=0;

	/* Parse command line options */
	/* Assume that since the first checks were passed, options start at argument 8
	getopt ignores the first array element, so start at element 8 */
	char **opt_arr = &av[8];
	int remaining_opts = ac - 8;
	while (1){
		static struct option long_options[] = {
			{"help", no_argument, 0, 'u'}, /* -u, -U */
			{"version", no_argument, 0, 'v'}, /* -v, -V */
			{"dat",    no_argument, &paramset.datafile, 1}, /* -d, -D */
			{"mask",    no_argument, &paramset.maskedfile, 1}, /* -m, -M */
			{"flank",    no_argument, &paramset.flankingsequence, 1}, /* -f, -F */
			{"html-off",    no_argument, &paramset.HTMLoff, 1}, /* -h, -H */
			{"redund-off",    no_argument, &paramset.redundoff, 1}, /* -r, -R */
#if (defined(UNIXGUI)+defined(UNIXCONSOLE))>=1
			{"ngs",   no_argument, &paramset.ngs, 1}, /* -ngs */
			{"Ngs",   no_argument, &paramset.ngs, 1}, /* -Ngs */
			{"NGS",   no_argument, &paramset.ngs, 1}, /* -NGS */
#endif
			{"maxlength",  required_argument, 0, 'l'}, /* -l, -L */
			{0, 0, 0, 0}
		};
		/* getopt_long stores the option index here. */
		int option_index = 0;

		/* Accept upper and lower-case variants of options */
		int c = getopt_long_only(remaining_opts, opt_arr, "uvdmfhrUVDMFHRl:L:",
			long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c)
		{
			case 0:
			if (long_options[option_index].flag == 0)
				break;

			/* Here you can use a function like strcasecmp to check if a
			particular flag long option was given, and then do something
			if you like (eg, print a message when testing). */

			// Example:
			/*if (strcasecmp(long_options[option_index].name, "ngs") == 0){
				printf("NGS option was given.\n");
			}
			*/

			break;

			case 'u':
			case 'U':
			printf(usage, av[0]);
			exit(0);
			break;

			case 'v':
			case 'V':
			PrintBanner();
			exit(0);
			break;

			case 'd':
			case 'D':
			paramset.datafile = 1;
			break;

			case 'm':
			case 'M':
			paramset.maskedfile = 1;
			break;

			case 'f':
			case 'F':
			paramset.flankingsequence = 1;
			break;

			case 'h':
			case 'H':
			paramset.HTMLoff = 1;
			break;

			case 'r':
			case 'R':
			paramset.redundoff = 1;
			break;

			case 'l':
			case 'L':
			if ((atol(optarg) < 1)){
				fprintf(stderr, "Error: max TR length must be at least 1 million\n");
				PrintBanner();
				exit(2);
			}

			if (ParseUInt(av[8], &paramset.maxwraplength) == 0) {
				fprintf(stderr, "Error while parsing max TR length (option '-L') value\n");
				PrintBanner();
				exit(1);
			}
			paramset.maxwraplength *= 1e6;

			break;

			case '?':
			/* getopt_long already printed an error message. */
			break;

			default:
			break;
		}
	}

	/* get input parameters */
	strcpy(paramset.inputfilename,av[1]);
	strcpy(paramset.outputprefix,GetNamePartAddress(av[1]));

	/* Validate these parameters */
	if (ParseUInt(av[2], &paramset.match) == 0) {
		paramset.endstatus = "Error parsing match parameter."
		" Value must be a positive integer.";
	}
	else if (ParseUInt(av[3], &paramset.mismatch) == 0) {
		paramset.endstatus = "Error parsing mismatch parameter."
		" Value must be a positive integer.";
	}
	else if (ParseUInt(av[4], &paramset.indel) == 0) {
		paramset.endstatus = "Error parsing indel parameter."
		" Value must be a positive integer.";
	}
	else if (ParseUInt(av[5], &paramset.PM) == 0) {
		paramset.endstatus = "Error parsing PM parameter."
		" Value must be a positive integer.";
	}
	else if (ParseUInt(av[6], &paramset.PI) == 0) {
		paramset.endstatus = "Error parsing PI parameter."
		" Value must be a positive integer.";
	}
	else if (ParseUInt(av[7], &paramset.minscore) == 0) {
		paramset.endstatus = "Error parsing Minscore parameter."
		" Value must be a positive integer.";
	}
	else if ((ParseUInt(av[8], &paramset.maxperiod) == 0) ||
		(paramset.maxperiod > 2000) || (paramset.maxperiod == 0)) {
		paramset.endstatus = "Error parsing MaxPeriod parameter."
		" Value must be between 1 and 2000, inclusive.";
	}

	/* Error if any validation failed */
	if(paramset.endstatus) {
		printf("%s\n\n", paramset.endstatus);
		printf("Please run with -h for help, or visit https://tandem.bu.edu/trf/trf.unix.help.html\n");
		exit(1);
	}

	// paramset.datafile must be set if HTMLoff is set
	paramset.datafile |= paramset.HTMLoff;

	if  (paramset.ngs == 1) {
		paramset.datafile=1;
	} else {
		PrintBanner();
	}

#if (defined(UNIXGUI)+defined(UNIXCONSOLE))>=1
	if (0==strcmp("-", av[1])) {
		if (paramset.ngs) {
			paramset.use_stdin = 1;
			paramset.HTMLoff = 1;
		} else {
			fprintf(stderr,"\n\nPlease use -ngs flag if piping input into TRF");
			fprintf(stderr,"\n");
			exit(-1);
		}
	}
#endif

	/* call the fuction on trfrun.h that controls execution */
	TRFControlRoutine();
	
	/* Check the status by looking at the output count and the endstatus
	   members of the paramset struct */
	if (paramset.outputcount == 0 && !paramset.endstatus) {
		/* If no TRs were found, print informative message to STDERR */
		printf("No TRs found. Exiting...\n");
		return 0;
	}
	else if (paramset.endstatus) {
		printf("Error processing input: %s\n", paramset.endstatus);
		return 1;
	}
	else {
		return 0;
	}
}


char* GetNamePartAddress(char* name)
{
	int i;
	char *pname;
#if defined(UNIXCONSOLE)||defined(UNIXGUI)
	char dirsymbol = '/';
#elif defined(WINDOWSCONSOLE)||defined(WINDOWSGUI)
	char dirsymbol = '\\';
#endif


	i = strlen(name)-1;
	pname = &name[i];
	while(i>0&&*pname!=dirsymbol)
	{
		pname--;
		i--;
	}
	if(*pname==dirsymbol) pname++;
	return pname;
}

// Use these two function for parameter validation
// TODO Move declarations to other header file
static int ParseInt(const char *str, int *dest)
{
    errno = 0;
    char *temp;
    long val = strtol(str, &temp, 0);

    // Print message and return false (0) if unable
    // to convert to a valid int
    if (temp == str || *temp != '\0' ||
        ((val == LONG_MIN || val == LONG_MAX) && errno == ERANGE)) {
        // fprintf(stderr, "Error parsing parameter '%s' as integer value.\n",
        //         str);
   		return 0;
	}

	// Set dest pointer and return true (1) if successful
	*dest = val;
    return 1;
}

static int ParseUInt(const char *str, unsigned int *dest)
{
	int temp;

    int success = ParseInt(str, &temp);

	// Return truth value of: conversion successful
	// AND the value is positive. Also set *dest if
	// those conditions are satisfied.
    if (success && (temp >= 0)) {
    	*dest = temp;
    	return 1;
    }
    else {
    	return 0;
    }
}

void    PrintBanner(void)
{
	fprintf(stderr,"\nTandem Repeats Finder, Version %s", versionstring);
	fprintf(stderr,"\nCopyright (C) Dr. Gary Benson 1999-2012. All rights reserved.\n");

	return;
}

