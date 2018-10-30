#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    // 画像の読み込み
    inputOfImg.load("test.jpg");
    inputOfImg.update();

    // Mat変換
    cv::Mat mat, mat_gray, mat_gaus, saliencyMap_norm;

    // Mat画像に変換
    mat = ofxCv::toCv(inputOfImg);

    // 白黒Mat画像に変換
    cvtColor(mat.clone(), mat_gray, cv::COLOR_BGR2GRAY);

    // ぼかし
    cv::GaussianBlur(mat_gray.clone(), mat_gaus, cv::Size(5, 5), 1, 1);
    // 顕著性マップ(SPECTRAL_RESIDUAL)に変換(顕著性マップを求めるアルゴリズム)
    cv::Ptr<cv::saliency::Saliency> saliencyAlgorithm;
    saliencyAlgorithm = cv::saliency::StaticSaliencySpectralResidual::create();
    saliencyAlgorithm->computeSaliency(mat_gaus.clone(), saliencyMap_SPECTRAL_RESIDUAL);

    // アルファチャンネルの正規化を行う
    cv::normalize(saliencyMap_SPECTRAL_RESIDUAL.clone(), saliencyMap_norm, 0.0, 255.0, cv::NORM_MINMAX);
    // Matの型（ビット深度）を変換する
    saliencyMap_norm.convertTo(saliencyMap, CV_8UC3);

    // 最小と最大の要素値とそれらの位置を求める
    minMaxLoc(saliencyMap, &minMax.min_val, &minMax.max_val, &minMax.min_loc, &minMax.max_loc, cv::Mat());

    // 画像(ofImage)に変換
    ofxCv::toOf(saliencyMap.clone(), outputOfImg);
    outputOfImg.update();

    // 疑似カラー（カラーマップ）変換 :（0:赤:顕著性が高い, 255:青:顕著性が低い）
    applyColorMap(saliencyMap.clone(), saliencyMap_color, cv::COLORMAP_JET);

    // 画像(ofImage)に変換
    ofxCv::toOf(saliencyMap.clone(), outputOfImg2);
    outputOfImg2.update();

}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

    // 元画像
    inputOfImg.draw(0,0,ofGetWidth()/2, ofGetHeight()/2);
    // 顕著性マップ(SPECTRAL_RESIDUAL)を出力
    ofxCv::drawMat(saliencyMap_SPECTRAL_RESIDUAL, ofGetWidth()-ofGetWidth()/2, 0, ofGetWidth()/2, ofGetHeight()/2);
    // 顕著性マップを出力
    outputOfImg.draw(0,ofGetHeight()/2,ofGetWidth()/2, ofGetHeight()/2);
    // 顕著性マップのヒートマップを出力
    outputOfImg2.draw(ofGetWidth()/2,ofGetHeight()/2,ofGetWidth()/2, ofGetHeight()/2);

    // Label
    ofDrawBitmapStringHighlight("original", 20, 20);
    ofDrawBitmapStringHighlight("saliencyMap", ofGetWidth()/2+20, 20);
    ofDrawBitmapStringHighlight("saliencyMap-after", 20, ofGetHeight()/2+20);
    ofDrawBitmapStringHighlight("saliencyMap-heatMap", ofGetWidth()/2+20, ofGetHeight()/2+20);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
