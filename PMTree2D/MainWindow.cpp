#include "MainWindow.h"
#include <QDir>
#include <QDate>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <fstream>
#include "DataPartition.h"
#include "GaussianProcess.h"

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags) : QMainWindow(parent, flags) {
	ui.setupUi(this);

	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));
	connect(ui.actionSaveImage, SIGNAL(triggered()), this, SLOT(onSaveImage()));
	connect(ui.actionGenerateRandom, SIGNAL(triggered()), this, SLOT(onGenerateRandom()));
	connect(ui.actionGenerateSamples, SIGNAL(triggered()), this, SLOT(onGenerateSamples()));
	connect(ui.actionGenerateTrainingFiles, SIGNAL(triggered()), this, SLOT(onGenerateTrainingFiles()));
	connect(ui.actionInversePMByLinearRegression, SIGNAL(triggered()), this, SLOT(onInversePMByLinearRegression()));
	connect(ui.actionInversePMByLinearRegression2, SIGNAL(triggered()), this, SLOT(onInversePMByLinearRegression2()));
	connect(ui.actionInversePMByLinearRegression3, SIGNAL(triggered()), this, SLOT(onInversePMByLinearRegression3()));
	connect(ui.actionInversePMByHierarchicalLR, SIGNAL(triggered()), this, SLOT(onInversePMByHierarchicalLR()));
	connect(ui.actionInversePMByGaussianProcess, SIGNAL(triggered()), this, SLOT(onInversePMByGaussianProcess()));
	
	glWidget = new GLWidget3D(this);
	setCentralWidget(glWidget);

	// setup the docking widgets
	controlWidget = new ControlWidget(this);

	controlWidget->show();
	addDockWidget(Qt::LeftDockWidgetArea, controlWidget);
}

void MainWindow::onSaveImage() {
	if (!QDir("screenshots").exists()) QDir().mkdir("screenshots");

	QString fileName = "screenshots/" + QDate::currentDate().toString("yyMMdd") + "_" + QTime::currentTime().toString("HHmmss") + ".png";
	glWidget->grabFrameBuffer().save(fileName);
}

void MainWindow::sample(int type, int N, cv::Mat_<double>& dataX, cv::Mat_<double>& dataY, cv::Mat_<double>& normalized_dataX, cv::Mat_<double>& normalized_dataY, cv::Mat_<double>& muX, cv::Mat_<double>& muY, cv::Mat_<double>& maxX, cv::Mat_<double>& maxY) {
	if (type == 0) {
		dataX = cv::Mat_<double>(N, 14);
		dataY = cv::Mat_<double>(N, 5);
	} else if (type == 1) {
		dataX = cv::Mat_<double>(N, 14);
		dataY = cv::Mat_<double>(N, 12);
	} else {
		dataX = cv::Mat_<double>(N, 14);
		dataY = cv::Mat_<double>(N, 16);
	}

	int seed_count = 0;
	for (int iter = 0; iter < N; ++iter) {
		cout << iter << endl;

		while (true) {
			glWidget->tree->randomInit(seed_count++);
			if (glWidget->tree->generate()) break;
		}

		vector<float> params = glWidget->tree->getParams();
		for (int col = 0; col < dataX.cols; ++col) {
			dataX(iter, col) = params[col];
		}

		vector<float> statistics = glWidget->tree->getStatistics(type);
		for (int col = 0; col < dataY.cols - 1; ++col) {
			dataY(iter, col) = statistics[col];
		}
		dataY(iter, dataY.cols - 1) = 1; // 定数項
	}

	// normalization
	cv::reduce(dataX, muX, 0, CV_REDUCE_AVG);
	cv::reduce(dataY, muY, 0, CV_REDUCE_AVG);
	normalized_dataX = dataX - cv::repeat(muX, N, 1);
	normalized_dataY = dataY - cv::repeat(muY, N, 1);

	// [-1, 1]にする
	cv::reduce(cv::abs(normalized_dataX), maxX, 0, CV_REDUCE_MAX);
	cv::reduce(cv::abs(normalized_dataY), maxY, 0, CV_REDUCE_MAX);
	normalized_dataX /= cv::repeat(maxX, N, 1);
	normalized_dataY /= cv::repeat(maxY, N, 1);

	// 定数項
	for (int r = 0; r < N; ++r) {
		normalized_dataY(r, normalized_dataY.cols - 1) = 1;
	}
}

