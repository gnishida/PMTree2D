#include "MainWindow.h"
#include <QDir>
#include <QDate>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <fstream>
#include "DataPartition.h"

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

void MainWindow::onGenerateRandom() {
	while (true) {
		glWidget->tree->randomInit(time(0));
		if (glWidget->tree->generate()) break;
	}

	glWidget->update();
	controlWidget->update();
}

void MainWindow::onGenerateSamples() {
	const int N = 1000;

	if (!QDir("samples").exists()) QDir().mkdir("samples");

	cout << "Generating samples..." << endl;

	ofstream ofs("samples/samples.txt");

	cv::Mat_<double> params(N, 14);
	int seed_count = 0;
	for (int iter = 0; iter < N; ++iter) {
		cout << iter << endl;

		while (true) {
			glWidget->tree->randomInit(seed_count++);
			if (glWidget->tree->generate()) break;
		}

		if (iter % 100 == 0) {
			glWidget->updateGL();
			QString fileName = "samples/" + QString::number(iter) + ".png";
			glWidget->grabFrameBuffer().save(fileName);
		}

		vector<float> params = glWidget->tree->getParams();
		for (int i = 0; i < params.size(); ++i) {
			if (i > 0) {
				ofs << ",";
			}
			ofs << params[i];
		}
		ofs << endl;
	}
	ofs.close();

	glWidget->update();
	controlWidget->update();
}

