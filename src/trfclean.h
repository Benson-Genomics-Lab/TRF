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


/****************************************************************
 *
 *   TRFCLEAN.H : Routines to remove redundancy from output files
 *
 *****************************************************************
 *   the following routine are used to remove redundancy from the
 *   alignment file as well as to produce the HTML otput for the
 *   sumary table file(s) and the optional data and masked files.
 *   The general procedure is the following :
 *   i.      Use data from .dat file to build a linked of records.
 *   ii.     Remove pattern sizes greater than MPS.
 *   iii.    Get List sorted by first index keeping track of
 *           original order.
 *   iv.     Remove redundant elements based on given criteria.
 *   v.      Sort remaining elements back to original order.
 *   vi.     In the Alignment file remove sections that correspond
 *           to records deleted in ii and iv.
 *   vii.    Break alignment file in a many files including a
 *           maximun number of alignments per file.
 *   vii.    Save all remaining elements in HTML format. Creating
 *           as many files as necesary.
 *   viii.   Delete old data file and make new one if option is
 *           set.
 *   ix.     Produce Masked file if option is set.
 *   x.      Free linked list memory. 
 *   Note that the sorting of elements is made to make the
 *   different activities optimal (i.e. removing redundancy and
 *   removing from alignment file.)
 *
 *
 * Last updated Dec 14,2004
 *****************************************************************/

#ifndef TRFCLEAN_H
#define TRFCLEAN_H

#include <stdio.h>


extern void PrintError(char* errortext); /* defined in trfrun.h */


/* Global strings to store non-tabulated information in html file */
char hsequence[256];
char hparameters[256];
char hlength[256];

/* max # of items in tables for extended output format*/
#define EO_MAX_TBL 120


/***********************************
 *   Support Procedures Declarations
 ***********************************/

IL*     GetList(char * datafile);
IL*     RemoveBySize(IL * headptr, int maxsize);
IL*     SortByIndex(IL * headptr);
IL*     RemoveRedundancy(IL * headptr);
IL*     SortByCount(IL * headptr);
void    CleanAlignments(IL * headptr, char * alignmentfile);
void    BreakAlignments(IL * headptr, char * alignmentfile);
void    OutputHTML(IL * headptr, char * tablefile, char * alignmentfile);
void    MakeDataFile(IL * headptr,char * datafile,int data);
void    MakeMaskedFile(IL* headptr,int masked,char*  Sequence,char* maskfile);

void    FreeList(IL * headptr);

int     IntervalOverlap(IL* iptr, IL* jptr);
int     IsRedundant(IL* iptr, IL* jptr);
void    MakeFileName(char* newname, char* oldname, int tag);
void    OutputHeading(FILE* fp, char* tablefile, char* alignmentfile);

/***********************************
 *   Chief Procedure Definition
 ***********************************/

void TRFClean( char * datafile, char* alignmentfile, char* tablefile,
		int maxsize, int data, int masked, char* Sequence,
		char* maskfile)
{
	IL *headptr=NULL,*currptr;
	int i;

	/* Added by Yevgeniy Gelfand on Jan 27, 2010  */
	/* To have smaller sequences not send results */
	/* to disc to improve performance             */
	headptr = GlobalIndexList;

	headptr = RemoveBySize(headptr, maxsize);

	headptr = SortByIndex(headptr);

	if (!paramset.redundoff) {
		headptr = RemoveRedundancy(headptr);
	}

	headptr = SortByCount(headptr);

	if(!paramset.HTMLoff)
	{

		CleanAlignments(headptr, alignmentfile);

		BreakAlignments(headptr, alignmentfile);

		OutputHTML(headptr, tablefile, alignmentfile);
	}

	/* Added by Yevgeniy Gelfand on Jan 27, 2010  */
	/* To have smaller sequences not send results */
	/* to disc to improve performance             */
	//MakeDataFile(headptr,datafile,data);

	//MakeMaskedFile(headptr, masked, Sequence, maskfile);

	/* update the global result */
	for(i=0,currptr=headptr;currptr!=NULL;i++,currptr=currptr->next);
	paramset.outputcount= i;

	/* Added by Yevgeniy Gelfand on Jan 27, 2010  */
	/* To have smaller sequences not send results */
	/* to disc to improve performance             */
	GlobalIndexList = headptr;

	return;
}

