//
//  saliencyTools.hpp
//  saliency-rank
//
//  Created by 佐藤賢 on 2018/10/30.
//

#ifndef saliencyTools_hpp
#define saliencyTools_hpp


#include "ofMain.h"
#include "ofxOpenCv.h"

class SaliencyTools {
public:
    // 最小と最大の要素値とそれらの位置
    struct MinMax {
        cv::Point min_loc, max_loc;
        double min_val, max_val;
    };

};

#endif /* saliencyTools_hpp */
