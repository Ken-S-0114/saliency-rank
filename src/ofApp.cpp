#include "ofApp.h"

#define PORT 8000
#define HOST "127.0.0.1"

//--------------------------------------------------------------
void ofApp::createSaliencyMap(cv::Mat img) {
	cv::Mat mat_gray, mat_gaus, saliencyMap_norm;

	cv::Ptr<cv::saliency::Saliency> saliencyAlgorithm;

	cv::Mat s1, s2, t1, t2;
	cv::Mat saliency_copy, saliency_copy2;
	cv::Mat saliencyMap_SPECTRAL_RESIDUAL, saliencyMap_SPECTRAL_RESIDUAL2;

	switch (loadState)
	{
	case ConstTools::FIRST:
		mat_copy = img.clone();

		cvtColor(img.clone(), mat_gray, cv::COLOR_BGR2GRAY);

		cv::GaussianBlur(mat_gray.clone(), mat_gaus, cv::Size(3, 3), 1, 1);

		saliencyAlgorithm = cv::saliency::StaticSaliencySpectralResidual::create();
		saliencyAlgorithm->computeSaliency(mat_gaus.clone(), saliencyMap_SPECTRAL_RESIDUAL);

		cv::normalize(saliencyMap_SPECTRAL_RESIDUAL.clone(), saliencyMap_norm, 0.0, 255.0, cv::NORM_MINMAX);

		saliencyMap_norm.convertTo(saliencyMap, CV_8UC3);

		//    minMaxLoc(saliencyMap, &minMax.min_val, &minMax.max_val, &minMax.min_loc, &minMax.max_loc, cv::Mat());

		s1 = saliencyMap.clone();
		ofxCv::toOf(s1, outputOfIMG_FIRST.outputOfSaliencyImg);

		outputOfIMG_FIRST.outputOfSaliencyImg.update();

		saliency_copy = saliencyMap.clone();

		for (int x = 0; x < saliency_copy.rows; ++x) {
			for (int y = 0; y < saliency_copy.cols; ++y) {
				saliency_copy.at<uchar>(x, y) = 255 - (int)saliency_copy.at<uchar>(x, y);
				//ofLogNotice()<<"(int)saliency_copy.at<uchar>("<<x<<","<<y<< ") : "<<(int)saliency_copy.at<uchar>( x, y );
			}
		}

		cv::applyColorMap(saliency_copy.clone(), saliencyMap_color, cv::COLORMAP_JET);

		s2 = saliencyMap_color.clone();

		ofxCv::toOf(s2, outputOfIMG_FIRST.outputOfHeatMapImg);
		outputOfIMG_FIRST.outputOfHeatMapImg.update();
		outputOfIMG_FIRST.outputOfHeatMapImg.save(outputfileName.outputOfSaliencyImg);
		break;

	case ConstTools::SECOND:
		//mat_copy = img.clone();

		cvtColor(s11.clone(), mat_gray, cv::COLOR_BGR2GRAY);

		cv::GaussianBlur(mat_gray.clone(), mat_gaus, cv::Size(3, 3), 1, 1);

		saliencyAlgorithm->computeSaliency(mat_gaus.clone(), saliencyMap_SPECTRAL_RESIDUAL2);

		cv::normalize(saliencyMap_SPECTRAL_RESIDUAL2.clone(), saliencyMap_norm, 0.0, 255.0, cv::NORM_MINMAX);

		saliencyMap_norm.convertTo(saliencyMap_second, CV_8UC3);

		//    minMaxLoc(saliencyMap, &minMax.min_val, &minMax.max_val, &minMax.min_loc, &minMax.max_loc, cv::Mat());

		t1 = saliencyMap_second.clone();
		ofxCv::toOf(t1, outputOfIMG_SECOND.outputOfSaliencyImg);

		outputOfIMG_SECOND.outputOfSaliencyImg.update();

		saliency_copy2 = saliencyMap.clone();

		for (int x = 0; x < saliency_copy2.rows; ++x) {
			for (int y = 0; y < saliency_copy2.cols; ++y) {
				saliency_copy2.at<uchar>(x, y) = 255 - (int)saliency_copy2.at<uchar>(x, y);
				//ofLogNotice()<<"(int)saliency_copy.at<uchar>("<<x<<","<<y<< ") : "<<(int)saliency_copy.at<uchar>( x, y );
			}
		}

		cv::applyColorMap(saliency_copy2.clone(), saliencyMap_color_second, cv::COLORMAP_JET);

		t2 = saliencyMap_color_second.clone();

		ofxCv::toOf(t2, outputOfIMG_SECOND.outputOfHeatMapImg);
		outputOfIMG_SECOND.outputOfHeatMapImg.update();
		outputOfIMG_SECOND.outputOfHeatMapImg.save(outputfileName2.outputOfSaliencyImg);
		break;
	}
}

