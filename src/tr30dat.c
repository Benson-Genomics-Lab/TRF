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


#ifndef TR30DAT_C
#define TR30DAT_C

#include <stdlib.h> /* has calloc definition */
#include <math.h>
#include "tr30dat.h"


/* March 3,1999 removed main to replace with OnStartSearch() . AR */
/* last update May 20 to test multiple tuple sizes */

/* August 12th, 1998  put print alignment headings and print alignment in get statistics */

/* This is a test version which contains the narrow band alignment routines
   narrowbnd.c, prscores.c, pairalgn.c */


#define new1Darrayfunc(type,functionname,length)\
	type *functionname(int length)\
{\
	type *objptr=calloc(length,sizeof(*objptr));\
	if(objptr==NULL)\
	{\
		paramset.endstatus = "functionname: Out of memory!";\
		return NULL;\
	}\
	return(objptr);\
}
new1Darrayfunc(char,newAlignPairtext,length)

new1Darrayfunc(char,newLine,length)

new1Darrayfunc(int,newAlignPairindex,length)

new1Darrayfunc(int,newTags,length)


	/*******************************************************************/

int d_range(int d) 
{
	return((int)floor(2.3*sqrt(Pindel*d)));
}

/* Jan 27, 2006, Gelfand, changed to use Similarity Matrix to avoid N matching itself */
/* This function may be called multiple times (for different match/mismatch scores) */
/*******************************************************************/
/**************************    init_sm()   *************************/
/*******************************************************************/
void init_sm(int match, int mismatch)

{
	static int sm_allocated = 0;
	int i,j,*currint;

	/* SM has 256*w56 entries to map MATCH-MISMATCH matrix */
	if (!sm_allocated) 
	{
		SM=(int *)calloc(256*256,sizeof(int));
		if(SM==NULL){trf_message("\nInit_sm: Out of memory!");exit(-1);}
		sm_allocated = 1;
	}

	/* generate 256x256 into matrix */
	for(i=0,currint=SM;i<=255;i++)
	{
		for(j=0;j<=255;j++,currint++)
		{
			*currint=mismatch;
		}
	}

	SM['A'*256+'A']=match;
	SM['C'*256+'C']=match;
	SM['G'*256+'G']=match;
	SM['T'*256+'T']=match;
}

/*******************************************************************/

/**************************  init_index()  *************************/

/*******************************************************************/

void init_index()

{
	/* index has 256 entries so that finding the entries for A, C, G and T */
	/* require no calculation */

	Index=(int *)calloc(256,sizeof(int));
	if(Index==NULL){trf_message("\nInit_index: Out of memory!");exit(-1);}
	Index['A']=0;
	Index['C']=1;
	Index['G']=2;
	Index['T']=3;
}


/*******************************************************************/

/***************************** narrowbandwrap()  macros **************************/

/*******************************************************************/

/* *pcurr>=maxscore added 2.17.05 gary benson -- to extend alignment as far as possible */
#define test_trace_and_backwards_maxscore \
	if((realr<=start-size)&&(*pcurr==0))\
pleft=(*pcurr=-1000);\
else end_of_trace=FALSE;\
if(*pcurr>=maxscore)\
{\
	maxscore=*pcurr;\
	minrealrow=realr;\
	mincol=c;\
	/***6/9/05 G. Benson***/ mincolbandcenter=Bandcenter[r];\
	/*** 6/14/05 G. Benson ***/ mincolposition=i;\
}


#define test_maxrowscore_with_match \
	if(*pcurr>maxrowscore)\
{\
	maxrowscore=*pcurr;\
	if((*pcurr==*pdiag)&&(match_yes_no==Alpha))\
	matchatmax_col=c;\
	else matchatmax_col=-2;\
}

#define test_maxrowscore_without_match \
	if(*pcurr>maxrowscore)\
matchatmax_col=-2;

/* *pcurr>=maxscore added 2.17.05 gary benson -- to extend alignment as far as possible */     
#define test_trace_and_forward_maxscore \
	if((realr>=start)&&(*pcurr==0))\
pleft=(*pcurr=-1000);\
else end_of_trace=FALSE;\
if(*pcurr>=maxscore)\
{\
	maxscore=*pcurr;\
	maxrealrow=realr;\
	maxrow=r;\
	maxcol=c;\
}

/*******************************************************************/

/***************************** narrowbandwrap()  **************************/

/*******************************************************************/
void narrowbandwrap(int start, int size,int bandradius, int bandradiusforward, 
		int option,int tuplesize)

/* start is end of pattern in text */
/* tuplesize is the size of tuple used for this pattern size */

/* started Feb. 7 1997 */
{
	int g;
	register int *pup, *pdiag, *pcurr,pleft;
	int c,realr,end_of_trace,
	    maxscore,maxrow,maxcol,mincol;
	int maxrealrow, minrealrow;
	char currchar;
	int w,matches_in_diagonal,matchatmax_col,i,k,maxrowscore,
	    lastmatchatmax_col,match_yes_no;
	/*** 6/9/05 G. Benson ***/ 
	int mincolbandcenter,zeroat,mincolposition;
	/* Feb 16, 2016 Yozen */
	unsigned int r;

	/* G. Benson */
	/* 1/26/10 */
	/* change MAXWRAPLENGTH to MAXWRAPLENGTHCONST so MAXWRAPLENGTH can be used as an int */
	/* int Bandcenter[MAXWRAPLENGTH+1]; */
	// if (Bandcenter == NULL) {
	// 	Bandcenter = calloc(maxwraplength+1, sizeof(*Bandcenter));
	// }
	/* Bandcenter init removed by Yozen on Feb 16, 2016. */

	w=bandradius;
	if(MAXBANDWIDTH<2*w+1)
	{
		trf_message("\nIn narrowbandwrap, MAXBANDWIDTH: %d exceeded by 2*w+1: %d\n",
				MAXBANDWIDTH,2*w+1);
		exit(-1);
	}

	/* fill EC */
	if(option==WITHCONSENSUS)
		for(g=0;g<size;g++) EC[g]=Consensus.pattern[g];
	else
		for(g=0;g<size;g++) EC[g]=Sequence[start-size+g+1];

	/* backward wdp */
	maxscore=0;
	realr=start+1;
	r=maxwraplength;
	Bandcenter[r]=0;
	matches_in_diagonal=0;
	matchatmax_col=-2;

	pcurr=&S[r][0];
	pdiag=&Diag[0];
	pup=&Up[0];




	/* 3/14/05 gary benson -- reverse direction */
	/* change zeroth row values to put in -1000 in unreachable cells
	   and gap penalty in cells beyond start location */


	for (i=0;i<=w;i++)
	{
		*pup=(*pdiag=(*pcurr=0+Delta*(w-i)))+ Delta;
		pup++;
		pdiag++;
		pcurr++;
	}

	for (i=w+1;i<=2*w;i++)
	{
		*pup=(*pdiag=(*pcurr=-1000))+ Delta;
		pup++;
		pdiag++;
		pcurr++;
	}





	/* backwards */

	end_of_trace=FALSE;  
	while ((!end_of_trace) && (realr>1) && (r>0))
	{
		r--;
		realr--;
		Rows++;
		maxrowscore=-1;
		lastmatchatmax_col=matchatmax_col;
		end_of_trace=TRUE;
		currchar=Sequence[realr];
		pcurr=&S[r][2*w];
		pleft=-1000;

		if(matches_in_diagonal>=tuplesize)
		{
			/* recenter band */
			Bandcenter[r]=(matchatmax_col-1+size)%size;
		}
		else
		{

			/* don't recenter */
			Bandcenter[r]=(Bandcenter[r+1]-1+size)%size;
		}

		/* change of bandcenter determines which inputs go into which
		   cells */

		k=(Bandcenter[r]-Bandcenter[r+1]+size)%size;
		if(size-k<=k) k=-(size-k);
		c=(Bandcenter[r]+w)%size;
		if(k<=-1)           /* band shifts left */
		{
			k=-k;
			pdiag=&Diag[2*w-k+1];
			pup=&Up[2*w-k];
			for(i=2*w;i>=k;i--)
			{
				*pdiag+=(match_yes_no=match(currchar,EC[c]));
				pleft=(*pcurr=max4(0,*pdiag,*pup,pleft))+Delta;
				test_trace_and_backwards_maxscore;
				test_maxrowscore_with_match;
				pcurr--;
				pdiag--;
				pup--;
				c=(c-1+size)%size;
			}
			i=k-1;
			*pdiag+=(match_yes_no=match(currchar,EC[c]));
			pleft=(*pcurr=max3(0,*pdiag,pleft))+Delta;
			test_trace_and_backwards_maxscore;
			test_maxrowscore_with_match;
			pcurr--;
			c=(c-1+size)%size;
			for(i=k-2;i>=0;i--)
			{
				pleft=(*pcurr=max2(0,pleft))+Delta;
				test_trace_and_backwards_maxscore;
				test_maxrowscore_without_match;
				pcurr--;
				c=(c-1+size)%size;
			}
		}
		else            /* band shifts right */
		{
			pdiag=&Diag[2*w];
			pup=&Up[2*w];
			for(i=2*w;i>=2*w-k+1;i--)
			{
				pleft=(*pcurr=max2(0,pleft))+Delta;
				test_trace_and_backwards_maxscore;
				test_maxrowscore_without_match;
				pcurr--;
				c=(c-1+size)%size;
			}
			i=2*w-k;
			pleft=(*pcurr=max3(0,*pup,pleft))+Delta;
			test_trace_and_backwards_maxscore;
			test_maxrowscore_without_match;
			pcurr--;
			pup--;
			c=(c-1+size)%size;
			for(i=2*w-k-1;i>=0;i--)
			{
				*pdiag+=(match_yes_no=match(currchar,EC[c]));
				pleft=(*pcurr=max4(0,*pdiag,*pup,pleft))+Delta;
				test_trace_and_backwards_maxscore;
				test_maxrowscore_with_match;
				pcurr--;
				pdiag--;
				pup--;
				c=(c-1+size)%size;
			}
		}
		pcurr=&S[r][0];
		pdiag=&Diag[0];
		pup=&Up[0];
		for (i=0;i<=2*w;i++)
		{
			*pup=(*pdiag=*pcurr)+Delta;
			pcurr++;
			pdiag++;
			pup++;
		}
		if((matchatmax_col-lastmatchatmax_col+size)%size==size-1)
			matches_in_diagonal++;
		else matches_in_diagonal=0;
	}


	/**********************************************************************/
	/**********************************************************************/
	/**********************************************************************/
	/**********************************************************************/

	/* forward */

	if (ldong==1) /* null statement */;
	else
	{

		r=0;
		realr=minrealrow-1;
		/* mincol matches minrealrow in previous alignment above */

		/*** 6/14/05 G. Benson ***/
		/* modification version g */
		Bandcenter[0]=(mincolbandcenter-1+size)%size;
		matches_in_diagonal=0;
		matchatmax_col=-2;
		maxscore=0;
		pup=&Up[0];
		pdiag=&Diag[0];
		pcurr=&S[0][0];




		/* 3/14/05 gary benson -- forward direction */
		/* change zeroth row values to put in -1000 in unreachable cells
		   and gap penalty in cells beyond start location */

		/* initialize top row */

		/*** 6/14/05 G. Benson ***/
		/* set zero at mincol, not at bandcenter, which is now mincolbandcenter, 
		   so that forward alignment starts out the same as backwards alignment ended. */ 
		zeroat=mincolposition-w;



		/* July 9, 20009, G. Benson */
		/* widening the narrowband in the forward direction to catch alignments that don't return to correct start column */
		w = bandradiusforward;

		if(MAXBANDWIDTH<2*w+1)
		{
			trf_message("\nIn narrowbandwrap, MAXBANDWIDTH: %d exceeded by bandradiusforward 2*w+1: %d\n",
					MAXBANDWIDTH,2*w+1);
			exit(-1);
		}

		//	if((zeroat>w)||(zeroat<-w)) fprintf(stderr,"\nfound zeroat out of bounds");
		/*** 6/14/05 G. Benson restore what was in all earlier versions ***/
		for(i=0;i<w+zeroat;i++)
		{
			*pup=(*pdiag=(*pcurr=-1000))+Delta;
			pup++;
			pdiag++;
			pcurr++;
		}

		/*** 6/9/05 G. Benson ***/
		for(i=w+zeroat;i<=2*w;i++)
		{
			/*** 6/9/05 G. Benson ***/
			*pup=(*pdiag=(*pcurr=0+Delta*(i-(w+zeroat))))+Delta;
			pup++;
			pdiag++;
			pcurr++;
		}




		/* compute until end of trace */      
		end_of_trace=FALSE;  
		while ((!end_of_trace) && (realr<Length) && (r<maxwraplength))
		{
			r++;
			realr++;
			Rows++;
			end_of_trace=TRUE;
			maxrowscore=-1;
			lastmatchatmax_col=matchatmax_col;
			currchar=Sequence[realr];
			pcurr=&S[r][0];
			pleft=-1000;      /* don't use pleft for first entry */   

			if(matches_in_diagonal>=tuplesize)
			{
				/* recenter band */
				Bandcenter[r]=(matchatmax_col+1)%size;
			}
			else
			{

				/* don't recenter */
				Bandcenter[r]=(Bandcenter[r-1]+1)%size;
			}

			/* change of bandcenter determines which inputs go into which
			   cells */

			k=(Bandcenter[r]-Bandcenter[r-1]+size)%size;
			if(size-k<=k) k=-(size-k);
			if(k>=1)          /* band shifts right */
			{
				pdiag=&Diag[k-1];
				pup=&Up[k];
				c=(Bandcenter[r]-w+size)%size;
				for(i=0;i<=2*w-k;i++)
				{
					*pdiag+=(match_yes_no=match(currchar,EC[c]));
					pleft=(*pcurr=max4(0,*pdiag,*pup,pleft))+Delta;
					test_trace_and_forward_maxscore;
					test_maxrowscore_with_match;
					pcurr++;
					pdiag++;
					pup++;
					c=(c+1)%size;
				}
				i=2*w-k+1;
				*pdiag+=(match_yes_no=match(currchar,EC[c]));
				pleft=(*pcurr=max3(0,*pdiag,pleft))+Delta;
				test_trace_and_forward_maxscore;
				test_maxrowscore_with_match;
				pcurr++;
				c=(c+1)%size; 
				for(i=2*w-k+2;i<=2*w;i++)
				{
					pleft=(*pcurr=max2(0,pleft))+Delta;
					test_trace_and_forward_maxscore;
					test_maxrowscore_without_match;
					pcurr++;
					c=(c+1)%size;
				}
			}
			else          /* band shifts left */
			{
				k=-k;
				c=(Bandcenter[r]-w+size)%size;
				pup=&Up[0];
				pdiag=&Diag[0];
				for(i=0;i<=k-1;i++)
				{
					pleft=(*pcurr=max2(0,pleft))+Delta;
					test_trace_and_forward_maxscore;
					test_maxrowscore_without_match;
					pcurr++;
					c=(c+1)%size;
				}
				i=k;
				pleft=(*pcurr=max3(0,*pup,pleft))+Delta;
				test_trace_and_forward_maxscore;
				test_maxrowscore_without_match;
				pcurr++;
				pup++;
				c=(c+1)%size;
				for(i=k+1;i<=2*w;i++)
				{
					*pdiag+=(match_yes_no=match(currchar,EC[c]));
					pleft=(*pcurr=max4(0,*pdiag,*pup,pleft))+Delta;
					test_trace_and_forward_maxscore;
					test_maxrowscore_with_match;
					pcurr++;
					pdiag++;
					pup++;
					c=(c+1)%size;
				}
			}
			pcurr=&S[r][0];
			pdiag=&Diag[0];
			pup=&Up[0];
			for (i=0;i<=2*w;i++)
			{
				*pup=(*pdiag=*pcurr)+Delta;
				pcurr++;
				pdiag++;
				pup++;
			}

			if((matchatmax_col-lastmatchatmax_col+size)%size==1)
				matches_in_diagonal++;
			else matches_in_diagonal=0;
		}
		/* store last position scanned with this pattern */
		Wrapend=realr;

		if (Criteria_print) trf_message("   DWPlength:%d",Wrapend-(minrealrow-1)+1);
		/* test for report */
		if (maxscore>=Reportmin)
		{
			Maxrealrow=maxrealrow;
			Maxrow=maxrow;
			Maxcol=maxcol;
			Maxscore=maxscore;

		}

	}

}



/*******************************************************************/

/*******************************************************************/

/***************************** globalnarrowbandwrap()  macros **************************/

/*******************************************************************/


#define global_test_maxrowscore_with_match \
	if(*pcurr>maxrowscore)\
{\
	maxrowscore=*pcurr;\
	if((*pcurr==*pdiag)&&(match_yes_no==Alpha))\
	matchatmax_col=c;\
	else matchatmax_col=-2;\
}

#define global_test_maxrowscore_without_match \
	if(*pcurr>maxrowscore)\
matchatmax_col=-2;



/*******************************************************************/

/***************************** newwrap()  **************************/

/*******************************************************************/
void newwrap(int start, int size, int consensuspresent)

{
	int g;
	register int *pup, *pdiag, *pcurr,pleft;
	int adjlength,adjmone,c,realr,end_of_trace,
	    maxscore,minrow,maxrow,maxcol,modstart,maxrealrow;
	char currchar;
	/* Feb 16, 2016 Yozen */
	unsigned int r;

	/* fill EC */
	if(consensuspresent)
		for(g=0;g<size;g++) EC[g]=Consensus.pattern[g];
	else
		for(g=0;g<size;g++) EC[g]=Sequence[start-size+g+1];


	/* backward wdp */
	maxscore=0;
	realr=start+1;
	r=maxwraplength;

	adjlength=size-1;
	adjmone=adjlength-1;
	pup=Up;
	pdiag=Diag;
	pcurr=&S[r][0];

	for (c=0;c<size;c++)
	{
		*pup=(*pdiag=(*pcurr=maxscore))+ Delta;
		pup++;
		pdiag++;
		pcurr++;
	}
	/* backwards */
	end_of_trace=FALSE;  
	while ((!end_of_trace) && (realr>1) && (r>0))
	{
		r--;
		realr--;
		Rows++;
		currchar=Sequence[realr];

		pcurr=&S[r][adjlength];
		pleft=Delta;        /* first pass. set S[r][0]=0 */
		pdiag=&Diag[adjlength];
		pup=&Up[adjlength];
		for (c=adjlength;c>=0;c--)
		{
			*pdiag+=match(currchar,EC[c]);
			pleft=(*pcurr=max4(0,*pdiag,*pup,pleft))+Delta;
			pcurr--;
			pdiag--;
			pup--;
		}

		end_of_trace=TRUE;      /* setup for encountering a break in the trace */
		/*second pass */
		pcurr=&S[r][adjlength];
		/* pleft set from first pass */
		pdiag=&Diag[adjlength];
		pup=&Up[adjlength];
		for (c=adjlength;c>=0;c--)
		{
			*pup=(pleft=(*pcurr=max4(0,*pdiag,*pup,pleft))+Delta);
			if((realr<=start-max(size,Min_Distance_Window))&&(*pcurr==0)) 
				/* if (*pcurr==0) doesn't work with consensus */
				*pup=pleft=*pcurr=-1000;
			else
				end_of_trace=FALSE;


			/* 3/14/05 gary benson -- made >= to extend alignments as far as possible */ 
			if (*pcurr>=maxscore)  /* test for maximum */
			{
				maxscore=*pcurr;
				minrow=realr;
			}
			pcurr--;
			pdiag--;
			pup--;

		}  
		pcurr=&S[r][adjlength];
		pdiag=&Diag[adjmone];
		for (c=adjmone;c>=0;c--)
		{
			*pdiag=*pcurr;
			pcurr--;
			pdiag--;
		}
		Diag[adjlength]=*pcurr;


	}
	/***********************************/
	r=0;
	realr=start-1;

	pup=Up;
	pdiag=&Diag[0];
	pcurr=&S[r][0];

	/* initialize_scoring_array top row (*pcurr)*/
	/* initialize diagonal branch (*pdiag) and up branch (*pup) */
	for (c=0;c<size;c++)
	{
		*pup=(*pdiag=(*pcurr=maxscore))+ Delta;
		pup++;
		pdiag++;
		pcurr++;
	}

	end_of_trace=FALSE;  
	while ((!end_of_trace) && (realr<Length) && (r<maxwraplength))
	{
		r++;
		realr++;
		Rows++;
		currchar=Sequence[realr];

		pcurr=&S[r][0];
		pleft=Delta;        /* first pass. S[r][adjlength]=0  */
		pdiag=&Diag[0];
		pup=&Up[0];
		for (c=0;c<size;c++)
		{
			*pdiag+=match(currchar,EC[c]);
			pleft=(*pcurr=max4(0,*pdiag,*pup,pleft))+Delta;
			pcurr++;
			pdiag++;
			pup++;
		}
		end_of_trace=TRUE;      /* setup for encountering a break in the trace */
		/*second pass */
		pcurr=&S[r][0];
		/* pleft set from first pass */
		pdiag=&Diag[0];
		pup=&Up[0];
		for (c=0;c<size;c++)
		{
			*pup=(pleft=(*pcurr=max4(0,*pdiag,*pup,pleft))+Delta);
			if((realr>=start)&&(*pcurr==0))
				/* if (*pcurr==0) doesn't work with consensus */
				*pup=pleft=*pcurr=-1000;
			else
				end_of_trace=FALSE;


			/* 3/14/05 gary benson -- made >= to extend alignments as far as possible */ 
			if (*pcurr>=maxscore)  /* test for maximum */
			{
				maxscore=*pcurr;
				maxrow=realr;       /* ??? do we need this? */
			}
			pcurr++;
			pdiag++;
			pup++;

		}  
		pcurr=&S[r][0];
		pdiag=&Diag[1];
		for (c=0;c<size;c++)
		{
			*pdiag=*pcurr;
			pcurr++;
			pdiag++;
		}
		Diag[0]=Diag[size];

	}    

	/* store last position scanned with this pattern */

	Wrapend=realr;

	/* redo for an accurate alignment if score is big enough to report */
	Reportmin=0;
	if (maxscore>=Reportmin)
	{

		r=0;
		modstart=start+1;
		realr=minrow-size-1;    /* go back at least one additional pattern */
		if (realr<0) realr=0;   /* length in case of misalignment in */
		/* backwards scan due to starting with */
		/* maxscore in every column */
		maxscore=0;
		/****/
		pup=Up;
		pdiag=&Diag[0];
		pcurr=&S[r][0];

		/* initialize_scoring_array top row (*pcurr)*/
		/* initialize diagonal branch (*pdiag) and up branch (*pup) */
		for (c=0;c<size;c++)
		{
			*pup=(*pdiag=(*pcurr=maxscore))+ Delta;
			pup++;
			pdiag++;
			pcurr++;
		}

		end_of_trace=FALSE;  
		while ((!end_of_trace) && (realr<Length) && (r<maxwraplength))
		{
			r++;
			realr++;
			Rows++;
			currchar=Sequence[realr];

			pcurr=&S[r][0];
			pleft=Delta;      /* first pass. S[r][adjlength]=0  */
			pdiag=&Diag[0];
			pup=&Up[0];
			for (c=0;c<size;c++)
			{
				*pdiag+=match(currchar,EC[c]);
				pleft=(*pcurr=max4(0,*pdiag,*pup,pleft))+Delta;
				pcurr++;
				pdiag++;
				pup++;
			}
			end_of_trace=TRUE;    /* setup for encountering a break in the trace */
			/*second pass */
			pcurr=&S[r][0];
			/* pleft set from first pass */
			pdiag=&Diag[0];
			pup=&Up[0];
			for (c=0;c<size;c++)
			{
				*pup=(pleft=(*pcurr=max4(0,*pdiag,*pup,pleft))+Delta);
				if ((realr>=modstart)
						&&(*pcurr==0))
				{*pup=pleft=*pcurr=-1000;}
				else
					end_of_trace=FALSE;

				if (*pcurr>maxscore)    /* test for maximum */
				{
					maxscore=*pcurr;
					maxrealrow=realr; 
					maxrow=r;
					maxcol=c;
				}
				pcurr++;
				pdiag++;
				pup++;

			}  
			pcurr=&S[r][0];
			pdiag=&Diag[1];
			for (c=0;c<size;c++)
			{
				*pdiag=*pcurr;
				pcurr++;
				pdiag++;
			}
			Diag[0]=Diag[size];

		}          
	}

	if(Criteria_print)trf_message("   DWPlength:%d",realr-(minrow-size+1)+1);

	/* test for report */
	if (maxscore>=Reportmin)
	{
		Maxrealrow=maxrealrow;
		Maxrow=maxrow;
		Maxcol=maxcol;
		Maxscore=maxscore;

	}

}


