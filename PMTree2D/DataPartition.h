#pragma once

#include <opencv/cv.h>
#include <opencv/highgui.h>

using namespace std;

class DataPartition {
protected:
	DataPartition() {}

public:
	static void partition(cv::Mat_<float> X, cv::Mat_<float> Y, cv::Mat_<float> Z, vector<int> indices, int minSize, vector<cv::Mat_<float> >&clusterX, vector<cv::Mat_<float> >&clusterY, vector<cv::Mat_<float> >&clusterZ, vector<vector<int> >& clusterIndices);
};

