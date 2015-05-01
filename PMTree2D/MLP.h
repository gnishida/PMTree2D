#pragma once

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "HiddenLayer.h"
#include "LogisticRegression.h"
#include "LinearRegression.h"

using namespace std;
using namespace cv;

class MLP {
private:
	HiddenLayer* hiddenLayer;
	//LogisticRegression* regressionLayer;
	LinearRegression* regressionLayer;

public:
	MLP(const Mat_<double>& X, const Mat_<double>& Y, int n_hidden);

	void train(const Mat_<double>& X, const Mat_<double>& Y, double lambda, double alpha, int maxIter);
	void numericalGrad(const cv::Mat_<double>& X, const cv::Mat_<double>& Y, double lambda, cv::Mat_<double>& dW1, cv::Mat_<double>& db1, cv::Mat_<double>& dW2, cv::Mat_<double>& db2);
	cv::Mat_<double> predict(const Mat_<double>& input);

	double cost(const cv::Mat_<double>& X, const cv::Mat_<double>& Y, double lambda);
	void encodeParams(const cv::Mat_<double>& W1, const cv::Mat_<double>& b1, const cv::Mat_<double>& W2, const cv::Mat_<double>& b2, cv::Mat_<double>& theta);
	void decodeParams(const cv::Mat_<double>& theta, cv::Mat_<double>& W1, cv::Mat_<double>& b1, cv::Mat_<double>& W2, cv::Mat_<double>& b2);

private:
	double mat_sqsum(const cv::Mat_<double> mat);
	double mat_sum(const cv::Mat_<double> mat);
};

