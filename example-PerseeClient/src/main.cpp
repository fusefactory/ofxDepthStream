// OF
#include "ofMain.h"
// addons
#include "ofxOrbbecPersee/ofxOrbbecPersee.h"
// local

class ofApp : public ofBaseApp{

	public:
		void setup() override;
		void update() override;
		void draw() override;

		// void keyPressed(int key);
		// void keyReleased(int key);
		// void mouseMoved(int x, int y );
		// void mouseDragged(int x, int y, int button);
		// void mousePressed(int x, int y, int button);
		// void mouseReleased(int x, int y, int button);
		// void mouseEntered(int x, int y);
		// void mouseExited(int x, int y);
		// void windowResized(int w, int h);
		// void dragEvent(ofDragInfo dragInfo);
		// void gotMessage(ofMessage msg);

};

void ofApp::setup() {
}

void ofApp::update() {

}

void ofApp::draw() {
  ofBackground(0);
}

//========================================================================

int main( ){
	ofSetupOpenGL(800, 600, OF_WINDOW);
	ofRunApp(new ofApp());
}
