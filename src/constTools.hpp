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

    struct FileName {
        std::string lenna = "lenna.png";
        std::string christmas = "sample.jpg";
        std::string cardboard = "sample2.jpg";
    };

    // 環境指定
    enum Use {
        RELEASE,
        DEBUG,
        SALIENCY
    };
    
};
#endif /* constTools_hpp */
