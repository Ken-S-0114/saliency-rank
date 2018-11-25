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

        std::string fireworks2 = "fireworks2.jpg";
        std::string castle = "castle.jpg";
        std::string landscape = "landscape.jpg";
        std::string forest = "forest.jpg";

        std::string dandelion = "dandelion.jpg";
        std::string group = "group.jpg";
        std::string fireworks = "fireworks.jpg";

        std::string snowylandscape = "snowylandscape.jpg";
        std::string hotellandscape = "hotellandscape.jpg";
        std::string cherryblossoms = "cherryblossoms.jpg";
        std::string hotel = "hotel.jpg";
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
