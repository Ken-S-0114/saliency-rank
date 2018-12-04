#include "ofApp.h"

#define PORT 8000
#define HOST "127.0.0.1"

#define THRESH_PIC 40
#define THRESH_EYE 20

#define THRESH_MAXVAL_PIC 0.3
#define THRESH_MAXVAL_EYE 0.3

//--------------------------------------------------------------
void ofApp::setup() {

	receiver.setup(PORT);

	eyeTrackState = ConstTools::STANDBY;
	loadState = ConstTools::PICLOAD;
	infomation = ConstTools::VIEW;
	mode = ConstTools::SLEEP;

	ofLogNotice() << "ofGetScreenWidth: " << ofGetScreenWidth();
	ofLogNotice() << "ofGetScreenHeight: " << ofGetScreenHeight();

	viewEyeMat.eyeGazeMat = cv::Mat::zeros(WINHEIGHT, WINWIDTH, CV_8UC1);
	cv::Mat s9 = viewEyeMat.eyeGazeMat.clone();
	ofxCv::toOf(s9, outputOfEyeIMG.eyeGaze);
	outputOfEyeIMG.eyeGaze.update();

	heatMap.gray.setup(WINWIDTH, WINHEIGHT, 32);
	heatMap.grays.setup(WINWIDTH, WINHEIGHT, 32);
	heatMap.spectral.setup(WINWIDTH, WINHEIGHT, 32);
	heatMap.spectrals.setup(WINWIDTH, WINHEIGHT, 32);

	enterState = ConstTools::NONE;
	enterPicCount = 0;
	enterEyeCount = 0;
	enterPicCountString << "The " << enterPicCount + 1 << " most saliency place";
	enterEyeCountString << "The " << enterEyeCount + 1 << " most saliency place";

	std::string inputFilePath;
	//--------------------------------------------------------------
	// Add image file !!
	fileName = inputFileName.mul.brothers;
	inputFilePath = prefixPath.image + "/" + fileName + ext.jpg;
	folderName = prefixPath.image;
	//--------------------------------------------------------------
	// Add image-IPU file !!
	//fileName = inputIPUFileName.obj.tree;
	//inputFilePath = prefixPath.image_IPU + "/" + fileName + ext.jpg;
	//folderName = prefixPath.image_IPU;
	//--------------------------------------------------------------
	//fileName = inputMockFileName.dog;
	//inputFilePath = prefixPath.sampleImage + "/" + fileName + ext.jpg;
	//--------------------------------------------------------------

	eyeGazePath = prefixPath.eyeGaze + "/" + folderName + "/" + fileName + "/" + outputOfEyeFileName.eyeGazeHeatMapGraySoft + ext.png;

	if (inputOfImg.load(inputFilePath))
	{
		inputOfImg.update();
		originalPicMat.original = ofxCv::toCv(inputOfImg);
		createSaliencyMap(originalPicMat.original.clone());

		ofLogNotice() << "viewPicMat.saliencyMap.type: " << viewPicMat.saliencyMap.type();
		ofLogNotice() << "viewPicMat.saliencyMap.depth: " << viewPicMat.saliencyMap.depth();
		ofLogNotice() << "viewPicMat.saliencyMap.channels: " << viewPicMat.saliencyMap.channels();

		createWatershed(viewPicMat.saliencyMap.clone());
	}
	else {
		ofLogWarning() << "Can not find file";
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

			int X = (int)remoteEyeGazeX;
			int Y = (int)remoteEyeGazeY;

			if (eyeTrackState == ConstTools::TRACKING)
			{
				if ((0 > X) || (0 > Y))
				{
					return;
				}
				if ((X <= WINWIDTH) && (Y <= WINHEIGHT))
				{
					/*ofLogNotice() << "X(after): " << X;
					ofLogNotice() << "Y(after): " << Y;*/
					if ((int)viewEyeMat.eyeGazeMat.at<uchar>(Y, X) >= 255)
					{
						return;
					}
					//ofLogNotice() << "eyeGazeMat: " << (int)eyeGazeMat.at<uchar>(Y, X);
					viewEyeMat.eyeGazeMat.at<uchar>(Y, X) += 254;
					cv::Mat s9 = viewEyeMat.eyeGazeMat.clone();
					ofxCv::toOf(s9, outputOfEyeIMG.eyeGaze);
					outputOfEyeIMG.eyeGaze.update();

					
					heatMap.gray.addPoint(X, Y);
					heatMap.grays.addPoint(X, Y);
					heatMap.spectral.addPoint(X, Y);
					heatMap.spectrals.addPoint(X, Y);
				}
			}
		}
	}

	if (eyeTrackState == ConstTools::TRACKING)
	{
		heatMap.gray.update(OFX_HEATMAP_CS_GRAYS);
		heatMap.grays.update(OFX_HEATMAP_CS_GRAYS_SOFT);
		heatMap.spectral.update(OFX_HEATMAP_CS_SPECTRAL);
		heatMap.spectrals.update(OFX_HEATMAP_CS_SPECTRAL_SOFT);
	}

	if (enterState == ConstTools::SALIENCYMAP) {
		enterPicCountString.str("");
		enterPicCountString.clear(stringstream::goodbit);

		int maxValue = *std::max_element(saliencyPicPoint.saved.begin(), saliencyPicPoint.saved.end());

		if (maxValue != 0) {

			for (int i = 0; i < saliencyPicPoint.saved.size(); i++) {
				ofLogNotice() << "saliencyPoint[" << i << "]: " << saliencyPicPoint.saved[i];
			}

			maxSaliencyPicPoint.iter = std::max_element(saliencyPicPoint.saved.begin(), saliencyPicPoint.saved.end());
			maxSaliencyPicPoint.maxIndex = std::distance(saliencyPicPoint.saved.begin(), maxSaliencyPicPoint.iter);
			ofLogNotice() << "Index of max element: " << maxSaliencyPicPoint.maxIndex;

			viewPicMat.saliencyHighest = cv::Mat::zeros(viewPicMat.saliencyHighest.size(), CV_8UC3);
			originalPicMat.copy = originalPicMat.original.clone();

			for (int i = 0; i < markersPicSave.rows; i++) {
				for (int j = 0; j < markersPicSave.cols; j++)
				{
					int index = markersPicSave.at<int>(i, j);
					if (index == maxSaliencyPicPoint.maxIndex + 1) {
						viewPicMat.saliencyHighest.at<cv::Vec3b>(i, j) = colorPicTab[index - 1];
					}
					else {
						originalPicMat.copy.at<cv::Vec3b>(i, j) = cv::Vec3b((uchar)0, (uchar)0, (uchar)0);
						//mat_copy.at<cv::Vec3b>(i,j) = cv::Vec3b((uchar)255, (uchar)255, (uchar)255);
					}
				}
			}

			viewPicMat.saliencyHighest = viewPicMat.saliencyHighest*0.5 + picImgG*0.5;

			cv::Mat s7 = viewPicMat.saliencyHighest.clone();
			ofxCv::toOf(s7, outputOfPicIMG.watershedHighest);
			outputOfPicIMG.watershedHighest.update();

			viewPicMat.matMix = originalPicMat.original*0.2 + originalPicMat.copy*0.8;

			std::ostringstream number;
			number << enterPicCount+1;

			cv::Mat s8 = viewPicMat.matMix.clone();
			ofxCv::toOf(s8, outputOfPicIMG.saliencyMapHighest);
			outputOfPicIMG.saliencyMapHighest.update();
			outputOfPicIMG.saliencyMapHighest
				.save(prefixPath.picture + "/" + folderName + "/" + fileName + "/" + outputOfPicFileName.saliencyMapHighest + "_" + number.str() + ext.png);

			enterPicCountString << "The " << enterPicCount + 1 << " most saliency place";

		}
		else {
			enterPicCountString << "Finish";
		};

		enterState = ConstTools::NONE;
	}
	else if (enterState == ConstTools::EYEGAZE) {
		enterEyeCountString.str("");
		enterEyeCountString.clear(stringstream::goodbit);

		int maxValue = *std::max_element(saliencyEyePoint.saved.begin(), saliencyEyePoint.saved.end());

		if (maxValue != 0) {

			for (int i = 0; i < saliencyEyePoint.saved.size(); i++) {
				ofLogNotice() << "saliencyPoint[" << i << "]: " << saliencyEyePoint.saved[i];
			}

			maxSaliencyEyePoint.iter = 
				std::max_element(saliencyEyePoint.saved.begin(), saliencyEyePoint.saved.end());
			maxSaliencyEyePoint.maxIndex = 
				std::distance(saliencyEyePoint.saved.begin(), maxSaliencyEyePoint.iter);
			ofLogNotice() << "Index of max element: " << maxSaliencyEyePoint.maxIndex;


			viewEyeMat.saliencyHighest = cv::Mat::zeros(viewEyeMat.saliencyHighest.size(), CV_8UC3);
			originalEyeMat.copy = originalEyeMat.original.clone();

			for (int i = 0; i < markersEyeSave.rows; i++) {
				for (int j = 0; j < markersEyeSave.cols; j++)
				{
					int index = markersEyeSave.at<int>(i, j);
					if (index == maxSaliencyEyePoint.maxIndex + 1) {
						viewEyeMat.saliencyHighest.at<cv::Vec3b>(i, j) = colorEyeTab[index - 1];
					}
					else {
						originalEyeMat.copy.at<cv::Vec3b>(i, j) = cv::Vec3b((uchar)0, (uchar)0, (uchar)0);
						//mat_copy.at<cv::Vec3b>(i,j) = cv::Vec3b((uchar)255, (uchar)255, (uchar)255);
					}
				}
			}

			viewEyeMat.saliencyHighest = viewEyeMat.saliencyHighest*0.5 + eyeImgG*0.5;

			cv::Mat s7 = viewEyeMat.saliencyHighest.clone();
			ofxCv::toOf(s7, outputOfEyeIMG.watershedHighest);
			outputOfEyeIMG.watershedHighest.update();

			viewEyeMat.matMix = originalEyeMat.original*0.2 + originalEyeMat.copy*0.8;

			std::ostringstream number;
			number << enterEyeCount + 1;

			cv::Mat s8 = viewEyeMat.matMix.clone();
			ofxCv::toOf(s8, outputOfEyeIMG.saliencyMapHighest);
			outputOfEyeIMG.saliencyMapHighest.update();
			outputOfEyeIMG.saliencyMapHighest
				.save(prefixPath.eyeGaze + "/" + folderName + "/" + fileName + "/" + outputOfEyeFileName.saliencyMapHighest + "_" + number.str() + ext.png);

			enterEyeCountString << "The " << enterEyeCount + 1 << " most saliency place";

		}
		else {
			enterEyeCountString << "Finish";
		};

		enterState = ConstTools::NONE;
	}
}

