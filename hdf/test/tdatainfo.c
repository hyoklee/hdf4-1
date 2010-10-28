/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 *                                                                           *
 * This file is part of HDF.  The full HDF copyright notice, including       *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at      *
 * http://hdfgroup.org/products/hdf4/doc/Copyright.html.  If you do not have *
 * access to either file, you may request a copy from help@hdfgroup.org.     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* $Id: tdatainfo.c 5334 2010-01-28 06:10:50Z bmribler $ */

#include "hdf.h"

#define DATAINFO_TESTER
#define MFAN_TESTER
#include "hdatainfo.h"
#include "tdatainfo.h"
#include "tproto.h"

/************ Draft - still need a little more clean up ***********/

/***********************************************************************
  NOTE: At this time, many of the offsets in these tests are verified
	by hard-coding.  When adding new data to the files in exiting
	tests, please either add data at the bottom of the files or
	change the hard-coded offsets appropriately, using the values
	from the error messages.  However, you'll need to verify the
	correctness of these values first using the command 'od' on
	the hdf file.
*************************************************************************/

/* Structure to hold annotation datainfo temporarily */
typedef struct
  {
	char anntext[100]; /* values of the annotation */
	int32 offset; /* offset of data block */
	int32 length; /* length of data block */
  } t_ann_info_t;

/*********** Functions to access t_hdfdatainfo_t **********************/

/* alloc_info is a utility function that allocates hdf_datainfo_t's members*/
intn alloc_info(t_hdf_datainfo_t *info, uintn info_count)
{
    info->offsets = (int32 *) HDmalloc(info_count * sizeof(int32));
    if (info->offsets == NULL)
        return -1;
    info->lengths = (int32 *) HDmalloc(info_count * sizeof(int32));
    if (info->lengths == NULL)
        return -1;
    return 0;
}

void free_info(t_hdf_datainfo_t *info)
{
    if (info != NULL) {
        if (info->offsets != NULL)
            HDfree(info->offsets);
        if (info->lengths != NULL)
            HDfree(info->lengths);
    }
}

/****************************************************************************
   Name: test_simple_vs() - tests non-special Vdatas

   Description:
	This routine creates and writes contiguous data to vdatas and verifies
        some data and data information with VSgetdatainfo.  The tests include
        the following vdatas:
	- a single field vdata with int32 values
	- a multiple field/multiple order vdata with all int32 values
	- a single field vdata with character values
	Only the character vdata's values are verified against the original
	buffer.  The other two were verified by examining the hdf file using
	od. (Todo 1)
   BMR - Jul 2010
 ****************************************************************************/
#define SIMPLE_FILE	"datainfo_simple.hdf"   /* data file */
#define NONSPECIAL_VS	"Non-Special Vdata"
#define CONTCLASS_NAME	"Contiguous Vdata"
#define LINKED_BLOCK	"Linked-block Vdata"
#define FIELD1_NAME	"Field1"	/* contains three integers */
#define FIELD2_NAME	"Field2"	/* contains one integer */
#define FIELD3_NAME	"Field3"	/* contains two integers */
#define	FIELD_NAME_LIST	"Field1,Field2,Field3"
#define ORDER_1 	3	/* order of first field of 1st vdata */
#define ORDER_2 	1	/* order of second field of 1st vdata */
#define ORDER_3 	2	/* order of third field of 1st vdata */
#define	BLOCK_SIZE	80	/* arbitrary number for block size */
#define	NUM_BLOCKS	8	/* arbitrary number for number of blocks */
#define N_VALS_PER_REC_1   1    /* # of values per record in single vdata */
#define N_VALS_PER_REC_2 (ORDER_1+ORDER_2+ORDER_3) /* # of vals/rec. in multiple vdata */
#define N_RECORDS	5	/* number of records to be written to the
				   vdatas at every write */
