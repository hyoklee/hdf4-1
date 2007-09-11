/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF.  The full HDF copyright notice, including       *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at      *
 * http://hdfgroup.org/products/hdf4/doc/Copyright.html.  If you do not have *
 * access to either file, you may request a copy from help@hdfgroup.org.     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#ifndef REPACK_LSTTABLE_H_
#define REPACK_LSTTABLE_H_


#include "hdf.h"
#include "mfhdf.h"

#define PFORMAT  "  %-7s %-7s %-7s\n" /*chunk info, compression info, name*/
#define PFORMAT1 "  %-7s %-7s %-7s"   /*chunk info, compression info, name*/

#ifdef __cplusplus
extern "C" {
#endif


/*
 structure to store the tag/ref and path of an object 
 the pair tag/ref uniquely identifies an HDF object 
*/
typedef struct obj_info_t {
 int   tag;
 int   ref;
 char  *path;  /* build a path for each object using the folder separation symbol "/" 
                  along the file traversal */
} obj_info_t;

/*struct that stores all objects */
typedef struct list_table_t {
 int        size;
 int        nobjs;
 obj_info_t *objs;
} list_table_t;


/* table methods */
void  list_table_init(list_table_t **list_tbl);
void  list_table_free(list_table_t *list_tbl);
int   list_table_search(list_table_t *list_tbl, int tag, int ref );
void  list_table_add(list_table_t *list_tbl, int tag, int ref, char* obj_name);
const char* list_table_check(list_table_t *list_tbl, char*obj_name);
void  list_table_print(list_table_t *list_tbl);

#ifdef __cplusplus
}
#endif


#endif  /* REPACK_LSTTABLE_H_ */
