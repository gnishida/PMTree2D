#include "LinearRegression.h"

using namespace std;

LinearRegression::LinearRegression(cv::Mat_<double> input, int n_in, int n_out) {
	this->input = input;
	this->n_in = n_in;
	this->n_out = n_out;

	W = cv::Mat_<double>(n_in, n_out);
	cv::randu(W, -sqrt(6.0 / (n_in + n_out)), sqrt(6.0 / (n_in + n_out)));

	b = cv::Mat_<double>::zeros(1, n_out);

	predict(input);
}

/**
 * inputデータを更新し、それに基づきoutputデータを計算し、outputを更新すると共に、それを返却する。
 * inputデータは、K次元の入力データがNサンプルある時、NxKの行列で表現される。
 * つまり、各行が各サンプルに相当する。
 * 一方、outputは、L次元の出力データがNサンプル分ある時、NxLの行列で表現される。
 *
 * @param input		inputデータ
 * @return			outputデータ
 */
cv::Mat_<double> LinearRegression::predict(const cv::Mat_<double>& input) {
	this->input = input;
	this->output = input * W + cv::repeat(b, input.rows, 1);
	return this->output;
}

/**
 * outputデータでの誤差に基づいて、W、bを更新する。
 * また、入力データでの誤差を計算して返却する。
 *
 * @param delta		output誤差
 * @param lambda	正規化項の係数
 * @param alpha		学習速度
 * @return			input誤差
 */
void LinearRegression::grad(const cv::Mat_<double>& delta, double lambda, cv::Mat_<double>& dW, cv::Mat_<double>& db) {
	dW = cv::Mat_<double>::zeros(W.size());
	db = cv::Mat_<double>::zeros(b.size());

	// dW, dbを計算する
	int N = delta.rows;
	for (int r = 0; r < dW.rows; ++r) {
		for (int c = 0; c < dW.cols; ++c) {
			for (int i = 0; i < N; ++i) {
				dW(r, c) -= delta(i, c) * input(i, r);
			}
			dW(r, c) += lambda * W(r, c);
		}
	}
	for (int c = 0; c < dW.cols; ++c) {
		for (int i = 0; i < N; ++i) {
			db(0, c) -= delta(i, c);
		}
		db(0, c) += lambda * b(0, c);
	}
}
