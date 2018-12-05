#ifndef OPENCV_H
#define OPENCV_H

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "opencv2/opencv.hpp"
#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include <iostream>
#include <mutex>
#include <condition_variable>

static std::mutex mu;
static std::condition_variable _cv_buffer;
static cv::Mat _frame;
static std::deque<cv::Mat> _buffer;

class OpenCV
{
    public:
        cv::VideoCapture startStream();
        void stopStream();
        void frameBuffer(cv::VideoCapture _cap);
        void startFrameRate();
        void endFrameRate();
        void countFrameRate(bool start);
        float getFrameRate();
        cv::Mat getFrameColor();
        cv::Mat getFrameGray();
        cv::Mat getFaceFrame(float circle_dia);
        cv::Mat getRGBFrame(int chR, int chG, int chB);

    private:
        int _queue_length = 1000;
        cv::Mat _output, _output1, _frameFace, _frameRGB;
        int frame_count = 0;
        float frame_rate;


        void add(cv::Mat _frame)
        {
            std::unique_lock<std::mutex> buff_lock(mu);
            _cv_buffer.wait(buff_lock, [this]{ return !isFull(); });
            _buffer.push_front(_frame);
            buff_lock.unlock();
            _cv_buffer.notify_one();
        }

        cv::Mat get()
        {
            std::unique_lock<std::mutex> buff_lock(mu);
            _cv_buffer.wait(buff_lock, [this]{ return !isEmpty(); });
            cv::Mat request = _buffer.back();
            _buffer.pop_back();
            buff_lock.unlock();
            _cv_buffer.notify_one();
            return request;
        }

        bool isFull() const
        {
           return _buffer.size() >= static_cast<unsigned int>(_queue_length);
        }

        bool isEmpty() const
        {
            return _buffer.size() == static_cast<unsigned int>(0);
        }
};
#endif
