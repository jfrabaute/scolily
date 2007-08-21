/***********************************************************************************
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
************************************************************************************/

#ifndef _SCOLILY_H
#define _SCOLILY_H     1

#include "record.h"

#include <math.h>

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>

#include <pthread.h>
#include <dlfcn.h>

#define MINUTE_USEC	60*1000*1000

void * ComputeLoopTh(void *);
void * WriteNotesTh(void *);
#endif

