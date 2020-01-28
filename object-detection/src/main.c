#define OPENCV 1

#include <stdio.h>
#include <darknet.h>
#include "utils.h"
#include <sys/time.h>
#include "option_list.h"
#include "parser.h"
#include <unistd.h>

#include "MYSort.h"

static char **demo_names;
static image **demo_alphabet;
static int demo_classes;

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
static float thresh 		= .6;
static int ext_output 		= 0;
static int json_port 		= -1;
static char *prefix 		= 0;
static float hier_thresh 	= .5;
static int mjpeg_port 		= -1;
static int classes 		= 80;
char **names 			= NULL;

static double maxFPS = 30.;
static int framecounter = 0;
static double framecounteracc = 0.0; 

#define NFRAMES 3

static mat_cv* cv_images[NFRAMES];

static volatile int image_index = 0;

static mat_cv* in_img;

static volatile int flag_exit = 0;
static int letter_box = 0;

void* fetch_image(void *ptr){
	in_s = get_image_from_stream_resize(cap, 416, 416, 3, &in_img, 0);	
}

void* get_results(void *ptr){
	
	network_predict_image(net,det_s);
				
	dets = get_network_boxes(net, in_s.w, in_s.h, thresh, hier_thresh, 0, 1, &nboxes,0);			
		
	float nms = .60;    // 0.4F
	if (nms) do_nms_sort(dets, nboxes, classes, nms);
	
	free_image(det_s);
}

int main() {
	
	// Path to configuration file.
    static char *cfg_file = "../data/yolov3.cfg";
    // Path to weight file.
    static char *weight_file = "../data/yolov3.weights";
    // Path to a file describing classes names.
    static char *names_file = "../data/coco.names";
    
    size_t classes = 0;
	names = get_labels(names_file);
	while (names[classes] != NULL) {
         classes++;
    }
        
    net = load_network_custom(cfg_file, weight_file,1,1);
    
    set_batch_network(net, 1);
    
    fuse_conv_batchnorm(*net);
    calculate_binary_weights(*net);
    srand(2222222); 
    
	
	cap = get_capture_video_stream("shmsrc socket-path=/dev/shm/camera_small ! video/x-raw, format=BGR, width=416, height=416, framerate=30/1 ! videoconvert ! video/x-raw, format=BGR ! appsink drop=true");
	
		if (!cap) {
			error("Couldn't connect to webcam.\n");
		}
	
	in_s = get_image_from_stream_resize(cap, 416, 416, 3, &in_img, 0);	
	
	//pthread_t fetch_thread;
	//pthread_t detect_thread;
	 	
	 	
	init_trackers(classes);
		
	//create_window_cv("Demo", 0, 1280, 720);
	
	double before = get_time_point();
	 
	while (1){
		
	
		det_s = in_s;
		local_dets = dets;
		local_nboxes = nboxes;

		//in_s = get_image_from_stream_resize(cap, 416, 416, 3, &in_img, 0);

		fetch_image(0);
		get_results(0);
				
		//if(pthread_create(&detect_thread, 0, get_results, 0)) error("Thread creation failed");
		//if(pthread_create(&fetch_thread, 0, fetch_image, 0)) error("Thread creation failed");
		
		//const char *CLEAR_SCREEN_ANSI = "\e[1;1H\e[2J";
		//write(STDOUT_FILENO, CLEAR_SCREEN_ANSI, 12);
		
		updateTrackers(local_dets, local_nboxes, thresh, &tracked_dets, &tracked_nboxes);	
		
		//char* det_json = detection_to_json(dets, num_boxes, classes, names, 0 , "");
		//printf(det_json);
		
		//draw_detections_cv_v3(in_img, local_dets, local_nboxes, thresh, names, NULL, classes, 0);
		//show_image_mat(in_img, "Demo");	
		//int c = wait_key_cv(1);
		//if (c == 27 || c == 1048603) flag_exit = 1;
		
		
		release_mat(&in_img);
		
		printf("{\"DETECTED_OBJECTS\": [");
		
		if (tracked_nboxes > 0){
			char itemstring[200];
			printf("{\"TrackID\": %li, \"name\": %s, \"center\": [%f,%f], \"w_h\": [%f,%f]}", tracked_dets[0].trackerID, names[tracked_dets[0].objectTyp], tracked_dets[0].bbox.x, tracked_dets[0].bbox.y , tracked_dets[0].bbox.h , tracked_dets[0].bbox.w);
			
			int i=1;
			
			for(i=1;i<tracked_nboxes;i++){
				printf(", {\"TrackID\": %li, \"name\": %s, \"center\": [%f,%f], \"w_h\": [%f,%f]}", tracked_dets[i].trackerID, names[tracked_dets[i].objectTyp], tracked_dets[i].bbox.x, tracked_dets[i].bbox.y , tracked_dets[i].bbox.h , tracked_dets[i].bbox.w);
				
			}

			tracked_nboxes = 0;
			free(tracked_dets);

		}
		
		printf("]}\n");
			
		free_detections(local_dets, local_nboxes);

		double minFrameTime = 1.0 / maxFPS;			
		double after = get_time_point();    // more accurate time measurements
		double curr = 1000000. / (after - before);
		if (maxFPS < curr){
			double sleepingTime = (1.0/maxFPS)-(1.0/curr);
			sleep( sleepingTime);
			curr = maxFPS;
			after = get_time_point();
		}
		framecounteracc += curr;
		before = after;
		
		framecounter += 1;

		if (framecounter > maxFPS){

			printf("{\"OBJECT_DET_FPS\": %.1f}\n", (framecounteracc / framecounter));
			framecounteracc = 0.0;
			framecounter = 0;

		}
		
		
		
		
		if (flag_exit == 1) break;   
	}
	
}
