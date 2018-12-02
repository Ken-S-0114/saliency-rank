#include "ofMain.h"
#include "ofxOpenCv.h"

class ConstTools {
public:

	struct InputFileName {
		std::string field = "field";
		std::string brick = "brick";

		std::string furniture = "furniture";
		std::string football = "football";
		std::string sunset = "sunset";
		std::string waterfall = "waterfall";

		std::string balloon = "balloon";
		std::string livingRoom = "living-room";
		std::string architecture = "architecture";
		std::string loadLine = "load-line";

		std::string moon = "moon";
		std::string weather = "weather";

		std::string lion = "lion";
		std::string child = "child";

		std::string brothers = "brothers";
		std::string sibling = "sibling";

	};

	struct InputIPUFileName {
		std::string sky = "sky";
		std::string grass = "grass";

		std::string tree = "tree";
		std::string fireHydrant = "fireHydrant";
		std::string sign = "sign";

		std::string corn = "corn";
		std::string vendingMachine = "vendingMachine";
		std::string treeTrees = "treeTrees";
		std::string board = "board";
		std::string pc = "pc";
		std::string landscape = "landscape";
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
	
	struct OutputOfImg
	{
		ofImage outputOfSaliencyImg, outputOfHeatMapImg;
		ofImage outputOfBackgroundImg, outputOfUnknownImg, outputOfWatershedImg, outputOfWatershedAfterImg;
		ofImage outputOfWatershedHighestImg, outputOfSaliencyMapHighestImg;
		ofImage outputOfEyeGazeImg, outputOfEyeGazeHeatMapImg;
		ofImage outputOfEyeGazeResultImg;
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
	
	struct PrefixPath
	{
		std::string sampleImage = "sampleImage";
		std::string image = "image";
		std::string image_IPU = "image_IPU";
		std::string picture = "result/picture";
		std::string eyeGaze = "result/eyeGaze";
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
