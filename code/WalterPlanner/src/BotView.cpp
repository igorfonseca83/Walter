/*
 * BotViewController.cpp
 *
 * Author: JochenAlt
 */


#include <windows.h>  // openGL windows
#include <thread>

#include <GL/gl.h>
#include <GL/freeglut.h>
#include <GL/glut.h>  // GLUT, includes glu.h and gl.h
#include <GL/Glui.h>

#include "spatial.h"
#include "Util.h"
#include <BotView.h>
#include <WindowController.h>
#include "Trajectory.h"

#include "BotDrawer.h"
#include "uiconfig.h"
#include "TrajectorySimulation.h"

using namespace std;


// compute a value floating from start to target during startup time
// (used for eye position to get a neat animation)
float BotView::startupFactor(float start, float target) {
	if (startupAnimationRatio < 1.0) {
		float myStartupRatio = 0.01;
		if (startupAnimationRatio >= 0.3)
			myStartupRatio = (startupAnimationRatio-0.3)/0.7;
		float distortedFactor = (1.0-(1.0-myStartupRatio)*(1.0-myStartupRatio));
		float startupFactorAngle = distortedFactor*PI/2.0;
		if (start == 0.0)
			return target*sin(startupFactorAngle);

		return target + (start-target)*cos(startupFactorAngle);
	}
	return target;
}

BotView::BotView() {
	windowHandle = 0;
	startupAnimationRatio = 0.0f;
	currEyeDistance = ViewEyeDistance;
	baseAngle = -45;
	heightAngle = 0;
	mainBotView = false;

	BotDrawer::getInstance().setup();
}

void BotView::setStartupAnimationRatio(float ratio) {
	startupAnimationRatio = ratio;
}

void BotView::setLights()
{
	const float lightDistance = 1500.0f;
  GLfloat light_ambient[] =  {0.02, 0.02, 0.02, 0.0};
  GLfloat light_diffuse[] =  {0.4, 0.4, 0.4, 1.0};
  GLfloat light_specular[] = {0.1, 0.1, 0.1, 1.0};
  GLfloat light_position0[] = {2*lightDistance, 2*lightDistance, 3*lightDistance, 0.0};		// ceiling left
  GLfloat light_position1[] = {-2*lightDistance, 2*lightDistance, 3*lightDistance, 0.0};	// ceiling right
  GLfloat light_position2[] = {0, 2*lightDistance, -3*lightDistance, 0.0};					// far away from the back
  GLfloat mat_ambient[] =  {0.1, 0.1, 0.1, 0.0};
  GLfloat mat_diffuse[] =  {0.4, 0.8, 0.4, 1.0};
  GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};

  glMaterialfv(GL_LIGHT0, GL_AMBIENT, mat_ambient);
  glMaterialfv(GL_LIGHT0, GL_DIFFUSE, mat_diffuse);
  glMaterialfv(GL_LIGHT0, GL_SPECULAR, mat_specular);

  glMaterialfv(GL_LIGHT1, GL_AMBIENT, mat_ambient);
  glMaterialfv(GL_LIGHT1, GL_DIFFUSE, mat_diffuse);

  glMaterialfv(GL_LIGHT2, GL_AMBIENT, mat_ambient);
  glMaterialfv(GL_LIGHT2, GL_DIFFUSE, mat_diffuse);

  glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
  glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

  glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
  glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);

  glLightfv(GL_LIGHT2, GL_POSITION, light_position2);
  glLightfv(GL_LIGHT2, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT2, GL_DIFFUSE, light_diffuse);

  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);
  glEnable(GL_LIGHT2);

  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);

  // GLfloat global_ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
  // glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
}


void BotView::printSubWindowTitle(std::string text ) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();                 // Reset the model-view matrix
	gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, glWindowTitleColor);
	glRasterPos2f(-0.9,0.8);
	glutBitmapString(GLUT_BITMAP_HELVETICA_12,(const unsigned char*) text.c_str());
}


