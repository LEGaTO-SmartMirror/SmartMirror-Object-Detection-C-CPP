#define OPENCV 1

#include <stdio.h>
#include <darknet.h>
#include "utils.h"
#include <sys/time.h>
#include "option_list.h"
#include "parser.h"
#include <unistd.h>
#include <time.h>

#include "MYSort.h"

static int image_width = 416;
static int image_height = 416;

static int nboxes = 0;
static int local_nboxes = 0;
static int tracked_nboxes = 0;
static detection *dets = NULL;
static detection *local_dets = NULL;
static TrackedObject *tracked_dets = NULL;

static network* net;
static image in_s;
static image det_s;

static cap_cv *cap;
static float fps 		= 0;
static float thresh 		= .4;
static float hier_thresh 	= .4;
static int classes 		= 80;
char **names 			= NULL;

static double maxFPS = 5.;
double minFrameTime = 1000000.0 / 5.;
static int framecounter = 0;
static double framecounteracc = 0.0; 

#define NFRAMES 3

static mat_cv* cv_images[NFRAMES];

static volatile int image_index = 0;

static mat_cv* in_img;

static volatile int flag_exit = 0;
static int letter_box = 0;

double before = 0.0;

void* fetch_image(void *ptr);
void* get_results(void *ptr);
void initialize_network(char *cfg_file, char *weight_file);
int main(int argc, char *argv[]);
