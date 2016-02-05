/**************************************************************
 * trf.c :   Command line interface to the Tandem Repeats Finder
 *
 ***************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "trfrun.h"



char* GetNamePartAddress(char* name);
void PrintBanner(void);


int main(int ac, char** av)
{
	char *pname;


	paramset.datafile = 0;
	paramset.maskedfile = 0;
	paramset.flankingsequence = 0;
	paramset.flankinglength = 500;
	paramset.HTMLoff = 0;
	paramset.redundoff = 0;
	paramset.maxwraplength = 0;
	paramset.ngs = 0;

	if(ac==10 || ac==11 || ac==12 || ac==13 || ac==14 || ac==15 || ac==16) {
		if(!strcmp(av[9],"-m") || !strcmp(av[9],"-M")) paramset.maskedfile=1;
		else if(!strcmp(av[9],"-f") || !strcmp(av[9],"-F")) paramset.flankingsequence=1;
			else if(!strcmp(av[9],"-d") || !strcmp(av[9],"-D")) paramset.datafile=1;
				else if(!strcmp(av[9],"-h") || !strcmp(av[9],"-H")) paramset.HTMLoff=1;
					else if(!strcmp(av[9],"-r") || !strcmp(av[9],"-R")) paramset.redundoff=1;
						else if(!strcmp(av[9],"-l") || !strcmp(av[9],"-L")) paramset.maxwraplength=1;
							else if(!strcmp(av[9],"-ngs") || !strcmp(av[9],"-Ngs") || !strcmp(av[9],"-NGS") || !strcmp(av[9],"-N") || !strcmp(av[9],"-n")) paramset.ngs=1;
								else ac=30; /* to force error message */
	}
	if(ac==11 || ac==12 || ac==13 || ac==14 || ac==15 || ac==16) {
		if(!strcmp(av[10],"-m") || !strcmp(av[10],"-M")) paramset.maskedfile=1;
		else if(!strcmp(av[10],"-f") || !strcmp(av[10],"-F")) paramset.flankingsequence=1;
			else if(!strcmp(av[10],"-d") || !strcmp(av[10],"-D")) paramset.datafile=1;
				else if(!strcmp(av[10],"-h") || !strcmp(av[10],"-H")) paramset.HTMLoff=1;
					else if(!strcmp(av[10],"-r") || !strcmp(av[10],"-R")) paramset.redundoff=1;
						else if(!strcmp(av[10],"-l") || !strcmp(av[10],"-L")) paramset.maxwraplength=1;
							else if(!strcmp(av[10],"-ngs") || !strcmp(av[10],"-Ngs") || !strcmp(av[10],"-NGS") || !strcmp(av[10],"-N") || !strcmp(av[10],"-n")) paramset.ngs=1;
								else ac=30; /* to force error message */
	}
	if(ac==12 || ac==13 || ac==14 || ac==15 || ac==16) {
		if(!strcmp(av[11],"-m") || !strcmp(av[11],"-M")) paramset.maskedfile=1;
		else if(!strcmp(av[11],"-f") || !strcmp(av[11],"-F")) paramset.flankingsequence=1;
			else if(!strcmp(av[11],"-d") || !strcmp(av[11],"-D")) paramset.datafile=1;
				else if(!strcmp(av[11],"-h") || !strcmp(av[11],"-H")) paramset.HTMLoff=1;
					else if(!strcmp(av[11],"-r") || !strcmp(av[11],"-R")) paramset.redundoff=1;
						else if(!strcmp(av[11],"-l") || !strcmp(av[11],"-L")) paramset.maxwraplength=1;
							else if(!strcmp(av[11],"-ngs") || !strcmp(av[11],"-Ngs") || !strcmp(av[11],"-NGS") || !strcmp(av[11],"-N") || !strcmp(av[11],"-n")) paramset.ngs=1;
								else ac=30; /* to force error message */
	} // TODO finish updating for new option: needs to scan from 13 to 16, all indexes in block are 12, not 11 (and so on for next blocks)
	if(ac==12 || ac==13 || ac==14 || ac==15) {
		if(!strcmp(av[11],"-m") || !strcmp(av[11],"-M")) paramset.maskedfile=1;
		else if(!strcmp(av[11],"-f") || !strcmp(av[11],"-F")) paramset.flankingsequence=1;
			else if(!strcmp(av[11],"-d") || !strcmp(av[11],"-D")) paramset.datafile=1;
				else if(!strcmp(av[11],"-h") || !strcmp(av[11],"-H")) paramset.HTMLoff=1;
					else if(!strcmp(av[11],"-r") || !strcmp(av[11],"-R")) paramset.redundoff=1;
						else if(!strcmp(av[11],"-l") || !strcmp(av[11],"-L")) paramset.maxwraplength=1;
							else if(!strcmp(av[11],"-ngs") || !strcmp(av[11],"-Ngs") || !strcmp(av[11],"-NGS") || !strcmp(av[11],"-N") || !strcmp(av[11],"-n")) paramset.ngs=1;
								else ac=30; /* to force error message */
	}
	if(ac==13 || ac==14 || ac==15) {
		if(!strcmp(av[12],"-m") || !strcmp(av[12],"-M")) paramset.maskedfile=1;
		else if(!strcmp(av[12],"-f") || !strcmp(av[12],"-F")) paramset.flankingsequence=1;
			else if(!strcmp(av[12],"-d") || !strcmp(av[12],"-D")) paramset.datafile=1;
				else if(!strcmp(av[12],"-h") || !strcmp(av[12],"-H")) paramset.HTMLoff=1;
					else if(!strcmp(av[12],"-r") || !strcmp(av[12],"-R")) paramset.redundoff=1;
						else if(!strcmp(av[12],"-l") || !strcmp(av[12],"-L")) paramset.maxwraplength=1;
							else if(!strcmp(av[12],"-ngs") || !strcmp(av[12],"-Ngs") || !strcmp(av[12],"-NGS") || !strcmp(av[12],"-N") || !strcmp(av[12],"-n")) paramset.ngs=1;
								else ac=30; /* to force error message */
	}
	if(ac==14 || ac==15) {
		if(!strcmp(av[13],"-m") || !strcmp(av[13],"-M")) paramset.maskedfile=1;
		else if(!strcmp(av[13],"-f") || !strcmp(av[13],"-F")) paramset.flankingsequence=1;
			else if(!strcmp(av[13],"-d") || !strcmp(av[13],"-D")) paramset.datafile=1;
				else if(!strcmp(av[13],"-h") || !strcmp(av[13],"-H")) paramset.HTMLoff=1;
					else if(!strcmp(av[13],"-r") || !strcmp(av[13],"-R")) paramset.redundoff=1;
						else if(!strcmp(av[13],"-l") || !strcmp(av[13],"-L")) paramset.maxwraplength=1;
							else if(!strcmp(av[13],"-ngs") || !strcmp(av[13],"-Ngs") || !strcmp(av[13],"-NGS") || !strcmp(av[13],"-N") || !strcmp(av[13],"-n")) paramset.ngs=1;
								else ac=30; /* to force error message */
	}
	if(ac==15) {
		if(!strcmp(av[14],"-m") || !strcmp(av[14],"-M")) paramset.maskedfile=1;
		else if(!strcmp(av[14],"-f") || !strcmp(av[14],"-F")) paramset.flankingsequence=1;
			else if(!strcmp(av[14],"-d") || !strcmp(av[14],"-D")) paramset.datafile=1;
				else if(!strcmp(av[14],"-h") || !strcmp(av[14],"-H")) paramset.HTMLoff=1;
					else if(!strcmp(av[14],"-r") || !strcmp(av[14],"-R")) paramset.redundoff=1;
						else if(!strcmp(av[14],"-l") || !strcmp(av[14],"-L")) paramset.maxwraplength=1;
							else if(!strcmp(av[14],"-ngs") || !strcmp(av[14],"-Ngs") || !strcmp(av[14],"-NGS") || !strcmp(av[14],"-N") || !strcmp(av[14],"-n")) paramset.ngs=1;
								else ac=30; /* to force error message */
	}