void BotView::drawCoordSystem(bool withRaster) {
	// draw coordinate system
	const float axisLength = 400.0f;
	const float arrowLength = 20.0f;
	const float unitLength = 100.0f;
	const float rasterLineLength = axisLength*2;
	if (withRaster) {
		glPushAttrib(GL_LIGHTING_BIT);
		glBegin(GL_LINES);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, glRasterColor3v);
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, glWhiteColor);
			glColor3fv(glRasterColor3v);
			for (float i = -rasterLineLength;i<=rasterLineLength;i = i + unitLength ) {
				glVertex3f(i, 0.0, -rasterLineLength);glVertex3f(i,0.0f, rasterLineLength);
			}
			for (float i = -rasterLineLength;i<=rasterLineLength;i = i + unitLength ) {
				glVertex3f(-rasterLineLength, 0.0f, i);glVertex3f(rasterLineLength, 0.0f, i);
			}
		glEnd();

		// draw colored base area
		glBegin(GL_QUADS);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, glCoordSystemAreaColor3v);
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, glWhiteColor);
			glColor3fv(glCoordSystemAreaColor3v);
            glNormal3f(0.0,1.0,0.0);
			glVertex3f(-rasterLineLength, 0.0, -rasterLineLength);
			glVertex3f(-rasterLineLength, 0.0f, rasterLineLength);
			glVertex3f(rasterLineLength, 0.0f, rasterLineLength);
			glVertex3f(rasterLineLength, 0.0f, -rasterLineLength);
		glEnd();

		glPopAttrib();
	}

	glBegin(GL_LINES);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, glCoordSystemColor3v);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, glCoordSystemColor3v);
		glColor3fv(glCoordSystemColor3v);


		float z = 2.0f;
		// robot's x-axis
		glVertex3f(0.0f, z, -arrowLength);glVertex3f(0.0f, z, axisLength);
		glVertex3f(0.0f, z, axisLength);glVertex3f(0.0f,+arrowLength/2, axisLength-arrowLength);
		glVertex3f(0.0f, z, axisLength);glVertex3f(0.0f,-arrowLength/2, axisLength-arrowLength);
		for (float i = 0;i<axisLength;i = i + unitLength ) {
			glVertex3f(0.0f, -arrowLength/2, i);glVertex3f(0.0f,+arrowLength/2, i);
		}

		// robot's y-axis
		glVertex3f(-arrowLength, z, 0.0f);glVertex3f(axisLength, z, 0.0f);
		glVertex3f(axisLength, z, 0.0f);glVertex3f(axisLength-arrowLength, -arrowLength/2, 0.0f);
		glVertex3f(axisLength, z, 0.0f);glVertex3f(axisLength-arrowLength, arrowLength/2, 0.0f);
		for (float i = 0;i<axisLength;i = i + unitLength ) {
			glVertex3f(i, -arrowLength/2, 0.0f);glVertex3f(i,+arrowLength/2, 0.0f);
		}

		// robot's z-axis
		glVertex3f(0.0f, -arrowLength, 0.0f);glVertex3f(0.0f, axisLength,0.0f);
		glVertex3f(0.0f, axisLength,0.0f);glVertex3f(+arrowLength/2,axisLength-arrowLength, 0.0f);
		glVertex3f(0.0f, axisLength,0.0f);glVertex3f(-arrowLength/2, axisLength-arrowLength,0.0f);
		for (float i = 0;i<axisLength;i = i + unitLength ) {
			glVertex3f(-arrowLength/2, i,0.0f);glVertex3f(+arrowLength/2, i,0.0f);
		}
	glEnd();

	glRasterPos3f(axisLength+arrowLength, 0.0f, 0.0f);
	glutBitmapString(GLUT_BITMAP_HELVETICA_12,(const unsigned char*) "y");
	glRasterPos3f(0.0f, 0.0f, axisLength+arrowLength);
	glutBitmapString(GLUT_BITMAP_HELVETICA_12,(const unsigned char*) "x");
	glRasterPos3f(0.0f, axisLength+arrowLength,0.0f);
	glutBitmapString(GLUT_BITMAP_HELVETICA_12,(const unsigned char*) "z");
}


void BotView::getTCPDot(GLint* &pViewport, GLdouble* &pModelview, GLdouble* &pProjmatrix) {
	pViewport = viewport;
	pModelview = modelview;
	pProjmatrix = projection;
}