//--------------------------------------------------------------
void ofApp::draw() {

	switch (mode) {
	case ConstTools::RELEASE:
		inputOfImg.draw(0, 0, ofGetWidth() / 2, ofGetHeight() / 2);
		outputOfPicIMG.saliencyMapHighest.draw(0, ofGetHeight() / 2, ofGetWidth() / 2, ofGetHeight() / 2);

		// Label
		ofDrawBitmapStringHighlight("original", ofGetWidth() / 2 + 20, 20);
		ofDrawBitmapStringHighlight("saliencyMap-watershed", ofGetWidth() / 2 + 20, ofGetHeight() / 2 + 20);

		// Label
		ofDrawBitmapStringHighlight("Enter: Next HighSaliency Place\nDelete: Reset",
			ofGetWidth() - ofGetWidth() / 3 - 20, 20);

		ofDrawBitmapStringHighlight(enterPicCountString.str(), ofGetWidth() / 2 + 20, ofGetHeight() / 2 + 50);

		ofSetWindowTitle("RELEASE");
		break;

	case ConstTools::DEBUG:
		inputOfImg.draw(0, 0, ofGetWidth() / 3, ofGetHeight() / 3);
		outputOfPicIMG.saliencyMap.draw(ofGetWidth() / 3, 0, ofGetWidth() / 3, ofGetHeight() / 3);
		outputOfPicIMG.heatMap.draw(ofGetWidth() - ofGetWidth() / 3, 0, ofGetWidth() / 3, ofGetHeight() / 3);
		outputOfPicIMG.background.draw(0, ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);
		outputOfPicIMG.unknown.draw(ofGetWidth() / 3, ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);
		outputOfPicIMG.watershed.draw(ofGetWidth() - ofGetWidth() / 3, ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);
		outputOfPicIMG.watershedAfter.draw(0, ofGetHeight() - ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);
		outputOfPicIMG.watershedHighest.draw(ofGetWidth() / 3, ofGetHeight() - ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);
		outputOfPicIMG.saliencyMapHighest.draw(ofGetWidth() - ofGetWidth() / 3, ofGetHeight() - ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);

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
		outputOfEyeIMG.eyeGaze.draw(0, 0, WINWIDTH, WINHEIGHT);
		//saliencyMap.draw(ofGetWidth() / 2, 0, ofGetWidth() / 2, ofGetHeight() / 2);

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
		//heatMap.gray.draw(0, 0);
		heatMap.grays.draw(0, 0);
		//heatMap.spectral.draw(0, 0);
		//heatMap.spectrals.draw(0, 0);

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
		switch (eyeTrackState)
		{
			case ConstTools::SAVE:
			ofDrawBitmapStringHighlight("Saved EyeGazeHeatMap", 20, 20);
			break;
		}

		break;

	case ConstTools::RESULT:

		//outputOfPicIMG.heatMap.draw(0, 0, ofGetWidth() / 3, ofGetHeight() / 3);
		outputOfEyeIMG.eyeGazeResult.draw(0, 0, ofGetWidth() / 3, ofGetHeight() / 3);
		outputOfEyeIMG.loadMat8UC1.draw(ofGetWidth() / 3, 0, ofGetWidth() / 3, ofGetHeight() / 3);
		outputOfEyeIMG.gray.draw(ofGetWidth() - ofGetWidth() / 3, 0, ofGetWidth() / 3, ofGetHeight() / 3);
		outputOfEyeIMG.background.draw(0, ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);
		outputOfEyeIMG.unknown.draw(ofGetWidth() / 3, ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);
		outputOfEyeIMG.watershed.draw(ofGetWidth() - ofGetWidth() / 3, ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);
		outputOfEyeIMG.watershedAfter.draw(0, ofGetHeight() - ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);
		outputOfEyeIMG.watershedHighest.draw(ofGetWidth() / 3, ofGetHeight() - ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);
		outputOfEyeIMG.saliencyMapHighest.draw(ofGetWidth() - ofGetWidth() / 3, ofGetHeight() - ofGetHeight() / 3, ofGetWidth() / 3, ofGetHeight() / 3);
		
		ofDrawBitmapStringHighlight("View EyeGazeHeatMap", 20, 20);
		//ofDrawBitmapStringHighlight(enterEyeCountString.str(), ofGetWidth() / 2 + 20, ofGetHeight() / 2 + 50);
		ofSetWindowTitle("RESULT");
		break;

	case ConstTools::SLEEP:
		ofBackground(0, 0, 0);
		break;

	}

	switch (infomation)
	{
	case ConstTools::VIEW:
		ofDrawBitmapStringHighlight(
			"SELECT KEY PRESSED\r\n  *Z: RELEASE\n  *X: DEBUG\n  *C: EYETRACK\n  *V: EYETRACKHEATMAP \n  *B: IMAGEVIEW\n  *N: RESULT\n  *M: SLEEP",
			ofGetWidth() - ofGetWidth() / 6, 20
		);
		break;
	case ConstTools::HIDE:
		break;

	}
}

