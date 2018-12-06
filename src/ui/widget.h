#ifndef WIDGET_H
#define WIDGET_H

#include "opencv.h"

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include <QWidget>
#include <QTimer>
#include <QSlider>

#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <vector>


namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

    public:
        explicit Widget(QWidget *parent = nullptr);
        ~Widget();

    private:
        Ui::Widget *ui;
        cv::VideoCapture cap_w;
        cv::Mat frame_w;
        QSlider *slider;
        QTimer *timer;
        QImage qt_image;
        bool stateThread = true;
        bool stateStream = true;
        std::thread thread_buffer;
        OpenCV cl_o;

    private slots:
        void on_pushButton_open_Webcam_clicked();
        void on_pushButton_close_Webcam_clicked();
        void update_window();
        void start_thread();
        void on_checkBox_CannyEdge_clicked(bool checked);
        void canny_edge();
        void on_horizontalSlider_CannyEdge_valueChanged();
        void on_checkBox_FaceDetection_clicked(bool checked);
        void face_detector();
        void on_horizontalSlider_FaceDetection_valueChanged();
        void on_checkBox_RGB_Modifier_clicked(bool checked);
        void on_verticalSlider_R_valueChanged();
        void on_verticalSlider_G_valueChanged();
        void on_verticalSlider_B_valueChanged();
        void rgb_modifier();
        void on_checkBox_FancyMode_clicked(bool checked);
        void fancy_mode();
};

#endif // WIDGET_H
