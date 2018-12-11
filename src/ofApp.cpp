#include "ofApp.h"

#define PORT 8000
#define HOST "127.0.0.1"

#define THRESH_PIC 40
#define THRESH_EYE 0

#define THRESH_MAXVAL_PIC 0.3
#define THRESH_MAXVAL_EYE 0.0

#define ALPHANOTZERO 255/10


//--------------------------------------------------------------
void ofApp::setup() {

	receiver.setup(PORT);

	eyeTrackState = ConstTools::EyeTrackState::STANDBY;
	loadState = ConstTools::LoadState::PICLOAD;
	infomation = ConstTools::Infomation::VIEW;
	mode = ConstTools::Mode::SLEEP;
	rankingState = ConstTools::RankingState::NOT;
	
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
	picCntStr << "The " << enterPicCount + 1 << " most saliency place";
	eyeCntStr << "The " << enterEyeCount + 1 << " most saliency place";

}

//--------------------------------------------------------------
void ofApp::setupView(std::string path) {
	if (inputOfImg.load(path))
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

			if (eyeTrackState == ConstTools::EyeTrackState::TRACKING)
			{
				if ((0 > X) || (0 > Y)) { return; }
				if ((X <= WINWIDTH) && (Y <= WINHEIGHT))
				{
					/*ofLogNotice() << "X(after): " << X;
					ofLogNotice() << "Y(after): " << Y;*/
					if ((int)viewEyeMat.eyeGazeMat.at<uchar>(Y, X) >= 255) { return; }
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

	if (eyeTrackState == ConstTools::EyeTrackState::TRACKING)
	{
		heatMap.gray.update(OFX_HEATMAP_CS_GRAYS);
		heatMap.grays.update(OFX_HEATMAP_CS_GRAYS_SOFT);
		heatMap.spectral.update(OFX_HEATMAP_CS_SPECTRAL);
		heatMap.spectrals.update(OFX_HEATMAP_CS_SPECTRAL_SOFT);
	}



	if (enterState == ConstTools::EnterState::SALIENCYMAP) {
		picCntStr.str("");
		picCntStr.clear(stringstream::goodbit);

		int maxValue = *std::max_element(saliencyPicPoint.saved.begin(), saliencyPicPoint.saved.end());

		if (maxValue != 0) {

			maxSaliencyPicPoint.maxIndex = -1;
			int maxValue = 0;
			int maxTotalValue = 0;
			int maxValueCount = 0;
			for (size_t i = 0; i < saliencyPicPoint.saved.size(); i++)
			{
				ofLogNotice() << "saliencyPoint[" << i << "]: " << saliencyPicPoint.saved[i];
				ofLogNotice() << "maxValue: " << maxValue;

				if (maxValue < saliencyPicPoint.saved[i])
				{
					maxSaliencyPicPoint.maxIndex = i;
					maxValue = saliencyPicPoint.saved[i];
					maxTotalValue = saliencyPicTotalPoint.saved[i];
					maxValueCount = maxValuePicCount[i];
				}
				else if (maxValue == saliencyPicPoint.saved[i]) {
					ofLogNotice() << "maxValuePicCount[" << i << "]: " << maxValuePicCount[i];
					ofLogNotice() << "maxValueCount: " << maxValueCount;
					if (maxValueCount < maxValuePicCount[i])
					{
						maxSaliencyPicPoint.maxIndex = i;
						maxValue = saliencyPicPoint.saved[i];
						maxTotalValue = saliencyPicTotalPoint.saved[i];
						maxValueCount = maxValuePicCount[i];
					}
					else if (maxValueCount == maxValuePicCount[i])
					{
						ofLogNotice() << "saliencyTotalPoint[" << i << "]: " << saliencyPicTotalPoint.saved[i];
						ofLogNotice() << "maxTotalValue: " << maxTotalValue;
						if (maxTotalValue < saliencyPicTotalPoint.saved[i])
						{
							maxSaliencyPicPoint.maxIndex = i;
							maxValue = saliencyPicPoint.saved[i];
							maxTotalValue = saliencyPicTotalPoint.saved[i];
							maxValueCount = maxValuePicCount[i];
						}
					}
				}
			}


			/*maxSaliencyPicPoint.iter = std::max_element(saliencyPicPoint.saved.begin(), saliencyPicPoint.saved.end());
			maxSaliencyPicPoint.maxIndex = std::distance(saliencyPicPoint.saved.begin(), maxSaliencyPicPoint.iter);*/
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
					}
				}
			}

			viewPicMat.saliencyHighest = viewPicMat.saliencyHighest*0.5 + picImgG*0.5;

			cv::Mat s7 = viewPicMat.saliencyHighest.clone();
			ofxCv::toOf(s7, outputOfPicIMG.watershedHighest);
			outputOfPicIMG.watershedHighest.update();

			viewPicMat.matMix = originalPicMat.original*0.1 + originalPicMat.copy*0.9;

			cv::Mat s8 = viewPicMat.matMix.clone();
			ofxCv::toOf(s8, outputOfPicIMG.saliencyMapHighest);
			outputOfPicIMG.saliencyMapHighest.update();
			//outputOfPicIMG.saliencyMapHighest
				//.save(prefixPath.picture + "/" + folderName + "/" + fileName + "/" + outputOfPicFileName.saliencyMapHighest + "_" + number.str() + ext.png);

			picCntStr << "The " << enterPicCount + 1 << " most saliency place";

		}
		else {
			picCntStr << "Finish";
		};

		enterState = ConstTools::EnterState::NONE;
	}
	else if (enterState == ConstTools::EnterState::EYEGAZE) {
		eyeCntStr.str("");
		eyeCntStr.clear(stringstream::goodbit);

		int maxValue = *std::max_element(saliencyEyePoint.saved.begin(), saliencyEyePoint.saved.end());

		if (maxValue != 0) {

			maxSaliencyEyePoint.maxIndex = -1;
			int maxValue = 0;
			int maxTotalValue = 0;
			int maxValueCount = 0;
			for (size_t i = 0; i < saliencyEyePoint.saved.size(); i++)
			{

				ofLogNotice() << "saliencyEyePoint[" << i << "]: " << saliencyEyePoint.saved[i];
				ofLogNotice() << "maxValue: " << maxValue;

				if (maxValue < saliencyEyePoint.saved[i])
				{
					maxSaliencyEyePoint.maxIndex = i;
					maxValue = saliencyEyePoint.saved[i];
					maxTotalValue = saliencyEyeTotalPoint.saved[i];
					maxValueCount = maxValueEyeCount[i];
				}
				else if (maxValue == saliencyEyePoint.saved[i]) {
					ofLogNotice() << "maxValueEyeCount[" << i << "]: " << maxValueEyeCount[i];
					ofLogNotice() << "maxValueCount: " << maxValueCount;
					if (maxValueCount < maxValueEyeCount[i])
					{
						maxSaliencyEyePoint.maxIndex = i;
						maxValue = saliencyEyePoint.saved[i];
						maxTotalValue = saliencyEyeTotalPoint.saved[i];
						maxValueCount = maxValueEyeCount[i];
					}
					else if (maxValueCount == maxValueEyeCount[i])
					{
						ofLogNotice() << "saliencyEyeTotalPoint[" << i << "]: " << saliencyEyeTotalPoint.saved[i];
						ofLogNotice() << "maxTotalValue: " << maxTotalValue;
						if (maxTotalValue < saliencyEyeTotalPoint.saved[i])
						{
							maxSaliencyEyePoint.maxIndex = i;
							maxValue = saliencyEyePoint.saved[i];
							maxTotalValue = saliencyEyeTotalPoint.saved[i];
							maxValueCount = maxValueEyeCount[i];
						}
					}
				}
			}

			/*maxSaliencyEyePoint.iter = 
				std::max_element(saliencyEyePoint.saved.begin(), saliencyEyePoint.saved.end());
			maxSaliencyEyePoint.maxIndex = 
				std::distance(saliencyEyePoint.saved.begin(), maxSaliencyEyePoint.iter);*/
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
					}
				}
			}

			viewEyeMat.saliencyHighest = viewEyeMat.saliencyHighest*0.5 + eyeImgG*0.5;

			cv::Mat s7 = viewEyeMat.saliencyHighest.clone();
			ofxCv::toOf(s7, outputOfEyeIMG.watershedHighest);
			outputOfEyeIMG.watershedHighest.update();

			viewEyeMat.matMix = originalEyeMat.original*0 + originalEyeMat.copy*1;

			cv::Mat s8 = viewEyeMat.matMix.clone();
			ofxCv::toOf(s8, outputOfEyeIMG.saliencyMapHighest);
			outputOfEyeIMG.saliencyMapHighest.update();
			//outputOfEyeIMG.saliencyMapHighest
				//.save(prefixPath.eyeGaze + "/" + folderName + "/" + fileName + "/" + outputOfEyeFileName.saliencyMapHighest + "_" + number.str() + ext.png);

			eyeCntStr << "The " << enterEyeCount + 1 << " most saliency place";
		}
		else {
			eyeCntStr << "Finish";
		};

		enterState = ConstTools::EnterState::NONE;
	}
}