/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/**********************************************************/
/**********************************************************/

void init_bestperiodlist(void)
{
	Bestperiodlist->next=NULL;
}

/**********************************************************/
/**********************************************************/

void free_bestperiodlist(void)
{
	struct bestperiodlistelement *entry, *entrylast;
	entry=Bestperiodlist->next;
	Bestperiodlist->next=NULL;
	while (entry!=NULL)
	{
		entrylast=entry;
		entry=entry->next;
		free(entrylast);
	}
}

/**********************************************************/
/**********************************************************/

/*** modified 6/2/05 G. Benson ***/
void add_to_bestperiodlist(int d)

{
	struct bestperiodlistelement *ptr;

	/*** added 6/2/05 G. Benson***/
	/* when distance is 1, no Sortmultiples are defined due to change in multiples_criteria_4*/
	if(d!=1)
	{

		ptr=(struct bestperiodlistelement *)
			calloc(1,sizeof(struct bestperiodlistelement));
		if(ptr==NULL){trf_message("\nAdd_to_bestperiodlist: Out of memory!");exit(-1);}

		ptr->indexlow=AlignPair.indexprime[AlignPair.length]; 
		ptr->indexhigh=AlignPair.indexprime[1];
		/*** G. Benson 6/2/05 fixed bug using Sortmultiples 1-5 instead of 0-4  ***/
		ptr->best1=Sortmultiples[0];
		ptr->best2=Sortmultiples[1];
		ptr->best3=Sortmultiples[2];
		ptr->best4=Sortmultiples[3];
		ptr->best5=Sortmultiples[4];
		ptr->next=Bestperiodlist->next;
		Bestperiodlist->next=ptr;
	}
}


/*** modified 6/2/05 G. Benson ***/
void adjust_bestperiod_entry(int d)
	/* shortens length of best period entry if the consensus alignment 
	   turns out to be shorter than the first alignment; similar 
	   to distanceseen */
{
	struct bestperiodlistelement *ptr;

	/*** added 6/2/05 G. Benson***/
	/* when distance is 1, no Sortmultiples are defined due to change in multiples_criteria_4*/
	if(d!=1)
	{

		ptr=Bestperiodlist->next;
		if(ptr->indexhigh>AlignPair.indexprime[1])
			ptr->indexhigh=AlignPair.indexprime[1];

	}
}
/**********************************************************/
/**********************************************************/

/* difference plus/minus 5 percent of smaller */
/* +/- 1 percent*/
#define XYZ(a,b) ((a>b)?(a-b)<=(0.01*b):(b-a)<=(0.01*a))

int search_for_range_in_bestperiodlist(int start, int distance)

	/* returns true if
	   1) an entry brackets the range start-distance to start and
	   the distance matches one of the first three best periods
	   2) no entry brackets the range
	   returns false otherwise */

{
	struct bestperiodlistelement *entry, *entrylast, *temp;
	int range_covered;

	entry=Bestperiodlist->next;
	entrylast=Bestperiodlist;
	range_covered=FALSE;
	while (entry!=NULL)
	{
		if(entry->indexhigh<start-2*MAXDISTANCE)
		{
			/* remove current entry, too far back */

			entrylast->next=entry->next;
			temp=entry;
			entry=entrylast;
			free(temp);
		}
		else 
		{
			/* this specifies how much must be bracketed */
			if((entry->indexlow<=start-2*distance+1+Distance[distance].waiting_time_criteria)
					&&(entry->indexhigh>=start)/*&&(entry->indexhigh-entry->indexlow<=3*distance)*/)
			{
				range_covered=TRUE;

				if((entry->best1==distance)
						||(entry->best2==distance)
						||(entry->best3==distance)
						||(entry->best4==distance)
						||(entry->best5==distance)) return(TRUE);


				/* change 1 */
				/*

				   if(XYZ(entry->best1,distance)
				   ||XYZ(entry->best2,distance)
				   ||XYZ(entry->best3,distance)
				   ||XYZ(entry->best4,distance)
				   ||XYZ(entry->best5,distance)) return(TRUE);
				 */


			}
		}
		/* go to next entry */
		entrylast=entry;  
		entry=entry->next;
	}
	if(!range_covered) return(TRUE);
	else return(FALSE);


}


/*******************************************************************/
/*******************************************************************/
void init_distanceseenarray(void)
	/* created 5/23/05 G. Benson */
{
	Distanceseenarray=(struct distanceseenarrayelement *)
		calloc(MAXDISTANCECONSTANT+1,sizeof(struct distanceseenarrayelement));
	if(Distanceseenarray==NULL){trf_message("\nInit Distanceseenarray: Out of memory!");
		exit(-1);}
}
/*******************************************************************/
/*******************************************************************/
void free_distanceseenarray(void)
	/* created 5/23/05 G. Benson */
{
	free(Distanceseenarray);
}
/*******************************************************************/
/*******************************************************************/
void add_to_distanceseenarray(int location,int distance,int end,int score)
	/* created 5/23/05 G. Benson */
	/* adds the extent of an alignment (end) at the given distance 
	   location and score are not currently used */
{
	struct distanceseenarrayelement *ptr;

	ptr = &Distanceseenarray[distance];

	ptr->index=location;
	ptr->end=end; 
	ptr->score=score;

}
/*******************************************************************/
/*******************************************************************/
int search_for_distance_match_in_distanceseenarray(int distance,int start)
	/* created 5/23/05 G. Benson */
	/* searches for an alignment with patternsize of distance in the region 
	   including start.  True means found and alignment should be blocked.*/
{
	struct distanceseenarrayelement ptr;

	ptr = Distanceseenarray[distance];

	if (ptr.end>=start) return(TRUE);
	else return(FALSE);
}
/*******************************************************************/
/*******************************************************************/


/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
void init_distanceseenlist()
{
	Distanceseenlist->next=NULL;
}

/*******************************************************************/
/*******************************************************************/

void free_distanceseenlist()
{
	struct distancelistelement *entry, *entrylast;
	entry=Distanceseenlist->next;
	Distanceseenlist->next=NULL;
	while (entry!=NULL)
	{
		entrylast=entry;
		entry=entry->next;
		free(entrylast);
	}
}

/*******************************************************************/
/*******************************************************************/

void add_to_distanceseenlist(int location,int distance,int end,int  score, int acceptstatus)



{
	int changed_from_distance; /* 3/14/05 */
	struct distancelistelement *ptr, *entry;
	ptr=(struct distancelistelement *)
		calloc(1,sizeof(struct distancelistelement));
	if(ptr==NULL){trf_message("\nAdd_to_distanceseenlist: Out of memory!");exit(-1);}

	/* 3/15/02 Gary Benson
	   fix to remove from Distanceseenlist the first report of a repeat when
	   it blocks further along in the sequence than reported by the consensus 
	   sequence */
	entry=Distanceseenlist->next;
	/*
	   if((acceptstatus==WITHCONSENSUS)&&(distance==entry->distance)
	   &&(location==entry->index)&&(end<=entry->end))
	 */
	if((acceptstatus==WITHCONSENSUS) /* removed matching d because consensussize may be different */
			&&(location==entry->index)&&(end<=entry->end)&&
			((distance<=250)||(distance==entry->distance))) /* put back d match unless d under 250 */
	{
		/* don't remove Distanceseenlist element,
		   but make end=0 so it will be removed by 
		   search_for_distance_in_distanceseenlist
		   procedure */
		entry->end=0;

	}

	/* 3/14/05 */
	if(acceptstatus==WITHCONSENSUS) changed_from_distance=entry->distance;

	ptr->index=location;
	ptr->distance=distance;
	/* 3/14/05 gary benson -- new field to allow blocking of a distance that always shifts
	   to a nearby distance when consensus is computed */
	if(acceptstatus==WITHCONSENSUS) ptr->changed_from_distance=changed_from_distance;
	else ptr->changed_from_distance=distance;
	ptr->end=end;
	ptr->score=score;
	ptr->accepted=acceptstatus;
	ptr->next=Distanceseenlist->next;
	Distanceseenlist->next=ptr;
}

/*******************************************************************/
/*******************************************************************/

int search_for_distance_match_in_distanceseenlist(int distance,int start)

	/* tests for exact same distance or a difference in distance 
	   of less equal 2; finding a match means ruling 
	   out redoing the alignment with that distance */

	/* later, should include a test based on multiples where the actual
	   score and the theoretical best score (all matches) are compared; if
	   the actual score is close to the theoretical score, then don't redo
	   the alignment */     

{
	struct distancelistelement *entry, *entrylast, *temp;
	int absdiff;
	entry=Distanceseenlist->next;
	entrylast=Distanceseenlist;
	while (entry!=NULL)
	{

		/*	if (entry->end<min(start-distance,start-Min_Distance_Window))*/ /***???***/
		/* This change added 11/1/01 to allow repeats that are smaller than Min_Distance_Window to
		   be detected *following* another repeat.  The problem was that the first repeat could block
		   any repeat that followed it, but ended within Min_Distance_Window of the end of the first
		   repeat.  By using Minscore/Alpha, the first repeat can't block anything past its own end. */

		/*    if (entry->end<min(start-distance,start-Minscore/Alpha)+1)  */

		if (entry->end<min(start-distance,max(start-Minscore/Alpha,start-Min_Distance_Window))+1) 

			/* +1 added to correct for the < instead of <= */

			/* changed start to start-distance */
		{
			entrylast->next=entry->next;
			temp=entry;
			entry=entrylast;
			free(temp);
		}
		else 
		{
			if(distance>=entry->distance) absdiff=distance-entry->distance;
			else absdiff=entry->distance-distance;
			if(absdiff==0)
			{
				return(TRUE);
			}
			/* new idea 2/11/05 use an extra field called changed_from_distance to store the old distance
			   when you have a consensus.  In search_for_distance_on_distanceseen_list, if distance doesn't
			   match, but is close, say within 5, then also look at changed_from_distance for a match.
			 */  
			else if((absdiff<=5) && (distance==entry->changed_from_distance))
			{
				return(TRUE);
			}



		}
		/* go to next entry */
		entrylast=entry;  
		entry=entry->next;
	}
	return(FALSE);

}



/*******************************************************************/

/*********************  get_narrowband_pair_alignment_with_copynumber()  **********************/

/*******************************************************************/


void get_narrowband_pair_alignment_with_copynumber(int size,
		int bandradius,int option)


/* for a repeat of EC, do a traceback alignment, */
/* ending at row Maxrow and column Maxcol */




#define test_match_mismatch \
	if (S[r][i]==S[r-1][upi-1]+match(x[realr], y[c]))\
{\
	length++;\
	if(c==fullcopy)Copynumber++;\
	if(option==LOCAL){fill_align_pair(x[realr],y[c],length,realr,c);c=(c-1+size)%size;}\
	else {fill_align_pair(x[realr],y[c],length,realr,c+Maxrealcol-Maxcol);c=c-1;}\
	realr--;\
	r--;\
	i=upi-1;\
}\
else

#define test_up \
	if (S[r][i]==S[r-1][upi]+Delta)\
{\
	length++;\
	if(option==LOCAL) {fill_align_pair(x[realr],'-',length,realr,(c+1)%size);}\
	else {fill_align_pair(x[realr],'-',length,realr,c+1+Maxrealcol-Maxcol);}\
	realr--;\
	r--;\
	i=upi;\
}\
else

#define test_left \
	if(i==0)\
{\
	trf_message("\nget_pair_alignment_with_copynumber: error in trace back");\
	trf_message("\nattempted to compute left branch when i==0");\
	trf_message("\nS[%d][%d]=%d",r,i,S[r][i]);\
	break;\
}\
else if (S[r][i]==S[r][i-1]+Delta)\
{\
	length++;\
	if(c==fullcopy)Copynumber++;\
	if(option==LOCAL){fill_align_pair('-',y[c],length,realr+1,c);c=(c-1+size)%size;}\
	else {fill_align_pair('-',y[c],length,realr+1,c+Maxrealcol-Maxcol);c=c-1;}\
	i=i-1;\
}\
else

#define report_error_match_up_left \
{\
	trf_message("\nget_pair_alignment_with_copynumber: error in trace back");\
	trf_message("\nS: row=%d  column=%d  upi=%d  Sequence: realrow=%d  EC: realcol=%d",\
			r,i,upi,realr,c);\
	trf_message("\nS=%d  Sleft=%d  Sup=%d  Sdiag=%d  match=%d",\
			S[r][i],S[r][i-1],S[r-1][upi],\
			S[r-1][upi-1],match(x[realr], y[c]));\
	break;\
}
#define report_error_match_left \
{\
	trf_message("\nget_pair_alignment_with_copynumber: error in trace back");\
	trf_message("\nS: row=%d  column=%d  Sequence: realrow=%d  EC: realcol=%d",\
			r,i,realr,c);\
	trf_message("\nS=%d  Sleft=%d  Sdiag=%d  match=%d",\
			S[r][i],S[r][i-1],S[r-1][upi-1],match(x[realr], y[c]));\
	break;\
}
#define report_error_up_left \
{\
	trf_message("\nget_pair_alignment_with_copynumber: error in trace back");\
	trf_message("\nS: row=%d  column=%d  Sequence: realrow=%d  EC: realcol=%d",\
			r,i,realr,c);\
	trf_message("\nS=%d  Sleft=%d  Sup=%d",\
			S[r][i],S[r][i-1],S[r-1][upi]);\
	break;\
}
#define report_error_left \
{\
	trf_message("\nget_pair_alignment_with_copynumber: error in trace back");\
	trf_message("\nS: row=%d  column=%d  Sequence: realrow=%d  EC: realcol=%d",\
			r,i,realr,c);\
	trf_message("\nS=%d  Sleft=%d",\
			S[r][i],S[r][i-1]);\
	break;\
}


#define report_error_up \
{\
	trf_message("\nget_pair_alignment_with_copynumber: error in trace back");\
	trf_message("\nfailed to go up when c=-1");\
	trf_message("\nS: row=%d  column=%d  upi=%d  Sequence: realrow=%d  EC: realcol=%d",\
			r,i,upi,realr,c);\
	trf_message("\nS=%d  up=%d",\
			S[r][i],S[r-1][upi]);\
	break;\
}




{

	int i,length,fullcopy;
	unsigned char *x,*y;
	int realr,r,c,w;
	int upi,k;
	int legitimateZero;



	if(ldong) /* null statement */;

	else
	{
		w=bandradius;
		x=Sequence;
		y=EC;


		realr=Maxrealrow;
		r=Maxrow;
		c=Maxcol;
		k=(Maxcol-Bandcenter[r]+size)%size;
		if(size-k<=k) k=-(size-k);
		i=w+k;          
		fullcopy=(Maxcol+1)%size;

		AlignPair.score=S[r][i]; 
		length=0;
		Copynumber=0;
		for (;;)
		{      
			/* stop at zeros or -1000 for local */
			/* stop at r=0 for global */
			
			//change 8.20.12 Gary Benson
			//check for legitimate zero in alignment score (not from local alignment zeroing)
			//and allow to go back past that zero in alignment array
			//used to capture some alignments with mutations near the beginning
			//check to see if LOCAL zero score is a legitimate zero not a local zero
			legitimateZero=0;
			if((option==LOCAL)&&(S[r][i]==0)&&r!=0)
			{
				//get upi
				k=(c-Bandcenter[r-1]+size)%size;
				if(size-k<=k) k=-(size-k);
				upi=w+k;
				//get k
				k=(Bandcenter[r]-Bandcenter[r-1]+size)%size;
				if(size-k<=k) k=-(size-k);
				if(k>=1)      /* band shifts right */
				{
					if(i<=2*w-k)
					{
						if((S[r][i]==S[r-1][upi-1]+match(x[realr], y[c]))
						   ||(S[r][i]==S[r-1][upi]+Delta)
						   ||(S[r][i]==S[r][i-1]+Delta)) legitimateZero=1;
					}
					else if(i==2*w-k+1)
					{
						if((S[r][i]==S[r-1][upi-1]+match(x[realr], y[c]))
						   ||(S[r][i]==S[r][i-1]+Delta)) legitimateZero=1;
					}
					else 
					{
						if(S[r][i]==S[r][i-1]+Delta) legitimateZero=1;						
					}
				}
				else          /* (k<=0) band shifts left */
				{
					k=-k;
					if(i<=k-1)
					{
						if(S[r][i]==S[r][i-1]+Delta) legitimateZero=1;
					}
					else if(i==k)
					{
						if((S[r][i]==S[r-1][upi]+Delta)
						   ||(S[r][i]==S[r][i-1]+Delta)) legitimateZero=1;	
					}
					else
					{
						if((S[r][i]==S[r-1][upi-1]+match(x[realr], y[c]))
						   ||(S[r][i]==S[r-1][upi]+Delta)
						   ||(S[r][i]==S[r][i-1]+Delta)) legitimateZero=1;
					}
				}
				//debug
				//if (legitimateZero) {
					//report where
				//	printf("\nlegitimate zero at r:%d,i:%d",r,i);
				//}
			}
			
			
			/* stop at zeros or -1000 for local */
			/* stop at r=0 for global */
			if (r==0
				||((option==LOCAL)&&(S[r][i]<=0)&&!legitimateZero)
				||((option==GLOBAL)&&(r==0)&&(c==-1)))
				//debug (uncomment to restore)	
			/*
			 if (((option==LOCAL)&&(S[r][i]<=0))||((option==GLOBAL)&&(r==0)&&(c==-1)))
			 */
			{
				legitimateZero=0;
				AlignPair.length=length;
				if(Maxcol>=c){Copynumber+=((double)(Maxcol-c))/size;}
				else {Copynumber+=((double)(Maxcol+size-c))/size;}return;
			}
			else
			{
				k=(c-Bandcenter[r-1]+size)%size;
				if(size-k<=k) k=-(size-k);
				upi=w+k;
				/* in case global goes to c==-1 */
				if((option==GLOBAL)&&(c==-1))
				{
					test_up
						report_error_up
				}
				else
				{
					k=(Bandcenter[r]-Bandcenter[r-1]+size)%size;
					if(size-k<=k) k=-(size-k);
					if(k>=1)      /* band shifts right */
					{
						if(i<=2*w-k)
						{
							test_match_mismatch
								test_up
								test_left
								report_error_match_up_left;
						}
						else if(i==2*w-k+1)
						{
							test_match_mismatch
								test_left
								report_error_match_left;
						}
						else 
						{
							test_left
								report_error_left;
						}
					}
					else          /* (k<=0) band shifts left */
					{
						k=-k;
						if(i<=k-1)
						{
							test_left
								report_error_left;
						}
						else if(i==k)
						{
							test_up
								test_left
								report_error_up_left;
						}
						else
						{
							test_match_mismatch
								test_up
								test_left
								report_error_match_up_left;
						}
					}
				}
			}
		}
	}
}

