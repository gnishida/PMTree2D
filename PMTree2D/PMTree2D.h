#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <QColor>
#include <vector>
#include <opencv/cv.h>
#include <opencv/highgui.h>

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
	int curveRes;
	int levels;
	vector<float> base;
	vector<int> curve;
	vector<int> curveV;
	vector<int> branches;
	vector<int> downAngle;
	vector<float> ratio;

	QColor colorStem;

	PMTree2DStats stats;
	
public:
	PMTree2D();

	bool generate();
	void randomInit();
	void setParam(const cv::Mat_<float>& mat);

private:
	void generateStem(int level, glm::mat4 modelMat, float radius, float length);
	void generateSegment(int level, int index, glm::mat4 modelMat, float radius1, float radius2, float length, float segment_length, int& rot, const QColor& color, float curvature);
	void drawQuad(const glm::mat4& modelMat, float top, float base, float height, const QColor& color, float curvature);

	float genRand();
	float genRand(float a, float b);
	float genRandV(float a, float b);
	float deg2rad(float deg);
};

