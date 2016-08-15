#ifndef FRICBGS_HPP
#define FRICBGS_HPP

#include "dbg/RDBG.h"
#include "IBGS.h"

class FricBgs:public IBGS
{
public:
    FricBgs():firstTime(true){rdbg = nullptr;}
    ~FricBgs(){
        if(rdbg!=nullptr)
              delete rdbg;
    }

    void process(const Mat &img_input, Mat &img_foreground, Mat &img_background)
    {
        if(firstTime)
        {
            rdbg = new RDBG();
            firstTime = false;
        }

        rdbg->update(img_input);
        img_foreground = rdbg->getForeImg(0);
        img_background= img_input.clone();
    }

private:
    void loadConfig(){};
    void saveConfig(){};
    RDBG* rdbg;
    bool firstTime;
};
#endif // FRICBGS_HPP

