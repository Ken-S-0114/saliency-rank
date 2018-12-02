#include "ofMain.h"
#include "ofxOpenCv.h"

class ConstTools {
public:

	struct InputFileName {
		struct NonObject {
			std::string field = "field";
			std::string brick = "brick";
		};

		struct Object {
		std::string furniture = "furniture";
		std::string football = "football";
		std::string sunset = "sunset";
		std::string waterfall = "waterfall";
		};

		struct ManyObjects {
		std::string balloon = "balloon";
		std::string livingRoom = "living-room";
		std::string architecture = "architecture";
		std::string loadLine = "load-line";
		};

		struct Dark {
		std::string moon = "moon";
		std::string weather = "weather";
		};

		struct OnePerson {
		std::string lion = "lion";
		std::string child = "child";
		};

		struct MultiplePeople {
		std::string brothers = "brothers";
		std::string sibling = "sibling";
		};

		NonObject non;
		Object obj;
		ManyObjects objs;
		Dark dark;
		OnePerson one;
		MultiplePeople mul;
	};

	struct InputIPUFileName {
		struct NonObject {
		std::string sky = "sky";
		std::string grass = "grass";
		};

		struct Object {
		std::string tree = "tree";
		std::string fireHydrant = "fireHydrant";
		std::string sign = "sign";
		};

		struct ManyObjects {
		std::string corn = "corn";
		std::string vendingMachine = "vendingMachine";
		std::string treeTrees = "treeTrees";
		std::string board = "board";
		std::string pc = "pc";
		std::string landscape = "landscape";
		};

		NonObject non;
		Object obj;
		ManyObjects objs;
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
		ofImage saliencyMap, heatMap;
		ofImage background, unknown, watershed, watershedAfter;
		ofImage watershedHighest, saliencyMapHighest;
		ofImage eyeGaze, eyeGazeHeatMap;
		ofImage eyeGazeResult;
	};

	struct OutputFileName_Picture {
		std::string saliencyMap = "outputOfSaliencyImg";
		std::string watershed = "outputOfWatershedImg";
		std::string saliencyMapHighest = "outputOfSaliencyMapHighestImg";
	};

	struct OutputFileName_EyeGaze {
		std::string watershed = "outputOfWatershedImg";
		std::string saliencyMapHighest = "outputOfSaliencyMapHighestImg";
		std::string eyeGazeHeatMap = "outputOfEyeGazeHeatMapImg";
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
		PICLOAD,
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