//--------------------------------------------------------------
void ofApp::createSaliencyMap(cv::Mat img) {
	cv::Mat mat_gray, mat_gaus, saliencyMap_norm;

	cv::Ptr<cv::saliency::Saliency> saliencyAlgorithm;

	cv::Mat s1, s2;
	cv::Mat saliency_copy;
	cv::Mat saliencyMap_SPECTRAL_RESIDUAL;

	originalPicMat.copy = img.clone();

	cvtColor(img.clone(), mat_gray, cv::COLOR_BGR2GRAY);

	cv::GaussianBlur(mat_gray.clone(), mat_gaus, cv::Size(3, 3), 1, 1);

	saliencyAlgorithm = cv::saliency::StaticSaliencySpectralResidual::create();
	saliencyAlgorithm->computeSaliency(mat_gaus.clone(), saliencyMap_SPECTRAL_RESIDUAL);

	cv::normalize(saliencyMap_SPECTRAL_RESIDUAL.clone(), saliencyMap_norm, 0.0, 255.0, cv::NORM_MINMAX);

	saliencyMap_norm.convertTo(viewPicMat.saliencyMap, CV_8UC3);

	//    minMaxLoc(saliencyMap, &minMax.min_val, &minMax.max_val, &minMax.min_loc, &minMax.max_loc, cv::Mat());

	s1 = viewPicMat.saliencyMap.clone();
	ofxCv::toOf(s1, outputOfPicIMG.saliencyMap);
	outputOfPicIMG.saliencyMap.update();

	saliency_copy = viewPicMat.saliencyMap.clone();

	for (int x = 0; x < saliency_copy.rows; ++x) {
		for (int y = 0; y < saliency_copy.cols; ++y) {
			saliency_copy.at<uchar>(x, y) = 255 - (int)saliency_copy.at<uchar>(x, y);
			//ofLogNotice()<<"(int)saliency_copy.at<uchar>("<<x<<","<<y<< ") : "<<(int)saliency_copy.at<uchar>( x, y );
		}
	}

	cv::applyColorMap(saliency_copy.clone(), viewPicMat.saliencyMapColor, cv::COLORMAP_JET);

	s2 = viewPicMat.saliencyMapColor.clone();

	ofxCv::toOf(s2, outputOfPicIMG.heatMap);
	outputOfPicIMG.heatMap.update();
	outputOfPicIMG.heatMap.save(prefixPath.picture + "/" + folderName + "/" + fileName + "/" + outputOfPicFileName.saliencyMap + ext.jpg);

}