/***********************************
 *   Support Procedures Definitions
 ***********************************/


/***********GetList()*******************************************************/

IL*     GetList(char * datafile)
{
	FILE    *fp;
	IL      *headptr, *newptr, *lastptr;
	int     counter, i;
	char patbuffer[MAXDISTANCECONSTANT+1000];

	headptr=newptr=lastptr=NULL;

	/* open file */
	fp = fopen(datafile, "r");
	if ( NULL==fp ) return NULL;

	/* get hsequence line on ninth line of data file*/
	for (counter =0; counter<9; counter++) fgets(hsequence, 255, fp);

	/* get hparameters line on twelvth line of data file*/
	for (counter =0; counter<3; counter++) fgets(hparameters, 255, fp);

	/* get hlength from another global variable (bad practice)*/
	sprintf(hlength, "Length:  %d", Length);

	/* loop to fill out list from buffer */
	counter = 1; /* keeps track of order they are found */
	while(1)
	{
		/* create new index list element */
		newptr = (IL*) malloc(sizeof(IL));
		if(newptr==NULL)
		{
			FreeList(headptr);
			return NULL;
		}
		newptr->count = counter++;

		/* get data from file */
		i = fscanf(fp, "%s %d %d %d %f %d %d %d %d %d %d %d %d %f %s",
				newptr->ref, &newptr->first, &newptr->last,
				&newptr->period, &newptr->copies, &newptr->size,
				&newptr->matches, &newptr->indels, &newptr->score,
				&newptr->acount, &newptr->ccount, &newptr->gcount,
				&newptr->tcount, &newptr->entropy,patbuffer);

		if(i==EOF)
		{
			free(newptr);
			break;
		}

		/* allocate memory to place the pattern and copy data into it*/
		newptr->pattern = (char*) malloc((strlen(patbuffer)+1)*sizeof(char));
		if(newptr->pattern==NULL)
		{
			free(newptr);
			FreeList(headptr);
			return NULL;
		}
		strcpy(newptr->pattern,patbuffer);

		if (headptr == NULL) /* first element */
		{
			headptr = lastptr = newptr;
			lastptr->next = NULL;
		}
		else /* add new element to end of list */
		{
			lastptr->next = newptr;
			lastptr = newptr;
			lastptr->next = NULL;
		}
	}

	fclose(fp);
	return  headptr;
}


/************ RemoveBySize() **********************************************/

IL*     RemoveBySize(IL * headptr, int maxsize)
{
	IL* currptr;
	IL* prevptr;

	/* loop thru list removing all elements with period > maxsize */
	for(currptr=headptr; currptr!=NULL;)
	{
		if(currptr->period>maxsize) /* remove */
		{
			if(currptr==headptr)
			{
				headptr=headptr->next;
				free(currptr->pattern);
				free(currptr);
				currptr=headptr; 
			}
			else
			{
				prevptr->next = currptr->next;
				free(currptr->pattern);
				free(currptr);
				currptr = prevptr->next;
			}

		}
		else
		{
			prevptr= currptr;
			currptr= currptr->next;
		}
	}

	return headptr;

}


IL*     SortByIndex(IL * headptr)
{
	IL* currptr;
	IL* holdptr;
	IL* prevptr;
	int dif; /* flags when changes occur in one pass */

	if(headptr==NULL) return headptr; /* return if no elements */
	if(headptr->next==NULL) return headptr; /* return if one element only */

	dif=1;
	currptr=headptr;

	while(dif)
	{
		dif = 0;
		/* repeat inner loop until end is reached */
		while(currptr->next!=NULL)
		{
			if(currptr->first > currptr->next->first) /* swap */
			{
				if(currptr==headptr)
				{
					holdptr=currptr->next->next;
					headptr=currptr->next;
					headptr->next=currptr;
					currptr->next=holdptr;
					prevptr=headptr;

				}
				else
				{
					prevptr->next=currptr->next;
					holdptr=currptr->next->next;
					prevptr->next->next=currptr;
					currptr->next=holdptr;
					prevptr=prevptr->next;
				}

				dif=1; /* mark as changed */
			}
			else
			{
				prevptr=currptr;
				currptr=currptr->next;
			}

		}
		currptr=headptr; /* restart from begining */
	}

	return headptr;
}

