#include "MLP.h"
#include <iostream>
#include <sstream>

/**
 * Neural networkを構築する。
 *
 * @param X				入力データ（各行が、各入力x_iに相当）
 * @param Y				出力データ（各行が、各出力y_iに相当）
 * @param hiddenSize	hiddenレイヤのユニット数
 */
MLP::MLP(const Mat_<double>& X, const Mat_<double>& Y, int n_hidden) {
	int n_in = X.cols;
	int n_out = Y.cols;
	hiddenLayer = new HiddenLayer(X, n_in, n_hidden);
	regressionLayer = new LinearRegression(hiddenLayer->output, n_hidden, n_out);
}

/**
 * 学習する。
 *
 * @return			コスト
 */
void MLP::train(const Mat_<double>& X, const Mat_<double>& Y, double lambda, double alpha, int maxIter) {
	// 微分のチェック
	{
		cv::Mat_<double> theta1;
		cv::Mat_<double> dW1, db1, dW2, db2;
		regressionLayer->grad(Y - predict(X), lambda, dW2, db2);
		hiddenLayer->grad((Y - predict(X)) * regressionLayer->W.t(), lambda, dW1, db1);
		encodeParams(dW1, db1, dW2, db2, theta1);

		cv::Mat_<double> theta2;
		numericalGrad(X, Y, lambda, dW1, db1, dW2, db2);
		encodeParams(dW1, db1, dW2, db2, theta2);

		for (int i = 0; i < theta1.cols; ++i) {
			cout << theta1(0, i) << ", " << theta2(0, i) << ", " << theta1(0, i) - theta2(0, i) << endl;
			if (abs(theta1(0, i) - theta2(0, i)) > 0.00001) {
				cout << "There seems something wrong with derivatives." << endl;
				return;
			}
		}
	}

	// トレーニング
	for (int iter = 0; iter < maxIter; ++iter) {
		predict(X);

		cout << "Score: " << cost(X, Y, lambda) << endl;

		cv::Mat_<double> h2 = predict(X);

		cv::Mat_<double> theta1;
		cv::Mat_<double> dW1, db1, dW2, db2;
		regressionLayer->grad(Y - h2, lambda, dW2, db2);
		hiddenLayer->grad((Y - h2) * regressionLayer->W.t(), lambda, dW1, db1);
		
		// W、bを更新
		hiddenLayer->W -= dW1 * alpha;
		hiddenLayer->b -= db1 * alpha;
		regressionLayer->W -= dW2 * alpha;
		regressionLayer->b -= db2 * alpha;
	}

	// 微分のチェック
	/*
	{
		predict(X);

		cv::Mat_<double> theta1;
		cv::Mat_<double> dW1, db1, dW2, db2;
		regressionLayer->grad(Y - predict(X), lambda, dW2, db2);
		hiddenLayer->grad((Y - predict(X)) * regressionLayer->W.t(), lambda, dW1, db1);
		encodeParams(dW1, db1, dW2, db2, theta1);

		cv::Mat_<double> theta2;
		numericalGrad(X, Y, lambda, dW1, db1, dW2, db2);
		encodeParams(dW1, db1, dW2, db2, theta2);

		for (int i = 0; i < theta1.cols; ++i) {
			cout << theta1(0, i) << ", " << theta2(0, i) << ", " << theta1(0, i) - theta2(0, i) << endl;
			if (abs(theta1(0, i) - theta2(0, i)) > 0.00001) {
				cout << "There seems something wrong with derivatives." << endl;
				//return;
			}
		}
	}
	*/
}

