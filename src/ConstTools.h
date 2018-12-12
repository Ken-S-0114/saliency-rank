#include "ofMain.h"
#include "ofxOpenCv.h"

class ConstTools {
public:

	struct InputFileName {
		struct NonObject {
			const std::string field = "field";
			const std::string brick = "brick";
		};

		struct Object {
			const std::string furniture = "furniture";
			const std::string football = "football";
			const std::string sunset = "sunset";
			const std::string waterfall = "waterfall";
		};

		struct ManyObjects {
			const std::string balloon = "balloon";
			const std::string livingRoom = "living-room";
			const std::string architecture = "architecture";
			const std::string loadLine = "load-line";
		};

		struct Dark {
			const std::string moon = "moon";
			const std::string weather = "weather";
		};

		struct OnePerson {
			const std::string lion = "lion";
			const std::string child = "child";
		};

		struct MultiplePeople {
			const std::string brothers = "brothers";
			const std::string sibling = "sibling";
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
			const std::string sky = "sky";
			const std::string grass = "grass";
		};

		struct Object {
			const std::string tree = "tree";
			const std::string fireHydrant = "fireHydrant";
			const std::string sign = "sign";
		};

		struct ManyObjects {
			const std::string corn = "corn";
			const std::string vendingMachine = "vendingMachine";
			const std::string treeTrees = "treeTrees";
			const std::string board = "board";
			const std::string pc = "pc";
			const std::string landscape = "landscape";
		};

		NonObject non;
		Object obj;
		ManyObjects objs;
	};

	struct InputMockFileName {
		const std::string lenna = "lenna";
		const std::string dog = "dog";
		const std::string nightview = "nightview";
		const std::string groupphoto = "groupphoto";
		const std::string lighttower = "lighttower";
		const std::string sea = "sea";
		const std::string hotellandscape = "hotellandscape";
	};
	
	struct OutputOfImg
	{
		ofImage saliencyMap, heatMap;
		ofImage loadMat8UC1, gray;
		ofImage background, unknown, watershed, watershedAfter;
		ofImage watershedHighest, saliencyMapHighest;
		ofImage eyeGaze, eyeGazeHeatMap;
		ofImage eyeGazeResult;
	};

	struct OutputOfPicFileName {
		const std::string saliencyMap = "saliencyMap";
		const std::string watershed = "watershed";
		const std::string saliencyMapHighest = "outputOfSaliencyMapHighestImg";
	};

	struct OutputOfEyeFileName {
		const std::string watershed = "watershed";
		const std::string saliencyMapHighest = "saliencyMapHighest";
		const std::string eyeGazeHeatMapGray = "eyeGazeHeatMapGray";
		const std::string eyeGazeHeatMapGraySoft = "eyeGazeHeatMapGraySoft";
		const std::string eyeGazeHeatMap = "eyeGazeHeatMap";
		const std::string eyeGazeHeatMapSoft = "eyeGazeHeatMapSoft";
		const std::string rank = "eyeGazeRank";
	};
	
	struct PrefixPath
	{
		const std::string sampleImage = "sampleImage";
		const std::string image = "image";
		const std::string image_IPU = "image_IPU";
		const std::string picture = "result/picture";
		const std::string eyeGaze = "result/eyeGaze";
		const std::string rank = "rank";
	};

	struct FileNameExtension {
		const std::string jpg = ".jpg";
		const std::string png = ".png";
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

	enum RankingState {
		MAKING,
		DONE,
		NOT
	};

	enum PictureState {
		NONPIC,
		IMAGE,
		IPU
	};
};
