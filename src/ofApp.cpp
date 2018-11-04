#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    enterState = false;

    // 画像の読み込み
    ConstTools::FileName fileName;

    inputOfImg.load(fileName.lenna);
    inputOfImg.update();
    
    // Mat変換
    cv::Mat mat_gray, mat_gaus, saliencyMap_norm;
    
    // Mat画像に変換
    mat = ofxCv::toCv(inputOfImg);
    
    mat_copy = mat.clone();

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
    //    minMaxLoc(saliencyMap, &minMax.min_val, &minMax.max_val, &minMax.min_loc, &minMax.max_loc, cv::Mat());

    cv::Mat saliency_copy = saliencyMap.clone();
    // 画像(ofImage)に変換
    ofxCv::toOf(saliencyMap.clone(), outputOfSaliencyImg);
    outputOfSaliencyImg.update();
    //    outputOfSaliencyImg.save("outputOfSaliencyImg.png");

    // 画素値の反転（現状 : 0:黒:顕著性が低い, 255:白:顕著性が高い）
    for(int x = 0; x < saliency_copy.rows; ++x){
        for(int y = 0; y < saliency_copy.cols; ++y){
            saliency_copy.at<uchar>( x, y ) = 255 - (int)saliency_copy.at<uchar>(x, y);
            //            ofLog()<<"(int)saliencyMap.at<uchar>("<<x<<","<<y<< ") : "<<(int)saliencyMap.at<uchar>( x, y );
        }
    }
    // ヒートマップへ変換 :（0:赤:顕著性が高い, 255:青:顕著性が低い）
    cv::applyColorMap(saliency_copy.clone(), saliencyMap_color, cv::COLORMAP_JET);
    
    // 画像(ofImage)に変換
    ofxCv::toOf(saliencyMap_color.clone(), outputOfHeatMapImg);
    outputOfHeatMapImg.update();
    outputOfHeatMapImg.save("outputOfSaliencyImg.png");

    // 二値化
    cv::Mat thresh;
    cv::threshold(saliencyMap.clone(), thresh, 0, 255, cv::THRESH_OTSU);
    
    // ノイズ除去
    cv::Mat opening;
    cv::Mat kernel(3, 3, CV_8U, cv::Scalar(1));
    cv::morphologyEx(thresh.clone(), opening, cv::MORPH_OPEN, kernel, cv::Point(-1,-1), 2);
    
    // 背景領域抽出
    cv::Mat sure_bg;
    cv::dilate(opening.clone(), sure_bg, kernel, cv::Point(-1,-1), 3);
    
    // 前景領域抽出
    cv::Mat dist_transform;
    cv::distanceTransform(opening, dist_transform, CV_DIST_L2, 5);
    
    // 最小と最大の要素値とそれらの位置を求める
    cv::Mat sure_fg;
    cv::minMaxLoc(dist_transform, &minMax.min_val, &minMax.max_val, &minMax.min_loc, &minMax.max_loc);
    cv::threshold(dist_transform, sure_fg, 0.3*minMax.max_val, 255, 0);
    
    dist_transform = dist_transform/minMax.max_val;
    
    // 画像(ofImage)に変換
    ofxCv::toOf(sure_bg.clone(), outputOfBackgroundImg);
    outputOfBackgroundImg.update();
    
    // 不明領域抽出
    cv::Mat unknown, sure_fg_uc1;
    sure_fg.convertTo(sure_fg_uc1, CV_8UC1);
    cv::subtract(sure_bg, sure_fg_uc1, unknown);
    
    // 画像(ofImage)に変換
    ofxCv::toOf(unknown.clone(), outputOfUnknownImg);
    outputOfUnknownImg.update();
    
    // 前景ラベリング
    int compCount = 0;
    // すべてのマーカーを取得
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    sure_fg.convertTo(sure_fg, CV_32SC1, 1.0);
    cv::findContours(sure_fg, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);
    if( contours.empty() ) return;
    
    //    ofLogNotice()<<"contours: "<<&contours;
    
    // watershedに流し込む用のマーカー画像作成
    cv::Mat markers = cv::Mat::zeros(sure_fg.rows, sure_fg.cols, CV_32SC1);
    // マーカーを描画
    int idx = 0;
    for( ; idx >= 0; idx = hierarchy[idx][0], compCount++ )
        cv::drawContours(markers, contours, idx, cv::Scalar::all(compCount+1), -1, 8, hierarchy, INT_MAX);
    markers = markers+1;
    
    // 不明領域は今のところゼロ
    for(int i=0; i<markers.rows; i++){
        for(int j=0; j<markers.cols; j++){
            unsigned char &v = unknown.at<unsigned char>(i, j);
            if(v==255){
                markers.at<int>(i, j) = 0;
            }
        }
    }
    
    // 分水嶺
    cv::watershed(mat, markers);
    
    // 背景黒のMat画像
    watershedHighest = cv::Mat::zeros(mat.size(), CV_8UC3);
    saliencyHighest = cv::Mat::zeros(mat.size(), CV_8UC3);
    
    cv::Mat wshed(markers.size(), CV_8UC3);
    //    std::vector<cv::Vec3b> colorTab;
    
    std::vector<int> saliencyPoint(compCount, 0);
    
    ofLogNotice()<<"count: "<<compCount;
    for(int i = 0; i < compCount; i++ )
    {
        int b = cv::theRNG().uniform(0, 255);
        int g = cv::theRNG().uniform(0, 255);
        int r = cv::theRNG().uniform(0, 255);
        
        colorTab.push_back(cv::Vec3b((uchar)b, (uchar)g, (uchar)r));
    }
    
    // 分割した画像をそれぞれの画像に書き込む
    for(int i = 0; i < markers.rows; i++ ){
        for(int j = 0; j < markers.cols; j++ )
        {
            //            ofLogNotice()<<"index: "<<index;
            int index = markers.at<int>(i,j);
            
            if( index == -1 ) {
                wshed.at<cv::Vec3b>(i,j) = cv::Vec3b(255,255,255);
            }
            else if( index <= 0 || index > compCount ) {
                wshed.at<cv::Vec3b>(i,j) = cv::Vec3b(0,0,0);
            }
            else if( index == 1 ) {
                wshed.at<cv::Vec3b>(i,j) = colorTab[index - 1];
            }
            else {
                watershedHighest.at<cv::Vec3b>(i,j) = colorTab[index - 1];
                saliencyPoint[index-1] += (int)saliencyMap.at<uchar>(i, j);
            }
        }
    }
    
    for (int i=0; i<saliencyPoint.size(); i++) {
        ofLogNotice() << "saliencyPoint[" << i << "]: " << saliencyPoint[i];
    }

    // 最大値の要素番号を取得
    iter = std::max_element(saliencyPoint.begin(), saliencyPoint.end());
    saliencyPointMaxIndex = std::distance(saliencyPoint.begin(), iter);
    ofLogNotice() << "Index of max element: " << saliencyPointMaxIndex;

    for(int i = 0; i < markers.rows; i++ ){
        for(int j = 0; j < markers.cols; j++ )
        {
            int index = markers.at<int>(i,j);
            if( index == saliencyPointMaxIndex+1 ) {
                saliencyHighest.at<cv::Vec3b>(i,j) = colorTab[index - 1];
            } else {
                mat_copy.at<cv::Vec3b>(i,j) = cv::Vec3b((uchar)0, (uchar)0, (uchar)0);
                //                mat_copy.at<cv::Vec3b>(i,j) = cv::Vec3b((uchar)255, (uchar)255, (uchar)255);
            }
        }
    }
    
    cvtColor(saliencyMap.clone(), imgG, cv::COLOR_GRAY2BGR);
    wshed = wshed*0.5 + imgG*0.5;
    watershedHighest = watershedHighest*0.5 + imgG*0.5;
    saliencyHighest = saliencyHighest*0.5 + imgG*0.5;
    // 画像(ofImage)に変換
    ofxCv::toOf(wshed.clone(), outputOfWatershedImg);
    outputOfWatershedImg.update();
    ofxCv::toOf(watershedHighest.clone(), outputOfWatershedAfterImg);
    outputOfWatershedAfterImg.update();
    outputOfWatershedAfterImg.save("outputOfWatershedAfterImg.png");
    ofxCv::toOf(saliencyHighest.clone(), outputOfWatershedHighestImg);
    outputOfWatershedHighestImg.update();


    mat_mix = mat*0.2 + mat_copy*0.8;
    // 画像(ofImage)に変換
    ofxCv::toOf(mat_mix.clone(), outputOfSaliencyMapHighestImg);
    outputOfSaliencyMapHighestImg.update();
    outputOfSaliencyMapHighestImg.save("outputOfSaliencyMapHighestImg.png");


    markersSave = markers.clone();
    
    
    if (!saliencyPoint.empty()) {
        saliencyPointSave = saliencyPoint;
        saliencyPoint.clear();
    }
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
    if (enterState) {
        
        //        for(int i = 0; i < markersSave.rows; i++ ){
        //            for(int j = 0; j < markersSave.cols; j++ )
        //            {
        //                //            ofLogNotice()<<"index: "<<index;
        //                int index = markersSave.at<int>(i,j);
        //
        //                if( index != -1 && index > 0 && index != 1 ) {
        //                    saliencyPointSave[index-1] += (int)saliencyMap.at<uchar>(i, j);
        //                }
        //            }
        //        }

        for (int i=0; i<saliencyPointSave.size(); i++) {
            ofLogNotice() << "saliencyPoint[" << i << "]: " << saliencyPointSave[i];
        }

        // 最大値の要素番号を取得
        iter = std::max_element(saliencyPointSave.begin(), saliencyPointSave.end());
        saliencyPointMaxIndex = std::distance(saliencyPointSave.begin(), iter);
        ofLogNotice() << "Index of max element: " << saliencyPointMaxIndex;
        
        // 初期化
        saliencyHighest = cv::Mat::zeros(saliencyHighest.size(), CV_8UC3);
        mat_copy = mat.clone();
        
        // 画像に書き込む
        for(int i = 0; i < markersSave.rows; i++ ){
            for(int j = 0; j < markersSave.cols; j++ )
            {
                int index = markersSave.at<int>(i,j);
                if(index == saliencyPointMaxIndex+1) {
                    saliencyHighest.at<cv::Vec3b>(i,j) = colorTab[index - 1];
                } else {
                    mat_copy.at<cv::Vec3b>(i,j) = cv::Vec3b((uchar)0, (uchar)0, (uchar)0);
                    //                    mat_copy.at<cv::Vec3b>(i,j) = cv::Vec3b((uchar)255, (uchar)255, (uchar)255);
                }
            }
        }
        
        saliencyHighest = saliencyHighest*0.5 + imgG*0.5;
        // 画像(ofImage)に変換
        ofxCv::toOf(saliencyHighest.clone(), outputOfWatershedHighestImg);
        outputOfWatershedHighestImg.update();


        mat_mix = mat*0.2 + mat_copy*0.8;
        // 画像(ofImage)に変換
        ofxCv::toOf(mat_mix.clone(), outputOfSaliencyMapHighestImg);
        outputOfSaliencyMapHighestImg.update();
        //        outputOfSaliencyMapHighestImg.save("outputOfSaliencyMapHighestImg.png");

        enterState = false;
    }
    
}

