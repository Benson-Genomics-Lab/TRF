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
 *   trfrun.h :  This file contains the code that calls the TRF
 *               algorithm.
 *               It declares all the variables that control how
 *               the program runs and where the output is saved.
 *               If the input file contains more than one
 *               sequence then the input in broken into single
 *               sequences by the control routine and fed to the
 *               algorithm one sequence at a time.
 *               The output is assembled by the control routine
 *               as described in the file readme.txt
 *
 *                                           December 10, 2001
 *


 Last updated Dec 14,2004
 ***************************************************************/


#ifndef TRFRUN_H
#define TRFRUN_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

/* Added by Yozen to explicitly include header for windows GUI definitons on Jan 25, 2016 */
#ifdef WINDOWSGUI
 #include <windows.h>
 #include <windef.h>
 #include <winuser.h>
#endif

/* These declarations moved by Yevgeniy Gelfand on Jan 27, 2010  */
/* To have smaller sequences not send results */
/* to disc to improve performance             */

/* define Index List structure */

struct index_list
{
	int     count;      /* indicates order in original file */
	char    ref[45];    /* records label for linking */
	int     first;      /* first index */
	int     last;       /* last index */
	int     period;     /* period size */
	float   copies;     /* number of copies */
	int     size;  /* consensus size */
	int     matches;
	int     indels;
	int     score;
	int     acount;
	int     ccount;
	int     gcount;
	int     tcount;
	float   entropy;
	char*   pattern;

	struct index_list* next;
};
typedef struct index_list IL;

IL *GlobalIndexList = NULL;
IL *GlobalIndexListTail = NULL;

void    FreeList(IL * headptr);

/* end of changes  on Jan 27, 2010  */

#include "tr30dat.h"
#include "tr30dat.c"
#include "trfclean.h"




#ifndef _MAX_PATH
#define _MAX_PATH 260
#endif

// how much memory to allocate at first when loading a sequence
// (added by Eugene Scherba, 2010-02-16)
#define MEMBLOCK (10*1024*1024)


int LoadSequenceFromFileEugene(FASTASEQUENCE *pseq,FILE* fp); /* may use stdin and better file reading for handling large files over 2GB */
int LoadSequenceFromFileBenson(FASTASEQUENCE *pseq,FILE* fp); /* old function, uses filepos, 32bit version of this would not process a file over 2GB properly */
void TRFControlRoutine(void);
void TRF(FASTASEQUENCE* pseq);
void PrintError(char* errortext);
void PrintProgress(char* progresstext);
void SetProgressBar(void);

#ifdef UNIXGUI
extern void set_progress_bar( double fraction );
#endif

/***********************************************
 *   This routine can act on a multiple-sequence
 *   file and calls TRF() routine as many times
 *   as it needs to.
 ************************************************/

