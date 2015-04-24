#include "MainWindow.h"
#include <QDir>
#include <QDate>
#include <opencv/cv.h>
#include <opencv/highgui.h>

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags) : QMainWindow(parent, flags) {
	ui.setupUi(this);

	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));
	connect(ui.actionSaveImage, SIGNAL(triggered()), this, SLOT(onSaveImage()));
	connect(ui.actionGenerateRandom, SIGNAL(triggered()), this, SLOT(onGenerateRandom()));
	connect(ui.actionGenerateSamples, SIGNAL(triggered()), this, SLOT(onGenerateSamples()));
	connect(ui.actionInversePMByLinearRegression, SIGNAL(triggered()), this, SLOT(onInversePMByLinearRegression()));
	connect(ui.actionInversePMByLinearRegression2, SIGNAL(triggered()), this, SLOT(onInversePMByLinearRegression2()));
	connect(ui.actionInversePMByLinearRegression3, SIGNAL(triggered()), this, SLOT(onInversePMByLinearRegression3()));

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
		glWidget->tree->randomInit();
		if (glWidget->tree->generate()) break;
	}

	glWidget->update();
	controlWidget->update();
}

void MainWindow::onGenerateSamples() {
	const int N = 1000;

	if (!QDir("samples").exists()) QDir().mkdir("samples");

	FILE* fp = fopen("samples/samples.txt", "w");

	cout << "Generating samples..." << endl;

	for (int iter = 0; iter < N; ++iter) {
		cout << iter << endl;

		while (true) {
			glWidget->tree->randomInit();
			if (glWidget->tree->generate()) break;
		}

		if ((iter + 1) % 100 == 0) {
			glWidget->updateGL();
			QString fileName = "samples/" + QString::number(iter) + ".png";
			glWidget->grabFrameBuffer().save(fileName);
		}

		fprintf(fp, "[%lf,%d,%d,%lf,%d,%d,%d,%d,%lf,%d,%d,%d,%d,%lf],[%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf]\n", 
			glWidget->tree->base[0],
			glWidget->tree->curve[0],
			glWidget->tree->curveV[0],
			glWidget->tree->base[1],
			glWidget->tree->curve[1],
			glWidget->tree->curveV[1],
			glWidget->tree->branches[1],
			glWidget->tree->downAngle[1],
			glWidget->tree->ratio[1],
			glWidget->tree->curve[2],
			glWidget->tree->curveV[2],
			glWidget->tree->branches[2],
			glWidget->tree->downAngle[2],
			glWidget->tree->ratio[2],
			glWidget->tree->stats.maxY,
			glWidget->tree->stats.maxX - glWidget->tree->stats.minX,
			glWidget->tree->stats.density_histogram[0],
			glWidget->tree->stats.density_histogram[1],
			glWidget->tree->stats.density_histogram[2],
			glWidget->tree->stats.density_histogram[3],
			glWidget->tree->stats.density_histogram[4],
			glWidget->tree->stats.density_histogram[5],
			glWidget->tree->stats.density_histogram[6],
			glWidget->tree->stats.density_histogram[7],
			glWidget->tree->stats.density_histogram[8],
			glWidget->tree->stats.density_histogram[9]);
	}
		
	fclose(fp);

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
	for (int iter = 0; iter < N; ++iter) {
		cout << iter << endl;

		while (true) {
			glWidget->tree->randomInit();
			if (glWidget->tree->generate()) break;
		}

		if ((iter + 1) % 100 == 0) {
			glWidget->updateGL();
			QString fileName = "samples/" + QString::number(iter / 100) + ".png";
			glWidget->grabFrameBuffer().save(fileName);
		}

		dataX(iter, 0) = glWidget->tree->base[0];
		dataX(iter, 1) = glWidget->tree->curve[0];
		dataX(iter, 2) = glWidget->tree->curveV[0];
		dataX(iter, 3) = glWidget->tree->base[1];
		dataX(iter, 4) = glWidget->tree->curve[1];
		dataX(iter, 5) = glWidget->tree->curveV[1];
		dataX(iter, 6) = glWidget->tree->branches[1];
		dataX(iter, 7) = glWidget->tree->downAngle[1];
		dataX(iter, 8) = glWidget->tree->ratio[1];
		dataX(iter, 9) = glWidget->tree->curve[2];
		dataX(iter, 10) = glWidget->tree->curveV[2];
		dataX(iter, 11) = glWidget->tree->branches[2];
		dataX(iter, 12) = glWidget->tree->downAngle[2];
		dataX(iter, 13) = glWidget->tree->ratio[2];

		dataY(iter, 0) = glWidget->tree->stats.maxY;
		dataY(iter, 1) = glWidget->tree->stats.maxX - glWidget->tree->stats.minX;
		dataY(iter, 2) = 1 - glWidget->tree->stats.density_histogram[0];
		dataY(iter, 3) = glWidget->tree->stats.avg_curvature;
		dataY(iter, 4) = 1; // 定数項
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

		if ((iter + 1) % 100 == 0) {
			glWidget->tree->setParam(x_hat);
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
	for (int iter = 0; iter < N; ++iter) {
		cout << iter << endl;

		while (true) {
			glWidget->tree->randomInit();
			if (glWidget->tree->generate()) break;
		}

		if ((iter + 1) % 100 == 0) {
			glWidget->updateGL();
			QString fileName = "samples/" + QString::number(iter / 100) + ".png";
			glWidget->grabFrameBuffer().save(fileName);
		}

		dataX(iter, 0) = glWidget->tree->base[0];
		dataX(iter, 1) = glWidget->tree->curve[0];
		dataX(iter, 2) = glWidget->tree->curveV[0];
		dataX(iter, 3) = glWidget->tree->base[1];
		dataX(iter, 4) = glWidget->tree->curve[1];
		dataX(iter, 5) = glWidget->tree->curveV[1];
		dataX(iter, 6) = glWidget->tree->branches[1];
		dataX(iter, 7) = glWidget->tree->downAngle[1];
		dataX(iter, 8) = glWidget->tree->ratio[1];
		dataX(iter, 9) = glWidget->tree->curve[2];
		dataX(iter, 10) = glWidget->tree->curveV[2];
		dataX(iter, 11) = glWidget->tree->branches[2];
		dataX(iter, 12) = glWidget->tree->downAngle[2];
		dataX(iter, 13) = glWidget->tree->ratio[2];

		dataY(iter, 0) = glWidget->tree->stats.maxY;
		dataY(iter, 1) = glWidget->tree->stats.maxX - glWidget->tree->stats.minX;
		dataY(iter, 2) = glWidget->tree->stats.density_histogram[0];
		dataY(iter, 3) = glWidget->tree->stats.density_histogram[1];
		dataY(iter, 4) = glWidget->tree->stats.density_histogram[2];
		dataY(iter, 5) = glWidget->tree->stats.density_histogram[3];
		dataY(iter, 6) = glWidget->tree->stats.density_histogram[4];
		dataY(iter, 7) = glWidget->tree->stats.density_histogram[5];
		dataY(iter, 8) = glWidget->tree->stats.density_histogram[6];
		dataY(iter, 9) = glWidget->tree->stats.density_histogram[7];
		dataY(iter, 10) = glWidget->tree->stats.avg_curvature;
		dataY(iter, 11) = 1; // 定数項
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

		if ((iter + 1) % 100 == 0) {
			glWidget->tree->setParam(x_hat);
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
	for (int iter = 0; iter < N; ++iter) {
		cout << iter << endl;

		while (true) {
			glWidget->tree->randomInit();
			if (glWidget->tree->generate()) break;
		}

		if ((iter + 1) % 100 == 0) {
			glWidget->updateGL();
			QString fileName = "samples/" + QString::number(iter / 100) + ".png";
			glWidget->grabFrameBuffer().save(fileName);
		}

		dataX(iter, 0) = glWidget->tree->base[0];
		dataX(iter, 1) = glWidget->tree->curve[0];
		dataX(iter, 2) = glWidget->tree->curveV[0];
		dataX(iter, 3) = glWidget->tree->base[1];
		dataX(iter, 4) = glWidget->tree->curve[1];
		dataX(iter, 5) = glWidget->tree->curveV[1];
		dataX(iter, 6) = glWidget->tree->branches[1];
		dataX(iter, 7) = glWidget->tree->downAngle[1];
		dataX(iter, 8) = glWidget->tree->ratio[1];
		dataX(iter, 9) = glWidget->tree->curve[2];
		dataX(iter, 10) = glWidget->tree->curveV[2];
		dataX(iter, 11) = glWidget->tree->branches[2];
		dataX(iter, 12) = glWidget->tree->downAngle[2];
		dataX(iter, 13) = glWidget->tree->ratio[2];

		float lambda = 1.0f;

		dataY(iter, 0) = glWidget->tree->stats.maxY;
		dataY(iter, 1) = glWidget->tree->stats.maxX - glWidget->tree->stats.minX;
		dataY(iter, 2) = glWidget->tree->stats.density_histogram[0];
		dataY(iter, 3) = glWidget->tree->stats.density_histogram[1];
		dataY(iter, 4) = glWidget->tree->stats.density_histogram[2];
		dataY(iter, 5) = glWidget->tree->stats.density_histogram[3];
		dataY(iter, 6) = glWidget->tree->stats.density_histogram[4];
		dataY(iter, 7) = glWidget->tree->stats.density_histogram[5];
		dataY(iter, 8) = glWidget->tree->stats.density_histogram[6];
		dataY(iter, 9) = glWidget->tree->stats.density_histogram[7];
		dataY(iter, 10) = glWidget->tree->stats.curvature_histogram[0] * lambda;
		dataY(iter, 11) = glWidget->tree->stats.curvature_histogram[1] * lambda;
		dataY(iter, 12) = glWidget->tree->stats.curvature_histogram[2] * lambda;
		dataY(iter, 13) = glWidget->tree->stats.curvature_histogram[3] * lambda;
		dataY(iter, 14) = glWidget->tree->stats.curvature_histogram[4] * lambda;
		dataY(iter, 15) = 1; // 定数項
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

		if ((iter + 1) % 100 == 0) {
			glWidget->tree->setParam(x_hat);
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
