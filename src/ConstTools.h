#include "ofMain.h"
#include "ofxOpenCv.h"

class ConstTools {
public:

	struct InputFileName {
		std::string lenna = "lenna.jpg";
		//std::string lenna = "lenna-mac.png";
		std::string mountain = "mountain.jpg";
		std::string sea = "sea.jpg";
		std::string sunflower = "sunflower.jpg";
	};

	struct OutputFileName {
		std::string outputOfSaliencyImg = "outputOfSaliencyImg.png";
		std::string outputOfWatershedAfterImg = "outputOfWatershedAfterImg.png";
		std::string outputOfSaliencyMapHighestImg = "outputOfSaliencyMapHighestImg.png";
		std::string outputOfEyeGazeHeatMapImg = "outputOfEyeGazeHeatMapImg.png";
	};

	enum Use {
		RELEASE,
		DEBUG,
		EYETRACK,
		EYETRACKHEATMAP,
		IMAGEVIEW
	};

	enum EyeTrackState {
		STANDBY,
		TRACKING
	};

};
