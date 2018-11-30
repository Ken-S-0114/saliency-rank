#include "ofMain.h"
#include "ofxOpenCv.h"

class ConstTools {
public:

	struct InputFileName {
		std::string architecture = "architecture";
		std::string balloon = "balloon";
		std::string field = "field";
		std::string furniture = "furniture";
		std::string lion = "lion";
		std::string livingRoom = "living-room";
		std::string loadLine = "load-line";
		std::string moon = "moon";
		std::string sibling = "sibling";
		std::string sunset = "sunset";
		std::string waterfall = "waterfall";
		std::string weather = "weather";
	};

	struct InputMockFileName {
		std::string lenna = "lenna";

		std::string dog = "dog";
		std::string nightview = "nightview";
		std::string groupphoto = "groupphoto";
		std::string lighttower = "lighttower";
		std::string sea = "sea";
		std::string hotellandscape = "hotellandscape";
	};

	struct OutputFileName_Picture {
		std::string outputOfSaliencyImg = "outputOfSaliencyImg";
		std::string outputOfWatershedImg = "outputOfWatershedImg";
		std::string outputOfSaliencyMapHighestImg = "outputOfSaliencyMapHighestImg";
	};

	struct OutputFileName_EyeGaze {
		std::string outputOfWatershedImg = "outputOfWatershedImg";
		std::string outputOfSaliencyMapHighestImg = "outputOfSaliencyMapHighestImg";
		std::string outputOfEyeGazeHeatMapImg = "outputOfEyeGazeHeatMapImg";
	};

	struct FileNameExtension {
		std::string jpgPath = ".jpg";
		std::string pngPath = ".png";
	};

	enum Mode {
		RELEASE,
		DEBUG,
		EYETRACK,
		EYETRACKHEATMAP,
		IMAGEVIEW,
		RESULT,
		SLEEP
	};

	enum EyeTrackState {
		STANDBY,
		TRACKING,
		SAVE
	};

	enum LoadState {
		SALLOAD,
		EYELOAD
	};

	enum Infomation {
		VIEW,
		HIDE
	};

	enum EnterState {
		NONE,
		SALIENCYMAP,
		EYEGAZE
	};
};