/************************************************************/ 



/*******************************************************************/

/*********************  get_pair_alignment_with_copynumber()  **********************/

/*******************************************************************/


void get_pair_alignment_with_copynumber(int size)


	/* for a repeat of EC, do a traceback alignment, */
	/* ending at row Maxrow and column Maxcol */






{

	int i,j,si,length,adjlength,fullcopy;
	unsigned char *x,*y;



	x=Sequence;
	y=EC;


	i=Maxrealrow;
	si=Maxrow;
	j=Maxcol;
	adjlength=size-1;
	fullcopy=(j+1)%size;

	AlignPair.score=S[si][j]; 

	length=0;
	Copynumber=0;
	for (;;)
	{      
		/* stop at zeros or -1000 */
		if (S[si][j]<=0)
		{
			AlignPair.length=length;
			if(Maxcol>=j){Copynumber+=((double)(Maxcol-j))/size;}
			else {Copynumber+=((double)(Maxcol+size-j))/size;}return;
		}

		/* check match/mismatch branch */

		else if (S[si][j]==S[si-1][(j+adjlength)%size]+match(x[i], y[j]))
		{
			length++;
			if(j==fullcopy)Copynumber++;
			fill_align_pair(x[i],y[j],length,i,j);
			i--;
			si--;
			j=(j+adjlength)%size;
		}

		/* check deletion branch */
		else if (S[si][j]==S[si-1][j]+Delta)
		{
			length++;
			fill_align_pair(x[i],'-',length,i,(j+1)%size);
			i--;
			si--;
		}

		/* check other deletion branch */
		else if (S[si][j]==S[si][(j+adjlength)%size]+Delta)
		{
			length++;
			if(j==fullcopy)Copynumber++;
			fill_align_pair('-',y[j],length,i+1,j);
			j=(j+adjlength)%size;
		}
		else
		{
			trf_message("\nget_pair_alignment_with_copynumber: error in trace back");
			trf_message("\nrow=%d  column=%d",i,j);
			trf_message("\nS=%d  Sleft=%d  Sup=%d  Sdiag=%d  match=%d",S[si][j],
					S[si][(j+adjlength)%size],S[si-1][j],
					S[si-1][(j+adjlength)%size],match(x[i], y[j]));

			break;
		}
	}


}










/*******************************************************************/

/**************************  reverse()  ****************************/

/*******************************************************************/


void reverse()

	/* reverses the alignment in AlignPair */




{
	int j, tempi,ml;
	char temp;

	ml=AlignPair.length;

	for (j=1;j<=ml/2;j++)
	{

		temp=AlignPair.textprime[j];
		AlignPair.textprime[j]=AlignPair.textprime[ml-j+1];
		AlignPair.textprime[ml-j+1]=temp;

		temp=AlignPair.textsecnd[j];
		AlignPair.textsecnd[j]=AlignPair.textsecnd[ml-j+1];
		AlignPair.textsecnd[ml-j+1]=temp;

		tempi=AlignPair.indexprime[j];
		AlignPair.indexprime[j]=AlignPair.indexprime[ml-j+1];
		AlignPair.indexprime[ml-j+1]=tempi;

		tempi=AlignPair.indexsecnd[j];
		AlignPair.indexsecnd[j]=AlignPair.indexsecnd[ml-j+1];
		AlignPair.indexsecnd[ml-j+1]=tempi;
	}
}









void shift_pattern_indices (int patternsize)
{
	int downshift, upshift, l;

	downshift = AlignPair.indexsecnd[1];
	upshift = patternsize-downshift;
	for(l=1;l<=AlignPair.length;l++)
	{
		if(AlignPair.indexsecnd[l]>=downshift)
			AlignPair.indexsecnd[l]-=downshift;
		else
			AlignPair.indexsecnd[l]+=upshift;
	}
} 

/*******************************************************************/

/***********************  alt_print_alignment()  ***********************/

/*******************************************************************/

void alt3_print_alignment(int patternwidth) /* change added to make indices run
					       from 1 to patternsize in the 
					       output */

	/* prints out the alignment in AlignPair */

{

	extern int pwidth;
	int i, j, g, h, first, m;

	shift_pattern_indices(patternwidth);

	if (pwidth>0)
	{
		if(AlignPair.indexprime[1]!=1)
		{
			m=AlignPair.indexprime[1]-10;
			if (m<1) m=1;
			j=m;
			fprintf(Fptxt,"  %9d ",j);
			for(i=1;i<=10;i++)
			{
				fputc(Sequence[j],Fptxt);
				j++;
				if(j==AlignPair.indexprime[1]) break;
			}
			fprintf(Fptxt,"\n\n");
		}
		g=1;
		for (;;)
		{
			j=g;
			fprintf(Fptxt,"            ");
			first=TRUE;
			h=0;
			for (i=0;i<pwidth-10; ++i)
			{
				if (j==AlignPair.length+1)
					break;
				if ((!first)&&(AlignPair.indexsecnd[j]==0)
						&&(AlignPair.indexsecnd[j-1]!=0))
				{
					if ((patternwidth>6)||(pwidth-10-h<(2*patternwidth)))
						break;
					fprintf(Fptxt," ");
					h++;
				}
				first=FALSE;
				if ((AlignPair.textprime[j]!=AlignPair.textsecnd[j])&&
						(AlignPair.textprime[j]!='-') &&
						(AlignPair.textsecnd[j]!='-'))
				{fputc('*',Fptxt);}
				else fputc(' ',Fptxt);
				j++;
				h++;
			}  
			fputc('\n',Fptxt);
			j=g;
			fprintf(Fptxt,"  %9d ",AlignPair.indexprime[j]);
			first=TRUE;
			h=0;
			for (i=0;i<pwidth-10; ++i)
			{
				if (j==AlignPair.length+1) break;
				if ((!first)&&(AlignPair.indexsecnd[j]==0)
						&&(AlignPair.indexsecnd[j-1]!=0))
				{
					if ((patternwidth>6)||(pwidth-10-h<(2*patternwidth)))
						break;
					fprintf(Fptxt," ");
					h++;
				}
				first=FALSE;
				fputc(AlignPair.textprime[j++],Fptxt);
				h++;
			}  
			fputc('\n',Fptxt);
			j=g;
			fprintf(Fptxt,"  %9d ",AlignPair.indexsecnd[j]+1); /* +1 added to make 
									      indices run from 
									      1 to 
									      patternwidth in 
									      output */
			first=TRUE;
			h=0;
			for (i=0;i<pwidth-10; ++i)
			{
				if (j==AlignPair.length+1)
					break;
				if ((!first)&&(AlignPair.indexsecnd[j]==0)
						&&(AlignPair.indexsecnd[j-1]!=0))
				{
					if ((patternwidth>6)||(pwidth-10-h<(2*patternwidth)))
						break;
					fprintf(Fptxt," ");
					h++;
				}
				first=FALSE;
				fputc(AlignPair.textsecnd[j++],Fptxt);
				h++;
			}
			fprintf(Fptxt,"\n\n");
			g=j;
			if (j==AlignPair.length+1) break;
		}
		if(AlignPair.indexprime[AlignPair.length]!=Length)
		{
			m=AlignPair.indexprime[AlignPair.length]+10;
			if (m>Length) m=Length;
			j=AlignPair.indexprime[AlignPair.length]+1;
			fprintf(Fptxt,"  %9d ",j);
			for(i=1;i<=10;i++) 
			{
				fputc(Sequence[j],Fptxt); 
				j++;
				if(j>m) break;
			}
			fprintf(Fptxt,"\n\n");
		}
	}
	else
		fprintf(Fptxt,"Error, pwidth<=0, can't report alignments.\n");
}



void print_alignment_headings(int consensuslength)
{
	/* headings */
	if (Heading==0)
	{
		Heading=1;
	}
	fprintf(Fptxt,"\n\n<A NAME=\"%d--%d,%d,%3.1f,%d,%d\">",
			AlignPair.indexprime[1],
			AlignPair.indexprime[AlignPair.length],Period,Copynumber,consensuslength,(int) OUTPUTcount);
	fprintf(Fptxt,"</A>");


#if defined(WINDOWSGUI)
	fprintf(Fptxt,"<P>See <FONT COLOR=\"#0000FF\">Alignment Explanation</FONT> in Tandem Repeats Finder Help</P><BR>\n");
#elif defined(WINDOWSCONSOLE)
	fprintf(Fptxt,"<A HREF=\"http://tandem.bu.edu/trf/trf.definitions.html#alignment\" target =\"explanation\">Alignment explanation</A><BR><BR>\n");
#elif defined(UNIXGUI)
	fprintf(Fptxt,"<P>See <FONT COLOR=\"#0000FF\">Alignment Explanation</FONT> in Tandem Repeats Finder Help</P><BR>\n");
#elif defined(UNIXCONSOLE)
	fprintf(Fptxt,"<A HREF=\"http://tandem.bu.edu/trf/trf.definitions.html#alignment\" target =\"explanation\">Alignment explanation</A><BR><BR>\n");
#endif




	fprintf(Fptxt,"    Indices: %d--%d",
			AlignPair.indexprime[1],
			AlignPair.indexprime[AlignPair.length]);
	fprintf(Fptxt,"  Score: %d",Maxscore);
	fprintf(Fptxt,"\n    Period size: %d  Copynumber: %3.1f  Consensus size: %d\n\n",
			Period,Copynumber,consensuslength);
}  

#if WEIGHTCONSENSUS
/*******************************************************************/

/****************************  get_consensus()  ********************/

/*******************************************************************/

void get_consensus(int patternsize)

{
	int c,lastindex,j,i,max,letters,newinsert,insertA,insertC,insertG,insertT;
	char maxchar;

	/* initialize counts */

	for (c=0;c<=2*(MAXPATTERNSIZE);c++)
	{
		Consensus.A[c]=0;
		Consensus.C[c]=0;
		Consensus.G[c]=0;
		Consensus.T[c]=0;
		Consensus.dash[c]=0;
		Consensus.insert[c]=0;  /* times insert occurs */
		Consensus.letters[c]=0; /* number of letters in all inserts */
		Consensus.total[c]=0;   /* occurrences of a position */
		Consensus.pattern[c]=DASH;
	}

	/* start consensus */

	lastindex=-1;

	i=1;
	while (i<=AlignPair.length)
	{
		if (AlignPair.indexsecnd[i]!=lastindex)
		{
			switch(AlignPair.textprime[i]){
				case 'A':
					Consensus.A[2*AlignPair.indexsecnd[i]+1]++;
					break;
				case 'C':
					Consensus.C[2*AlignPair.indexsecnd[i]+1]++;
					break;
				case 'G':
					Consensus.G[2*AlignPair.indexsecnd[i]+1]++;
					break;
				case 'T':
					Consensus.T[2*AlignPair.indexsecnd[i]+1]++;
					break;
				case DASH:
					Consensus.dash[2*AlignPair.indexsecnd[i]+1]++;
					break;
			}
			Consensus.total[2*AlignPair.indexsecnd[i]+1]++;
			if (lastindex!=-1)
			{
				if (AlignPair.indexsecnd[i]==patternsize-1)
				{
					Consensus.total[0]++;
				}
				else
				{
					Consensus.total[2*AlignPair.indexsecnd[i]+2]++;
				}
			}
			lastindex=AlignPair.indexsecnd[i];
			i++;

		}
		else            /* AlignPair.indexsecnd[i]==lastindex */
		{
			Consensus.insert[2*AlignPair.indexsecnd[i]]++;
			insertA=0;
			insertC=0;
			insertG=0;
			insertT=0;
			while((AlignPair.indexsecnd[i]==lastindex)
					&&
					(i<=AlignPair.length))
			{

				switch(AlignPair.textprime[i]){
					case 'A':
						if (insertA==0)
						{
							Consensus.A[2*AlignPair.indexsecnd[i]]++;
							insertA=1;
							break;
						}
					case 'C':
						if (insertC==0)
						{
							Consensus.C[2*AlignPair.indexsecnd[i]]++;
							insertC=1;
							break;
						}
					case 'G':
						if (insertG==0)
						{
							Consensus.G[2*AlignPair.indexsecnd[i]]++;
							insertG=1;
							break;
						}
					case 'T':
						if (insertT==0)
						{
							Consensus.T[2*AlignPair.indexsecnd[i]]++;
							insertT=1;
							break;
						}
				}
				Consensus.letters[2*AlignPair.indexsecnd[i]]++;
				i++;
			}
		}

	} 

	/* get consensus for letters that exist */
	for(i=1;i<=2*(patternlength);i+=2)
	{
		letters=Consensus.total[i]-Consensus.dash[i];
		max=Consensus.A[i];maxchar='A';
		if(max<Consensus.C[i]){max=Consensus.C[i];maxchar='C';}
		if(max<Consensus.G[i]){max=Consensus.G[i];maxchar='G';}
		if(max<Consensus.T[i]){max=Consensus.T[i];maxchar='T';}

		/* weighted consensus */
		if (((max*Alpha)+((letters-max)*Beta)+(Consensus.dash[i]*Delta))
				>=(letters*Delta))
			Consensus.pattern[i]=maxchar;
		else Consensus.pattern[i]=DASH;

	}

	/* get consensus for inserted letters */
	for(i=0;i<=2*(patternlength);i+=2)
	{
		if (Consensus.total[i]!=0)
		{
			max=Consensus.A[i];maxchar='A';
			if(max<Consensus.C[i]){max=Consensus.C[i];maxchar='C';}
			if(max<Consensus.G[i]){max=Consensus.G[i];maxchar='G';}
			if(max<Consensus.T[i]){max=Consensus.T[i];maxchar='T';}
			if (((max*Alpha)+
						((Consensus.insert[i]-max)*Beta)+
						((Consensus.letters[i]-Consensus.insert[i])*Delta)+
						((Consensus.total[i]-Consensus.insert[i])*Delta))
					>
					(Consensus.letters[i]*Delta))
				Consensus.pattern[i]=maxchar;
			else Consensus.pattern[i]=DASH;

		}
	}

	/* compress consensus */

	j=0;
	for(i=0;i<=2*patternlength;i++)
		if (Consensus.pattern[i]!=DASH)
		{
			Consensus.pattern[j]=Consensus.pattern[i];
			j++;
		}
	ConsClasslength=j;

}





#else


/*******************************************************************/

/****************************  get_consensus()  ********************/

/*******************************************************************/

void get_consensus(int patternlength)

{
	int c,lastindex,j,i,max;
	char maxchar;

	/* initialize counts */

	/* Y. Hernandez Oct 15, 2018 */
	/* Changed from MAXPATTERNSIZE to MAXPATTERNSIZECONSTANT since that
	 * is what is used to init the arrays in Consensus. Using
	 * MAXPATTERNSIZE will result in an out-of-bounds error if > 2000.
	 */
	for (c=0;c<=2*(MAXPATTERNSIZECONSTANT);c++)
	{
		Consensus.A[c]=0;
		Consensus.C[c]=0;
		Consensus.G[c]=0;
		Consensus.T[c]=0;
		Consensus.dash[c]=0;
		Consensus.insert[c]=0;
		Consensus.total[c]=0;
		Consensus.pattern[c]=DASH;
	}

	/* start consensus */

	lastindex=-1;

	i=1;
	while (i<=AlignPair.length)
	{
		if (AlignPair.indexsecnd[i]!=lastindex)
		{
			switch(AlignPair.textprime[i]){
				case 'A':
					Consensus.A[2*AlignPair.indexsecnd[i]+1]++;
					break;
				case 'C':
					Consensus.C[2*AlignPair.indexsecnd[i]+1]++;
					break;
				case 'G':
					Consensus.G[2*AlignPair.indexsecnd[i]+1]++;
					break;
				case 'T':
					Consensus.T[2*AlignPair.indexsecnd[i]+1]++;
					break;
				case DASH:
					Consensus.dash[2*AlignPair.indexsecnd[i]+1]++;
					break;
			}
			if (lastindex!=-1)
			{
				if (AlignPair.indexsecnd[i]==patternlength-1)
				{
					Consensus.total[0]++;
				}
				else
				{
					Consensus.total[2*AlignPair.indexsecnd[i]+2]++;
				}
			}
			lastindex=AlignPair.indexsecnd[i];
			i++;

		}
		else            /* AlignPair.indexsecnd[i]==lastindex */
		{
			Consensus.insert[2*AlignPair.indexsecnd[i]]++;

			while((AlignPair.indexsecnd[i]==lastindex)
					&&
					(i<=AlignPair.length))
			{

				switch(AlignPair.textprime[i]){
					case 'A':
						Consensus.A[2*AlignPair.indexsecnd[i]]++;
						break;
					case 'C':
						Consensus.C[2*AlignPair.indexsecnd[i]]++;
						break;
					case 'G':
						Consensus.G[2*AlignPair.indexsecnd[i]]++;
						break;
					case 'T':
						Consensus.T[2*AlignPair.indexsecnd[i]]++;
						break;
					case DASH:
						Consensus.dash[2*AlignPair.indexsecnd[i]]++;
						break;
				}
				i++;
			}
		}

	} 

	for(i=1;i<=2*(patternlength);i+=2)
	{
		max=Consensus.dash[i];
		maxchar=DASH;
		if(max<Consensus.A[i]){max=Consensus.A[i];maxchar='A';}
		if(max<Consensus.C[i]){max=Consensus.C[i];maxchar='C';}
		if(max<Consensus.G[i]){max=Consensus.G[i];maxchar='G';}
		if(max<Consensus.T[i]){max=Consensus.T[i];maxchar='T';}
		Consensus.pattern[i]=maxchar;
	}
	for(i=0;i<=2*(patternlength);i+=2)
	{
		if ((Consensus.total[i]!=0)
				&&
				(((float)Consensus.insert[i]/Consensus.total[i])>=0.5))
		{
			max=Consensus.A[i];maxchar='A';
			if(max<Consensus.C[i]){max=Consensus.C[i];maxchar='C';}
			if(max<Consensus.G[i]){max=Consensus.G[i];maxchar='G';}
			if(max<Consensus.T[i]){max=Consensus.T[i];maxchar='T';}
			/*      if (max<Consensus.total[i]/2){maxchar='X';} */
		}


		else maxchar=DASH;
		Consensus.pattern[i]=maxchar;

	}
	j=0;
	for(i=0;i<=2*patternlength;i++)
		if (Consensus.pattern[i]!=DASH)
		{
			Consensus.pattern[j]=Consensus.pattern[i];
			j++;
		}
	ConsClasslength=j;

}


#endif


struct distanceentry *_DistanceEntries;

/****************************************************/

struct distancelist *new_distancelist()
{
	int g,N,K;
	struct distanceentry *ptr;

	struct distancelist *objptr=(struct distancelist *)
		calloc(MAXDISTANCE+1,sizeof(struct distancelist));

	K = Min_Distance_Entries + 1; 
	N = MAXDISTANCE + 1;
	//ptr = _DistanceEntries = malloc( (((N*(N+1) - K*(K+1))/2) + K*(K-1)) * sizeof(struct distanceentry) );
	ptr = _DistanceEntries = malloc( ((K+N)*(N-K+1)/2 + K*(K-1) ) * sizeof(struct distanceentry) );

	for(g=1;g<=MAXDISTANCE;g++) {
		objptr[g].entry = ptr;
		memset(objptr[g].entry, 0, (max(g,Min_Distance_Entries)+1) * sizeof(struct distanceentry));
		ptr += (max(g,Min_Distance_Entries)+1);
	}

	/*

	   for(g=1;g<=MAXDISTANCE;g++)
	   {
	   objptr[g].entry=(struct distanceentry *)
	   calloc(max(g,Min_Distance_Entries)+1,sizeof(struct distanceentry));
	   }
	 */  

	return(objptr);
}

void clear_distancelist(struct distancelist *objptr)
{
	int g;
	for(g=1;g<=MAXDISTANCE;g++)
	{
		objptr[g].lowindex=0;    
		objptr[g].highindex=max(g,Min_Distance_Entries);
		objptr[g].numentries=0;
		objptr[g].nummatches=0;
	}
}
void init_links()
{
	Distance[0].linkup=MAXDISTANCE+1;
}

