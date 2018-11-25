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
        std::string christmas = "sample";
        std::string cardboard = "sample2.jpg";

        std::string landscape = "landscape";

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
        FIRST,
        SECOND
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
#endif /* constTools_hpp */