IL*     RemoveRedundancy(IL * headptr)
{
	IL* iptr;       /* first pointer of a pair being examined */
	IL* jptr;       /* second pointer of a pair being examined */
	IL* previptr;   /* points to element before iptr */
	IL* prevjptr;   /* points to element before jptr */
	int overlap;    /* overlap of two intervals */
	int iinterval;  /* first pointer index interval */
	int jinterval;  /* second pointer index interval */

	if(headptr==NULL) return headptr; /* return if no elements */
	if(headptr->next==NULL) return headptr; /* return if one element only */

	iptr = headptr;       /* initialize to start at head of list */

	while(iptr!=NULL) /* loop from start to end of list*/
	{
		//iinterval = iptr->last - iptr->first + 1;

		jptr=iptr->next;
		prevjptr=iptr;
		while(jptr!=NULL) /* loop until not enough overlap */
		{
			iinterval = iptr->last - iptr->first + 1;
			jinterval = jptr->last - jptr->first + 1;
			overlap = IntervalOverlap(iptr, jptr);
			if(overlap==0) break;

			/* if neither overlap satisfies minimum of 90% break loop*/
			/*if( (overlap/(double)iinterval<0.9) && */
			/*  (overlap/(double)jinterval<0.9) ) break; */

			/* overlap in relation to iinterval */
			if( !(overlap/(double)iinterval<0.9) && IsRedundant(iptr,jptr))
			{
				/* remove iptr and break from inner loop*/
				if(iptr==headptr)
				{
					headptr=headptr->next;
					free(iptr->pattern);
					free(iptr);
					iptr=headptr;
					jptr=iptr->next;
					prevjptr=iptr;
					continue;
				}
				else
				{
					previptr->next = iptr->next;
					free(iptr->pattern);
					free(iptr);
					iptr = previptr->next;
					jptr=iptr->next;
					prevjptr=iptr;
					continue;
				}
			}

			/* overlap in relation to jinterval */
			if( !(overlap/(double)jinterval<0.9) && IsRedundant(jptr,iptr))
			{
				/* remove jptr and continue next iteration of inner loop*/
				prevjptr->next = jptr->next;
				free(jptr->pattern);
				free(jptr);
				jptr = prevjptr->next;
				continue;
			}

			/* update and continue to next iteration of inner loop */
			prevjptr = jptr;
			jptr = jptr->next;

		}
		previptr = iptr;
		iptr = iptr->next;
	}
	return headptr;
}

int     IntervalOverlap(IL* iptr, IL* jptr)
{
	int beg,end, overlap;
	beg = ( iptr->first > jptr->first ) ? iptr->first : jptr->first;
	end = ( iptr->last  < jptr->last  ) ? iptr->last  : jptr->last;

	overlap = end - beg + 1;    
	return (( overlap > 0 ) ? overlap:0);
}

/* this fuction retuns 1 if iptr is redundant in reference to jptr */
int     IsRedundant(IL* iptr,IL* jptr)
{

	if( (iptr->period>jptr->period)&&
			(iptr->period%jptr->period==0)&&
			(iptr->score<= 1.1*jptr->score)  ) return 1;
	if( (iptr->period==jptr->period)&&
			(iptr->score<= jptr->score)  ) return 1;

	return 0;
}    


IL*     SortByCount(IL * headptr)
{
	IL* currptr;
	IL* holdptr;
	IL* prevptr;
	int dif; /* flags when changes occur in one pass */

	if(headptr==NULL) return headptr; /* return if no elements */
	if(headptr->next==NULL) return headptr; /* return if one element only */

	dif=1;
	currptr=headptr;

	while(dif)
	{
		dif = 0;
		/* repeat inner loop until end is reached */
		while(currptr->next!=NULL)
		{
			if(currptr->count > currptr->next->count) /* swap */
			{
				if(currptr==headptr)
				{
					holdptr=currptr->next->next;
					headptr=currptr->next;
					headptr->next=currptr;
					currptr->next=holdptr;
					prevptr=headptr;

				}
				else
				{
					prevptr->next=currptr->next;
					holdptr=currptr->next->next;
					prevptr->next->next=currptr;
					currptr->next=holdptr;
					prevptr=prevptr->next;
				}

				dif=1; /* mark as changed */
			}
			else
			{
				prevptr=currptr;
				currptr=currptr->next;
			}

		}
		currptr=headptr; /* restart from begining */
	}

	return headptr;
}

