#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <QColor>
#include <vector>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <random>

using namespace std;

class PMTree2DStats {
public:
	vector<float> totalLength;
	vector<float> totalVolume;
	float maxY;
	float minX, maxX;
	float avg_curvature;
	cv::Mat_<int> density;
	cv::Mat_<float> curvature;
	vector<float> density_histogram;
	vector<float> curvature_histogram;

public:
	void clear();
};

class PMTree2D {
public:
	int shape;
	int levels;
	int curveRes;
	vector<float> base;
	vector<int> curve;
	vector<int> curveV;
	vector<int> branches;
	vector<int> downAngle;
	vector<float> ratio;

	QColor colorStem;

	std::mt19937 mt;
	PMTree2DStats stats;
	
public:
	PMTree2D();

	bool generate();
	void randomInit(int seed);
	void setParams(const cv::Mat_<float>& mat);
	vector<float> getParams();
	vector<float> getStatistics(int type);

private:
	void generateStem(int level, glm::mat4 modelMat, float radius, float length);
	void generateSegment(int level, int index, glm::mat4 modelMat, float radius1, float radius2, float length, float segment_length, int& rot, const QColor& color, float curvature);
	void drawQuad(const glm::mat4& modelMat, float top, float base, float height, const QColor& color, float curvature);

	float shapeRatio(int shape, float ratio);

	float genRand();
	float genRand(float a, float b);
	float genRandV(float a, float b);
	float deg2rad(float deg);
};

