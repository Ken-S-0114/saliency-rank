#include "ofApp.h"

#define PORT 8000
#define HOST "127.0.0.1"

//--------------------------------------------------------------
void ofApp::setup() {

	receiver.setup(PORT);

	ofLogNotice() << "ofGetScreenWidth: " << ofGetScreenWidth();
	ofLogNotice() << "ofGetScreenHeight: " << ofGetScreenHeight();

	eyeGazeMat = cv::Mat::zeros(WINHEIGHT, WINWIDTH, CV_8UC1);

	enterState = false;
	enterCount = 0;

	enterCountString << "The " << enterCount + 1 << " most saliency place";

	ConstTools::InputFileName inputFileName;
	ConstTools::OutputFileName outputfileName;
	inputOfImg.load(inputFileName.lenna);
	inputOfImg.update();

	//    saliencyCreated(inputOfImg);

	cv::Mat mat_gray, mat_gaus, saliencyMap_norm;

	mat = ofxCv::toCv(inputOfImg);

	mat_copy = mat.clone();

	cvtColor(mat.clone(), mat_gray, cv::COLOR_BGR2GRAY);

	cv::GaussianBlur(mat_gray.clone(), mat_gaus, cv::Size(5, 5), 1, 1);

	cv::Ptr<cv::saliency::Saliency> saliencyAlgorithm;
	saliencyAlgorithm = cv::saliency::StaticSaliencySpectralResidual::create();
	saliencyAlgorithm->computeSaliency(mat_gaus.clone(), saliencyMap_SPECTRAL_RESIDUAL);

	cv::normalize(saliencyMap_SPECTRAL_RESIDUAL.clone(), saliencyMap_norm, 0.0, 255.0, cv::NORM_MINMAX);

	saliencyMap_norm.convertTo(saliencyMap, CV_8UC3);

	//    minMaxLoc(saliencyMap, &minMax.min_val, &minMax.max_val, &minMax.min_loc, &minMax.max_loc, cv::Mat());

	cv::Mat s1 = saliencyMap.clone();
	ofxCv::toOf(s1, outputOfSaliencyImg);


	outputOfSaliencyImg.update();
    outputOfSaliencyImg.save("outputOfSaliencyImg.png");

	cv::Mat saliency_copy = saliencyMap.clone();

	for (int x = 0; x < saliency_copy.rows; ++x) {
		for (int y = 0; y < saliency_copy.cols; ++y) {
			saliency_copy.at<uchar>(x, y) = 255 - (int)saliency_copy.at<uchar>(x, y);
			//ofLogNotice()<<"(int)saliency_copy.at<uchar>("<<x<<","<<y<< ") : "<<(int)saliency_copy.at<uchar>( x, y );
		}
	}

	cv::applyColorMap(saliency_copy.clone(), saliencyMap_color, cv::COLORMAP_JET);

	cv::Mat s2 = saliencyMap_color.clone();

	ofxCv::toOf(s2, outputOfHeatMapImg);
	outputOfHeatMapImg.update();
	outputOfHeatMapImg.save(outputfileName.outputOfSaliencyImg);

	//    watershedCreated(saliencyMap.clone());

	cv::Mat thresh;
	cv::threshold(saliencyMap.clone(), thresh, 0, 255, cv::THRESH_OTSU);

	cv::Mat opening;
	cv::Mat kernel(3, 3, CV_8U, cv::Scalar(1));
	cv::morphologyEx(thresh.clone(), opening, cv::MORPH_OPEN, kernel, cv::Point(-1, -1), 2);

	cv::Mat sure_bg;
	cv::dilate(opening.clone(), sure_bg, kernel, cv::Point(-1, -1), 3);

	cv::Mat dist_transform;
	cv::distanceTransform(opening, dist_transform, CV_DIST_L2, 5);

	cv::Mat sure_fg;

	cv::minMaxLoc(dist_transform, &minMax.min_val, &minMax.max_val, &minMax.min_loc, &minMax.max_loc);
	cv::threshold(dist_transform, sure_fg, 0.5*minMax.max_val, 255, 0);

	dist_transform = dist_transform / minMax.max_val;

	cv::Mat s3 = sure_bg.clone();
	ofxCv::toOf(s3, outputOfBackgroundImg);
	outputOfBackgroundImg.update();

	cv::Mat unknown;
	cv::Mat sure_fg_uc1;
	sure_fg.convertTo(sure_fg_uc1, CV_8UC1);
	cv::subtract(sure_bg, sure_fg_uc1, unknown);

	cv::Mat s4 = unknown.clone();
	ofxCv::toOf(s4, outputOfUnknownImg);
	outputOfUnknownImg.update();

	int compCount = 0;

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	sure_fg.convertTo(sure_fg, CV_32SC1, 1.0);
	cv::findContours(sure_fg, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);
	if (contours.empty()) return;

	//    ofLogNotice()<<"contours: "<<&contours;

	cv::Mat markers = cv::Mat::zeros(sure_fg.rows, sure_fg.cols, CV_32SC1);

	int idx = 0;
	for (; idx >= 0; idx = hierarchy[idx][0], compCount++)
		cv::drawContours(markers, contours, idx, cv::Scalar::all(compCount + 1), -1, 8, hierarchy, INT_MAX);
	markers = markers + 1;

	for (int i = 0; i<markers.rows; i++) {
		for (int j = 0; j<markers.cols; j++) {
			unsigned char &v = unknown.at<unsigned char>(i, j);
			if (v == 255) {
				markers.at<int>(i, j) = 0;
			}
		}
	}

	cv::watershed(mat, markers);

	watershedHighest = cv::Mat::zeros(mat.size(), CV_8UC3);
	saliencyHighest = cv::Mat::zeros(mat.size(), CV_8UC3);

	cv::Mat wshed(markers.size(), CV_8UC3);
	//    std::vector<cv::Vec3b> colorTab;

	std::vector<int> saliencyPoint(compCount, 0);

	ofLogNotice() << "count: " << compCount;
	for (int i = 0; i < compCount; i++)
	{
		int b = cv::theRNG().uniform(0, 255);
		int g = cv::theRNG().uniform(0, 255);
		int r = cv::theRNG().uniform(0, 255);

		colorTab.push_back(cv::Vec3b((uchar)b, (uchar)g, (uchar)r));
	}

	for (int i = 0; i < markers.rows; i++) {
		for (int j = 0; j < markers.cols; j++)
		{
			//            ofLogNotice()<<"index: "<<index;
			int index = markers.at<int>(i, j);

			if (index == -1) {
				wshed.at<cv::Vec3b>(i, j) = cv::Vec3b(255, 255, 255);
			}
			else if (index <= 0 || index > compCount) {
				wshed.at<cv::Vec3b>(i, j) = cv::Vec3b(0, 0, 0);
			}
			else if (index == 1) {
				wshed.at<cv::Vec3b>(i, j) = colorTab[index - 1];
			}
			else {
				watershedHighest.at<cv::Vec3b>(i, j) = colorTab[index - 1];
				if (saliencyPoint[index - 1] < (int)saliencyMap.at<uchar>(i, j)) {
					saliencyPoint[index - 1] = (int)saliencyMap.at<uchar>(i, j);
				}
			}
		}
	}

	for (int i = 0; i<saliencyPoint.size(); i++) {
		ofLogNotice() << "saliencyPoint[" << i << "]: " << saliencyPoint[i];
	}

	iter = std::max_element(saliencyPoint.begin(), saliencyPoint.end());
	saliencyPointMaxIndex = std::distance(saliencyPoint.begin(), iter);
	ofLogNotice() << "Index of max element: " << saliencyPointMaxIndex;

	for (int i = 0; i < markers.rows; i++) {
		for (int j = 0; j < markers.cols; j++)
		{
			int index = markers.at<int>(i, j);
			if (index == saliencyPointMaxIndex + 1) {
				saliencyHighest.at<cv::Vec3b>(i, j) = colorTab[index - 1];
			}
			else {
				mat_copy.at<cv::Vec3b>(i, j) = cv::Vec3b((uchar)0, (uchar)0, (uchar)0);
				//                mat_copy.at<cv::Vec3b>(i,j) = cv::Vec3b((uchar)255, (uchar)255, (uchar)255);
			}
		}
	}

	cvtColor(saliencyMap.clone(), imgG, cv::COLOR_GRAY2BGR);
	wshed = wshed*0.5 + imgG*0.5;
	watershedHighest = watershedHighest*0.5 + imgG*0.5;
	saliencyHighest = saliencyHighest*0.5 + imgG*0.5;

	cv::Mat s5 = wshed.clone();
	ofxCv::toOf(s5, outputOfWatershedImg);
	outputOfWatershedImg.update();

	cv::Mat s6 = watershedHighest.clone();
	ofxCv::toOf(s6, outputOfWatershedAfterImg);
	outputOfWatershedAfterImg.update();
	outputOfWatershedAfterImg.save(outputfileName.outputOfWatershedAfterImg);

	cv::Mat s7 = saliencyHighest.clone();
	ofxCv::toOf(s7, outputOfWatershedHighestImg);
	outputOfWatershedHighestImg.update();


	mat_mix = mat*0.2 + mat_copy*0.8;

	cv::Mat s8 = mat_mix.clone();
	ofxCv::toOf(s8, outputOfSaliencyMapHighestImg);
	outputOfSaliencyMapHighestImg.update();
	outputOfSaliencyMapHighestImg.save(outputfileName.outputOfSaliencyMapHighestImg);


	markersSave = markers.clone();

	if (!saliencyPoint.empty()) {
		saliencyPointSave = saliencyPoint;
		saliencyPointBackUp = saliencyPoint;
		saliencyPoint.clear();
	}

}

