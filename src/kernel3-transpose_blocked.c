/***************************************************************************
 *   Copyright (C) 2014 by Edson Borin                                     *
 *   edson@ic.unicamp.br                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <float.h> // FLT_MAX
#include <stdio.h> // printf

/* Matrix size = MATRIX_N x MATRIX_N. */
#ifndef MATRIX_N
#define MATRIX_N 5000
#endif

/* Blocking factor (block size). 
   NOTES:
   - Avoid power of 2 (cache sub set corner cases)
   - MATRIX_N must be a multiple of BLK_FACTOR.
 */
#define BLK_FACTOR 250

/* Array data type. */
#ifndef DATATYPE
#define DATATYPE double
#endif

/* Number of times each kernel will be executed. */
#define RPT 10

/* Useful macros! */
#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))
#define XSTR(s)  STR(s)
#define STR(s)   #s
/*------------------------------------------------*/
/* Code to remove data from the processor caches. */
#define KB (1024)
#define MB (1024 * KB)
#define GB (1024 * MB)
#define LARGEST_CACHE_SZ (16 * MB)
static unsigned char dummy_buffer[LARGEST_CACHE_SZ];
void clean_cache()
{
  unsigned long long i;
  for (i=0; i<LARGEST_CACHE_SZ; i++)
    dummy_buffer[i] += 1;
}

/*------------------------------------------------*/
/* Code to read the wall clock time.              */
#include <sys/time.h>
double mysecond()
{
  struct timeval tp;
  struct timezone tzp;
  gettimeofday(&tp,&tzp);
  return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}

/*------------------------------------------------*/
/* Numeric kernels and data        .              */

/* Matrices. */
DATATYPE ma[MATRIX_N][MATRIX_N];
DATATYPE mb[MATRIX_N][MATRIX_N];

/* Kernel name. */
const char* kernel_name = "transposed_blocked";

void kernel()
{
  int i,j,ik,jk;
  for (ik=0; ik<MATRIX_N; ik+=BLK_FACTOR)
    for (jk=0; jk<MATRIX_N; jk+=BLK_FACTOR)
      for (i=ik; i<(ik+BLK_FACTOR); i++)
	for (j=jk; j<(jk+BLK_FACTOR); j++)
	  mb[j][i] = ma[i][j];
}

/* Amount of bytes accessed: 2 (1 read + 1 write) * matrix size * element size (in bytes)  */
double bytes = 2 * (MATRIX_N * MATRIX_N) * sizeof(DATATYPE);

/* -----------------------------*/
int main()
{
  unsigned long long k;
  double	     times[RPT];
  double             mintime = FLT_MAX;
  double             avgtime = 0;
  double             maxtime = 0;
  double             rate;
  double             t;

  printf("Kernel name     : %s\n",kernel_name);
  printf("Matrix datatype : %s\n", XSTR(DATATYPE));
  printf("# of runs       : %d\n", RPT);
  printf("Matrices size   : %d x %d\n", MATRIX_N, MATRIX_N);
  printf("Blocking factor : %d\n", BLK_FACTOR);
  if (MATRIX_N % BLK_FACTOR != 0) {
    printf("WARNING: MATRIX_N (%d) must be a multiple of BLK_FACTOR (%d)\n", 
	   MATRIX_N, BLK_FACTOR);
  }
  
  /* Main loop. */
  for (k=0; k<RPT; k++)
  {
    clean_cache();
    t = mysecond();
    /* Kernel */
    kernel();
    times[k] = mysecond() - t;
    //printf(" -> %6.2f s\n", times[k]);
  }


  /* Final report */
  for (k=1; k<RPT; k++) 
  /* Discard first iteration (k=1). */
  { 
    avgtime = avgtime + times[k];
    mintime = MIN(mintime, times[k]);
    maxtime = MAX(maxtime, times[k]);
  }
  avgtime = avgtime / (RPT-1);
  rate = (bytes / mintime) / GB;
  printf("Best Rate GB/s  : %6.2f\n",rate);
  printf("Avg time        : %6.2f\n",avgtime);
  printf("Min time        : %6.2f\n",mintime);
  printf("Max time        : %6.2f\n",maxtime);

  return 0;
}

