#ifndef OPENCV_H
#define OPENCV_H

//FIXME use <> for system includes, "" for local
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "opencv2/opencv.hpp"
#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include <iostream>
#include <mutex>
#include <condition_variable>

//FIXME why would you want global declarations of the following variables? should be possible to make them member variables.
static std::mutex mu;
static std::condition_variable cv_buffer;
static cv::Mat frame;
static std::deque<cv::Mat> buffer;

class OpenCV
{
    public:
        cv::VideoCapture startStream();
        void stopStream();
        void frameBuffer(cv::VideoCapture _cap);
        cv::Mat getFrameColor();
        cv::Mat getFrameGray();
        cv::Mat getFaceFrame(float circle_dia);
        cv::Mat getRGBFrame(int chR, int chG, int chB);

    private:
        //FIXME assign default value with {} i.e. int _queue_length{1000}
        //FIXME never start variables with _, might collide with compiler standards i.e. int m_queue_length{1000}
        int _queue_length = 1000;

        //FIXME use header files for declaration, move implementation to source file (.cpp)

        /**
         * @brief add
         * @param _frame:   frame from the camera stream
         * function add singel frames to the ringbuffer, mutex protected
         */
        void add(cv::Mat _frame)
        {
            // NOTE why would you prefer unique lock over lock_guard?
            std::unique_lock<std::mutex> buff_lock(mu);
            cv_buffer.wait(buff_lock, [this]{ return !isFull(); });
            buffer.push_front(_frame);
            buff_lock.unlock();
            cv_buffer.notify_one();
        }

        /**
         * @brief get
         * @return:     single frame from the ringbuffer
         */
        cv::Mat get()
        {
            // NOTE why would you prefer unique lock over lock_guard?
            std::unique_lock<std::mutex> buff_lock(mu);
            cv_buffer.wait(buff_lock, [this]{ return !isEmpty(); });
            cv::Mat request = buffer.back();
            buffer.pop_back();
            buff_lock.unlock();
            cv_buffer.notify_one();
            return request;
        }

        /**
         * @brief isFull
         * @return:     check if ringbuffer is full
         */
        bool isFull() const
        {
           return buffer.size() >= static_cast<unsigned int>(_queue_length);
        }

        /**
         * @brief isEmpty
         * @return:     check if reingbuffer is empty
         */
        bool isEmpty() const
        {
            return buffer.size() == static_cast<unsigned int>(0);
        }
};
#endif