void MLP::numericalGrad(const cv::Mat_<double>& X, const cv::Mat_<double>& Y, double lambda, cv::Mat_<double>& dW1, cv::Mat_<double>& db1, cv::Mat_<double>& dW2, cv::Mat_<double>& db2) {
	cv::Mat_<double> backup_W1, backup_b1, backup_W2, backup_b2;
	hiddenLayer->W.copyTo(backup_W1);
	hiddenLayer->b.copyTo(backup_b1);
	regressionLayer->W.copyTo(backup_W2);
	regressionLayer->b.copyTo(backup_b2);

	cv::Mat_<double> theta;
	encodeParams(hiddenLayer->W, hiddenLayer->b, regressionLayer->W, regressionLayer->b, theta);

	cv::Mat_<double> g(theta.size());
	double epsilon = 0.0001;
	for (int i = 0; i < theta.cols; ++i) {
		cv::Mat_<double> d_theta = cv::Mat_<double>::zeros(theta.size());
		d_theta(0, i) = epsilon;

		decodeParams(theta + d_theta, hiddenLayer->W, hiddenLayer->b, regressionLayer->W, regressionLayer->b);

		predict(X);
		double c1 = cost(X, Y, lambda);
		decodeParams(theta - d_theta, hiddenLayer->W, hiddenLayer->b, regressionLayer->W, regressionLayer->b);

		predict(X);
		double c2 = cost(X, Y, lambda);

		g(0, i) = (c1 - c2) / 2.0 / epsilon;
	}
	decodeParams(g, dW1, db1, dW2, db2);

	backup_W1.copyTo(hiddenLayer->W);
	backup_b1.copyTo(hiddenLayer->b);
	backup_W2.copyTo(regressionLayer->W);
	backup_b2.copyTo(regressionLayer->b);
}

cv::Mat_<double> MLP::predict(const Mat_<double>& input) {
	cv::Mat_<double> h = hiddenLayer->predict(input);
	return regressionLayer->predict(h);
}

double MLP::cost(const cv::Mat_<double>& X, const cv::Mat_<double>& Y, double lambda) {
	double L = mat_sqsum(Y - regressionLayer->output) * 0.5;
	L += lambda * 0.5 * (mat_sqsum(hiddenLayer->W) + mat_sqsum(hiddenLayer->b) + mat_sqsum(regressionLayer->W) + mat_sqsum(regressionLayer->b));
	return L;
}

void MLP::encodeParams(const cv::Mat_<double>& W1, const cv::Mat_<double>& b1, const cv::Mat_<double>& W2, const cv::Mat_<double>& b2, cv::Mat_<double>& theta) {
	theta = cv::Mat_<double>(1, W1.rows * W1.cols + b1.rows * b1.cols + W2.rows * W2.cols + b2.rows * b2.cols);

	int index = 0;
	for (int r = 0; r < W1.rows; ++r) {
		for (int c = 0; c < W1.cols; ++c) {
			theta(0, index++) = W1(r, c);
		}
	}
	for (int r = 0; r < b1.rows; ++r) {
		for (int c = 0; c < b1.cols; ++c) {
			theta(0, index++) = b1(r, c);
		}
	}
	for (int r = 0; r < W2.rows; ++r) {
		for (int c = 0; c < W2.cols; ++c) {
			theta(0, index++) = W2(r, c);
		}
	}
	for (int r = 0; r < b2.rows; ++r) {
		for (int c = 0; c < b2.cols; ++c) {
			theta(0, index++) = b2(r, c);
		}
	}
}

void MLP::decodeParams(const cv::Mat_<double>& theta, cv::Mat_<double>& W1, cv::Mat_<double>& b1, cv::Mat_<double>& W2, cv::Mat_<double>& b2) {
	int index = 0;
	for (int r = 0; r < W1.rows; ++r) {
		for (int c = 0; c < W1.cols; ++c) {
			W1(r, c) = theta(0, index++);
		}
	}
	for (int r = 0; r < b1.rows; ++r) {
		for (int c = 0; c < b1.cols; ++c) {
			b1(r, c) = theta(0, index++);
		}
	}
	for (int r = 0; r < W2.rows; ++r) {
		for (int c = 0; c < W2.cols; ++c) {
			W2(r, c) = theta(0, index++);
		}
	}
	for (int r = 0; r < b2.rows; ++r) {
		for (int c = 0; c < b2.cols; ++c) {
			b2(r, c) = theta(0, index++);
		}
	}
}

double MLP::mat_sqsum(const cv::Mat_<double> mat) {
	return mat_sum(mat.mul(mat));
}

double MLP::mat_sum(const cv::Mat_<double> mat) {
	cv::Mat_<double> tmp;
	cv::reduce(mat, tmp, 0, CV_REDUCE_SUM);
	cv::reduce(tmp, tmp, 1, CV_REDUCE_SUM);
	return tmp(0, 0);
}