static void
test_simple_vs(void)
{
    int32 fid,		/* File ID */
	  vsid,		/* Vdata ID */
	  vs_ref,	/* Vdata ref# */
	  n_records,	/* number of records actually written to vdata */
	  data_buf0[N_RECORDS][N_VALS_PER_REC_1], /* for single vdata's data */
	  data_buf1[N_RECORDS][N_VALS_PER_REC_2]; /* for multiple vdata's data*/
    char  data_buf2[N_RECORDS] = {'H', '4', 'M', 'A', 'P'};
    int16 rec_num;	/* current record number */
    int32 offset, length; /* offset/length buffers for single block of data */
    intn  n_blocks;	/* number of blocks a vdata has */
#ifdef NOTUSED
    t_hdf_datainfo_t vs_info; /* data structure to hold offset/length arrays and
	some other information about the data */
#endif
    int32 status;	/* Status values from routines */
    intn status_n;	/* Status values from routines */

    /* Open the HDF file and initialize the interface. */
    fid = Hopen(SIMPLE_FILE, DFACC_CREATE, 0);
    CHECK_VOID(fid,FAIL,"Hopen");
    status = Vstart(fid);
    CHECK_VOID(status,FAIL,"Vstart");

    /* Create and write to the first vdata using high-level func VHstoredata */
    for (rec_num = 0; rec_num < N_RECORDS; rec_num++)
	data_buf0[rec_num][0] = 5 + rec_num;

    vs_ref = VHstoredata(fid, "Only field", (const uint8 *)data_buf0, N_RECORDS, DFNT_INT32, "One Field One Order", CONTCLASS_NAME);

    /*
     * Get access to the vdata just created then get and verify offset/length
     * of its data
     */

    vsid = VSattach (fid, vs_ref, "w");
    CHECK_VOID(vsid, FAIL, "VSattach");

    /* Get the number of data blocks first, should be 1 */
    n_blocks = VSgetdatainfo(vsid, 0, 0, NULL, NULL);
    CHECK_VOID(n_blocks, FAIL, "VSgetdatainfo");
    VERIFY_VOID(n_blocks, 1, "VSgetdatainfo");

    /* Get the offset/length of the data, should be 294/20 */
    n_blocks = VSgetdatainfo(vsid, 0, n_blocks, &offset, &length);
    CHECK_VOID(n_blocks, FAIL, "VSgetdatainfo");

    /* Verify offset/length */
    VERIFY_VOID(offset, 294, "VSgetdatainfo offset from 'One Field One Order'");
    VERIFY_VOID(length, 20, "VSgetdatainfo length from 'One Field One Order'");

    status = VSdetach (vsid);
    CHECK_VOID(status, FAIL, "Vdetach");

    /*
     * Create a multi-field/multi-order vdata, named NONSPECIAL_VS, in class
     * CONTCLASS_NAME, write data to it, then get and verify its data's
     * offsets/lengths
     */
    vsid = VSattach (fid, -1, "w");
    CHECK_VOID(vsid, FAIL, "VSattach");

    /* Set name and class name of the vdata. */
    status = VSsetname (vsid, NONSPECIAL_VS);
    CHECK_VOID(status, FAIL, "VSsetname");
    status = VSsetclass (vsid, CONTCLASS_NAME);
    CHECK_VOID(status, FAIL, "VSsetclass");

    /* Record its reference number for later access before detaching it */
    vs_ref = VSQueryref(vsid);
    CHECK_VOID(vs_ref, FAIL, "VSQueryref:vsid");
    status = VSdetach (vsid);
    CHECK_VOID(status, FAIL, "Vdetach");

    /* Attach to vdata NONSPECIAL_VS to write data, but first verify that
	number of data blocks is 0 */
    vsid = VSattach (fid, vs_ref, "w");
    CHECK_VOID(vsid, FAIL, "VSattach");

    n_blocks = VSgetdatainfo(vsid, 0, 0, NULL, NULL);
    CHECK_VOID(n_blocks, FAIL, "VSgetdatainfo");
    VERIFY_VOID(n_blocks, 0, "VSgetdatainfo");

    /* Define the fields in the vdata */
    status_n = VSfdefine (vsid, FIELD1_NAME, DFNT_INT32, ORDER_1);
    CHECK_VOID(status_n, FAIL, "VSfdefine");
    status_n = VSfdefine (vsid, FIELD2_NAME, DFNT_INT32, ORDER_2);
    CHECK_VOID(status_n, FAIL, "VSfdefine");
    status_n = VSfdefine (vsid, FIELD3_NAME, DFNT_INT32, ORDER_3);
    CHECK_VOID(status_n, FAIL, "VSfdefine");
    status_n = VSsetfields (vsid, FIELD_NAME_LIST);
    CHECK_VOID(status_n, FAIL, "VSsetfields");

    /* 
     * Buffer the data by the record for fully interlaced mode.  Note that the
     * first three elements contain the three values of the first field, the
     * fourth element contains the value of the second field, and the last two
     * elements contain the two values of the third field.
     */
    for (rec_num = 0; rec_num < N_RECORDS; rec_num++)
    {
        data_buf1[rec_num][0] = 2;	/* easier to inspect the bin file */
        data_buf1[rec_num][1] = 2;
        data_buf1[rec_num][2] = 2;
        data_buf1[rec_num][3] = 2;
        data_buf1[rec_num][4] = 2;
        data_buf1[rec_num][5] = 2;
        /*  data_buf1[rec_num][0] = 1 + rec_num;
        data_buf1[rec_num][1] = 2 + rec_num;
        data_buf1[rec_num][2] = 3 + rec_num;
        data_buf1[rec_num][3] = 10 + rec_num;
        data_buf1[rec_num][4] = 10;
        data_buf1[rec_num][5] = 65;
 */ 
    }

    /* Set the block size and the number of blocks */
    status_n = VSsetblocksize(vsid, BLOCK_SIZE);
    CHECK_VOID(status_n, FAIL, "VSsetblocksize");
    status_n = VSsetnumblocks(vsid, NUM_BLOCKS);
    CHECK_VOID(status_n, FAIL, "VSsetnumblocks");

    /* Write the data from data_buf1 to the non special vdata */
    n_records = VSwrite(vsid, (uint8 *)data_buf1, N_RECORDS, FULL_INTERLACE);
    VERIFY_VOID(n_records, N_RECORDS, "VSwrite:vsid");

    n_blocks = VSgetdatainfo(vsid, 0, 0, NULL, NULL);
    CHECK_VOID(n_blocks, FAIL, "VSgetdatainfo");
    VERIFY_VOID(n_blocks, 1, "VSgetdatainfo:vsid");

    n_blocks = VSgetdatainfo(vsid, 0, n_blocks, &offset, &length);
    CHECK_VOID(n_blocks, FAIL, "VSgetdatainfo");

    /* Verify offset/length */
    VERIFY_VOID(offset, 456, "VSgetdatainfo offset from NONSPECIAL_VS");
    VERIFY_VOID(length, 120, "VSgetdatainfo length from NONSPECIAL_VS");

    status_n = VSdetach (vsid);
    CHECK_VOID(status, FAIL, "Vdetach");

    /* Create and write to another simple vdata, named 'Characters Only', in
	class CONTCLASS_NAME */
    vs_ref = VHstoredata(fid, "Only field", (const uint8 *)data_buf2,
		 N_RECORDS, DFNT_CHAR, "Characters Only", CONTCLASS_NAME); 


    /* Attach to vdata 'Characters Only' and get offset and length of its data */
    vsid = VSattach(fid, vs_ref, "r");

    n_blocks = VSgetdatainfo(vsid, 0, 0, NULL, NULL);
    CHECK_VOID(n_blocks, FAIL, "VSgetdatainfo");
    VERIFY_VOID(n_blocks, 1, "VSgetdatainfo NONSPECIAL_VS");

#ifdef NOTUSED
    /* Allocate space to record the vdata's data info */
    alloc_info(&vs_info, n_blocks);

    /* Record various info */
    vs_info.n_values = 5;
    vs_info.numtype = DFNT_CHAR;
#endif
    /* Get offset/length */
    n_blocks = VSgetdatainfo(vsid, 0, n_blocks, &offset, &length);
    CHECK_VOID(n_blocks, FAIL, "VSgetdatainfo");

    /* Close everything */
    status = VSdetach (vsid);
    CHECK_VOID(status, FAIL, "Vdetach");

    status_n = Vend(fid);
    CHECK_VOID(status,FAIL,"Vend");

    status_n = Hclose(fid);
    CHECK_VOID(status,FAIL,"Hclose");

    /* Open the file with fopen, read data at the offset obtained and verify
       the values */
    status_n = readnoHDF_char(SIMPLE_FILE, offset, length, data_buf2);
    if (status_n == FAIL)
	fprintf(stderr, "Attempt reading data without HDF4 library failed at line %d\n", __LINE__);
} /* test_simple_vs() */


/****************************************************************************
   Name: test_append_vs() - tests Vdatas with linked-blocks

   Description:
	This routine creates and writes data to an appendable vdata and
	verifies its data information with VSgetdatainfo.  The tests include
        the following:
	- create and write to an appendable vdata
	- create and write to a simple vdata
	- add data to the appendable vdata
	- use VSgetdatainfo to get offsets/lengths of the linked-blocks of data
   BMR - Jul 2010
 ****************************************************************************/