void TRFControlRoutine(void)
{
	FILE *srcfp,*outmfp,*destmfp,*destdfp = NULL;
	char  source[_MAX_PATH],input[_MAX_PATH],outm[_MAX_PATH],
	      prefix[_MAX_PATH],destm[_MAX_PATH],destd[_MAX_PATH],
	      desth[_MAX_PATH],paramstring[_MAX_PATH],outh[_MAX_PATH];
	int a,i,loadstatus,foundsome=0;
	unsigned int tr_count = 0;
	char  line[1000];
	FILE  *desthfp;
	FASTASEQUENCE seq;


	/* save names locally so they can be replaced later */
	strcpy(source,paramset.inputfilename);
	strcpy(prefix,paramset.outputprefix);


	/* open input file for reading */
	if (paramset.use_stdin) {
		srcfp = stdin;
	} else {
		srcfp = fopen(source,"rb");
		if(srcfp==NULL)
		{
			sprintf(line,"Error opening %s: %s\n",source, strerror(errno));
			PrintError(line);
			paramset.endstatus = "Bad filename";
			paramset.running = 0;
			// return CTRL_BADFNAME;
			return;
		}
	}

	/* get the first sequence */
	if (paramset.ngs != 1) PrintProgress("Loading sequence...");


	loadstatus = LoadSequenceFromFileEugene(&seq,srcfp);


	if(loadstatus<0)
	{
		PrintError("Could not load sequence. Empty file or bad format.");
		paramset.endstatus = "Bad format."; /* ok for now */
		paramset.running = 0;
		fclose(srcfp);
		// return CTRL_BADFORMAT;
		return;
	}


	/* generate the parameter string to be used in file names */
	sprintf(paramstring,"%d.%d.%d.%d.%d.%d.%d",
			paramset.match,paramset.mismatch,paramset.indel,
			paramset.PM,paramset.PI,paramset.minscore,paramset.maxperiod);


	sprintf(hparameters,"Parameters: %d %d %d %d %d %d %d\n",
			paramset.match,paramset.mismatch,paramset.indel,
			paramset.PM,paramset.PI,paramset.minscore,paramset.maxperiod);



	/* based on number of sequences in file use different approach */
	if(loadstatus==0) /* only one sequence in file */
	{

		sprintf(hsequence,"Sequence: %s\n",seq.name);
		sprintf(hlength,"Length:  %d",seq.length);

		paramset.multisequencefile = 0;
		paramset.sequenceordinal = 1;
		/* call trf and return */
		counterInSeq=0;
		TRF(&seq);

		if (paramset.endstatus) {
			return;
		}

		if(paramset.datafile)
		{


			/* Added by Yevgeniy Gelfand on Jan 27, 2010  */
			/* To have smaller sequences not send results */
			/* to disc to improve performance             */



	                if (paramset.ngs) {
        	                destdfp = stdout;
	                } else {
        	                sprintf(destd,"%s.%s.dat",prefix,paramstring);
	                        destdfp = fopen(destd,"w");
        	                if(destdfp==NULL)
                	        {
                        	        PrintError("Unable to open data file for writing in TRFControlRoutine routine!");
	                                exit(-1);
        	                }
	                }


			{

				IL* lpointer;
				int charcount;

				if (paramset.ngs != 1) {
					fprintf(destdfp,"Tandem Repeats Finder Program written by:\n\n");
					fprintf(destdfp,"Gary Benson\n");
					fprintf(destdfp,"Program in Bioinformatics\n");
					fprintf(destdfp,"Boston University\n");
					fprintf(destdfp,"Version %s\n", versionstring);
				}

				//fprintf(destdfp,"\n\n%s%s",hsequence, hparameters);

				if (paramset.ngs) {

					/* only print if we have at least 1 record */
					if (NULL!=GlobalIndexList) {
						fprintf(destdfp,"@%s\n",seq.name);
					}

				} else {
					fprintf(destdfp,"\n\nSequence: %s\n\n\n\nParameters: %d %d %d %d %d %d %d\n\n\n",
							seq.name,paramset.match,paramset.mismatch,paramset.indel,paramset.PM,paramset.PI,paramset.minscore,paramset.maxperiod);
				}

				for(lpointer=GlobalIndexList;lpointer!=NULL;lpointer=lpointer->next)
				{
					fprintf(destdfp,"%d %d %d %.1f %d %d %d %d %d %d %d %d %.2f %s ",
							lpointer->first, lpointer->last, lpointer->period,
							lpointer->copies, lpointer->size, lpointer->matches,
							lpointer->indels, lpointer->score, lpointer->acount,
							lpointer->ccount, lpointer->gcount, lpointer->tcount,
							lpointer->entropy, lpointer->pattern );
					for(charcount=lpointer->first; charcount<=lpointer->last;charcount++)
						fprintf(destdfp,"%c", Sequence[charcount]);

					/* print short flanks to .dat file */
					if (paramset.ngs) {
						int flankstart,flankend;

	  					flankstart = lpointer->first - 50; 
						flankstart=max(1,flankstart);
						flankend = lpointer->last + 50; 
						flankend=min(Length,flankend);

						fprintf(destdfp," ");				
						if (lpointer->first == 1) {
							fprintf(destdfp,".");
						} else {
							for(charcount=flankstart; charcount<lpointer->first;charcount++)
								fprintf(destdfp,"%c", Sequence[charcount]);
						}

						fprintf(destdfp," ");	
						if (lpointer->last == Length) {
							fprintf(destdfp,".");
						} else {
							for(charcount=lpointer->last+1; charcount<=flankend;charcount++)
								fprintf(destdfp,"%c", Sequence[charcount]);
						}
			
					}
					
					fprintf(destdfp,"\n");
					++tr_count;
				}


			} 

		}


		/* masked file moved here so Sequence is not "ruined" by Ns for .dat output */
		{		
			char maskstring[_MAX_PATH];
			sprintf(maskstring,"%s.%s.mask",paramset.outputprefix,paramstring);
			MakeMaskedFile(GlobalIndexList, paramset.maskedfile, Sequence, maskstring);
		}


		FreeList(GlobalIndexList);
		GlobalIndexList = NULL;
		GlobalIndexListTail = NULL;


		free(seq.sequence);
		fclose(srcfp);

		if(destdfp) { fclose(destdfp); destdfp = NULL; }

		paramset.endstatus = NULL;
		paramset.running = 0;
		// return CTRL_SUCCESS;
		return;
	}
	paramset.multisequencefile = 1;
	paramset.sequenceordinal = 1;

	/*********************************************************
	 *   if there are more files need to produce sumary-style
	 *   output.
	 **********************************************************/

	/* generate the parameter string to be used in file names */
	sprintf(paramstring,"%d.%d.%d.%d.%d.%d.%d",
			paramset.match,paramset.mismatch,paramset.indel,
			paramset.PM,paramset.PI,paramset.minscore,paramset.maxperiod);

	/* open sumary table file */
	sprintf(desth,"%s.%s.summary.html",prefix,paramstring);
	if (!paramset.HTMLoff) {
		desthfp = fopen(desth,"w");
		if(desthfp==NULL)
		{
			PrintError("Unable to open summary file for writing in TRFControlRoutine routine!");
			exit(-1);
		}
	}

	/* open masked file if requested */
	if(paramset.maskedfile)
	{
		sprintf(destm,"%s.%s.mask",prefix,paramstring);
		destmfp = fopen(destm,"w");
		if(destmfp==NULL)
		{
			PrintError("Unable to open masked file for writing in TRFControlRoutine routine!");
			exit(-1);
		}
	}
	/* open datafile if requested */
	if(paramset.datafile)
	{
		if (paramset.ngs) {
			destdfp = stdout;
		} else {
			sprintf(destd,"%s.%s.dat",prefix,paramstring);
			destdfp = fopen(destd,"w");
			if(destdfp==NULL)
			{
				PrintError("Unable to open data file for writing in TRFControlRoutine routine!");
				exit(-1);
			}
		}
	}

	if (!paramset.HTMLoff) {
		/* start output of sumary file */
		fprintf(desthfp,"<HTML>");
		fprintf(desthfp,"<HEAD>");
		fprintf(desthfp,"<TITLE>Output Summary</TITLE>");
		/* fprintf(desthfp,"<BASE TARGET=\"Table\">"); */
		fprintf(desthfp,"</HEAD>");
		fprintf(desthfp,"<BODY bgcolor=\"#FBF8BC\">");
		fprintf(desthfp,"<PRE>");

		fprintf(desthfp,"\nTandem Repeats Finder Program written by:<CENTER>");
		fprintf(desthfp,"\nGary Benson");
		fprintf(desthfp,"\nProgram in Bioinformatics");
		fprintf(desthfp,"\nBoston University");
		fprintf(desthfp,"\nVersion %s</CENTER>\n", versionstring);
		fprintf(desthfp,"\nPlease cite:\nG. Benson,\n\"Tandem repeats finder: a program to analyze DNA sequences\"\nNucleic Acid Research(1999)\nVol. 27, No. 2, pp. 573-580.\n");


		fprintf(desthfp,"\n\n<B>Multiple Sequence Summary</B>\n\n");
		fprintf(desthfp,"Only sequences containing repeats are shown!\n\n");
		fprintf(desthfp,"Click on sequence description to view repeat table.\n\n");

		/* print beginning of table */
		fprintf(desthfp,"<TABLE BORDER=1 CELLSPACING=0 CELLPADDING=0>\n");
		fprintf(desthfp,"<TR><TD WIDTH=80><CENTER>Sequence\nIndex</CENTER></TD>"
				"<TD WIDTH=400><CENTER>Sequence\nDescription</CENTER></TD>"
				"<TD WIDTH=80><CENTER>Number of\nRepeats</CENTER></TD>"
				"</TR>\n");
	}

	/******************************************
	 *   process every sequence in file
	 *******************************************/
	i=1;
	for(;;)
	{

		sprintf(hsequence,"Sequence: %s\n",seq.name);
		sprintf(hlength,"Length:  %d",seq.length);

		/* set the prefix to be used for naming of output */
		sprintf(input,"%s.s%d",prefix,i);
		strcpy(paramset.inputfilename,input);
		strcpy(paramset.outputprefix,input);

		/* call the tandem repeats finder routine */
		counterInSeq=0;
		TRF(&seq);

		if(paramset.datafile)
		{

			/* Added by Yevgeniy Gelfand on Jan 27, 2010  */
			/* To have smaller sequences not send results */
			/* to disc to improve performance             */
			{

				IL* lpointer;
				int charcount;

				/* only for the first one write the header */
				if (i==1) {
					if (paramset.ngs != 1) {
						fprintf(destdfp,"Tandem Repeats Finder Program written by:\n\n");
						fprintf(destdfp,"Gary Benson\n");
						fprintf(destdfp,"Program in Bioinformatics\n");
						fprintf(destdfp,"Boston University\n");
						fprintf(destdfp,"Version %s\n", versionstring);
					}
				}

				//fprintf(destdfp,"\n\n%s%s",hsequence, hparameters);

				if (paramset.ngs) {

					/* only print if we have at least 1 record */
					if (NULL!=GlobalIndexList) {
						fprintf(destdfp,"@%s\n",seq.name);
					}

				} else {
					fprintf(destdfp,"\n\nSequence: %s\n\n\n\nParameters: %d %d %d %d %d %d %d\n\n\n",
							seq.name,paramset.match,paramset.mismatch,paramset.indel,paramset.PM,paramset.PI,paramset.minscore,paramset.maxperiod);
				}

				for(lpointer=GlobalIndexList;lpointer!=NULL;lpointer=lpointer->next)
				{
					fprintf(destdfp,"%d %d %d %.1f %d %d %d %d %d %d %d %d %.2f %s ",
							lpointer->first, lpointer->last, lpointer->period,
							lpointer->copies, lpointer->size, lpointer->matches,
							lpointer->indels, lpointer->score, lpointer->acount,
							lpointer->ccount, lpointer->gcount, lpointer->tcount,
							lpointer->entropy, lpointer->pattern );
					for(charcount=lpointer->first; charcount<=lpointer->last;charcount++)
						fprintf(destdfp,"%c", Sequence[charcount]);

					/* print short flanks to .dat file */
					if (paramset.ngs) {
						int flankstart,flankend;

	  					flankstart = lpointer->first - 50; 
						flankstart=max(1,flankstart);
						flankend = lpointer->last + 50; 
						flankend=min(Length,flankend);

						fprintf(destdfp," ");				
						if (lpointer->first == 1) {
							fprintf(destdfp,".");
						} else {
							for(charcount=flankstart; charcount<lpointer->first;charcount++)
								fprintf(destdfp,"%c", Sequence[charcount]);
						}

						fprintf(destdfp," ");	
						if (lpointer->last == Length) {
							fprintf(destdfp,".");
						} else {
							for(charcount=lpointer->last+1; charcount<=flankend;charcount++)
								fprintf(destdfp,"%c", Sequence[charcount]);
						}
			
					}
					
					fprintf(destdfp,"\n");
				}


			} 

		}

		if (!paramset.HTMLoff) {
			/* print table rows based on repeat count */
			sprintf(outh,"%s.%s.1.html",input,paramstring);
			if(paramset.outputcount>0)
			{
				/* print a table raw to the summary table */
				fprintf(desthfp,"<TR><TD><CENTER>%d</CENTER></TD>"
						"<TD><CENTER><A TARGET=\"%s\" HREF=\"%s\">%s</A>"
						"</CENTER></TD><TD><CENTER>%d</CENTER></TD></TR>",
						i,outh,outh,seq.name,paramset.outputcount);
				foundsome=1;
			}
			else
			{
				/* remove html files if no output in it */
				remove(outh);
				sprintf(line,"%s.%s.1.txt.html",input,paramstring);
				remove(line);
			}
		}


		/* masked file moved here so Sequence is not "ruined" by Ns for .dat output */
		{		
			char maskstring[_MAX_PATH];
			sprintf(maskstring,"%s.s%d.%s.mask",prefix,i,paramstring);
			MakeMaskedFile(GlobalIndexList, paramset.maskedfile, Sequence, maskstring);
		}


		/* append new output to destination files */
		if(paramset.maskedfile)
		{
			/* recreate the name of the masked sequence file */
			sprintf(outm,"%s.s%d.%s.mask",prefix,i,paramstring);
			outmfp = fopen(outm,"r");
			if(outmfp==NULL)
			{
				PrintError("Unable to open masked file for reading in TRFControlRoutine routine!");
				exit(-1);
			}
			/* copy until end of file */
			while(1)
			{
				a = getc(outmfp);
				if(a==EOF) break;
				putc(a,destmfp);
			}
			fclose(outmfp);

			/* remove intermediary file */
			remove(outm);
		}



		FreeList(GlobalIndexList);
		GlobalIndexList = NULL;
		GlobalIndexListTail = NULL;


		/* free the data associated with the sequence */
		free(seq.sequence);

		/* if more sequences load and repeat */
		if(loadstatus>0)
		{
			if (paramset.ngs != 1) PrintProgress("Loading sequence file...");

			loadstatus = LoadSequenceFromFileEugene(&seq,srcfp);

			paramset.sequenceordinal++;
			i++;
		}
		else
		{
			break;
		}
	}

	if (!paramset.HTMLoff) {
		/* close table and html body */
		fprintf(desthfp,"\n</TABLE>\n");
		if(!foundsome)
		{
			fprintf(desthfp, "\nNo Repeats Found!<BR>");
		}
		fprintf(desthfp,"\n</BODY></HTML>\n");
	}

	/* close files */
	fclose(srcfp);
	if(paramset.maskedfile) fclose(destmfp);
	if(paramset.datafile) fclose(destdfp);

	if (!paramset.HTMLoff)
		fclose(desthfp);

	/* set output file name to the summary table */
	strcpy(paramset.outputfilename,desth);

	paramset.endstatus = NULL;
	paramset.running = 0;
	// return CTRL_SUCCESS;
	return;
}

