//
//  constTools.hpp
//  saliency-rank
//
//  Created by 佐藤賢 on 2018/11/02.
//

#ifndef constTools_hpp
#define constTools_hpp

#include "ofMain.h"
#include "ofxOpenCv.h"

class ConstTools {
public:

    struct InputFileName {
        std::string lenna = "lenna.png";
        std::string christmas = "sample.jpg";
        std::string cardboard = "sample2.jpg";

//        std::string lenna = "lenna.jpg";
//        //std::string lenna = "lenna-mac.png";
//        std::string mountain = "mountain.jpg";
//        std::string sea = "sea.jpg";
//        std::string sunflower = "sunflower.jpg";
    };

    struct OutputFileName_FIRST {
        std::string outputOfSaliencyImg = "outputOfSaliencyImg.png";
        std::string outputOfWatershedAfterImg = "outputOfWatershedAfterImg.png";
        std::string outputOfSaliencyMapHighestImg = "outputOfSaliencyMapHighestImg.png";
        std::string outputOfEyeGazeHeatMapImg = "outputOfEyeGazeHeatMapImg.png";
    };

    struct OutputFileName_SECOND {
        std::string outputOfSaliencyImg = "outputOfSaliencyImg2.png";
        std::string outputOfWatershedAfterImg = "outputOfWatershedAfterImg2.png";
        std::string outputOfSaliencyMapHighestImg = "outputOfSaliencyMapHighestImg2.png";
        std::string outputOfEyeGazeHeatMapImg = "outputOfEyeGazeHeatMapImg2.png";
    };

    enum Use {
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
        FIRST,
        SECOND
    };

    enum Infomation {
        VIEW,
        HIDE
    };

};
#endif /* constTools_hpp */
