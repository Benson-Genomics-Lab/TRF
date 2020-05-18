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

/* Feb. 14th, 1997 */
/* This is a version which contains the narrow band alignment routines
   narrowbnd.c, prscores.c, pairalign.c */

#ifndef TR30DAT_H
#define TR30DAT_H

/* These declarations moved by Yevgeniy Gelfand on Jan 27, 2010  */
/* To have smaller sequences not send results */
/* to disc to improve performance             */
int counterInSeq = 0;

/* uncomment only one platform target identifier */

//#define WINDOWSGUI
//#define WINDOWSCONSOLE
//#define UNIXCONSOLE
//#define UNIXGUI

/* make sure only one platform is defined */
#if ( defined( WINDOWSGUI ) + defined( WINDOWSCONSOLE ) + defined( UNIXGUI ) + \
      defined( UNIXCONSOLE ) ) > 1
#error Only one Platform can be defined in tr30dat.h
#endif

/* make sure at least one platform is defined */
// #if ( defined( WINDOWSGUI ) + defined( WINDOWSCONSOLE ) + defined( UNIXGUI ) + \
//       defined( UNIXCONSOLE ) ) == 0
// #pragma message( \
//   "You forgot to define a platform when compiling. Setting UNIXCONSOLE." )
#if __unix__
#define UNIXCONSOLE
#undef WINDOWSGUI
#undef WINDOWSCONSOLE
#elif _WIN32
#define WINDOWSCONSOLE
#undef UNIXCONSOLE
#undef UNIXGUI
#endif

#ifdef UNIXCONSOLE
#define _MAX_PATH 260
#elif defined( UNIXGUI )

/* it is possible to do a windows GTK+ version */
#if !defined( _WIN32 )
#define _MAX_PATH 260  /* max. length of full pathname */
#define _MAX_DIR 260   /* max. length of path component */
#define _MAX_FNAME 260 /* max. length of file name component */
#define _MAX_EXT 260   /* max. length of extension component */
#endif

#endif

/* all include  libraries */
#include <stdio.h>
#include <stddef.h> /* has size_t definition */
#include <stdlib.h> /* has calloc definition */
#include <ctype.h>  /* includes toupper(c) */
#include <string.h> /* includes strncat() */
#include <math.h>   /* for ceil function */
#include <stdarg.h> /* for trf_message() function */

/* use semantic versioning, please: https://semver.org/ */
#ifndef PACKAGE_VERSION
#define versionstring "4.10.0"
#else
#define versionstring PACKAGE_VERSION
#endif

#define farcalloc calloc

#ifndef UNIXGUI
/* alredy defined in glib, used in unix graphical version */
#define TRUE 1
#define FALSE 0
#endif

#define WEIGHTCONSENSUS 0
#define REPEATCONSENSUS 0
#define MAXDISTANCECONSTANT 2000 /* should replaced by a variable, later */
#define MINDISTANCE 10
#define MINBANDRADIUS 6
#define RECENTERCRITERION 3
/* 6/24/05 G. Benson */
//#define MAXBANDWIDTH 300
#define MAXBANDWIDTH 150
#define MAXTUPLESIZES 10
/* 6/17/05 G. Benson */
/* 1/26/10 G. Benson */
/* make MAXWRAPLENGTH an integer instead of a constant */
/* #define MAXWRAPLENGTH 500000 */
/* #define MAXWRAPLENGTHCONST 500000 */
/* 11/17/15 G. Benson */
/* make MAXWRAPLENGTHCONST longer to accomodate long centromeric repeats */
/* #define MAXWRAPLENGTHCONST 5000000 */
/* 01/13/16 Y. Hernandez */
/* make MAXWRAPLENGTHCONST longer to accomodate longer repeat in Human chr 18,
 * HG38 */
/* #define MAXWRAPLENGTHCONST 10000000 */
/* 01/22/16 Y. Hernandez */
/* make MAXWRAPLENGTHCONST longer to accomodate longer repeat in Human chr 18,
 * HG38 */
/* 01/26/16 Y. Hernandez */
/* Let MAXWRAPLENGTHCONST be definable on the command line. Easier to update
 * without changing source. */
/* 02/05/16 Y. Hernandez */
/* End use of MAXWRAPLENGTHCONST macro, use a command line option instead. */
//#ifndef MAXWRAPLENGTHCONST
//#define MAXWRAPLENGTHCONST 10000000
//#endif
/* 02/05/16 Y. Hernandez */
/* Since this is no longer a macro, use all lower case to avoid confusion. */
unsigned int maxwraplength = 0;