//--------------------------------------------------------------
void ofApp::createWatershed(cv::Mat saliencyImg) {
	if (loadState == ConstTools::PICLOAD)
	{
		cv::Mat thresh;
		//cv::threshold(saliencyImg.clone(), thresh, 0, 255, cv::THRESH_OTSU);
		cv::threshold(saliencyImg.clone(), thresh, THRESH_PIC, 255, CV_THRESH_BINARY);

		cv::Mat opening;
		cv::Mat kernel(3, 3, CV_8U, cv::Scalar(1));
		cv::morphologyEx(thresh.clone(), opening, cv::MORPH_OPEN, kernel, cv::Point(-1, -1), 2);

		cv::Mat sure_bg;
		cv::dilate(opening.clone(), sure_bg, kernel, cv::Point(-1, -1), 3);

		cv::Mat dist_transform;
		cv::distanceTransform(opening, dist_transform, CV_DIST_L2, 5);

		cv::Mat sure_fg;

		cv::Point min_loc, max_loc;
		double min_val, max_val;

		cv::minMaxLoc(dist_transform, &min_val, &max_val, &min_loc, &max_loc);
		cv::threshold(dist_transform.clone(), sure_fg, THRESH_MAXVAL_PIC*max_val, 255, 0);

		dist_transform = dist_transform / max_val;

		cv::Mat s3 = sure_bg.clone();

		ofxCv::toOf(s3, outputOfPicIMG.background);
		outputOfPicIMG.background.update();


		cv::Mat unknown;
		cv::Mat sure_fg_uc1;
		sure_fg.convertTo(sure_fg_uc1, CV_8UC1);
		cv::subtract(sure_bg, sure_fg_uc1, unknown);

		cv::Mat s4 = unknown.clone();

		ofxCv::toOf(s4, outputOfPicIMG.unknown);
		outputOfPicIMG.unknown.update();

		int compCount = 0;

		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;
		sure_fg.convertTo(sure_fg, CV_32SC1, 1.0);
		cv::findContours(sure_fg, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);
		if (contours.empty()) return;

		cv::Mat markers = cv::Mat::zeros(sure_fg.rows, sure_fg.cols, CV_32SC1);

		int idx = 0;
		for (; idx >= 0; idx = hierarchy[idx][0], compCount++) {
			cv::drawContours(markers, contours, idx, cv::Scalar::all(compCount + 1), -1, 8, hierarchy, INT_MAX);
		}
		markers = markers + 1;

		for (int i = 0; i < markers.rows; i++) {
			for (int j = 0; j < markers.cols; j++) {
				unsigned char &v = unknown.at<unsigned char>(i, j);
				if (v == 255) {
					markers.at<int>(i, j) = 0;
				}
			}
		}

		cv::watershed(originalPicMat.original.clone(), markers);
		viewPicMat.watershedHighest = cv::Mat::zeros(originalPicMat.original.size(), CV_8UC3);
		viewPicMat.saliencyHighest = cv::Mat::zeros(originalPicMat.original.size(), CV_8UC3);

		cv::Mat wshed(markers.size(), CV_8UC3);

		std::vector<int> saliencyPoint(compCount + 1, 0);

		ofLogNotice() << "count: " << compCount;
		for (int i = 0; i < compCount + 1; i++)
		{
			int b = cv::theRNG().uniform(0, 255);
			int g = cv::theRNG().uniform(0, 255);
			int r = cv::theRNG().uniform(0, 255);

			colorPicTab.push_back(cv::Vec3b((uchar)b, (uchar)g, (uchar)r));
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
					wshed.at<cv::Vec3b>(i, j) = colorPicTab[index - 1];
				}
				else {
					viewPicMat.watershedHighest.at<cv::Vec3b>(i, j) = colorPicTab[index - 1];
					if (saliencyPoint[index - 1] < (int)saliencyImg.at<uchar>(i, j)) {
						saliencyPoint[index - 1] = (int)saliencyImg.at<uchar>(i, j);
					}
				}
			}
		}

		for (int i = 0; i < saliencyPoint.size(); i++) {
			ofLogNotice() << "saliencyPoint[" << i << "]: " << saliencyPoint[i];
		}

		maxSaliencyPicPoint.iter = std::max_element(saliencyPoint.begin(), saliencyPoint.end());
		maxSaliencyPicPoint.maxIndex = std::distance(saliencyPoint.begin(), maxSaliencyPicPoint.iter);
		ofLogNotice() << "Index of max element: " << maxSaliencyPicPoint.maxIndex;

		for (int i = 0; i < markers.rows; i++) {
			for (int j = 0; j < markers.cols; j++)
			{
				int index = markers.at<int>(i, j);
				if (index == maxSaliencyPicPoint.maxIndex + 1) {
					viewPicMat.saliencyHighest.at<cv::Vec3b>(i, j) = colorPicTab[index - 1];
				}
				else {
					originalPicMat.copy.at<cv::Vec3b>(i, j) = cv::Vec3b((uchar)0, (uchar)0, (uchar)0);
					//mat_copy.at<cv::Vec3b>(i,j) = cv::Vec3b((uchar)255, (uchar)255, (uchar)255);
				}
			}
		}

		cvtColor(saliencyImg.clone(), picImgG, cv::COLOR_GRAY2BGR);

		//wshed = wshed*0.5 + picImgG*0.5;
		viewPicMat.watershedHighest = viewPicMat.watershedHighest*0.5 + picImgG*0.5;
		viewPicMat.saliencyHighest = viewPicMat.saliencyHighest*0.5 + picImgG*0.5;

		cv::Mat s5 = wshed.clone();

		cv::Mat s6 = viewPicMat.watershedHighest.clone();

		cv::Mat s7 = viewPicMat.saliencyHighest.clone();

		cvtColor(saliencyImg.clone(), picImgG, cv::COLOR_GRAY2BGR);

		ofxCv::toOf(s5, outputOfPicIMG.watershed);
		outputOfPicIMG.watershed.update();

		ofxCv::toOf(s6, outputOfPicIMG.watershedAfter);
		outputOfPicIMG.watershedAfter.update();
		outputOfPicIMG.watershedAfter
			.save(prefixPath.picture + "/" + folderName + "/" + fileName + "/" + outputOfPicFileName.watershed + ext.jpg);

		ofxCv::toOf(s7, outputOfPicIMG.watershedHighest);
		outputOfPicIMG.watershedHighest.update();

		viewPicMat.matMix = originalPicMat.original*0.2 + originalPicMat.copy*0.8;

		cv::Mat s8 = viewPicMat.matMix.clone();

		ofxCv::toOf(s8, outputOfPicIMG.saliencyMapHighest);
		outputOfPicIMG.saliencyMapHighest.update();
		outputOfPicIMG.saliencyMapHighest
			.save(prefixPath.picture + "/" + folderName + "/" + fileName + "/" + outputOfPicFileName.saliencyMapHighest + "_1" + ext.jpg);

		markersPicSave = markers.clone();

		if (!saliencyPoint.empty()) {
			saliencyPicPoint.saved = saliencyPoint;
			saliencyPicPoint.backup = saliencyPoint;
			saliencyPoint.clear();
		}
	}
	else if (loadState == ConstTools::EYELOAD)
	{
		cv::Mat thresh;
		cv::threshold(saliencyImg.clone(), thresh, THRESH_EYE, 255, CV_THRESH_BINARY);

		cv::Mat opening;
		cv::Mat kernel(3, 3, CV_8U, cv::Scalar(1));
		cv::morphologyEx(thresh.clone(), opening, cv::MORPH_OPEN, kernel, cv::Point(-1, -1), 2);

		cv::Mat sure_bg;
		cv::dilate(opening.clone(), sure_bg, kernel, cv::Point(-1, -1), 3);

		cv::Mat dist_transform;
		cv::distanceTransform(opening, dist_transform, CV_DIST_L2, 5);

		cv::Mat sure_fg;
		cv::Point min_loc, max_loc;
		double min_val, max_val;

		cv::minMaxLoc(dist_transform, &min_val, &max_val, &min_loc, &max_loc);
		cv::threshold(dist_transform.clone(), sure_fg, THRESH_MAXVAL_EYE*max_val, 255, 0);

		dist_transform = dist_transform / max_val;

		cv::Mat s3 = sure_bg.clone();

		ofxCv::toOf(s3, outputOfEyeIMG.background);
		outputOfEyeIMG.background.update();

		cv::Mat unknown;
		cv::Mat sure_fg_uc1;
		sure_fg.convertTo(sure_fg_uc1, CV_8UC1);
		cv::subtract(sure_bg, sure_fg_uc1, unknown);

		cv::Mat s4 = unknown.clone();

		ofxCv::toOf(s4, outputOfEyeIMG.unknown);
		outputOfEyeIMG.unknown.update();

		int compCount = 0;

		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;
		sure_fg.convertTo(sure_fg, CV_32SC1, 1.0);
		cv::findContours(sure_fg, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);
		if (contours.empty()) return;

		cv::Mat markers = cv::Mat::zeros(sure_fg.rows, sure_fg.cols, CV_32SC1);

		int idx = 0;
		for (; idx >= 0; idx = hierarchy[idx][0], compCount++) {
			cv::drawContours(markers, contours, idx, cv::Scalar::all(compCount + 1), -1, 8, hierarchy, INT_MAX);
		}
		markers = markers + 1;


		for (int i = 0; i < markers.rows; i++) {
			for (int j = 0; j < markers.cols; j++) {
				unsigned char &v = unknown.at<unsigned char>(i, j);
				if (v == 255) {
					markers.at<int>(i, j) = 0;
				}
			}
		}

		cv::watershed(originalEyeMat.original.clone(), markers);
		viewEyeMat.watershedHighest = cv::Mat::zeros(originalEyeMat.original.size(), CV_8UC3);
		viewEyeMat.saliencyHighest = cv::Mat::zeros(originalEyeMat.original.size(), CV_8UC3);

		cv::Mat wshed(markers.size(), CV_8UC3);

		ofLogNotice() << "count: " << compCount;
		std::vector<int> saliencyPoint(compCount + 1, 0);

		for (int i = 0; i < compCount + 1; i++)
		{
			int b = cv::theRNG().uniform(0, 255);
			int g = cv::theRNG().uniform(0, 255);
			int r = cv::theRNG().uniform(0, 255);

			colorEyeTab.push_back(cv::Vec3b((uchar)b, (uchar)g, (uchar)r));
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
					wshed.at<cv::Vec3b>(i, j) = colorEyeTab[index - 1];
				}
				else {
					viewEyeMat.watershedHighest.at<cv::Vec3b>(i, j) = colorEyeTab[index - 1];
					if (saliencyPoint[index - 1] < (int)saliencyImg.at<uchar>(i, j)) {
						saliencyPoint[index - 1] = (int)saliencyImg.at<uchar>(i, j);
					}
					//saliencyPoint[index - 1] += (int)saliencyImg.at<uchar>(i, j);
				}
			}
		}

		for (int i = 0; i < saliencyPoint.size(); i++) {
			ofLogNotice() << "saliencyEyePoint[" << i << "]: " << saliencyPoint[i];
		}

		maxSaliencyEyePoint.iter = std::max_element(saliencyPoint.begin(), saliencyPoint.end());
		maxSaliencyEyePoint.maxIndex = std::distance(saliencyPoint.begin(), maxSaliencyEyePoint.iter);
		ofLogNotice() << "Index of max element: " << maxSaliencyEyePoint.maxIndex;

		for (int i = 0; i < markers.rows; i++) {
			for (int j = 0; j < markers.cols; j++)
			{
				int index = markers.at<int>(i, j);
				if (index == maxSaliencyEyePoint.maxIndex + 1) {
					viewEyeMat.saliencyHighest.at<cv::Vec3b>(i, j) = colorEyeTab[index - 1];
				}
				else {
					originalEyeMat.copy.at<cv::Vec3b>(i, j) = cv::Vec3b((uchar)0, (uchar)0, (uchar)0);
					//mat_copy_SECOND.at<cv::Vec3b>(i,j) = cv::Vec3b((uchar)255, (uchar)255, (uchar)255)
				}
			}
		}

		cvtColor(saliencyImg.clone(), eyeImgG, cv::COLOR_GRAY2BGR);


		//wshed = wshed*0.5 + picImgG*0.5;
		viewEyeMat.watershedHighest = viewEyeMat.watershedHighest*0.5 + eyeImgG*0.5;
		viewEyeMat.saliencyHighest = viewEyeMat.saliencyHighest*0.5 + eyeImgG*0.5;

		cv::Mat s5 = wshed.clone();

		cv::Mat s6 = viewEyeMat.watershedHighest.clone();

		cv::Mat s7 = viewEyeMat.saliencyHighest.clone();

		cvtColor(saliencyImg.clone(), eyeImgG, cv::COLOR_GRAY2BGR);

		ofxCv::toOf(s5, outputOfEyeIMG.watershed);
		outputOfEyeIMG.watershed.update();
		ofxCv::toOf(s6, outputOfEyeIMG.watershedAfter);
		outputOfEyeIMG.watershedAfter.update();
		outputOfEyeIMG.watershedAfter
			.save(prefixPath.eyeGaze + "/" + folderName + "/" + fileName + "/" + outputOfEyeFileName.watershed + ext.jpg);
		ofxCv::toOf(s7, outputOfEyeIMG.watershedHighest);
		outputOfEyeIMG.watershedHighest.update();
		viewEyeMat.matMix = originalEyeMat.original*0.2 + originalEyeMat.copy*0.8;

		cv::Mat s8 = viewEyeMat.matMix.clone();

		ofxCv::toOf(s8, outputOfEyeIMG.saliencyMapHighest);
		outputOfEyeIMG.saliencyMapHighest.update();
		outputOfEyeIMG.saliencyMapHighest
			.save(prefixPath.eyeGaze + "/" + folderName + "/" + fileName + "/" + outputOfEyeFileName.saliencyMapHighest + "_1" + ext.jpg);

		markersEyeSave = markers.clone();

		if (!saliencyPoint.empty()) {
			saliencyEyePoint.saved = saliencyPoint;
			saliencyEyePoint.backup = saliencyPoint;
			saliencyPoint.clear();
		}
		else {
			ofLogWarning() << "saliencyPoint is empty.";
		}
	}
}

