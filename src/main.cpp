#include "ofMain.h"
#include "ofApp.h"

#define MACWIDTH 1024
#define MACHEIGHT 768

#define WINWIDTH 1920
#define WINHEIGHT 1080

//========================================================================
int main( ){
	ofSetupOpenGL(MACWIDTH, MACHEIGHT, OF_WINDOW);
	//ofSetupOpenGL(WINWIDTH, WINHEIGHT,OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());

}
