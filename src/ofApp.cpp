#include "ofApp.h"


void ofApp::setup() {
    
    fontSize = 15;
    
    font.load("Arial Unicode.ttf", fontSize, true, true);
    
    for(int i =0; i < 4; i++){
        blob b;
        blobs.push_back(b);
    }
    
    loadXml("paths.xml");

    ofSetVerticalSync(true);
    ofSetWindowShape(projectionWidth, projectionHeight);
    
    blobColors[0] = ofColor(255, 0, 0);
    blobColors[1] = ofColor(0, 255, 0);
    blobColors[2] = ofColor(0, 0, 255);
    blobColors[3] = ofColor(255, 255, 0);
    
    // set up kinect
    kinect.setRegistration(true);
    kinect.init();
    kinect.open();
    
    grayImage.allocate(kinect.width, kinect.height);
    grayThreshNear.allocate(kinect.width, kinect.height);
    grayThreshFar.allocate(kinect.width, kinect.height);
    
    kpt.loadCalibration("calibration.xml");
   // kpt.setContourSmoothness(4);
    
    // setup gui
    gui.setup("parameters");
    gui.add(nearThreshold.set("nearThresh", 230, 0, 255));
    gui.add(farThreshold.set("farThresh", 10, 0, 255));
    gui.add(minArea.set("minArea", 1000, 0, 5000));
    gui.add(maxArea.set("maxArea", 70000, 15000, 150000));
    gui.add(threshold.set("threshold", 15, 1, 100));
    gui.add(persistence.set("persistence", 15, 1, 100));
    gui.add(maxDistance.set("maxDistance", 32, 1, 100));
    gui.add(displayText.set("displayText", true));
    gui.add(displayTexture.set("displayTexture", true));
    
    //set up fbo
    fbo.allocate(projectionWidth, projectionHeight, GL_RGB);
    
    fbo.begin();
        ofClear(255,255,255, 0);
    fbo.end();

    noProjector = false;
    ;
}

void ofApp::loadXml(string xmlPath){
    
    xml.loadFile(xmlPath);
    
    xml.pushTag("root");
    
    xml.pushTag("config");
    settingsPath =xml.getValue("settingsPath", "");
    
    xml.pushTag("projectorResolution");
    projectionWidth  =xml.getValue("width", 1280);
    ofLog(projectionWidth);
    projectionHeight =xml.getValue("height", 960);
    ofLog(projectionHeight);
    xml.popTag();

    xml.popTag();
    
    int total = xml.getNumTags("path");
    
    ofLog() << "loading itens! total = " << total;
    // go through each stage
    for (int i = 0; i < total; i++) {
        xml.pushTag("path", i);
        string url = xml.getValue("url", "");
        paths.push_back(url);
        xml.popTag();
    }
    xml.popTag();
    
    loadTextFiles();
}

void ofApp::loadTextFiles(){
    for(int p = 0; p < paths.size(); p++){
        vector < string > linesOfTheFile;
        string s = "";
        ofBuffer text;
        string currentUrl = paths[p];
        ofHttpResponse resp = ofLoadURL(currentUrl);
        text = resp.data;
        for (auto line : text.getLines()){
            linesOfTheFile.push_back(line);
        }
        
        for(int l = 0; l < linesOfTheFile.size(); l++){
            s += linesOfTheFile[l];
            s += " ";
        }
        blob b;
        blobs.push_back(b);
        blobs[p].s = s;
        cout << "text["<< p << "]" << s << endl;
    }
}

void ofApp::update() {
    kinect.update();
    
    if(kinect.isFrameNew()) {
        // process kinect depth image
        grayImage.setFromPixels(kinect.getDepthPixels().getData(), kinect.width, kinect.height);
        grayThreshNear = grayImage;
        grayThreshFar = grayImage;
        grayThreshNear.threshold(nearThreshold, true);
        grayThreshFar.threshold(farThreshold);
        cvAnd(grayThreshNear.getCvImage(), grayThreshFar.getCvImage(), grayImage.getCvImage(), NULL);
        grayImage.flagImageChanged();
        
        // set contour tracker parameters
        contourFinder.setMinArea(minArea);
        contourFinder.setMaxArea(maxArea);
        contourFinder.setThreshold(threshold);
        contourFinder.getTracker().setPersistence(persistence);
        contourFinder.getTracker().setMaximumDistance(maxDistance);
        
        // determine found contours
        contourFinder.findContours(grayImage);
    }
    
    fbo.readToPixels(pixels);
    
    int w = projectionWidth;
    int h = projectionHeight;
    
    fbo.begin();
    // MAIN WINDOW
    ofBackground(0);
    
    RectTracker& tracker = contourFinder.getTracker();
    
    for(int i = 0; i < contourFinder.size(); i++) {
        // get contour, label, center point, and age of contour
        vector<cv::Point> points = contourFinder.getContour(i);
        int label = contourFinder.getLabel(i);
        ofPoint center = toOf(contourFinder.getCenter(i));
        int age = tracker.getAge(label);
        
        // map contour using calibration and draw to main window
        ofBeginShape();
        ofFill();
        ofSetColor(blobColors[label % 4]);
        for (int j=0; j<points.size(); j++) {
            ofVec3f worldPoint = kinect.getWorldCoordinateAt(points[j].x, points[j].y);
            ofVec2f projectedPoint = kpt.getProjectedPoint(worldPoint);
            ofVertex( projectionWidth * projectedPoint.x, projectionHeight * projectedPoint.y );
        }
        ofEndShape();
    }
    fbo.end();
    
    for(int i =0; i < blobs.size(); i++){
        blobs[i].positions.clear();
    }
    
    int minWidth = 5;
    for (int y = 0; y < h; y += fontSize) {
        float begin = 0;
        ofColor c;
        int tryCount = 0;
        for (int x = 0; x < w; x++) {
            int index = (y * w)*3 + x*3;
            ofColor color = ofColor(pixels[index], pixels[index + 1], pixels[index + 2]);
            // set variable for first and last X position which is
            //check current pixels brightness
            int blobIndex;
            
            if (color.getBrightness() > 50) {
                
                if (begin == 0) {
                    begin = x;
                    for(int i = 0; i < 4; i++){
                        if(color == blobColors[i]) blobIndex = i;
                    }
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
                            blobs[blobIndex].positions.push_back(d);
                        }
                        begin = 0;
                        tryCount = 0;
                    }
                }
            }
        }
    }
}

