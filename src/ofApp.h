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
#include "sstream"9
#include "vector"
#include "algorithm"

#include "saliencyTools.hpp"
#include "constTools.hpp"

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
    // 画像からMat画像に変換
    cv::Mat mat, mat_copy, mat_mix;
    
    // 出力データ
    ofImage outputOfSaliencyImg, outputOfHeatMapImg;
    ofImage outputOfBackgroundImg, outputOfUnknownImg, outputOfWatershedImg, outputOfWatershedAfterImg, outputOfWatershedHighestImg, outputOfSaliencyMapHighestImg;

    // 顕著性マップ
    cv::Mat saliencyMap_SPECTRAL_RESIDUAL, saliencyMap, saliencyMap_color;

    // 顕著箇所に対しての色配分
    std::vector<cv::Vec3b> colorTab;
    // watershedに流し込む用のマーカー画像
    cv::Mat markersSave;
    // 顕著箇所の出力
    cv::Mat watershedHighest, saliencyHighest;
    // 顕著性マップのグレースケール画像
    cv::Mat imgG;

    // 最小と最大の要素値とそれらの位置
    SaliencyTools::MinMax minMax;
//    std::vector<int> pixelsList;
    
    // 顕著性マップの画素値格納
    std::vector<int> saliencyPointSave;
    std::vector<int> saliencyPointBackUp;

    // vector内の最大値の要素番号
    std::vector<int>::iterator iter;
    int saliencyPointMaxIndex;

    // エンターキー
    bool enterState;
    int enterCount;
    std::stringstream enterCountString;
    
    // 環境選択
    ConstTools::Use use;

};
