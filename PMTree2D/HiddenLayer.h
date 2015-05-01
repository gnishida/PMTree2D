#pragma once

#include <opencv/cv.h>
#include <opencv/highgui.h>

class HiddenLayer {
public:
	int n_in;
	int n_out;
	cv::Mat_<double> input;
	cv::Mat_<double> output;
	cv::Mat_<double> W;
	cv::Mat_<double> b;

public:
	HiddenLayer(cv::Mat_<double> input, int n_in, int n_out);
	HiddenLayer(cv::Mat_<double> input, int n_in, int n_out, cv::Mat_<double> W, cv::Mat_<double> b);
	cv::Mat_<double> params();
	cv::Mat_<double> predict(const cv::Mat_<double>& input);
	void grad(const cv::Mat_<double>& delta, double lambda, cv::Mat_<double>& dW, cv::Mat_<double>& db);
	cv::Mat_<double> back_propagation(const cv::Mat_<double>& delta, double lambda, double alpha);

private:
	void init(cv::Mat_<double> input, int n_in, int n_out, cv::Mat_<double> W, cv::Mat_<double> b);
	cv::Mat_<double> mat_tanh(const cv::Mat_<double>& mat);
};

