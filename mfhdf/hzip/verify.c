/****************************************************************************
 * NCSA HDF                                                                 *
 * Software Development Group                                               *
 * National Center for Supercomputing Applications                          *
 * University of Illinois at Urbana-Champaign                               *
 * 605 E. Springfield, Champaign IL 61820                                   *
 *                                                                          *
 * For conditions of distribution and use, see the accompanying             *
 * hdf/COPYING file.                                                        *
 *                                                                          *
 ****************************************************************************/


#include "hdf.h"
#include "mfhdf.h"
#include "hcomp.h"
#include "verify.h"

/*-------------------------------------------------------------------------
 * Function: sds_verifiy_comp
 *
 * Purpose: utility function to verify compression for SDS_NAME
 *
 * Return: void
 *
 * Programmer: Pedro Vicente, pvn@ncsa.uiuc.edu
 *
 * Date: August 3, 2003
 *
 *-------------------------------------------------------------------------
 */

int sds_verifiy_comp(char *sds_name, int32 in_comp_type, int32 in_comp_info)
{
 intn          status_n;     /* returned status_n for functions returning an intn  */
 comp_coder_t  comp_type;    /* to retrieve compression type into */
 comp_info     comp_info;    /* compression structure */ 
 int32         sd_id,
               sds_id, 
               sds_index;   

 /* get chunk and comp */
 sd_id     = SDstart (FILENAME_OUT, DFACC_RDONLY);
 sds_index = SDnametoindex(sd_id, sds_name);
 if ((sds_id = SDselect(sd_id, sds_index))==FAIL) {
  printf("error:cannot open sds <%s>", sds_name);
  status_n = SDend (sd_id);
  return -1;
 }

/*-------------------------------------------------------------------------
 * retrieve and verify the compression info
 *-------------------------------------------------------------------------
 */
 
 comp_type = COMP_CODE_NONE;  /* reset variables before retrieving info */
 HDmemset(&comp_info, 0, sizeof(comp_info)) ;
 status_n = SDgetcompress(sds_id, &comp_type, &comp_info);
 if ( comp_type != in_comp_type )
 {
  printf("error:compression type does not match ");
  status_n = SDendaccess (sds_id);
  status_n = SDend (sd_id);
  return -1;
 }
 if (in_comp_info) {
  if ( comp_info.skphuff.skp_size != in_comp_info )
  {
   printf("error:compresion information does not match ");
   status_n = SDendaccess (sds_id);
   status_n = SDend (sd_id);
   return -1;
  }
 }
 
 /* terminate access to the sds */
 status_n = SDendaccess (sds_id);
 
 /* terminate access to the sd interface */
 status_n = SDend (sd_id);
 
 return 0;
 
}

/*-------------------------------------------------------------------------
 * Function: sds_verifiy_comp_all
 *
 * Purpose: utility function to verify compression for all SDSs
 *
 * Return: void
 *
 * Programmer: Pedro Vicente, pvn@ncsa.uiuc.edu
 *
 * Date: August 3, 2003
 *
 *-------------------------------------------------------------------------
 */

int sds_verifiy_comp_all(int32 in_comp_type, int32 in_comp_info)
{
 intn          status_n;     /* returned status_n for functions returning an intn  */
 comp_coder_t  comp_type;    /* to retrieve compression type into */
 comp_info     comp_info;    /* compression structure */ 
 int32         sd_id,
               sds_id, 
               sds_index,
               n_datasets,   /* number of datasets in the file */
               n_file_attrs, /* number of file attributes */
               data_type,              /* number type  */
               rrank,                  /* read rank */
               n_attrs,                /* number of attributes */
               dim_sizes[MAX_VAR_DIMS];/* dimensions of an image */
 char          name[MAX_GR_NAME];      /* name of dataset */

 /* initialize the sd interface */
 sd_id  = SDstart (FILENAME_OUT, DFACC_READ);
 
 /* determine the number of data sets in the file */
 if ((status_n = SDfileinfo (sd_id, &n_datasets, &n_file_attrs))==FAIL) {
  printf("error:cannot get file information");
  status_n = SDend (sd_id);
  return -1;
 }
 
 for (sds_index = 0; sds_index < n_datasets; sds_index++)
 {
  sds_id   = SDselect (sd_id, sds_index);
  
  /* skip dimension scales */
  if ( SDiscoordvar(sds_id) ) {
   SDendaccess(sds_id);
   continue;
  }

  status_n = SDgetinfo(sds_id, name, &rrank, dim_sizes, &data_type, &n_attrs);
 
 /*-------------------------------------------------------------------------
  * retrieve and verify the compression info
  *-------------------------------------------------------------------------
  */
  
  comp_type = COMP_CODE_NONE;  /* reset variables before retrieving info */
  HDmemset(&comp_info, 0, sizeof(comp_info)) ;
  status_n = SDgetcompress(sds_id, &comp_type, &comp_info);
  if ( comp_type != in_comp_type )
  {
   printf("error:compression type does not match ");
   status_n = SDendaccess (sds_id);
   status_n = SDend (sd_id);
   return -1;
  }
  if (in_comp_info) {
   if ( comp_info.skphuff.skp_size != in_comp_info )
   {
    printf("error:compresion information does not match ");
    status_n = SDendaccess (sds_id);
    status_n = SDend (sd_id);
    return -1;
   }
  }
  
  /* terminate access to the current dataset */
  status_n = SDendaccess (sds_id);
 }
 
 /* terminate access to the sd interface */
 status_n = SDend (sd_id);
 
 return 0;
}

