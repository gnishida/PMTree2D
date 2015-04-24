#include "PMTree2D.h"
#include <QGLWidget>
#include <iostream>
#include <time.h>

#define M_PI	3.141592653589

PMTree2D::PMTree2D() {
	curveRes = 10;
	levels = 2;

	base.resize(levels + 1);
	curve.resize(levels + 1);
	curveV.resize(levels + 1);
	branches.resize(levels + 1);
	downAngle.resize(levels + 1);
	ratio.resize(levels + 1);

	base[0] = 0.3;
	curve[0] = 0;
	curveV[0] = 20;

	base[1] = 0.2;
	curve[1] = 20;
	curveV[1] = 4;
	branches[1] = 20;
	downAngle[1] = 60;
	ratio[1] = 0.5;

	curve[2] = 10;
	curveV[2] = 4;
	branches[2] = 20;
	downAngle[2] = 35;
	ratio[2] = 0.5;

	colorStem = QColor(30, 162, 0);
}

/**
 * 木の2Dモデルを生成する。
 * ただし、物理的な有り得ない形状の場合は、falseを返却する。
 *
 * @return		true - 物理的にOK / false - 物理的にNG
 */
bool PMTree2D::generate() {
	srand(0);

	float radius0 = 0.15;
	float length0 = 10.0;

	// 統計情報をクリア
	totalLength.clear();
	totalLength.resize(levels + 1, 0);
	totalVolume.clear();
	totalVolume.resize(levels + 1, 0);
	maxY = 0.0f;
	minX = 0.0f;
	maxX = 0.0f;
	density = cv::Mat_<int>::zeros(20, 20);
	
	glm::mat4 modelMat;
	generateStem(0, modelMat, radius0, length0);

	/*
	cout << "Total Length:" << endl;
	for (int i = 0; i < levels + 1; ++i) {
		cout << "[" << i << "]: " << totalLength[i] << endl;
	}
	cout << "Total Volume:" << endl;
	for (int i = 0; i < levels + 1; ++i) {
		cout << "[" << i << "]: " << totalVolume[i] << endl;
	}
	*/
	
	if (2 * (totalLength[0] + totalLength[1]) > totalLength[2]) {
		//cout << "too few leaves!" << endl;
		return false;
	}
	/*
	if (totalVolume[0] < 2 * (totalVolume[1] + totalVolume[2])) {
		cout << "too many branches!" << endl;
		return false;
	}
	if (totalVolume[1] < 2 * totalVolume[2]) {
		cout << "too many leaves!" << endl;
		return false;
	}
	*/

	// ToDo
	// conflictチェック！


	// 密度のヒストグラムを計算する
	histogram.clear();
	histogram.resize(10, 0);
	for (int r = 0; r < 20; ++r) {
		for (int c = 0; c < 20; ++c) {
			if (density(r, c) < 10) {
				histogram[density(r, c)]++;
			} else {
				histogram[9]++;
			}
		}
	}
	// normalize
	for (int i = 0; i < histogram.size(); ++i) {
		histogram[i] /= 400.0f;
	}

	return true;
}

void PMTree2D::randomInit() {
	//srand((unsigned int)time(NULL));

	base[0] = genRand(0, 0.5);
	curve[0] = genRand(-30, 30);
	curveV[0] = genRand(0, 100);

	base[1] = genRand(0, 0.5);
	curve[1] = genRand(-110, 110);
	curveV[1] = genRand(0, 100);
	branches[1] = genRand(10, 40);
	downAngle[1] = genRand(20, 70);
	ratio[1] = genRand(0.3, 0.7);

	curve[2] = genRand(-110, 110);
	curveV[2] = genRand(0, 100);
	branches[2] = genRand(10, 40);
	downAngle[2] = genRand(10, 50);
	ratio[2] = genRand(0.3, 0.7);
}

/**
 * 指定された行列に格納されたパラメータ値をセットする。
 *
 * @param mat		パラメータ値が格納された行列
 */
void PMTree2D::setParam(const cv::Mat_<float>& mat) {
	cv::Mat_<float> m;
	if (mat.rows == 1) {
		m = mat.t();
	} else {
		m = mat.clone();
	}

	base[0] = m(0, 0);
	curve[0] = m(1, 0);
	curveV[0] = m(2, 0);
	base[1] = m(3, 0);
	curve[1] = m(4, 0);
	curveV[1] = m(5, 0);
	branches[1] = m(6, 0);
	downAngle[1] = m(7, 0);
	ratio[1] = m(8, 0);
	curve[2] = m(9, 0);
	curveV[2] = m(10, 0);
	branches[2] = m(11, 0);
	downAngle[2] = m(12, 0);
	ratio[2] = m(13, 0);
}

void PMTree2D::generateStem(int level, glm::mat4 modelMat, float radius, float length) {
	float segment_length = length / curveRes;

	int rot = 0;
	for (int i = 0; i < curveRes; ++i) {
		float r1 = radius * (curveRes - i) / curveRes;
		float r2 = radius * (curveRes - i - 1) / curveRes;
		generateSegment(level, i, modelMat, r1, r2, length, segment_length, rot, QColor(0, 160 * i / curveRes, 0));

		modelMat = glm::translate(modelMat, glm::vec3(0, segment_length, 0));		
		modelMat = glm::rotate(modelMat, deg2rad(genRandV(curve[level] / curveRes, curveV[level] / curveRes)), glm::vec3(0, 0, 1));
		//modelMat = rotate(modelMat, deg2rad(rot), vec3(0, 1, 0));
	}
}

