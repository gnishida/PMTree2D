#include "GaussianProcess.h"

/**
 * ガウス過程を初期化する。
 *
 * @param X		データ群 (各行が、各データx_iを表す)
 */
GaussianProcess::GaussianProcess(const cv::Mat_<double>& X) {
	// データ数
	int N = X.rows;

	// hyperparameterを適当にセットする
	theta_0 = 1.0;
	theta_1 = 16.0;
	theta_2 = 0.0;
	theta_3 = 0.0;

	// Covを計算する
	cv::Mat_<double> cov(N, N);
	for (int r = 0; r < N; ++r) {
		for (int c = 0; c < N; ++c) {
			std::cout << ".";
			cov(r, c) = covariance_function(X.row(r), X.row(c));
		}
	}

	invCov = cov.inv(cv::DECOMP_SVD);
}

/**
 * ガウス過程により、指定されたデータxに対応する値を推定する。
 *
 * @param x		データ (行ベクトル)
 * @param X		データ群 (各行が、各データx_iを表す)
 * @param Y		観測データ群 (各行が、各観測データy_iを表す)
 * @return		推定された値（行ベクトル）
 */
cv::Mat_<double> GaussianProcess::predict(const cv::Mat_<double>& x, const cv::Mat_<double>& X, const cv::Mat_<double>& Y) {
	cv::Mat_<double> k(Y.rows, 1);
	for (int r = 0; r < k.rows; ++r) {
		k(r, 0) = covariance_function(X.row(r), x);
	}

	return k.t() * invCov * Y;
}

/**
 * 共分散を定義する関数。
 *
 * @param x1
 * @param x2
 * @return		共分散
 */
double GaussianProcess::covariance_function(const cv::Mat_<double>& x1, const cv::Mat_<double>& x2) {
	double n = cv::norm(x1 - x2);
	return theta_0 * exp(-theta_1 / 2.0 * n * n) + theta_2 + theta_3 * x1.dot(x2);
}