//--------------------------------------------------------------
void ofApp::createWatershed(cv::Mat saliencyImg) {
	cv::Mat thresh;
	//cv::threshold(saliencyMap.clone(), thresh, 0, 255, cv::THRESH_OTSU);
	cv::threshold(saliencyImg.clone(), thresh, 40, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

	cv::Mat opening;
	cv::Mat kernel(3, 3, CV_8U, cv::Scalar(1));
	cv::morphologyEx(thresh.clone(), opening, cv::MORPH_OPEN, kernel, cv::Point(-1, -1), 2);

	cv::Mat sure_bg;
	cv::dilate(opening.clone(), sure_bg, kernel, cv::Point(-1, -1), 3);

	cv::Mat dist_transform;
	cv::distanceTransform(opening, dist_transform, CV_DIST_L2, 5);

	cv::Mat sure_fg;

	cv::minMaxLoc(dist_transform, &minMax.min_val, &minMax.max_val, &minMax.min_loc, &minMax.max_loc);
	cv::threshold(dist_transform.clone(), sure_fg, 0.3*minMax.max_val, 255, 0);

	dist_transform = dist_transform / minMax.max_val;

	cv::Mat s3 = sure_bg.clone();
	ofxCv::toOf(s3, outputOfIMG_FIRST.outputOfBackgroundImg);
	outputOfIMG_FIRST.outputOfBackgroundImg.update();

	cv::Mat unknown;
	cv::Mat sure_fg_uc1;
	sure_fg.convertTo(sure_fg_uc1, CV_8UC1);
	cv::subtract(sure_bg, sure_fg_uc1, unknown);

	cv::Mat s4 = unknown.clone();
	ofxCv::toOf(s4, outputOfIMG_FIRST.outputOfUnknownImg);
	outputOfIMG_FIRST.outputOfUnknownImg.update();

	int compCount = 0;

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	sure_fg.convertTo(sure_fg, CV_32SC1, 1.0);
	cv::findContours(sure_fg, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);
	if (contours.empty()) return;

	ofLogNotice() << "contours: " << &contours;

	cv::Mat markers = cv::Mat::zeros(sure_fg.rows, sure_fg.cols, CV_32SC1);

	int idx = 0;
	for (; idx >= 0; idx = hierarchy[idx][0], compCount++) {
		cv::drawContours(markers, contours, idx, cv::Scalar::all(compCount + 1), -1, 8, hierarchy, INT_MAX);
	}
	markers = markers + 1;


	for (int i = 0; i<markers.rows; i++) {
		for (int j = 0; j<markers.cols; j++) {
			unsigned char &v = unknown.at<unsigned char>(i, j);
			if (v == 255) {
				markers.at<int>(i, j) = 0;
			}
		}
	}

	cv::watershed(mat_original.clone(), markers);

	watershedHighest = cv::Mat::zeros(mat_original.size(), CV_8UC3);
	saliencyHighest = cv::Mat::zeros(mat_original.size(), CV_8UC3);

	cv::Mat wshed(markers.size(), CV_8UC3);

	std::vector<int> saliencyPoint(compCount + 1, 0);

	ofLogNotice() << "count: " << compCount;
	for (int i = 0; i < compCount + 1; i++)
	{
		int b = cv::theRNG().uniform(0, 255);
		int g = cv::theRNG().uniform(0, 255);
		int r = cv::theRNG().uniform(0, 255);

		colorTab.push_back(cv::Vec3b((uchar)b, (uchar)g, (uchar)r));
	}

	for (int i = 0; i < markers.rows; i++) {
		for (int j = 0; j < markers.cols; j++)
		{
			//ofLogNotice()<<"index: "<<index;
			int index = markers.at<int>(i, j);

			if (index == -1) {
				wshed.at<cv::Vec3b>(i, j) = cv::Vec3b(255, 255, 255);
			}
			else if (index <= 0) {
				wshed.at<cv::Vec3b>(i, j) = cv::Vec3b(0, 0, 0);
			}
			else if (index - 1 > compCount) {
				wshed.at<cv::Vec3b>(i, j) = cv::Vec3b(255, 0, 0);
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
				//mat_copy.at<cv::Vec3b>(i,j) = cv::Vec3b((uchar)255, (uchar)255, (uchar)255);
			}
		}
	}

	cvtColor(saliencyMap.clone(), imgG, cv::COLOR_GRAY2BGR);
	//wshed = wshed*0.5 + imgG*0.5;
	watershedHighest = watershedHighest*0.5 + imgG*0.5;
	saliencyHighest = saliencyHighest*0.5 + imgG*0.5;

	cv::Mat s5 = wshed.clone();
	ofxCv::toOf(s5, outputOfIMG_FIRST.outputOfWatershedImg);
	outputOfIMG_FIRST.outputOfWatershedImg.update();

	cv::Mat s6 = watershedHighest.clone();
	ofxCv::toOf(s6, outputOfIMG_FIRST.outputOfWatershedAfterImg);
	outputOfIMG_FIRST.outputOfWatershedAfterImg.update();
	outputOfIMG_FIRST.outputOfWatershedAfterImg.save(outputfileName.outputOfWatershedAfterImg);

	cv::Mat s7 = saliencyHighest.clone();
	ofxCv::toOf(s7, outputOfIMG_FIRST.outputOfWatershedHighestImg);
	outputOfIMG_FIRST.outputOfWatershedHighestImg.update();

	mat_mix = mat_original*0.2 + mat_copy*0.8;

	cv::Mat s8 = mat_mix.clone();
	ofxCv::toOf(s8, outputOfIMG_FIRST.outputOfSaliencyMapHighestImg);
	outputOfIMG_FIRST.outputOfSaliencyMapHighestImg.update();
	outputOfIMG_FIRST.outputOfSaliencyMapHighestImg.save(outputfileName.outputOfSaliencyMapHighestImg);

	markersSave = markers.clone();

	if (!saliencyPoint.empty()) {
		saliencyPointSave = saliencyPoint;
		saliencyPointBackUp = saliencyPoint;
		saliencyPoint.clear();
	}
}

//--------------------------------------------------------------
void ofApp::setup() {

	receiver.setup(PORT);
	eyeTrackState = ConstTools::STANDBY;
	loadState = ConstTools::FIRST;
	infomation = ConstTools::VIEW;

	ofLogNotice() << "ofGetScreenWidth: " << ofGetScreenWidth();
	ofLogNotice() << "ofGetScreenHeight: " << ofGetScreenHeight();

	eyeGazeMat = cv::Mat::zeros(WINHEIGHT, WINWIDTH, CV_8UC1);
	cv::Mat s9 = eyeGazeMat.clone();
	ofxCv::toOf(s9, outputOfIMG_FIRST.outputOfEyeGazeImg);
	outputOfIMG_FIRST.outputOfEyeGazeImg.update();

	heatmap.setup(WINWIDTH, WINHEIGHT, 32);

	enterState = false;
	enterCount = 0;

	enterCountString << "The " << enterCount + 1 << " most saliency place";

	inputOfImg.load(inputFileName.lenna);
	//inputOfImg.load(outputfileName.outputOfEyeGazeHeatMapImg);
	inputOfImg.update();

	mat_original = ofxCv::toCv(inputOfImg);

	createSaliencyMap(mat_original.clone());

	createWatershed(saliencyMap.clone());

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

			ofLogNotice() << "remoteEyeGazeX: " << remoteEyeGazeX;
			ofLogNotice() << "remoteEyeGazeY: " << remoteEyeGazeY;

			int remoteEyeGazeIntX = (int)remoteEyeGazeX;
			int remoteEyeGazeIntY = (int)remoteEyeGazeY;

			if (eyeTrackState == ConstTools::TRACKING)
			{
				if ((0 > remoteEyeGazeIntX) || (0 > remoteEyeGazeIntY))
				{
					return;
				}
				if ((remoteEyeGazeIntX <= WINWIDTH) && (remoteEyeGazeIntY <= WINHEIGHT))
				{
					ofLogNotice() << "remoteEyeGazeIntX(after): " << remoteEyeGazeIntX;
					ofLogNotice() << "remoteEyeGazeIntY(after): " << remoteEyeGazeIntY;
					if ((int)eyeGazeMat.at<uchar>(remoteEyeGazeIntY, remoteEyeGazeIntX) >= 255)
					{
						return;
					}
					ofLogNotice() << "eyeGazeMat: " << (int)eyeGazeMat.at<uchar>(remoteEyeGazeIntY, remoteEyeGazeIntX);
					eyeGazeMat.at<uchar>(remoteEyeGazeIntY, remoteEyeGazeIntX) = (int)eyeGazeMat.at<uchar>(remoteEyeGazeIntY, remoteEyeGazeIntX) + 254;
					cv::Mat s9 = eyeGazeMat.clone();
					ofxCv::toOf(s9, outputOfIMG_FIRST.outputOfEyeGazeImg);
					outputOfIMG_FIRST.outputOfEyeGazeImg.update();

					heatmap.addPoint(remoteEyeGazeIntX, remoteEyeGazeIntY);
				}
			}
		}
	}

	if (eyeTrackState == ConstTools::TRACKING)
	{
		heatmap.update(OFX_HEATMAP_CS_SPECTRAL_SOFT);
	}

	if (enterState) {
		enterCountString.str("");
		enterCountString.clear(stringstream::goodbit);

		int maxValue = *std::max_element(saliencyPointSave.begin(), saliencyPointSave.end());

		if (maxValue != 0) {

			for (int i = 0; i<saliencyPointSave.size(); i++) {
				ofLogNotice() << "saliencyPoint[" << i << "]: " << saliencyPointSave[i];
			}

			iter = std::max_element(saliencyPointSave.begin(), saliencyPointSave.end());
			saliencyPointMaxIndex = std::distance(saliencyPointSave.begin(), iter);
			ofLogNotice() << "Index of max element: " << saliencyPointMaxIndex;

			saliencyHighest = cv::Mat::zeros(saliencyHighest.size(), CV_8UC3);
			mat_copy = mat_original.clone();
			
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
			ofxCv::toOf(s7, outputOfIMG_FIRST.outputOfWatershedHighestImg);
			outputOfIMG_FIRST.outputOfWatershedHighestImg.update();

			mat_mix = mat_original*0.2 + mat_copy*0.8;
			
			cv::Mat s8 = mat_mix.clone();
			ofxCv::toOf(s8, outputOfIMG_FIRST.outputOfSaliencyMapHighestImg);
			outputOfIMG_FIRST.outputOfSaliencyMapHighestImg.update();
			outputOfIMG_FIRST.outputOfSaliencyMapHighestImg.save(outputfileName.outputOfSaliencyMapHighestImg);

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
		outputOfIMG_FIRST.outputOfSaliencyMapHighestImg.draw(0, ofGetHeight() / 2, ofGetWidth() / 2, ofGetHeight() / 2);

		// Label
		ofDrawBitmapStringHighlight("original", ofGetWidth() / 2 + 20, 20);
		ofDrawBitmapStringHighlight("saliencyMap-watershed", ofGetWidth() / 2 + 20, ofGetHeight() / 2 + 20);

		// Label
		ofDrawBitmapStringHighlight("Enter: Next HighSaliency Place\nDelete: Reset",
			ofGetWidth() - ofGetWidth() / 3 -20, 20);
		ofDrawBitmapStringHighlight(enterCountString.str(), ofGetWidth() / 2 + 20, ofGetHeight() / 2 + 50);

		ofSetWindowTitle("RELEASE");
		break;

	case ConstTools::DEBUG:
		inputOfImg.draw(0, 0, ofGetWidth() / 3, ofGetHeight() / 3);
		//ofxCv::drawMat(saliencyMap.clone(), ofGetWidth() / 3, 0, ofGetWidth() / 3, ofGetHeight() / 3);
		outputOfIMG_FIRST.outputOfSaliencyImg.draw(ofGetWidth() / 3, 0, ofGetWidth() / 3, ofGetHeight() / 3);
		//ofxCv::drawMat(saliencyMap_color.clone(), ofGetWidth() - ofGetWidth() / 3, 0, ofGetWidth() / 3, ofGetHeight() / 3);
		outputOfIMG_FIRST.outputOfHeatMapImg.draw(ofGetWidth() - ofGetWidth() / 3, 0, ofGetWidth() / 3, ofGetHeight() / 3);
		//ofxCv::drawMat(sure_bg.clone(), 0, ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);
		outputOfIMG_FIRST.outputOfBackgroundImg.draw(0, ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);
		//ofxCv::drawMat(unknown.clone(), ofGetWidth() / 3, ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);
		outputOfIMG_FIRST.outputOfUnknownImg.draw(ofGetWidth() / 3, ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);
		//ofxCv::drawMat(wshed.clone(), ofGetWidth() - ofGetWidth() / 3, ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);
		outputOfIMG_FIRST.outputOfWatershedImg.draw(ofGetWidth() - ofGetWidth() / 3, ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);
		//ofxCv::drawMat(watershedHighest.clone(), 0, ofGetHeight() - ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);
		outputOfIMG_FIRST.outputOfWatershedAfterImg.draw(0, ofGetHeight() - ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);
		//ofxCv::drawMat(saliencyHighest.clone(), ofGetWidth() / 3, ofGetHeight() - ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);
		outputOfIMG_FIRST.outputOfWatershedHighestImg.draw(ofGetWidth() / 3, ofGetHeight() - ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);
		//ofxCv::drawMat(mat_mix.clone(), ofGetWidth() - ofGetWidth() / 3, ofGetHeight() - ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);
		outputOfIMG_FIRST.outputOfSaliencyMapHighestImg.draw(ofGetWidth() - ofGetWidth() / 3, ofGetHeight() - ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);

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
		
		ofSetWindowTitle("DEBUG"); 
		
		break;

	case ConstTools::EYETRACK:
		//ofxCv::drawMat(eyeGazeMat.clone(),0,0);
		outputOfIMG_FIRST.outputOfEyeGazeImg.draw(0, 0, WINWIDTH, WINHEIGHT);
		//outputOfSaliencyImg.draw(ofGetWidth() / 2, 0, ofGetWidth() / 2, ofGetHeight() / 2);

		ofSetWindowTitle("EYETRACK");

		switch (eyeTrackState)
		{
		case ConstTools::STANDBY:
			ofDrawBitmapStringHighlight("STANDBY(Pixels): Please Space Key", 20, 20);
			break;
		case ConstTools::TRACKING:
			ofDrawBitmapStringHighlight("TRACKING", 20, 20);
			break;
		}

		break;

	case ConstTools::EYETRACKHEATMAP:
		ofBackground(0, 0, 0);
		ofSetColor(255, 255);
		heatmap.draw(0, 0);

		ofSetWindowTitle("EYETRACKHEATMAP");

		switch (eyeTrackState)
		{
		case ConstTools::STANDBY:
			ofDrawBitmapStringHighlight("STANDBY(HeatMap): Please Space Key", 20, 20);
			break;
		case ConstTools::TRACKING:
			ofDrawBitmapStringHighlight("TRACKING: Save S Key", 20, 20);
			break;
		case ConstTools::SAVE:
			ofDrawBitmapStringHighlight("Saved EyeGazeHeatMap", 20, 20);
			break;
		}

		break;

	case ConstTools::IMAGEVIEW:
		inputOfImg.draw(0, 0, ofGetWidth(), ofGetHeight());
		ofSetWindowTitle("IMAGEVIEW");

		break;

	case ConstTools::RESULT:
		outputOfIMG_FIRST.outputOfResultImg.draw(0, 0, ofGetWidth()/2, ofGetHeight()/2);
		outputOfIMG_FIRST.outputOfHeatMapImg.draw(ofGetWidth() / 2, 0, ofGetWidth() / 2, ofGetHeight() / 2);
		outputOfIMG_SECOND.outputOfHeatMapImg.draw(0, ofGetHeight() / 2, ofGetWidth() / 2, ofGetHeight() / 2);
		ofDrawBitmapStringHighlight("View EyeGazeHeatMap", 20, 20);
		ofSetWindowTitle("RESULT");

	}

	switch (infomation)
	{
	case ConstTools::VIEW:
		ofDrawBitmapStringHighlight("SELECT KEY PRESSED\r\n  *Z: RELEASE\n  *X: DEBUG\n  *C: EYETRACK\n  *V: EYETRACKHEATMAP \n  *B: RESULT\n  *N: IMAGEVIEW",
			ofGetWidth() - ofGetWidth() / 6, 20);
		break;
	case ConstTools::HIDE:
		break;

	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	ofLogNotice() << "keyPressed: " << key;

	switch (key) {
	case 13:
		// "Enter"‚ð‰Ÿ‚µ‚½Žž:
		saliencyPointSave[saliencyPointMaxIndex] = 0;

		enterCount++;
		enterState = true;

		break;
	case 8:
		// "BackSpace"‚ð‰Ÿ‚µ‚½Žž:
		saliencyPointSave = saliencyPointBackUp;
		enterCount = 0;
		enterState = true;

		break;
	case ' ':
		// "Space"‚ð‰Ÿ‚µ‚½Žž:
		if ((use == ConstTools::EYETRACK) || (use == ConstTools::EYETRACKHEATMAP) || (use == ConstTools::IMAGEVIEW))
		{
			if (eyeTrackState == ConstTools::STANDBY) {
				eyeTrackState = ConstTools::TRACKING;
			}
			else {
				eyeTrackState = ConstTools::STANDBY;
			}
		}
		break;

	case 's':
		// "S"‚ð‰Ÿ‚µ‚½Žž:
		if (eyeTrackState == ConstTools::STANDBY && ((use == ConstTools::EYETRACK) || (use == ConstTools::EYETRACKHEATMAP)))
		{
			heatmap.save(outputfileName.outputOfEyeGazeHeatMapImg);
			eyeTrackState = ConstTools::SAVE;
		}
		break;

	case 'r':
		heatmap.clear();
		break;
	case 'i':
		switch (infomation)
		{
		case ConstTools::VIEW:
			infomation = ConstTools::HIDE;
			break;
		case ConstTools::HIDE:
			infomation = ConstTools::VIEW;
			break;
		}
		break;
		//-------------   ŠÂ‹«   ------------------
	case 'z':
		// "Z"‚ð‰Ÿ‚µ‚½Žž: release
		use = ConstTools::RELEASE;
		break;
	case 'x':
		// "X"‚ð‰Ÿ‚µ‚½Žž: debug
		use = ConstTools::DEBUG;
		break;
	case 'c':
		// "C"‚ð‰Ÿ‚µ‚½Žž: eyeTrack
		use = ConstTools::EYETRACK;
		break;
	case 'v':
		// "V"‚ð‰Ÿ‚µ‚½Žž: eyeTrackHeatMap
		use = ConstTools::EYETRACKHEATMAP;
		break;
	case 'b':
		// "B"‚ð‰Ÿ‚µ‚½Žž: result
		use = ConstTools::RESULT;
		if (loadOfImage.load(outputfileName.outputOfEyeGazeHeatMapImg))
		{
			loadOfImage.update();

			cv::Mat loadMat = ofxCv::toCv(loadOfImage);

			cv::Mat s10 = loadMat.clone();

			ofxCv::toOf(s10, outputOfIMG_FIRST.outputOfResultImg);
			outputOfIMG_FIRST.outputOfResultImg.update();

			//loadState = ConstTools::SECOND; 
			//s11 = s10.clone();
			//createSaliencyMap(s11);
		}
		break;
	case 'n':
		// "N"‚ð‰Ÿ‚µ‚½Žž: imageView
		use = ConstTools::IMAGEVIEW;

		break;

	default:
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
