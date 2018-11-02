#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ofxGui.h"

#include "saliencySpecializedClasses.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc.hpp"
#include "iostream"
#include "vector"
#include "algorithm"

#include "saliencyTools.hpp"

class ofApp : public ofBaseApp{

public:
    void setup();
    void update();
    void draw();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

    // 画像
    ofImage inputOfImg;

    cv::Mat mat, mat_copy;
    
    // 出力データ
    ofImage outputOfSaliencyImg, outputOfHeatMapImg;
    ofImage outputOfImg, outputOfImg2, outputOfImg3, outputOfImg4, outputOfImg5, outputOfImg6;
    // 顕著性マップ
    cv::Mat saliencyMap_SPECTRAL_RESIDUAL, saliencyMap, saliencyMap_color;

    std::vector<cv::Vec3b> colorTab;
    cv::Mat markersSave;
    
    cv::Mat dividA, dividB;
    
    cv::Mat imgG;

    // 最小と最大の要素値とそれらの位置
    SaliencyTool::MinMax minMax;
//    std::vector<int> pixelsList;

    std::vector<int> saliencyPointSave;

    // vector内の最大値の要素番号
    std::vector<int>::iterator iter;
    int saliencyPointMaxIndex;

    bool state;
};
