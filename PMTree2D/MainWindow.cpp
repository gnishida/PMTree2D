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
			glWidget->tree->maxY,
			glWidget->tree->maxX - glWidget->tree->minX,
			glWidget->tree->histogram[0],
			glWidget->tree->histogram[1],
			glWidget->tree->histogram[2],
			glWidget->tree->histogram[3],
			glWidget->tree->histogram[4],
			glWidget->tree->histogram[5],
			glWidget->tree->histogram[6],
			glWidget->tree->histogram[7],
			glWidget->tree->histogram[8],
			glWidget->tree->histogram[9]);
	}
		
	fclose(fp);

	glWidget->update();
	controlWidget->update();
}

/**
 * 1000個のサンプルを生成して、対応するhigh-level indicatorを計算し、
 * inverseマッピングをlinear regressionにより求める。
 */
void MainWindow::onInversePMByLinearRegression() {
	const int N = 1000;

	if (!QDir("samples").exists()) QDir().mkdir("samples");

	cout << "Generating samples..." << endl;

	cv::Mat_<float> dataX(N, 14);
	cv::Mat_<float> dataY(N, 4);
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

		dataY(iter, 0) = glWidget->tree->maxY;
		dataY(iter, 1) = glWidget->tree->maxX - glWidget->tree->minX;
		dataY(iter, 2) = 1 - glWidget->tree->histogram[0];
		dataY(iter, 3) = 1; // 定数項
	}

	glWidget->update();
	controlWidget->update();

	// normalizationもどき
	cv::Mat_<float> muX, muY;
	cv::reduce(dataX, muX, 0, CV_REDUCE_AVG);
	cv::reduce(dataY, muY, 0, CV_REDUCE_AVG);
	cv::Mat_<float> dataX2 = dataX - cv::repeat(muX, N, 1);
	cv::Mat_<float> dataY2 = dataY - cv::repeat(muY, N, 1);
	for (int r = 0; r < N; ++r) {
		dataY2(r, 3) = 1;
	}

	// Linear regressionにより、Wを求める（yW = x より、W = y^+ x)
	cv::Mat_<float> W = dataY2.inv(cv::DECOMP_SVD) * dataX2;

	// reverseで木を生成する
	for (int iter = 0; iter < 10; ++iter) {
		int index = iter * 100 + 99;

		cout << iter << ": " << dataY2.row(index) << endl;
		cv::Mat x_hat = dataY2.row(index) * W + muX;
		cout << "x_hat: " << x_hat << endl;

		glWidget->tree->setParam(x_hat);
		glWidget->updateGL();
		QString fileName = "samples/reversed_" + QString::number(index) + ".png";
		glWidget->grabFrameBuffer().save(fileName);
	}
}