void MainWindow::onGenerateRandom() {
	while (true) {
		glWidget->tree->randomInit(time(0));
		if (glWidget->tree->generate()) break;
	}

	glWidget->update();
	controlWidget->update();
}

void MainWindow::onGenerateSamples() {
	const int N = 2000;

	if (!QDir("samples").exists()) QDir().mkdir("samples");

	cout << "Generating samples..." << endl;

	cv::Mat_<double> dataX(N, 14);
	cv::Mat_<double> dataY(N, 5);
	cv::Mat_<double> normalized_dataX;
	cv::Mat_<double> normalized_dataY;
	cv::Mat_<double> muX, muY;
	cv::Mat_<double> maxX, maxY;
	sample(0, N, dataX, dataY, normalized_dataX, normalized_dataY, muX, muY, maxX, maxY);

	for (int iter = 0; iter < N; ++iter) {
		if (iter % 100 == 0) {
			glWidget->tree->setParams(dataX.row(iter));
			glWidget->updateGL();
			QString fileName = "samples/" + QString::number(iter / 100) + ".png";
			glWidget->grabFrameBuffer().save(fileName);
		}
	}

	glWidget->update();
	controlWidget->update();
}

void MainWindow::onGenerateTrainingFiles() {
	const int N = 2000;

	if (!QDir("samples").exists()) QDir().mkdir("samples");

	cout << "Generating samples..." << endl;

	ofstream ofs("samples/samples.txt");

	cv::Mat_<double> dataX(N, 14);
	cv::Mat_<double> dataY(N, 5);
	cv::Mat_<double> normalized_dataX;
	cv::Mat_<double> normalized_dataY;
	cv::Mat_<double> muX, muY;
	cv::Mat_<double> maxX, maxY;
	sample(2, N, dataX, dataY, normalized_dataX, normalized_dataY, muX, muY, maxX, maxY);

	for (int iter = 0; iter < N; ++iter) {
		ofs << "[";
		for (int c = 0; c < dataX.cols; ++c) {
			if (c > 0) ofs << ",";
			ofs << dataX(iter, c);
		}
		ofs << "],[";
		for (int c = 0; c < dataY.cols; ++c) {
			if (c > 0) ofs << ",";
			ofs << dataY(iter, c);
		}
		ofs << "]" << endl;
	}
	ofs.close();

	glWidget->update();
	controlWidget->update();
}

/**
 * 1000個のサンプルを生成して、対応するhigh-level indicatorを計算し、
 * inverseマッピングをlinear regressionにより求める。
 * high-level indicatorとして、以下を使用する。
 *    height, width, coverage ratio, average curvature
 */
void MainWindow::onInversePMByLinearRegression() {
	const int N = 2000;

	if (!QDir("samples").exists()) QDir().mkdir("samples");

	cout << "Generating samples..." << endl;

	cv::Mat_<double> dataX(N, 14);
	cv::Mat_<double> dataY(N, 5);
	cv::Mat_<double> normalized_dataX;
	cv::Mat_<double> normalized_dataY;
	cv::Mat_<double> muX, muY;
	cv::Mat_<double> maxX, maxY;
	sample(0, N, dataX, dataY, normalized_dataX, normalized_dataY, muX, muY, maxX, maxY);

	// Linear regressionにより、Wを求める（yW = x より、W = y^+ x)
	cv::Mat_<double> W = normalized_dataY.inv(cv::DECOMP_SVD) * normalized_dataX;

	// reverseで木を生成する
	cv::Mat_<double> error = cv::Mat_<double>::zeros(1, dataX.cols);
	cv::Mat_<double> error2 = cv::Mat_<double>::zeros(1, dataX.cols);
	for (int iter = 0; iter < N; ++iter) {
		cv::Mat normalized_x_hat = normalized_dataY.row(iter) * W;
		cv::Mat x_hat = normalized_x_hat.mul(maxX) + muX;

		error += (normalized_dataX.row(iter) - normalized_x_hat).mul(normalized_dataX.row(iter) - normalized_x_hat);
		error2 += (dataX.row(iter) - x_hat).mul(dataX.row(iter) - x_hat);

		if (iter % 100 == 0) {
			glWidget->tree->setParams(dataX.row(iter));
			glWidget->updateGL();
			QString fileName = "samples/" + QString::number(iter / 100) + ".png";
			glWidget->grabFrameBuffer().save(fileName);

			glWidget->tree->setParams(x_hat);
			glWidget->updateGL();
			fileName = "samples/reversed_" + QString::number(iter / 100) + ".png";
			glWidget->grabFrameBuffer().save(fileName);
		}
	}

	error /= N;
	error2 /= N;
	cv::sqrt(error, error);
	cv::sqrt(error2, error2);

	cout << "Prediction error (normalized):" << endl;
	cout << error << endl;
	cout << "Prediction error:" << endl;
	cout << error2 << endl;
}

