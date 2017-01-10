// This example shows how to work with the BodyIndex image in order to create
// a green screen effect. Note that this isn't super fast, but is helpful
// in understanding how the different image types & coordinate spaces work
// together. If you need performance, you will probably want to do this with shaders!

#include "ofApp.h"

#define DEPTH_WIDTH 512
#define DEPTH_HEIGHT 424
#define DEPTH_SIZE DEPTH_WIDTH * DEPTH_HEIGHT

#define COLOR_WIDTH 1920
#define COLOR_HEIGHT 1080

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetWindowShape(DEPTH_WIDTH * 2, DEPTH_HEIGHT);

	kinect.open();
	kinect.initDepthSource();
	kinect.initColorSource();
	kinect.initInfraredSource();
	kinect.initBodySource();
	kinect.initBodyIndexSource();

	if (kinect.getSensor()->get_CoordinateMapper(&coordinateMapper) < 0) {
		ofLogError() << "Could not acquire CoordinateMapper!";
	}

	numBodiesTracked = 0;
	bHaveAllStreams = false;

	bodyIndexImg.allocate(DEPTH_WIDTH, DEPTH_HEIGHT, OF_IMAGE_COLOR);
	foregroundImg.allocate(DEPTH_WIDTH, DEPTH_HEIGHT, OF_IMAGE_COLOR);

	colorCoords.resize(DEPTH_WIDTH * DEPTH_HEIGHT);

}

//--------------------------------------------------------------
void ofApp::update() {
	kinect.update();

	// Get pixel data
	auto& bodyIndexPix = kinect.getBodyIndexSource()->getPixels();
	auto& colorPix = kinect.getColorSource()->getPixels();

	// Make sure there's some data here, otherwise the cam probably isn't ready yet
	if (!bodyIndexPix.size() || !colorPix.size()) {
		bHaveAllStreams = false;
		return;
	}
	else {
		bHaveAllStreams = true;
	}


	// Count number of tracked bodies
	numBodiesTracked = 0;
	auto& bodies = kinect.getBodySource()->getBodies();
	for (auto& body : bodies) {
		if (body.tracked) {
			numBodiesTracked++;
		}
	}


	int w = bodyIndexPix.getWidth();
	int h = bodyIndexPix.getHeight();


	positions.clear();
	int minWidth = 5;
	for (int y = 0; y < h; y += 10) {
		float begin = 0;
		ofColor c;
		int tryCount = 0;
		for (int x = 0; x < w; x++) {
			int index = (y * w) + x;
			float val = bodyIndexPix[index];
			/*if (val >= bodies.size()) {
				continue;
			}*/
			ofColor color = ofColor(bodyIndexPix[index]);
			// set variable for first and last X position which is 
			//check current pixels brightness
			if (color.getBrightness() < 50) {
				if (begin == 0) {
					begin = x;
					c = ofColor::fromHsb(val * 255 / bodies.size(), 200, 255);
				}
				tryCount = 0;
			}
			else {
				if (begin != 0) {
					tryCount++;
					if (tryCount > minWidth || x == w - 1) {
						float end = x - minWidth;
						if (end - begin > minWidth) {
							data d;
							d.begin = begin;
							d.end = end;
							d.y = y;
							d.color = c;
							positions.push_back(d);
						}
						begin = 0;
						tryCount = 0;
					}
				}
			}
		}
	}

}

//--------------------------------------------------------------
void ofApp::draw() {
	kinect.getBodyIndexSource()->draw(0, 0);

	stringstream ss;
	ss << "fps : " << ofGetFrameRate() << endl;
	ss << "Tracked bodies: " << numBodiesTracked;
	if (!bHaveAllStreams) ss << endl << "Not all streams detected!";
	ofDrawBitmapStringHighlight(ss.str(), 20, 20);

	for (int n = 0; n < positions.size(); n++) {
		ofPushStyle();
		ofSetColor(positions[n].color);
		ofLine(positions[n].begin, positions[n].y, positions[n].end, positions[n].y);
		ofPopStyle();

		ofPushStyle();
		ofNoFill();
		ofSetColor(0, 255, 50);
		ofDrawCircle(positions[n].begin, positions[n].y, 5);
		ofSetColor(255, 0, 50);
		ofDrawCircle(positions[n].end, positions[n].y, 5);
		ofPopStyle();
	}

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}