/* Added by Yevgeniy Gelfand on Jan 27, 2010  */
/* To have smaller sequences not send results */
/* to disc to improve performance             */
// int TempFilesToDisc = 0;

#define MAXPATTERNSIZECONSTANT                    \
    MAXDISTANCECONSTANT /* replaced by a variable \
                         */
#define DASH '-'
#define BLANK ' '

#define WITHCONSENSUS 1
#define WITHOUTCONSENSUS 0

#define MULTIPLES 3 /* We keep MULTIPLES*d distance entries */
#define FILLMULTIPLE 3
#define LOWERENDMULTIPLE                     \
    3 /* how close the lower end of distance \
         indices has to be to the end of one copy */
#define ACCEPTED 1
#define NOTACCEPTED 0

#define SMALLDISTANCE 20 /* these are sizes for which we do a */
/* full array wraparound dynamic */
/* programming and for which we set d_range */
/* by hand */

int Min_Distance_Entries = 20; /* minimum number of places to store a tuple
                                match.  Usually this is the same as the
                                distance, but for small distances, we
                                allow more tuple matches because we want
                                see a significant number of matches */

int Min_Distance_Window = 20; /* minimum size of distance window. */
/* Usually this is the same as the */
/* distance, but for small distances we */
/* allow more space because we want a */
/* significant number of matches */

#define TAGSEP 50 /* index separation for tags table for linking */
/* active distances for distance range addition of */
/* matches */
int    PM;
int    PI;
double Pindel; /* expected probability of a single character indel in
                  the worst case tandem repeat. Pindel should be tied
                  to the indel cost parameter */

int        MAXDISTANCE    = 500;
int        MAXPATTERNSIZE = 500;
/* */ char debugbuffer[500];

/* G. Benson 1/28/2004 */
/* size of EC increased to avoid memory error when consensus length exceeds
   MAXPATTERNSIZECONSTANT after returning from get_consensus(d) */
/* Y. Hernandez 10/15/2018 */
/* If patternsize over 2000 is ever allowed, must change how this
 * variable is initialized. Must be a dynamically allocated array,
 * should use MAXPATTERNSIZE instead (but set elsewhere, after
 * user parameters have been processed).
 */
unsigned char EC[2 * ( MAXPATTERNSIZECONSTANT + 1 )];

int *Index;
int *ACGTcount;

unsigned char *Sequence;
int            Length;

/* int S[MAXWRAPLENGTH+1][MAXPATTERNSIZE];*/
int    Delta;   /* indel penalty */
int    Alpha;   /* match bonus */
int    Beta;    /* mismatch penalty */
int    AFDelta; /* affine gap initiation penalty */
int    AFGamma; /* affine gap extension penalty */
int    pwidth = 75;
int    Reportmin, Heading;
int    Classlength;
int    Test;
double Rows;
double Totalcharacters;
/* int Lookcount;*/
int  Wrapend;
int  Maxrealrow, Maxrow, Maxcol;
int  Maxscore;
int  ConsClasslength;
int *Tag;    /* list of tags for linking active distances */
int  Toptag; /* last tag in list */

struct pairalign {
    int   length;
    int   score;
    char *textprime, *textsecnd;
    int * indexprime, *indexsecnd;
} AlignPair;

struct cons_data {
    char pattern[2 * ( MAXPATTERNSIZECONSTANT + 1 )];
    int  A[2 * ( MAXPATTERNSIZECONSTANT + 1 )],
      C[2 * ( MAXPATTERNSIZECONSTANT + 1 )],
      G[2 * ( MAXPATTERNSIZECONSTANT + 1 )],
      T[2 * ( MAXPATTERNSIZECONSTANT + 1 )],
      dash[2 * ( MAXPATTERNSIZECONSTANT + 1 )],
      insert[2 * ( MAXPATTERNSIZECONSTANT + 1 )],
      letters[2 * ( MAXPATTERNSIZECONSTANT + 1 )],
      total[2 * ( MAXPATTERNSIZECONSTANT + 1 )];
} Consensus;

/* int Up[MAXPATTERNSIZE+1], Diag[MAXPATTERNSIZE+1];*/

/*******************************************/
/*** new program started 11-29-95 **********/

struct bestperiodlistelement {
    int                           indexhigh;
    int                           indexlow;
    int                           best1;
    int                           best2;
    int                           best3;
    int                           best4;
    int                           best5;
    struct bestperiodlistelement *next;
} Bestperiodlist[1];