#if (defined(UNIXGUI)+defined(UNIXCONSOLE))<1
	paramset.ngs = 0; /* this is for unix systems only */
#endif


	if  (paramset.ngs == 1) {
		paramset.datafile=1;
	} else {
		PrintBanner();
	}


	if ( ((ac!=9) && (ac!=10) && (ac!=11) && (ac!=12) && (ac!=13) && (ac!=14)) || (atoi(av[8])==0))
	{
		fprintf(stderr,"\n\nPlease use: %s File Match Mismatch Delta PM PI Minscore MaxPeriod [options]\n", av[0]);
		fprintf(stderr,"\nWhere: (all weights, penalties, and scores are positive)");
		fprintf(stderr,"\n  File = sequences input file");
		fprintf(stderr,"\n  Match  = matching weight"); 
		fprintf(stderr,"\n  Mismatch  = mismatching penalty"); 
		fprintf(stderr,"\n  Delta = indel penalty");
		fprintf(stderr,"\n  PM = match probability (whole number)");
		fprintf(stderr,"\n  PI = indel probability (whole number)");
		fprintf(stderr,"\n  Minscore = minimum alignment score to report");
		fprintf(stderr,"\n  MaxPeriod = maximum period size to report");
		fprintf(stderr,"\n  [options] = one or more of the following :");
		fprintf(stderr,"\n               -m    masked sequence file");
		fprintf(stderr,"\n               -f    flanking sequence");
		fprintf(stderr,"\n               -d    data file");
		fprintf(stderr,"\n               -h    suppress html output");
		fprintf(stderr,"\n               -r    no redundancy elimination");
		fprintf(stderr,"\n               -l    maximum TR length expected");
#if (defined(UNIXGUI)+defined(UNIXCONSOLE))>=1
		fprintf(stderr,"\n               -ngs  more compact .dat output on multisequence files, returns 0 on success. You may pipe input in with this option using - for file name. Short 50 flanks are appended to .dat output. See more information on TRF Unix Help web page.");
#endif
		fprintf(stderr,"\n");
		fprintf(stderr,"\nNote the sequence file should be in FASTA format:");
		fprintf(stderr,"\n");
		fprintf(stderr,"\n>Name of sequence");
		fprintf(stderr,"\n   aggaaacctg ccatggcctc ctggtgagct gtcctcatcc actgctcgct gcctctccag");
		fprintf(stderr,"\n   atactctgac ccatggatcc cctgggtgca gccaagccac aatggccatg gcgccgctgt");
		fprintf(stderr,"\n   actcccaccc gccccaccct cctgatcctg ctatggacat ggcctttcca catccctgtg");
		fprintf(stderr,"\n");

		exit(-1);
	}



	/* get other input parameters */
	strcpy(paramset.inputfilename,av[1]);
	paramset.use_stdin = 0;
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

	pname = GetNamePartAddress(av[1]);
	strcpy(paramset.outputprefix,pname);
	paramset.match    = atoi(av[2]);
	paramset.mismatch = atoi(av[3]);
	paramset.indel    = atoi(av[4]);
	paramset.PM = atoi(av[5]);
	paramset.PI = atoi(av[6]);
	paramset.minscore = atoi(av[7]);
	paramset.maxperiod = atoi(av[8]);
	paramset.guihandle=0;
	if(paramset.HTMLoff) paramset.datafile=1;

	/* call the fuction on trfrun.h that controls execution */
	if (paramset.ngs) {

		int rc = TRFControlRoutine();

		if (rc>=1) 
			return 0;
		if (rc==0) 
			return CTRL_NOTHINGPROCESSED;
		else 
			return rc;
	} else {
		return TRFControlRoutine();
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

void    PrintBanner(void)
{
	fprintf(stderr,"\nTandem Repeats Finder, Version %s", versionstring);
	fprintf(stderr,"\nCopyright (C) Dr. Gary Benson 1999-2012. All rights reserved.\n");

	return;
}

