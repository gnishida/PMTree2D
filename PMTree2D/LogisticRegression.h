#pragma once

#include <opencv/cv.h>
#include <opencv/highgui.h>

class LogisticRegression {
private:
	int N;				// データ数
	cv::Mat_<double> W;
	cv::Mat_<double> b;

public:
	LogisticRegression(const cv::Mat_<double>& X, const cv::Mat_<double>& Y, float lambda, float alpha, int maxIter);

	cv::Mat_<double> predict(const cv::Mat_<double>& x);

private:
	void train(const cv::Mat_<double>& X, const cv::Mat_<double>& Y, float lambda, float alpha, int maxIter);
	double cost(const cv::Mat_<double>& X, const cv::Mat_<double>& Y, float lambda);
	void grad(const cv::Mat_<double>& X, const cv::Mat_<double>& Y, float lambda, cv::Mat_<double>& dW, cv::Mat_<double>& db);

	void numericalGrad(const cv::Mat_<double>& X, const cv::Mat_<double>& Y, float lambda, cv::Mat_<double>& dW, cv::Mat_<double>& db);
	void encodeParam(const cv::Mat_<double>& W, const cv::Mat_<double>& b, cv::Mat_<double>& theta);
	void decodeParam(const cv::Mat_<double>& theta, cv::Mat_<double>& W, cv::Mat_<double>& b);
};

