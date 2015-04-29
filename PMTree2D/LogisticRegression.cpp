#include "LogisticRegression.h"

using namespace std;

/**
 * Logistic regressionの初期化。
 *
 * @param X			入力データ (各行が各入力x_iに相当する）
 * @param Y			出力データ（各行が各出力y_iに相当する）
 * @param lambda	正規化項の係数
 */
LogisticRegression::LogisticRegression(const cv::Mat_<double>& X, const cv::Mat_<double>& Y, float lambda, float alpha, int maxIter) {
	N = X.rows;
	W = cv::Mat_<double>::zeros(X.cols, Y.cols);
	b = cv::Mat_<double>::zeros(1, Y.cols);

	cv::randu(W, -0.1, 0.1);

	train(X, Y, lambda, alpha, maxIter);
}

/**
 * 与えられた入力に対する出力を推定する。
 *
 * @param x		入力データ
 * @return		推定された出力データ
 */
cv::Mat_<double> LogisticRegression::predict(const cv::Mat_<double>& X) {
	cv::Mat_<double> tmp;
	cv::exp(-X * W - cv::repeat(b, X.rows, 1), tmp);
	return 1 / (1 + tmp);
}

void LogisticRegression::train(const cv::Mat_<double>& X, const cv::Mat_<double>& Y, float lambda, float alpha, int maxIter) {
	cv::Mat_<double> dW, db;

	// デバッグ（微分の実装が正しいか、チェック）
	cv::Mat_<double> theta1, theta2;
	grad(X, Y, lambda, dW, db);
	encodeParam(dW, db, theta1);
	numericalGrad(X, Y, lambda, dW, db);
	encodeParam(dW, db, theta2);
	for (int i = 0; i < theta1.cols; ++i) {
		//cout << theta1(0, i) << ", " << theta2(0, i) << ", " << theta1(0, i) - theta2(0, i) << endl;
		if (abs(theta1(0, i) - theta2(0, i)) > 0.0000001) {
			cout << "There seems to be something wrong with derivatives." << endl;
			return;
		}
	}

	for (int iter = 0; iter < maxIter; ++iter) {
		double c = cost(X, Y, lambda);
		//cout << "Cost: " << c << endl;

		grad(X, Y, lambda, dW, db);

		W -= dW * alpha;
		b -= db * alpha;
	}
}

double LogisticRegression::cost(const cv::Mat_<double>& X, const cv::Mat_<double>& Y, float lambda) {
	cv::Mat_<double> y_hat = predict(X);

	cv::Mat_<double> log_y_hat, log_one_minus_y_hat;
	cv::log(y_hat, log_y_hat);
	cv::log(1 - y_hat, log_one_minus_y_hat);

	cv::Mat_<double> entropy;
	cv::reduce(Y.mul(log_y_hat) + (1 - Y).mul(log_one_minus_y_hat), entropy, 0, CV_REDUCE_AVG);

	cv::reduce(entropy, entropy, 1, CV_REDUCE_SUM);

	double n = cv::norm(W);
	double cost = -entropy(0, 0) + lambda * n * n;

	return cost;
}

void LogisticRegression::grad(const cv::Mat_<double>& X, const cv::Mat_<double>& Y, float lambda, cv::Mat_<double>& dW, cv::Mat_<double>& db) {
	dW = cv::Mat_<double>::zeros(W.size());
	db = cv::Mat_<double>::zeros(b.size());

	cv::Mat_<double> Y_hat = predict(X);

	for (int r = 0; r < dW.rows; ++r) {
		for (int c = 0; c < dW.cols; ++c) {
			for (int i = 0; i < N; ++i) {
				dW(r, c) -= (Y(i, c) - Y_hat(i, c)) * X(i, r);
			}
			dW(r, c) = dW(r, c) / N + 2 * lambda * W(r, c);
		}
	}
	for (int c = 0; c < db.cols; ++c) {
		for (int i = 0; i < N; ++i) {
			db(0, c) -= Y(i, c) - Y_hat(i, c);
		}
		db(0, c) = db(0, c) / N;
	}
}

void LogisticRegression::numericalGrad(const cv::Mat_<double>& X, const cv::Mat_<double>& Y, float lambda, cv::Mat_<double>& dW, cv::Mat_<double>& db) {
	cv::Mat_<double> backup_W, backup_b;
	W.copyTo(backup_W);
	b.copyTo(backup_b);

	cv::Mat_<double> theta;
	encodeParam(W, b, theta);
	cv::Mat_<double> g(theta.size());
	double epsilon = 0.0001;
	for (int i = 0; i < theta.cols; ++i) {
		cv::Mat_<double> d_theta = cv::Mat_<double>::zeros(theta.size());
		d_theta(0, i) = epsilon;

		decodeParam(theta + d_theta, W, b);
		double c1 = cost(X, Y, lambda);
		decodeParam(theta - d_theta, W, b);
		double c2 = cost(X, Y, lambda);

		g(0, i) = (c1 - c2) / 2.0 / epsilon;
	}
	decodeParam(g, dW, db);

	backup_W.copyTo(W);
	backup_b.copyTo(b);
}

void LogisticRegression::encodeParam(const cv::Mat_<double>& W, const cv::Mat_<double>& b, cv::Mat_<double>& theta) {
	theta = cv::Mat_<double>(1, W.rows * W.cols + b.rows * b.cols);
	int index = 0;
	for (int c = 0; c < W.cols; ++c) {
		for (int r = 0; r < W.rows; ++r) {
			theta(0, index++) = W(r, c);
		}
	}
	for (int c = 0; c < b.cols; ++c) {
		for (int r = 0; r < b.rows; ++r) {
			theta(0, index++) = b(r, c);
		}
	}
}

void LogisticRegression::decodeParam(const cv::Mat_<double>& theta, cv::Mat_<double>& W, cv::Mat_<double>& b) {
	int index = 0;
	for (int c = 0; c < W.cols; ++c) {
		for (int r = 0; r < W.rows; ++r) {
			W(r, c) = theta(0, index++);
		}
	}
	for (int c = 0; c < b.cols; ++c) {
		for (int r = 0; r < b.rows; ++r) {
			b(r, c) = theta(0, index++);
		}
	}
}