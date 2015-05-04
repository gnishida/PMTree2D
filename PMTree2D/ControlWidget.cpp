#include "ControlWidget.h"
#include <QFileDialog>
#include "MainWindow.h"
#include "GLWidget3D.h"

ControlWidget::ControlWidget(MainWindow* mainWin) : QDockWidget("Control Widget", (QWidget*)mainWin) {
	this->mainWin = mainWin;

	// set up the UI
	ui.setupUi(this);

	ui.comboBoxShape->addItem("conical");
	ui.comboBoxShape->addItem("spherical");
	ui.comboBoxShape->addItem("hemispherical");
	ui.comboBoxShape->addItem("cylindrical");
	ui.comboBoxShape->addItem("tapered cylindrical");
	ui.comboBoxShape->addItem("flame");
	ui.comboBoxShape->addItem("inverse conical");
	ui.comboBoxShape->addItem("tend flame");
	ui.horizontalSliderRadius->setMinimum(1);
	ui.horizontalSliderRadius->setMaximum(100);
	ui.lineEditBaseSplits->setText("0");
	ui.horizontalSliderSplitAngle->setMinimum(0);
	ui.horizontalSliderSplitAngle->setMaximum(80);
	ui.lineEditTaper->setText("0");

	ui.horizontalSliderBase0->setMaximum(80);
	ui.horizontalSliderCurve0->setMinimum(-80);
	ui.horizontalSliderCurve0->setMaximum(80);
	ui.horizontalSliderCurveV0->setMinimum(0);
	ui.horizontalSliderCurveV0->setMaximum(100);

	ui.horizontalSliderBase1->setMaximum(80);
	ui.horizontalSliderCurve1->setMinimum(-110);
	ui.horizontalSliderCurve1->setMaximum(110);
	ui.horizontalSliderCurveV1->setMinimum(0);
	ui.horizontalSliderCurveV1->setMaximum(140);
	ui.horizontalSliderBranches1->setMinimum(5);
	ui.horizontalSliderBranches1->setMaximum(40);
	ui.horizontalSliderDownAngle1->setMinimum(20);
	ui.horizontalSliderDownAngle1->setMaximum(70);
	ui.horizontalSliderRatio1->setMinimum(20);
	ui.horizontalSliderRatio1->setMaximum(70);

	ui.horizontalSliderCurve2->setMinimum(-110);
	ui.horizontalSliderCurve2->setMaximum(110);
	ui.horizontalSliderCurveV2->setMinimum(0);
	ui.horizontalSliderCurveV2->setMaximum(140);
	ui.horizontalSliderBranches2->setMinimum(5);
	ui.horizontalSliderBranches2->setMaximum(40);
	ui.horizontalSliderDownAngle2->setMinimum(10);
	ui.horizontalSliderDownAngle2->setMaximum(50);
	ui.horizontalSliderRatio2->setMinimum(20);
	ui.horizontalSliderRatio2->setMaximum(70);

	update();

	connect(ui.comboBoxShape, SIGNAL(currentIndexChanged(int)), this, SLOT(onValueChanged()));
	connect(ui.horizontalSliderRadius, SIGNAL(sliderMoved(int)), this, SLOT(onValueChanged()));
	connect(ui.lineEditBaseSplits, SIGNAL(textChanged(const QString &)), this, SLOT(onValueChanged()));
	connect(ui.horizontalSliderSplitAngle, SIGNAL(sliderMoved(int)), this, SLOT(onValueChanged()));
	connect(ui.lineEditTaper, SIGNAL(textChanged(const QString &)), this, SLOT(onValueChanged()));

	connect(ui.horizontalSliderBase0, SIGNAL(sliderMoved(int)), this, SLOT(onValueChanged()));
	connect(ui.horizontalSliderCurve0, SIGNAL(sliderMoved(int)), this, SLOT(onValueChanged()));
	connect(ui.horizontalSliderCurveV0, SIGNAL(sliderMoved(int)), this, SLOT(onValueChanged()));

	connect(ui.horizontalSliderBase1, SIGNAL(sliderMoved(int)), this, SLOT(onValueChanged()));
	connect(ui.horizontalSliderCurve1, SIGNAL(sliderMoved(int)), this, SLOT(onValueChanged()));
	connect(ui.horizontalSliderCurveV1, SIGNAL(sliderMoved(int)), this, SLOT(onValueChanged()));
	connect(ui.horizontalSliderBranches1, SIGNAL(sliderMoved(int)), this, SLOT(onValueChanged()));
	connect(ui.horizontalSliderDownAngle1, SIGNAL(sliderMoved(int)), this, SLOT(onValueChanged()));
	connect(ui.horizontalSliderRatio1, SIGNAL(sliderMoved(int)), this, SLOT(onValueChanged()));

	connect(ui.horizontalSliderCurve2, SIGNAL(sliderMoved(int)), this, SLOT(onValueChanged()));
	connect(ui.horizontalSliderCurveV2, SIGNAL(sliderMoved(int)), this, SLOT(onValueChanged()));
	connect(ui.horizontalSliderBranches2, SIGNAL(sliderMoved(int)), this, SLOT(onValueChanged()));
	connect(ui.horizontalSliderDownAngle2, SIGNAL(sliderMoved(int)), this, SLOT(onValueChanged()));
	connect(ui.horizontalSliderRatio2, SIGNAL(sliderMoved(int)), this, SLOT(onValueChanged()));

	hide();	
}

