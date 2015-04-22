#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <QColor>
#include <vector>

using namespace std;

class PMTree2D {
public:
	int levels;
	vector<float> base;
	vector<int> curveRes;
	vector<int> curve;
	vector<int> branches;
	vector<float> downAngle;
	vector<float> ratio;

	QColor colorStem;

public:
	PMTree2D();

	void generate();

private:
	void generateStem(int level, glm::mat4 modelMat, float radius, float length);
	void generateSegment(int level, int index, glm::mat4 modelMat, float radius1, float radius2, float length, float segment_length, int& rot, const QColor& color);
	void drawQuad(const glm::mat4& modelMat, float top, float base, float height, const QColor& color);

	float deg2rad(float deg);
};