/*************************************************
 *   This routine acts on single-sequence files and
 *   is used by the control routine above.
 **************************************************/
void TRF(FASTASEQUENCE* pseq)
{
	unsigned int i;  /* used at the end to free memory */
	int *stemp;
	char htmlstring[_MAX_PATH], txtstring[_MAX_PATH],
	     paramstring[_MAX_PATH],datstring[_MAX_PATH],
	     maskstring[_MAX_PATH],messagebuffer[100];

	/* added Nov 1, 2012 by Y. Gelfand */
	init_bestperiodlist();


	/*  Set global print_flanking that controls the generation of flanking */
	print_flanking = paramset.flankingsequence;

	/* allocate memory for file names */
	if (paramset.ngs != 1) PrintProgress("Allocating Memory...");


	/* change made for NGS data analysis */
	/* G. Benson */
	/* 1.26.10 */
	/* make MAXWRAPLENGTH = 1000 for smaller for small sequences */
	maxwraplength = min(paramset.maxwraplength,pseq->length);

	/* allocate memory */
	S = (int **) malloc((maxwraplength+1) *  sizeof(int *));
	if(S==NULL)
	{
		PrintError("Unable to allocate memory for S array");
		exit(-1);
	}
	// stemp = (int *) malloc(((MAXWRAPLENGTH+1)*(MAXBANDWIDTH+1)) * sizeof(int));
	// memset(stemp,0,((MAXWRAPLENGTH+1)*(MAXBANDWIDTH+1)) * sizeof(int));
	/* Yozen Jan 26, 2016: We control the compilation and we're going to be using C99
	or greater standard C; don't need to cast, and we can use the pointer to determine the size.
	Also, use calloc instead of malloc+memset. */
	stemp = calloc(((size_t)(maxwraplength+1)*(MAXBANDWIDTH+1)), sizeof(*stemp));
	if(stemp==NULL)
	{
		char errmsg[255];
		#if (__x86_64__ + __x86_64 + __amd64 + __amd64__ + _M_AMD64 + _M_X64) > 1
		snprintf(errmsg, 255, "Unable to allocate %lu bytes for stemp array. Please set a lower value for the longest TR length. (%s:%d)\n", ((maxwraplength+1)*(MAXBANDWIDTH+1)) * sizeof(*stemp), __FILE__, __LINE__);
		#else
		snprintf(errmsg, 255, "Unable to allocate %u bytes for stemp array. Please set a lower value for the longest TR length. (%s:%d)\n", ((maxwraplength+1)*(MAXBANDWIDTH+1)) * sizeof(*stemp), __FILE__, __LINE__);
		#endif
		PrintError(errmsg);
		exit(-1);
	}
	for(i=0;i<=maxwraplength;i++)
	{
		S[i]= stemp;
		stemp+=MAXBANDWIDTH+1;
	}
	S[0][0]=1;

	/* AlignPair holds the characters and alignments of the current */
	/* primary and secondary sequences  */
	AlignPair.textprime=newAlignPairtext(2*maxwraplength);
	if (paramset.endstatus) {
		return;
	}
	AlignPair.textsecnd=newAlignPairtext(2*maxwraplength);
	if (paramset.endstatus) {
		return;
	}
	AlignPair.indexprime=newAlignPairindex(2*maxwraplength);
	if (paramset.endstatus) {
		return;
	}
	AlignPair.indexsecnd=newAlignPairindex(2*maxwraplength);
	if (paramset.endstatus) {
		return;
	}

	/* set algorithm's parameters */
	Alpha = paramset.match;
	Beta  = -paramset.mismatch;
	Delta = -paramset.indel;
	PM    = paramset.PM;
	PI    = paramset.PI;
	Minscore = paramset.minscore;
	MaxPeriod= paramset.maxperiod;
	MAXDISTANCE=MAXPATTERNSIZE=paramset.maxperiod;
	if(MAXDISTANCE<500)
	{
		MAXDISTANCE = MAXPATTERNSIZE=500;
	}

	MAXDISTANCE = MAXPATTERNSIZE = min( MAXDISTANCE, (int) (pseq->length * .6));
	MAXDISTANCE = MAXPATTERNSIZE = max( MAXDISTANCE, 200);


	/* generate the parameter string to be used in file names */
	sprintf(paramstring,"%d.%d.%d.%d.%d.%d.%d",
			paramset.match,paramset.mismatch,paramset.indel,
			paramset.PM,paramset.PI,paramset.minscore,paramset.maxperiod);


	Reportmin= 0;
	ldong=0;
	Rows=0;
	Totalcharacters=0;
	Test=1;

	/* print the names of the files */
	sprintf(htmlstring,"%s.%s.html",paramset.outputprefix,paramstring);
	sprintf(txtstring ,"%s.%s.txt.html",paramset.outputprefix,paramstring);
	sprintf(datstring ,"%s.%s.dat",paramset.outputprefix,paramstring);
	sprintf(maskstring,"%s.%s.mask",paramset.outputprefix,paramstring);

	/* start txt file */
	if (!paramset.HTMLoff) {
		Fptxt=fopen(txtstring,"w");
		if(Fptxt==NULL)
		{
			PrintError("Unable to open alignment file for writing in TRF() routine!");
			exit(-1);
		}



		fprintf(Fptxt,"<HTML>");
		fprintf(Fptxt,"<HEAD>");
		fprintf(Fptxt,"<TITLE>%s</TITLE>",txtstring);
		fprintf(Fptxt,"</HEAD>");
		fprintf(Fptxt,"<BODY bgcolor=\"#FBF8BC\">");
		fprintf(Fptxt,"<PRE>");

		fprintf(Fptxt,"\nTandem Repeats Finder Program written by:");
		fprintf(Fptxt,"\n\n                 Gary Benson");
		fprintf(Fptxt,"\n      Program in Bioinformatics");
		fprintf(Fptxt,"\n          Boston University");
		fprintf(Fptxt,"\n\nVersion %s", versionstring);

		fprintf(Fptxt,"\n\nSequence: %s\n\nParameters: %d %d %d %d %d %d %d\n",
				pseq->name,paramset.match,paramset.mismatch,paramset.indel,
				paramset.PM,paramset.PI,paramset.minscore,paramset.maxperiod);
	}



	if (paramset.ngs != 1) PrintProgress("Initializing data structures...");



	Distance=new_distancelist();
	clear_distancelist(Distance);
	Tag=newTags(MAXDISTANCE/TAGSEP+1);
	Toptag=(int)ceil(MAXDISTANCE/TAGSEP);
	init_links();

	init_index();
	/* init_distanceseenlist(); */
	/* modified 5/23/05 G. Benson */
	init_distanceseenarray();


	if (paramset.ngs != 1) PrintProgress("Computing TR Model Statistics...");

	init_and_fill_coin_toss_stats2000_with_4tuplesizes();

	/* over allocate statistics_distance array to prevent spill in alignments
	   with execive insertion counts Jan 07, 2003 */
	Statistics_Distance=(int *)calloc(4*MAXDISTANCE,sizeof(int));
	if(Statistics_Distance==NULL)
	{
		PrintError("Unable to allocate memory for Statistics_Distance array");
		exit(-3);
	}



	/* set the sequence pointer. more global vars! */
	Sequence = pseq->sequence-1; /* start one character before */
	Length=pseq->length;

	if (!paramset.HTMLoff) {
		fprintf(Fptxt,"\n\nLength: %d",Length);
		fprintf(Fptxt,"\nACGTcount: A:%3.2f, C:%3.2f, G:%3.2f, T:%3.2f\n\n",
				(double)pseq->composition['A'-'A']/Length,
				(double)pseq->composition['C'-'A']/Length,
				(double)pseq->composition['G'-'A']/Length,
				(double)pseq->composition['T'-'A']/Length);

		if((pseq->length-pseq->nucleotides)>0)
		{
			fprintf(Fptxt,"Warning! %d characters in sequence are not A, C, G, or T\n\n",
					(pseq->length-pseq->nucleotides));
		}
	}

	Totalcharacters+=Length;
	WDPcount=0;

	/* G. Benson 1/28/2004 */
	/* following four memory allocations increased to avoid memory error when
	   consensus length exceeds MAXDISTANCE after returning from get_consensus(d) */

	Criteria_count=(int *)calloc(2*(MAXDISTANCE+1),sizeof(int));
	if(Criteria_count==NULL)
	{
		PrintError("Unable to allocate Criteria_count");
		exit(-4);
	}
	Consensus_count=(int *)calloc(2*(MAXDISTANCE+1),sizeof(int));
	if(Consensus_count==NULL)
	{
		PrintError("Unable to allocate memory for Consensus_count");
		exit(-5);
	}
	Cell_count=(double *)calloc(2*(MAXDISTANCE+1),sizeof(double));
	if(Cell_count==NULL)
	{
		PrintError("Unable to allocate memory for Cell_count");
		exit(-6);
	}
	Outputsize_count=(int *)calloc(2*(MAXDISTANCE+1),sizeof(int));
	if(Outputsize_count==NULL)
	{
		PrintError("Unable to allocate memory for Outputsize_count");
		exit(-7);
	}


	if(paramset.multisequencefile)
	{
		sprintf(messagebuffer,"Scanning Sequence %d...",
				paramset.sequenceordinal);
		if (paramset.ngs != 1) PrintProgress(messagebuffer);
	}
	else if (paramset.ngs != 1)
	{
		PrintProgress("Scanning...");
	}


	clear_distancelist(Distance);

	newtupbo(); /* this is the main function of the algorithm */


	Cell_total=0;
	for(i=MAXDISTANCE;i>=1;i--)
	{
		Cell_total+=Cell_count[i];
		if(i<=SMALLDISTANCE)
			Wasted_total+=(Criteria_count[i]+Consensus_count[i])*i*i*2;
		else
			Wasted_total+=(Criteria_count[i]+Consensus_count[i])*i
				*(2*d_range(i)+1)*2;
	}

	if (!paramset.HTMLoff) {
		fprintf(Fptxt,"Done.\n");
		fprintf(Fptxt,"</PRE>");
		fprintf(Fptxt,"</BODY>");
		fprintf(Fptxt,"</HTML>");
		fclose(Fptxt);
	}


	/****************************************************************
	 * The following memory deallocations where not originally
	 * implemented. There may still exist some marginal memory leaks
	 * but due messy code structure and extensive use of macros and
	 * global variables this could be impossible to fix.
	 *****************************************************************/

	if (paramset.ngs != 1) PrintProgress("Freeing Memory...");

	free(S[0]);
	free(S);
	free(Statistics_Distance);
	free(Criteria_count);
	free(Consensus_count);
	free(Cell_count);
	free(Outputsize_count);
	free(AlignPair.textprime);
	free(AlignPair.textsecnd);
	free(AlignPair.indexprime);
	free(AlignPair.indexsecnd);
	free(Tag);
	free(Index);
	/* free_distanceseenlist(); */
	/* modified 5/23/05 G. Benson */
	free_distanceseenarray();

	/* free distance list and all its entries */
	//for(i=1;i<=MAXDISTANCE;i++) free(Distance[i].entry);
	free(_DistanceEntries);
	free(Distance);

	for(i=1; i<=NTS; i++)
	{
		free(Tuplehash[i]);
		free(History[i]);
	}

	free(Sortmultiples);

	if (paramset.ngs != 1) PrintProgress("Resolving output...");

	/* this function define on trfclean.h */
	TRFClean( datstring, txtstring, htmlstring,MaxPeriod,
			paramset.datafile,paramset.maskedfile,Sequence,maskstring);

	/* Set the name of the outputfilename global to name given to
	   file in routines defined in trfclean.h */
	MakeFileName(paramset.outputfilename,htmlstring,1);

	/* added Nov 1, 2012 by Y. Gelfand */
	free_bestperiodlist();

	if (paramset.ngs != 1) PrintProgress("Done.");

	return;
}