#define APPEND_FILE	"datainfo_linkblock.hdf"   /* data file */
#define APPENDABLE_VD	"Appendable Vdata"
static void
test_append_vs()
{
    int32 fid;       /* file ID */
    int32 apvsid;    /* vdata IDs */
    int32 vs1_ref,   /* vdata ref# */
   	 n_records,  /* number of records written to vdata */
         data_buf0[N_RECORDS][N_VALS_PER_REC_1], /* for "Very Simple Vdata" */
         data_buf1[N_RECORDS][N_VALS_PER_REC_2]; /* for first vdata's data */
    int16 rec_num;      /* current record number */
    intn n_blocks;
    t_hdf_datainfo_t vs_info;
    int32 status;       /* Status values from routines */
    intn status_n;       /* Status values from routines */

    /* Open the HDF file. */
    fid = Hopen(APPEND_FILE, DFACC_CREATE, 0);
    CHECK_VOID(fid,FAIL,"Hopen");

    /* Initialize HDF for subsequent vgroup/vdata access. */
    status_n = Vstart(fid);
    CHECK_VOID(status_n,FAIL,"Vstart");

    /* Create the first vdata */
    apvsid = VSattach (fid, -1, "w");
    CHECK_VOID(apvsid, FAIL, "VSattach");

    vs1_ref = VSQueryref(apvsid);
    CHECK_VOID(vs1_ref, FAIL, "VSQueryref:apvsid");

    status = VSdetach (apvsid);
    CHECK_VOID(status, FAIL, "Vdetach");

    apvsid = VSattach (fid, vs1_ref, "w");
    CHECK_VOID(apvsid, FAIL, "VSattach");

    /* Set name and class name of the vdata. */
    status = VSsetname (apvsid, APPENDABLE_VD);
    CHECK_VOID(status, FAIL, "VSsetname");
    status = VSsetclass (apvsid, LINKED_BLOCK);
    CHECK_VOID(status, FAIL, "VSsetclass");

    n_blocks = VSgetdatainfo(apvsid, 0, 0, NULL, NULL);
    CHECK_VOID(n_blocks, FAIL, "VSgetdatainfo");
    VERIFY_VOID(n_blocks, 0, "VSgetdatainfo");

    /* Introduce each field's name, data type, and order.  This is the first
      part in defining a field.  */
    status_n = VSfdefine (apvsid, FIELD1_NAME, DFNT_INT32, ORDER_1);
    CHECK_VOID(status_n, FAIL, "VSfdefine");
    status_n = VSfdefine (apvsid, FIELD2_NAME, DFNT_INT32, ORDER_2);
    CHECK_VOID(status_n, FAIL, "VSfdefine");
    status_n = VSfdefine (apvsid, FIELD3_NAME, DFNT_INT32, ORDER_3);
    CHECK_VOID(status_n, FAIL, "VSfdefine");

    /* Finalize the definition of the fields. */
    status_n = VSsetfields (apvsid, FIELD_NAME_LIST);
    CHECK_VOID(status_n, FAIL, "VSsetfields");

    /* 
     * Buffer the data by the record for fully interlaced mode.  Note that the
     * first three elements contain the three values of the first field, the
     * fourth element contains the value of the second field, and the last two
     * elements contain the two values of the third field.
     */
    for (rec_num = 0; rec_num < N_RECORDS; rec_num++)
    {
        data_buf1[rec_num][0] = 5;	/* easier to inspect the bin file */
        data_buf1[rec_num][1] = 5;
        data_buf1[rec_num][2] = 5;
        data_buf1[rec_num][3] = 5;
        data_buf1[rec_num][4] = 5;
        data_buf1[rec_num][5] = 5;

	 /* data_buf1[rec_num][0] = 1 + rec_num;
        data_buf1[rec_num][1] = 2 + rec_num;
        data_buf1[rec_num][2] = 3 + rec_num;
        data_buf1[rec_num][3] = 10 + rec_num;
        data_buf1[rec_num][4] = 10;
        data_buf1[rec_num][5] = 65;
 */ 
    }

    /* Set the block size and the number of blocks the first vdata */
    status_n = VSsetblocksize(apvsid, BLOCK_SIZE);
    CHECK_VOID(status_n, FAIL, "VSsetblocksize");
    status_n = VSsetnumblocks(apvsid, NUM_BLOCKS);
    CHECK_VOID(status_n, FAIL, "VSsetnumblocks");

    /* Write the data from data_buf1 to vdata APPENDABLE_VD the first time */
    n_records = VSwrite(apvsid, (uint8 *)data_buf1, N_RECORDS, FULL_INTERLACE);
    VERIFY_VOID(n_records, N_RECORDS, "VSwrite:apvsid");

    n_blocks = VSgetdatainfo(apvsid, 0, 0, NULL, NULL);
    CHECK_VOID(n_blocks, FAIL, "VSgetdatainfo");

    /* Allocate space to record the vdata's data info */
    alloc_info(&vs_info, n_blocks);

    n_blocks = VSgetdatainfo(apvsid, 0, n_blocks, vs_info.offsets, vs_info.lengths);
    CHECK_VOID(n_blocks, FAIL, "VSgetdatainfo");
    free_info(&vs_info);

    vs1_ref = VSQueryref(apvsid);
    CHECK_VOID(vs1_ref, FAIL, "VSQueryref:apvsid");

    {
	int32 vd2_ref;

	for (rec_num = 0; rec_num < N_RECORDS; rec_num++)
	    data_buf0[rec_num][0] = 10 + rec_num;

	/* Create and write to another very simple vdata */
	vd2_ref = VHstoredata(fid, "Field 1", (const uint8 *)data_buf0, N_RECORDS, DFNT_INT32, "Another One Field One Order", "Very Simple Vdata");
    }

    for (rec_num = 0; rec_num < N_RECORDS; rec_num++)
    {
        data_buf1[rec_num][0] = 100 + rec_num;
        data_buf1[rec_num][1] = 200 + rec_num;
        data_buf1[rec_num][2] = 300 + rec_num;
        data_buf1[rec_num][3] = 1000 + rec_num;
        data_buf1[rec_num][4] = 1000;
        data_buf1[rec_num][5] = 6500;
    }
    /* Write the data to vdata APPENDABLE_VD the second time */
    n_records = VSwrite(apvsid, (uint8 *)data_buf1, N_RECORDS, FULL_INTERLACE);
    VERIFY_VOID(n_records, N_RECORDS, "VSwrite:apvsid");

    status = VSdetach (apvsid);
    CHECK_VOID(status, FAIL, "Vdetach");

    apvsid = VSattach (fid, vs1_ref, "w");
    CHECK_VOID(apvsid, FAIL, "VSattach");

    n_blocks = VSgetdatainfo(apvsid, 0, 0, NULL, NULL);
    CHECK_VOID(n_blocks, FAIL, "VSgetdatainfo");
    VERIFY_VOID(n_blocks, 3, "VSgetdatainfo:apvsid");

    /* Allocate space to record the vdata's data info */
    alloc_info(&vs_info, n_blocks);

    /* Record various info */
    vs_info.n_values = 30;
    vs_info.numtype = DFNT_INT32;

    n_blocks = VSgetdatainfo(apvsid, 0, n_blocks, vs_info.offsets, vs_info.lengths);
    CHECK_VOID(n_blocks, FAIL, "VSgetdatainfo");

    {
	int ii;
	int32 check_offsets[] = {294, 556, 636};
	int32 check_lengths[] = {120, 80, 40}; /* last chunk is not completely filled */
	for (ii = 0; ii < n_blocks; ii++)
	{
	    VERIFY_VOID(vs_info.offsets[ii], check_offsets[ii], "VSgetdatainfo offset");
	    VERIFY_VOID(vs_info.lengths[ii], check_lengths[ii], "VSgetdatainfo length");
	}
    }
    free_info(&vs_info);

    /* Close everything. */
    status = VSdetach (apvsid);
    CHECK_VOID(status, FAIL, "Vdetach");
    status = Vend(fid);
    CHECK_VOID(status,FAIL,"Vend");
    status = Hclose(fid);
    CHECK_VOID(status,FAIL,"Hclose");
} /* test_append_vs */