void add_tuple_match_to_Distance_entry(int location, int size, int d,
		struct distancelist *objptr)
{
	int *lo, *hi;
	int *z, *m;
	int windowleftend, windowsize;
	struct distancelist *list;

	/* get distance window pointers */
	windowsize=max(d,Min_Distance_Entries)+1; /* this value is used to */
	/* mod the index to the */
	/* entries.  The entries */
	/* run from 0 to */
	/* max(d,Min_Distance_Entries) */ 
	list=&(objptr[d]);
	hi=&(list->highindex);
	lo=&(list->lowindex);
	z=&(list->numentries);
	m=&(list->nummatches);
	if ((*z)!=0) /* there exist previous entries on this list */
	{
		/* first remove trailing entries, i.e., */
		/* those with location<location-d or */
		/* <location-Min_Distance_Window, whichever is further back */
		windowleftend=min(location-d,location-Min_Distance_Window)+1;
		/* note that preceding is based on Min_Distance_Window */
		/* rather than Min_Distance_Entries */

		while((list->entry[*lo].location<windowleftend)&&((*z)>0))
		{
			(*z)--;
			(*m)-=list->entry[*lo].size;
			(*lo)++;
			(*lo)%=windowsize;
		}

	}
	if(((*z)!=0)&&(list->entry[*hi].location==location-1)) 
		/* there are still more entries and this is an adjacent tuple, */
		/* just add on to last entry */
	{
		list->entry[*hi].location++;
		list->entry[*hi].size++;
		(*m)++;
	}
	else          /* need a new entry here */
	{
		(*z)++;
		(*hi)++;
		(*hi)%=windowsize;
		list->entry[*hi].location=location;
		list->entry[*hi].size=size;
		(*m)+=size;
	}
}

void link_Distance_window(int d)
{

	int t,f,h;
	/* get next highest tag */
	t=(int)ceil(d/TAGSEP);
	if(Tag[t]<d)            /* Tag[t] is the largest index less or */
		/* equal to (t)x(TAGSEP) that is linked */
	{
		f=Tag[t];           /* f is first linked index below d */
		while((t<=Toptag)&&(Tag[t]<d))
		{
			Tag[t]=d;
			t++;
		}
	}
	else if(Tag[t]>d)     /* follow links to insert d */
	{
		f=Tag[t];
		while(f>d) f=Distance[f].linkdown;
		/* f is first linked index below d */
		if (f==d)trf_message("\nTag error following links.  f==d=%d",d);
	}
	else //if(Tag[t]==d) // 'if' Not needed; only remaining case
	{
		trf_message("\nTag error Tag[%d]=%d",t,d);
		exit(-2);
	}

	/* link in d */
	Distance[d].linkdown=f;
	h=Distance[f].linkup;
	Distance[d].linkup=h;
	Distance[f].linkup=d;
	if(h<=MAXDISTANCE) Distance[h].linkdown=d;
	Distance[d].linked=TRUE;
}


void untag_Distance_window(int d, int linkdown)
{

	int t;
	/* get next highest tag */
	t=(int)ceil(d/TAGSEP);
	if(Tag[t]!=d) return;    /* Tag[t] is the largest index less or */
	/* equal to (t)x(TAGSEP) that is linked */
	else
	{
		while((t<=Toptag)&&(Tag[t]==d)) /* check higer tags and replace d */
			/* with its linkdown */
		{
			Tag[t]=linkdown;
			t++;
		}
	}
}

int no_matches_so_unlink_Distance(int d, int location, 
		struct distancelist *objptr)
{
	int *lo, *hi, g, h;
	int *z, *m;
	int windowleftend, windowsize;
	struct distancelist *list;

	windowsize=max(d,Min_Distance_Entries)+1; /* this value is used to */
	/* mod the index to the */
	/* entries.  The entries */
	/* run from 0 to */
	/* max(d,Min_Distance_Entries) */ 

	list=&(objptr[d]);
	hi=&(list->highindex);
	lo=&(list->lowindex);
	z=&(list->numentries);
	m=&(list->nummatches);
	if ((*z)!=0) /* there exist previous entries on this list */
	{
		/* first remove trailing entries, i.e., */
		/* those with location<location-d or */
		/* <location-Min_Distance_Window, whichever is further back */
		windowleftend=min(location-d,location-Min_Distance_Window)+1;
		/* note that preceding is based on Min_Distance_Window */
		/* rather than Min_Distance_Entries */

		while((list->entry[*lo].location<windowleftend)&&((*z)>0))
		{
			(*z)--;
			(*m)-=list->entry[*lo].size;
			(*lo)++;
			(*lo)%=windowsize;
			if((*lo)>windowsize)trf_message("\n    no_matches; *lo:%d windowsize:%d",*lo,windowsize);
		}
	}
	if((*z)==0) /* no more matches, so unlink */
	{
		/* given a distance d with zero matches, unlink from list */
		g=Distance[d].linkdown;
		h=Distance[d].linkup;

		Distance[g].linkup=h;
		if (h<=MAXDISTANCE) Distance[h].linkdown=g;
		Distance[d].linked=FALSE;

		/* once unlinked, we must also untag if it is a tag */
		untag_Distance_window(d,g);


		return(1);
	}
	return(0);  
}








int GetTopPeriods(unsigned char* pattern, int length, int* toparray)
{
	int topind;
	double topval;
	int heads[16];
	int *history;
	double* counts;
	int i,t,end,tupid;
	int curr,dist;
	double n,xysum,xsum,ysum,x2sum,s;

	/* allocate an array of counts */
	counts = (double*) calloc(length,sizeof(double));
	if(counts==NULL) return 1;

	/* allocate history array */
	history = (int*) malloc(length*sizeof(int));
	if(history==NULL)
	{
		free(counts);
		return 1;
	}

	/* clear the heads array which point into history array */
	for(i=0;i<16;i++) heads[i]=-1;

	/* scan pattern for tuples of size 2 */
	for(i=0,end=length-2;i<=end;i++)
	{
		/* figure out tuple id */
		tupid = Index[pattern[i]]*4+Index[pattern[i+1]];

		/* record last occurence into history and update heads[] pointer */
		history[i] = heads[tupid];
		heads[tupid]=i;

		/* loop into history and add distances */
		/* 11/17/15 G. Benson */
		/* limit maximum length of distance recorded between tuples to MAXDISTANCECONSTANT*3 = 6,000*/
		/* this should be long enough to deter finding periods that are not the most frequent */
		/* Without this change, this procudure is quadratic in the length, which could be several million */
		/* and caused the program to hang with long centromeric repeats */
		/* for(curr=i;history[curr]!=-1;curr=history[curr])*/
		dist = 0;
		for(curr=i;((history[curr]!=-1)&&(dist<(MAXDISTANCECONSTANT*3)));curr=history[curr])
		{
			dist = i-history[curr];
			counts[dist]+=1.0;
		}
	}

	/* compute slope using least-square regression */
	xysum=xsum=ysum=x2sum=0.0;
	end = length-2;
	for(i=1;i<=end;i++)
	{
		xysum += (i*counts[i]);
		xsum += (i);
		ysum += (counts[i]);
		x2sum += (i*i);
	}
	n = end;
	s = (n*xysum-xsum*ysum)/(n*x2sum-xsum*xsum);

	/* flatten trend by adding -s per increment */
	end = length-2;
	for(i=1;i<=end;i++)
	{
		counts[i] = counts[i] - i*s;
	}

	/* pick highest values */
	end = length-2;
	if(end>MAXDISTANCE) end = MAXDISTANCE; /* 3/14/05 accepts smaller multiples is best ones are too large */
	for(t=0;t<NUMBER_OF_PERIODS;t++)
	{
		/* do t passes to find t highes counts */
		topind=0;
		topval=0.0;
		for(i=1;i<=end;i++)
		{
			if(counts[i]>topval)
			{
				topind=i;
				topval = counts[i];
			}
		}

		/* copy to array passed as parameter */
		toparray[t] = topind;
		counts[topind]=0.0;
	}

	/* free memory */
	free(counts);
	free(history);

	return 0;
}

int multiples_criteria_4(int found_d)
{
	int g,lowerindex,upperindex;
	int topperiods[NUMBER_OF_PERIODS];
	unsigned char* pattern;
	int length;

	lowerindex=AlignPair.indexprime[AlignPair.length];
	upperindex=AlignPair.indexprime[1];
	pattern = Sequence+lowerindex;
	length = upperindex-lowerindex+1;

	/* size one is verified based on composition */
	/* only returns true if > 80% comp for any nucleotide */
	if(found_d==1)
	{
		int comps[4]={0};
		int total=0;
		int maxind=0;
		float percentmatch;

		for(g=0;g<length;g++)
		{
			comps[Index[pattern[g]]]++;
		}
		for(g=0;g<4;g++)
		{
			total+=comps[g];
			if(comps[g]>comps[maxind]) maxind=g;
		}
		percentmatch = comps[maxind]*100.0f/total;
		if(percentmatch>=80.0f) return TRUE; 
		else return FALSE;
	}


	if(GetTopPeriods(pattern, length, topperiods)) /* gettopperiods returns zero on success */
	{
		fprintf(stderr,"\nUnable to allocate counts array in GetTopPeriods()!!!");
		exit(-1);
	}

	/* copy into global for compatibility with best_period_list */
	/*
	   for(g=0;g<NUMBER_OF_PERIODS; g++)
	   {
	   Sortmultiples[g] = topperiods[g];
	   }
	 */

	/* modified 5/23/05 G. Benson */
	for(g=0;g<NUMBER_OF_PERIODS_INTO_SORTMULTIPLES; g++)
		Sortmultiples[g] = topperiods[g];

	/*
	   for(g=0;g<NUMBER_OF_PERIODS; g++)
	   {
	   if(found_d==topperiods[g]) return(TRUE);
	   }
	 */

	/* modified 5/23/05 G. Benson */
	for(g=0;g<NUMBER_OF_PERIODS_TO_TEST; g++)
		if(found_d==topperiods[g]) return(TRUE);

	return(FALSE);  
}


/********************* new_meet_criteria_3 ***************************/

int new_meet_criteria_3(int d, int location, int tuplesize)
{

	struct distancelist *main_d_info, *range_d_info;
	int min_krun_matches,max_first_match_location,main_d_matches,
	    range_d_min_for_waiting_time_test,main_d_first_match_location,
	    range_d_first_match_location,range_d_matches;
	int low_end_of_range, high_end_of_range,lopointer,hipointer,
	    d_range,waiting_time_ok,waiting_time_d;
	int t,m,s,d_still_best;

	/***********************************************************/



	/* collect info about d for criteria tests */
	main_d_info=&(Distance[d]);
	min_krun_matches=main_d_info->k_run_sums_criteria;
	max_first_match_location
		= max(0,location - max(d,Min_Distance_Window)) 
		+ main_d_info->waiting_time_criteria;

	low_end_of_range=main_d_info->lo_d_range;
	high_end_of_range=main_d_info->hi_d_range;

	/* get number of matches in d */
	main_d_matches=main_d_info->nummatches;

	/* set up a test for checking waiting time on range distances */
	/* criteria is that they must have at least 35% of the krun matches */
	range_d_min_for_waiting_time_test=(int)(0.35*min(min_krun_matches,main_d_matches));

	/* calculate the location of the first match for d */
	main_d_first_match_location 
		= main_d_info->entry[main_d_info->lowindex].location /* end of first run of matches */
		- main_d_info->entry[main_d_info->lowindex].size     /* minus size of run */
		+ tuplesize;                                         /* plus tuplesize */


	/* test waiting time criteria for d */
	waiting_time_ok=0;
	waiting_time_d=0;
	if(main_d_first_match_location<=max_first_match_location)
	{
		waiting_time_ok=1;
		waiting_time_d=d;
	}


	/* does d meet all criteria? */
	if((main_d_matches>=min_krun_matches)&&(waiting_time_ok))
	{	
		return(TRUE);
	}

	/* no, so now look in the low range to see if d has most matches */
	/* accumulate matches in low range */
	/* test for waiting time if main d didn't pass test */
	hipointer=d;
	lopointer=d;
	t=main_d_info->linkdown;
	m=main_d_matches;
	d_still_best=TRUE;
	while((t>=low_end_of_range) && (d_still_best))
	{
		range_d_info=&(Distance[t]);
		s=range_d_info->linkdown;
		if(!no_matches_so_unlink_Distance(t,location,Distance))
		{
			if((range_d_matches=range_d_info->nummatches)>main_d_matches) 
				d_still_best=FALSE;
			else
			{
				lopointer=t;
				m+=range_d_info->nummatches;
				if((!waiting_time_ok)&&(range_d_matches>=range_d_min_for_waiting_time_test))
				{
					/* calculate the location of the first match for range d */
					range_d_first_match_location 
						= range_d_info->entry[range_d_info->lowindex].location /* end of first run of matches */
						- range_d_info->entry[range_d_info->lowindex].size     /* minus size of run */
						+ tuplesize;                                           /* plus tuplesize */

					/* do waiting time test; if successful, store t*/
					if(range_d_first_match_location<=max_first_match_location)
					{
						waiting_time_ok=1;
						waiting_time_d=t;
					}
				}
			}
		}
		t=s;
	}

	/* stop if d not best */
	if(!d_still_best)
		return(FALSE);

	/* now check that d is best in upper range */
	t=main_d_info->linkup;
	while((t<=high_end_of_range)&&(d_still_best))
	{
		range_d_info=&(Distance[t]);
		s=range_d_info->linkup;
		if(!no_matches_so_unlink_Distance(t,location,Distance))
		{
			if(range_d_info->nummatches>main_d_matches) 
				d_still_best=FALSE;
		}
		t=s;
	}

	/* stop if d not best */
	if(!d_still_best)
		return(FALSE);

	/* now test lowest range */
	if((m>=min_krun_matches)&&(waiting_time_ok))
	{	
		return(TRUE);
	}

	/* lower range didn't work, now test higher ranges */

	d_range=d-low_end_of_range+1;
	t=main_d_info->linkup;
	while(t<=high_end_of_range)
	{
		range_d_info=&(Distance[t]);
		s=range_d_info->linkup;
		hipointer=t;
		range_d_matches=range_d_info->nummatches;
		m+=range_d_matches;
		while(lopointer<hipointer-d_range+1)
		{
			m-=Distance[lopointer].nummatches;
			if(m<=0){trf_message("\n*** error in meet crieria, m<=0"); exit(-16);}
			/* if lopointer made waiting time okay, clear out waiting time variables */
			if((waiting_time_ok)&&(lopointer==waiting_time_d))
			{
				waiting_time_ok=0;
				waiting_time_d=0;
			}
			lopointer=Distance[lopointer].linkup;
		}
		/* test for waiting time on range d no matter if waiting time is okay or not */
		if(range_d_matches>=range_d_min_for_waiting_time_test)
		{
			/* calculate the location of the first match for range d */
			range_d_first_match_location 
				= range_d_info->entry[range_d_info->lowindex].location /* end of first run of matches */
				- range_d_info->entry[range_d_info->lowindex].size     /* minus size of run */
				+ tuplesize;                                           /* plus tuplesize */

			/* do waiting time test; if successful, store t*/
			if(range_d_first_match_location<=max_first_match_location)
			{
				waiting_time_ok=1;
				waiting_time_d=t;
			}
		}

		/* now test range */
		if((m>=min_krun_matches)&&(waiting_time_ok))
		{	
			return(TRUE);
		}
		t=s;
	}

	return(FALSE);
}

/****************************************************/




/******************** flanking sequence *********************/

void print_flanking_sequence(int flank_length)
{

	int m,n,k,i,j;

	m=AlignPair.indexprime[1]-flank_length;
	if (m<1) m=1;
	n=AlignPair.indexprime[AlignPair.length]+flank_length;
	if (n>Length) n=Length;

	if (m==AlignPair.indexprime[1])
	{
		fprintf(Fptxt,"\nLeft flanking sequence: None");
	}
	else
	{
		fprintf(Fptxt,"\nLeft flanking sequence: Indices %d -- %d\n",
				m,AlignPair.indexprime[1]-1);
		k=AlignPair.indexprime[1];
		j=m;
		for(;;)
		{
			for(i=1;i<=pwidth-10;i++)
			{
				fputc(Sequence[j],Fptxt);
				j++;
				if(j>=k) break;
			}
			fprintf(Fptxt,"\n");
			if(j>=k) break;
		} 
	}
	if (n==AlignPair.indexprime[AlignPair.length])
	{
		fprintf(Fptxt,"\n\nRight flanking sequence: None");
	}
	else
	{
		fprintf(Fptxt,"\n\nRight flanking sequence: Indices %d -- %d\n",
				AlignPair.indexprime[AlignPair.length]+1,n);
		j=AlignPair.indexprime[AlignPair.length]+1;
		for(;;)
		{
			for(i=1;i<=pwidth-10;i++)
			{
				fputc(Sequence[j],Fptxt);
				j++;
				if(j>n) break;
			}
			fprintf(Fptxt,"\n");
			if(j>n) break;
		} 
	}
	fprintf(Fptxt,"\n\n");

	return;

}


void printECtoAlignments(FILE* fp, int start, int width)
{
	int counter;
	int chars=0;

	fprintf(Fptxt,"\nConsensus pattern (%d bp):   ",width);
	for(counter=start;counter<width;counter++,chars++)
	{
		if(chars%65==0) fprintf(fp,"\n");
		fprintf(fp,"%c",EC[counter]);
	}
	for(counter=0;counter<start;counter++,chars++)
	{
		if(chars%65==0) fprintf(fp,"\n");
		fprintf(fp,"%c",EC[counter]);
	}

	fprintf(fp,"\n");

	return;
}

void printECtoBuffer(char *trg, int start, int width)
{
	int counter;

	for(counter=start;counter<width;counter++,trg++)
	{
		*trg=EC[counter];
	}
	for(counter=0;counter<start;counter++,trg++)
	{
		*trg=EC[counter];
	}
	*trg='\0';
	return;
}

void printECtoData(FILE* fp, int start, int width)
{
	int counter;

	for(counter=start;counter<width;counter++)
	{
		fprintf(fp,"%c",EC[counter]);
	}
	for(counter=0;counter<start;counter++)
	{
		fprintf(fp,"%c",EC[counter]);
	}
	fprintf(fp,"\n");
	return;
}



/************************************************************/ 

/*******************************************************************/

/***************************** statistics() **************************/

/*******************************************************************/

