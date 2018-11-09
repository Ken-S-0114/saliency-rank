#include "ofMain.h"
#include "ofxOpenCv.h"

class SaliencyTools {
public:
	// 最小と最大の要素値とそれらの位置
	struct MinMax {
		cv::Point min_loc, max_loc;
		double min_val, max_val;
	};
	//    // 最小の位置
	//    struct MinPlace {
	//        int widthMin;
	//        int heightMin;
	//    };

	//    void saliencyWatershed(ofImage img);

};