/*******************************************************************
  Name: readnoHDF_char - utility routine to read and verify character
			data without HDF4 library

  Description:
	readnoHDF_char opens the file and reads in data at the specified
	offset.  The read data is compared against the original data passed
	by caller.  If any mis-match occurs, an error message will be
	displayed but the process will continue.

  Parameters:
	char *filename	IN: name of the file
	int32 offset	IN: where to start read data
	int32 length	IN: how long to read the data

  Return value:
	SUCCEED/FAIL
  BMR - Jul 2010
********************************************************************/
intn readnoHDF_char(const char *filename, const int32 offset, const int32 length, const char *orig_buf)
{
    FILE  *fd;		/* file descriptor */
    size_t readlen=0;	/* number of bytes actually read */
    char *readcbuf;
    intn ret_value = SUCCEED;

    /* Open the file for reading without SD API */
    fd = fopen(filename, "r");
    if (fd == NULL)
    {
	fprintf(stderr, "readnoHDF_char: unable to open file %s", filename);
        ret_value = FAIL;
    }

    /* Forward to the position of the first block of data */
    if (fseek(fd, (off_t)offset, SEEK_SET) == -1)
    {
        fprintf(stderr, "readnoHDF_char: unable to seek offset %d\n",
                (int)offset);
        ret_value = FAIL;
    }

    /* Allocate buffers for SDS' data */
    readcbuf = (char *) HDmalloc(length * sizeof(char));
    if (readcbuf == NULL)
    {
	fprintf(stderr, "readnoHDF_char: allocation readcbuf failed\n");
        ret_value = FAIL;
    }

    /* Read in this block of data */
    readlen = fread((void*)readcbuf, 1, length, fd);
    if (readlen > 0)
    {
        /* Compare data read without HDF4 lib against the original buffer */
	if (HDstrncmp(readcbuf, orig_buf, readlen) != 0)
	    fprintf(stderr, "Failure: non-HDF reading got different values than written values\n   >>> written = %s\n   >>> read = %s\n", orig_buf, readcbuf);
    }
    HDfree(readcbuf);
    /* Close the file */
    if (fclose(fd) == -1)
    {
	fprintf(stderr, "readnoHDF_char: unable to close file %s", filename);
        ret_value = FAIL;
    }
    return ret_value;
}

/*******************************************************************
  Name: get_annot_datainfo - utility routine to get datainfo of
	an annotation

  Description:
	get_annot_datainfo gets access to each annotation, then attempts to 
	get the offset/length of its data.  If successful, increment the
	data info count and eventually returns that number to caller.  If
	failure occurs, simply return FAIL and all the previous data info
	will be discarded by the caller.

  Parameters:
	int32 an_id		IN: annotation ID
	ann_type annot_type	IN: type of the annotations
	int32 num_anns		IN: number of annotations of the specified type,
				    currently only 1, but will be adjusted after
				    the last parameter become arrays of strings
	t_ann_info_t *ann_info	OUT: structure that holds ann data info
	intn ann_info_num	IN: indicates where in ann_info to start
				    storing info 
	char* ann_text		IN: text of the annotation (this will be changed
				    to array of strings when time permits)

  Return value:
	The number of annotations whose data info is successfully retrieved
	and stored in ann_info or FAIL if failure occurs.
********************************************************************/
intn get_annot_datainfo(int32 an_id, ann_type annot_type, int32 num_anns, t_ann_info_t *ann_info, intn ann_info_num, char* ann_text)
{
    int32 ann_id, ann_index;
    intn status_n, ret_value = 0;

    /* Get the annotation. */
    for (ann_index = 0; ann_index < num_anns; ann_index++)
    {
	/* Get the identifier of the current annotation. */
	ann_id = ANselect (an_id, ann_index, annot_type);
	CHECK(ann_id, FAIL, "ANselect");

	/* Get annotation's data info. */
	status_n = ANgetdatainfo(ann_id, &ann_info[ann_info_num].offset, &ann_info[ann_info_num].length);
	CHECK(status_n, FAIL, "ANgetdatainfo");

	/* Store annotation text for later checking against data read from
	   the file without HDF4 library */
	HDstrcpy(ann_info[ann_info_num].anntext, ann_text);

	/* Terminate access to the current annotation. */
	status_n = ANendaccess (ann_id);
	CHECK(status_n, FAIL, "ANendaccess");

	/* Number of annotations whose datainfo is retrieved */
	ret_value++;
    }
    return(ret_value);
} /* get_annot_datainfo */


