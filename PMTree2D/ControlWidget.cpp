#include "ControlWidget.h"
#include <QFileDialog>
#include "MainWindow.h"
#include "GLWidget3D.h"

ControlWidget::ControlWidget(MainWindow* mainWin) : QDockWidget("Control Widget", (QWidget*)mainWin) {
	this->mainWin = mainWin;

	// set up the UI
	ui.setupUi(this);

	ui.horizontalSliderCurve0->setMinimum(-180);
	ui.horizontalSliderCurve0->setMaximum(180);

	ui.horizontalSliderCurve1->setMinimum(-180);
	ui.horizontalSliderCurve1->setMaximum(180);
	ui.horizontalSliderBranches1->setMinimum(0);
	ui.horizontalSliderBranches1->setMaximum(50);
	ui.horizontalSliderDownAngle1->setMinimum(0);
	ui.horizontalSliderDownAngle1->setMaximum(90);

	ui.horizontalSliderCurve2->setMinimum(-180);
	ui.horizontalSliderCurve2->setMaximum(180);
	ui.horizontalSliderBranches2->setMinimum(0);
	ui.horizontalSliderBranches2->setMaximum(50);
	ui.horizontalSliderDownAngle2->setMinimum(0);
	ui.horizontalSliderDownAngle2->setMaximum(90);

	update();

	connect(ui.horizontalSliderBase0, SIGNAL(sliderMoved(int)), this, SLOT(onValueChanged()));
	connect(ui.horizontalSliderCurve0, SIGNAL(sliderMoved(int)), this, SLOT(onValueChanged()));

	connect(ui.horizontalSliderBase1, SIGNAL(sliderMoved(int)), this, SLOT(onValueChanged()));
	connect(ui.horizontalSliderCurve1, SIGNAL(sliderMoved(int)), this, SLOT(onValueChanged()));
	connect(ui.horizontalSliderBranches1, SIGNAL(sliderMoved(int)), this, SLOT(onValueChanged()));
	connect(ui.horizontalSliderDownAngle1, SIGNAL(sliderMoved(int)), this, SLOT(onValueChanged()));
	connect(ui.horizontalSliderRatio1, SIGNAL(sliderMoved(int)), this, SLOT(onValueChanged()));

	connect(ui.horizontalSliderCurve2, SIGNAL(sliderMoved(int)), this, SLOT(onValueChanged()));
	connect(ui.horizontalSliderBranches2, SIGNAL(sliderMoved(int)), this, SLOT(onValueChanged()));
	connect(ui.horizontalSliderDownAngle2, SIGNAL(sliderMoved(int)), this, SLOT(onValueChanged()));
	connect(ui.horizontalSliderRatio2, SIGNAL(sliderMoved(int)), this, SLOT(onValueChanged()));

	hide();	
}

void ControlWidget::update() {
	PMTree2D* tree = mainWin->glWidget->tree;

	ui.horizontalSliderBase0->setValue(tree->base[0] * 100);
	ui.horizontalSliderCurve0->setValue(tree->curve[0]);

	ui.horizontalSliderBase1->setValue(tree->base[1] * 100);
	ui.horizontalSliderCurve1->setValue(tree->curve[1]);
	ui.horizontalSliderBranches1->setValue(tree->branches[1]);
	ui.horizontalSliderDownAngle1->setValue(tree->downAngle[1]);
	ui.horizontalSliderRatio1->setValue(tree->ratio[1] * 100);

	ui.horizontalSliderCurve2->setValue(tree->curve[2]);
	ui.horizontalSliderBranches2->setValue(tree->branches[2]);
	ui.horizontalSliderDownAngle2->setValue(tree->downAngle[2]);
	ui.horizontalSliderRatio2->setValue(tree->ratio[2] * 100);
}

void ControlWidget::onValueChanged() {
	PMTree2D* tree = mainWin->glWidget->tree;

	tree->base[0] = ui.horizontalSliderBase0->value() / 100.0f;
	tree->curve[0] = ui.horizontalSliderCurve0->value();

	tree->base[1] = ui.horizontalSliderBase1->value() / 100.0f;
	tree->curve[1] = ui.horizontalSliderCurve1->value();
	tree->branches[1] = ui.horizontalSliderBranches1->value();
	tree->downAngle[1] = ui.horizontalSliderDownAngle1->value();
	tree->ratio[1] = ui.horizontalSliderRatio1->value() / 100.0f;

	tree->curve[2] = ui.horizontalSliderCurve2->value();
	tree->branches[2] = ui.horizontalSliderBranches2->value();
	tree->downAngle[2] = ui.horizontalSliderDownAngle2->value();
	tree->ratio[2] = ui.horizontalSliderRatio2->value() / 100.0f;

	mainWin->glWidget->updateGL();
}