void ofApp::draw() {
    
    // GUI
    ofBackground(0);
    ofSetColor(255);
    
    if(!noProjector) {
    ofPushMatrix();
    kinect.draw(0, 0);
    ofTranslate(640, 0);
    grayImage.draw(0, 0);
    ofTranslate(-640, 480);
    contourFinder.draw();
    ofTranslate(640, 0);
    ofPopMatrix();
    } else {
    ofBackground(0);
    
    if(displayTexture) fbo.draw(0, 0);
    
    if(displayText){
        // char index in string
        // for each line
        
        for(int j =0; j < blobs.size(); j++){
            
            string currentText = blobs[j].s;
            
            float totalTextWidth = font.stringWidth(currentText);
            
            float totalWritingSpace = 0;
            float currentWritingSpace = 0;
            float charWidthAverage = totalTextWidth/currentText.length();
            
            int charNum = 0;
            int charLimit = currentText.length();
            
            
            for (int i = 0; i < blobs[j].positions.size(); i++) {
                
                if(charNum >= currentText.length()){
                    charNum = 0;
                }
                
                currentWritingSpace = blobs[j].positions[i].end - blobs[j].positions[i].begin;
                charLimit = int(currentWritingSpace / charWidthAverage);
                
                string currentString = "";
                
                // char number from the total string
                int counterChar = c%currentText.length();
                ofPushStyle();
                ofSetColor(255);
                // staring x value to be added to position.begin
                int x = 0;
                // while x + position begin is smaller the end positing, get iterating chars
                
                
                for(int j = 0; j < charLimit; j++){
                    string currentChar = ofToString(currentText[charNum% currentText.length()]);
                    currentString += currentChar;
                    charNum++;
                }
                
                font.drawString(currentString, blobs[j].positions[i].begin, blobs[j].positions[i].y);
                ofPopStyle();
            }
        }
    }
    }
    
    gui.draw();
    
}

void ofApp::drawSecondWindow(ofEventArgs &args){
  
    if(!noProjector) {
        ofBackground(0);
        
        if(displayTexture) fbo.draw(0, 0);
        
        if(displayText){
            // char index in string
            // for each line
            
            for(int j =0; j < blobs.size(); j++){
                
                string currentText = blobs[j].s;
                
                float totalTextWidth = font.stringWidth(currentText);
                
                float totalWritingSpace = 0;
                float currentWritingSpace = 0;
                float charWidthAverage = totalTextWidth/currentText.length();
                
                int charNum = 0;
                int charLimit = currentText.length();
                
                //cout << "asuodhaiuhsduiashdiu";
                
                for (int i = 0; i < blobs[j].positions.size(); i++) {
                    
                    if(charNum >= currentText.length()){
                        charNum = 0;
                    }
                    
                    currentWritingSpace = blobs[j].positions[i].end - blobs[j].positions[i].begin;
                    charLimit = int(currentWritingSpace / charWidthAverage);
                    
                    string currentString = "";
                    
                    // char number from the total string
                    int counterChar = c%currentText.length();
                    ofPushStyle();
                    ofSetColor(255);
                    // staring x value to be added to position.begin
                    int x = 0;
                    // while x + position begin is smaller the end positing, get iterating chars
                    
                    
                    for(int j = 0; j < charLimit; j++){
                        string currentChar = ofToString(currentText[charNum% currentText.length()]);
                        currentString += currentChar;
                        charNum++;
                    }
                    font.drawString(currentString, blobs[j].positions[i].begin, blobs[j].positions[i].y);
                    ofPopStyle();
                }
            }
        }
    }
    
}

void ofApp::keyPressed(int key) {
    if(key == 't'){
        loadTextFiles();
    }
    
    if(key == '1' ||key == '2' ||key == '3' ||key == '4'){
        
        int number = key - '0' - 1;
        
        vector < string > linesOfTheFile;
        string s = "";
        ofBuffer text;
        string currentUrl = paths[number];
        ofHttpResponse resp = ofLoadURL(currentUrl);
        text = resp.data;
        for (auto line : text.getLines()){
            linesOfTheFile.push_back(line);
        }
        
        for(int l = 0; l < linesOfTheFile.size(); l++){
            s += linesOfTheFile[l];
            s += " ";
        }
        blobs[number].s = s;
        //cout << "text["<< p << "]" << s << endl;

    }
}
