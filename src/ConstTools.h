#include "ofMain.h"
#include "ofxOpenCv.h"

class ConstTools {
public:

	struct InputFileName {
		std::string lenna = "lenna.jpg";
		std::string lennamac = "lenna-mac.png";

		std::string mountain = "mountain.jpg";
		std::string sunflower = "sunflower.jpg";
		std::string group = "group.jpg";
		std::string outdoors = "outdoors.jpg";
		std::string board = "board.jpg";
		std::string fireworks = "fireworks.jpg";

		std::string dog = "dog";
		std::string nightview = "nightview";
		std::string groupphoto = "groupphoto";
		std::string lighttower = "lighttower";
		std::string sea = "sea";
		std::string hotellandscape = "hotellandscape";
	};

	struct OutputFileName_Picture {
		std::string outputOfSaliencyImg = "outputOfSaliencyImg.png";
		std::string outputOfWatershedAfterImg = "outputOfWatershedAfterImg.png";
		std::string outputOfSaliencyMapHighestImg = "outputOfSaliencyMapHighestImg";
		//std::string outputOfSaliencyMapHighestImg = "outputOfSaliencyMapHighestImg.png";
	};

	struct OutputFileName_EyeGaze {
		std::string outputOfSaliencyImg = "outputOfSaliencyImg2.png";
		std::string outputOfWatershedAfterImg = "outputOfWatershedAfterImg2.png";
		std::string outputOfSaliencyMapHighestImg = "outputOfSaliencyMapHighestImg2";
		//std::string outputOfSaliencyMapHighestImg = "outputOfSaliencyMapHighestImg2.png";
		std::string outputOfEyeGazeHeatMapImg = "outputOfEyeGazeHeatMapImg.png";
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
		RESULT
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
