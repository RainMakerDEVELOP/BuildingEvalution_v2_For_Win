/***************************************************************************
 * mseed2ascii.c
 *
 * Convert miniSEED waveform data to ASCII
 *
 * Written by Chad Trabant, IRIS Data Management Center
 *
 * modified 2017.093
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <math.h>

#include "../Include/libmseed.h"

#define VERSION "2.1"
#define PACKAGE "mseed2ascii"

struct listnode {
  char *key;
  char *data;
  struct listnode *next;
};

class CMseed2Gascii
{
public:
	CMseed2Gascii();
	~CMseed2Gascii();

	int MakeMseed2Gascii (char* intputFile, char* outputFile, double dwCalib);
	int64_t writeascii (MSTrace *mst);
	//int parameter_proc (int argcount, char **argvec);
	int parameter_proc (char* sFileName);
	char *getoptval (int argcount, char **argvec, int argopt, int dasharg);
	int readlistfile (char *listfile);
	struct listnode *addnode (struct listnode **listroot, void *key, int keylen,
		void *data, int datalen);
	void usage (void);

	int    verbose;//      = 0;    /* Verbosity level */
	int    reclen;//       = -1;   /* Record length, -1 = autodetected */
	int    deriverate;//   = 0;    /* Use sample rate derived instead of the reported rate */
	int    indifile;//     = 0;    /* Individual file processing flag */
	char  unitsstr[32];//     = "Counts"; /* Units to write into output headers */
	char  *outputfile;//   = 0;    /* Output file name for single file output */
	FILE  *ofp;//          = 0;    /* Output file pointer */
	int    outformat;//    = 3;    /* Output file format */
	int    slistcols;//    = 1;    /* Number of columns for sample list output */
	double timetol;//      = -1.0; /* Time tolerance for continuous traces */
	double sampratetol;//  = -1.0; /* Sample rate tolerance for continuous traces */
	double g_dwCalibration;//	   = 0.0003110039;

	/* A list of input files */
	struct listnode *filelist;// = 0;
protected:
private:
};