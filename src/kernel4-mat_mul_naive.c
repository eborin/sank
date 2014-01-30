/***************************************************************************
 *   Copyright (C) 2014 by Edson Borin and Raul Baldin                     *
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

/* Matrices dimentions. */
#ifndef MATRIX_M
#define MATRIX_M 800
#endif
#ifndef MATRIX_K
#define MATRIX_K 800
#endif
#ifndef MATRIX_N
#define MATRIX_N 800
#endif

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
DATATYPE ma[MATRIX_M*MATRIX_K];
DATATYPE mb[MATRIX_K*MATRIX_N];
DATATYPE mc[MATRIX_M*MATRIX_N];

/* Kernel name. */
const char* kernel_name = "mat_mult_naive";

void mat_mult_naive (int m, int n, int k, double *A, double *B, double *C) 
{
  unsigned long i, j, p;
  double t;

  for (i = 0; i < (m); i++) {
    for (j = 0; j < (n); j++) {
      t = 0;
      for (p = 0; p < (k); p++) {
	t += A[i*k + p] * B[n*p + j];
      }
      C[j+i*n] = t ;
    }
  }
}

void kernel()
{
  mat_mult_naive (MATRIX_M, MATRIX_N, MATRIX_K, ma, mb, mc);
}

/* Amount of bytes accessed: (2 (read A, read B) * M*N*K + 1 (write C) * M*N )  * element size (in bytes)  */
double bytes = (2*(MATRIX_M * MATRIX_N * MATRIX_K) + 1*(MATRIX_M * MATRIX_N))  * sizeof(DATATYPE);
double fops = (MATRIX_M * MATRIX_N * MATRIX_K) * 2 /* 1 mult + 1 sum */;

/* -----------------------------*/
int main()
{
  unsigned long long k;
  double	     times[RPT];
  double             mintime = FLT_MAX;
  double             avgtime = 0;
  double             maxtime = 0;
  double             rate, avgrate;
  double             flrate, flavgrate;
  double             t;

  printf("Kernel name     : %s\n",kernel_name);
  printf("Matrix datatype : %s\n", XSTR(DATATYPE));
  printf("# of runs       : %d\n", RPT);
  printf("Matrices size   : C(%i x %i) = A(%i x %i) x B(%i x %i)\n", 
	 MATRIX_M, MATRIX_N, MATRIX_M, MATRIX_K, MATRIX_K, MATRIX_N);
  
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
  avgrate = (bytes / avgtime) / GB;
  flrate = (fops / mintime) / MB;
  flavgrate = (fops / avgtime) / MB;
  printf("Best Rate GB/s  : %6.2f\n",rate);
  printf("Avg  Rate GB/s  : %6.2f\n",avgrate);
  printf("Best MFLOPS     : %6.2f\n",flrate);
  printf("Avg  MFLOPS     : %6.2f\n",flavgrate);
  printf("Avg time        : %6.2f\n",avgtime);
  printf("Min time        : %6.2f\n",mintime);
  printf("Max time        : %6.2f\n",maxtime);

  return 0;
}