/*-------------------------------------------------------------------------
 * Function: sds_verifiy_chunk
 *
 * Purpose: utility function to verify chunking for  SDS_NAME
 *
 * Return: void
 *
 * Programmer: Pedro Vicente, pvn@ncsa.uiuc.edu
 *
 * Date: August 3, 2003
 *
 *-------------------------------------------------------------------------
 */

int sds_verifiy_chunk(char *sds_name, int32 in_chunk_flags, int rank, 
                      int32 *in_chunk_lengths)
{
 intn          status_n;     /* returned status_n for functions returning an intn  */
 HDF_CHUNK_DEF chunk_def;    /* chunk defintion read */ 
 int32         chunk_flags;  /* chunking flag */ 
 int32         sd_id,
               sds_id, 
               sds_index;   
 int           i;

 /* get chunk and comp */
 sd_id     = SDstart (FILENAME_OUT, DFACC_RDONLY);
 sds_index = SDnametoindex(sd_id, sds_name);
 if ((sds_id = SDselect(sd_id, sds_index))==FAIL) {
  printf("error:cannot open sds <%s>", sds_name);
  status_n = SDend (sd_id);
  return -1;
 }
 status_n  = SDgetchunkinfo (sds_id, &chunk_def, &chunk_flags);

/*-------------------------------------------------------------------------
 * retrieve and verify the chunk info
 *-------------------------------------------------------------------------
 */
 if ( chunk_flags != (in_chunk_flags) )
 {
  printf("error:chunk flags do not match");
  status_n = SDendaccess (sds_id);
  status_n = SDend (sd_id);
  return -1;
 }
 for (i = 0; i < rank; i++)
 {
  if (chunk_def.chunk_lengths[i] != in_chunk_lengths[i] )
  {
   printf("error:chunk lengths do not match ");
   status_n = SDendaccess (sds_id);
   status_n = SDend (sd_id);
   return -1;
  }
 }

 /* terminate access to the sds */
 status_n = SDendaccess (sds_id);
 
 /* terminate access to the sd interface */
 status_n = SDend (sd_id);
 
 return 0;
 
}

/*-------------------------------------------------------------------------
 * Function: sds_verifiy_chunk_all
 *
 * Purpose: utility function to verify chunking for all SDSs
 *
 * Return: void
 *
 * Programmer: Pedro Vicente, pvn@ncsa.uiuc.edu
 *
 * Date: August 3, 2003
 *
 *-------------------------------------------------------------------------
 */

int sds_verifiy_chunk_all(int32 in_chunk_flags, int rank, 
                          int32 *in_chunk_lengths)
{
 intn          status_n;     /* returned status_n for functions returning an intn  */
 HDF_CHUNK_DEF chunk_def;    /* chunk defintion read */ 
 int32         chunk_flags;  /* chunking flag */ 
 int32         sd_id,
               sds_id, 
               sds_index,
               n_datasets,   /* number of datasets in the file */
               n_file_attrs, /* number of file attributes */
               data_type,              /* number type  */
               rrank,                  /* read rank */
               n_attrs,                /* number of attributes */
               dim_sizes[MAX_VAR_DIMS];/* dimensions of an image */
 char          name[MAX_GR_NAME];      /* name of dataset */
 int           i;

 /* initialize the sd interface */
 sd_id  = SDstart (FILENAME_OUT, DFACC_READ);
 
 /* determine the number of data sets in the file */
 if ((status_n = SDfileinfo (sd_id, &n_datasets, &n_file_attrs))==FAIL) {
  printf("error:cannot get file information");
  status_n = SDend (sd_id);
  return -1;
 }
 
 for (sds_index = 0; sds_index < n_datasets; sds_index++)
 {
  sds_id   = SDselect (sd_id, sds_index);
  
  /* skip dimension scales */
  if ( SDiscoordvar(sds_id) ) {
   SDendaccess(sds_id);
   continue;
  }

  status_n = SDgetinfo(sds_id, name, &rrank, dim_sizes, &data_type, &n_attrs);
  status_n = SDgetchunkinfo (sds_id, &chunk_def, &chunk_flags);
  
 /*-------------------------------------------------------------------------
  * retrieve and verify the chunk info
  *-------------------------------------------------------------------------
  */
  if ( chunk_flags != (in_chunk_flags) )
  {
   printf("error:chunk flags do not match");
   status_n = SDendaccess (sds_id);
   status_n = SDend (sd_id);
   return -1;
  }
  for (i = 0; i < rank; i++)
  {
   if (chunk_def.chunk_lengths[i] != in_chunk_lengths[i] )
   {
    printf("error:chunk lengths do not match ");
    status_n = SDendaccess (sds_id);
    status_n = SDend (sd_id);
    return -1;
   }
  }
  
  /* terminate access to the current dataset */
  status_n = SDendaccess (sds_id);
 }
 
 /* terminate access to the sd interface */
 status_n = SDend (sd_id);
 
 return 0;
 
}