//--------------------------------------------------------------
void ofApp::draw() {

	switch (mode) {
	case ConstTools::Mode::RELEASE:
		inputOfImg.draw(0, 0, ofGetWidth() / 2, ofGetHeight() / 2);
		outputOfPicIMG.saliencyMapHighest.draw(0, ofGetHeight() / 2, ofGetWidth() / 2, ofGetHeight() / 2);

		// Label
		ofDrawBitmapStringHighlight("original", ofGetWidth() / 2 + 20, 20);
		ofDrawBitmapStringHighlight("saliencyMap-watershed", ofGetWidth() / 2 + 20, ofGetHeight() / 2 + 20);

		// Label
		ofDrawBitmapStringHighlight("Enter: Next HighSaliency Place\nDelete: Reset",
			ofGetWidth() - ofGetWidth() / 3 - 20, 20);

		ofDrawBitmapStringHighlight(picCntStr.str(), ofGetWidth() / 2 + 20, ofGetHeight() / 2 + 50);

		ofSetWindowTitle("RELEASE");
		break;

	case ConstTools::Mode::DEBUG:
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

	case ConstTools::Mode::EYETRACK:
		outputOfEyeIMG.eyeGaze.draw(0, 0, WINWIDTH, WINHEIGHT);
		//saliencyMap.draw(ofGetWidth() / 2, 0, ofGetWidth() / 2, ofGetHeight() / 2);

		ofSetWindowTitle("EYETRACK");

		switch (eyeTrackState)
		{
		case ConstTools::EyeTrackState::STANDBY:
			ofDrawBitmapStringHighlight("STANDBY(Pixels): Please Space Key", 20, 20);
			break;
		case ConstTools::EyeTrackState::TRACKING:
			ofDrawBitmapStringHighlight("TRACKING", 20, 20);
			break;
		}

		break;

	case ConstTools::Mode::EYETRACKHEATMAP:
		ofBackground(0, 0, 0);
		ofSetColor(255, 255);
		//heatMap.gray.draw(0, 0);
		heatMap.grays.draw(0, 0);
		//heatMap.spectral.draw(0, 0);
		//heatMap.spectrals.draw(0, 0);

		ofSetWindowTitle("EYETRACKHEATMAP");

		switch (eyeTrackState)
		{
		case ConstTools::EyeTrackState::STANDBY:
			ofDrawBitmapStringHighlight("STANDBY(HeatMap): Please Space Key", 20, 20);
			break;
		case ConstTools::EyeTrackState::TRACKING:
			ofDrawBitmapStringHighlight("TRACKING: Save S Key", 20, 20);
			break;
		case ConstTools::EyeTrackState::SAVE:
			ofDrawBitmapStringHighlight("Saved EyeGazeHeatMap", 20, 20);
			break;
		}

		break;

	case ConstTools::Mode::IMAGEVIEW:
		inputOfImg.draw(0, 0, ofGetWidth(), ofGetHeight());
		ofSetWindowTitle("IMAGEVIEW");
		switch (eyeTrackState)
		{
			case ConstTools::EyeTrackState::SAVE:
			ofDrawBitmapStringHighlight("Saved EyeGazeHeatMap", 20, 20);
			break;
		}

		break;

	case ConstTools::Mode::RESULT:

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
		//ofDrawBitmapStringHighlight(eyeCntStr.str(), ofGetWidth() / 2 + 20, ofGetHeight() / 2 + 50);
		ofSetWindowTitle("RESULT");
		break;

	case ConstTools::Mode::SLEEP:
		ofBackground(0, 0, 0);
		break;

	}

	switch (infomation)
	{
	case ConstTools::Infomation::VIEW:
		ofDrawBitmapStringHighlight(
			"SELECT KEY PRESSED\r\n"
			"  *Z: RELEASE\n"
			"  *X: DEBUG\n"
			"  *C: EYETRACK\n"
			"  *V: EYETRACKHEATMAP\n"
			"  *B: IMAGEVIEW\n"
			"  *N: RESULT\n"
			"  *M: SLEEP\n"
			"  *F: INFORMATION\n\n"
			"  if picture 'Z'or'X' key pressed\n"
			"    *Enter: Next HighSaliency Place\n"
			"    *Delete: Reset\n"
			"  if picture 'C'or'V'or'B' pressed\n"
			"    *SPACE: START・STOP\n"
			"    *S: SAVE\n"
			"    *D: DELETE\n"
			"  if picture 'Z'or'X' pressed\n"
			"  if eyeGaze 'N' key pressed\n"
			"    *A: RANKING\n",
			ofGetWidth() - ofGetWidth() / 5, 20
		);
		break;
	case ConstTools::Infomation::HIDE:
		break;
	}

	switch (rankingState)
	{
	case ConstTools::DONE:
		ofDrawBitmapStringHighlight(
			"Saved RankingMap",
			20, ofGetHeight() - 20
		);
		break;
	case ConstTools::NOT:
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
	if (loadState == ConstTools::LoadState::PICLOAD)
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

		unsigned int compCount = 0;

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
		std::vector<int> saliencyTotalPoint(compCount + 1, 0);
		std::vector<int> maxValueCount(compCount + 1, 0);

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
					if (saliencyPoint[index - 1] <= (int)saliencyImg.at<uchar>(i, j)) {
						saliencyPoint[index - 1] = (int)saliencyImg.at<uchar>(i, j);
						if ((int)saliencyImg.at<uchar>(i, j) == 255)
						{
							maxValueCount[index -1]++;
						}
					}
					saliencyTotalPoint[index - 1] += (int)saliencyImg.at<uchar>(i, j);
				}
			}
		}

		maxSaliencyPicPoint.maxIndex = -1;
		int maxValue = 0;
		int maxTotalValue = 0;
		int count = 0;
		for (size_t i = 0; i < saliencyPoint.size(); i++)
		{

			ofLogNotice() << "saliencyPoint[" << i << "]: " << saliencyPoint[i];
			if (maxValue < saliencyPoint[i])
			{
				maxSaliencyPicPoint.maxIndex = i;
				maxValue = saliencyPoint[i];
				maxTotalValue = saliencyTotalPoint[i];
				count = maxValueCount[i];
			}
			else if (maxValue == saliencyPoint[i]) {
				if (count < maxValueCount[i])
				{
					maxSaliencyPicPoint.maxIndex = i;
					maxValue = saliencyPoint[i];
					maxTotalValue = saliencyTotalPoint[i];
					count = maxValueCount[i];
				}
				else if (count == maxValueCount[i])
				{
					if (maxTotalValue < saliencyTotalPoint[i])
					{
						maxSaliencyPicPoint.maxIndex = i;
						maxValue = saliencyPoint[i];
						maxTotalValue = saliencyTotalPoint[i];
						count = maxValueCount[i];
					}
				}
			}
		}


		/*maxSaliencyPicPoint.iter = std::max_element(saliencyPoint.begin(), saliencyPoint.end());
		maxSaliencyPicPoint.maxIndex = std::distance(saliencyPoint.begin(), maxSaliencyPicPoint.iter);*/
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
		//outputOfPicIMG.watershedAfter
			//.save(prefixPath.picture + "/" + folderName + "/" + fileName + "/" + outputOfPicFileName.watershed + ext.jpg);

		ofxCv::toOf(s7, outputOfPicIMG.watershedHighest);
		outputOfPicIMG.watershedHighest.update();

		viewPicMat.matMix = originalPicMat.original*0.1 + originalPicMat.copy*0.9;

		cv::Mat s8 = viewPicMat.matMix.clone();

		ofxCv::toOf(s8, outputOfPicIMG.saliencyMapHighest);
		outputOfPicIMG.saliencyMapHighest.update();
		//outputOfPicIMG.saliencyMapHighest
			//.save(prefixPath.picture + "/" + folderName + "/" + fileName + "/" + outputOfPicFileName.saliencyMapHighest + "_1" + ext.jpg);

		markersPicSave = markers.clone();

		if (!saliencyPoint.empty()) {
			saliencyPicPoint.saved = saliencyPoint;
			saliencyPicPoint.backup = saliencyPoint;
			saliencyPoint.clear();
		}


		if (!saliencyTotalPoint.empty()) {
			saliencyPicTotalPoint.saved = saliencyTotalPoint;
			saliencyPicTotalPoint.backup = saliencyTotalPoint;
			saliencyTotalPoint.clear();
		}

		if (!maxValueCount.empty()) {
			maxValuePicCount = maxValueCount;
			maxValueCount.clear();
		}
	}
	else if (loadState == ConstTools::LoadState::EYELOAD)
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

		unsigned int compCount = 0;

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
		std::vector<int> saliencyTotalPoint(compCount + 1, 0);
		std::vector<int> maxValueCount(compCount + 1, 0);

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
					if (saliencyPoint[index - 1] <= (int)saliencyImg.at<uchar>(i, j)) {
						saliencyPoint[index - 1] = (int)saliencyImg.at<uchar>(i, j);
						if ((int)saliencyImg.at<uchar>(i, j) == 255)
						{
							maxValueCount[index - 1]++;
						}
					}
					saliencyTotalPoint[index - 1] += (int)saliencyImg.at<uchar>(i, j);
				}
			}
		}

		maxSaliencyEyePoint.maxIndex = -1;
		int maxValue = 0;
		int maxTotalValue = 0;
		int count = 0;
		for (size_t i = 0; i < saliencyPoint.size(); i++)
		{
			ofLogNotice() << "saliencyEyePoint[" << i << "]: " << saliencyPoint[i];
			if (maxValue < saliencyPoint[i])
			{
				maxSaliencyEyePoint.maxIndex = i;
				maxValue = saliencyPoint[i];
				maxTotalValue = saliencyTotalPoint[i];
				count = maxValueCount[i];
			}
			else if (maxValue == saliencyPoint[i]) {
				if (count < maxValueCount[i])
				{
					if (maxTotalValue < saliencyTotalPoint[i])
					{
						maxSaliencyEyePoint.maxIndex = i;
						maxValue = saliencyPoint[i];
						maxTotalValue = saliencyTotalPoint[i];
						count = maxValueCount[i];
					}
				}
				else if (count == maxValueCount[i])
				{
					if (maxTotalValue < saliencyTotalPoint[i])
					{
						maxSaliencyEyePoint.maxIndex = i;
						maxValue = saliencyPoint[i];
						maxTotalValue = saliencyTotalPoint[i];
						count = maxValueCount[i];
					}
				}
			}
		}

		/*maxSaliencyEyePoint.iter = std::max_element(saliencyPoint.begin(), saliencyPoint.end());
		maxSaliencyEyePoint.maxIndex = std::distance(saliencyPoint.begin(), maxSaliencyEyePoint.iter);*/
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
		viewEyeMat.matMix = originalEyeMat.original*0 + originalEyeMat.copy*1;

		cv::Mat s8 = viewEyeMat.matMix.clone();

		ofxCv::toOf(s8, outputOfEyeIMG.saliencyMapHighest);
		outputOfEyeIMG.saliencyMapHighest.update();
		//outputOfEyeIMG.saliencyMapHighest
			//.save(prefixPath.eyeGaze + "/" + folderName + "/" + fileName + "/" + outputOfEyeFileName.saliencyMapHighest + "_1" + ext.jpg);

		markersEyeSave = markers.clone();

		if (!saliencyPoint.empty()) {
			saliencyEyePoint.saved = saliencyPoint;
			saliencyEyePoint.backup = saliencyPoint;
			saliencyPoint.clear();
		}
		
		if (!saliencyTotalPoint.empty()) {
			saliencyEyeTotalPoint.saved = saliencyTotalPoint;
			saliencyEyeTotalPoint.backup = saliencyTotalPoint;
			saliencyTotalPoint.clear();
		}

		if (!maxValueCount.empty()) {
			maxValueEyeCount = maxValueCount;
			maxValueCount.clear();
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

		for (int x = 0; x < loadMat.rows; ++x) {
			for (int y = 0; y < loadMat.cols; ++y) {
				//ofLogNotice() << "loadMat.at<cv::Vec4b>(" << x << "," << y << ") : " << (float)loadMat.at<cv::Vec4b>(x, y)[3];
				if ((float)loadMat.at<cv::Vec4b>(x, y)[3] > 0.0 )
				{
					if (loadMat.at<cv::Vec4b>(x, y)[0] <= ALPHANOTZERO)
					{
						loadMat.at<cv::Vec4b>(x, y)[0] += ALPHANOTZERO;
					}
		
					if (loadMat.at<cv::Vec4b>(x, y)[1] <= ALPHANOTZERO)
					{
						loadMat.at<cv::Vec4b>(x, y)[1] += ALPHANOTZERO;
					}

					if (loadMat.at<cv::Vec4b>(x, y)[2] <= ALPHANOTZERO)
					{
						loadMat.at<cv::Vec4b>(x, y)[2] += ALPHANOTZERO;
					}
				}
			}
		}

		cv::Mat s10 = loadMat.clone();
		ofxCv::toOf(s10, outputOfEyeIMG.eyeGazeResult);
		outputOfEyeIMG.eyeGazeResult.update();

		cv::cvtColor(loadMat.clone(), loadMatBGR, CV_BGRA2BGR);
		loadMatBGR.clone().convertTo(loadMat8UC1, CV_8UC1);
		
		originalEyeMat.original = loadMat8UC1.clone();
		originalEyeMat.copy = loadMat8UC1.clone();

		cv::Mat s12 = loadMat8UC1.clone();
		ofxCv::toOf(s12, outputOfEyeIMG.loadMat8UC1);
		outputOfEyeIMG.loadMat8UC1.update();

		viewEyeMat.saliencyMap = loadMat_gray.clone();
		cvtColor(loadMat8UC1.clone(), loadMat_gray, cv::COLOR_BGR2GRAY);

		viewEyeMat.saliencyMap = loadMat_gray.clone();
		cv::Mat s13 = viewEyeMat.saliencyMap.clone();
		ofxCv::toOf(s13, outputOfEyeIMG.gray);
		outputOfEyeIMG.gray.update();

		createWatershed(viewEyeMat.saliencyMap.clone());

	}
	else {
		ofLogWarning() << "Can not load EyeGazeHeatMap file.";
	}
}