/****************************************************************************
   Name: test_annotation() - tests getting data info of annotations

   Description:
	This routine tests getting data info of file label and description,
	and data label and description.
	- add a file label and a file description
	- create a vgroup then add a data label and a data description to it
	- use the utility routine get_annot_datainfo to record the offset,
	  length, and annotation text
	- use utility routine readnoHDF_char to read the annotation text
	  without the use of the HDF4 library and verify the correctness of
	  the read data

	Todo 2: Should add more annotations so there will be multiple annots
	for an object or file.  get_annot_datainfo needs to be fixed to
	accommodate this.
   BMR - Aug 2010
 ****************************************************************************/
#define ANNOT_FILE	"datainfo_annot.hdf"   /* data file */
#define	VG_NAME		"AN Vgroup"
#define	FILE_LABEL_TXT	"General HDF objects"
#define	FILE_DESC_TXT	"This is an HDF file that contains general HDF objects"
#define	DATA_LABEL_TXT	"Common AN Vgroup"
#define	DATA_DESC_TXT	"This is a vgroup that is used to test data annotations"
static void
test_annotation()
{
    int32 fid,		/* file ID */
	  an_id, 	/* AN interface ID */
	  file_label_id, 	/* file label ID */
	  file_desc_id, 	/* file description ID */
	  data_label_id, 	/* data label ID */
	  data_desc_id, 	/* data description ID */
	  vgroup_id;	/* vgroup ID */
    uint16 vgroup_tag, vgroup_ref; /* vgroup tag/ref */
    t_ann_info_t ann_info[4];	/* temporary storage of annotation info */
    intn  status_n;	/* returned status for functions returning an intn  */
    int32 status;	/* returned status for functions returning an int32 */

    /* Create the HDF file. */
    fid = Hopen (ANNOT_FILE, DFACC_CREATE, 0);
    CHECK_VOID(fid, FAIL, "Hopen");

    /* Initialize the AN interface. */
    an_id = ANstart(fid);
    CHECK_VOID(an_id, FAIL, "ANstart");

    /* Create and write a file label. */
    file_label_id = ANcreatef(an_id, AN_FILE_LABEL);
    CHECK_VOID(file_label_id, FAIL, "ANcreatef");
    status = ANwriteann(file_label_id, FILE_LABEL_TXT, strlen (FILE_LABEL_TXT));
    CHECK_VOID(status, FAIL, "ANwriteann");

    /* Create and write a file description. */
    file_desc_id = ANcreatef(an_id, AN_FILE_DESC);
    CHECK_VOID(file_desc_id, FAIL, "ANcreatef");
    status = ANwriteann(file_desc_id, FILE_DESC_TXT, strlen (FILE_DESC_TXT));
    CHECK_VOID(status, FAIL, "ANwriteann");

    /* Create a vgroup to add annotation to it. */
    status_n = Vstart(fid);
    CHECK_VOID(status_n, FAIL, "Vstart");
    vgroup_id = Vattach(fid, -1, "w");
    CHECK_VOID(vgroup_id, FAIL, "Vattach");
    status = Vsetname (vgroup_id, VG_NAME);
    CHECK_VOID(status, FAIL, "Vsetname");

    /* Get the tag and ref number of the vgroup for ANcreate. */
    vgroup_tag = (uint16) VQuerytag (vgroup_id);
    CHECK_VOID(vgroup_tag, 0, "VQuerytag");
    vgroup_ref = (uint16) VQueryref (vgroup_id);
    CHECK_VOID(vgroup_ref, 0, "VQueryref");

    /* Add a data label to the vgroup. */
    data_label_id = ANcreate(an_id, vgroup_tag, vgroup_ref, AN_DATA_LABEL);
    CHECK_VOID(data_label_id, FAIL, "ANcreate");
    status = ANwriteann(data_label_id, DATA_LABEL_TXT, strlen(DATA_LABEL_TXT));
    CHECK_VOID(status, FAIL, "ANwriteann");

    /* Add a data description to the vgroup. */
    data_desc_id = ANcreate(an_id, vgroup_tag, vgroup_ref, AN_DATA_DESC);
    CHECK_VOID(data_desc_id, FAIL, "ANcreate");
    status = ANwriteann(data_desc_id, DATA_DESC_TXT, strlen(DATA_DESC_TXT));
    CHECK_VOID(status, FAIL, "ANwriteann");

    /* Terminate access to each annotation explicitly. */
    status_n = ANendaccess(file_label_id);
    CHECK_VOID(status_n, FAIL, "ANendaccess");
    status_n = ANendaccess(file_desc_id);
    CHECK_VOID(status_n, FAIL, "ANendaccess");
    status_n = ANendaccess(data_label_id);
    CHECK_VOID(status_n, FAIL, "ANendaccess");
    status_n = ANendaccess(data_desc_id);
    CHECK_VOID(status_n, FAIL, "ANendaccess");

    /* Teminate access to the vgroup and to the V interface. */
    status = Vdetach(vgroup_id);
    status_n = Vend(fid);

    /* Terminate access to the AN interface and close the HDF file. */
    status = ANend(an_id);
    CHECK_VOID(status, FAIL, "ANend");
    status_n = Hclose (fid);
    CHECK_VOID(status_n, FAIL, "Hclose");

    /* Open the file and read in location/size of all annotations */

    {
	int32 ann_index,	/* annotation index */
	      n_file_labels,	/* numbers of file labels */
	      n_file_descs,	/* numbers of file descs */
	      n_data_labels,	/* numbers of data labels */
	      n_data_descs;	/* numbers of data descs */
	intn  num_anns = 0, ann_info_num = 0;

	/* Open the file. */
	fid = Hopen (ANNOT_FILE, DFACC_RDONLY, 0);
	CHECK_VOID(fid, FAIL, "Hopen");

	/* Initialize the AN interface. */
	an_id = ANstart(fid);
	CHECK_VOID(an_id, FAIL, "ANstart");

	/* Get the number of data/file labels/descriptions */
	status_n = ANfileinfo (an_id, &n_file_labels, &n_file_descs,
                        &n_data_labels, &n_data_descs);
	CHECK_VOID(status_n, FAIL, "ANfileinfo");
	VERIFY_VOID(n_file_labels, 1, "ANfileinfo");
	VERIFY_VOID(n_file_descs, 1, "ANfileinfo");
	VERIFY_VOID(n_data_labels, 1, "ANfileinfo");
	VERIFY_VOID(n_data_descs, 1, "ANfileinfo");

	/* Get access to each annotation then call ANgetdatainfo to retrieve
	   the offset/length of the annotation data */

	/* Explanation of some parameters to get_annot_datainfo:
	   3rd parameter - number of annotations of the specified type,
	   currently only 1, but will adjust the last parameter to have arrays
	   of character strings when have time.
	   4th parameter - ann_info_num indicates where in ann_info to start
	   storing info */

	/* AN_DATA_LABEL */
	num_anns = get_annot_datainfo(an_id, AN_DATA_LABEL, 1, ann_info, ann_info_num, DATA_LABEL_TXT);
	CHECK_VOID(num_anns, FAIL, "get_annot_datainfo");
	VERIFY_VOID(num_anns, 1, "get_annot_datainfo");
	ann_info_num = ann_info_num + num_anns;

	/* AN_DATA_DESC */
	num_anns = get_annot_datainfo(an_id, AN_DATA_DESC, 1, ann_info, ann_info_num, DATA_DESC_TXT);
	CHECK_VOID(num_anns, FAIL, "get_annot_datainfo");
	VERIFY_VOID(num_anns, 1, "get_annot_datainfo");
	ann_info_num = ann_info_num + num_anns;

	/* AN_FILE_LABEL */
	num_anns = get_annot_datainfo(an_id, AN_FILE_LABEL, 1, ann_info, ann_info_num, FILE_LABEL_TXT);
	CHECK_VOID(num_anns, FAIL, "get_annot_datainfo");
	VERIFY_VOID(num_anns, 1, "get_annot_datainfo");
	ann_info_num = ann_info_num + num_anns;

	/* AN_FILE_DESC */
	num_anns = get_annot_datainfo(an_id, AN_FILE_DESC, 1, ann_info, ann_info_num, FILE_DESC_TXT);
	CHECK_VOID(num_anns, FAIL, "get_annot_datainfo");
	VERIFY_VOID(num_anns, 1, "get_annot_datainfo");
	ann_info_num = ann_info_num + num_anns;

	/* Terminate access to the AN interface and close the HDF file. */
	status = ANend(an_id);
	CHECK_VOID(status, FAIL, "ANend");
	status_n = Hclose (fid);
	CHECK_VOID(status_n, FAIL, "Hclose");

	/* calling readnoHDF_char to verify data without the use of HDF lib */

	for (ann_index = 0; ann_index < ann_info_num; ann_index++)
	{
	    /* Open the file with fopen, read data at the offset obtained and 
	       verify the values */
	    status_n = readnoHDF_char(ANNOT_FILE, ann_info[ann_index].offset,
		    ann_info[ann_index].length, ann_info[ann_index].anntext);
	    if (status_n == FAIL)
		fprintf(stderr, "Attempt reading data without HDF4 library failed at line %d\n", __LINE__);
	}
    }
} /* test_annotation */

