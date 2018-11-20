#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ofxGui.h"
#include "ofxOsc.h"
#include "ofxHeatMap.h"

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

	void createSaliencyMap(cv::Mat img);
	void createWatershed(cv::Mat saliencyImg);

	ConstTools::InputFileName inputFileName;
	ConstTools::OutputFileName_FIRST outputfileName;
	ConstTools::OutputFileName_SECOND outputfileName2;

	ofImage inputOfImg, loadOfImage;

	cv::Mat mat_original, mat_copy, mat_mix;

	struct OutputOfImg
	{
		ofImage outputOfSaliencyImg, outputOfHeatMapImg;
		ofImage outputOfBackgroundImg, outputOfUnknownImg, outputOfWatershedImg, outputOfWatershedAfterImg, outputOfWatershedHighestImg, outputOfSaliencyMapHighestImg, outputOfEyeGazeImg;
		ofImage outputOfEyeGazeHeatMapImg, outputOfResultImg;
	};


	OutputOfImg outputOfIMG_FIRST, outputOfIMG_SECOND;

	struct ViewMat
	{
		cv::Mat saliencyMap, saliencyMapColor;
		cv::Mat watershedHighest, saliencyHighest;
	};

	ViewMat viewMatSaliency, viewMatEyeGaze;

	std::vector<cv::Vec3b> colorTabSaliency, colorTabEyeGaze;

	cv::Mat markersSave, markersSave_SECOND;

	cv::Mat imgG, imgG2;

	SaliencyTools::MinMax minMax;
	//    std::vector<int> pixelsList;

	struct SaliencyPoint
	{
		std::vector<int> save, backup;
	};
	SaliencyPoint saliencyPointPicture, saliencyPointEyeGaze;

	struct MaxSaliencyPoint
	{
		std::vector<int>::iterator iter;
		int maxIndex;
	};
	MaxSaliencyPoint maxSaliencyPointPicture, maxSaliencyPointEyeGaze;

	ConstTools::EnterState enterState;
	int enterCount;
	std::stringstream enterCountString;

	ConstTools::Mode mode;

	ofxOscReceiver receiver;
	float remoteEyeGazeX, remoteEyeGazeY;

	void dumpOSC(ofxOscMessage m);

	cv::Mat eyeGazeMat;
	ConstTools::EyeTrackState eyeTrackState;
	ConstTools::LoadState loadState;

	ofxHeatMap heatmap;

	cv::Mat s11;

	ConstTools::Infomation infomation;

	cv::Mat mat_original_SECOND, mat_copy_SECOND;

};
