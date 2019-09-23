#ifndef MYSORT_H
#define MYSORT_H

#include "KalmanTrackerWrapper.h"
#include "Hungarian.h"
#include <stdio.h>
#include <darknet.h>


//struct KalmanTracker* tracker;
static struct KalmanTracker*** trackers;
static size_t tracker_types;
static size_t* tracker_amount;

static size_t max_age = 10;
static double iouThreshold = 0.3;
static double TrackerIOUsimThreshhold = 0.8;

// for an efficent tracking all detecitons needs to be sorted for object types.
// detection[tracker_types][dets_sorted_number[tracker_types]]
static detection*** dets_sorted;
static size_t* dets_sorted_number;

// get the prediction of each tracker of each type to this.
// detection[tracker_types][tracker_amount[tracker_types]]
static box** dets_predictions;

// -----------------------
// Fuctions
// -----------------------

// initialize pointer with the max amount of types
void init_trackers(size_t max_index);

// public 
void updateTrackers(detection* dets, int nboxes, float thresh);

//internal functions for compuational splitting
static void addDetToArray(size_t index, detection* det);
static void extentTrackers(size_t index, box inital_rect);
static void removeTracker(size_t index, size_t removeIndex);
static int valueinarray(int val, int arr[], size_t n);
static float calculateIOU(box a, box b);


#endif
