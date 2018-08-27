#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ofxKinect.h"
#include "ofxKinectProjectorToolkit.h"
#include "ofxGui.h"
#include "ofTrueTypeFont.h"
#include "ofxXmlSettings.h"
#include "ofxTrueTypeFontUC.h"

using namespace ofxCv;
using namespace cv;


class ofApp : public ofBaseApp
{
public:
    void setup();
    void update();
    void draw();
    void drawSecondWindow(ofEventArgs& args);
    void keyPressed(int key);
    
    void loadXml(string xmlPath);
    void loadTextFiles();
    
    struct data {
        float begin;
        float end;
        float y;
        int index;
    };
    
    struct blob {
        ofColor color;
        vector<data> positions;
        string s; 
    };
    
    string currentString;
    
    string settingsPath;
    int projectionWidth;
    int projectionHeight;
    
    ofxXmlSettings xml;
    
    ofxTrueTypeFontUC font;
    int fontSize;
    
    string s = "好";
    
    vector <string> paths;
    
    ofxPanel                    gui;
    ofColor                     blobColors[4];
    
    ofPixels pixels;
    ofFbo fbo;
    
    vector<blob> blobs;
  //  vector<data> positions;
    
    bool noProjector;
    
    int c;
    
    ofxCv::ContourFinder        contourFinder;
    ofxKinectProjectorToolkit   kpt;
    ofxKinect                   kinect;
    
    ofxCvGrayscaleImage         bgImage;
    ofxCvGrayscaleImage         grayImage;
    ofxCvGrayscaleImage         grayThreshNear;
    ofxCvGrayscaleImage         grayThreshFar;
    
    ofParameter<float>          nearThreshold;
    ofParameter<float>          farThreshold;
    ofParameter<float>          minArea;
    ofParameter<float>          maxArea;
    ofParameter<float>          threshold;
    ofParameter<float>          persistence;
    ofParameter<float>          maxDistance;
    ofParameter<bool>          displayText;
    ofParameter<bool>          displayTexture;
    
};
