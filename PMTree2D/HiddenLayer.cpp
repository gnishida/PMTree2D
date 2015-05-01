#include "HiddenLayer.h"
#include <math.h>

using namespace std;

HiddenLayer::HiddenLayer(cv::Mat_<double> input, int n_in, int n_out) {
	W = cv::Mat_<double>(n_in, n_out);
	cv::randu(W, -sqrt(6.0 / (n_in + n_out)), sqrt(6.0 / (n_in + n_out)));

	b = cv::Mat_<double>::zeros(1, n_out);

	init(input, n_in, n_out, W, b);
}

HiddenLayer::HiddenLayer(cv::Mat_<double> input, int n_in, int n_out, cv::Mat_<double> W, cv::Mat_<double> b) {
	init(input, n_in, n_out, W, b);
}

/**
 * パラメータW, bを１つの行ベクトルにして返却する。
 *
 * @return		パラメータ群（行ベクトル）
 */
cv::Mat_<double> HiddenLayer::params() {
	cv::Mat_<double> ret(1, W.rows * W.cols + b.rows * b.cols);

	int index = 0;
	for (int r = 0; r < W.rows; ++r) {
		for (int c = 0; c < W.cols; ++c) {
			ret(0, index++) = W(r, c);
		}
	}
	for (int r = 0; r < b.rows; ++r) {
		for (int c = 0; c < b.cols; ++c) {
			ret(0, index++) = b(r, c);
		}
	}

	return ret;
}

/**
 * inputデータを更新し、それに基づきoutputデータを計算し、outputを更新すると共に、それを返却する。
 * inputデータは、J次元の入力データがNサンプルある時、NxJの行列で表現される。
 * つまり、各行が各サンプルに相当する。
 * 一方、outputは、K次元の出力データがNサンプル分ある時、NxKの行列で表現される。
 *
 * @param input		inputデータ
 * @return			outputデータ
 */
cv::Mat_<double> HiddenLayer::predict(const cv::Mat_<double>& input) {
	this->input = input;
	this->output = mat_tanh(input * W + cv::repeat(b, input.rows, 1));
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
void HiddenLayer::grad(const cv::Mat_<double>& delta, double lambda, cv::Mat_<double>& dW, cv::Mat_<double>& db) {
	dW = cv::Mat_<double>::zeros(W.size());
	db = cv::Mat_<double>::zeros(b.size());

	// dW, dbを計算する
	int N = delta.rows;
	for (int r = 0; r < dW.rows; ++r) {
		for (int c = 0; c < dW.cols; ++c) {
			for (int i = 0; i < N; ++i) {
				dW(r, c) -= delta(i, c) * (1.0 - output(i, c) * output(i, c)) * input(i, r);
			}
			dW(r, c) += lambda * W(r, c);
		}
	}
	for (int c = 0; c < dW.cols; ++c) {
		for (int i = 0; i < N; ++i) {
			db(0, c) -= delta(i, c) * (1.0 - output(i, c) * output(i, c));
		}
		db(0, c) += lambda * b(0, c);
	}
}

void HiddenLayer::init(cv::Mat_<double> input, int n_in, int n_out, cv::Mat_<double> W, cv::Mat_<double> b) {
	this->input = input;
	this->n_in = n_in;
	this->n_out = n_out;
	this->W = W;
	this->b = b;

	predict(input);
}

cv::Mat_<double> HiddenLayer::mat_tanh(const cv::Mat_<double>& mat) {
	cv::Mat_<double> ret(mat.size());

	for (int r = 0; r < mat.rows; ++r) {
		for (int c = 0; c < mat.cols; ++c) {
			ret(r, c) = tanh(mat(r, c));
		}
	}
	return ret;
}