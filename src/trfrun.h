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

/* Added by Yozen to explicitly include header for windows GUI definitons on Jan
 * 25, 2016 */
#ifdef WINDOWSGUI
#include <windows.h>
#include <windef.h>
#include <winuser.h>
#endif

#include "tr30dat.h"

#ifndef _MAX_PATH
#define _MAX_PATH 260
#endif

// how much memory to allocate at first when loading a sequence
// (added by Eugene Scherba, 2010-02-16)
#define MEMBLOCK ( 10 * 1024 * 1024 )

#define new1Darrayfunc( type, functionname, length )             \
    type *functionname( int length ) {                           \
        type *objptr = calloc( length, sizeof( *objptr ) );      \
        if ( objptr == NULL ) {                                  \
            paramset.endstatus = "functionname: Out of memory!"; \
            return NULL;                                         \
        }                                                        \
        return ( objptr );                                       \
    }

int LoadSequenceFromFileEugene(
  FASTASEQUENCE *pseq, FILE *fp ); /* may use stdin and better file reading for
                                      handling large files over 2GB */
int LoadSequenceFromFileBenson( FASTASEQUENCE *pseq,
  FILE *fp ); /* old function, uses filepos, 32bit version of this would not
                 process a file over 2GB properly */
void TRFControlRoutine( void );
void init_bestperiodlist( void );
void free_bestperiodlist( void );
void TRF( FASTASEQUENCE *pseq );
void PrintError( char *errortext );
void PrintProgress( char *progresstext );
void SetProgressBar( void );
int  LoadSequenceFromFileBenson( FASTASEQUENCE *pseq, FILE *fp );
int  LoadSequenceFromFileEugene( FASTASEQUENCE *pseq, FILE *fp );

#ifdef UNIXGUI
extern void set_progress_bar( double fraction );
#endif

#endif