void BotView::drawTrajectory() {
	vector<TrajectoryNode>& trajectory = TrajectorySimulation::getInstance().getTrajectory().getSupportNodes();
	for (unsigned int i = 0;i<trajectory.size();i++) {
		TrajectoryNode& node = trajectory[i];

		const GLfloat* color = midPearlColor;
		if (i == 0)
			color = startPearlColor;
		else
			if (i == trajectory.size()-1)
				color = endPearlColor;

		string name = node.name;
		if (name.empty() || (name == "")) {
			// if no name is given, print the time
			float time = ((float)node.time)/1000.0;
			name = string_format("%3.1fs",time);
		}
		if (!mainBotView)
			name = "";
		drawTCPMarker(node.pose,color, name);

		if ((trajectory.size()> 1) && (i < trajectory.size()-1)) {
			// draw bezier curve
			int start_ms = node.time;
			int end_ms = start_ms + node.duration;
			TrajectoryNode curr = node;
			TrajectoryNode prev;
			TrajectoryNode prevprev;

			Trajectory& trajectory = TrajectorySimulation::getInstance().getTrajectory();
			prevprev.pose.angles = trajectory.get(0).pose.angles;
			prev.pose.angles = trajectory.get(0).pose.angles;

			for (int t = start_ms+pearlChainDistance_ms;t<=end_ms;t+=pearlChainDistance_ms) {
				prevprev = prev;
				prev = curr;
				curr = trajectory.getCompiledNodeByTime(t);

				// compute speed and acceleration
				int speedJointNo, accJointNo;
				float speedRatio = Kinematics::maxSpeed(prev.pose.angles, curr.pose.angles, pearlChainDistance_ms, speedJointNo);
				float accRatio = Kinematics::maxAcceleration(prevprev.pose.angles, prev.pose.angles, curr.pose.angles, pearlChainDistance_ms,accJointNo);

				if (mainBotView) {
					glPushMatrix();
						glLoadIdentity();
						glTranslatef(prev.pose.position[1], prev.pose.position[2], prev.pose.position[0]);
						glRotatef(degrees(prev.pose.orientation[2]), 0.0,1.0,0.0);
						glRotatef(degrees(prev.pose.orientation[1]), 1.0,0.0,0.0);
						glRotatef(degrees(prev.pose.orientation[0]), 0.0,0.0,1.0);

						if ((accRatio > 1.0) || (speedRatio > 1.0)) {
							float exceeed = max(accRatio,speedRatio);
							// compute color profile from yellow to red
							float colorValue = (1.0/exceeed);
							float sphereRadius = min(8.0, 2.5 + exceeed);
							const GLfloat exceedColor[]	= { 1.0f, colorValue, 0.0f };
							glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, exceedColor);

							glutSolidSphere(sphereRadius, 18, 18);
							glRasterPos3f(0.0f, -16.0f, 0.0f);
							stringstream errorText;
							errorText.precision(1);

							if (speedRatio > 1.0) {
								float speedValue = radians(Kinematics::getAngularSpeed(prev.pose.angles[speedJointNo], curr.pose.angles[speedJointNo], pearlChainDistance_ms));
								errorText << "v(" << speedJointNo << ")=" << std::fixed << speedValue <<  "(" << int((speedRatio-1.0)*100) << "%)";
							}
							if (accRatio > 1.0) {
								float accValue = radians(Kinematics::getAngularAcceleration(prevprev.pose.angles[accJointNo], prev.pose.angles[accJointNo], curr.pose.angles[accJointNo], pearlChainDistance_ms));
								errorText << "a(" << accJointNo << ")=" << std::fixed << accValue << "(" << int((accRatio-1.0)*100) << "%)";
							}

							glutBitmapString(GLUT_BITMAP_HELVETICA_12,(const unsigned char*)errorText.str().c_str());
						}
						else {
							glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, midPearlColor);
							glutSolidSphere(2.5, 18, 18);

						}
					glPopMatrix();
				}
				glPushAttrib(GL_LIGHTING_BIT);
					glBegin(GL_LINES);
						glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, glCoordSystemColor3v);
						glColor3fv(glCoordSystemColor3v);

						glVertex3f(prev.pose.position[1], prev.pose.position[2], prev.pose.position[0]);
						glVertex3f(curr.pose.position[1], curr.pose.position[2], curr.pose.position[0]);
					glEnd();
				glPopAttrib();
			}
		}
	}
}

void BotView::drawTCPMarker(const Pose& pose, const GLfloat* dotColor, string text) {
	glMatrixMode(GL_MODELVIEW);
	glClearColor(glSubWindowColor3DView[0], glSubWindowColor3DView[1],glSubWindowColor3DView[2],0.0f);

	const float tcpCoordLen = 40;

	// base plate
	glPushMatrix();
		glLoadIdentity();
		glTranslatef(pose.position[1], pose.position[2], pose.position[0]);
		glRotatef(degrees(pose.orientation[2]), 0.0,1.0,0.0);
		glRotatef(degrees(pose.orientation[1]), 1.0,0.0,0.0);
		glRotatef(degrees(pose.orientation[0]), 0.0,0.0,1.0);

		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, dotColor);
		glutSolidSphere(tcpCoordLen/7, 18, 18);

		glPushAttrib(GL_LIGHTING_BIT);
		glBegin(GL_LINES);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, glCoordSystemColor3v);
			glColor4fv(glCoordSystemColor3v);

			glVertex3f(-tcpCoordLen/2, 0.0f, -tcpCoordLen/6);glVertex3f(-tcpCoordLen/2,0.0f, tcpCoordLen);
			glVertex3f(tcpCoordLen/2, 0.0f, -tcpCoordLen/6);glVertex3f(tcpCoordLen/2, 0.0f, tcpCoordLen);

			glVertex3f(-tcpCoordLen/3*2, 0.0f, 0.0f);glVertex3f(tcpCoordLen/3*2, 0.0f, 0.0f);
		glEnd();

		glRasterPos3f(0.0f, 0.0f, 12.0f);
		glutBitmapString(GLUT_BITMAP_HELVETICA_12,(const unsigned char*)text.c_str());
		glPopAttrib();

	glPopMatrix();
}


