#include "ofMain.h"
#include "ofxOpenCv.h"

class ConstTools {
public:

	struct InputFileName {
		std::string lenna = "lenna.jpg";
		//std::string christmas = "sample.jpg";
		//std::string cardboard = "sample2.jpg";
	};

	struct OutputFileName {
		std::string outputOfSaliencyImg = "outputOfSaliencyImg.png";
		std::string outputOfWatershedAfterImg = "outputOfWatershedAfterImg.png";
		std::string outputOfSaliencyMapHighestImg = "outputOfSaliencyMapHighestImg.png";
	};

	enum Use {
		RELEASE,
		DEBUG,
		EYETRACK,
		EYETRACKHEATMAP
	};

	enum EyeTrackState {
		STANDBY,
		TRACKING
	};

};
