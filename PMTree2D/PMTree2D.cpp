#include "PMTree2D.h"
#include <QGLWidget>

#define M_PI	3.141592653589

using namespace glm;

PMTree2D::PMTree2D() {
	levels = 2;

	base.resize(levels + 1);
	curveRes.resize(levels + 1);
	curve.resize(levels + 1);
	branches.resize(levels + 1);
	downAngle.resize(levels + 1);
	ratio.resize(levels + 1);

	base[0] = 0.4;
	curveRes[0] = 3;
	curve[0] = 2;
	//branches[0] = 0;
	//downAngle[0] = 0;
	//ratio[0] = 0;

	base[1] = 0.2;
	curveRes[1] = 5;
	curve[1] = 4;
	branches[1] = 30;
	downAngle[1] = 75;
	ratio[1] = 0.5;

	base[2] = 0.2;
	curveRes[2] = 3;
	curve[2] = 4;
	branches[2] = 25;
	downAngle[2] = 35;
	ratio[2] = 0.4;

	colorStem = QColor(30, 162, 0);
}

void PMTree2D::generate() {
	float radius0 = 0.25;
	float length0 = 10.0;
	
	mat4 modelMat;
	generateStem(0, modelMat, radius0, length0);
}

void PMTree2D::generateStem(int level, glm::mat4 modelMat, float radius, float length) {
	float segment_length = length / curveRes[level];

	int rot = 0;
	for (int i = 0; i < curveRes[level]; ++i) {
		float r1 = radius * (curveRes[level] - i) / curveRes[level];
		float r2 = radius * (curveRes[level] - i - 1) / curveRes[level];
		generateSegment(level, i, modelMat, r1, r2, length, segment_length, rot, QColor(255 * i / curveRes[level], 0, 255));

		modelMat = translate(modelMat, vec3(0, segment_length, 0));
		modelMat = rotate(modelMat, deg2rad(curve[level] / curveRes[level]), vec3(0, 0, 1));
		//modelMat = rotate(modelMat, deg2rad(rot), vec3(0, 1, 0));
	}
}

void PMTree2D::generateSegment(int level, int index, mat4 modelMat, float radius1, float radius2, float length, float segment_length, int& rot, const QColor& color) {
	drawQuad(modelMat, radius2 * 2, radius1 * 2, segment_length, color);

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

	int substems_eff = branches[level + 1] / (float)curveRes[level] * (segment_length - stem_start) / segment_length;
	float interval = (segment_length - stem_start) / substems_eff;

	modelMat = translate(modelMat, vec3(0, stem_start, 0));
	modelMat = rotate(modelMat, deg2rad(rot), vec3(0, 1, 0));
	for (int i = 0; i < substems_eff; ++i) {
		float offset = stem_start + i * interval + segment_length * index;

		mat4 modelMat2 = rotate(modelMat, deg2rad(downAngle[level + 1]), vec3(0, 0, 1));

		float sub_ratio = ratio[level + 1] * (length - offset) / length;

		generateStem(level + 1, modelMat2, radius1 * sub_ratio, length * sub_ratio);

		modelMat = rotate(modelMat, deg2rad(180), vec3(0, 1, 0));
		rot = (rot + 180) % 360;

		modelMat = translate(modelMat, vec3(0, interval, 0));
	}
}

void PMTree2D::drawQuad(const mat4& modelMat, float top, float base, float height, const QColor& color) {
	vec4 p1(-base * 0.5, 0, 0, 1);
	vec4 p2(base * 0.5, 0, 0, 1);
	vec4 p3(top * 0.5, height, 0, 1);
	vec4 p4(-top * 0.5, height, 0, 1);

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
}

float PMTree2D::deg2rad(float deg) {
	return deg * M_PI / 180.0;
}
