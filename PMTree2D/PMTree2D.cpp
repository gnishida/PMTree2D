#include "PMTree2D.h"
#include <QGLWidget>
#include <iostream>
#include <time.h>

#define M_PI	3.141592653589

void PMTree2DStats::clear() {
	totalLength.clear();
	totalVolume.clear();
	maxY = 0.0f;
	minX = 0.0f;
	maxX = 0.0f;
	avg_curvature = 0.0f;
	density_histogram.clear();
	curvature_histogram.clear();
}

PMTree2D::PMTree2D() {
	shape = 7;
	radius = 0.15;
	baseSplits = 0;
	splitAngle = 0;
	taper = 1;
	levels = 2;
	curveRes = 10;

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
	vector<unsigned> seeds(1);
	seeds[0] = 0;
	std::seed_seq seq(seeds.begin(), seeds.end());
	mt.seed(seq);

	float length0 = 10.0;

	// 統計情報をクリア
	stats.clear();
	stats.totalLength.resize(levels + 1, 0);
	stats.totalVolume.resize(levels + 1, 0);
	stats.density = cv::Mat_<int>::zeros(20, 20);
	stats.curvature = cv::Mat_<int>::zeros(20, 20);
	
	// 木を生成する
	glm::mat4 modelMat;
	generateStem(0, modelMat, radius, length0);

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
	
	// 変な木が生成された場合は、falseを返却する
	if (2 * (stats.totalLength[0] + stats.totalLength[1]) > stats.totalLength[2]) {
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

	// curvatureを計算する
	{
		int cnt_curvature = 0;
		for (int r = 0; r < 20; ++r) {
			for (int c = 0; c < 20; ++c) {
				if (stats.density(r, c) > 0) {
					stats.avg_curvature += stats.curvature(r, c);
					cnt_curvature += stats.density(r, c);
					stats.curvature(r, c) /= (float)stats.density(r, c);
				}
			}
		}
		stats.avg_curvature /= cnt_curvature;
	}

	// ヒストグラムを計算する
	{
		stats.density_histogram.resize(8, 0);
		stats.curvature_histogram.resize(5, 0);
		int cnt_curvature_histogram = 0;
		for (int r = 0; r < 20; ++r) {
			for (int c = 0; c < 20; ++c) {
				if (stats.density(r, c) < stats.density_histogram.size()) {
					stats.density_histogram[stats.density(r, c)]++;
				} else {
					stats.density_histogram.back()++;
				}

				if (stats.density(r, c) > 0) {
					int index = stats.curvature(r, c) / 10;
					if (index < stats.curvature_histogram.size()) {
						stats.curvature_histogram[index]++;
					} else {
						stats.curvature_histogram.back()++;
					}
					cnt_curvature_histogram++;
				}
			}
		}

		// normalize
		for (int i = 0; i < stats.density_histogram.size(); ++i) {
			stats.density_histogram[i] /= 400.0f;
		}
		for (int i = 0; i < stats.curvature_histogram.size(); ++i) {
			stats.curvature_histogram[i] /= cnt_curvature_histogram;
		}
	}

	for (int i = 0; i < stats.curvature_histogram.size(); ++i) {
		//cout << stats.curvature_histogram[i] << "," << endl;
	}

	return true;
}

void PMTree2D::randomInit(int seed) {
	vector<unsigned> seeds(1);
	seeds[0] = seed;
	std::seed_seq seq(seeds.begin(), seeds.end());
	mt.seed(seq);

	shape = (int)genRand(0, 8);
	radius = genRand(0.01, 0.99);
	baseSplits = genRand() > 0.9 ? 1 : 0;
	splitAngle = 0;
	if (baseSplits == 1) {
		splitAngle = genRand(10, 80);
	}
	taper = genRand(0, 3);

	base[0] = genRand(0, 0.8);
	curve[0] = genRand(-30, 30);
	curveV[0] = genRand(0, 100);

	base[1] = genRand(0, 0.8);
	curve[1] = genRand(-110, 110);
	curveV[1] = genRand(0, 140);
	branches[1] = genRand(5, 40);
	downAngle[1] = genRand(20, 70);
	ratio[1] = genRand(0.2, 0.7);

	curve[2] = genRand(-110, 110);
	curveV[2] = genRand(0, 140);
	branches[2] = genRand(5, 40);
	downAngle[2] = genRand(10, 50);
	ratio[2] = genRand(0.2, 0.7);
}

/**
 * 指定された行列に格納されたパラメータ値をセットする。
 *
 * @param mat		パラメータ値が格納された行列
 */
void PMTree2D::setParams(const cv::Mat_<float>& mat) {
	cv::Mat_<float> m;
	if (mat.rows == 1) {
		m = mat.t();
	} else {
		m = mat.clone();
	}

	shape = m(0, 0);
	radius = m(1, 0);
	baseSplits = m(2, 0);
	splitAngle = m(3, 0);
	taper = m(4, 0);

	base[0] = m(5, 0);
	curve[0] = m(6, 0);
	curveV[0] = m(7, 0);
	base[1] = m(8, 0);
	curve[1] = m(9, 0);
	curveV[1] = m(10, 0);
	branches[1] = m(11, 0);
	downAngle[1] = m(12, 0);
	ratio[1] = m(13, 0);
	curve[2] = m(14, 0);
	curveV[2] = m(15, 0);
	branches[2] = m(16, 0);
	downAngle[2] = m(17, 0);
	ratio[2] = m(18, 0);

	// Hard constraintsに従って、値を修正する
	shape = (int)glm::clamp(shape, 0, 7);
	radius = glm::clamp(radius, 0.01f, 0.99f);
	baseSplits = (int)glm::clamp(baseSplits, 0, 1);
	if (baseSplits == 0) {
		splitAngle = 0;
	} else {
		splitAngle = glm::clamp(splitAngle, 10, 80);
	}
	taper = glm::clamp(taper, 0.0f, 3.0f);

	base[0] = glm::clamp(base[0], 0.0f, 0.5f);
	curve[0] = glm::clamp(curve[0], -30, 30);
	curveV[0] = glm::clamp(curveV[0], 0, 100);

	base[1] = glm::clamp(base[0], 0.0f, 0.5f);
	curve[1] = glm::clamp(curve[0], -110, 110);
	curveV[1] = glm::clamp(curveV[0], 0, 100);
	branches[1] = glm::clamp(branches[1], 10, 40);
	downAngle[1] = glm::clamp(downAngle[1], 20, 70);
	ratio[1] = glm::clamp(ratio[1], 0.3f, 0.7f);

	curve[2] = glm::clamp(curve[0], -110, 110);
	curveV[2] = glm::clamp(curveV[0], 0, 100);
	branches[2] = glm::clamp(branches[1], 10, 40);
	downAngle[2] = glm::clamp(downAngle[1], 10, 50);
	ratio[2] = glm::clamp(ratio[1], 0.3f, 0.7f);
}

/**
 * パラメータの配列を返却する。
 *
 * @return		パラメータの配列
 */
vector<float> PMTree2D::getParams() {
	vector<float> ret(19);

	ret[0] = shape;
	ret[1] = radius;
	ret[2] = baseSplits;
	ret[3] = splitAngle;
	ret[4] = taper;

	ret[5] = base[0];
	ret[6] = curve[0];
	ret[7] = curveV[0];
	ret[8] = base[1];
	ret[9] = curve[1];
	ret[10] = curveV[1];
	ret[11] = branches[1];
	ret[12] = downAngle[1];
	ret[13] = ratio[1];
	ret[14] = curve[2];
	ret[15] = curveV[2];
	ret[16] = branches[2];
	ret[17] = downAngle[2];
	ret[18] = ratio[2];

	return ret;
}

vector<float> PMTree2D::getStatistics(int type) {
	if (type == 0) {
		vector<float> ret(4);
		ret[0] = stats.maxY;
		ret[1] = stats.maxX - stats.minX;
		ret[2] = 1 - stats.density_histogram[0];
		ret[3] = stats.avg_curvature;

		return ret;
	} else if (type == 1) {
		vector<float> ret(11);
		ret[0] = stats.maxY;
		ret[1] = stats.maxX - stats.minX;
		ret[2] = stats.density_histogram[0];
		ret[3] = stats.density_histogram[1];
		ret[4] = stats.density_histogram[2];
		ret[5] = stats.density_histogram[3];
		ret[6] = stats.density_histogram[4];
		ret[7] = stats.density_histogram[5];
		ret[8] = stats.density_histogram[6];
		ret[9] = stats.density_histogram[7];
		ret[10] = stats.avg_curvature;

		return ret;
	} else if (type == 2) {
		vector<float> ret(15);
		ret[0] = stats.maxY;
		ret[1] = stats.maxX - stats.minX;
		ret[2] = stats.density_histogram[0];
		ret[3] = stats.density_histogram[1];
		ret[4] = stats.density_histogram[2];
		ret[5] = stats.density_histogram[3];
		ret[6] = stats.density_histogram[4];
		ret[7] = stats.density_histogram[5];
		ret[8] = stats.density_histogram[6];
		ret[9] = stats.density_histogram[7];
		ret[10] = stats.curvature_histogram[0];
		ret[11] = stats.curvature_histogram[1];
		ret[12] = stats.curvature_histogram[2];
		ret[13] = stats.curvature_histogram[3];
		ret[14] = stats.curvature_histogram[4];

		return ret;
	} else if (type == 3) {
		// デバッグ中。。。
		vector<float> ret(1);
		ret[0] = stats.maxY;
		return ret;
	} else {
		return vector<float>(0);
	}
}

/**
 * １つの枝(stem)を生成する。
 *
 * @param level			階層レベル
 * @param modelMat		モデル行列
 * @param raidus		根元側の半径
 * @param length		枝の長さ
 */
void PMTree2D::generateStem(int level, glm::mat4 modelMat, float radius, float length) {
	float segment_length = length / curveRes;

	if (level == 0 && baseSplits > 0 && splitAngle != 0.0f) { // base splitありの場合の、枝の生成 (論文の4.2節、nBaseSplitsを参照)
		int splitIndex = base[0] * curveRes; // splitを開始するセグメントindex

		int rot = 0;

		// split前のセグメント
		for (int i = 0; i < splitIndex; ++i) {
			float z1 = (float)i / curveRes;
			float z2 = (float)(i + 1) / curveRes;

			float c = genRandV(curve[level] / curveRes, curveV[level] / curveRes);
			generateSegment(level, i, modelMat, radius, z1, z2, length, segment_length, rot, QColor(0, 160 * i / curveRes, 0), c / segment_length);

			modelMat = glm::translate(modelMat, glm::vec3(0, segment_length, 0));
			modelMat = glm::rotate(modelMat, deg2rad(c), glm::vec3(0, 0, 1));
		}

		vector<glm::mat4> splitModelMat(baseSplits + 1);
		for (int s = 0; s < baseSplits + 1; ++s) {
			splitModelMat[s] = glm::rotate(modelMat, deg2rad(splitAngle), glm::vec3(0, 0, 1));

			modelMat = glm::rotate(modelMat, deg2rad(360 / (baseSplits + 1)), glm::vec3(0, 1, 0));
		}

		// split後のセグメント
		for (int s = 0; s < baseSplits + 1; ++s) {
			for (int i = splitIndex; i < curveRes; ++i) {
				float z1 = (float)i / curveRes;
				float z2 = (float)(i + 1) / curveRes;

				float c = genRandV(curve[level] / curveRes, curveV[level] / curveRes);
				generateSegment(level, i, splitModelMat[s], radius, z1, z2, length, segment_length, rot, QColor(0, 160 * i / curveRes, 0), c / segment_length);

				splitModelMat[s] = glm::translate(splitModelMat[s], glm::vec3(0, segment_length, 0));
				splitModelMat[s] = glm::rotate(splitModelMat[s], deg2rad(-splitAngle / (curveRes - splitIndex - 1)), glm::vec3(0, 0, 1));
				splitModelMat[s] = glm::rotate(splitModelMat[s], deg2rad(c), glm::vec3(0, 0, 1));
			}
		}
	} else { // base splitなしの場合の、枝の生成
		int rot = 0;
		for (int i = 0; i < curveRes; ++i) {
			float z1 = (float)i / curveRes;
			float z2 = (float)(i + 1) / curveRes;

			float c = genRandV(curve[level] / curveRes, curveV[level] / curveRes);
			generateSegment(level, i, modelMat, radius, z1, z2, length, segment_length, rot, QColor(0, 160 * i / curveRes, 0), c / segment_length);

			modelMat = glm::translate(modelMat, glm::vec3(0, segment_length, 0));		
			modelMat = glm::rotate(modelMat, deg2rad(c), glm::vec3(0, 0, 1));
		}
	}
}

/**
 * 枝(stem)の１つのセグメントを生成する。
 *
 * @param level				階層レベル
 * @param index				セグメントのindex
 * @param modelMat			モデル行列
 * @param radius1			根元側の半径
 * @param radius2			先端側の半径
 * @param length			枝の長さ
 * @param segment_length	このセグメントの長さ
 * @param rot				この枝における、回転角度
 * @param color				色
 * @param curvature			この枝の曲率
 */
void PMTree2D::generateSegment(int level, int index, glm::mat4 modelMat, float radius, float z1, float z2, float length, float segment_length, int& rot, const QColor& color, float curvature) {
	int stacks = ceil(segment_length / 0.5);
	for (int i = 0; i < stacks; ++i) {
		float zs1 = z1 + (z2 - z1) * i / stacks;
		float zs2 = z1 + (z2 - z1) * (i + 1) / stacks;
		glm::mat4 modelMat2 = glm::translate(modelMat, glm::vec3(0, segment_length / stacks * i, 0));

		drawQuad(modelMat2, computeRadius(radius, length, zs2) * 2, computeRadius(radius, length, zs1) * 2, segment_length / stacks, color, curvature);
	}

	// 統計情報を更新
	stats.totalLength[level] += segment_length;
	stats.totalVolume[level] += segment_length * (computeRadius(radius, length, z1) + computeRadius(radius, length, z2));

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

	float interval = length * (1 - base[level]) / (branches[level + 1] - 1);
	int substems_eff = (segment_length - stem_start) / interval + 1;

	modelMat = glm::translate(modelMat, glm::vec3(0, stem_start, 0));
	modelMat = glm::rotate(modelMat, deg2rad(rot), glm::vec3(0, 1, 0));
	for (int i = 0; i < substems_eff; ++i) {
		float offset = stem_start + i * interval + segment_length * index;

		// この位置におけるセグメントの半径
		float r = computeRadius(radius, length, offset / length);

		glm::mat4 modelMat2 = glm::rotate(modelMat, deg2rad(downAngle[level + 1]), glm::vec3(0, 0, 1));
		modelMat2 = glm::translate(modelMat2, glm::vec3(0, r, 0));

		float sub_ratio = ratio[level + 1] * shapeRatio(shape, (length - offset) / (length * (1.0 - base[level])));

		generateStem(level + 1, modelMat2, r * sub_ratio, length * sub_ratio);

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
void PMTree2D::drawQuad(const glm::mat4& modelMat, float top, float base, float height, const QColor& color, float curvature) {
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
	{
		stats.maxY = max(stats.maxY, p1.y);
		stats.maxY = max(stats.maxY, p2.y);
		stats.maxY = max(stats.maxY, p3.y);
		stats.maxY = max(stats.maxY, p4.y);
		stats.minX = min(stats.minX, p1.x);
		stats.minX = min(stats.minX, p2.x);
		stats.minX = min(stats.minX, p3.x);
		stats.minX = min(stats.minX, p4.x);
		stats.maxX = max(stats.maxX, p1.x);
		stats.maxX = max(stats.maxX, p2.x);
		stats.maxX = max(stats.maxX, p3.x);
		stats.maxX = max(stats.maxX, p4.x);

		cv::Mat_<int> dDensity = cv::Mat_<int>::zeros(20, 20);
		cv::Mat_<float> dCurvature = cv::Mat_<float>::zeros(20, 20);
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
				dCurvature(v, u) = fabs(curvature);
			}
		}
		stats.density += dDensity;
		stats.curvature += dCurvature;
	}
}

/**
 * Shape ratioを返却する。
 * 論文Cretion and rendering of realistic treesの4.3節に記載されている内容に基づく。
 *
 * @param shape		shape id
 * @param ratio		ratio
 * @return			shape ratio
 */
float PMTree2D::shapeRatio(int shape, float ratio) {
	if (shape == 0) {
		return 0.2f + 0.8f * ratio;
	} else if (shape == 1) {
		return 0.2f + 0.8f * sinf(M_PI * ratio);
	} else if (shape == 2) {
		return 0.2f + 0.8f * sinf(0.5f * M_PI * ratio);
	} else if (shape == 3) {
		return 1.0f;
	} else if (shape == 4) {
		return 0.5f + 0.5f * ratio;
	} else if (shape == 5) {
		if (ratio <= 0.7f) {
			return ratio / 0.7f;
		} else {
			return (1.0f - ratio) / 0.3f;
		}
	} else if (shape == 6) {
		return 1.0f - 0.8f * ratio;
	} else if (shape == 7) {
		if (ratio <= 0.7f) {
			return 0.5f + 0.5f * ratio / 0.7f;
		} else {
			return 0.5f + 0.5f * (1.0f - ratio) / 0.3f;
		}
	} else {
		return 0.0f;
	}
}

/**
 * この枝(stem)における、指定された位置での半径を計算して返却する。
 * 論文の4.4節に基づく。
 *
 * @param radius	この枝の根元側の半径
 * @param length	この枝の長さ
 * @param z			この枝における、指定された位置の高さ（[0, 1]で指定）
 * @return			指定された位置での半径
 */
float PMTree2D::computeRadius(float radius, float length, float z) {
	float unit_taper;
	if (taper < 1) {
		unit_taper = taper;
	} else if (taper < 2) {
		unit_taper = 2 - taper;
	} else if (taper <= 3) {
		unit_taper = 0;
	}

	float taper_z = radius * (1.0 - unit_taper * z);

	if (taper < 1) {
		return max(0.001f, taper_z);
	} else {
		float z2 = (1.0 - z) * length;
		float depth;
		if (taper < 2 || z2 < taper_z) {
			depth = 1;
		} else {
			depth = taper - 2;
		}

		float z3;
		if (taper < 2) {
			z3 = z2;
		} else {
			z3 = fabs(z2 - 2 * taper_z * (int)(z2 / (2.0f * taper_z) + 0.5));
		}

		if (taper < 2 && z3 > taper_z) {
			return max(0.001f, taper_z);
		} else {
			return max(0.001, (1.0 - depth) * taper_z + depth * sqrtf(taper_z * taper_z - (z3 - taper_z) * (z3 - taper_z)));
		}
	}
}

/**
 * Uniform乱数[0, 1)を生成する
 */
float PMTree2D::genRand() {
	std::uniform_real_distribution<> r(0.0, 1.0); 
	return r(mt);
}

float PMTree2D::genRand(float a, float b) {
	std::uniform_real_distribution<> r(a, b); 
	return r(mt);
}

/**
 * meanを中心とし、varianceの幅でuniformに乱数を生成する。
 */
float PMTree2D::genRandV(float mean, float variance) {
	std::uniform_real_distribution<> r(mean - variance, mean + variance);
	return r(mt);
}

float PMTree2D::deg2rad(float deg) {
	return deg * M_PI / 180.0;
}