void get_statistics(int consensussize)
{
	int g,x,lp,rp,match,mismatch,indel,mindistance,maxdistance;
	int d;
	int best_match_distance,best_match_count;
	int i,count;
	double diversity[4];
	double  temp, entropy;
	int e,f;
	int size;
	int startECpos; /* to print consensus pattern */
	int ACGTcount[26];

	size=consensussize;


	for(g=1;g<=MAXDISTANCE+d_range(MAXDISTANCE);g++)
		Statistics_Distance[g]=0;

	match=0;
	mismatch=0;
	indel=0;



	lp=1;
	while(AlignPair.textsecnd[lp]=='-')
	{
		lp++;
		if(lp>AlignPair.length)
		{
			trf_message("\nError in statistics.");
			trf_message("\nInitial left pointer exceeds AlignPair.length while");
			trf_message("\nlooking for first non -");
			exit(-10); 
		}
	}
	rp=lp+1;
	if(rp>AlignPair.length)
	{
		trf_message("\nError in statistics.");
		trf_message("\nInitial right pointer exceeds AlignPair.length");
		exit(-10);
	} 
	while(AlignPair.indexsecnd[rp]!=AlignPair.indexsecnd[lp])
	{ 
		rp++;
		if(rp>AlignPair.length)
		{
			trf_message("\nError in statistics.");
			trf_message("\nInitial right pointer exceeds AlignPair.length while");
			trf_message("\nlooking for AlignPair.indexsecnd[lp]");
			exit(-10); 
		}
	}
	while(AlignPair.textsecnd[rp]=='-')
	{
		rp++;
		if(rp>AlignPair.length)
		{
			trf_message("\nError in statistics.");
			trf_message("\nInitial right pointer exceeds AlignPair.length while");
			trf_message("\nlooking for first non -");
			exit(-10); 
		}
	}
	if(AlignPair.indexsecnd[lp]!=AlignPair.indexsecnd[rp])
	{
		trf_message("\nError in statistics.");
		trf_message("\nInitial left pointer index not the same as");
		trf_message("\ninitial right pointer index");
		exit(-10);
	}

	/* now do the tests */
	mindistance=size;
	maxdistance=size;

	while((rp<=AlignPair.length)&&(lp<rp))
	{

		if((AlignPair.textsecnd[lp]!='-')&&(AlignPair.textsecnd[rp]!='-'))
		{
			if((AlignPair.textprime[lp]!='-')&&(AlignPair.textprime[rp]!='-'))
			{
				if(AlignPair.indexsecnd[lp]!=AlignPair.indexsecnd[rp])
				{
					trf_message("\nError in statistics.");
					trf_message("\nLeft pointer index not the same as right pointer index");
					trf_message("\non match or mismatch when nothing is dash.");
					trf_message("\nAlignPair.indexsecnd[%d]: %d, AlignPair.indexprime[%d]: %d"
							,lp,AlignPair.indexsecnd[lp],lp,AlignPair.indexprime[lp]);
					trf_message("\nAlignPair.indexsecnd[%d]: %d, AlignPair.indexprime[%d]: %d"
							,rp,AlignPair.indexsecnd[rp],rp,AlignPair.indexprime[rp]);

					exit(-10);
				}
				if(AlignPair.textprime[lp]==AlignPair.textprime[rp])
				{
					match++;
					d=AlignPair.indexprime[rp]-AlignPair.indexprime[lp];


					if(d<0)d=-d;

					/* protect for memory override January 08, 2003 */
					if(d<(4*MAXDISTANCE))
					{
						Statistics_Distance[d]++;
						if (d<mindistance)mindistance=d;
						if (d>maxdistance)maxdistance=d;



					}	  
				}
				else 
				{
					mismatch++;

				}
				lp++;
				rp++;


			}
			else if((AlignPair.textprime[lp]=='-')&&(AlignPair.textprime[rp]=='-'))
			{
				/* do nothing */
				if(AlignPair.indexsecnd[lp]!=AlignPair.indexsecnd[rp])
				{
					trf_message("\nError in statistics.");
					trf_message("\nLeft pointer index not the same as right pointer index");
					trf_message("\non do nothing.\nlp: %d,  rp: %d",lp,rp);
					exit(-10);
				}

				lp++;
				rp++;
			}
			else if((AlignPair.textprime[lp]=='-')||(AlignPair.textprime[rp]=='-'))
			{
				if(AlignPair.indexsecnd[lp]!=AlignPair.indexsecnd[rp])
				{
					trf_message("\nError in statistics.");
					trf_message("\nLeft pointer index not the same as right pointer index");
					trf_message("\non indel caused by dash in textprime.");
					trf_message("\nAlignPair.indexsecnd[%d]: %d, AlignPair.indexprime[%d]: %d"
							,lp,AlignPair.indexsecnd[lp],lp,AlignPair.indexprime[lp]);
					trf_message("\nAlignPair.indexsecnd[%d]: %d, AlignPair.indexprime[%d]: %d"
							,rp,AlignPair.indexsecnd[rp],rp,AlignPair.indexprime[rp]);

					exit(-10);
				}
				indel++;

				lp++;
				rp++;
			}
		}
		else if ((AlignPair.textsecnd[lp]=='-')&&(AlignPair.textsecnd[rp]=='-'))
		{
			if(AlignPair.indexsecnd[lp]!=AlignPair.indexsecnd[rp])
			{
				trf_message("\nError in statistics.");
				trf_message("\nLeft pointer index not the same as right pointer index");
				trf_message("\non match or mismatch when both secnd are dash.");
				trf_message("\nlp: %d,  rp: %d",lp,rp);
				exit(-10);
			}
			if (AlignPair.textprime[lp]==AlignPair.textprime[rp])
			{
				match++;
				d=AlignPair.indexprime[rp]-AlignPair.indexprime[lp];
				if(d<0) d=-d;

				/* protect for memory override January 08, 2003 */
				if(d<(4*MAXDISTANCE))
				{
					Statistics_Distance[d]++;
					if (d<mindistance)mindistance=d;
					if (d>maxdistance)maxdistance=d;
				}

			}
			else 
			{
				mismatch++;

			}
			lp++;
			rp++;
		}
		else if(AlignPair.textsecnd[lp]=='-')
		{

			indel++;
			lp++;
		}
		else if(AlignPair.textsecnd[rp]=='-') 
		{

			indel++;
			rp++;
		}
	}
	if(lp>=rp)
	{
		trf_message("\nError in statistics.");
		trf_message("\nLeft pointer >= right pointer");
		exit(-10); 
	}

	x=match+mismatch+indel;

	best_match_distance=0;
	best_match_count=0;
	for(g=mindistance;g<=maxdistance;g++)
		if(Statistics_Distance[g]!=0)
		{
			if(Statistics_Distance[g]>best_match_count)
			{
				best_match_count=Statistics_Distance[g];
				best_match_distance=g;
			}
		}


	reverse();
	Period=best_match_distance;

	if (!paramset.HTMLoff)
		print_alignment_headings(Classlength);

	/* save the starting position of consensus in EC */
	startECpos = (int) AlignPair.indexsecnd[1];

	if (!paramset.HTMLoff)
		alt3_print_alignment(Classlength);

	reverse();

	if (!paramset.HTMLoff) {	
		fprintf(Fptxt,"\nStatistics");
		fprintf(Fptxt,"\nMatches: %d,  Mismatches: %d, Indels: %d",match, mismatch, indel);
		fprintf(Fptxt,"\n        %0.2f            %0.2f        %0.2f",
				(float)match/x, (float)mismatch/x, (float)indel/x);
		fprintf(Fptxt,"\n");
		fprintf(Fptxt,"\nMatches are distributed among these distances:");
		for(g=mindistance;g<=maxdistance;g++)
			if(Statistics_Distance[g]!=0)
			{
				fprintf(Fptxt,"\n %3d  %3d  %0.2f",
						g,Statistics_Distance[g],(float)Statistics_Distance[g]/match);
			}
	}

	ACGTcount['A'-'A']=0;
	ACGTcount['C'-'A']=0;
	ACGTcount['G'-'A']=0;
	ACGTcount['T'-'A']=0;
	i=1;
	count=0;
	while (i<=AlignPair.length)
	{
		if(AlignPair.textprime[i]!='-')
		{
			ACGTcount[AlignPair.textprime[i]-'A']++;
			count++;
		}
		i++;
	}

	if(count!=0)
		trf_message("\n\nACGTcount: A:%3.2f, C:%3.2f, G:%3.2f, T:%3.2f",
				(double)ACGTcount['A'-'A']/count,(double)ACGTcount['C'-'A']/count,
				(double)ACGTcount['G'-'A']/count,(double)ACGTcount['T'-'A']/count); 
	else trf_message("\nError in statistics: ACGTcount=0");

	diversity[0]=(double)ACGTcount['A'-'A']/count;
	diversity[1]=(double)ACGTcount['C'-'A']/count;
	diversity[2]=(double)ACGTcount['G'-'A']/count;
	diversity[3]=(double)ACGTcount['T'-'A']/count;

	for(e=2;e>=0;e--)
		for(f=0;f<=e;f++)
		{
			if(diversity[f]<diversity[f+1])
			{
				temp=diversity[f+1];
				diversity[f+1]=diversity[f];
				diversity[f]=temp;
			}
		}

	entropy=(((diversity[0]==0)?0:(diversity[0]*(log(diversity[0])/log(2))))+
			((diversity[1]==0)?0:(diversity[1]*(log(diversity[1])/log(2))))+
			((diversity[2]==0)?0:(diversity[2]*(log(diversity[2])/log(2))))+
			((diversity[3]==0)?0:(diversity[3]*(log(diversity[3])/log(2)))));
	if(entropy<0)entropy=-entropy;

	if (!paramset.HTMLoff) {
		fprintf(Fptxt,"\n");
		fprintf(Fptxt,"\n");
	}

	/*  changed by Gary Benson, 6/1/99, to remove summary
	    information and add flanking sequence */

	/*  fprintf(Fptxt,"\n\n %d--%d & %d & %3.1f & %d & %d & %d & %d & %d & %d & %d & %d & %3.2f \\\\\n\n",
	    AlignPair.indexprime[AlignPair.length],
	    AlignPair.indexprime[1],best_match_distance,Copynumber,Classlength,
	    (int)(100*(float)match/x),(int)(100*(float)indel/x),AlignPair.score,
	    (int)(100*(double)ACGTcount['A'-'A']/count),
	    (int)(100*(double)ACGTcount['C'-'A']/count), 
	    (int)(100*(double)ACGTcount['G'-'A']/count),
	    (int)(100*(double)ACGTcount['T'-'A']/count),
	    entropy); 
	 */

	/* prints line showing the consensus pattern */

	if (!paramset.HTMLoff) {

		printECtoAlignments(Fptxt, startECpos, consensussize);


		if(print_flanking)
		{
			reverse();
			print_flanking_sequence(paramset.flankinglength);
			reverse();
		}       

	}
	/* end of change, 6/1/99 */

	/* Added by Yevgeniy Gelfand on Jan 27, 2010  */
	/* To have smaller sequences not send results */
	/* to disc to improve performance             */
	{

		IL      *newptr;

		/* create new index list element */
		newptr = (IL*) malloc(sizeof(IL));
		if(newptr==NULL)
		{
			FreeList(GlobalIndexList);
			GlobalIndexList=NULL;
			GlobalIndexListTail=NULL;
			return;
		}
		counterInSeq++;
		newptr->count = counterInSeq;



		/* assign data to fields */
		sprintf(newptr->ref,"%d--%d,%d,%3.1f,%d,%d",
				AlignPair.indexprime[AlignPair.length],AlignPair.indexprime[1],best_match_distance,Copynumber,Classlength,(int ) OUTPUTcount);

		newptr->first = AlignPair.indexprime[AlignPair.length];
		newptr->last = AlignPair.indexprime[1];
		newptr->period = best_match_distance;
		newptr->copies = Copynumber;
		newptr->size = Classlength;
		newptr->matches = (int)(100*(float)match/x);
		newptr->indels = (int)(100*(float)indel/x);
		newptr->score = AlignPair.score;
		newptr->acount = (int)(100*(double)ACGTcount['A'-'A']/count);
		newptr->ccount = (int)(100*(double)ACGTcount['C'-'A']/count); 
		newptr->gcount = (int)(100*(double)ACGTcount['G'-'A']/count);
		newptr->tcount = (int)(100*(double)ACGTcount['T'-'A']/count);
		newptr->entropy = entropy;


		/* allocate memory to place the pattern and copy data into it*/
		newptr->pattern = (char*) malloc((consensussize+1)*sizeof(char));
		if(newptr->pattern==NULL)
		{
			free(newptr);
			FreeList(GlobalIndexList);
			GlobalIndexList=NULL;
			GlobalIndexListTail=NULL;
			return;
		}
		printECtoBuffer(newptr->pattern, startECpos, consensussize);

		if (GlobalIndexList == NULL) /* first element */
		{
			GlobalIndexList = GlobalIndexListTail = newptr;
			GlobalIndexListTail->next = NULL;
		}
		else /* add new element to end of list */
		{
			GlobalIndexListTail->next = newptr;
			GlobalIndexListTail = newptr;
			GlobalIndexListTail->next = NULL;
		}

	}



}



