#include "MainWindow.h"
#include <QDir>
#include <QDate>

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags) : QMainWindow(parent, flags) {
	ui.setupUi(this);

	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));
	connect(ui.actionSaveImage, SIGNAL(triggered()), this, SLOT(onSaveImage()));
	connect(ui.actionGenerateRandom, SIGNAL(triggered()), this, SLOT(onGenerateRandom()));
	connect(ui.actionGenerateSamples, SIGNAL(triggered()), this, SLOT(onGenerateSamples()));

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

		fprintf(fp, "[%d,%d,%d,%lf,%d,%d,%d,%d,%lf,%d,%d,%d,%d,%lf],[%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf]\n", 
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
