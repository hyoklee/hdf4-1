#ifdef RCSID
static char RcsId[] = "@(#)$Revision$";
#endif
/*
$Header$

$Log$
Revision 1.1  1992/06/09 17:36:25  mfolk
Initial revision

*/
/*****************************************************************************
*
*                         NCSA HDF version 3.2beta
*                            February 29, 1992
*
* NCSA HDF Version 3.2 source code and documentation are in the public
* domain.  Specifically, we give to the public domain all rights for future
* licensing of the source code, all resale rights, and all publishing rights.
*
* We ask, but do not require, that the following message be included in all
* derived works:
*
* Portions developed at the National Center for Supercomputing Applications at
* the University of Illinois at Urbana-Champaign.
*
* THE UNIVERSITY OF ILLINOIS GIVES NO WARRANTY, EXPRESSED OR IMPLIED, FOR THE
* SOFTWARE AND/OR DOCUMENTATION PROVIDED, INCLUDING, WITHOUT LIMITATION,
* WARRANTY OF MERCHANTABILITY AND WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE
*
*****************************************************************************/

/* This program converts a series raster image hdf files into   */
/* a single 3D sds hdf file. Each ris hdf file contains one 	*/
/* or more raster images. All images should be of the same      */
/* dimension sizes. And, if there is a palette associated to    */
/* the images, the palette should be included in the first      */
/* ris file. Any subsequent palettes will be ignored.           */
/* Sept. 23, 1991						*/

/*   USAGE: ristosds infile{ infile} -o outfile			*/


#include "hdf.h"

#define r_imgae 2;
#define r_sds   3;
#define USAGE "ristosds infile{ infile} -o outfile"

int main
  PROTO((int, char **));
int cntimage
  PROTO((char *filename, int *p_w, int *p_h, int *n_images));


#ifdef PROTOTYPE
main(int argc, char *argv[])
#else
main(argc, argv)
    int argc;
    char *argv[];
#endif PROTOTYPE
{

    int i,j;
    int nimg, nimg0, w, h;  /* nimg, nimg0 -- number of images	*/
			    /* w, h -- width and height of images */
    int  ret, n_infile, getoutfile, ispal, dimsizes[3];
    char *infile, *outfile, **argv_infile, *indata0, *indata, palette[768];
    float32 *outdata, *outdata0;


    if (argc < 4) {
	printf("Usage %s.\n", USAGE);
	finishing();
    }
    /* initialization	*/

    nimg = 0;
    w = h = 0;
    n_infile = 0;		/* count number of input files   */	
    getoutfile = 0;
    argv_infile = &argv[1];	/* save start address of input file names. */

    /* Count total number of images to be converted.   */

    while (--argc > 1)   	{   /* reach -o. go to get output file name */
	infile = *++argv;
	if (infile[0] == '-')   {
	    getoutfile = 1;
	    break;
	}
	else			{
            n_infile++;
	    ret = cntimage(infile, &w, &h, &nimg);
	    if (ret != 0) finishing();
	}
    }

    nimg0 = nimg;
    printf("Total number of images: %d.\n", nimg);    

    /* get output file name     */

    argc--;

    if (getoutfile == 0 || argc < 1)  	{   /* is -o or outfile missing?    */
	printf("Bad commend line. \n\n\t\t %s\n", USAGE);
	finishing();
    }

    outfile = *++argv;
    
    /* read in images from all input files.  	*/

    outdata0 = outdata = (float32 *)malloc(nimg*w*h*(sizeof(float32)));
    indata0 = indata = (char *)malloc(w*h*sizeof(char));
    infile = *argv_infile;
    ret = DFR8getdims(infile, &w, &h, &ispal);
    if (ispal) {
	DFPgetpal(infile, palette);
 	DFR8restart();		/* in case the palette is not at the  */
				/* beginning of the first input file  */
   	DFPputpal(outfile,palette, 0, "a");
    }
    while (nimg > 0)	{
	indata = indata0;	/* Restart from the beginning of the buf */
	ret = DFR8getimage(infile, indata, w, h, palette);
	if (ret != 0 )	{	/* end of file. open next one   */
	 n_infile--;
	    if (n_infile < 1) 	{
		printf("Inconsistent number of files and images\n");
		finishing();
	    }
	    infile = *++argv_infile;
	    ret = DFR8getimage(infile, indata, w, h, palette);
    	    if (ret != 0) finishing();
	}
	
	/* convert image data into floating point and store in the array  */

	for (i=0; i<w; i++)
	    for (j=0; j<h; j++) *outdata++ = (float32) *indata++;
	nimg--;
    }

    dimsizes[0] = nimg0;
    dimsizes[1] = w;
    dimsizes[2] = h;
    ret = DFSDadddata(outfile, 3, dimsizes, outdata0);
    if (ret != 0) finishing();
    exit(0);
}



int finishing()	
{
    printf("end of ristosds.\n");
    exit(1);
} 


/* count # of images  */
 	    
#ifdef PROTOTYPE
int cntimage(char *filename, int *p_w, int *p_h, int *n_images);
#else
int cntimage(filename, p_w, p_h, n_images)  
    char *filename;		
    int *p_w, *p_h, *n_images;  /* p_w -- pointer to width   */
#endif PROTOTYPE

{     int ret, width, height, ispal, dimerror;

	dimerror = 0;

	ret = DFR8getdims(filename,&width, &height, &ispal);	
	if (ret == FAIL)	{
	    HEprint(stderr, 0);
	    finishing();
	}
	if (*p_w == 0 && *p_h == 0)	{	/* the first time  */
	   *p_w = width;
	   *p_h = height;
	}
	while (ret == 0)	{   /* count # of images and check dims    */
	    if ((width != *p_w) || (height != *p_h))	{
		printf("Inconsistent dims: %s .\n", filename);
		dimerror = -1;
		break;
	    } 
	    *n_images = *n_images + 1;
	    ret = DFR8getdims(filename, &width, &height, &ispal);
	}

	/* ready to return   */

	DFR8restart();	
	if (dimerror == -1) return(-1);
	else 	return(0);
}
