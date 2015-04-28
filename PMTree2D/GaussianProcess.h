#pragma once

#include <opencv/cv.h>
#include <opencv/highgui.h>

class GaussianProcess {
private:
	float theta_0;
	float theta_1;
	float theta_2;
	float theta_3;
	cv::Mat_<double> invCov;

public:
	GaussianProcess(const cv::Mat_<double>& X);
	cv::Mat_<double> predict(const cv::Mat_<double>& x, const cv::Mat_<double>& X, const cv::Mat_<double>& Y);
	double covariance_function(const cv::Mat_<double>& x1, const cv::Mat_<double>& x2);
};