//--------------------------------------------------------------
void ofApp::update() {

	while (receiver.hasWaitingMessages())
	{
		ofxOscMessage m;
		receiver.getNextMessage(&m);

		if (m.getAddress() == "/eyeGaze") {
			remoteEyeGazeX = m.getArgAsFloat(0);
			remoteEyeGazeY = m.getArgAsFloat(1);

			/*ofLogNotice() << "remoteEyeGazeX: " << remoteEyeGazeX;
			ofLogNotice() << "remoteEyeGazeY: " << remoteEyeGazeY;*/

			//dumpOSC(m);

			/*int remoteEyeGazeIntX = (int)remoteEyeGazeX;
			int remoteEyeGazeIntY = (int)remoteEyeGazeY;

			if ((0 <= remoteEyeGazeIntX) && (0 <= remoteEyeGazeIntY))
			{
				if ((remoteEyeGazeIntX <= WINWIDTH) && (remoteEyeGazeIntY <= WINHEIGHT))
				{
					ofLogNotice() << "remoteEyeGazeIntX(after): " << remoteEyeGazeIntX;
					ofLogNotice() << "remoteEyeGazeIntY(after): " << remoteEyeGazeIntY;
					if ((int)eyeGazeMat.at<uchar>(remoteEyeGazeIntX, remoteEyeGazeIntY) < 255)
					{
						ofLogNotice() << "eyeGazeMat: " << (int)eyeGazeMat.at<uchar>(remoteEyeGazeIntX, remoteEyeGazeIntY);
						eyeGazeMat.at<uchar>(remoteEyeGazeIntX, remoteEyeGazeIntY) = (int)eyeGazeMat.at<uchar>(remoteEyeGazeIntX, remoteEyeGazeIntY) + 254;
						cv::Mat s9 = eyeGazeMat.clone();
						ofxCv::toOf(s9, outputOfEyeGazeImg);
						outputOfEyeGazeImg.update();
					}
				}
			}*/

			int remoteEyeGazeIntX = (int)remoteEyeGazeX;
			int remoteEyeGazeIntY = (int)remoteEyeGazeY;

			if ((0 <= remoteEyeGazeIntX) && (0 <= remoteEyeGazeIntY))
			{
				if ((remoteEyeGazeIntX <= WINWIDTH) && (remoteEyeGazeIntY <= WINHEIGHT))
				{
					ofLogNotice() << "remoteEyeGazeIntX(after): " << remoteEyeGazeIntX;
					ofLogNotice() << "remoteEyeGazeIntY(after): " << remoteEyeGazeIntY;
					if ((int)eyeGazeMat.at<uchar>(remoteEyeGazeIntY, remoteEyeGazeIntX) < 255)
					{
						ofLogNotice() << "eyeGazeMat: " << (int)eyeGazeMat.at<uchar>(remoteEyeGazeIntY, remoteEyeGazeIntX);
						eyeGazeMat.at<uchar>(remoteEyeGazeIntY, remoteEyeGazeIntX) = (int)eyeGazeMat.at<uchar>(remoteEyeGazeIntY, remoteEyeGazeIntX) + 254;
						cv::Mat s9 = eyeGazeMat.clone();
						ofxCv::toOf(s9, outputOfEyeGazeImg);
						outputOfEyeGazeImg.update();
					}
				}
			}

		}

	}

	if (enterState) {
		enterCountString.str("");
		enterCountString.clear(stringstream::goodbit);

		int maxValue = *std::max_element(saliencyPointSave.begin(), saliencyPointSave.end());

		if (maxValue != 0) {

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

			for (int i = 0; i<saliencyPointSave.size(); i++) {
				ofLogNotice() << "saliencyPoint[" << i << "]: " << saliencyPointSave[i];
			}

			iter = std::max_element(saliencyPointSave.begin(), saliencyPointSave.end());
			saliencyPointMaxIndex = std::distance(saliencyPointSave.begin(), iter);
			ofLogNotice() << "Index of max element: " << saliencyPointMaxIndex;

			saliencyHighest = cv::Mat::zeros(saliencyHighest.size(), CV_8UC3);
			mat_copy = mat.clone();

			for (int i = 0; i < markersSave.rows; i++) {
				for (int j = 0; j < markersSave.cols; j++)
				{
					int index = markersSave.at<int>(i, j);
					if (index == saliencyPointMaxIndex + 1) {
						saliencyHighest.at<cv::Vec3b>(i, j) = colorTab[index - 1];
					}
					else {
						mat_copy.at<cv::Vec3b>(i, j) = cv::Vec3b((uchar)0, (uchar)0, (uchar)0);
		                //mat_copy.at<cv::Vec3b>(i,j) = cv::Vec3b((uchar)255, (uchar)255, (uchar)255);
					}
				}
			}

			saliencyHighest = saliencyHighest*0.5 + imgG*0.5;

			cv::Mat s7 = saliencyHighest.clone();
			ofxCv::toOf(s7, outputOfWatershedHighestImg);
			outputOfWatershedHighestImg.update();


			mat_mix = mat*0.2 + mat_copy*0.8;

			cv::Mat s8 = mat_mix.clone();
			ofxCv::toOf(s8, outputOfSaliencyMapHighestImg);
			outputOfSaliencyMapHighestImg.update();
			outputOfSaliencyMapHighestImg.save("outputOfSaliencyMapHighestImg.png");

			enterCountString << "The " << enterCount + 1 << " most saliency place";

		}
		else {
			enterCountString << "Finish";
		};

		enterState = false;
	}

}