void PMTree2D::generateSegment(int level, int index, glm::mat4 modelMat, float radius1, float radius2, float length, float segment_length, int& rot, const QColor& color) {
	radius1 = max(radius1, 0.001f);
	radius2 = max(radius2, 0.001f);

	drawQuad(modelMat, radius2 * 2, radius1 * 2, segment_length, color);

	// 統計情報を更新
	totalLength[level] += segment_length;
	totalVolume[level] += segment_length * (radius1 * radius1);

	if (level >= levels) return;

	float stem_start = 0.0f;
	{
		if (segment_length * index >= length * base[level]) { // ベースより完全に上
		} else if (segment_length * (index + 1) <= length * base[level]) { // ベースより完全に下
			return;
		} else {
			stem_start = length * base[level] - segment_length * index;
		}
	}

	//int substems_eff = branches[level + 1] / (float)curveRes * (segment_length - stem_start) / segment_length;
	//float interval = (segment_length - stem_start) / substems_eff;
	float interval = length * (1 - base[level]) / (branches[level + 1] - 1);
	int substems_eff = (segment_length - stem_start) / interval + 1;

	modelMat = glm::translate(modelMat, glm::vec3(0, stem_start, 0));
	modelMat = glm::rotate(modelMat, deg2rad(rot), glm::vec3(0, 1, 0));
	for (int i = 0; i < substems_eff; ++i) {
		float offset = stem_start + i * interval + segment_length * index;

		glm::mat4 modelMat2 = glm::rotate(modelMat, deg2rad(downAngle[level + 1]), glm::vec3(0, 0, 1));

		float sub_ratio = ratio[level + 1] * (length - offset) / length;

		generateStem(level + 1, modelMat2, radius1 * sub_ratio, length * sub_ratio);

		modelMat = glm::rotate(modelMat, deg2rad(180), glm::vec3(0, 1, 0));
		rot = (rot + 180) % 360;

		modelMat = glm::translate(modelMat, glm::vec3(0, interval, 0));
	}
}

/**
 * 底辺の中心が原点にある左右対称の台形を描画する。
 *
 * @param modelMat		モデル行列
 * @param top			上辺の長さ
 * @param base			底辺の長さ
 * @param height		高さ
 * @param color			色
 */
void PMTree2D::drawQuad(const glm::mat4& modelMat, float top, float base, float height, const QColor& color) {
	glm::vec4 p1(-base * 0.5, 0, 0, 1);
	glm::vec4 p2(base * 0.5, 0, 0, 1);
	glm::vec4 p3(top * 0.5, height, 0, 1);
	glm::vec4 p4(-top * 0.5, height, 0, 1);

	p1 = modelMat * p1;
	p2 = modelMat * p2;
	p3 = modelMat * p3;
	p4 = modelMat * p4;

	glBegin(GL_QUADS);
	glNormal3f(0, 0, 1);
	glColor3f(color.redF(), color.greenF(), color.blueF());
	glVertex3f(p1.x, p1.y, p1.z);
	glVertex3f(p2.x, p2.y, p2.z);
	glVertex3f(p3.x, p3.y, p3.z);
	glVertex3f(p4.x, p4.y, p4.z);
	glEnd();

	// 統計情報を更新
	maxY = max(maxY, p1.y);
	maxY = max(maxY, p2.y);
	maxY = max(maxY, p3.y);
	maxY = max(maxY, p4.y);
	minX = min(minX, p1.x);
	minX = min(minX, p2.x);
	minX = min(minX, p3.x);
	minX = min(minX, p4.x);
	maxX = max(maxX, p1.x);
	maxX = max(maxX, p2.x);
	maxX = max(maxX, p3.x);
	maxX = max(maxX, p4.x);
	cv::Mat_<int> dDensity = cv::Mat_<int>::zeros(20, 20);
	int stacks = height / 0.25;
	float h = height / (float)stacks;
	for (int i = 0; i < stacks; ++i) {
		float y = i * h;
		glm::vec4 p(0, y, 0, 1);
		p = modelMat * p;
		int u = floor((p.x + 5) / 0.5);
		int v = floor(p.y / 0.5);
		if (u >= 0 && u < 20 && v >= 0 && v < 20) {
			dDensity(v, u) = 1;
		}
	}
	density += dDensity;
}

/**
 * Uniform乱数[0, 1)を生成する
 */
float PMTree2D::genRand() {
	return rand() / (float(RAND_MAX) + 1);
}

float PMTree2D::genRand(float a, float b) {
	return genRand() * (b - a) + a;
}

/**
 * meanを中心とし、varianceの幅でuniformに乱数を生成する。
 */
float PMTree2D::genRandV(float mean, float variance) {
	return genRand() * variance * 2.0 + mean - variance;
}

float PMTree2D::deg2rad(float deg) {
	return deg * M_PI / 180.0;
}
