#ifndef FRAMERATE_H
#define FRAMERATE_H

#include <chrono>




class FrameRate
{

    private:
        int frames = 0;
        double starttime = 0;
        bool first = true;
        float fps = 0.0;

    public:
        void startClock();
        void endClock();
        void counter();

};
#endif
