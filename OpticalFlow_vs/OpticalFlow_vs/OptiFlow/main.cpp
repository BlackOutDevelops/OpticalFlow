// main.cpp

#include "stdafx.h"
#include "common.h"
#include "between.h"
#include "Tracker.h"

int main(int argc, char* argv[])
{
	char opt[8] = {'\0'};
	strcpy(opt, argv[1] ? argv[1] : "camera");
	if (!strcmp(opt, "frames")) { 
		// interpolate flow between frames
		between_frames(argc, NULL);
	} else if (!strcmp(opt, "camera")) {
		// track camera video in real time
		Tracker* tracker = new Tracker(20 /*n corners*/, 4 /*win size*/);
		tracker->start();
		delete tracker;
	} return (0);
}

