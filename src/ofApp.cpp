#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    audioInput.printDeviceList();

    smoothedVol = 0.0;
    samplesPerBuffer = 128;
    useMicrophone.set(true);
    int inputDevice = (useMicrophone.get()) ? 0 : 5;
    audioInput.setDeviceID(inputDevice);
    audioInput.setup(this, 0, 2, 44100, samplesPerBuffer, 4);
    left.assign(samplesPerBuffer, 0.0);
    right.assign(samplesPerBuffer, 0.0);

    ofSetVerticalSync(true);
    ofSetFrameRate(60);
    ofBackground(66,66,66);

    //initialize the video grabber
    vidGrabber.setVerbose(true);
    vidGrabber.setup(320,240);

    resetMesh();

    //this is an annoying thing that is used to flip the camera
    cam.setScale(1,-1,1);


    //this determines how much we push the meshes out
    extrusionAmount = 70.0;

    setupGui();
}

void ofApp::setupGui(){
    panel.setup("settings");
    panel.setDefaultBackgroundColor(ofColor(0, 0, 0, 127));
    panel.setDefaultFillColor(ofColor(160, 160, 160, 160));

    panel.add(doFullScreen.set("fullscreen (F)", false));
    doFullScreen.addListener(this, &ofApp::setFullScreen);
    panel.add(toggleGuiDraw.set("show GUI (G)", true));
    panel.add(useMicrophone.set("use microphone (M)", useMicrophone.get()));
    useMicrophone.addListener(this, &ofApp::setAudioSource);
    panel.add(reset.setup("reset (R)"));
    reset.addListener(this, &ofApp::resetMesh);
}

//--------------------------------------------------------------
void ofApp::update(){
    float value = smoothedVol;

    //grab a new frame
    vidGrabber.update();

    //update the mesh if we have a new frame
    if (vidGrabber.isFrameNew()){
        for (int i=0; i<vidGrabber.getWidth()*vidGrabber.getHeight(); i++){

            ofFloatColor sampleColor(vidGrabber.getPixels()[i*3]/255.f,                // r
                                     vidGrabber.getPixels()[i*3+1]/255.f,            // g
                                     vidGrabber.getPixels()[i*3+2]/255.f);            // b

            //now we get the vertex at this position
            ofVec3f tmpVec = mainMesh.getVertex(i);

            //melt a little if the sound is loud enough
            float threshold = 0.01;
            float meltFactor = (useMicrophone.get()) ? 10 : 25;
            if (value > threshold) {
                int yInitial = tmpVec.y;
                tmpVec.y += sampleColor.getBrightness() * value * meltFactor;
                tmpVec.y = (int)tmpVec.y % (int)vidGrabber.getHeight(); // make the bottom pixels jump to the top
                tmpVec.z += (tmpVec.y - yInitial) / 5;
//                tmpVec.y += (sampleColor.getBrightness()) / 8;
            }
            mainMesh.setVertex(i, tmpVec);

            mainMesh.setColor(i, sampleColor);
        }
    }

    //vertically center the camera
    float rotateAmount = 0;

    //move the camera around the mesh
    ofVec3f camDirection(0,0,1);
    ofVec3f centre(vidGrabber.getWidth()/2.f,vidGrabber.getHeight()/2.f, 255/2.f);
    ofVec3f camDirectionRotated = camDirection.getRotated(rotateAmount, ofVec3f(1,0,0));
    ofVec3f camPosition = centre + camDirectionRotated * extrusionAmount;

    cam.setPosition(camPosition);
    cam.lookAt(centre);
}

//--------------------------------------------------------------
void ofApp::draw(){
    //we have to disable depth testing to draw the video frame
    ofDisableDepthTest();
    //    vidGrabber.draw(20,20);

    //but we want to enable it to show the mesh
    ofEnableDepthTest();
    cam.begin();

    //You can either draw the mesh or the wireframe
    // mainMesh.drawWireframe();
    mainMesh.drawFaces();
    cam.end();

    //tell the user how loud input is
    ofSetColor(255);
    string msg = "amplitude: " + ofToString(smoothedVol, 4);
    ofDrawBitmapString(msg, ofGetWidth() - msg.length() * 10, 20);

    if (toggleGuiDraw.get()) {
        ofDisableDepthTest();
        panel.draw();
    }
}

//--------------------------------------------------------------
void ofApp::audioIn(float * input, int bufferSize, int nChannels){

    // see audioInputExample
    float curVol = 0.0;

    // samples are "interleaved"
    int numCounted = 0;

    //lets go through each sample and calculate the root mean square which is a rough way to calculate volume
    for (int i = 0; i < bufferSize; i++){
        left[i]        = input[i*2]*0.5;
        right[i]    = input[i*2+1]*0.5;

        curVol += left[i] * left[i];
        curVol += right[i] * right[i];
        numCounted+=2;
    }

    //this is how we get the mean of rms :)
    curVol /= (float)numCounted;

    // this is how we get the root of rms :)
    curVol = sqrt( curVol );

    smoothedVol *= 0.93;
    smoothedVol += 0.07 * curVol;

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch(key) {
        case 'f':
            doFullScreen.set(!doFullScreen.get());
            break;
        case 'g':
            toggleGuiDraw.set(!toggleGuiDraw.get());
            break;
        case 'm':
            useMicrophone.set(!useMicrophone.get());
            break;
        case 'r':
            resetMesh();
            break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

//--------------------------------------------------------------
void ofApp::setAudioSource(bool& _useMicrophone) {
    cout << "are we gonna use mic? " << _useMicrophone << endl;

    if (_useMicrophone) {
        audioInput.setDeviceID(0); // microphone
    } else {
        audioInput.setDeviceID(5); // Loopback audio
    }

    audioInput.setup(this, 0, 2, 44100, samplesPerBuffer, 4);

}

//--------------------------------------------------------------
void ofApp::resetMesh() {
    mainMesh.clear();

    //store the width and height for convenience
    int width = vidGrabber.getWidth();
    int height = vidGrabber.getHeight();

    //add one vertex to the mesh for each pixel
    for (int y = 0; y < height; y++){
        for (int x = 0; x<width; x++){
            mainMesh.addVertex(ofPoint(x,y,0));    // mesh index = x + y*width
            // this replicates the pixel array within the camera bitmap...
            mainMesh.addColor(ofFloatColor(0,0,0));  // placeholder for colour data, we'll get this from the camera
        }
    }

    for (int y = 0; y<height-1; y++){
        for (int x=0; x<width-1; x++){
            mainMesh.addIndex(x+y*width);                // 0
            mainMesh.addIndex((x+1)+y*width);            // 1
            mainMesh.addIndex(x+(y+1)*width);            // 10

            mainMesh.addIndex((x+1)+y*width);            // 1
            mainMesh.addIndex((x+1)+(y+1)*width);        // 11
            mainMesh.addIndex(x+(y+1)*width);            // 10
        }
    }
}