struct distanceentry {
    int location;
    int size;
};

struct distancelist {
    int k_run_sums_criteria, waiting_time_criteria, lo_d_range, hi_d_range;
    int numentries, nummatches;
    int lowindex, highindex;
    int linked;
    int linkdown, linkup;
    struct distanceentry *entry;
} * Distance;

#define Lookratio .4

/* created 5/23/05 G. Benson */
/* this array replaces the distanceseenlist.  It stores the extent of
   alignments, both pre- or post-consensus, and is used to block an
   alignment in the same region with the same distance */

struct distanceseenarrayelement {
    int index;
    int end;
    int score;
} * Distanceseenarray;

struct distancelistelement {
    int index;
    int distance;
    int changed_from_distance; /* use for test in
                                  search_for_distance_match_in_distanceseenlist
                                  3/10/05 */
    int end;
    int score;
    int best_possible_score; /* number of copies X length X match weight */
    int accepted;
    struct distancelistelement *next;
} Distanceseenlist[1];
/*******************************************/

/* macros */

#define max4( a, b, c, d )                                                 \
    ( ( a >= b )                                                           \
        ? ( ( a >= c ) ? ( ( a >= d ) ? a : d ) : ( ( c >= d ) ? c : d ) ) \
        : ( ( b >= c ) ? ( ( b >= d ) ? b : d ) : ( ( c >= d ) ? c : d ) ) )
/* returns max of 4 in order a,b,c,d */

#define max3( a, b, c ) \
    ( ( a >= b ) ? ( ( a >= c ) ? a : c ) : ( ( b >= c ) ? b : c ) )
/* returns max of 3 in order a,b,c */

//#define match(a, b) ((a==b)?Alpha:Beta)
/* returns match mismatch matrix value */

/* Jan 27, 2006, Gelfand, changed to use Similarity Matrix to avoid N matching
 * itself */
/* This function may be called multiple times (for different match/mismatch
 * scores) */
int *SM = NULL;
#define match( a, b ) ( SM[256 * ( ( a ) ) + ( b )] )

#define fill_align_pair( c1, c2, l, i, j ) \
    AlignPair.textprime[l]  = c1;          \
    AlignPair.textsecnd[l]  = c2;          \
    AlignPair.indexprime[l] = i;           \
    AlignPair.indexsecnd[l] = j

#define max( a, b ) ( ( ( a ) >= ( b ) ) ? ( a ) : ( b ) )
#define min( a, b ) ( ( ( a ) <= ( b ) ) ? ( a ) : ( b ) )

double  Copynumber;
double  WDPcount;
double  OUTPUTcount;
int *   Criteria_count;
int *   Consensus_count;
int *   Outputsize_count;
double *Cell_count;
double  Try_waiting_time_count, Fail_waiting_time_count;
double  Cell_total, Wasted_total;
/**********************************************************************/
/* New to 2A */

#define GLOBAL 0
#define LOCAL 1

typedef struct {
    unsigned int match;
    unsigned int mismatch;
    unsigned int indel;
    unsigned int minscore;
    unsigned int maxperiod;
    unsigned int PM;
    unsigned int PI;
    int          datafile;
    int          maskedfile;
    int          flankingsequence;
    unsigned int flankinglength;
    int          HTMLoff;
    int          redundoff;
    int          ngs;
    int          use_stdin;
    unsigned int maxwraplength;

    char  inputfilename[_MAX_PATH]; /* constant defined in stdlib */
    char  outputprefix[_MAX_PATH];
    char  outputdirectory[_MAX_PATH];
    char  outputfilename[_MAX_PATH];
    int   multisequencefile; /* flags if file has more than one sequence */
    int   sequenceordinal;   /* holds seq. index starting on 1 */
    int   outputcount;       /* repeats found */
    int   guihandle;         /* this variable is only used in the GUI version */
    int   running;
    char *endstatus;
    int   percent;
} TRFPARAMSET;

TRFPARAMSET paramset; /* this global controls the algorithm */

/* G. Benson */
/* 1/26/10 */
/* change MAXWRAPLENGTH to MAXWRAPLENGTHCONST so MAXWRAPLENGTH can be used as an
 * int */
/* int Bandcenter[MAXWRAPLENGTH+1]; */
int *Bandcenter = NULL;

