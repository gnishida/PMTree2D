#include "ControlWidget.h"
#include <QFileDialog>
#include "MainWindow.h"
#include "GLWidget3D.h"

ControlWidget::ControlWidget(MainWindow* mainWin) : QDockWidget("Control Widget", (QWidget*)mainWin) {
	this->mainWin = mainWin;

	// set up the UI
	ui.setupUi(this);

	ui.horizontalSliderCurveRes0->setMinimum(1);
	ui.horizontalSliderCurveRes0->setMaximum(10);
	ui.horizontalSliderCurve0->setMinimum(-180);
	ui.horizontalSliderCurve0->setMaximum(180);

	ui.horizontalSliderCurveRes1->setMinimum(1);
	ui.horizontalSliderCurveRes1->setMaximum(10);
	ui.horizontalSliderCurve1->setMinimum(-180);
	ui.horizontalSliderCurve1->setMaximum(180);
	ui.horizontalSliderBranches1->setMinimum(0);
	ui.horizontalSliderBranches1->setMaximum(50);
	ui.horizontalSliderDownAngle1->setMinimum(0);
	ui.horizontalSliderDownAngle1->setMaximum(90);

	ui.horizontalSliderCurveRes2->setMinimum(1);
	ui.horizontalSliderCurveRes2->setMaximum(10);
	ui.horizontalSliderCurve2->setMinimum(-180);
	ui.horizontalSliderCurve2->setMaximum(180);
	ui.horizontalSliderBranches2->setMinimum(0);
	ui.horizontalSliderBranches2->setMaximum(50);
	ui.horizontalSliderDownAngle2->setMinimum(0);
	ui.horizontalSliderDownAngle2->setMaximum(90);

	init();

	connect(ui.horizontalSliderBase0, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged()));
	connect(ui.horizontalSliderCurveRes0, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged()));
	connect(ui.horizontalSliderCurve0, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged()));

	connect(ui.horizontalSliderBase1, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged()));
	connect(ui.horizontalSliderCurveRes1, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged()));
	connect(ui.horizontalSliderCurve1, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged()));
	connect(ui.horizontalSliderBranches1, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged()));
	connect(ui.horizontalSliderDownAngle1, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged()));
	connect(ui.horizontalSliderRatio1, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged()));

	connect(ui.horizontalSliderBase2, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged()));
	connect(ui.horizontalSliderCurveRes2, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged()));
	connect(ui.horizontalSliderCurve2, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged()));
	connect(ui.horizontalSliderBranches2, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged()));
	connect(ui.horizontalSliderDownAngle2, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged()));
	connect(ui.horizontalSliderRatio2, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged()));

	hide();	
}

void ControlWidget::init() {
	PMTree2D* tree = mainWin->glWidget->tree;

	ui.horizontalSliderBase0->setValue(tree->base[0] * 100);
	ui.horizontalSliderCurveRes0->setValue(tree->curveRes[0]);
	ui.horizontalSliderCurve0->setValue(tree->curve[0]);

	ui.horizontalSliderBase1->setValue(tree->base[1] * 100);
	ui.horizontalSliderCurveRes1->setValue(tree->curveRes[1]);
	ui.horizontalSliderCurve1->setValue(tree->curve[1]);
	ui.horizontalSliderBranches1->setValue(tree->branches[1]);
	ui.horizontalSliderDownAngle1->setValue(tree->downAngle[1]);
	ui.horizontalSliderRatio1->setValue(tree->ratio[1] * 100);

	ui.horizontalSliderBase2->setValue(tree->base[2] * 100);
	ui.horizontalSliderCurveRes2->setValue(tree->curveRes[2]);
	ui.horizontalSliderCurve2->setValue(tree->curve[2]);
	ui.horizontalSliderBranches2->setValue(tree->branches[2]);
	ui.horizontalSliderDownAngle2->setValue(tree->downAngle[2]);
	ui.horizontalSliderRatio2->setValue(tree->ratio[2] * 100);
}

void ControlWidget::onValueChanged() {
	PMTree2D* tree = mainWin->glWidget->tree;

	tree->base[0] = ui.horizontalSliderBase0->value() / 100.0f;
	tree->curveRes[0] = ui.horizontalSliderCurveRes0->value();
	tree->curve[0] = ui.horizontalSliderCurve0->value();

	tree->base[1] = ui.horizontalSliderBase1->value() / 100.0f;
	tree->curveRes[1] = ui.horizontalSliderCurveRes1->value();
	tree->curve[1] = ui.horizontalSliderCurve1->value();
	tree->branches[1] = ui.horizontalSliderBranches1->value();
	tree->downAngle[1] = ui.horizontalSliderDownAngle1->value();
	tree->ratio[1] = ui.horizontalSliderRatio1->value() / 100.0f;

	tree->base[2] = ui.horizontalSliderBase2->value() / 100.0f;
	tree->curveRes[2] = ui.horizontalSliderCurveRes2->value();
	tree->curve[2] = ui.horizontalSliderCurve2->value();
	tree->branches[2] = ui.horizontalSliderBranches2->value();
	tree->downAngle[2] = ui.horizontalSliderDownAngle2->value();
	tree->ratio[2] = ui.horizontalSliderRatio2->value() / 100.0f;

	mainWin->glWidget->updateGL();
}