/*******************************************************************/
void init_and_fill_coin_toss_stats2000_with_4tuplesizes(void)
{
	/* generated with the following parameters:
	   0.800000, 0 0, 4 1, 5 30, 7 160 */
	/* done */
	int waitdata80[] = {0, 18, 18, 18, 18, 18, 18, 18, 18, 18,
		18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 19, 19,
		19, 19, 28, 28, 27, 28, 29, 28, 29, 28, 29, 28, 29, 29, 29, 29, 29, 30,
		29, 29, 29, 30, 29, 30, 30, 30, 30, 30, 30, 30, 30, 30, 31, 31, 31, 31,
		31, 30, 30, 30, 31, 31, 31, 30, 31, 31, 31, 31,/*->*/ 30, 31, 31, 31, 31, 31,
		31, 31, 31, 31, 31, 32, 31, 31, 31, 31, 31, 31, 31, 31, 32, 32, 31, 31,
		31, 31, 32, 31, 32, 32, 31, 32, 31, 31, 31, 32, 31, 32, 32, 32, 32, 32,
		32, 31, 31, 32, 31, 31, 32, 32, 31, 32, 32, 32, 31, 32, 31, 32, 32, 32,
		32, 32, 32, 32, 32, 32, 32, 32, 32, 31, 32, 31, 32, 32, 33, 32, 32, 32,
		32, 32, 32, 32, 32, 33, 62, 63, 63, 62, 63, 64, 64, 64, 64, 65, 64, 63,
		64, 63, 64, 65, 64, 64, 63, 64, 64, 64, 63, 64, 65, 64, 64, 65, 65, 64,
		63, 64, 63, 63, 65, 64, 64, 65, 64, 64, 63, 63, 64, 63, 64, 64, 64, 65,
		64, 64, 64, 64, 63, 65, 65, 65, 64, 65, 64, 65, 64, 65, 64, 64, 64, 65,
		64, 65, 65, 63, 63, 65, 64, 65, 65, 65, 65, 65, 64, 65, 63, 65, 65, 64,
		65, 66, 64, 64, 64, 66, 65, 65, 65, 64, 65, 65, 63, 64, 65, 65, 65, 64,
		65, 65, 64, 63, 64, 65, 65, 65, 65, 65, 64, 65, 65, 65, 65, 66, 65, 64,
		65, 65, 66, 65, 64, 66, 66, 64, 66, 65, 66, 64, 66, 65, 65, 66, 64, 66,
		66, 66, 64, 66, 64, 65, 65, 65, 65, 66, 64, 65, 64, 65, 65, 64, 65, 64,
		65, 65, 65, 65, 64, 66, 65, 64, 65, 66, 65, 66, 65, 66, 65, 66, 64, 66,
		65, 66, 65, 65, 66, 64, 66, 65, 65, 66, 67, 65, 65, 65, 65, 65, 65, 65,
		66, 66, 65, 65, 65, 65, 65, 65, 65, 64, 65, 67, 65, 66, 66, 67, 65, 65,
		65, 65, 65, 67, 65, 66, 65, 66, 67, 65, 65, 65, 64, 65, 65, 66, 65, 66,
		66, 67, 67, 67, 66, 67, 66, 66, 66, 67, 66, 66, 66, 65, 66, 65, 65, 65,
		66, 65, 67, 65, 66, 66, 66, 65, 64, 66, 67, 66, 65, 66, 67, 64, 66, 66,
		66, 66, 65, 65, 65, 65, 66, 65, 67, 66, 66, 66, 66, 66, 67, 66, 65, 66,
		66, 66, 65, 67, 65, 66, 67, 66, 65, 66, 66, 66, 67, 65, 65, 66, 65, 66,
		66, 66, 66, 67, 66, 65, 67, 67, 66, 66, 66, 66, 66, 67, 67, 66, 66, 66,
		66, 67, 65, 66, 65, 66, 66, 67, 67, 67, 66, 66, 66, 66, 66, 66, 67, 66,
		65, 66, 65, 66, 65, 66, 68, 65, 67, 66, 66, 67, 67, 67, 67, 67, 66, 65,
		67, 65, 66, 67, 65, 65, 65, 67, 66, 68, 66, 66, 66, 67, 66, 66, 65, 65,
		66, 66, 66, 67, 66, 67, 65, 67, 68, 66, 68, 65, 66, 67, 66, 66, 66, 66,
		66, 67, 67, 66, 65, 66, 66, 65, 68, 67, 67, 66, 67, 66, 66, 67, 66, 66,
		66, 66, 67, 66, 67, 64, 66, 65, 66, 67, 67, 66, 66, 65, 66, 66, 66, 66,
		66, 68, 66, 68, 68, 67, 66, 65, 66, 65, 66, 67, 67, 67, 66, 66, 66, 68,
		66, 66, 66, 68, 68, 66, 67, 65, 67, 67, 67, 66, 65, 67, 66, 65, 66, 67,
		67, 66, 66, 67, 67, 67, 66, 67, 64, 65, 65, 67, 66, 66, 66, 65, 65, 66,
		66, 67, 67, 66, 66, 66, 68, 66, 67, 67, 65, 66, 67, 67, 66, 67, 65, 66,
		66, 66, 66, 68, 67, 66, 66, 66, 67, 67, 68, 67, 66, 67, 67, 66, 66, 66,
		67, 68, 65, 67, 66, 65, 65, 67, 67, 66, 66, 67, 66, 67, 65, 67, 67, 67,
		65, 67, 67, 65, 66, 66, 67, 65, 68, 67, 67, 67, 67, 65, 66, 67, 67, 66,
		67, 67, 66, 66, 65, 66, 67, 67, 68, 66, 65, 66, 68, 67, 66, 65, 66, 67,
		66, 67, 67, 66, 67, 66, 68, 67, 65, 68, 67, 66, 67, 67, 68, 66, 67, 67,
		68, 66, 67, 67, 66, 66, 67, 66, 67, 66, 67, 66, 67, 67, 67, 68, 66, 66,
		66, 66, 66, 67, 67, 67, 68, 67, 66, 66, 67, 66, 68, 67, 66, 66, 67, 67,
		68, 68, 66, 67, 66, 65, 65, 67, 65, 66, 66, 66, 66, 67, 66, 67, 66, 67,
		66, 68, 66, 65, 66, 67, 66, 67, 67, 66, 65, 66, 66, 67, 67, 67, 67, 68,
		66, 65, 67, 67, 67, 67, 67, 65, 66, 67, 68, 67, 67, 67, 66, 67, 66, 66,
		66, 66, 67, 67, 68, 67, 69, 67, 67, 67, 67, 67, 66, 67, 67, 68, 67, 66,
		66, 68, 67, 66, 68, 67, 67, 67, 66, 67, 67, 66, 67, 66, 67, 66, 66, 67,
		68, 66, 67, 67, 68, 68, 66, 66, 68, 67, 66, 67, 66, 66, 67, 66, 67, 67,
		67, 65, 66, 67, 67, 65, 67, 66, 69, 68, 67, 67, 67, 66, 67, 67, 68, 67,
		67, 68, 68, 65, 66, 66, 67, 66, 67, 66, 66, 67, 67, 66, 67, 66, 67, 67,
		67, 68, 66, 66, 67, 66, 67, 67, 68, 67, 65, 66, 68, 66, 67, 67, 68, 66,
		67, 66, 69, 65, 68, 69, 66, 66, 66, 67, 66, 67, 67, 66, 67, 67, 66, 66,
		68, 67, 68, 66, 67, 67, 66, 67, 67, 66, 66, 66, 67, 67, 66, 67, 68, 67,
		67, 68, 67, 66, 67, 67, 66, 68, 67, 67, 66, 66, 67, 66, 66, 68, 68, 68,
		67, 68, 68, 67, 66, 67, 67, 67, 67, 66, 69, 67, 67, 65, 68, 67, 67, 67,
		66, 68, 66, 67, 66, 66, 68, 68, 66, 68, 66, 69, 67, 65, 66, 67, 67, 67,
		67, 67, 67, 67, 66, 68, 67, 67, 66, 67, 67, 69, 67, 67, 67, 67, 67, 67,
		67, 68, 68, 68, 66, 68, 67, 67, 66, 67, 67, 67, 67, 67, 67, 66, 67, 67,
		66, 66, 67, 67, 67, 68, 66, 67, 66, 68, 67, 67, 67, 67, 66, 66, 67, 67,
		67, 67, 68, 67, 67, 67, 68, 67, 67, 67, 67, 67, 68, 67, 68, 67, 68, 67,
		68, 67, 67, 67, 67, 67, 67, 65, 67, 66, 67, 67, 68, 66, 67, 65, 67, 67,
		67, 66, 69, 68, 66, 67, 66, 66, 67, 68, 66, 67, 67, 67, 68, 67, 68, 68,
		66, 67, 66, 66, 67, 67, 66, 67, 68, 66, 67, 68, 67, 67, 66, 67, 67, 67,
		68, 68, 66, 66, 66, 68, 66, 68, 68, 66, 67, 65, 67, 67, 66, 68, 68, 66,
		67, 68, 66, 68, 66, 67, 68, 67, 67, 67, 66, 67, 67, 68, 67, 65, 67, 66,
		67, 68, 68, 68, 66, 67, 68, 68, 68, 67, 66, 66, 67, 67, 67, 67, 68, 67,
		67, 69, 67, 68, 67, 67, 66, 66, 67, 67, 66, 68, 68, 68, 65, 68, 65, 67,
		67, 67, 68, 67, 68, 66, 67, 67, 67, 68, 68, 67, 66, 67, 69, 66, 67, 68,
		67, 66, 67, 67, 68, 68, 68, 66, 66, 68, 68, 68, 68, 67, 67, 67, 66, 68,
		68, 67, 68, 66, 67, 67, 67, 67, 67, 67, 68, 68, 66, 66, 66, 67, 68, 68,
		68, 66, 66, 67, 67, 68, 68, 67, 66, 66, 67, 68, 68, 66, 67, 68, 67, 68,
		67, 67, 66, 66, 66, 67, 67, 67, 67, 67, 68, 67, 66, 67, 67, 67, 67, 67,
		67, 67, 66, 66, 68, 66, 67, 66, 67, 68, 68, 66, 68, 67, 67, 67, 66, 66,
		66, 67, 68, 66, 68, 67, 66, 66, 68, 67, 67, 67, 67, 67, 67, 68, 67, 67,
		68, 68, 67, 67, 67, 67, 68, 67, 68, 68, 67, 66, 67, 68, 67, 67, 67, 67,
		67, 67, 67, 66, 67, 67, 66, 68, 68, 67, 67, 67, 66, 67, 67, 66, 68, 68,
		67, 67, 67, 66, 67, 68, 66, 67, 68, 67, 67, 68, 66, 67, 68, 66, 67, 67,
		67, 68, 68, 66, 68, 67, 65, 66, 67, 66, 66, 67, 69, 68, 67, 66, 67, 68,
		68, 67, 67, 68, 68, 68, 67, 67, 66, 67, 68, 66, 67, 67, 68, 66, 67, 67,
		66, 67, 67, 68, 68, 68, 67, 68, 67, 68, 67, 68, 67, 66, 67, 67, 68, 67,
		66, 66, 68, 66, 68, 67, 66, 66, 68, 67, 68, 68, 66, 67, 68, 67, 68, 68,
		67, 67, 68, 68, 69, 67, 67, 68, 67, 67, 67, 68, 66, 68, 67, 69, 68, 66,
		67, 66, 66, 67, 68, 67, 67, 67, 66, 67, 68, 67, 67, 66, 68, 69, 67, 68,
		67, 67, 68, 66, 68, 67, 68, 67, 67, 68, 67, 66, 67, 67, 67, 66, 68, 67,
		67, 67, 67, 67, 67, 66, 68, 66, 67, 66, 67, 68, 68, 68, 66, 67, 68, 66,
		67, 67, 67, 66, 67, 68, 67, 68, 68, 68, 67, 67, 67, 67, 66, 69, 67, 67,
		67, 69, 66, 66, 65, 67, 68, 69, 66, 67, 69, 67, 68, 69, 67, 68, 67, 66,
		68, 67, 66, 67, 66, 68, 68, 67, 66, 67, 68, 68, 69, 67, 67, 68, 68, 66,
		66, 67, 68, 68, 68, 66, 67, 67, 67, 68, 67, 67, 68, 66, 67, 67, 68, 67,
		67, 68, 67, 67, 68, 69, 67, 68, 68, 68, 67, 67, 68, 67, 67, 65, 68, 67,
		66, 66, 67, 67, 68, 66, 68, 67, 68, 66, 67, 67, 68, 66, 67, 66, 67, 68,
		67, 67, 67, 67, 69, 66, 66, 67, 67, 69, 68, 67, 66, 68, 67, 67, 68, 67,
		67, 68, 67, 68, 68, 68, 68, 66, 66, 67, 68, 67, 68, 67, 66, 68, 67, 67,
		67, 67, 67, 68, 67, 67, 66, 68, 67, 68, 66, 67, 67, 67, 68, 68, 66, 66,
		67, 67, 67, 67, 67, 67, 68, 66, 67, 67, 67, 66, 68, 67, 67, 68, 66, 67,
		67, 67, 68, 67, 67, 67, 66, 68, 67, 67, 67, 67, 68, 66, 67, 67, 67, 66,
		68, 68, 68, 67, 66, 67, 67, 69, 66, 66, 67, 68, 67, 68, 68, 66, 68, 68,
		66, 67, 67, 67, 67, 69, 67, 68, 68, 66, 68, 68, 68, 67, 67, 67, 68, 68,
		66, 67, 68, 68, 68, 68, 67, 67, 68, 67, 69, 69, 67, 67, 67, 66, 67, 67,
		67, 68, 67, 67, 67, 68, 68, 67, 66, 68, 67, 68, 67, 69, 67, 68, 67, 67,
		67, 67, 67, 67, 67, 68, 67, 68, 68, 69, 67, 67, 66, 66, 67, 67, 67, 67,
		69, 69, 68, 66, 67, 67, 67, 68, 67, 67, 67, 68, 68, 67, 66, 67, 68, 67,
		68, 67, 67, 68, 68, 68, 66, 68, 68, 67, 67, 67, 67, 68, 67, 66, 67, 66,
		68, 68, 68, 67, 67, 68, 67, 67, 69, 67, 67, 67, 68, 67, 68, 69, 67, 68,
		68, 67, 68, 68, 68, 67, 67, 67, 68, 67, 67, 67, 66, 67, 68, 68, 67, 66,
		67, 68, 69, 68, 68, 69, 67, 68, 69, 69, 69, 67, 67, 67, 67, 68, 67, 69,
		67, 67, 67, 67, 68, 69, 68, 66, 66, 68, 67, 68, 68, 66, 68, 68, 68, 68,
		67, 67, 67, 68, 68, 67, 67, 66, 69, 69, 68, 69, 67, 67, 68, 67, 67, 66,
		67, 69, 67, 67, 67, 68, 67, 67, 67, 68, 66, 68, 67, 67, 67, 67, 67, 67,
		67, 68, 66, 67, 67, 68, 66, 68, 67, 67, 68};

	/* done */

	int sumdata80[] = {0, 5,   5,   5,   5,   5,   5,
		5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,  
		5,   5,   6,   6,   7,   8,   8,   9,   9,   6,   6,   7,   7,   7,   8,
		8,   9,   9,  10,  10,  10,  11,  11,  12,  12,  13,  13,  14,  14, 
		15,  15,  16,  16,  16,  17,  17,  18,  18,  19,  19,  20,  20,  21, 
		21,  22,  22,  23,  23,  24,  24,  24,  25,  25,  26,  26,  27,  27, 
		28,  28,  29,  29,  30,  30,  31,  31,  32,  32,  33,  33,  34,  34, 
		35,  35,  36,  36,  37,  37,  38,  38,  39,  39,  40,  40,  41,  41, 
		42,  42,  43,  43,  44,  44,  45,  45,  46,  46,  47,  47,  48,  48, 
		49,  49,  50,  50,  51,  51,  52,  52,  53,  53,  54,  54,  55,  55, 
		56,  56,  57,  57,  58,  58,  59,  59,  60,  60,  61,  61,  62,  63, 
		63,  64,  64,  65,  65,  66,  66,  67,  67,  68,  68,  69,  43,  43, 
		43,  44,  44,  44,  45,  45,  46,  46,  46,  47,  47,  47,  48,  48, 
		49,  49,  49,  50,  50,  50,  51,  51,  52,  52,  52,  53,  53,  53, 
		54,  54,  55,  55,  55,  56,  56,  56,  57,  57,  58,  58,  58,  59, 
		59,  59,  60,  60,  61,  61,  61,  62,  62,  63,  63,  63,  64,  64, 
		64,  65,  65,  66,  66,  66,  67,  67,  68,  68,  68,  69,  69,  69, 
		70,  70,  71,  71,  71,  72,  72,  73,  73,  73,  74,  74,  74,  75, 
		75,  76,  76,  76,  77,  77,  78,  78,  78,  79,  79,  80,  80,  80, 
		81,  81,  81,  82,  82,  83,  83,  83,  84,  84,  85,  85,  85,  86, 
		86,  87,  87,  87,  88,  88,  89,  89,  89,  90,  90,  90,  91,  91, 
		92,  92,  92,  93,  93,  94,  94,  94,  95,  95,  96,  96,  96,  97, 
		97,  98,  98,  98,  99,  99, 100, 100, 100, 101, 101, 102, 102, 102,
		103, 103, 103, 104, 104, 105, 105, 105, 106, 106, 107, 107, 107, 108,
		108, 109, 109, 109, 110, 110, 111, 111, 111, 112, 112, 113, 113, 113,
		114, 114, 115, 115, 115, 116, 116, 117, 117, 117, 118, 118, 119, 119,
		119, 120, 120, 121, 121, 121, 122, 122, 123, 123, 123, 124, 124, 125,
		125, 125, 126, 126, 127, 127, 127, 128, 128, 129, 129, 129, 130, 130,
		131, 131, 131, 132, 132, 133, 133, 134, 134, 134, 135, 135, 136, 136,
		136, 137, 137, 138, 138, 138, 139, 139, 140, 140, 140, 141, 141, 142,
		142, 142, 143, 143, 144, 144, 144, 145, 145, 146, 146, 146, 147, 147,
		148, 148, 148, 149, 149, 150, 150, 151, 151, 151, 152, 152, 153, 153,
		153, 154, 154, 155, 155, 155, 156, 156, 157, 157, 157, 158, 158, 159,
		159, 159, 160, 160, 161, 161, 161, 162, 162, 163, 163, 164, 164, 164,
		165, 165, 166, 166, 166, 167, 167, 168, 168, 168, 169, 169, 170, 170,
		170, 171, 171, 172, 172, 173, 173, 173, 174, 174, 175, 175, 175, 176,
		176, 177, 177, 177, 178, 178, 179, 179, 179, 180, 180, 181, 181, 182,
		182, 182, 183, 183, 184, 184, 184, 185, 185, 186, 186, 186, 187, 187,
		188, 188, 189, 189, 189, 190, 190, 191, 191, 191, 192, 192, 193, 193,
		193, 194, 194, 195, 195, 196, 196, 196, 197, 197, 198, 198, 198, 199,
		199, 200, 200, 200, 201, 201, 202, 202, 203, 203, 203, 204, 204, 205,
		205, 205, 206, 206, 207, 207, 207, 208, 208, 209, 209, 210, 210, 210,
		211, 211, 212, 212, 212, 213, 213, 214, 214, 215, 215, 215, 216, 216,
		217, 217, 217, 218, 218, 219, 219, 219, 220, 220, 221, 221, 222, 222,
		222, 223, 223, 224, 224, 224, 225, 225, 226, 226, 227, 227, 227, 228,
		228, 229, 229, 229, 230, 230, 231, 231, 232, 232, 232, 233, 233, 234,
		234, 234, 235, 235, 236, 236, 237, 237, 237, 238, 238, 239, 239, 239,
		240, 240, 241, 241, 242, 242, 242, 243, 243, 244, 244, 244, 245, 245,
		246, 246, 246, 247, 247, 248, 248, 249, 249, 249, 250, 250, 251, 251,
		252, 252, 252, 253, 253, 254, 254, 254, 255, 255, 256, 256, 257, 257,
		257, 258, 258, 259, 259, 259, 260, 260, 261, 261, 262, 262, 262, 263,
		263, 264, 264, 264, 265, 265, 266, 266, 267, 267, 267, 268, 268, 269,
		269, 269, 270, 270, 271, 271, 272, 272, 272, 273, 273, 274, 274, 274,
		275, 275, 276, 276, 277, 277, 277, 278, 278, 279, 279, 280, 280, 280,
		281, 281, 282, 282, 282, 283, 283, 284, 284, 285, 285, 285, 286, 286,
		287, 287, 287, 288, 288, 289, 289, 290, 290, 290, 291, 291, 292, 292,
		293, 293, 293, 294, 294, 295, 295, 295, 296, 296, 297, 297, 298, 298,
		298, 299, 299, 300, 300, 301, 301, 301, 302, 302, 303, 303, 303, 304,
		304, 305, 305, 306, 306, 306, 307, 307, 308, 308, 309, 309, 309, 310,
		310, 311, 311, 311, 312, 312, 313, 313, 314, 314, 314, 315, 315, 316,
		316, 317, 317, 317, 318, 318, 319, 319, 319, 320, 320, 321, 321, 322,
		322, 322, 323, 323, 324, 324, 325, 325, 325, 326, 326, 327, 327, 327,
		328, 328, 329, 329, 330, 330, 330, 331, 331, 332, 332, 333, 333, 333,
		334, 334, 335, 335, 336, 336, 336, 337, 337, 338, 338, 338, 339, 339,
		340, 340, 341, 341, 341, 342, 342, 343, 343, 344, 344, 344, 345, 345,
		346, 346, 347, 347, 347, 348, 348, 349, 349, 349, 350, 350, 351, 351,
		352, 352, 352, 353, 353, 354, 354, 355, 355, 355, 356, 356, 357, 357,
		358, 358, 358, 359, 359, 360, 360, 360, 361, 361, 362, 362, 363, 363,
		363, 364, 364, 365, 365, 366, 366, 366, 367, 367, 368, 368, 369, 369,
		369, 370, 370, 371, 371, 372, 372, 372, 373, 373, 374, 374, 374, 375,
		375, 376, 376, 377, 377, 377, 378, 378, 379, 379, 380, 380, 380, 381,
		381, 382, 382, 383, 383, 383, 384, 384, 385, 385, 386, 386, 386, 387,
		387, 388, 388, 388, 389, 389, 390, 390, 391, 391, 391, 392, 392, 393,
		393, 394, 394, 394, 395, 395, 396, 396, 397, 397, 397, 398, 398, 399,
		399, 400, 400, 400, 401, 401, 402, 402, 403, 403, 403, 404, 404, 405,
		405, 406, 406, 406, 407, 407, 408, 408, 408, 409, 409, 410, 410, 411,
		411, 411, 412, 412, 413, 413, 414, 414, 414, 415, 415, 416, 416, 417,
		417, 417, 418, 418, 419, 419, 420, 420, 420, 421, 421, 422, 422, 423,
		423, 423, 424, 424, 425, 425, 426, 426, 426, 427, 427, 428, 428, 429,
		429, 429, 430, 430, 431, 431, 432, 432, 432, 433, 433, 434, 434, 434,
		435, 435, 436, 436, 437, 437, 437, 438, 438, 439, 439, 440, 440, 440,
		441, 441, 442, 442, 443, 443, 443, 444, 444, 445, 445, 446, 446, 446,
		447, 447, 448, 448, 449, 449, 449, 450, 450, 451, 451, 452, 452, 452,
		453, 453, 454, 454, 455, 455, 455, 456, 456, 457, 457, 458, 458, 458,
		459, 459, 460, 460, 461, 461, 461, 462, 462, 463, 463, 464, 464, 464,
		465, 465, 466, 466, 467, 467, 467, 468, 468, 469, 469, 470, 470, 470,
		471, 471, 472, 472, 473, 473, 473, 474, 474, 475, 475, 476, 476, 476,
		477, 477, 478, 478, 479, 479, 479, 480, 480, 481, 481, 482, 482, 482,
		483, 483, 484, 484, 485, 485, 485, 486, 486, 487, 487, 488, 488, 488,
		489, 489, 490, 490, 491, 491, 491, 492, 492, 493, 493, 494, 494, 494,
		495, 495, 496, 496, 497, 497, 497, 498, 498, 499, 499, 500, 500, 500,
		501, 501, 502, 502, 503, 503, 503, 504, 504, 505, 505, 506, 506, 506,
		507, 507, 508, 508, 509, 509, 509, 510, 510, 511, 511, 512, 512, 512,
		513, 513, 514, 514, 515, 515, 515, 516, 516, 517, 517, 518, 518, 518,
		519, 519, 520, 520, 521, 521, 521, 522, 522, 523, 523, 524, 524, 524,
		525, 525, 526, 526, 527, 527, 527, 528, 528, 529, 529, 530, 530, 530,
		531, 531, 532, 532, 533, 533, 533, 534, 534, 535, 535, 536, 536, 536,
		537, 537, 538, 538, 539, 539, 539, 540, 540, 541, 541, 542, 542, 543,
		543, 543, 544, 544, 545, 545, 546, 546, 546, 547, 547, 548, 548, 549,
		549, 549, 550, 550, 551, 551, 552, 552, 552, 553, 553, 554, 554, 555,
		555, 555, 556, 556, 557, 557, 558, 558, 558, 559, 559, 560, 560, 561,
		561, 561, 562, 562, 563, 563, 564, 564, 564, 565, 565, 566, 566, 567,
		567, 567, 568, 568, 569, 569, 570, 570, 570, 571, 571, 572, 572, 573,
		573, 574, 574, 574, 575, 575, 576, 576, 577, 577, 577, 578, 578, 579,
		579, 580, 580, 580, 581, 581, 582, 582, 583, 583, 583, 584, 584, 585,
		585, 586, 586, 586, 587, 587, 588, 588, 589, 589, 589, 590, 590, 591,
		591, 592, 592, 592, 593, 593, 594, 594, 595, 595, 595, 596, 596, 597,
		597, 598, 598, 599, 599, 599, 600, 600, 601, 601, 602, 602, 602, 603,
		603, 604, 604, 605, 605, 605, 606, 606, 607, 607, 608, 608, 608, 609,
		609, 610, 610, 611, 611, 611, 612, 612, 613, 613, 614, 614, 614, 615,
		615, 616, 616, 617, 617, 618, 618, 618, 619, 619, 620, 620, 621, 621,
		621, 622, 622, 623, 623, 624, 624, 624, 625, 625, 626, 626, 627, 627,
		627, 628, 628, 629, 629, 630, 630, 630, 631, 631, 632, 632, 633, 633,
		634, 634, 634, 635, 635, 636, 636, 637, 637, 637, 638, 638, 639, 639,
		640, 640, 640, 641, 641, 642, 642, 643, 643, 643, 644, 644, 645, 645,
		646, 646, 646, 647, 647, 648, 648, 649, 649, 650, 650, 650, 651, 651,
		652, 652, 653, 653, 653, 654, 654, 655, 655, 656, 656, 656, 657, 657,
		658, 658, 659, 659, 659, 660, 660, 661, 661, 662, 662, 662, 663, 663,
		664, 664, 665, 665, 666, 666, 666, 667, 667, 668, 668, 669, 669, 669,
		670, 670, 671, 671, 672, 672, 672, 673, 673, 674, 674, 675, 675, 675,
		676, 676, 677, 677, 678, 678, 679, 679, 679, 680, 680, 681, 681, 682,
		682, 682, 683, 683, 684, 684, 685, 685, 685, 686, 686, 687, 687, 688,
		688, 688, 689, 689, 690, 690, 691, 691, 692, 692, 692, 693, 693, 694,
		694, 695, 695, 695, 696, 696, 697, 697, 698, 698, 698, 699, 699, 700,
		700, 701, 701, 701, 702, 702, 703, 703, 704, 704, 705, 705, 705, 706,
		706, 707, 707, 708, 708, 708, 709, 709, 710, 710, 711, 711, 711, 712,
		712, 713, 713, 714, 714, 715, 715, 715, 716, 716, 717, 717, 718, 718,
		718, 719, 719, 720, 720, 721, 721, 721, 722, 722, 723, 723, 724, 724,
		724, 725, 725, 726, 726, 727, 727, 728, 728, 728, 729, 729, 730, 730,
		731, 731, 731, 732, 732, 733, 733, 734, 734, 734, 735, 735, 736, 736,
		737, 737, 738, 738, 738, 739, 739, 740, 740, 741, 741, 741, 742, 742,
		743, 743, 744, 744, 744, 745, 745, 746, 746, 747, 747, 748, 748, 748,
		749, 749, 750, 750, 751, 751, 751, 752, 752, 753, 753, 754, 754, 754,
		755, 755, 756, 756, 757, 757, 758, 758, 758, 759, 759, 760, 760, 761,
		761, 761, 762, 762, 763, 763, 764, 764, 764, 765, 765, 766, 766, 767,
		767, 768, 768, 768, 769, 769, 770, 770, 771, 771, 771, 772, 772, 773,
		773, 774, 774, 774, 775, 775, 776, 776, 777, 777, 778, 778, 778, 779,
		779, 780, 780, 781, 781, 781, 782, 782, 783, 783, 784, 784, 784, 785,
		785, 786, 786, 787, 787, 788, 788, 788, 789, 789, 790, 790, 791, 791,
		791, 792, 792, 793, 793, 794, 794, 794, 795, 795, 796, 796, 797, 797,
		798, 798, 798, 799, 799, 800, 800, 801, 801, 801, 802, 802, 803, 803,
		804, 804, 804, 805, 805, 806, 806, 807, 807, 808, 808, 808, 809, 809,
		810, 810, 811, 811, 811, 812, 812, 813, 813, 814, 814, 815, 815, 815,
		816, 816, 817, 817, 818};


	/* generated with the following parameters :
	   0.750000, 3 1, 4 30, 5 44, 7 160 */
	/* done */
	int waitdata75[] = {0, 15, 15, 15, 15, 15, 15, 15, 15, 15,
		15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
		15, 15, 25, 25, 25, 25, 25, 25, 26, 25, 26, 25, 26, 26, 26, 26, 37, 38,
		39, 38, 38, 39, 39, 39, 40, 40, 40, 40, 40, 41, 41, 40, 41, 41, 40, 41,
		41, 41, 41, 41, 41, 41, 41, 41, 42, 41, 41, 41, 41, 41, 41, 42, 41, 42,
		41, 42, 41, 42, 41, 41, 42, 42, 42, 42, 42, 41, 42, 41, 42, 43, 42, 41,
		42, 42, 42, 42, 43, 42, 42, 42, 42, 43, 43, 43, 43, 43, 43, 43, 43, 42,
		43, 42, 43, 42, 43, 43, 43, 43, 43, 42, 43, 43, 43, 43, 43, 43, 43, 43,
		43, 43, 43, 43, 43, 44, 43, 44, 43, 43, 43, 43, 43, 42, 44, 43, 43, 44,
		43, 43, 43, 43, 44, 44, 93, 91, 91, 93, 92, 93, 92, 93, 92, 92, 93, 93,
		93, 94, 93, 95, 92, 92, 92, 94, 92, 93, 93, 93, 92, 94, 94, 93, 93, 93,
		92, 92, 93, 94, 95, 95, 94, 94, 93, 94, 93, 94, 93, 94, 95, 95, 95, 94,
		93, 95, 94, 94, 94, 94, 94, 94, 94, 95, 95, 93, 95, 94, 93, 93, 94, 93,
		94, 93, 96, 95, 96, 96, 94, 94, 94, 95, 96, 95, 96, 94, 95, 94, 96, 94,
		96, 96, 94, 97, 94, 96, 96, 96, 96, 96, 95, 95, 96, 95, 95, 96, 95, 95,
		95, 95, 95, 94, 95, 97, 96, 94, 95, 95, 94, 97, 98, 96, 94, 97, 96, 95,
		95, 94, 97, 96, 94, 95, 97, 95, 95, 94, 98, 96, 95, 95, 96, 96, 95, 98,
		95, 94, 94, 97, 95, 96, 98, 98, 95, 97, 97, 97, 95, 95, 97, 96, 98, 97,
		97, 97, 96, 95, 97, 97, 97, 96, 97, 95, 97, 97, 98, 96, 98, 96, 96, 97,
		95, 96, 95, 98, 98, 96, 98, 96, 97, 96, 96, 95, 95, 97, 97, 96, 97, 97,
		97, 96, 95, 98, 98, 97, 97, 97, 95, 97, 97, 99, 98, 96, 98, 96, 96, 96,
		97, 96, 96, 98, 97, 97, 95, 97, 98, 96, 97, 96, 97, 99, 97, 99, 98, 98,
		98, 98, 98, 96, 98, 98, 97, 98, 99, 98, 97, 97, 97, 98, 98, 97, 97, 97,
		98, 98, 97, 98, 98, 98, 98, 97, 98, 99, 99, 99, 99, 98, 96, 97, 96, 97,
		98, 96, 96, 98, 97, 99, 97, 97, 97, 98, 97, 98, 97, 99, 97, 98, 99, 98,
		96, 97, 96, 99, 99, 96, 96, 96, 95, 98, 99, 98, 99, 96, 97, 98, 96, 98,
		97, 97, 97, 99, 98, 98, 99, 98, 98, 97, 98, 98, 96, 98, 98, 95, 98, 99,
		99, 98, 100, 98, 99, 97, 97, 97, 97, 97, 97, 98, 98, 98, 98, 98, 99, 99,
		99, 99, 97, 97, 97, 98, 98, 99, 97, 97, 98, 97, 99, 99, 98, 96, 99, 97,
		98, 98, 97, 96, 97, 96, 97, 98, 97, 97, 98, 98, 99, 100, 98, 98, 97, 98,
		97, 97, 100, 97, 98, 99, 97, 99, 100, 98, 98, 97, 98, 99, 98, 96, 100,
		96, 98, 98, 99, 96, 98, 99, 98, 96, 97, 99, 97, 97, 99, 98, 98, 98, 97,
		98, 98, 99, 100, 96, 99, 95, 98, 97, 97, 99, 96, 100, 99, 97, 100, 99,
		98, 98, 99, 100, 96, 98, 101, 97, 99, 98, 99, 98, 100, 97, 99, 98, 98,
		98, 98, 98, 98, 97, 98, 98, 99, 98, 99, 98, 98, 98, 98, 98, 98, 99, 97,
		96, 98, 98, 99, 98, 98, 100, 100, 99, 97, 98, 99, 99, 98, 99, 98, 99,
		97, 98, 97, 98, 98, 99, 99, 100, 97, 97, 98, 98, 98, 99, 98, 99, 99, 98,
		98, 97, 97, 99, 98, 98, 98, 98, 98, 98, 99, 99, 100, 97, 100, 98, 99,
		97, 99, 97, 98, 98, 99, 99, 98, 98, 97, 98, 98, 99, 98, 96, 96, 99, 99,
		97, 98, 99, 98, 98, 96, 98, 98, 98, 99, 100, 98, 97, 99, 98, 99, 99, 99,
		97, 97, 99, 99, 98, 99, 101, 97, 100, 99, 98, 100, 99, 101, 99, 96, 98,
		100, 98, 98, 98, 98, 100, 98, 98, 99, 97, 98, 98, 99, 99, 98, 99, 99,
		101, 99, 99, 99, 98, 101, 97, 99, 97, 100, 99, 100, 96, 100, 99, 98, 97,
		97, 101, 99, 99, 99, 99, 98, 98, 96, 99, 96, 98, 99, 102, 99, 98, 100,
		97, 99, 100, 100, 98, 98, 98, 100, 100, 100, 100, 99, 100, 100, 100, 99,
		99, 98, 98, 98, 99, 99, 100, 98, 98, 98, 98, 97, 101, 97, 99, 99, 100,
		99, 98, 98, 98, 98, 98, 98, 99, 100, 101, 98, 99, 101, 99, 98, 100, 99,
		99, 101, 99, 100, 101, 100, 99, 100, 98, 100, 99, 100, 99, 100, 97, 100,
		99, 99, 99, 101, 100, 100, 98, 100, 101, 99, 99, 101, 99, 98, 98, 101,
		100, 98, 100, 99, 100, 98, 98, 99, 99, 99, 100, 100, 99, 100, 100, 96,
		98, 99, 99, 99, 100, 99, 99, 99, 98, 99, 101, 100, 99, 99, 98, 97, 99,
		99, 100, 100, 99, 100, 100, 98, 98, 99, 97, 101, 100, 100, 101, 99, 99,
		99, 100, 100, 99, 99, 100, 100, 99, 97, 100, 100, 97, 99, 97, 99, 99,
		98, 99, 99, 100, 100, 99, 98, 100, 99, 99, 98, 98, 100, 99, 100, 99,
		100, 100, 100, 100, 101, 100, 101, 99, 99, 99, 101, 98, 97, 99, 99, 100,
		98, 100, 101, 97, 99, 98, 98, 100, 100, 99, 100, 98, 101, 100, 99, 100,
		98, 99, 98, 98, 99, 100, 98, 100, 99, 99, 101, 100, 98, 101, 99, 99,
		101, 98, 99, 102, 100, 100, 97, 99, 99, 100, 98, 100, 101, 100, 99, 98,
		99, 100, 99, 98, 101, 100, 99, 98, 100, 100, 98, 98, 100, 99, 100, 99,
		100, 100, 99, 99, 99, 100, 100, 101, 98, 101, 98, 99, 100, 99, 101, 99,
		99, 97, 99, 100, 98, 101, 100, 100, 100, 100, 99, 99, 101, 100, 100, 99,
		101, 98, 100, 98, 101, 101, 99, 100, 100, 99, 99, 100, 100, 99, 100, 98,
		100, 98, 99, 99, 98, 99, 97, 100, 100, 100, 100, 99, 100, 100, 100, 102,
		99, 99, 100, 99, 100, 98, 99, 101, 98, 99, 99, 100, 99, 99, 101, 99,
		100, 100, 100, 100, 100, 99, 99, 99, 101, 98, 100, 99, 100, 99, 100, 99,
		99, 99, 101, 101, 100, 100, 99, 98, 98, 99, 99, 99, 101, 100, 100, 98,
		99, 100, 99, 99, 99, 99, 100, 100, 98, 99, 99, 98, 99, 100, 101, 100,
		99, 101, 99, 101, 100, 100, 99, 100, 101, 100, 100, 99, 100, 98, 100,
		100, 100, 98, 99, 99, 100, 101, 99, 101, 100, 99, 101, 101, 100, 99,
		100, 100, 102, 99, 100, 100, 100, 101, 100, 100, 97, 100, 100, 99, 100,
		100, 100, 101, 101, 99, 99, 100, 99, 100, 101, 101, 99, 100, 101, 100,
		100, 100, 100, 99, 97, 101, 99, 100, 100, 98, 99, 102, 100, 102, 102,
		100, 99, 99, 99, 100, 99, 99, 103, 99, 99, 99, 99, 100, 100, 100, 100,
		99, 100, 101, 99, 100, 102, 101, 102, 98, 97, 100, 102, 100, 100, 101,
		101, 100, 100, 99, 102, 100, 101, 98, 100, 100, 101, 100, 100, 100, 99,
		98, 100, 101, 101, 100, 102, 99, 99, 98, 97, 101, 99, 99, 101, 100, 100,
		101, 101, 99, 99, 101, 99, 101, 100, 99, 100, 100, 99, 101, 100, 99,
		100, 101, 101, 101, 100, 100, 99, 101, 99, 101, 100, 100, 100, 99, 98,
		98, 101, 101, 98, 98, 100, 100, 100, 103, 101, 102, 100, 101, 101, 99,
		101, 99, 100, 99, 99, 100, 101, 100, 100, 101, 101, 101, 98, 100, 100,
		100, 101, 102, 102, 99, 100, 99, 99, 101, 98, 100, 100, 98, 98, 102,
		100, 99, 102, 100, 100, 103, 100, 99, 100, 100, 100, 101, 99, 98, 98,
		100, 100, 98, 101, 98, 99, 100, 99, 98, 99, 100, 101, 99, 102, 99, 101,
		100, 100, 100, 100, 99, 100, 100, 101, 100, 98, 100, 100, 100, 99, 101,
		101, 98, 101, 100, 99, 100, 100, 99, 98, 101, 100, 99, 101, 99, 102,
		101, 98, 99, 99, 96, 98, 99, 100, 101, 99, 100, 99, 101, 101, 99, 99,
		101, 102, 101, 100, 100, 100, 99, 98, 99, 100, 99, 100, 100, 101, 100,
		100, 99, 98, 100, 101, 100, 100, 101, 100, 100, 100, 102, 101, 99, 99,
		98, 99, 100, 101, 99, 101, 100, 101, 103, 99, 100, 99, 98, 101, 100,
		101, 100, 100, 98, 98, 101, 100, 99, 99, 99, 102, 100, 100, 99, 100, 99,
		100, 100, 99, 101, 99, 101, 101, 100, 99, 99, 98, 100, 103, 100, 100,
		98, 99, 100, 100, 100, 99, 101, 101, 101, 100, 102, 99, 101, 98, 99,
		101, 101, 101, 100, 99, 101, 99, 99, 97, 98, 99, 99, 99, 100, 100, 100,
		99, 100, 99, 100, 101, 99, 100, 97, 98, 103, 100, 99, 99, 100, 101, 99,
		100, 99, 99, 99, 98, 99, 99, 101, 99, 101, 100, 99, 99, 98, 99, 102,
		100, 101, 100, 100, 99, 99, 99, 100, 102, 100, 101, 99, 101, 100, 100,
		99, 99, 102, 99, 101, 101, 100, 101, 101, 102, 100, 99, 99, 100, 99,
		102, 100, 100, 100, 100, 102, 99, 100, 97, 100, 100, 100, 101, 99, 100,
		100, 100, 100, 100, 100, 101, 101, 100, 102, 99, 100, 101, 101, 102,
		101, 101, 99, 101, 100, 101, 100, 100, 99, 99, 99, 101, 98, 100, 99,
		100, 101, 101, 99, 101, 100, 101, 100, 101, 100, 99, 100, 102, 101, 99,
		99, 100, 100, 101, 100, 101, 100, 101, 99, 100, 100, 101, 101, 101, 101,
		99, 100, 100, 101, 100, 100, 100, 99, 100, 100, 101, 100, 101, 100, 100,
		101, 100, 100, 101, 100, 101, 100, 102, 100, 99, 101, 103, 100, 99, 100,
		100, 101, 100, 103, 98, 99, 97, 100, 99, 101, 100, 100, 101, 100, 99,
		101, 101, 100, 101, 100, 100, 102, 98, 99, 99, 98, 101, 100, 99, 99, 98,
		101, 99, 99, 100, 100, 98, 101, 101, 99, 98, 101, 100, 99, 100, 100, 98,
		99, 101, 100, 101, 101, 99, 100, 99, 102, 100, 99, 99, 101, 102, 98,
		101, 100, 101, 98, 99, 99, 100, 101, 100, 101, 101, 100, 101, 102, 102,
		100, 100, 99, 103, 100, 101, 100, 101, 98, 100, 100, 101, 99, 99, 99,
		100, 101, 99, 99, 100, 100, 100, 100, 100, 100, 99, 101, 100, 101, 100,
		99, 100, 101, 99, 98, 99, 101, 100, 101, 101, 100, 99, 100, 102, 100,
		100, 102, 100, 101, 101, 99, 100, 101, 98, 100, 100, 101, 101, 98, 101,
		102, 99, 100, 99, 101, 101, 100, 101, 99, 102, 99, 100, 101, 100, 100,
		100, 100, 101, 100, 102, 101, 101, 101, 100, 102, 99, 100, 100, 100,
		100, 101, 99, 99, 100, 101, 101, 101, 100, 101, 99, 100, 101, 99, 100,
		100, 100, 101, 100, 100, 100, 97, 101, 99, 100, 101, 101, 103, 101, 101,
		99, 100, 100, 102, 102, 99, 100, 101, 99, 100, 102, 102, 99, 99, 101,
		100, 101, 101, 100, 99, 99, 101, 99, 100, 100, 101, 101, 101, 101, 99,
		99, 101, 102, 99, 99, 101, 102, 100, 100, 99, 100, 100, 99, 100, 99,
		100, 99, 100, 100, 101, 100, 100, 100, 99, 101, 99, 101, 100, 100, 100,
		100, 100, 99, 99, 101, 100, 99, 101, 98, 101, 101, 101, 101, 102, 100,
		100, 100, 101, 100, 100, 99, 100, 98, 100, 99, 100, 101, 100, 101, 103,
		99, 100, 101, 100, 101, 101, 100};
	/* done */
	int sumdata75[] = {0, 5,   5,   5,   5,   5,  
		5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
		6,   6,   7,   7,   8,   8,   9,   9,  10,   6,   6,   7,   7,   8,  
		8,   8,   9,   9,  10,  10,  11,  11,  11,   7,   7,   7,   8,   8,   8,
		9,   9,   9,  10,  10,  10,  11,  11,  11,  12,  12,  12,  13,  13, 
		13,  14,  14,  14,  15,  15,  16,  16,  16,  17,  17,  17,  18,  18, 
		18,  19,  19,  20,  20,  20,  21,  21,  21,  22,  22,  23,  23,  23, 
		24,  24,  24,  25,  25,  26,  26,  26,  27,  27,  27,  28,  28,  29, 
		29,  29,  30,  30,  30,  31,  31,  32,  32,  32,  33,  33,  34,  34, 
		34,  35,  35,  35,  36,  36,  37,  37,  37,  38,  38,  39,  39,  39, 
		40,  40,  41,  41,  41,  42,  42,  42,  43,  43,  44,  44,  44,  45, 
		45,  46,  46,  46,  47,  47,  48,  48,  48,  49,  49,  50,  24,  24, 
		24,  24,  25,  25,  25,  25,  26,  26,  26,  26,  27,  27,  27,  27, 
		28,  28,  28,  28,  29,  29,  29,  29,  30,  30,  30,  30,  31,  31, 
		31,  31,  32,  32,  32,  32,  33,  33,  33,  34,  34,  34,  34,  35, 
		35,  35,  35,  36,  36,  36,  36,  37,  37,  37,  37,  38,  38,  38, 
		38,  39,  39,  39,  39,  40,  40,  40,  40,  41,  41,  41,  41,  42, 
		42,  42,  43,  43,  43,  43,  44,  44,  44,  44,  45,  45,  45,  45, 
		46,  46,  46,  46,  47,  47,  47,  48,  48,  48,  48,  49,  49,  49, 
		49,  50,  50,  50,  50,  51,  51,  51,  51,  52,  52,  52,  53,  53, 
		53,  53,  54,  54,  54,  54,  55,  55,  55,  55,  56,  56,  56,  57, 
		57,  57,  57,  58,  58,  58,  58,  59,  59,  59,  60,  60,  60,  60, 
		61,  61,  61,  61,  62,  62,  62,  62,  63,  63,  63,  64,  64,  64, 
		64,  65,  65,  65,  65,  66,  66,  66,  67,  67,  67,  67,  68,  68, 
		68,  68,  69,  69,  69,  70,  70,  70,  70,  71,  71,  71,  71,  72, 
		72,  72,  73,  73,  73,  73,  74,  74,  74,  74,  75,  75,  75,  76, 
		76,  76,  76,  77,  77,  77,  78,  78,  78,  78,  79,  79,  79,  79, 
		80,  80,  80,  81,  81,  81,  81,  82,  82,  82,  82,  83,  83,  83, 
		84,  84,  84,  84,  85,  85,  85,  86,  86,  86,  86,  87,  87,  87, 
		87,  88,  88,  88,  89,  89,  89,  89,  90,  90,  90,  91,  91,  91, 
		91,  92,  92,  92,  92,  93,  93,  93,  94,  94,  94,  94,  95,  95, 
		95,  96,  96,  96,  96,  97,  97,  97,  98,  98,  98,  98,  99,  99, 
		99,  99, 100, 100, 100, 101, 101, 101, 101, 102, 102, 102, 103, 103,
		103, 103, 104, 104, 104, 105, 105, 105, 105, 106, 106, 106, 107, 107,
		107, 107, 108, 108, 108, 109, 109, 109, 109, 110, 110, 110, 110, 111,
		111, 111, 112, 112, 112, 112, 113, 113, 113, 114, 114, 114, 114, 115,
		115, 115, 116, 116, 116, 116, 117, 117, 117, 118, 118, 118, 118, 119,
		119, 119, 120, 120, 120, 120, 121, 121, 121, 122, 122, 122, 122, 123,
		123, 123, 124, 124, 124, 124, 125, 125, 125, 126, 126, 126, 126, 127,
		127, 127, 128, 128, 128, 128, 129, 129, 129, 130, 130, 130, 130, 131,
		131, 131, 132, 132, 132, 132, 133, 133, 133, 134, 134, 134, 134, 135,
		135, 135, 136, 136, 136, 136, 137, 137, 137, 138, 138, 138, 138, 139,
		139, 139, 140, 140, 140, 140, 141, 141, 141, 142, 142, 142, 143, 143,
		143, 143, 144, 144, 144, 145, 145, 145, 145, 146, 146, 146, 147, 147,
		147, 147, 148, 148, 148, 149, 149, 149, 149, 150, 150, 150, 151, 151,
		151, 151, 152, 152, 152, 153, 153, 153, 153, 154, 154, 154, 155, 155,
		155, 156, 156, 156, 156, 157, 157, 157, 158, 158, 158, 158, 159, 159,
		159, 160, 160, 160, 160, 161, 161, 161, 162, 162, 162, 162, 163, 163,
		163, 164, 164, 164, 165, 165, 165, 165, 166, 166, 166, 167, 167, 167,
		167, 168, 168, 168, 169, 169, 169, 169, 170, 170, 170, 171, 171, 171,
		172, 172, 172, 172, 173, 173, 173, 174, 174, 174, 174, 175, 175, 175,
		176, 176, 176, 176, 177, 177, 177, 178, 178, 178, 179, 179, 179, 179,
		180, 180, 180, 181, 181, 181, 181, 182, 182, 182, 183, 183, 183, 183,
		184, 184, 184, 185, 185, 185, 186, 186, 186, 186, 187, 187, 187, 188,
		188, 188, 188, 189, 189, 189, 190, 190, 190, 191, 191, 191, 191, 192,
		192, 192, 193, 193, 193, 193, 194, 194, 194, 195, 195, 195, 196, 196,
		196, 196, 197, 197, 197, 198, 198, 198, 198, 199, 199, 199, 200, 200,
		200, 201, 201, 201, 201, 202, 202, 202, 203, 203, 203, 203, 204, 204,
		204, 205, 205, 205, 206, 206, 206, 206, 207, 207, 207, 208, 208, 208,
		208, 209, 209, 209, 210, 210, 210, 211, 211, 211, 211, 212, 212, 212,
		213, 213, 213, 213, 214, 214, 214, 215, 215, 215, 216, 216, 216, 216,
		217, 217, 217, 218, 218, 218, 219, 219, 219, 219, 220, 220, 220, 221,
		221, 221, 221, 222, 222, 222, 223, 223, 223, 224, 224, 224, 224, 225,
		225, 225, 226, 226, 226, 227, 227, 227, 227, 228, 228, 228, 229, 229,
		229, 229, 230, 230, 230, 231, 231, 231, 232, 232, 232, 232, 233, 233,
		233, 234, 234, 234, 235, 235, 235, 235, 236, 236, 236, 237, 237, 237,
		237, 238, 238, 238, 239, 239, 239, 240, 240, 240, 240, 241, 241, 241,
		242, 242, 242, 243, 243, 243, 243, 244, 244, 244, 245, 245, 245, 246,
		246, 246, 246, 247, 247, 247, 248, 248, 248, 248, 249, 249, 249, 250,
		250, 250, 251, 251, 251, 251, 252, 252, 252, 253, 253, 253, 254, 254,
		254, 254, 255, 255, 255, 256, 256, 256, 257, 257, 257, 257, 258, 258,
		258, 259, 259, 259, 260, 260, 260, 260, 261, 261, 261, 262, 262, 262,
		263, 263, 263, 263, 264, 264, 264, 265, 265, 265, 265, 266, 266, 266,
		267, 267, 267, 268, 268, 268, 268, 269, 269, 269, 270, 270, 270, 271,
		271, 271, 271, 272, 272, 272, 273, 273, 273, 274, 274, 274, 274, 275,
		275, 275, 276, 276, 276, 277, 277, 277, 277, 278, 278, 278, 279, 279,
		279, 280, 280, 280, 280, 281, 281, 281, 282, 282, 282, 283, 283, 283,
		283, 284, 284, 284, 285, 285, 285, 286, 286, 286, 286, 287, 287, 287,
		288, 288, 288, 289, 289, 289, 289, 290, 290, 290, 291, 291, 291, 292,
		292, 292, 292, 293, 293, 293, 294, 294, 294, 295, 295, 295, 295, 296,
		296, 296, 297, 297, 297, 298, 298, 298, 298, 299, 299, 299, 300, 300,
		300, 301, 301, 301, 301, 302, 302, 302, 303, 303, 303, 304, 304, 304,
		304, 305, 305, 305, 306, 306, 306, 307, 307, 307, 307, 308, 308, 308,
		309, 309, 309, 310, 310, 310, 311, 311, 311, 311, 312, 312, 312, 313,
		313, 313, 314, 314, 314, 314, 315, 315, 315, 316, 316, 316, 317, 317,
		317, 317, 318, 318, 318, 319, 319, 319, 320, 320, 320, 320, 321, 321,
		321, 322, 322, 322, 323, 323, 323, 323, 324, 324, 324, 325, 325, 325,
		326, 326, 326, 326, 327, 327, 327, 328, 328, 328, 329, 329, 329, 330,
		330, 330, 330, 331, 331, 331, 332, 332, 332, 333, 333, 333, 333, 334,
		334, 334, 335, 335, 335, 336, 336, 336, 336, 337, 337, 337, 338, 338,
		338, 339, 339, 339, 339, 340, 340, 340, 341, 341, 341, 342, 342, 342,
		343, 343, 343, 343, 344, 344, 344, 345, 345, 345, 346, 346, 346, 346,
		347, 347, 347, 348, 348, 348, 349, 349, 349, 349, 350, 350, 350, 351,
		351, 351, 352, 352, 352, 353, 353, 353, 353, 354, 354, 354, 355, 355,
		355, 356, 356, 356, 356, 357, 357, 357, 358, 358, 358, 359, 359, 359,
		359, 360, 360, 360, 361, 361, 361, 362, 362, 362, 363, 363, 363, 363,
		364, 364, 364, 365, 365, 365, 366, 366, 366, 366, 367, 367, 367, 368,
		368, 368, 369, 369, 369, 369, 370, 370, 370, 371, 371, 371, 372, 372,
		372, 373, 373, 373, 373, 374, 374, 374, 375, 375, 375, 376, 376, 376,
		376, 377, 377, 377, 378, 378, 378, 379, 379, 379, 380, 380, 380, 380,
		381, 381, 381, 382, 382, 382, 383, 383, 383, 383, 384, 384, 384, 385,
		385, 385, 386, 386, 386, 387, 387, 387, 387, 388, 388, 388, 389, 389,
		389, 390, 390, 390, 390, 391, 391, 391, 392, 392, 392, 393, 393, 393,
		394, 394, 394, 394, 395, 395, 395, 396, 396, 396, 397, 397, 397, 397,
		398, 398, 398, 399, 399, 399, 400, 400, 400, 401, 401, 401, 401, 402,
		402, 402, 403, 403, 403, 404, 404, 404, 404, 405, 405, 405, 406, 406,
		406, 407, 407, 407, 408, 408, 408, 408, 409, 409, 409, 410, 410, 410,
		411, 411, 411, 411, 412, 412, 412, 413, 413, 413, 414, 414, 414, 415,
		415, 415, 415, 416, 416, 416, 417, 417, 417, 418, 418, 418, 419, 419,
		419, 419, 420, 420, 420, 421, 421, 421, 422, 422, 422, 422, 423, 423,
		423, 424, 424, 424, 425, 425, 425, 426, 426, 426, 426, 427, 427, 427,
		428, 428, 428, 429, 429, 429, 430, 430, 430, 430, 431, 431, 431, 432,
		432, 432, 433, 433, 433, 433, 434, 434, 434, 435, 435, 435, 436, 436,
		436, 437, 437, 437, 437, 438, 438, 438, 439, 439, 439, 440, 440, 440,
		441, 441, 441, 441, 442, 442, 442, 443, 443, 443, 444, 444, 444, 444,
		445, 445, 445, 446, 446, 446, 447, 447, 447, 448, 448, 448, 448, 449,
		449, 449, 450, 450, 450, 451, 451, 451, 452, 452, 452, 452, 453, 453,
		453, 454, 454, 454, 455, 455, 455, 456, 456, 456, 456, 457, 457, 457,
		458, 458, 458, 459, 459, 459, 459, 460, 460, 460, 461, 461, 461, 462,
		462, 462, 463, 463, 463, 463, 464, 464, 464, 465, 465, 465, 466, 466,
		466, 467, 467, 467, 467, 468, 468, 468, 469, 469, 469, 470, 470, 470,
		471, 471, 471, 471, 472, 472, 472, 473, 473, 473, 474, 474, 474, 475,
		475, 475, 475, 476, 476, 476, 477, 477, 477, 478, 478, 478, 479, 479,
		479, 479, 480, 480, 480, 481, 481, 481, 482, 482, 482, 483, 483, 483,
		483, 484, 484, 484, 485, 485, 485, 486, 486, 486, 486, 487, 487, 487,
		488, 488, 488, 489, 489, 489, 490, 490, 490, 490, 491, 491, 491, 492,
		492, 492, 493, 493, 493, 494, 494, 494, 494, 495, 495, 495, 496, 496,
		496, 497, 497, 497, 498, 498, 498, 498, 499, 499, 499, 500, 500, 500,
		501, 501, 501, 502, 502, 502, 502, 503, 503, 503, 504, 504, 504, 505,
		505, 505, 506, 506, 506, 506, 507, 507, 507, 508, 508, 508, 509, 509,
		509, 510, 510, 510, 510, 511, 511, 511, 512, 512, 512, 513, 513, 513,
		514, 514, 514, 514, 515, 515, 515, 516, 516, 516, 517, 517, 517, 518,
		518, 518, 518, 519, 519, 519, 520, 520, 520, 521, 521, 521, 522, 522,
		522, 522, 523, 523, 523, 524, 524, 524, 525, 525, 525, 526, 526, 526,
		526, 527, 527, 527, 528, 528, 528, 529, 529, 529, 530, 530, 530, 530,
		531, 531, 531, 532, 532, 532, 533, 533, 533, 534, 534, 534, 534, 535,
		535, 535, 536, 536, 536, 537, 537, 537, 538, 538, 538, 539, 539, 539,
		539, 540, 540, 540, 541, 541, 541, 542, 542, 542, 543, 543, 543, 543,
		544, 544, 544, 545, 545, 545, 546, 546, 546, 547, 547, 547, 547, 548,
		548, 548, 549, 549, 549, 550, 550, 550, 551, 551, 551, 551, 552, 552,
		552, 553, 553, 553, 554, 554, 554, 555, 555, 555, 555, 556, 556, 556,
		557, 557, 557, 558, 558, 558, 559, 559, 559, 559, 560, 560, 560, 561,
		561, 561, 562, 562, 562, 563, 563, 563, 564, 564, 564, 564, 565, 565,
		565, 566, 566, 566, 567};

	int g,d;
	int *waitdata, *sumdata;

	/* random walk range */
	trf_message("\nPmatch=%3.2f,Pindel=%3.2f",(float)PM/100,(float)PI/100);
	Pindel=(float)PI/100;
	for(g=1;g<=MAXDISTANCE;g++)
	{
		if(g<=SMALLDISTANCE)
		{      
			Distance[g].lo_d_range=g	/* this can never be less than one */;
			Distance[g].hi_d_range=g; /* this can never be greater */
			/* than MAXDISTANCE */
		}
		else
		{
			Distance[g].lo_d_range=
				max(g-d_range(g),1);	/* this can never be less than one */

			Distance[g].hi_d_range=
				min(g+d_range(g),MAXDISTANCE); /* this can never be greater */
			/* than MAXDISTANCE */
		}
	}

	/* Waiting time calculations */
	if(PM==80)
	{
		NTS=3;          /* Tuplesize[NTS+1]={0,4,5,7}; */
		Tuplesize[0]=0;
		Tuplesize[1]=4;
		Tuplesize[2]=5;
		Tuplesize[3]=7;
		trf_message("\ntuple sizes 0,4,5,7");
		Tuplemaxdistance[0]=0;
		Tuplemaxdistance[1]=29;
		Tuplemaxdistance[2]=159;
		Tuplemaxdistance[3]=MAXDISTANCE;
		trf_message("\ntuple distances 0, 29, 159, %d",MAXDISTANCE);

		/* assign pointers to data */
		waitdata = waitdata80;
	}
	else if(PM==75)
	{
		NTS=4;
		Tuplesize[0]=0;
		Tuplesize[1]=3;
		Tuplesize[2]=4;
		Tuplesize[3]=5;
		Tuplesize[4]=7;
		trf_message("\ntuple sizes 0,3,4,5,7");
		Tuplemaxdistance[0]=0;
		Tuplemaxdistance[1]=29;
		Tuplemaxdistance[2]=43;
		Tuplemaxdistance[3]=159;
		Tuplemaxdistance[4]=MAXDISTANCE;
		trf_message("\ntuple distances 0, 29, 43, 159, %d",MAXDISTANCE);


		/* assign pointers to strings with data */
		waitdata = waitdata75;
	}
	else 
	{
		trf_message("\nNo wait table file for PM=%d",PM);
		fprintf(stderr,"\nNo wait table file for PM=%d",PM);
		exit(-13);
	}

	/* StepFunction(waitdata+1, MAXDISTANCE);*/

	/* Oct 15, 2018 Yozen: truncate value of MAXDISTANCE to 2000
	 * if it exceeds that value, to avoid out-of-bounds crashes here.
	 * Arrays are only as large as 2004. This is a temporary change
	 * while we figure out the best approach to larger pattern sizes.
	 * We may also decide that patterns larger than that are simply
	 * out of scope for TRF */
	for(d=1;d<=MAXDISTANCE;d++)
		Distance[d].waiting_time_criteria = waitdata[min(2000, d)];

	/* k_run_sums_criteria */

	if(PM==80)
	{
		/* assign pointers to strings with data */
		sumdata = sumdata80;
	}
	else if(PM==75)
	{
		/* assign pointers to strings with data */
		sumdata = sumdata75;
	}
	else 
	{
		trf_message("\nNo sum table file for PM=%d",PM);
		fprintf(stderr,"\nNo sum table file for PM=%d",PM);
		exit(-13);
	}

	/* Oct 15, 2018 Yozen: truncate value of MAXDISTANCE to 2000 */
	for(d=1;d<=MAXDISTANCE;d++)
		Distance[d].k_run_sums_criteria = sumdata[min(2000, d)];

}