/* version 2A changes this */
/* int S[MAXWRAPLENGTH+1][MAXDISTANCE+1];*/
/* int Up[MAXDISTANCE+1], Diag[MAXDISTANCE+1];*/

/* int S[MAXWRAPLENGTH+1][MAXBANDWIDTH+1];*/
int **S;

int Up[MAXBANDWIDTH + 1], Diag[MAXBANDWIDTH + 1];
/* version 2A adds max3 and max2 */
#define max2( a, b ) ( ( a >= b ) ? a : b )
/* returns max of 2 in order a,b */

#define max3( a, b, c ) \
    ( ( a >= b ) ? ( ( a >= c ) ? a : c ) : ( ( b >= c ) ? b : c ) )
/* returns max of 3 in order a,b,c */

int Maxrealcol;

/* new for 2Anewt */

int four_to_the[] = {
  1, 4, 16, 64, 256, 1024, 4096, 16384, 65536, 262144, 1048576 };

/* #define Number_tuple_sizes  4 */
/* #define Number_tuple_sizes  4 */

int NTS; /* number of different tuple sizes to use;
            preset for all distances */
/* int Tuplesize[NTS+1]={0,2,3,5,7};*/ /* what the different sizes are */
/* int Tuplesize[NTS+1]={0,4,5,6,7};*/ /* what the different sizes are */
/* int Tuplesize[NTS+1]={0,3,4,5,7};*/
int Tuplesize[MAXTUPLESIZES + 1];
int Tuplemaxdistance[MAXTUPLESIZES + 1];

/* int Tuplemaxdistance[MAXTUPLESIZES+1]={0,30,80,200,MAXDISTANCE};*/ /* upper
                                                                         distance
                                                                         for
                                                                         each
                                                                         tuplesize
                                                                       */
/* int Tuplemaxdistance[MAXTUPLESIZES+1]={0,29,83,159,MAXDISTANCE};*/
/* int Tuplemaxdistance[MAXTUPLESIZES+1]={0,29,159,MAXDISTANCE};*/
int Tuplecode[MAXTUPLESIZES + 1]; /* this is where the actual tuple codes
                                   encountered at a sequence location
                                   are stored. */

int *Tuplehash[MAXTUPLESIZES + 1]; /* points to last location of code
                                    in history list */

int Historysize[MAXTUPLESIZES + 1]; /* size of history lists */

int Nextfreehistoryindex[MAXTUPLESIZES +
                         1]; /*next free location in history index*/

struct historyentry {
    int location, previous, code;
} * History[MAXTUPLESIZES + 1];

struct distribution_parameters {
    double exp;
    double var;
};

int  Criteria_print      = 0;
int  Meet_criteria_print = 0;
int *Sortmultiples;

/* define NUMBER_OF_PERIODS 3 */ /* returns <= 3 best periods for a repeat */
/* modified 3/25/05 G. Benson */
#define NUMBER_OF_PERIODS 5 /* determines 5 best periods for a repeat */
#define NUMBER_OF_PERIODS_TO_TEST \
    3 /* only test 3 best periods for multiples test */
#define NUMBER_OF_PERIODS_INTO_SORTMULTIPLES \
    5 /* added 5/25/05 for compatibility with bestperiodslist */

struct MDDtype {
    int   distance;
    char *direction;
}; /* MDD[MAXWRAPLENGTH+1][MAXBANDWIDTH+1];*/

int F[MAXBANDWIDTH + 1], Fdistance[MAXBANDWIDTH + 1];

int ldong;

int *Statistics_Distance;

FILE *Fptxt;
FILE *Fpdat;

int Minsize = 1;
int Minscore;
int MaxPeriod;

int Period;

int print_flanking = 0;

#define CTRL_SUCCESS 0
#define CTRL_BADFNAME -1
#define CTRL_BADFORMAT -2
#define CTRL_NOTHINGPROCESSED -3

/* the following structure is used to pass a sequence to the algorithm */
#define MAXSEQNAMELEN 200

typedef struct {
    /* Changed to unsigned Feb 16, 2016 Yozen */
    unsigned int length;
    int          composition[26];
    int          nucleotides;
    char         name[MAXSEQNAMELEN];
    char *       sequence;

} FASTASEQUENCE;

void trf_message( char *format, ... ) {

    va_list argp;

    if ( format == NULL )
        return;

    va_start( argp, format );

    if ( !paramset.HTMLoff )
        vfprintf( Fptxt, format, argp );

    va_end( argp );
}

#endif