void   CleanAlignments(IL * headptr, char * alignmentfile)
{
	char string1[260];
	char string2[260];
	char string3[260];
	char tempfile[264];
	FILE *al_fp, *tmp_fp;
	IL*  currptr;
	int moving; /* in loop moving=1 to copy strings to tempfile */

	/* execute even if no repeats in list */

	/* make name of temporary file */
	strcpy(tempfile,alignmentfile);
	strcat(tempfile,".tmp");

	/* open files */
	al_fp = fopen(alignmentfile, "r");
	if(al_fp==NULL)
	{
		PrintError("Unable to open alignment file for reading in CleanAlignments routine!");
		exit(-1);
	}
	tmp_fp= fopen(tempfile, "w");
	if(tmp_fp==NULL)
	{
		PrintError("Unable to open temp file for writing in CleanAlignments routine!");
		exit(-1);
	}

	moving = 1; /* starts by moving lines to temporary file */
	currptr=headptr;
	while( fgets(string1, 260, al_fp) != NULL )
	{
		if( string1[0]=='F' /* Gelfand Dec 15, 2013 */&& string1[1]=='o') /* Ocurrs when "Found at" is encountered */
		{
			fgets(string2, 260, al_fp);
			fgets(string3, 260, al_fp);
			if(currptr!=NULL)
			{
				if( strstr(string3, currptr->ref)!=NULL )
				{
					fputs(string1, tmp_fp);
					fputs(string2, tmp_fp);
					fputs(string3, tmp_fp);
					currptr = currptr->next;
					moving = 1;
					continue;
				}
			}
			moving = 0;
		}

		if(string1[0]=='D' /* Gelfand Dec 15, 2013 */&& string1[1]=='o') /* Occurs when "Done." is encountered */
		{
			fgets(string2, 260, al_fp);
			fputs(string1, tmp_fp);
			fputs(string2, tmp_fp);
			fputs("\n", tmp_fp);
			break;
		}

		if(moving) fputs(string1, tmp_fp);
	}

	fclose(al_fp);
	fclose(tmp_fp);

	remove(alignmentfile);
	rename(tempfile,alignmentfile);

	return;

}

void   BreakAlignments(IL * headptr, char * alignmentfile)
{
	FILE * al_fp;
	FILE * out_fp;
	char outfile[260];
	int nfiles;
	IL* currptr;
	int alignments;
	char headlns[30][200]; /* to hold the heading section of the file */
	char buffer[200]; /* holds one line of alignment data */
	int headcnt;
	char nextchar;
	int i,j;


	/* Find out how many alignments there are and how many files will
	   be needed */
	for( alignments=0,currptr=headptr; currptr!=NULL; currptr=currptr->next, alignments++);
	nfiles = alignments / EO_MAX_TBL ;
	if( (alignments % EO_MAX_TBL )> 0 ) nfiles++ ;
	if(nfiles==0) nfiles = 1; /* make sure at least one file is generated */

	/* if only one file will be needed just rename file */
	if(nfiles==1)
	{
		MakeFileName( outfile, alignmentfile,1);
		remove(outfile);
		rename(alignmentfile,outfile);
		return;    
	}

	/* get heading lines */
	al_fp = fopen(alignmentfile,"r");
	if(al_fp==NULL)
	{
		PrintError("Unable to open alignment file for reading in BreakAlignments routine!");
		exit(-1);
	}

	for (i=0,headcnt=0; i<30; i++)
	{
		/* find if next line is the "Found at" line */
		nextchar = getc(al_fp); 
		ungetc(nextchar,al_fp);
		if(nextchar=='F') break;
		fgets( headlns[i], 200, al_fp);
		headcnt++;
	}

	/* loop creating files */
	for(i=1;i<=nfiles;i++)
	{
		/* create name of ith file */
		MakeFileName( outfile, alignmentfile,i);
		/* open the file for writing */
		out_fp = fopen(outfile,"w");
		if(out_fp==NULL)
		{
			PrintError("Unable to open output file for writing in BreakAlignments routine!");
			exit(-1);
		}
		/* output heading */
		for(j=0;j<headcnt;j++) fputs(headlns[j],out_fp);
		/* output n of N identifier */
		//fprintf(out_fp,"File %d of %d\n\n",i+1,nfiles);
		fprintf(out_fp,"File %d of %d\n\n",i,nfiles);

		/*move alignments to the current output file*/
		for(j=0; j<EO_MAX_TBL; j++)
		{
			/* copy the first line of the alignment */
			fgets(buffer, 200, al_fp);
			fputs(buffer, out_fp);
			/* copy successive lines checking for the "Found at" and
			   the "Done." lines */
			while(1)
			{
				nextchar = getc(al_fp);
				ungetc(nextchar,al_fp);
				if(nextchar=='F'||nextchar=='D') break;

				// added check for NULL, Gelfand Dec 15, 2013 because of suspicion this may under 
				// certain conditions enter an infinite loop. 
				if (NULL==fgets(buffer, 200, al_fp)) { nextchar='D'; break; }

				fputs(buffer, out_fp);
			}
			if(nextchar=='F') continue;
			if(nextchar=='D') break;
		}

		/* Output closing Lines */
		fprintf(out_fp,"\nDone.\n</PRE></BODY></HTML>\n");

		/*close file*/
		fclose(out_fp);
	}

	/*close original file*/
	fclose(al_fp);

	/* delete original file */
	remove(alignmentfile);

	return;
}

