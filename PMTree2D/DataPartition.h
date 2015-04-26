#pragma once

#include <opencv/cv.h>
#include <opencv/highgui.h>

using namespace std;

class DataPartition {
public:
	DataPartition(void);
	~DataPartition(void);

	static void partition(cv::Mat_<float> X, cv::Mat_<float> Y, int minSize, vector<cv::Mat_<float> >&clusterX, vector<cv::Mat_<float> >&clusterY);
};

