#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags) : QMainWindow(parent, flags) {
	ui.setupUi(this);

	glWidget = new GLWidget3D(this);
	setCentralWidget(glWidget);

	// setup the docking widgets
	controlWidget = new ControlWidget(this);

	controlWidget->show();
	addDockWidget(Qt::LeftDockWidgetArea, controlWidget);
}