void    MakeFileName(char* newname, char* oldname, int tag)
{
	char newext[20];
	char oldext[10];
	int numindex;
	int count;

	/*copy oldname to newname buffer*/
	strcpy(newname,oldname);

	/*find position of last number in name*/
	for(count=0, numindex=0; newname[count]!='\0';count++)
	{
		if(isdigit((int) newname[count])) numindex = count;
	}


	/*get old extension */
	strcpy(oldext,&newname[numindex+1]);

	/* create new extension based on tag*/
	sprintf(newext,".%d%s",tag,oldext);

	/* copy newext in place over old newname */
	strcpy(&newname[numindex+1], newext);

	return;
}



void    OutputHTML(IL * headptr, char * tablefile, char *alignmentfile)
{
	FILE* fp;
	IL* currptr;
	int i,j;
	int alignments;
	int nfiles;
	char outfile[260];
	char linkfile[260];
	char namebuffer[260];


	/* find out how many elements there are and how
	   many files will be needed */
	for(alignments=0,currptr=headptr;
			currptr!=NULL;
			currptr=currptr->next,alignments++);
	nfiles = alignments/EO_MAX_TBL;
	if((alignments%EO_MAX_TBL)>0)nfiles++;
	if(nfiles==0) nfiles=1; /* make sure at least one file is generated */

	/* loop creating files */
	currptr=headptr;
	for(i=1;i<=nfiles;i++)
	{
		/* create name of ith file */
		MakeFileName( outfile, tablefile, i);

		/* create name of file where the alignments are found */
		MakeFileName( linkfile, alignmentfile, i);

		/* open the file for writing */
		fp = fopen(outfile,"w");
		if(fp==NULL)
		{
			PrintError("Unable to open output file for writing in OutputHTML routine!");
			exit(-1);
		}

		/* output heading */
		OutputHeading(fp, outfile,alignmentfile);

		/* print links to other tables */
		fprintf(fp,"\n<P><PRE>Tables:   ");
		for(j=1;j<=nfiles;j++)
		{
			/* output a link for all but the current page */ 
			if(j!=i)
			{
				MakeFileName(namebuffer,tablefile,j);
				fprintf(fp,"<A HREF=\"%s\" target=\"_self\">%d</A>   ", namebuffer, j);
			}
			else
			{
				fprintf(fp,"%d   ", j);
			}
			if(j%16==0&&j<nfiles) fprintf(fp,"\n          ");
		}

		/* printf "Table n of N" line */
		fprintf(fp,"\n\nThis is table  %d  of  %d  ( %d repeats found )\n</PRE>",i,nfiles,alignments);

		/*print help lines */
		fprintf(fp,"<PRE>\nClick on indices to view alignment\n");

#if defined(WINDOWSGUI)
		fprintf(fp,"</PRE><P>See <FONT COLOR=\"#0000FF\">Table Explanation</FONT> in Tandem Repeats Finder Help</P><BR>\n");
#elif defined(WINDOWSCONSOLE)
		fprintf(fp,"</PRE><A HREF=\"http://tandem.bu.edu/trf/trf.definitions.html#table\" target = \"explanation\">Table Explanation</A><BR><BR>\n");
#elif defined(UNIXGUI)
		fprintf(fp,"</PRE><P>See <FONT COLOR=\"#0000FF\">Table Explanation</FONT> in Tandem Repeats Finder Help</P><BR>\n");
#elif defined(UNIXCONSOLE)
		fprintf(fp,"</PRE><A HREF=\"http://tandem.bu.edu/trf/trf.definitions.html#table\" target = \"explanation\">Table Explanation</A><BR><BR>\n");
#endif


		/* print beginning of table */
		fprintf(fp,"<TABLE BORDER=1 CELLSPACING=0 CELLPADDING=0>\n");

		/* output rows of data */
		for(j=0;j<EO_MAX_TBL && currptr!=NULL;currptr=currptr->next,j++)
		{

			if(j%22==0) fprintf(fp,"<TR><TD WIDTH=140><CENTER>Indices</CENTER></TD><TD WIDTH=80><CENTER>Period<BR>Size </CENTER></TD><TD WIDTH=70><CENTER>Copy<BR>Number</CENTER></TD><TD WIDTH=70><CENTER>Consensus<BR>Size</CENTER></TD><TD WIDTH=70><CENTER>Percent<BR>Matches</CENTER></TD><TD WIDTH=70><CENTER>Percent<BR>Indels</CENTER></TD><TD WIDTH=60><CENTER>Score</CENTER></TD><TD WIDTH=40><CENTER>A</CENTER></TD><TD WIDTH=40><CENTER>C</CENTER></TD><TD WIDTH=40><CENTER>G</CENTER></TD><TD WIDTH=40><CENTER>T</CENTER></TD><TD WIDTH=70><CENTER>Entropy<BR>(0-2)</CENTER></TD></TR>\n");
			fprintf(fp,"<TR><TD><CENTER><A HREF=\"%s#%s\">%d--%d</A></CENTER></TD><TD><CENTER>%d</CENTER></TD><TD><CENTER>%1.1f</CENTER></TD><TD><CENTER>%d</CENTER></TD><TD><CENTER>%d</CENTER></TD><TD><CENTER>%d</CENTER></TD><TD><CENTER>%d</CENTER></TD><TD><CENTER>%d</CENTER></TD><TD><CENTER>%d</CENTER></TD><TD><CENTER>%d</CENTER></TD><TD><CENTER>%d</CENTER></TD><TD><CENTER>%1.2f</CENTER></TD></TR>\n",
					linkfile,currptr->ref,currptr->first,currptr->last,currptr->period,currptr->copies,currptr->size,currptr->matches,currptr->indels,currptr->score,currptr->acount,currptr->ccount,currptr->gcount,currptr->tcount,currptr->entropy);
		}

		/* if empty list print at least one heading */
		if(headptr==NULL) fprintf(fp,"<TR><TD WIDTH=140><CENTER>Indices</CENTER></TD><TD WIDTH=80><CENTER>Period<BR>Size </CENTER></TD><TD WIDTH=70><CENTER>Copy<BR>Number</CENTER></TD><TD WIDTH=70><CENTER>Consensus<BR>Size</CENTER></TD><TD WIDTH=70><CENTER>Percent<BR>Matches</CENTER></TD><TD WIDTH=70><CENTER>Percent<BR>Indels</CENTER></TD><TD WIDTH=60><CENTER>Score</CENTER></TD><TD WIDTH=40><CENTER>A</CENTER></TD><TD WIDTH=40><CENTER>C</CENTER></TD><TD WIDTH=40><CENTER>G</CENTER></TD><TD WIDTH=40><CENTER>T</CENTER></TD><TD WIDTH=70><CENTER>Entropy<BR>(0-2)</CENTER></TD></TR>\n");

		/* close table */
		fprintf(fp,"\n</TABLE>\n");

		/* if no repeats print message */
		if(headptr==NULL) fprintf(fp, "\nNo Repeats Found!<BR>");/*for empty list*/

		/* print links to other tables (again)*/
		fprintf(fp,"\n<P><PRE>Tables:   ");
		for(j=1;j<=nfiles;j++)
		{
			/* output a link for all but the current page */ 
			if(j!=i)
			{
				MakeFileName(namebuffer,tablefile,j);
				fprintf(fp,"<A HREF=\"%s\" target=\"_self\">%d</A>   ", namebuffer, j);
			}
			else
			{
				fprintf(fp,"%d   ", j);
			}
			if(j%16==0&&j<nfiles) fprintf(fp,"\n          ");
		}
		fprintf(fp,"\n</PRE>");

		if(i==nfiles) fprintf(fp,"<P>The End!\n");

		fprintf(fp,"\n</BODY></HTML>\n");
		fclose(fp);
	}

	return;
}