/****************************************************************************
   Name: test_oneblock_ri() - tests non-linked-block images

   Description:
	This routine creates and writes data to images and verifies
        some data and data information with GRgetdatainfo.  The tests include
        the following images:
	- a simple, non-compressed image
	- three images with RLE, Skipping-Huffman, and Deflate compression
	Only the non-compressed image's values are verified against the original
	buffer.  The other three were verified by debugging, additional
	decompression code needed for further verification (Todo 2)
   BMR - Aug 2010
 ****************************************************************************/
#define IMAGE_FILE	"datainfo_images.hdf"   /* data file */
#define	NONCOMP_IMAGE	"Image with No Compression"
#define	RLE_IMAGE	"Image with RLE Compression"
#define	DEFLATE_IMAGE	"Image with Deflate Compression"
#define	SKPHUFF_IMAGE	"Image with Skphuff Compression"
#define	JPEG_IMAGE	"Image with JPEG Compression"
#define	SKPHUFF_SKIPSIZE	28
#define	DEFLATE_LEVEL	7
#define WIDTH		5
#define LENGTH		5
#define N_IMAGES	4

/* Convenient function to create and write to an image, used by
   test_oneblock_ri */
static intn make_comp_image( 
	int32 grid,
	char* img_name,
	char start_char,  /* first value in the image, for variety of data */
	int32 comp_type,  /* compression method */
	comp_info* cinfo) /* compression parameters */
{
    int32 riid;         /* raster image ID */
    int32 dims[2]={WIDTH,LENGTH};	/* dimensions for the image */
    char image0[WIDTH][LENGTH];		/* image data */
    int32 start[2];	/* start of image data to grab */
    int32 stride[2];	/* stride of image data to grab */
    intn ii, jj;	/* indices */
    intn status;        /* generic return value */
    intn ret_value=SUCCEED;

    /* Initialize data we are going to write out, each image created by this
       convenient function will use the same data pattern with the first value
       given by the caller  */
    for (ii = 0; ii < WIDTH; ii++)
        for (jj = 0; jj < LENGTH; jj++)
            image0[ii][jj] = start_char + jj;

    /* Create the image with 1 component, type char, pixel interlace, and
       dimension WIDTHxLENGTH */
    riid = GRcreate(grid, img_name, 1, DFNT_CHAR, MFGR_INTERLACE_PIXEL, dims);
    CHECK(riid, FAIL, "GRcreate");

    /* Set compression as instructed */
    if (comp_type != COMP_CODE_NONE)
    {
	status = GRsetcompress(riid, comp_type, cinfo);
	CHECK(status, FAIL, "GRsetcompress");
    }

    /* Write the entire image data out */
    start[0] = start[1] = 0;
    stride[0] = stride[1] = 1;
    status = GRwriteimage(riid, start, stride, dims, image0);
    CHECK(status, FAIL, "GRreadimage");

    /* Close the first image */
    status = GRendaccess(riid);
    CHECK(status, FAIL, "GRendaccess");

    return ret_value;
}

