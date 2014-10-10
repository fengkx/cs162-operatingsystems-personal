/* "Copyright (c) 2014 The Regents of the University of California.  
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement
 * is hereby granted, provided that the above copyright notice, the following
 * two paragraphs and the author appear in all copies of this software.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY
 * OF CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 */

/*
 * @author David E. Culler
 */

#ifndef _so_H_
#define _so_H_

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define SO_EMPTY 0
#define SO_FULL 1

#define SO_DEPTH 4

typedef struct so_val {
  int linenum;
  char * line;
  int tid;
} so_val_t;

typedef struct sharedobject {
  /* Object access control */
  pthread_mutex_t solock;	/* Protect this opject */
  //  pthread_cond_t flag_cv;	/* Support efficient scheduling of threads on it */
  int flag;			/* Status: 1 Full / 0 empty */
  /* Object data elements */
  FILE *rfile;			/* Current file stream */
  int front;
  int nextempty;
  so_val_t queue[SO_DEPTH];
} so_t;

so_t     *new_so(FILE *rfile);
int      so_init(so_t *so, FILE *rfile);
void     so_insert(so_t *so, so_val_t *val); /* insert object into queue by copy */
char     *so_remove(so_t *so, so_val_t *val); /* remove object from queue by copy */
int so_close(so_t *so);

#endif
