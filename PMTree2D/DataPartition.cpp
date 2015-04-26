#include "DataPartition.h"

using namespace std;

DataPartition::DataPartition(void)
{
}


DataPartition::~DataPartition(void)
{
}

void DataPartition::partition(cv::Mat_<float> X, cv::Mat_<float> Y, int minSize, vector<cv::Mat_<float> >&clusterX, vector<cv::Mat_<float> >&clusterY) {
	// サンプル数が32未満になるまで、繰り返し、dataX2、dataY2を分割する。
	cv::Mat samples;
	Y.convertTo(samples, CV_32F);
	cv::Mat centroids;
	cv::Mat labels;
	//cv::TermCriteria cri(cv::TermCriteria::MAX_ITER | cv::TermCriteria::EPS, 50, FLT_EPSILON);
	cv::TermCriteria cri(cv::TermCriteria::COUNT, 200, FLT_EPSILON);
	double compactness = cv::kmeans(samples, 2, labels, cri, 200, cv::KMEANS_PP_CENTERS, centroids);
		
	std::cout << labels << std::endl;

	int nClass1 = cv::countNonZero(labels);
	int nClass0 = X.rows - nClass1;

	if (nClass1 < minSize || nClass0 < minSize) {
		clusterX.push_back(X);
		clusterY.push_back(Y);
		return;
	}

	cv::Mat_<float> classX0(nClass0, X.cols);
	cv::Mat_<float> classX1(nClass1, X.cols);
	cv::Mat_<float> classY0(nClass0, Y.cols);
	cv::Mat_<float> classY1(nClass1, Y.cols);

	cout << X << endl;

	int index0 = 0;
	int index1 = 0;
	for (int r = 0; r < X.rows; ++r) {
		for (int c = 0; c < X.cols; ++c) {
			if (labels.at<float>(r, 0) == 0) {
				classX0(index0, c) = X(r, c);
			} else {
				classX1(index1, c) = X(r, c);
			}
		}
		for (int c = 0; c < Y.cols; ++c) {
			if (labels.at<float>(r, 0) == 0) {
				classY0(index0, c) = Y(r, c);
			} else {
				classY1(index1, c) = Y(r, c);
			}
		}
		if (labels.at<float>(r, 0) == 0) {
			index0++;
		} else {
			index1++;
		}
	}


	cout << classX0 << endl;
	cout << classX1 << endl;

	vector<cv::Mat_<float> > clusterX0, clusterX1;
	vector<cv::Mat_<float> > clusterY0, clusterY1;
	partition(classX0, classY0, minSize, clusterX0, clusterY0);
	partition(classX1, classY1, minSize, clusterX1, clusterY1);

	// clusterをマージする
	clusterX.insert(clusterX.end(), clusterX0.begin(), clusterX0.end());
	clusterX.insert(clusterX.end(), clusterX1.begin(), clusterX1.end());
	clusterY.insert(clusterY.end(), clusterY0.begin(), clusterY0.end());
	clusterY.insert(clusterY.end(), clusterY1.begin(), clusterY1.end());
}