//--------------------------------------------------------------
void ofApp::dumpOSC(ofxOscMessage m) {
	string msg_string;
	msg_string = m.getAddress();
	for (int i = 0; i < m.getNumArgs(); i++) {
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
void ofApp::loadEyeGaze(bool path) {
	if (path)
	{
		loadOfImage.update();

		cv::Mat loadMat, loadMat_gray, loadMatBGR, loadMat8UC1;
		loadMat = ofxCv::toCv(loadOfImage);

		cv::Mat s10 = loadMat.clone();
		ofxCv::toOf(s10, outputOfEyeIMG.eyeGazeResult);
		outputOfEyeIMG.eyeGazeResult.update();

		ofLogNotice() << "loadMat.type: " << loadMat.type();
		ofLogNotice() << "loadMat.depth: " << loadMat.depth();
		ofLogNotice() << "loadMat.channels: " << loadMat.channels();

		cv::cvtColor(loadMat.clone(), loadMatBGR, CV_BGRA2BGR);
		loadMatBGR.clone().convertTo(loadMat8UC1, CV_8UC1);
		
		originalEyeMat.original = loadMat8UC1.clone();
		originalEyeMat.copy = loadMat8UC1.clone();

		cv::Mat s12 = loadMat8UC1.clone();
		ofxCv::toOf(s12, outputOfEyeIMG.loadMat8UC1);
		outputOfEyeIMG.loadMat8UC1.update();

		cvtColor(loadMat8UC1.clone(), loadMat_gray, cv::COLOR_BGR2GRAY);

		cv::Point min_loc, max_loc;
		double min_val, max_val;

		cv::minMaxLoc(loadMat_gray.clone(), &min_val, &max_val, &min_loc, &max_loc);
		ofLogNotice() << "MaxValue: " << max_val;

		viewEyeMat.saliencyMap = loadMat_gray.clone();
		cv::Mat s13 = viewEyeMat.saliencyMap.clone();
		ofxCv::toOf(s13, outputOfEyeIMG.gray);
		outputOfEyeIMG.gray.update();

		ofLogNotice() << "viewEyeMat.saliencyMap.type: " << viewEyeMat.saliencyMap.type();
		ofLogNotice() << "viewEyeMat.saliencyMap.depth: " << viewEyeMat.saliencyMap.depth();
		ofLogNotice() << "viewEyeMat.saliencyMap.channels: " << viewEyeMat.saliencyMap.channels();

		createWatershed(viewEyeMat.saliencyMap.clone());

	}
	else {
		ofLogWarning() << "Can not load EyeGazeHeatMap file.";
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	ofLogNotice() << "keyPressed: " << key;

	switch (key) {
	case 13:
		// "Enter"‚ð‰Ÿ‚µ‚½Žž:
		switch (mode)
		{
		case ConstTools::RELEASE:
			saliencyPicPoint.saved[maxSaliencyPicPoint.maxIndex] = 0;
			enterPicCount++;
			enterState = ConstTools::SALIENCYMAP;
			break;
		case ConstTools::DEBUG:
			saliencyPicPoint.saved[maxSaliencyPicPoint.maxIndex] = 0;
			enterPicCount++;
			enterState = ConstTools::SALIENCYMAP;
			break;
		case ConstTools::RESULT:
			saliencyEyePoint.saved[maxSaliencyEyePoint.maxIndex] = 0;
			enterEyeCount++;
			enterState = ConstTools::EYEGAZE;
			break;
		default:
			break;
		}

		break;
	case 8:
		// "BackSpace"‚ð‰Ÿ‚µ‚½Žž:
		switch (mode)
		{
		case ConstTools::RELEASE:
			saliencyPicPoint.saved = saliencyPicPoint.backup;
			enterPicCount = 0;
			enterState = ConstTools::SALIENCYMAP;
			break;
		case ConstTools::DEBUG:
			saliencyPicPoint.saved = saliencyPicPoint.backup;
			enterPicCount = 0;
			enterState = ConstTools::SALIENCYMAP;
			break;
		case ConstTools::RESULT:
			saliencyEyePoint.saved = saliencyEyePoint.backup;
			enterEyeCount = 0;
			enterState = ConstTools::EYEGAZE;
			break;
		default:
			break;
		}

		break;
	case ' ':
		// "Space"‚ð‰Ÿ‚µ‚½Žž:
		if ((mode == ConstTools::EYETRACK) || (mode == ConstTools::EYETRACKHEATMAP) || (mode == ConstTools::IMAGEVIEW))
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
		if (eyeTrackState == ConstTools::STANDBY && 
			((mode == ConstTools::EYETRACK) || (mode == ConstTools::EYETRACKHEATMAP) || (mode == ConstTools::IMAGEVIEW))
			)
		{
			heatMap.gray
				.save(prefixPath.eyeGaze + "/" + folderName + "/" + fileName + "/" + outputOfEyeFileName.eyeGazeHeatMapGray + ext.png);
			heatMap.grays
				.save(prefixPath.eyeGaze + "/" + folderName + "/" + fileName + "/" + outputOfEyeFileName.eyeGazeHeatMapGraySoft + ext.png);
			heatMap.spectral
				.save(prefixPath.eyeGaze + "/" + folderName + "/" + fileName + "/" + outputOfEyeFileName.eyeGazeHeatMap + ext.png);
			heatMap.spectrals
				.save(prefixPath.eyeGaze + "/" + folderName + "/" + fileName + "/" + outputOfEyeFileName.eyeGazeHeatMapSoft + ext.png); 
			
			eyeTrackState = ConstTools::SAVE;
		}
		break;

	case 'r':
		heatMap.gray.clear();
		heatMap.grays.clear();
		heatMap.spectral.clear();
		heatMap.spectrals.clear();
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
		mode = ConstTools::RELEASE;
		loadState = ConstTools::PICLOAD;
		break;
	case 'x':
		// "X"‚ð‰Ÿ‚µ‚½Žž: debug
		mode = ConstTools::DEBUG;
		loadState = ConstTools::PICLOAD;
		break;
	case 'c':
		// "C"‚ð‰Ÿ‚µ‚½Žž: eyeTrack
		mode = ConstTools::EYETRACK;
		break;
	case 'v':
		// "V"‚ð‰Ÿ‚µ‚½Žž: eyeTrackHeatMap
		mode = ConstTools::EYETRACKHEATMAP;
		break;

	case 'b':
		// "B"‚ð‰Ÿ‚µ‚½Žž: imageView
		mode = ConstTools::IMAGEVIEW;

		break;
	case 'n':
		// "N"‚ð‰Ÿ‚µ‚½Žž: result
		mode = ConstTools::RESULT;
		loadState = ConstTools::EYELOAD;

		loadEyeGaze(loadOfImage.load(eyeGazePath));

		break;
	case 'm':
		mode = ConstTools::SLEEP;
		break;
	default:
		break;
	}

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