/**
 * 1000個のサンプルを生成して、対応するhigh-level indicatorを計算し、
 * inverseマッピングをlinear regressionにより求める。
 * high-level indicatorとして、以下を使用する。
 *    height, width, 密度のhistogram、average curvature
 */
void MainWindow::onInversePMByLinearRegression2() {
	const int N = 2000;

	if (!QDir("samples").exists()) QDir().mkdir("samples");

	cout << "Generating samples..." << endl;

	cv::Mat_<double> dataX(N, 14);
	cv::Mat_<double> dataY(N, 12);
	cv::Mat_<double> normalized_dataX;
	cv::Mat_<double> normalized_dataY;
	cv::Mat_<double> muX, muY;
	cv::Mat_<double> maxX, maxY;
	sample(1, N, dataX, dataY, normalized_dataX, normalized_dataY, muX, muY, maxX, maxY);

	// Linear regressionにより、Wを求める（yW = x より、W = y^+ x)
	cv::Mat_<double> W = normalized_dataY.inv(cv::DECOMP_SVD) * normalized_dataX;
	
	// reverseで木を生成する
	cv::Mat_<double> error = cv::Mat_<double>::zeros(1, dataX.cols);
	cv::Mat_<double> error2 = cv::Mat_<double>::zeros(1, dataX.cols);
	for (int iter = 0; iter < N; ++iter) {
		cv::Mat normalized_x_hat = normalized_dataY.row(iter) * W;
		cv::Mat x_hat = normalized_x_hat.mul(maxX) + muX;

		error += (normalized_dataX.row(iter) - normalized_x_hat).mul(normalized_dataX.row(iter) - normalized_x_hat);
		error2 += (dataX.row(iter) - x_hat).mul(dataX.row(iter) - x_hat);

		if (iter % 100 == 0) {
			glWidget->tree->setParams(dataX.row(iter));
			glWidget->updateGL();
			QString fileName = "samples/" + QString::number(iter / 100) + ".png";
			glWidget->grabFrameBuffer().save(fileName);

			glWidget->tree->setParams(x_hat);
			glWidget->updateGL();
			fileName = "samples/reversed_" + QString::number(iter / 100) + ".png";
			glWidget->grabFrameBuffer().save(fileName);
		}
	}
	error /= N;
	error2 /= N;
	cv::sqrt(error, error);
	cv::sqrt(error2, error2);

	cout << "Prediction error (normalized):" << endl;
	cout << error << endl;
	cout << "Prediction error:" << endl;
	cout << error2 << endl;
}

/**
 * 1000個のサンプルを生成して、対応するhigh-level indicatorを計算し、
 * inverseマッピングをlinear regressionにより求める。
 * high-level indicatorとして、以下を使用する。
 *    height, width, 密度のhistogram、curvatureのhistogram
 */