//--------------------------------------------------------------
void ofApp::ranking(ConstTools::EnterState enterState) {
	std::vector<int> points, totalPoints;
	ofApp::MaxSaliencyPoint maxSaliencyPoint;
	std::ostringstream number;
	int maxValueCheck;
	cv::Mat s8;

	switch (enterState)
	{
	case ConstTools::EnterState::SALIENCYMAP:
		points = saliencyPicPoint.backup;
		totalPoints = saliencyPicTotalPoint.backup;
		for (size_t i = 0; i < points.size(); i++)
		{
			if (i != 0){
				points[maxSaliencyPoint.maxIndex] = 0;
				totalPoints[maxSaliencyPoint.maxIndex] = 0;
			}

			maxValueCheck = *std::max_element(points.begin(), points.end());
			if (maxValueCheck == 0) { break; }

			/*maxSaliencyPoint.iter = std::max_element(points.begin(), points.end());
			maxSaliencyPoint.maxIndex = std::distance(points.begin(), maxSaliencyPoint.iter);*/

			maxSaliencyPoint.maxIndex = -1;
			int maxValue = 0;
			int maxTotalValue = 0;
			int maxValueCount = 0;
			for (size_t i = 0; i < points.size(); i++)
			{
				ofLogNotice() << "saliencyPoint[" << i << "]: " << points[i];
				if (maxValue < points[i])
				{
					maxSaliencyPoint.maxIndex = i;
					maxValue = points[i];
					maxTotalValue = totalPoints[i];
					maxValueCount = maxValuePicCount[i];
				}
				else if (maxValue == points[i]) 
				{
					if (maxValueCount < maxValuePicCount[i])
					{
						maxSaliencyPoint.maxIndex = i;
						maxValue = points[i];
						maxTotalValue = totalPoints[i];
						maxValueCount = maxValuePicCount[i];
					}
					else if (maxValueCount == maxValuePicCount[i])
					{
						if (maxTotalValue < totalPoints[i])
						{
							maxSaliencyPoint.maxIndex = i;
							maxValue = points[i];
							maxTotalValue = totalPoints[i];
						}
					}

				}
			}

			originalPicMat.copy = originalPicMat.original.clone();

			for (int i = 0; i < markersPicSave.rows; i++) {
				for (int j = 0; j < markersPicSave.cols; j++)
				{
					if (markersPicSave.at<int>(i, j) != maxSaliencyPoint.maxIndex + 1) {
						originalPicMat.copy.at<cv::Vec3b>(i, j) = cv::Vec3b((uchar)0, (uchar)0, (uchar)0);
					}
				}
			}

			viewPicMat.matMix = originalPicMat.original*0.1 + originalPicMat.copy*0.9;

			number.str("");
			number.clear(stringstream::goodbit);
			number << i + 1;

			s8 = viewPicMat.matMix.clone();
			ofxCv::toOf(s8, outputOfPicIMG.saliencyMapHighest);
			outputOfPicIMG.saliencyMapHighest.update();
			outputOfPicIMG.saliencyMapHighest
				.save(prefixPath.picture + "/" + folderName + "/" + fileName + "/" + outputOfPicFileName.saliencyMapHighest + "_" + number.str() + ext.png);
		}
		break;

	case ConstTools::EnterState::EYEGAZE:
		points = saliencyEyePoint.backup;
		totalPoints = saliencyEyeTotalPoint.backup;

		ofImage src;
		auto srcPath = ofToDataPath(prefixPath.eyeGaze + "/" + folderName + "/" + fileName + "/" + outputOfEyeFileName.eyeGazeHeatMap + ext.png);
		src.load(srcPath);
		cv::Mat originalMat = ofxCv::toCv(src); // カラー画像

		for (size_t i = 0; i < points.size(); i++)
		{
			if (i != 0){
				points[maxSaliencyPoint.maxIndex] = 0;
				totalPoints[maxSaliencyPoint.maxIndex] = 0;
			}
			
			maxValueCheck = *std::max_element(points.begin(), points.end());
			if (maxValueCheck == 0) { break; }

			/*maxSaliencyPoint.iter =
				std::max_element(points.begin(), points.end());
			maxSaliencyPoint.maxIndex =
				std::distance(points.begin(), maxSaliencyPoint.iter);*/

			maxSaliencyPoint.maxIndex = -1;
			int maxValue = 0;
			int maxTotalValue = 0;
			int maxValueCount = 0;

			ofLogNotice() << "maxValueEyeCount.size(): " << maxValueEyeCount.size();

			for (size_t i = 0; i < points.size(); i++)
			{
				/*ofLogNotice() << "saliencyPoint[" << i << "]: " << points[i];
				ofLogNotice() << "maxValue: " << maxValue;*/
				if (maxValue < points[i])
				{
					maxSaliencyPoint.maxIndex = i;
					maxValue = points[i];
					maxTotalValue = totalPoints[i];
					maxValueCount = maxValueEyeCount[i];
				}
				else if (maxValue == points[i]) 
				{
					/*ofLogNotice() << "maxValueEyeCount[" << i << "]: " << maxValueEyeCount[i];
					ofLogNotice() << "maxValueCount: " << maxValueCount;*/
					if (maxValueCount < maxValueEyeCount[i])
					{
						maxSaliencyPoint.maxIndex = i;
						maxValue = points[i];
						maxTotalValue = totalPoints[i];
						maxValueCount = maxValueEyeCount[i];
					}
					else if (maxValueCount == maxValueEyeCount[i])
					{
						/*ofLogNotice() << "saliencyEyeTotalPoint[" << i << "]: " << saliencyEyeTotalPoint.saved[i];
						ofLogNotice() << "maxTotalValue: " << maxTotalValue;*/
						if (maxTotalValue < totalPoints[i])
						{
							maxSaliencyPoint.maxIndex = i;
							maxValue = points[i];
							maxTotalValue = totalPoints[i];
							maxValueCount = maxValueEyeCount[i];
						}
					}
				}
			}


			originalEyeMat.copy = originalEyeMat.original.clone();

			for (int i = 0; i < markersEyeSave.rows; i++) {
				for (int j = 0; j < markersEyeSave.cols; j++)
				{
					if (markersEyeSave.at<int>(i, j) != maxSaliencyPoint.maxIndex + 1) {
						originalEyeMat.copy.at<cv::Vec3b>(i, j) = cv::Vec3b((uchar)0, (uchar)0, (uchar)0);
					}
				}
			}

			viewEyeMat.matMix = originalEyeMat.original*0 + originalEyeMat.copy*1;
			

			
			cv::Mat targetMat = originalEyeMat.copy.clone(); // 白黒のループしてくるやつ

			cv::threshold(targetMat, targetMat, 50, 255, CV_THRESH_BINARY_INV); // TODO: 閾値を変える

			std::vector<cv::Vec4i> hierarchy;
			std::vector < vector<cv::Point>> contours;

			cv::Mat targetMat8UC1, targetMat8UC3;
			cv::cvtColor(targetMat.clone(), targetMat8UC1, CV_BGR2GRAY);
			//targetMat.convertTo(targetMat8UC1, CV_8UC1);
			
			cv::findContours(targetMat8UC1, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

			cv::cvtColor(originalMat, originalMat, CV_BGRA2BGR);

			if (contours.size() >= 2  && i < 10) {
				auto textPoint = cv::Point(contours[1][0].x, contours[1][0].y);
				cv::putText(originalMat, std::to_string(i + 1), textPoint, 1, 5, (0, 0, 255), 5);
			}



			number.str("");
			number.clear(stringstream::goodbit);
			number << i + 1;

			s8 = viewEyeMat.matMix.clone();
			ofxCv::toOf(s8, outputOfEyeIMG.saliencyMapHighest);
			outputOfEyeIMG.saliencyMapHighest.update();
			outputOfEyeIMG.saliencyMapHighest
				.save(prefixPath.eyeGaze + "/" + folderName + "/" + fileName + "/" + "rank/"+ outputOfEyeFileName.saliencyMapHighest + "_" + number.str() + ext.png);

		}
		//cv::cvtColor(targetMat8UC1.clone(), targetMat8UC3, CV_GRAY2BGR);
		ofImage saveImage;
		cv::Mat s = originalMat.clone();
		ofxCv::toOf(s, saveImage);
		saveImage.update();
		saveImage.save(prefixPath.eyeGaze + "/" + folderName + "/" + fileName + "/" + outputOfEyeFileName.rank + ext.png);

		break;
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	ofLogNotice() << "keyPressed: " << key;

	eyeTrackState = ConstTools::EyeTrackState::STANDBY;
	ConstTools::PictureState picState = ConstTools::PictureState::NONPIC;
	bool picSet = false;

	rankingState = ConstTools::RankingState::NOT;

	switch (key) {
	case 13:
		// "Enter"を押した時:
		switch (mode)
		{
		case ConstTools::Mode::RELEASE:
		case ConstTools::Mode::DEBUG:
			saliencyPicPoint.saved[maxSaliencyPicPoint.maxIndex] = 0;
			saliencyPicTotalPoint.saved[maxSaliencyPicPoint.maxIndex] = 0;
			enterPicCount++;
			enterState = ConstTools::SALIENCYMAP;
			break;
		case ConstTools::Mode::RESULT:
			saliencyEyePoint.saved[maxSaliencyEyePoint.maxIndex] = 0;
			saliencyEyeTotalPoint.saved[maxSaliencyEyePoint.maxIndex] = 0;
			enterEyeCount++;
			enterState = ConstTools::EYEGAZE;
			break;
		default:
			break;
		}

		break;
	case 8:
		// "BackSpace"を押した時:
		switch (mode)
		{
		case ConstTools::Mode::RELEASE:
		case ConstTools::Mode::DEBUG:
			saliencyPicPoint.saved = saliencyPicPoint.backup;
			saliencyPicTotalPoint.saved = saliencyPicTotalPoint.backup;
			enterPicCount = 0;
			enterState = ConstTools::EnterState::SALIENCYMAP;
			break;
		case ConstTools::Mode::RESULT:
			saliencyEyePoint.saved = saliencyEyePoint.backup;
			saliencyEyeTotalPoint.saved = saliencyEyeTotalPoint.backup;
			enterEyeCount = 0;
			enterState = ConstTools::EnterState::EYEGAZE;
			break;
		default:
			break;
		}

		break;
	case ' ':
		// "Space"を押した時:
 		if ((mode == ConstTools::Mode::EYETRACK) || (mode == ConstTools::Mode::EYETRACKHEATMAP) || (mode == ConstTools::Mode::IMAGEVIEW))
		{
			if (eyeTrackState == ConstTools::EyeTrackState::STANDBY) {
				eyeTrackState = ConstTools::EyeTrackState::TRACKING;
			}
			else {
				eyeTrackState = ConstTools::EyeTrackState::STANDBY;
			}
		}
		break;

	case 's':
		if (eyeTrackState == ConstTools::EyeTrackState::STANDBY &&
			((mode == ConstTools::Mode::EYETRACK) || (mode == ConstTools::Mode::EYETRACKHEATMAP) || (mode == ConstTools::Mode::IMAGEVIEW))
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
			
			eyeTrackState = ConstTools::EyeTrackState::SAVE;
		}
		break;

	case 'a':
		switch (mode)
		{
		case ConstTools::Mode::RELEASE:
		case ConstTools::Mode::DEBUG:
			enterState = ConstTools::EnterState::SALIENCYMAP;
			ranking(enterState);
			rankingState = ConstTools::RankingState::DONE;
			break;
		case ConstTools::Mode::RESULT:
			enterState = ConstTools::EnterState::EYEGAZE;
			ranking(enterState);
			rankingState = ConstTools::RankingState::DONE;
			break;
		default:
			break;
		}

		enterState = ConstTools::EnterState::NONE;

		break;

	case 'd':
		heatMap.gray.clear();
		heatMap.grays.clear();
		heatMap.spectral.clear();
		heatMap.spectrals.clear();

		heatMap.gray.setup(WINWIDTH, WINHEIGHT, 32);
		heatMap.grays.setup(WINWIDTH, WINHEIGHT, 32);
		heatMap.spectral.setup(WINWIDTH, WINHEIGHT, 32);
		heatMap.spectrals.setup(WINWIDTH, WINHEIGHT, 32);

		break;
	case 'f':
		switch (infomation)
		{
		case ConstTools::Infomation::VIEW:
			infomation = ConstTools::Infomation::HIDE;
			break;
		case ConstTools::Infomation::HIDE:
			infomation = ConstTools::Infomation::VIEW;
			break;
		}
		break;

		//-------------   環境   ------------------
	case 'z':
		// "Z"を押した時: release
		mode = ConstTools::Mode::RELEASE;
		loadState = ConstTools::LoadState::PICLOAD;
		break;
	case 'x':
		// "X"を押した時: debug
		mode = ConstTools::Mode::DEBUG;
		loadState = ConstTools::LoadState::PICLOAD;
		break;
	case 'c':
		// "C"を押した時: eyeTrack
		mode = ConstTools::Mode::EYETRACK;
		break;
	case 'v':
		// "V"を押した時: eyeTrackHeatMap
		mode = ConstTools::Mode::EYETRACKHEATMAP;
		break;

	case 'b':
		// "B"を押した時: imageView
		mode = ConstTools::Mode::IMAGEVIEW;

		break;
	case 'n':
		// "N"を押した時: result
		mode = ConstTools::Mode::RESULT;
		loadState = ConstTools::LoadState::EYELOAD;

		loadEyeGaze(loadOfImage.load(eyeGazePath));

		break;
	case 'm':
		mode = ConstTools::Mode::SLEEP;
		break;


		//-------------   画像   ------------------
	case '1':
		fileName = inputFileName.non.field;
		picState = ConstTools::PictureState::IMAGE;
		picSet = true;
		break;

	case '2':
		fileName = inputFileName.non.brick;
		picState = ConstTools::PictureState::IMAGE;
		picSet = true;
		break;

	case '3':
		fileName = inputFileName.obj.furniture;
		picState = ConstTools::PictureState::IMAGE;
		picSet = true;
		break;

	case '4':
		fileName = inputFileName.obj.football;
		picState = ConstTools::PictureState::IMAGE;
		picSet = true;
		break;

	case '5':
		fileName = inputFileName.obj.sunset;
		picState = ConstTools::PictureState::IMAGE;
		picSet = true;
		break;

	case '6':
		fileName = inputFileName.obj.waterfall;
		picState = ConstTools::PictureState::IMAGE;
		picSet = true;
		break;

	case '7':
		fileName = inputFileName.objs.balloon;
		picState = ConstTools::PictureState::IMAGE;
		picSet = true;
		break;

	case '8':
		fileName = inputFileName.objs.livingRoom;
		picState = ConstTools::PictureState::IMAGE;
		picSet = true;
		break;

	case '9':
		fileName = inputFileName.objs.architecture;
		picState = ConstTools::PictureState::IMAGE;
		picSet = true;
		break;

	case '0':
		fileName = inputFileName.objs.loadLine;
		picState = ConstTools::PictureState::IMAGE;
		picSet = true;
		break;

	case 45:
		// "="を押した時:
		fileName = inputFileName.dark.moon;
		picState = ConstTools::PictureState::IMAGE;
		picSet = true;
		break;

	case 94:
		// "~"を押した時:
		fileName = inputFileName.dark.weather;
		picState = ConstTools::PictureState::IMAGE;
		picSet = true;
		break;

	case 92:
		// "|"を押した時:
		fileName = inputFileName.one.lion;
		picState = ConstTools::PictureState::IMAGE;
		picSet = true;
		break;

	case 'q':
		fileName = inputFileName.one.child;
		picState = ConstTools::PictureState::IMAGE;
		picSet = true;
		break;

	case 'w':
		fileName = inputFileName.mul.brothers;
		picState = ConstTools::PictureState::IMAGE;
		picSet = true;
		break;

	case 'e':
		fileName = inputFileName.mul.sibling;
		picState = ConstTools::PictureState::IMAGE;
		picSet = true;
		break;


	case 'r':
		fileName = inputIPUFileName.non.sky;
		picState = ConstTools::PictureState::IPU;
		picSet = true;
		break;

	case 't':
		fileName = inputIPUFileName.non.grass;
		picState = ConstTools::PictureState::IPU;
		picSet = true;
		break;

	case 'y':
		fileName = inputIPUFileName.obj.tree;
		picState = ConstTools::PictureState::IPU;
		picSet = true;
		break;

	case 'u':
		fileName = inputIPUFileName.obj.fireHydrant;
		picState = ConstTools::PictureState::IPU;
		picSet = true;
		break;

	case 'i':
		fileName = inputIPUFileName.obj.sign;
		picState = ConstTools::PictureState::IPU;
		picSet = true;
		break;

	case 'o':
		fileName = inputIPUFileName.objs.corn;
		picState = ConstTools::PictureState::IPU;
		picSet = true;
		break;

	case 'p':
		fileName = inputIPUFileName.objs.vendingMachine;
		picState = ConstTools::PictureState::IPU;
		picSet = true;
		break;

	case 64:
		// "`"を押した時:
		fileName = inputIPUFileName.objs.treeTrees;
		picState = ConstTools::PictureState::IPU;
		picSet = true;
		break;

	case 91:
		// "{"を押した時:
		fileName = inputIPUFileName.objs.board;
		picState = ConstTools::PictureState::IPU;
		picSet = true;
		break;

	case 58:
		// "*"を押した時:
		fileName = inputIPUFileName.objs.pc;
		picState = ConstTools::PictureState::IPU;
		picSet = true;
		break;

	case 93:
		// "}"を押した時:
		fileName = inputIPUFileName.objs.landscape;
		picState = ConstTools::PictureState::IPU;
		picSet = true;
		break;

	default:
		break;
	}

	switch (picState)
	{
	case ConstTools::PictureState::IMAGE:
		inputFilePath = prefixPath.image + "/" + fileName + ext.jpg;
		folderName = prefixPath.image;
		break;
	case ConstTools::PictureState::IPU:
		inputFilePath = prefixPath.image_IPU + "/" + fileName + ext.jpg;
		folderName = prefixPath.image_IPU;
		break;
	default:
		break;
	}

	if (picSet)
	{
		eyeGazePath = prefixPath.eyeGaze + "/" + folderName + "/" + fileName + "/" + outputOfEyeFileName.eyeGazeHeatMapGray + ext.png;
		setupView(inputFilePath);
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