void MainWindow::onGenerateTrainingFiles() {
	const int N = 2000;

	if (!QDir("samples").exists()) QDir().mkdir("samples");

	cout << "Generating samples..." << endl;

	ofstream ofs("samples/samples.txt");

	cv::Mat_<double> params(N, 14);
	int seed_count = 0;
	for (int iter = 0; iter < N; ++iter) {
		cout << iter << endl;

		while (true) {
			glWidget->tree->randomInit(seed_count++);
			if (glWidget->tree->generate()) break;
		}

		if (iter % 100 == 0) {
			glWidget->updateGL();
			QString fileName = "samples/" + QString::number(iter) + ".png";
			glWidget->grabFrameBuffer().save(fileName);
		}

		vector<float> params = glWidget->tree->getParams();
		ofs << "[";
		for (int i = 0; i < params.size(); ++i) {
			if (i > 0) {
				ofs << ",";
			}
			ofs << params[i];
		}
		ofs << "],[";

		vector<float> statistics = glWidget->tree->getStatistics3();
		for (int i = 0; i < statistics.size(); ++i) {
			if (i > 0) {
				ofs << ",";
			}
			ofs << statistics[i];
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
	int seed_count = 0;
	for (int iter = 0; iter < N; ++iter) {
		cout << iter << endl;

		while (true) {
			glWidget->tree->randomInit(seed_count++);
			if (glWidget->tree->generate()) break;
		}

		if (iter % 100 == 0) {
			glWidget->updateGL();
			QString fileName = "samples/" + QString::number(iter / 100) + ".png";
			glWidget->grabFrameBuffer().save(fileName);
		}

		vector<float> params = glWidget->tree->getParams();
		for (int col = 0; col < dataX.cols; ++col) {
			dataX(iter, col) = params[col];
		}

		vector<float> statistics = glWidget->tree->getStatistics1();
		for (int col = 0; col < dataY.cols - 1; ++col) {
			dataY(iter, col) = statistics[col];
		}
		dataY(iter, dataY.cols - 1) = 1; // 定数項
	}

	glWidget->update();
	controlWidget->update();

	// normalization
	cv::Mat_<double> muX, muY;
	cv::reduce(dataX, muX, 0, CV_REDUCE_AVG);
	cv::reduce(dataY, muY, 0, CV_REDUCE_AVG);
	cv::Mat_<double> dataX2 = dataX - cv::repeat(muX, N, 1);
	cv::Mat_<double> dataY2 = dataY - cv::repeat(muY, N, 1);

	// [-1, 1]にする
	cv::Mat_<double> maxX, maxY;
	cv::reduce(cv::abs(dataX2), maxX, 0, CV_REDUCE_MAX);
	cv::reduce(cv::abs(dataY2), maxY, 0, CV_REDUCE_MAX);
	dataX2 /= cv::repeat(maxX, N, 1);
	dataY2 /= cv::repeat(maxY, N, 1);

	// 定数項
	for (int r = 0; r < N; ++r) {
		dataY2(r, dataY2.cols - 1) = 1;
	}

	// Linear regressionにより、Wを求める（yW = x より、W = y^+ x)
	cv::Mat_<double> W = dataY2.inv(cv::DECOMP_SVD) * dataX2;

	// reverseで木を生成する
	cv::Mat_<double> error = cv::Mat_<double>::zeros(1, dataX.cols);
	cv::Mat_<double> error2 = cv::Mat_<double>::zeros(1, dataX.cols);
	for (int iter = 0; iter < N; ++iter) {
		cv::Mat normalized_x_hat = dataY2.row(iter) * W;
		cv::Mat x_hat = normalized_x_hat.mul(maxX) + muX;

		error += (dataX2.row(iter) - normalized_x_hat).mul(dataX2.row(iter) - normalized_x_hat);
		error2 += (dataX.row(iter) - x_hat).mul(dataX.row(iter) - x_hat);

		if (iter % 100 == 0) {
			glWidget->tree->setParams(x_hat);
			glWidget->updateGL();
			QString fileName = "samples/reversed_" + QString::number(iter / 100) + ".png";
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
	int seed_count = 0;
	for (int iter = 0; iter < N; ++iter) {
		cout << iter << endl;

		while (true) {
			glWidget->tree->randomInit(seed_count++);
			if (glWidget->tree->generate()) break;
		}

		if (iter % 100 == 0) {
			glWidget->updateGL();
			QString fileName = "samples/" + QString::number(iter / 100) + ".png";
			glWidget->grabFrameBuffer().save(fileName);
		}

		vector<float> params = glWidget->tree->getParams();
		for (int col = 0; col < dataX.cols; ++col) {
			dataX(iter, col) = params[col];
		}

		vector<float> statistics = glWidget->tree->getStatistics2();
		for (int col = 0; col < dataY.cols - 1; ++col) {
			dataY(iter, col) = statistics[col];
		}
		dataY(iter, dataY.cols - 1) = 1; // 定数項
	}

	glWidget->update();
	controlWidget->update();

	// normalization
	cv::Mat_<double> muX, muY;
	cv::reduce(dataX, muX, 0, CV_REDUCE_AVG);
	cv::reduce(dataY, muY, 0, CV_REDUCE_AVG);
	cv::Mat_<double> dataX2 = dataX - cv::repeat(muX, N, 1);
	cv::Mat_<double> dataY2 = dataY - cv::repeat(muY, N, 1);

	// [-1, 1]にする
	cv::Mat_<double> maxX, maxY;
	cv::reduce(cv::abs(dataX2), maxX, 0, CV_REDUCE_MAX);
	cv::reduce(cv::abs(dataY2), maxY, 0, CV_REDUCE_MAX);
	dataX2 /= cv::repeat(maxX, N, 1);
	dataY2 /= cv::repeat(maxY, N, 1);

	// 定数項
	for (int r = 0; r < N; ++r) {
		dataY2(r, dataY2.cols - 1) = 1;
	}

	// Linear regressionにより、Wを求める（yW = x より、W = y^+ x)
	cv::Mat_<double> W = dataY2.inv(cv::DECOMP_SVD) * dataX2;
	
	// reverseで木を生成する
	cv::Mat_<double> error = cv::Mat_<double>::zeros(1, dataX.cols);
	cv::Mat_<double> error2 = cv::Mat_<double>::zeros(1, dataX.cols);
	for (int iter = 0; iter < N; ++iter) {
		cv::Mat normalized_x_hat = dataY2.row(iter) * W;
		cv::Mat x_hat = normalized_x_hat.mul(maxX) + muX;

		error += (dataX2.row(iter) - normalized_x_hat).mul(dataX2.row(iter) - normalized_x_hat);
		error2 += (dataX.row(iter) - x_hat).mul(dataX.row(iter) - x_hat);

		if (iter % 100 == 0) {
			glWidget->tree->setParams(x_hat);
			glWidget->updateGL();
			QString fileName = "samples/reversed_" + QString::number(iter / 100) + ".png";
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
	int seed_count = 0;
	for (int iter = 0; iter < N; ++iter) {
		cout << iter << endl;

		while (true) {
			glWidget->tree->randomInit(seed_count++);
			if (glWidget->tree->generate()) break;
		}

		if (iter % 100 == 0) {
			glWidget->updateGL();
			QString fileName = "samples/" + QString::number(iter / 100) + ".png";
			glWidget->grabFrameBuffer().save(fileName);
		}

		vector<float> params = glWidget->tree->getParams();
		for (int col = 0; col < dataX.cols; ++col) {
			dataX(iter, col) = params[col];
		}

		vector<float> statistics = glWidget->tree->getStatistics3();
		for (int col = 0; col < dataY.cols - 1; ++col) {
			dataY(iter, col) = statistics[col];
		}
		dataY(iter, dataY.cols - 1) = 1; // 定数項
	}

	glWidget->update();
	controlWidget->update();

	// normalization
	cv::Mat_<double> muX, muY;
	cv::reduce(dataX, muX, 0, CV_REDUCE_AVG);
	cv::reduce(dataY, muY, 0, CV_REDUCE_AVG);
	cv::Mat_<double> dataX2 = dataX - cv::repeat(muX, N, 1);
	cv::Mat_<double> dataY2 = dataY - cv::repeat(muY, N, 1);

	// [-1, 1]にする
	cv::Mat_<double> maxX, maxY;
	cv::reduce(cv::abs(dataX2), maxX, 0, CV_REDUCE_MAX);
	cv::reduce(cv::abs(dataY2), maxY, 0, CV_REDUCE_MAX);
	dataX2 /= cv::repeat(maxX, N, 1);
	dataY2 /= cv::repeat(maxY, N, 1);

	// 定数項
	for (int r = 0; r < N; ++r) {
		dataY2(r, dataY2.cols - 1) = 1;
	}

	// Linear regressionにより、Wを求める（yW = x より、W = y^+ x)
	cv::Mat_<double> W = dataY2.inv(cv::DECOMP_SVD) * dataX2;

	// reverseで木を生成する
	cv::Mat_<double> error = cv::Mat_<double>::zeros(1, dataX.cols);
	cv::Mat_<double> error2 = cv::Mat_<double>::zeros(1, dataX.cols);
	for (int iter = 0; iter < N; ++iter) {
		cv::Mat normalized_x_hat = dataY2.row(iter) * W;
		cv::Mat x_hat = normalized_x_hat.mul(maxX) + muX;
		error += (dataX2.row(iter) - normalized_x_hat).mul(dataX2.row(iter) - normalized_x_hat);
		error2 += (dataX.row(iter) - x_hat).mul(dataX.row(iter) - x_hat);

		if (iter % 100 == 0) {
			glWidget->tree->setParams(x_hat);
			glWidget->updateGL();
			QString fileName = "samples/reversed_" + QString::number(iter / 100) + ".png";
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

void MainWindow::onInversePMByHierarchicalLR() {
	const int N = 2000;

	if (!QDir("samples").exists()) QDir().mkdir("samples");

	cout << "Generating samples..." << endl;

	cv::Mat_<double> dataX(N, 14);
	cv::Mat_<double> dataY(N, 16);
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

		vector<float> statistics = glWidget->tree->getStatistics3();
		for (int col = 0; col < dataY.cols - 1; ++col) {
			dataY(iter, col) = statistics[col];
		}
		dataY(iter, dataY.cols - 1) = 1; // 定数項
	}

	glWidget->update();
	controlWidget->update();

	// normalization
	cv::Mat_<double> muX, muY;
	cv::reduce(dataX, muX, 0, CV_REDUCE_AVG);
	cv::reduce(dataY, muY, 0, CV_REDUCE_AVG);
	cv::Mat_<double> dataX2 = dataX - cv::repeat(muX, N, 1);
	cv::Mat_<double> dataY2 = dataY - cv::repeat(muY, N, 1);

	// [-1, 1]にする
	cv::Mat_<double> maxX, maxY;
	cv::reduce(cv::abs(dataX2), maxX, 0, CV_REDUCE_MAX);
	cv::reduce(cv::abs(dataY2), maxY, 0, CV_REDUCE_MAX);
	dataX2 /= cv::repeat(maxX, N, 1);
	dataY2 /= cv::repeat(maxY, N, 1);

	// 定数項
	for (int r = 0; r < N; ++r) {
		dataY2(r, dataY2.cols - 1) = 1;
	}

	vector<cv::Mat_<float> > clusterX, clusterX2, clusterY2;
	vector<vector<int> > clusterIndices;
	{
		cv::Mat samplesX, samplesX2, samplesY2;
		dataX.convertTo(samplesX, CV_32F);
		dataX2.convertTo(samplesX2, CV_32F);
		dataY2.convertTo(samplesY2, CV_32F);
		vector<int> indices(N);
		for (int i = 0; i < N; ++i) indices[i] = i;
		DataPartition::partition(samplesX2, samplesY2, samplesX, indices, 20, clusterX2, clusterY2, clusterX, clusterIndices);

		for (int i = 0; i < clusterX.size(); ++i) {
			cout << clusterX[i].rows << endl;
		}
	}

	cv::Mat_<double> error = cv::Mat_<double>::zeros(1, dataX2.cols);
	cv::Mat_<double> error2 = cv::Mat_<double>::zeros(1, dataX2.cols);
	for (int clu = 0; clu < clusterX.size(); ++clu) {
		cv::Mat_<double> dataX;
		clusterX[clu].convertTo(dataX, CV_64F);
		cv::Mat_<double> dataX2;
		clusterX2[clu].convertTo(dataX2, CV_64F);
		cv::Mat_<double> dataY2;
		clusterY2[clu].convertTo(dataY2, CV_64F);

		// Linear regressionにより、Wを求める（yW = x より、W = y^+ x)
		cv::Mat_<double> W = dataY2.inv(cv::DECOMP_SVD) * dataX2;

		// reverseで木を生成する
		for (int iter = 0; iter < dataX2.rows; ++iter) {
			cv::Mat x2_hat = dataY2.row(iter) * W;
			cv::Mat x_hat = x2_hat.mul(maxX) + muX;
			error += (dataX2.row(iter) - x2_hat).mul(dataX2.row(iter) - x2_hat);
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
