#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include "ui_MainWindow.h"
#include "GLWidget3D.h"
#include "ControlWidget.h"

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	Ui::MainWindowClass ui;
	GLWidget3D* glWidget;
	ControlWidget* controlWidget;

public:
	MainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
	
	//void saveImage();
	void sample(int type, int N, cv::Mat_<double>& dataX, cv::Mat_<double>& dataY, cv::Mat_<double>& normalized_dataX, cv::Mat_<double>& normalized_dataY, cv::Mat_<double>& muX, cv::Mat_<double>& muY, cv::Mat_<double>& maxX, cv::Mat_<double>& maxY);
	void split(const cv::Mat_<double>& data, float train_ratio, float test_ratio, cv::Mat_<double>& train_data, cv::Mat_<double>& test_data);

public slots:
	void onSaveImage();
	void onGenerateRandom();
	void onGenerateSamples();
	void onGenerateTrainingFiles();
	void onInversePMByLinearRegression();
	void onInversePMByLinearRegression2();
	void onInversePMByLinearRegression3();
	void onInversePMByHierarchicalLR();
	void onInversePMByGaussianProcess();
};

#endif // MAINWINDOW_H