void MainWindow::onInversePMByLinearRegression3() {
	const int N = 2000;

	if (!QDir("samples").exists()) QDir().mkdir("samples");

	cout << "Generating samples..." << endl;

	cv::Mat_<double> dataX(N, 14);
	cv::Mat_<double> dataY(N, 16);
	cv::Mat_<double> normalized_dataX;
	cv::Mat_<double> normalized_dataY;
	cv::Mat_<double> muX, muY;
	cv::Mat_<double> maxX, maxY;
	sample(2, N, dataX, dataY, normalized_dataX, normalized_dataY, muX, muY, maxX, maxY);

	// Linear regressionにより、Wを求める（yW = x より、W = y^+ x)
	cv::Mat_<double> W = normalized_dataY.inv(cv::DECOMP_SVD) * normalized_dataX;

	// reverseで木を生成する
	cv::Mat_<double> error = cv::Mat_<double>::zeros(1, dataX.cols);
	cv::Mat_<double> error2 = cv::Mat_<double>::zeros(1, dataX.cols);
	for (int iter = 0; iter < N; ++iter) {
		cv::Mat normalized_x_hat = normalized_dataY.row(iter) * W;
		cv::Mat x_hat = normalized_x_hat.mul(maxX) + muX;
		error += (normalized_dataX.row(iter) - normalized_x_hat).mul(normalized_dataX.row(iter) - normalized_x_hat);
		error2 += (dataX.row(iter) - x_hat).mul(dataX.row(iter) - x_hat);

		if (iter % 100 == 0) {
			glWidget->tree->setParams(dataX.row(iter));
			glWidget->updateGL();
			QString fileName = "samples/" + QString::number(iter / 100) + ".png";
			glWidget->grabFrameBuffer().save(fileName);

			glWidget->tree->setParams(x_hat);
			glWidget->updateGL();
			fileName = "samples/reversed_" + QString::number(iter / 100) + ".png";
			glWidget->grabFrameBuffer().save(fileName);
		}
	}

	error /= N;
	error2 /= N;
	cv::sqrt(error, error);
	cv::sqrt(error2, error2);

	cout << "Prediction error (normalized):" << endl;
	cout << error << endl;
	cout << "Prediction error:" << endl;
	cout << error2 << endl;
}

/**
 * データを階層的にクラスタリングし、各クラスタについてLinear regression
 * を使って、high-level indicatorから対応するPMパラメータを計算する。
 *
 * 1) 2000個のサンプルを生成して、high-level indicatorを計算する。
 * 2) データを、k-meansにより階層的にクラスタリングする。
 * 3) 各クラスタについて、Linear regressionによりマッピング行列Wを計算する。
 * 4) マッピング行列Wを使って、high-level indicatorからPMパラメータを推定する。
 * 5) 推定値のエラーを計算する。
 */
void MainWindow::onInversePMByHierarchicalLR() {
	const int N = 2000;

	if (!QDir("samples").exists()) QDir().mkdir("samples");

	cout << "Generating samples..." << endl;

	cv::Mat_<double> dataX(N, 14);
	cv::Mat_<double> dataY(N, 16);
	cv::Mat_<double> normalized_dataX;
	cv::Mat_<double> normalized_dataY;
	cv::Mat_<double> muX, muY;
	cv::Mat_<double> maxX, maxY;
	sample(2, N, dataX, dataY, normalized_dataX, normalized_dataY, muX, muY, maxX, maxY);

	vector<cv::Mat_<float> > clusterX, clusterX2, clusterY2;
	vector<vector<int> > clusterIndices;
	{
		cv::Mat samplesX, samplesX2, samplesY2;
		dataX.convertTo(samplesX, CV_32F);
		normalized_dataX.convertTo(samplesX2, CV_32F);
		normalized_dataY.convertTo(samplesY2, CV_32F);
		vector<int> indices(N);
		for (int i = 0; i < N; ++i) indices[i] = i;
		DataPartition::partition(samplesX2, samplesY2, samplesX, indices, 20, clusterX2, clusterY2, clusterX, clusterIndices);

		for (int i = 0; i < clusterX.size(); ++i) {
			cout << clusterX[i].rows << endl;
		}
	}

	cv::Mat_<double> error = cv::Mat_<double>::zeros(1, normalized_dataX.cols);
	cv::Mat_<double> error2 = cv::Mat_<double>::zeros(1, normalized_dataX.cols);
	for (int clu = 0; clu < clusterX.size(); ++clu) {
		cv::Mat_<double> dataX;
		clusterX[clu].convertTo(dataX, CV_64F);
		cv::Mat_<double> normalized_dataX;
		clusterX2[clu].convertTo(normalized_dataX, CV_64F);
		cv::Mat_<double> normalized_dataY;
		clusterY2[clu].convertTo(normalized_dataY, CV_64F);

		// Linear regressionにより、Wを求める（yW = x より、W = y^+ x)
		cv::Mat_<double> W = normalized_dataY.inv(cv::DECOMP_SVD) * normalized_dataX;

		// reverseで木を生成する
		for (int iter = 0; iter < normalized_dataX.rows; ++iter) {
			cv::Mat x2_hat = normalized_dataY.row(iter) * W;
			cv::Mat x_hat = x2_hat.mul(maxX) + muX;
			error += (normalized_dataX.row(iter) - x2_hat).mul(normalized_dataX.row(iter) - x2_hat);
			error2 += (dataX.row(iter) - x_hat).mul(dataX.row(iter) - x_hat);

			if (clusterIndices[clu][iter] % 100 == 0) {
				glWidget->tree->setParams(dataX.row(iter));
				glWidget->updateGL();
				QString fileName = "samples/" + QString::number(clusterIndices[clu][iter] / 100) + ".png";
				glWidget->grabFrameBuffer().save(fileName);

				glWidget->tree->setParams(x_hat);
				glWidget->updateGL();
				fileName = "samples/reversed_" + QString::number(clusterIndices[clu][iter] / 100) + ".png";
				glWidget->grabFrameBuffer().save(fileName);
			}
		}
	}

	error /= N;
	error2 /= N;
	cv::sqrt(error, error);
	cv::sqrt(error2, error2);

	cout << "Prediction error (normalized):" << endl;
	cout << error << endl;
	cout << "Prediction error:" << endl;
	cout << error2 << endl;
}