void PrintError(char* errortext)
{
	/* the code here depends on the environment defined above */
#if defined(WINDOWSGUI)
	MessageBox((HWND)paramset.guihandle,errortext,"TRF",
			MB_ICONERROR|MB_OK);
#elif defined(WINDOWSCONSOLE)
	fprintf(stderr,"Error: %s\n",errortext);
#elif defined(UNIXGUI)
	unix_gui_error("Error: %s\n",errortext);
#elif defined(UNIXCONSOLE)
	fprintf(stderr,"Error: %s\n",errortext);
#endif

	return;
}

void PrintProgress(char* progresstext)
{
	/* platform dependent code */
#if defined(WINDOWSGUI)
	/* send message to status bar */
	SendDlgItemMessage((HWND)paramset.guihandle,IDC_STATUSBAR,
			SB_SETTEXT, (WPARAM) 0,(LPARAM)progresstext);
	SendDlgItemMessage((HWND)paramset.guihandle,IDC_STATUSBAR,
			WM_PAINT, (WPARAM) 0,(LPARAM)0);
#elif defined(WINDOWSCONSOLE)
	fprintf(stderr,"\n%s",progresstext);
#elif defined(UNIXGUI)
	print_progress("\n%s",progresstext);
#elif defined(UNIXCONSOLE)
	fprintf(stderr,"\n%s",progresstext);
#endif

	return;
}