//--------------------------------------------------------------
void ofApp::draw(){

    int count = 0;
    // Label
    ofDrawBitmapStringHighlight("keyPressed\n\n・Z: RELEASE\n・X: DEBUG\n・C: SALIENCY\n・Enter: Next HighSaliency Place", ofGetWidth()-ofGetWidth()/4-40, 20);

    //    ofDrawBitmapStringHighlight("Enter key count: %d", ofGetWidth()-ofGetWidth()/4-40, ofGetHeight()/2+20);
    switch (use) {
        case ConstTools::RELEASE:
            // 元画像
            inputOfImg.draw(0,0,ofGetWidth()/2, ofGetHeight()/2);
            // 領域分割（特定箇所）を出力
            outputOfSaliencyMapHighestImg.draw(0,ofGetHeight()/2,ofGetWidth()/2, ofGetHeight()/2);

            // Label
            ofDrawBitmapStringHighlight("original", ofGetWidth()/2+20, 20);
            ofDrawBitmapStringHighlight("saliencyMap-watershed", ofGetWidth()/2+20, ofGetHeight()/2+20);
            break;

        case ConstTools::DEBUG:
            // 元画像
            inputOfImg.draw(0,0,ofGetWidth()/3, ofGetHeight()/3);
            // 顕著性マップを出力
            outputOfSaliencyImg.draw(ofGetWidth()/3,0,ofGetWidth()/3, ofGetHeight()/3);
            // 顕著性マップのヒートマップを出力
            outputOfHeatMapImg.draw(ofGetWidth()-ofGetWidth()/3,0,ofGetWidth()/3, ofGetHeight()/3);
            // 分水嶺を出力
            outputOfBackgroundImg.draw(0,ofGetHeight()/3,ofGetWidth()/3, ofGetHeight()/3);
            // 不明領域を出力
            outputOfUnknownImg.draw(ofGetWidth()/3,ofGetHeight()/3,ofGetWidth()/3, ofGetHeight()/3);
            // 分水嶺を出力
            outputOfWatershedImg.draw(ofGetWidth()-ofGetWidth()/3,ofGetHeight()/3,ofGetWidth()/3, ofGetHeight()/3);
            // 領域分割を出力
            outputOfWatershedAfterImg.draw(0,ofGetHeight()-ofGetHeight()/3,ofGetWidth()/3, ofGetHeight()/3);
            // 領域分割（特定箇所）を出力
            outputOfWatershedHighestImg.draw(ofGetWidth()/3,ofGetHeight()-ofGetHeight()/3,ofGetWidth()/3, ofGetHeight()/3);
            // 領域分割（元画像から特定箇所）を出力
            outputOfSaliencyMapHighestImg.draw(ofGetWidth()-ofGetWidth()/3,ofGetHeight()-ofGetHeight()/3,ofGetWidth()/3, ofGetHeight()/3);


            // Label
            ofDrawBitmapStringHighlight("original", 20, 20);
            ofDrawBitmapStringHighlight("saliencyMap", ofGetWidth()/3+20, 20);
            ofDrawBitmapStringHighlight("saliencyMap-heatMap", ofGetWidth()-ofGetWidth()/3+20, 20);
            ofDrawBitmapStringHighlight("background", 20, ofGetHeight()/3+20);
            ofDrawBitmapStringHighlight("unknown", ofGetWidth()/3+20, ofGetHeight()/3+20);
            ofDrawBitmapStringHighlight("watershed", ofGetWidth()-ofGetWidth()/3+20, ofGetHeight()/3+20);
            ofDrawBitmapStringHighlight("watershed-after", 20, ofGetHeight()-ofGetHeight()/3+20);
            ofDrawBitmapStringHighlight("watershed-highest", ofGetWidth()/3+20, ofGetHeight()-ofGetHeight()/3+20);
            ofDrawBitmapStringHighlight("saliencyMap-highest", ofGetWidth()-ofGetWidth()/3+20, ofGetHeight()-ofGetHeight()/3+20);
            break;
        case ConstTools::SALIENCY:
            // 元画像
            inputOfImg.draw(0,0,ofGetWidth()/2,ofGetHeight()/2);
            // 顕著性マップを出力
            outputOfSaliencyImg.draw(ofGetWidth()/2,0,ofGetWidth()/2,ofGetHeight()/2);

            break;
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    ofLogNotice() << "keyPressed: " << key;
    
    switch (key) {
        case 13:

            //            if (!saliencyPointSave.empty()) {
            //                saliencyPointSave.clear();
            //            }
            //            for(int i = 0; i < markersSave.rows; i++ ){
            //                for(int j = 0; j < markersSave.cols; j++ )
            //                {
            //                    int index = markersSave.at<int>(i,j);
            //                    if( index == saliencyPointMaxIndex+1 ) {
            //                        markersSave.at<cv::Vec3b>(i,j) = cv::Vec3b((uchar)0, (uchar)0, (uchar)0);
            //                    }
            //                }
            //            }

            saliencyPointSave[saliencyPointMaxIndex] = 0;

            enterState = true;
            break;
            //-------------   環境   ------------------
        case 122:
            // "Z"を押した時: release
            use = ConstTools::RELEASE;
            break;
        case 120:
            // "X"を押した時: debug
            use = ConstTools::DEBUG;
            break;
        case 99:
            // "C"を押した時: saliency
            use = ConstTools::SALIENCY;
            break;
    }

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