/**
 * ガウス過程を使って、high-level indicatorから対応するPMパラメータを推定する。
 *
 * 1) 2000個のサンプルを生成して、high-level indicatorを計算する。
 * 2) ガウス過程を使って、high-level indictorから、PMパラメータを推定する。
 * 3) 推定値のエラーを計算する。
 */
void MainWindow::onInversePMByGaussianProcess() {
	const int N = 2000;

	if (!QDir("samples").exists()) QDir().mkdir("samples");

	cout << "Generating samples..." << endl;

	cv::Mat_<double> dataX(N, 14);
	cv::Mat_<double> dataY(N, 16);
	cv::Mat_<double> normalized_dataX;
	cv::Mat_<double> normalized_dataY;
	cv::Mat_<double> muX, muY;
	cv::Mat_<double> maxX, maxY;
	sample(2, N, dataX, dataY, normalized_dataX, normalized_dataY, muX, muY, maxX, maxY);

	cv::Mat_<double> error = cv::Mat_<double>::zeros(1, normalized_dataX.cols);
	cv::Mat_<double> error2 = cv::Mat_<double>::zeros(1, normalized_dataX.cols);

	GaussianProcess gp(normalized_dataY);
	for (int iter = 0; iter < normalized_dataY.rows; ++iter) {
		cout << iter << endl;

		cv::Mat normalized_x_hat = gp.predict(normalized_dataY.row(iter), normalized_dataY, normalized_dataX);
		cv::Mat x_hat = normalized_x_hat.mul(maxX) + muX;

		error += (normalized_dataX.row(iter) - normalized_x_hat).mul(normalized_dataX.row(iter) - normalized_x_hat);
		error2 += (dataX.row(iter) - x_hat).mul(dataX.row(iter) - x_hat);

		if (iter % 100 == 0) {
			glWidget->tree->setParams(dataX.row(iter));
			glWidget->updateGL();
			QString fileName = "samples/" + QString::number(iter / 100) + ".png";
			glWidget->grabFrameBuffer().save(fileName);
			
			glWidget->tree->setParams(x_hat);
			glWidget->updateGL();
			fileName = "samples/reversed_" + QString::number(iter / 100) + ".png";
			glWidget->grabFrameBuffer().save(fileName);
		}
	}

	error /= N;
	error2 /= N;
	cv::sqrt(error, error);
	cv::sqrt(error2, error2);

	cout << "Prediction error (normalized):" << endl;
	cout << error << endl;
	cout << "Prediction error:" << endl;
	cout << error2 << endl;
}
