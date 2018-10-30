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
    // 出力データ（SPECTRAL_RESIDUAL, UI(画像)）
    ofImage outputOfImg, outputOfImg2;
    // 顕著性マップ
    cv::Mat saliencyMap_SPECTRAL_RESIDUAL, saliencyMap, saliencyMap_color;

    // 最小と最大の要素値とそれらの位置
    SaliencyTool::MinMax minMax;
    std::vector<int> pixelsList;

    // 10*10の顕著マップの最小値の場所
    SaliencyTool::MinPlace minPlace;
};