//--------------------------------------------------------------
void ofApp::draw() {

	switch (use) {
	case ConstTools::RELEASE:
		inputOfImg.draw(0, 0, ofGetWidth() / 2, ofGetHeight() / 2);
		//ofxCv::drawMat(mat_mix.clone(), 0, ofGetHeight() / 2, ofGetWidth() / 2, ofGetHeight() / 2);
		outputOfSaliencyMapHighestImg.draw(0, ofGetHeight() / 2, ofGetWidth() / 2, ofGetHeight() / 2);

		// Label
		ofDrawBitmapStringHighlight("original", ofGetWidth() / 2 + 20, 20);
		ofDrawBitmapStringHighlight("saliencyMap-watershed", ofGetWidth() / 2 + 20, ofGetHeight() / 2 + 20);

		// Label
		ofDrawBitmapStringHighlight("KeyPressed\n\n・Z: RELEASE\n・X: DEBUG\n・C: SALIENCY\n\n・Enter: Next HighSaliency Place\n・Delete: Reset", ofGetWidth() - ofGetWidth() / 4 - 40, 20);

		ofDrawBitmapStringHighlight(enterCountString.str(), ofGetWidth() / 2 + 20, ofGetHeight() / 2 + 50);
		break;

	case ConstTools::DEBUG:
		inputOfImg.draw(0, 0, ofGetWidth() / 3, ofGetHeight() / 3);
		//ofxCv::drawMat(saliencyMap.clone(), ofGetWidth() / 3, 0, ofGetWidth() / 3, ofGetHeight() / 3);
		outputOfSaliencyImg.draw(ofGetWidth() / 3, 0, ofGetWidth() / 3, ofGetHeight() / 3);
		//ofxCv::drawMat(saliencyMap_color.clone(), ofGetWidth() - ofGetWidth() / 3, 0, ofGetWidth() / 3, ofGetHeight() / 3);
		outputOfHeatMapImg.draw(ofGetWidth() - ofGetWidth() / 3, 0, ofGetWidth() / 3, ofGetHeight() / 3);
		//ofxCv::drawMat(sure_bg.clone(), 0, ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);
		outputOfBackgroundImg.draw(0, ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);
		//ofxCv::drawMat(unknown.clone(), ofGetWidth() / 3, ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);
		outputOfUnknownImg.draw(ofGetWidth() / 3, ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);
		//ofxCv::drawMat(wshed.clone(), ofGetWidth() - ofGetWidth() / 3, ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);
		outputOfWatershedImg.draw(ofGetWidth() - ofGetWidth() / 3, ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);
		//ofxCv::drawMat(watershedHighest.clone(), 0, ofGetHeight() - ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);
		outputOfWatershedAfterImg.draw(0, ofGetHeight() - ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);
		//ofxCv::drawMat(saliencyHighest.clone(), ofGetWidth() / 3, ofGetHeight() - ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);
		outputOfWatershedHighestImg.draw(ofGetWidth() / 3, ofGetHeight() - ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);
		//ofxCv::drawMat(mat_mix.clone(), ofGetWidth() - ofGetWidth() / 3, ofGetHeight() - ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);
		outputOfSaliencyMapHighestImg.draw(ofGetWidth() - ofGetWidth() / 3, ofGetHeight() - ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);


		// Label
		ofDrawBitmapStringHighlight("original", 20, 20);
		ofDrawBitmapStringHighlight("saliencyMap", ofGetWidth() / 3 + 20, 20);
		ofDrawBitmapStringHighlight("saliencyMap-heatMap", ofGetWidth() - ofGetWidth() / 3 + 20, 20);
		ofDrawBitmapStringHighlight("background", 20, ofGetHeight() / 3 + 20);
		ofDrawBitmapStringHighlight("unknown", ofGetWidth() / 3 + 20, ofGetHeight() / 3 + 20);
		ofDrawBitmapStringHighlight("watershed", ofGetWidth() - ofGetWidth() / 3 + 20, ofGetHeight() / 3 + 20);
		ofDrawBitmapStringHighlight("watershed-after", 20, ofGetHeight() - ofGetHeight() / 3 + 20);
		ofDrawBitmapStringHighlight("watershed-highest", ofGetWidth() / 3 + 20, ofGetHeight() - ofGetHeight() / 3 + 20);
		ofDrawBitmapStringHighlight("saliencyMap-highest", ofGetWidth() - ofGetWidth() / 3 + 20, ofGetHeight() - ofGetHeight() / 3 + 20);
		break;
	case ConstTools::SALIENCY:
		//ofxCv::drawMat(eyeGazeMat.clone(),0,0);
		outputOfEyeGazeImg.draw(0, 0, WINWIDTH, WINHEIGHT);
		//outputOfSaliencyImg.draw(ofGetWidth() / 2, 0, ofGetWidth() / 2, ofGetHeight() / 2);

		break;
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	ofLogNotice() << "keyPressed: " << key;

	switch (key) {
	case 13:
		// "Enter"を押した時:

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

		enterCount++;
		enterState = true;

		break;
	case 8:
		// "BackSpace"を押した時:
		saliencyPointSave = saliencyPointBackUp;
		enterCount = 0;
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
void ofApp::dumpOSC(ofxOscMessage m) {
	string msg_string;
	msg_string = m.getAddress();
	for (int i = 0; i<m.getNumArgs(); i++) {
		msg_string += " ";
		if (m.getArgType(i) == OFXOSC_TYPE_INT32)
			msg_string += ofToString(m.getArgAsInt32(i));
		else if (m.getArgType(i) == OFXOSC_TYPE_FLOAT)
			msg_string += ofToString(m.getArgAsFloat(i));
		else if (m.getArgType(i) == OFXOSC_TYPE_STRING)
			msg_string += m.getArgAsString(i);
	}
	cout << msg_string << endl;
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}