static void
test_oneblock_ri(void)
{
    int32 fid, grid,	/* file ID and GR interface ID */
	  riid;		/* raster image ID */
    int32 offset, length; /* offset/length buffers for single block of data */
    intn  status;	/* status returned from routines */
    intn ii;		/* indices */
    int32 n_images, n_fattrs;
    comp_info cinfo;    /* Compression parameters - union */
    /* offsets/lengths to be used to verify offsets/lengths returned by
       GRgetdatainfo - confirmed by the command "od" on the hdf file */
    static int32 image_data_offsets[N_IMAGES] = {309, 363, 426, 486};
    static int32 image_data_lengths[N_IMAGES] = {25, 26, 29, 16};

    /****************************************************************
      Create a file in GR interface then create and write several
      images with and without compression.
    ****************************************************************/

    /* Create the HDF file and initialize the interface. */
    fid = Hopen(IMAGE_FILE, DFACC_CREATE, 0);
    CHECK_VOID(fid, FAIL, "Hopen");

    grid = GRstart(fid);
    CHECK_VOID(grid, FAIL, "GRstart");

    /* Create and write the non-compressed image to this file */
    status = make_comp_image(grid, NONCOMP_IMAGE, 'n', COMP_CODE_NONE, &cinfo);

    /* Create and write 3 more images: RLE, Deflate, and Skipping Huffman */

    /* No compression info for the RLE image */
    HDmemset(&cinfo, 0, sizeof(cinfo)) ;

    /* Create and write the RLE compressed image to this file, starting the
       data values with the letter 'r' */
    status = make_comp_image(grid, RLE_IMAGE, 'r', COMP_CODE_RLE, &cinfo);

    /* Set the compression info for the image with Skipping Huffman method */
    HDmemset(&cinfo, 0, sizeof(cinfo)) ;
    cinfo.skphuff.skp_size = SKPHUFF_SKIPSIZE;

    /* Create and write the Skipping Huffman compressed image to this file,
       starting the data values with the letter 's' */
    status = make_comp_image(grid, SKPHUFF_IMAGE, 's', COMP_CODE_SKPHUFF, &cinfo);

    /* Set the compression info for the image with Deflate method */
    HDmemset(&cinfo, 0, sizeof(cinfo)) ;
    cinfo.deflate.level = DEFLATE_LEVEL;

    /* Create and write the Deflate compressed image to this file, starting the
       data values with the letter 'd' */
    status = make_comp_image(grid, DEFLATE_IMAGE, 'd', COMP_CODE_DEFLATE, &cinfo);

    /* Set the compression method for the image with JPEG method */
    HDmemset(&cinfo, 0, sizeof(cinfo)) ;
    cinfo.jpeg.quality = 100;
    cinfo.jpeg.force_baseline = 1;

    /* Create and write the JPEG compressed image to this file, starting the
       data values with the letter 'j' - more work to be done for JPEG */
     /* status = make_comp_image(grid, JPEG_IMAGE, 'j', COMP_CODE_JPEG, &cinfo);
 */ 

    /* Terminate access to the GR interface and close the file */
    status = GRend(grid);
    CHECK_VOID(status, FAIL, "GRend");
    status = Hclose(fid);
    CHECK_VOID(status, FAIL, "Hclose");

    /****************************************************************
      Re-open the file to read the images and their data information
    ****************************************************************/

   /* Re-open the file and initialize the GR interface */
    fid = Hopen (IMAGE_FILE, DFACC_RDONLY, 0);
    CHECK_VOID(fid, FAIL, "Hopen");
    grid = GRstart (fid);
    CHECK_VOID(grid, FAIL, "GRstart");

    /* Get the number of images in the file */
    status = GRfileinfo(grid, &n_images, &n_fattrs);
    CHECK_VOID(status, FAIL, "GRfileinfo");
    VERIFY_VOID(n_images, N_IMAGES, "GRfileinfo");

    /* Open each image then get and verify its data information.  Note that
	currently, the offsets and lengths are obtained from debugging
	and the command od on the file */
    for (ii = 0; ii < n_images; ii++)
    {
	riid = GRselect(grid, ii);
	CHECK_VOID(riid, FAIL, "GRselect");

	status = GRgetdatainfo(riid, 0, 1, &offset, &length);
	CHECK_VOID(status, FAIL, "GRgetdatainfo");
	VERIFY_VOID(offset, image_data_offsets[ii], "GRgetdatainfo");
	VERIFY_VOID(length, image_data_lengths[ii], "GRgetdatainfo");

	/* Only verify data of the first image, which has non-compressed data. */
	if (ii == 0)
	{
	    intn jj, kk;
	    char check_image[WIDTH][LENGTH];
	    for (kk = 0; kk < WIDTH; kk++)
		for (jj = 0; jj < LENGTH; jj++)
		    check_image[kk][jj] = 'n' + jj;

	    /* Open the file with fopen, read data at the offset obtained and verify
	       the values */
	    status = readnoHDF_char(IMAGE_FILE, offset, length, check_image);
	    if (status == FAIL)
		fprintf(stderr, "Attempt reading data without HDF4 library failed at line %d\n", __LINE__);
	}

	/* Close the image */
	status = GRendaccess(riid);
	CHECK_VOID(status, FAIL, "GRendaccess");
    } /* for n_images */

    /* Terminate access to the GR interface and close the file */
    status = GRend(grid);
    CHECK_VOID(status, FAIL, "GRend");
    status = Hclose(fid);
    CHECK_VOID(status, FAIL, "Hclose");
} /* end test_oneblock_ri */

#define IMAGE_DF_FILE	"datainfo_dfri.hdf"	/* data file for DFR APIs */
#define N_DF_IMAGES	2	/* number of DF images in the file, 1 RI8 &
				   1 RI24 */
