#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ofxGui.h"
#include "ofxOsc.h"

#include "saliencySpecializedClasses.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc.hpp"
#include "iostream"
#include "sstream"
#include "vector"
#include "algorithm"

#include "saliencyTools.h"
#include "constTools.h"

#define MACWIDTH 1024
#define MACHEIGHT 768

#define WINWIDTH 1920
#define WINHEIGHT 1080

class ofApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);


private:
	ofImage inputOfImg;

	cv::Mat mat, mat_copy, mat_mix;

	ofImage outputOfSaliencyImg, outputOfHeatMapImg;
	ofImage outputOfBackgroundImg, outputOfUnknownImg, outputOfWatershedImg, outputOfWatershedAfterImg, outputOfWatershedHighestImg, outputOfSaliencyMapHighestImg, outputOfEyeGazeImg;

	cv::Mat saliencyMap_SPECTRAL_RESIDUAL, saliencyMap, saliencyMap_color;

	std::vector<cv::Vec3b> colorTab;

	cv::Mat markersSave;

	cv::Mat watershedHighest, saliencyHighest;

	cv::Mat imgG;

	SaliencyTools::MinMax minMax;
	//    std::vector<int> pixelsList;

	std::vector<int> saliencyPointSave;
	std::vector<int> saliencyPointBackUp;

	std::vector<int>::iterator iter;
	int saliencyPointMaxIndex;

	bool enterState;
	int enterCount;
	std::stringstream enterCountString;

	ConstTools::Use use;

	ofxOscReceiver receiver;
	float remoteEyeGazeX, remoteEyeGazeY;

	void dumpOSC(ofxOscMessage m);

	cv::Mat eyeGazeMat;
	ConstTools::EyeTrack eyeTrack;
};
