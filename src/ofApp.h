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

	void setupView(std::string path);
	void createSaliencyMap(cv::Mat img);
	void createWatershed(cv::Mat saliencyImg);
	void loadEyeGaze(bool path);
	void ranking(ConstTools::EnterState enterState);
	void dumpOSC(ofxOscMessage m);

	ConstTools::InputFileName inputFileName;
	ConstTools::InputIPUFileName inputIPUFileName;
	ConstTools::InputMockFileName inputMockFileName;
	ConstTools::OutputOfPicFileName outputOfPicFileName;
	ConstTools::OutputOfEyeFileName outputOfEyeFileName;

	ofImage inputOfImg, loadOfImage;

	struct OriginalMat {
		cv::Mat original, copy;
	};
	OriginalMat originalPicMat, originalEyeMat;

	ConstTools::OutputOfImg outputOfPicIMG, outputOfEyeIMG;

	struct ViewMat
	{
		cv::Mat saliencyMap, saliencyMapColor;
		cv::Mat watershedHighest, saliencyHighest;
		cv::Mat matMix;
		cv::Mat eyeGazeMat;
	};
	ViewMat viewPicMat, viewEyeMat;

	std::vector<cv::Vec3b> colorPicTab, colorEyeTab;

	cv::Mat markersPicSave, markersEyeSave;

	cv::Mat picImgG, eyeImgG;

	struct SaliencyPoint
	{
		std::vector<int> saved, backup;
	};
	SaliencyPoint saliencyPicPoint, saliencyEyePoint;
	SaliencyPoint saliencyPicTotalPoint, saliencyEyeTotalPoint;

	std::vector<int> maxValuePicCount, maxValueEyeCount;


	struct MaxSaliencyPoint
	{
		std::vector<int>::iterator iter;
	    int maxIndex;
	};
	MaxSaliencyPoint maxSaliencyPicPoint, maxSaliencyEyePoint;

	std::string fileName;
	std::string eyeGazePath;

	struct HeatMap {
		ofxHeatMap gray;
		ofxHeatMap grays;
		ofxHeatMap spectral;
		ofxHeatMap spectrals;
	};
	HeatMap heatMap;

	ofxOscReceiver receiver;
	float remoteEyeGazeX, remoteEyeGazeY;

	ConstTools::EnterState enterState;
	unsigned int enterPicCount, enterEyeCount;

	std::stringstream picCntStr, eyeCntStr;

	ConstTools::EyeTrackState eyeTrackState;
	ConstTools::LoadState loadState;

	ConstTools::Infomation infomation;
	ConstTools::Mode mode;
	ConstTools::PrefixPath prefixPath;
	ConstTools::FileNameExtension ext;
	ConstTools::RankingState rankingState;

	std::string folderName;
	std::string inputFilePath;
};