void SetProgressBar(void)
{
	static int ready=0;

#if defined(WINDOWSGUI)
	static HWND barhwnd;
	static HDC hdc;
	static RECT rect,rect2;
	static char buffer[100];
#elif defined(WINDOWSCONSOLE)

#elif defined(UNIXGUI)

#elif defined(UNIXCONSOLE)

#endif

	/* if percent is minus one then un-ready the progress indicator */
	if(paramset.percent<0)
	{
		if(!ready) return;
		ready = 0;
		/* hide the progress indicator */
#if defined(WINDOWSGUI)
		InvalidateRect(barhwnd,NULL,TRUE);
		ReleaseDC(barhwnd,hdc);
#elif defined(WINDOWSCONSOLE)
		putchar('\n');
#elif defined(UNIXGUI)
		set_progress_bar( 0.0 );
#elif defined(UNIXCONSOLE)
		putchar('\n');
#endif

		return;
	}


	/* if not ready make ready */
	if(!ready)
	{
		ready = 1;

		/* position the progress progress indicator */
#if defined(WINDOWSGUI)
		/* get the rectangle of the second part of status bar */
		barhwnd = GetDlgItem((HWND)paramset.guihandle,IDC_STATUSBAR);
		SendMessage(barhwnd, SB_GETRECT,(WPARAM) 1,
				(LPARAM)(LPRECT) &rect);
		/* adjust for borders */
		rect.left+=2;
		rect.right-=2;
		rect.top+=2;
		rect.bottom-=2;
		/* set up progress rectangle */
		rect2 = rect;
		rect2.right = rect2.left;

		/* obtain a dc and draw on second panel */
		hdc = GetDC(barhwnd);
		FillRect(hdc,&rect,(HBRUSH)(COLOR_WINDOW+1));
		FillRect(hdc,&rect2,(HBRUSH)(COLOR_ACTIVECAPTION+1));

#elif defined(WINDOWSCONSOLE)
		putchar('\n');
#elif defined(UNIXGUI)

#elif defined(UNIXCONSOLE)
		putchar('\n');
#endif
	}

	/* set progress indicator to percent value */
	if(paramset.percent>100) paramset.percent=100;

#if defined(WINDOWSGUI)
	FillRect(hdc,&rect,(HBRUSH)(COLOR_WINDOW+1));
	rect2.right = rect2.left+(rect.right-rect.left)*paramset.percent/100;
	FillRect(hdc,&rect2,(HBRUSH)(COLOR_ACTIVECAPTION+1));
#elif defined(WINDOWSCONSOLE)
	if(!(paramset.percent%5)) putchar('.');
#elif defined(UNIXGUI)
	set_progress_bar( paramset.percent / 100.0 );
#elif defined(UNIXCONSOLE)
	if(!(paramset.percent%5)) putchar('.');
#endif

	return;
}