void ControlWidget::update() {
	PMTree2D* tree = mainWin->glWidget->tree;

	ui.comboBoxShape->blockSignals(true);
	ui.lineEditBaseSplits->blockSignals(true);
	ui.lineEditTaper->blockSignals(true);
	
	ui.comboBoxShape->setCurrentIndex(tree->shape);
	ui.horizontalSliderRadius->setValue(tree->radius * 100);
	ui.lineEditBaseSplits->setText(QString::number(tree->baseSplits));
	ui.horizontalSliderSplitAngle->setValue(tree->splitAngle);
	ui.lineEditTaper->setText(QString::number(tree->taper));

	ui.horizontalSliderBase0->setValue(tree->base[0] * 100);
	ui.horizontalSliderCurve0->setValue(tree->curve[0]);
	ui.horizontalSliderCurveV0->setValue(tree->curveV[0]);

	ui.horizontalSliderBase1->setValue(tree->base[1] * 100);
	ui.horizontalSliderCurve1->setValue(tree->curve[1]);
	ui.horizontalSliderCurveV1->setValue(tree->curveV[1]);
	ui.horizontalSliderBranches1->setValue(tree->branches[1]);
	ui.horizontalSliderDownAngle1->setValue(tree->downAngle[1]);
	ui.horizontalSliderRatio1->setValue(tree->ratio[1] * 100);

	ui.horizontalSliderCurve2->setValue(tree->curve[2]);
	ui.horizontalSliderCurveV2->setValue(tree->curveV[2]);
	ui.horizontalSliderBranches2->setValue(tree->branches[2]);
	ui.horizontalSliderDownAngle2->setValue(tree->downAngle[2]);
	ui.horizontalSliderRatio2->setValue(tree->ratio[2] * 100);

	ui.comboBoxShape->blockSignals(false);
	ui.lineEditBaseSplits->blockSignals(false);
	ui.lineEditTaper->blockSignals(false);
}

void ControlWidget::onValueChanged() {
	PMTree2D* tree = mainWin->glWidget->tree;
	
	tree->shape = ui.comboBoxShape->currentIndex();
	tree->radius = ui.horizontalSliderRadius->value() / 100.0f;
	tree->baseSplits = ui.lineEditBaseSplits->text().toInt();
	tree->splitAngle = ui.horizontalSliderSplitAngle->value();
	tree->taper = ui.lineEditTaper->text().toFloat();

	tree->base[0] = ui.horizontalSliderBase0->value() / 100.0f;
	tree->curve[0] = ui.horizontalSliderCurve0->value();
	tree->curveV[0] = ui.horizontalSliderCurveV0->value();

	tree->base[1] = ui.horizontalSliderBase1->value() / 100.0f;
	tree->curve[1] = ui.horizontalSliderCurve1->value();
	tree->curveV[1] = ui.horizontalSliderCurveV1->value();
	tree->branches[1] = ui.horizontalSliderBranches1->value();
	tree->downAngle[1] = ui.horizontalSliderDownAngle1->value();
	tree->ratio[1] = ui.horizontalSliderRatio1->value() / 100.0f;

	tree->curve[2] = ui.horizontalSliderCurve2->value();
	tree->curveV[2] = ui.horizontalSliderCurveV2->value();
	tree->branches[2] = ui.horizontalSliderBranches2->value();
	tree->downAngle[2] = ui.horizontalSliderDownAngle2->value();
	tree->ratio[2] = ui.horizontalSliderRatio2->value() / 100.0f;

	mainWin->glWidget->updateGL();
}