void    OutputHeading(FILE* fp, char * tablefile, char *alignmentfile)
{
	/* output fixed (old) heading */
	fprintf(fp,"<HTML><HEAD><TITLE>%s</TITLE><BASE TARGET=\"%s\"></HEAD><BODY bgcolor=\"#FBF8BC\"><BR><PRE>Tandem Repeats Finder Program written by:</PRE><PRE><CENTER>Gary Benson<BR>Program in Bioinformatics<BR>Boston University<BR>Version %s<BR></CENTER>",tablefile, alignmentfile, versionstring);
	fprintf(fp,"\nPlease cite:\nG. Benson,\n\"Tandem repeats finder: a program to analyze DNA sequences\"\nNucleic Acid Research(1999)\nVol. 27, No. 2, pp. 573-580.\n");

	fprintf(fp,"\n%s",hsequence);
	fprintf(fp,"%s",hparameters);
	fprintf(fp,"%s</PRE>\n",hlength);

	return;
}




void    MakeDataFile(IL * headptr,char * datafile,int data)
{
	FILE* fp;
	IL* lpointer;
	int charcount;

	/* if data = 1 then produce new datafile overwriting old one */
	/* otherwise delete old file */
	if(data)
	{
		fp = fopen(datafile,"w");
		if(fp==NULL)
		{
			PrintError("Unable to open output file for writing in MakeDataFile routine!");
			exit(-1);
		}

		if (paramset.ngs != 1) {
			fprintf(fp,"Tandem Repeats Finder Program writen by:\n\nGary Benson\nProgram in Bioinformatics\nBoston University\nVersion %s\n\n\n%s\n\n\n%s\n\n",versionstring, hsequence, hparameters);
		}
		for(lpointer=headptr;lpointer!=NULL;lpointer=lpointer->next)
		{
			fprintf(fp,"%d %d %d %.1f %d %d %d %d %d %d %d %d %.2f %s ",
					lpointer->first, lpointer->last, lpointer->period,
					lpointer->copies, lpointer->size, lpointer->matches,
					lpointer->indels, lpointer->score, lpointer->acount,
					lpointer->ccount, lpointer->gcount, lpointer->tcount,
					lpointer->entropy, lpointer->pattern );
			for(charcount=lpointer->first; charcount<=lpointer->last;charcount++)
				fprintf(fp,"%c", Sequence[charcount]);
			fprintf(fp,"\n");
		}
		fclose(fp);
	}
	else remove(datafile);
	return;
}