/*******************************************************
 * this is an older version of LoadSequenceFromFile
 * it was replaced because the older version bactracked in the
 * input file after determining sequence length which made the
 * program fail when input stream from stdin had to be read.
 *
 * new version written on 2010-02-18 by Eugene Scherba
 ******************************************************/
int LoadSequenceFromFileBenson(FASTASEQUENCE *pseq,FILE* fp)
{
	int letter,i,pos1,length,next;
	char *ptext;

	// read the FASTA '>' symbol
	letter = getc(fp);
	if(letter!='>') return -1; // invalid format

	// read name and description text
	for(i=0,ptext=pseq->name;i<(MAXSEQNAMELEN-1);i++,ptext++)
	{
		letter = getc(fp);
		if(letter==EOF) break;
		*ptext = letter;
		if(*ptext==10||*ptext==13)
		{
			break;
		}
	}
	*ptext='\0';
	// if line was not read completely flush the rest
	if(i==(MAXSEQNAMELEN-1))
	{
		letter = 0;
		while(letter!=13&&letter!=10&&letter!=EOF) letter = getc(fp);
	}

	// get the length of the sequence
	pos1 = ftell(fp);
	length=0;
	for(;;)
	{
		letter = getc(fp);
		if(letter==EOF||letter=='>') break;
		length++;
	}

	// allocate memory including white space
	pseq->sequence = (char*) malloc(sizeof(char)*(length+1));
	if(pseq->sequence==NULL) return -1;

	// read sequence into buffer
	fseek(fp,pos1,SEEK_SET);
	pseq->length=0;
	ptext = pseq->sequence;
	for(i=0;i<26;i++) pseq->composition[i]=0;
	for(;;)
	{
		// get a character from file
		letter = getc(fp);
		// break if end of file
		if(letter==EOF) { next = 0; break; }
		// break if another sequence found
		if(letter=='>') { next = 1; ungetc('>',fp); break; }
		*ptext = letter;
		// if character is in range of alpha characters
		if(*ptext>='A'&&*ptext<='z') // in alpha range
		{
			if(*ptext<='Z') // in upper case range
			{
				pseq->composition[*ptext-'A']++;
				ptext++;
				pseq->length++;
			}
			else if(*ptext>='a') // in lower case range
			{
				// make upper case
				(*ptext)+=('A'-'a');

				pseq->composition[*ptext-'A']++;
				ptext++;
				pseq->length++;
			}
		}
	}
	// terminate sequence text as a string
	*ptext='\0';

	// compute member
	pseq->nucleotides =
		pseq->composition['A'-'A'] + pseq->composition['C'-'A'] +
		pseq->composition['G'-'A'] + pseq->composition['T'-'A'];

	return next;
}

