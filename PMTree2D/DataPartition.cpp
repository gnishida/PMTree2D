﻿#include "DataPartition.h"

using namespace std;

/**
 * データYをk-meansクラスタリングで階層的に分割していく。
 * それに合わせて、データX、データZも分割する。
 * データX、データY、データZの行数は同じであること。
 *
 * @param X						データX
 * @param Y						データY
 * @param Z						データZ
 * @param indices				各データ要素の、元データでのindex番号
 * @param minSize				クラスタの最小サイズ
 * @param clusterX [OUT]		データXのクラスタリング結果
 * @param clusterY [OUT]		データYのクラスタリング結果
 * @param clusterZ [OUT]		データZのクラスタリング結果
 * @param clusterIndices [OUT]	クラスタリング結果における各要素の、元データでのindex番号
 */
void DataPartition::partition(cv::Mat_<float> X, cv::Mat_<float> Y, cv::Mat_<float> Z, vector<int> indices, int minSize, vector<cv::Mat_<float> >&clusterX, vector<cv::Mat_<float> >&clusterY, vector<cv::Mat_<float> >&clusterZ, vector<vector<int> >& clusterIndices) {
	// サンプル数が32未満になるまで、繰り返し、dataX2、dataY2を分割する。
	cv::Mat samples;
	Y.convertTo(samples, CV_32F);
	cv::Mat centroids;
	cv::Mat labels;
	//cv::TermCriteria cri(cv::TermCriteria::MAX_ITER | cv::TermCriteria::EPS, 50, FLT_EPSILON);
	cv::TermCriteria cri(cv::TermCriteria::COUNT, 200, FLT_EPSILON);
	double compactness = cv::kmeans(samples, 2, labels, cri, 200, cv::KMEANS_PP_CENTERS, centroids);
		
	int nClass1 = cv::countNonZero(labels);
	int nClass0 = X.rows - nClass1;

	if (nClass1 < minSize || nClass0 < minSize) {
		clusterX.push_back(X);
		clusterY.push_back(Y);
		clusterZ.push_back(Z);
		clusterIndices.push_back(indices);
		return;
	}

	cv::Mat_<float> classX0(nClass0, X.cols);
	cv::Mat_<float> classX1(nClass1, X.cols);
	cv::Mat_<float> classY0(nClass0, Y.cols);
	cv::Mat_<float> classY1(nClass1, Y.cols);
	cv::Mat_<float> classZ0(nClass0, Z.cols);
	cv::Mat_<float> classZ1(nClass1, Z.cols);
	vector<int> indices0(nClass0);
	vector<int> indices1(nClass1);

	int index0 = 0;
	int index1 = 0;
	for (int r = 0; r < X.rows; ++r) {
		if (labels.at<float>(r, 0) == 0) {
			for (int c = 0; c < X.cols; ++c) {
				classX0(index0, c) = X(r, c);
			}
			for (int c = 0; c < Y.cols; ++c) {
				classY0(index0, c) = Y(r, c);
			}
			for (int c = 0; c < Z.cols; ++c) {
				classZ0(index0, c) = Z(r, c);
			}
			indices0[index0] = indices[r];

			index0++;
		} else {
			for (int c = 0; c < X.cols; ++c) {
				classX1(index1, c) = X(r, c);
			}
			for (int c = 0; c < Y.cols; ++c) {
				classY1(index1, c) = Y(r, c);
			}
			for (int c = 0; c < Z.cols; ++c) {
				classZ1(index1, c) = Z(r, c);
			}
			indices1[index1] = indices[r];

			index1++;
		}
	}

	/*
	vector<cv::Mat_<float> > clusterX0, clusterX1;
	vector<cv::Mat_<float> > clusterY0, clusterY1;
	vector<cv::Mat_<float> > clusterZ0, clusterZ1;
	*/
	partition(classX0, classY0, classZ0, indices0, minSize, clusterX, clusterY, clusterZ, clusterIndices);
	partition(classX1, classY1, classZ1, indices1, minSize, clusterX, clusterY, clusterZ, clusterIndices);

	// clusterをマージする
	/*
	clusterX.insert(clusterX.end(), clusterX0.begin(), clusterX0.end());
	clusterX.insert(clusterX.end(), clusterX1.begin(), clusterX1.end());
	clusterY.insert(clusterY.end(), clusterY0.begin(), clusterY0.end());
	clusterY.insert(clusterY.end(), clusterY1.begin(), clusterY1.end());
	clusterZ.insert(clusterZ.end(), clusterZ0.begin(), clusterZ0.end());
	clusterZ.insert(clusterZ.end(), clusterZ1.begin(), clusterZ1.end());
	*/
}