void    MakeMaskedFile(IL* headptr,int masked,char*  Sequence,char* maskfile)
{
	int count,printcr;
	int masker;
	FILE *fp;
	IL* lpointer;

	if(masked)
	{
		fp = fopen(maskfile,"w");
		if(fp==NULL)
		{
			PrintError("Unable to open output file for writing in MakeMaskedFile routine!");
			exit(-1);
		}

		/* Ouput sequence description from global variable to file*/
		fprintf(fp,">%s",&hsequence[10]);

		for(lpointer=headptr;lpointer!=NULL;lpointer=lpointer->next)
		{
			for(masker=lpointer->first; masker<=lpointer->last; masker++)
				Sequence[masker]='N';
		}
		printcr=0;
		for(count=1;Sequence[count]!='\0';count++)
		{
			fputc(Sequence[count], fp);
			printcr++;
			if(printcr>=60)
			{
				printcr=0;
				fputc('\n', fp);
			}
		}
		fputc('\n', fp);
		fputc('\n', fp);
		fclose(fp);
	}

	return;
}


void   FreeList(IL * headptr)
{
	IL* nextptr;
	IL* holdptr;

	nextptr = headptr;
	while(nextptr!=NULL)
	{
		holdptr = nextptr;
		nextptr = nextptr->next;
		free(holdptr->pattern);
		free(holdptr);
	}

	return;
}

#endif