/*******************************************************
 *   loads a sequence from an open file. If the routine
 *   comes to another sequence header while reading a
 *   sequence the sequence returns 1 to indicate that
 *   more sequences are present. Otherwise the routine
 *   returns 0 to indicate EOF or -1 to indicate error.
 *   The member sequence must be NULL before the routine
 *   iscalled. The calling function must free the allo-
 *   cated memory after use.
 ********************************************************/

int LoadSequenceFromFileEugene(FASTASEQUENCE *pseq, FILE* fp)
{
	int i, j, c;
	int next = -1;	// whether a next sequence was encountered
	char *ptemp;
	char to_upper;

	// read the FASTA '>' symbol
	c = getc(fp);
	if ((char)c != '>' || c == EOF) return -1; // invalid format

	// read name and description text
	for(i = 0; i < MAXSEQNAMELEN - 1; i++) {
		c = getc(fp);
		if (c == 10 || c == 13) {
			break;
		} else if (c == EOF) {
			PrintError("FASTA input terminated too early");
			return -1;
		} else {
			pseq->name[i] = (char)c;
		}
	}
	pseq->name[i] = '\0';

	// if line was not read completely flush the rest
	if (i == MAXSEQNAMELEN - 1) {
		c = 0;
		while (c != 13 && c != 10 && c != EOF) c = getc(fp);
	}

	// read sequence into buffer
	pseq->sequence = NULL;
	for (i = 0; i < 26; i++) pseq->composition[i] = 0;
	to_upper = 'A' - 'a';

	i = 0;
	for (j = 0; c != EOF && (char)c != '>'; j += MEMBLOCK) {
		if ((ptemp = realloc(pseq->sequence, sizeof(char)*(i + MEMBLOCK + 1))) == NULL) {
			PrintError("Insufficient memory");
			return -1;
		}
		pseq->sequence = ptemp;
		for(i = j; i < j + MEMBLOCK;) {
			c = getc(fp);			// get a character from file
			if ((char)c >= 'A' && (char)c <= 'Z') { // in upper-case range of alpha characters
				pseq->sequence[i] = (char)c;
				pseq->composition[c - (int)'A']++;
				i++;
			} else if ((char)c >= 'a' && (char)c <= 'z') {	// in lower-case range of alpha characters
				pseq->sequence[i] = (char)c + to_upper;	// make upper-case
				pseq->composition[c - (int)'a']++;
				i++;
			} else if ((char)c == '>') {	// break if another sequence found
				next = 1;
				ungetc('>', fp);
				break;
			} else if (c == EOF) {		// break if end of file
				next = 0;
				break;
			}
		}
	}
	pseq->length = i;		// set sequence length
	if (i > 0) {
		pseq->sequence[i] = '\0';	// terminate sequence text as a string
	}

	// compute member
	pseq->nucleotides =
		pseq->composition['A'-'A'] + pseq->composition['C'-'A'] +
		pseq->composition['G'-'A'] + pseq->composition['T'-'A'];

	return next;
}

#endif
