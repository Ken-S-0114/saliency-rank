#include "ofMain.h"
#include "ofxOpenCv.h"

class ConstTools {
public:

	struct InputFileName {
		std::string lenna = "lenna.png";
		std::string christmas = "sample.jpg";
		std::string cardboard = "sample2.jpg";
	};

	struct OutputFileName {
		std::string outputOfSaliencyImg = "outputOfSaliencyImg.png";
		std::string outputOfWatershedAfterImg = "outputOfWatershedAfterImg.png";
		std::string outputOfSaliencyMapHighestImg = "outputOfSaliencyMapHighestImg.png";
	};

	// ŠÂ‹«Žw’è
	enum Use {
		RELEASE,
		DEBUG,
		SALIENCY
	};

};