static void
test_dfr8_24(void)
{
    int32 fid, grid,	/* file ID and GR interface ID */
	  riid;		/* raster image ID */
    int32 offset, length; /* offset/length buffers for single block of data */
    intn  status;	/* status returned from routines */
    intn ii, jj;	/* indices */
    int32 n_images, n_fattrs;
    char  buf[WIDTH][LENGTH][3];

    /* offsets/lengths to be used to verify offsets/lengths returned by
       GRgetdatainfo - confirmed by the command "od" on the hdf file */
    static int32 image_data_offsets[N_IMAGES] = {294, 1132};
    static int32 image_data_lengths[N_IMAGES] = {30, 75};

    /* Initialize the 8-bit image array */
    static uint8 raster_data[WIDTH][LENGTH] =
	{ 1, 2, 3, 4, 5,
	  5, 4, 3, 2, 1,
	  1, 2, 3, 4, 5,
	  5, 4, 3, 2, 1,
	  6, 4, 2, 0, 2 };

   /* Write the 8-bit raster image to file */
   status = DFR8putimage(IMAGE_DF_FILE, raster_data, WIDTH, LENGTH, COMP_RLE);

    /* Initialize the 24-bit image array */
    for (ii = 0; ii < WIDTH; ii++)
	for (jj = 0; jj < LENGTH; jj++)
	{
	    buf[ii][jj][0] = buf[ii][jj][1] = buf[ii][jj][2] = (char)(ii + jj);
	}

    /* Set interlace for the 24-bit RI */
    status = DF24setil(DFIL_PIXEL);

    /* Make sure that no compression is being used - the variable Grcompr is
       global in dfgr.c so its value is inherited from other tests in the lib
       and causes this image to be compressed with JPEG unintentionally.  We
       want to be able to see the data to verify it with "od" */
    status = DF24setcompress(COMP_NONE, NULL);

    /* Write the 24-bit raster image to file */
    status = DF24addimage(IMAGE_DF_FILE, &(buf[0][0][0]), WIDTH, LENGTH);


    /****************************************************************
      Re-open the file to read the images and their data information
     ****************************************************************/
    /* Re-open the file and initialize the GR interface */
    fid = Hopen (IMAGE_DF_FILE, DFACC_RDONLY, 0);
    CHECK_VOID(fid, FAIL, "Hopen");
    grid = GRstart (fid);
    CHECK_VOID(grid, FAIL, "GRstart");

    /* Get the number of images in the file */
    status = GRfileinfo(grid, &n_images, &n_fattrs);
    CHECK_VOID(status, FAIL, "GRfileinfo");
    VERIFY_VOID(n_images, N_DF_IMAGES, "GRfileinfo");

    /* Open each image then get and verify its data information.  Note that
	currently, the offsets and lengths are obtained from debugging
	and the command od on the file */
    for (ii = 0; ii < n_images; ii++)
    {
	uintn info_count = 0;

	riid = GRselect(grid, ii);
	CHECK_VOID(riid, FAIL, "GRselect");

	/* Get the number of data blocks and verify; should be 1 */
	info_count = GRgetdatainfo(riid, 0, 0, NULL, NULL);
	CHECK_VOID(status, FAIL, "GRgetdatainfo");
	VERIFY_VOID(info_count, 1, "GRgetdatainfo");

	/* Get offset/length of the image and verify with pre-determined
	   values */
	status = GRgetdatainfo(riid, 0, info_count, &offset, &length);
	CHECK_VOID(status, FAIL, "GRgetdatainfo");
	VERIFY_VOID(offset, image_data_offsets[ii], "GRgetdatainfo");
	VERIFY_VOID(length, image_data_lengths[ii], "GRgetdatainfo");

	/* Close the image */
	status = GRendaccess(riid);
	CHECK_VOID(status, FAIL, "GRendaccess");
    } /* for n_images */

    /* Terminate access to the GR interface and close the file */
    status = GRend(grid);
    CHECK_VOID(status, FAIL, "GRend");
    status = Hclose(fid);
    CHECK_VOID(status, FAIL, "Hclose");
}  /* test_dfr8 */


/****************************************************************************
   Name: test_getntinfo() - tests getting number type's information

   Description:
	This routine simply calls Hgetntinfo with various types and verifies
	the information retrieved.
   BMR - Aug 2010
 ****************************************************************************/
static void
test_getntinfo(void)
{
    hdf_ntinfo_t nt_info;
    intn status = SUCCEED;

    status = Hgetntinfo(DFNT_UINT8, &nt_info);
    CHECK_VOID(status, FAIL, "Hgetntinfo DFNT_UINT8");
    VERIFY_CHAR_VOID(nt_info.type_name, "uint8", "Hgetntinfo DFNT_UINT8");
    VERIFY_CHAR_VOID(nt_info.byte_order, "bigEndian", "Hgetntinfo DFNT_UINT8");
    
    status = Hgetntinfo(DFNT_CHAR16, &nt_info);
    CHECK_VOID(status, FAIL, "Hgetntinfo DFNT_CHAR16");
    VERIFY_CHAR_VOID(nt_info.type_name, "char16", "Hgetntinfo DFNT_CHAR16");
    VERIFY_CHAR_VOID(nt_info.byte_order, "bigEndian", "Hgetntinfo DFNT_CHAR16");
    
    /* Native */
    status = Hgetntinfo(DFNT_NFLOAT32, &nt_info);
    CHECK_VOID(status, FAIL, "Hgetntinfo DFNT_NFLOAT32");
    VERIFY_CHAR_VOID(nt_info.type_name, "float32", "Hgetntinfo DFNT_NFLOAT32");
    VERIFY_CHAR_VOID(nt_info.byte_order, "bigEndian", "Hgetntinfo DFNT_NFLOAT32");
    
    /* Little endian */
    status = Hgetntinfo(DFNT_LFLOAT32, &nt_info);
    CHECK_VOID(status, FAIL, "Hgetntinfo DFNT_LFLOAT32");
    VERIFY_CHAR_VOID(nt_info.type_name, "float32", "Hgetntinfo DFNT_LFLOAT32");
    VERIFY_CHAR_VOID(nt_info.byte_order, "littleEndian", "Hgetntinfo DFNT_LFLOAT32");
    
    /* Little endian backward compatible */
    status = Hgetntinfo(DFNT_LCHAR, &nt_info);
    CHECK_VOID(status, FAIL, "Hgetntinfo DFNT_LCHAR");
    VERIFY_CHAR_VOID(nt_info.type_name, "char8", "Hgetntinfo DFNT_LCHAR");
    VERIFY_CHAR_VOID(nt_info.byte_order, "littleEndian", "Hgetntinfo DFNT_LCHAR");
    
    /* Backward compatible */
    status = Hgetntinfo(DFNT_DOUBLE, &nt_info);
    CHECK_VOID(status, FAIL, "Hgetntinfo DFNT_DOUBLE");
    VERIFY_CHAR_VOID(nt_info.type_name, "float64", "Hgetntinfo DFNT_DOUBLE");
    VERIFY_CHAR_VOID(nt_info.byte_order, "bigEndian", "Hgetntinfo DFNT_DOUBLE");
    
    /* Native backward compatible */
    status = Hgetntinfo(DFNT_NUCHAR, &nt_info);
    CHECK_VOID(status, FAIL, "Hgetntinfo DFNT_NUCHAR");
    VERIFY_CHAR_VOID(nt_info.type_name, "uchar8", "Hgetntinfo DFNT_NUCHAR");
    VERIFY_CHAR_VOID(nt_info.byte_order, "bigEndian", "Hgetntinfo DFNT_NUCHAR");
    
} /* test_getntinfo */

/* Test driver for testing the public functions VSgetdatainfo, ANgetdatainfo. */
void
test_datainfo(void)
{
    /* Test VSgetdatainfo with data stored in one contiguous block */
    test_simple_vs();

    /* Test VSgetdatainfo with data might be stored in linked blocks */
    test_append_vs();

    /* Test ANgetdatainfo */
    test_annotation();

    /* Test GRgetdatainfo with images stored in one contiguous block, with
       and without compression */
    test_oneblock_ri();

    /* Test GRgetdatainfo with RI8 and RI24 */
    test_dfr8_24();

    /* Test Hgetntinfo */
    test_getntinfo();
}