extern void SetProgressBar(void);



/*******************************************************************/

/****************************  newtupbo()  *****************************/

/*******************************************************************/

/*??*/

/* last update May 19, 1997 */
/* started may 13, 1997 */
/* uses different tuple sizes for different distances */

void newtupbo(void)
{
	int mintuplesize,maxtuplesize,build_entire_code,g,badcharindex;
	int code,y,i,h,d,yy,j;
	int found,progbarpos,percentincrease,onepercent;
	double oldcopynumber;
	int pass_multiples_test;

	/* Moved here by Yozen on Feb 16, 2016.
	Always initialize here. Freed at the end of this function.
	maxwraplength has already been initialized in the calling
	function, TRF */
	Bandcenter = calloc(maxwraplength+1, sizeof(*Bandcenter));

	/* Jan 27, 2006, Gelfand, changed to use Similarity Matrix to avoid N matching itself */
	/* This function may be called multiple times (for different match/mismatch scores) */
	init_sm(Alpha,Beta);


	/* set progress indicator to zero  */
	paramset.percent=0;
	if (paramset.ngs != 1) SetProgressBar();


	OUTPUTcount = 0; /* needed to make browser label unique */

	mintuplesize=Tuplesize[1];
	maxtuplesize=Tuplesize[NTS];
	for(g=1;g<=NTS;g++)
	{
		Tuplehash[g]=(int *)farcalloc(four_to_the[Tuplesize[g]],sizeof(int));
		Historysize[g]=2*(Tuplemaxdistance[g]+1)+2; /* The idea here is that no */
		/* previous history pointer points back */
		/* more than Tuplemaxdistance.  Then, when */
		/* History entry is reused, following */
		/* links from the current will exceed the */
		/* maxdistance before reaching the reused */
		/* entry. */
		History[g]=(struct historyentry *)
			farcalloc(Historysize[g], sizeof(struct historyentry));
		Nextfreehistoryindex[g]=1;  /* set all to 1 because 0 indicates */
		/* Tuplehash points to nothing */

	}

	Sortmultiples=(int *)calloc(MAXDISTANCE+1,sizeof(int));

	build_entire_code=1;

	onepercent = Length/100;
	percentincrease = 0;
	progbarpos = 0;
	for(i=0;i<=Length;i++)
	{
		/* if percent changed then set indicator */
		percentincrease++;
		if(percentincrease==onepercent)
		{
			percentincrease = 0;
			progbarpos++;
			paramset.percent = progbarpos;
			if (paramset.ngs != 1) SetProgressBar();
		}
		if((i==0)  /* before start of sequence or */
				||(strchr("acgtACGT",Sequence[i])==NULL))  /* not one of A,C,G,T */
		{
			badcharindex=i;
			build_entire_code=1;
			/* find first good string of mintupsize characters */
			g=0;
			while((g<mintuplesize)&&(i<Length))
			{
				i++;
				if(strchr("acgtACGT",Sequence[i])==NULL)
				{
					badcharindex=i;
					g=0;
				}
				else
					g++;
			}
			if(g<mintuplesize) break; /* i=Length and minimum tuple not found */
		}
		if(build_entire_code)
		{
			code=0;
			for(g=badcharindex+1;g<=i;g++)
			{
				code=code*4+Index[Sequence[g]];
			}
			if(i-badcharindex>=maxtuplesize){build_entire_code=0;}
		}
		else
		{
			code=(code%four_to_the[Tuplesize[NTS]-1])
				*4+Index[Sequence[i]];
		}
		Tuplecode[NTS]=code;
		for(h=NTS-1;h>=1;h--)
		{
			Tuplecode[h]=code%four_to_the[Tuplesize[h]];
		}

		/* process index i using all the tuplesizes */

		g=1;
		while((g<=NTS)&&(i-badcharindex>=Tuplesize[g]))
		{

			/* change 5/25/99 ends here */

			y=Tuplehash[g][Tuplecode[g]]; /* index in history list of last
							 occurrence of code */
			h=Nextfreehistoryindex[g]; /* next free index in history list */
			j=h+1;            /* advance next free index */
			if(j==Historysize[g]) j=1; /* we use a circular history list */

			if((History[g][j].location!=0) /* if the next entry has 
							  already been used */
					&&(j==Tuplehash[g][History[g][j].code])) /* check Tuplehash.  
										    If it still */
			{             /* points here, */
				Tuplehash[g][History[g][j].code]=0; /* zero it out.   */

			}
			Nextfreehistoryindex[g]=j;

			Tuplehash[g][Tuplecode[g]]=h; /* store index of current tuple */
			History[g][h].location=i; /* store info about current tuple */
			History[g][h].previous=y;
			History[g][h].code=Tuplecode[g];

			yy=h;         /* yy holds entry which points to y */
			while(y!=0)
			{
				d=i-History[g][y].location; /* d=distance between matching 
							       tuples */
				if(d>Tuplemaxdistance[g]) /* if d exceeds Tuplemaxdistance, 
							     then */
				{           /* make the previous location 0.  We */
					History[g][yy].previous=0; /* are no longer interested 
								      in the y */
					y=0;          /* entry.  It will be zeroed out when */
				} /* reused */
				else
				{
					yy=y;
					y=History[g][y].previous; /* get next matching tuple */
					/* process */
					/* is this a distance that is too small for the tuplesize? */
					/* recall that the History lists do not exclude distance */
					/* that are too short, only those that are too long  */
					if(d>Tuplemaxdistance[g-1])
					{
						/* add tuplematch to Distance */
						add_tuple_match_to_Distance_entry(i,Tuplesize[g],d,Distance);


						/* check if this distance has already been processed */

						/* found=search_for_distance_match_in_distanceseenlist(d,i); */
						/* modified 5/23/05 G. Benson */
						found=search_for_distance_match_in_distanceseenarray(d,i);


						if(!found)
						{


							/* is distance d linked into other nonzero distances? */
							/* if not, link it in */
							if(!Distance[d].linked) link_Distance_window(d);

							/* test criteria for candidate */


							if((new_meet_criteria_3(d,i,Tuplesize[g]))
									/* change 2: changed 250 into 100 */
									&&((d<=250)||(search_for_range_in_bestperiodlist(i,d)))) 
								/* use bestperiod list only for distances greater than 500 */           
							{
								/* align sequence against candidate */
								/* and get alignment */
								WDPcount++;
								Criteria_count[d]++;
								Rows=0;
								if(d<=SMALLDISTANCE)
								{
									newwrap(i,d,WITHOUTCONSENSUS);
									Cell_count[d]+=(Rows*d);
									get_pair_alignment_with_copynumber(d);
								}
								else        /* d is a large distance */
								{
									if (1%100==0) fprintf(stderr,"\ni=%d  d=%d",i,d);
									narrowbandwrap(i,d,max(MINBANDRADIUS,d_range(d)), min(2*max(MINBANDRADIUS,d_range(d)), (d/3) ), 
											WITHOUTCONSENSUS,RECENTERCRITERION);
									Cell_count[d]+=(Rows*(2*max(MINBANDRADIUS,d_range(d))+1));
									get_narrowband_pair_alignment_with_copynumber( d, min(2*max(MINBANDRADIUS,d_range(d)), (d/3) ), LOCAL );
								} 
								if(Meet_criteria_print)trf_message("\nFrom:%d, To:%d,  Copynumber:%f",
										AlignPair.indexprime[AlignPair.length],
										AlignPair.indexprime[1],Copynumber);
								/* add_to_distanceseenlist(i,d,Maxrealrow,Maxscore,WITHOUTCONSENSUS); */
								/* modified 5/23/05 G. Benson */
								add_to_distanceseenarray(i,d,Maxrealrow,Maxscore);




								/* changed 2/17/05 gary benson
								   change to make number of copies required less restrictive for pattern sizes >= 50
								   this ramps from 1.9 at pattern size = 50 down to 1.8 at pattern size = 100 or above */
								if((d<=50 && Copynumber<1.9)
										|| (d>50 && d<=100 && Copynumber < 1.9 - 0.002*(d-50))
										|| (d>100 && Copynumber < 1.8))
									/* if(Copynumber<1.9) */
									/* max(1.9,(double)Min_Distance_Window/(double)d)) */
									/*  ceil((double)Min_Distance_Window/(double)d)))*/
									/* rationale for max here; Min_Distance_Window is the */
									/* smallest actual tandem repeat we want to see, so */
									/* divide by d to get minimum number of copies */ 
								{

								}
								else
								{

									pass_multiples_test=multiples_criteria_4(d);
									add_to_bestperiodlist(d);
									if(pass_multiples_test)
									{

										/* get consensus */
										Classlength=d;
										get_consensus(d);
										if(ConsClasslength!=Classlength)
											Classlength=ConsClasslength;
										oldcopynumber=Copynumber;
										/* repeat alignment using consensus */
										Consensus_count[Classlength]++;
										Rows=0;
										if(Classlength<=SMALLDISTANCE)
										{
											newwrap(i,Classlength,WITHCONSENSUS);
											Cell_count[Classlength]+=(Rows*d);
											get_pair_alignment_with_copynumber(Classlength);
										}
										else    /* d is a large distance */
										{
											narrowbandwrap(i,Classlength,
													max(MINBANDRADIUS,d_range(Classlength)), min(2*max(MINBANDRADIUS,d_range(Classlength)), (Classlength/3) ), 
													WITHCONSENSUS,RECENTERCRITERION);
											Cell_count[Classlength]+=(Rows*(2*max(MINBANDRADIUS,
															d_range(Classlength))+1));
											get_narrowband_pair_alignment_with_copynumber( Classlength, min(2*max(MINBANDRADIUS,d_range(Classlength)), (Classlength/3) ), LOCAL );
										}
										/*add_to_distanceseenlist(i,Classlength,Maxrealrow,
										  Maxscore,WITHCONSENSUS); */
										/* modified 5/23/05 G. Benson */
										add_to_distanceseenarray(i,d,Maxrealrow,Maxscore);

										/* modified 6/10/05 G. Benson */
										adjust_bestperiod_entry(d);


										/* changed 2/17/05 gary
										   change to make number of copies required less restrictive for pattern sizes >= 50
										   this ramps from 1.9 at pattern size = 50 down to 1.8 at pattern size = 100 or above */
										if((Classlength<=50 && Copynumber<1.9)
												|| (Classlength>50 && Classlength<=100 && Copynumber < 1.9 - 0.002*(d-50))
												|| (Classlength>100 && Copynumber < 1.8))

											/* if(Copynumber<1.9) */
											/*  max(1.9,(double)Min_Distance_Window/(double)Classlength)) */
										{
										}
										else
										{
											if((Classlength>=Minsize)&&(AlignPair.score>=Minscore))
											{

												/* output repeat */
												OUTPUTcount++;          Outputsize_count[Classlength]++;
												/* sequence */
												trf_message("\nFound at i:%d original size:%d final size:%d",
														i,d,Classlength);
												get_statistics(Classlength);
											}


										}

									}



								}
							}
						}
					}
				}
			}
			g++;
		}
	}

	/* close progress indicator */
	paramset.percent = -1;
	if (paramset.ngs != 1) SetProgressBar();

	/* Added by Yozen on Feb 16, 2016. */
	free(Bandcenter);
}


#endif