void BotView::paintBot(const JointAngles& angles, const Pose& pose) {

	glMatrixMode(GL_MODELVIEW);
	glClearColor(glSubWindowColor3DView[0], glSubWindowColor3DView[1],glSubWindowColor3DView[2],0.0f);

	// coord system
	drawCoordSystem(true);

	BotDrawer::getInstance().display(angles, pose, glBotArmColor3DView, glBotaccentColor);

	drawTCPMarker(pose, glTCPColor3v, "");
}

int BotView::create(int mainWindow, string pTitle, View pView, bool pMainBotView) {
	// initially start with zero size, will be resized in reshape
	mainBotView = pMainBotView;
	title = pTitle;
	view = pView;
	windowHandle = glutCreateSubWindow(mainWindow, 1,1,1,1);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);   							// Enable depth testing for z-culling
	glDepthFunc(GL_LEQUAL);    							// Set the type of depth-test
	glShadeModel(GL_SMOOTH);   							// Enable smooth shading
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); 	// Nice perspective corrections

	setLights();
	float pos[] = {ViewEyeDistance*sinf(radians(-45)),ViewBotHeight, ViewEyeDistance*cosf(radians(-45))};
	setEyePosition(pos);

	return windowHandle;
}


void initSTL() {

}

void BotView::display() {
	glutSetWindow(windowHandle);

	glClearColor(glSubWindowColor3DView[0], glSubWindowColor3DView[1], glSubWindowColor3DView[2], 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	printSubWindowTitle(title);
	setWindowPerspective();
	paintBot(angles, pose);
	if (mainBotView)
		drawTrajectory();
}

void BotView::reshape(int x,int y, int w, int h) {
	glutSetWindow(windowHandle);
	glutShowWindow();
	glutPositionWindow(x, y);
	glutReshapeWindow(w, h);
	glViewport(0, 0, w, h);
}


void BotView::setWindowPerspective() {
	glMatrixMode(GL_PROJECTION);  // To operate on the Projection matrix
	glLoadIdentity();             // Reset the model-view matrix

	// Enable perspective projection with fovy, aspect, zNear and zFar
	gluPerspective(45.0f, (GLfloat)glutGet(GLUT_WINDOW_WIDTH) / (GLfloat)glutGet(GLUT_WINDOW_HEIGHT), 0.1f, 5000.0f);
	float startView[] = {-ViewEyeDistance,ViewEyeDistance, 0 };
	gluLookAt(startupFactor(startView[0], eyePosition[0]),startupFactor(startView[1],eyePosition[1]),startupFactor(startView[2], eyePosition[2]),
			0.0, startupFactor(0,ViewBotHeight/2), 0.0,
			0.0, 1.0, 0.0);
}

void BotView::setEyePosition(float* pEyePosition) {
	eyePosition[0] = pEyePosition[0];
	eyePosition[1] = pEyePosition[1];
	eyePosition[2] = pEyePosition[2];
}

void BotView::setEyePosition(float pCurrEyeDistance, float pBaseAngle, float pHeightAngle) {

	currEyeDistance = constrain(pCurrEyeDistance,ViewEyeDistance/10.0f,ViewEyeDistance*3.0f);
	baseAngle = pBaseAngle;
	heightAngle = constrain(pHeightAngle,-90.0f,45.0f);

	eyePosition[0] = currEyeDistance*( sinf(radians(baseAngle)) * cosf(radians(heightAngle)));
	eyePosition[1] = ViewBotHeight - currEyeDistance*sinf(radians(heightAngle));
	eyePosition[2] = currEyeDistance*(cosf(radians(baseAngle)) * cosf(radians(heightAngle)));
}

void BotView::changeEyePosition(float pCurrEyeDistance, float pBaseAngle, float pHeightAngle) {

	currEyeDistance += pCurrEyeDistance;
	baseAngle 		+= pBaseAngle;
	heightAngle 	+= pHeightAngle;

	setEyePosition(currEyeDistance, baseAngle, heightAngle);
}

void BotView::setAngles(const JointAngles& pAngles, const Pose& pPose) {
	angles = pAngles;
	pose = pPose;
}

void BotView::hide() {
	glutSetWindow(windowHandle);
	glutHideWindow();
}

void BotView::show() {
	glutSetWindow(windowHandle);
	glutShowWindow();

}
