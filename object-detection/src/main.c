#include "main.h"

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

void initialize_network(char *cfg_file, char *weight_file){
	net = load_network_custom(cfg_file, weight_file,1,1);
    
	set_batch_network(net, 1);
    
	fuse_conv_batchnorm(*net);
	calculate_binary_weights(*net);
}

int main(int argc, char *argv[]) {

	if(argc > 1){
		chdir(argv[1]);
	}

	if(argc > 3){
		image_width = atoi(argv[2]);
		image_height = atoi(argv[3]);
	}

	char cwd[100];
	getcwd(cwd,sizeof(cwd));
	printf("Current working dir: %s\n", cwd);

	char result[ 100 ];
	readlink( "/proc/self/exe", result, sizeof(result)-1);
	printf("readlink dir: %s\n", result);

	
	// Path to configuration file.
	//static char *cfg_file = "../data/yolov3.cfg";
	//static char *cfg_file = "../data/yolov3-tiny.cfg";
	static char *cfg_file = "../data/enet-coco.cfg";

	// Path to weight file.
	//static char *weight_file = "../data/yolov3.weights";
	//static char *weight_file = "../data/yolov3-tiny.weights";    
	static char *weight_file = "../data/enetb0-coco_final.weights";   
	
	// Path to a file describing classes names.
	static char *names_file = "../data/coco.names";
    
	size_t classes = 0;
	names = get_labels(names_file);
	while (names[classes] != NULL) {
		classes++;
	}
        
	initialize_network(cfg_file ,weight_file );

	char cap_str[200];

	sprintf(cap_str, "shmsrc socket-path=/dev/shm/camera_small ! video/x-raw, format=BGR, width=%i, height=%i, framerate=30/1 ! videoconvert ! video/x-raw, format=BGR ! appsink drop=true",image_width,image_height);

	cap = get_capture_video_stream(cap_str);
	
	if (!cap) {
		error("Couldn't connect to webcam.\n");
	}
	
	in_s = get_image_from_stream_resize(cap, 416, 416, 3, &in_img, 0);	
	
	//pthread_t fetch_thread;
	//pthread_t detect_thread;

	fd_set readfds;
	FD_ZERO(&readfds);

	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	char message[50];
	 		 	
	init_trackers(classes);
		
	//create_window_cv("Demo", 0, 1280, 720);
	
	
	while (1){

		// Time at start. Later needed for fps control
		before = get_time_point();
			
		// check stdin for a new maxFPS amount
		FD_SET(STDIN_FILENO, &readfds);

		if (select(1, &readfds, NULL, NULL, &timeout)) 	{
			scanf("%s", message);
			maxFPS = atof(message);
			minFrameTime = 1000000.0 / maxFPS;
		}

		// if no FPS are needed and maxFPS equals 0, wait and start from the beginning
		if (maxFPS == 0) {
			usleep(1 * 1000000);
			printf("{\"OBJECT_DET_FPS\": 0.0}\n");
			fflush(stdout);
			continue;
		}
	

		/* ----------------------------------------------------------
		/  The needed reassignment for threading
		/  ---------------------------------------------------------- */

		// fetched image to detection image
		det_s = in_s; 
		local_dets = dets; // detection of the last iteration is now processed by trackers
		local_nboxes = nboxes;


		/* ------------------------------------------------------------------------------------
		/  
		/  ------------------------------------------------------------------------------------ */
		fetch_image(0);
		get_results(0);
				
		//if(pthread_create(&detect_thread, 0, get_results, 0)) error("Thread creation failed");
		//if(pthread_create(&fetch_thread, 0, fetch_image, 0)) error("Thread creation failed");
		
		updateTrackers(local_dets, local_nboxes, thresh, &tracked_dets, &tracked_nboxes,image_width, image_height);	
		
		//char* det_json = detection_to_json(dets, num_boxes, classes, names, 0 , "");
		//printf(det_json);
		
		//draw_detections_cv_v3(in_img, local_dets, local_nboxes, thresh, names, NULL, classes, 0);
		//show_image_mat(in_img, "Demo");	
		//int c = wait_key_cv(1);
		//if (c == 27 || c == 1048603) flag_exit = 1;
		
		
		release_mat(&in_img);

		fflush(stdout);
		
		printf("{\"DETECTED_OBJECTS\": [");
		
		if (tracked_nboxes > 0){
			char itemstring[200];
			printf("{\"TrackID\": %li, \"name\": \"%s\", \"center\": [%.5f,%.5f], \"w_h\": [%.5f,%.5f]}", tracked_dets[0].trackerID, names[tracked_dets[0].objectTyp], tracked_dets[0].bbox.x, tracked_dets[0].bbox.y , tracked_dets[0].bbox.w , tracked_dets[0].bbox.h);
			
			int i=1;
			
			for(i=1;i<tracked_nboxes;i++){
				printf(", {\"TrackID\": %li, \"name\": \"%s\", \"center\": [%.5f,%.5f], \"w_h\": [%.5f,%.5f]}", tracked_dets[i].trackerID, names[tracked_dets[i].objectTyp], tracked_dets[i].bbox.x, tracked_dets[i].bbox.y , tracked_dets[i].bbox.w , tracked_dets[i].bbox.h);
				
			}

			tracked_nboxes = 0;
			free(tracked_dets);

		}
		
		printf("]}\n");
		fflush(stdout);
			
		free_detections(local_dets, local_nboxes);



		/* ------------------------------------------------------------------------------------
		/  Regulate the maximum frames per secound 
		/  The wanted fps amount is set in maxFPS.
		/  Check is the minFrameTime is bigger than the needed time,
		/  if true it can be waited for, because we do not need to be faster than that.
		   ------------------------------------------------------------------------------------ */
		
		double after = get_time_point();    // more accurate time measurements
		double curr_usec = (after - before);

		if (curr_usec < minFrameTime){
			usleep(minFrameTime - curr_usec);
			curr_usec = minFrameTime;
		}

		framecounteracc += curr_usec;
		framecounter += 1;


		/* The achieved fps are printed out once a second. */
		if (framecounter > maxFPS){
			printf("{\"OBJECT_DET_FPS\": %.2f}\n", (1000000. / (framecounteracc / framecounter)));
			fflush(stdout);
			framecounteracc = 0.0;
			framecounter = 0;
		}
	

		if (flag_exit == 1) break;   
	}
}
