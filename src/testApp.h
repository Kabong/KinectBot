#ifndef _TEST_APP
#define _TEST_APP

#include "ofMain.h"
#include "ofxONI.h"

#define MAX_PLAYERS 5

class testApp : public ofBaseApp{

	public:
		testApp();
		~testApp();
						
		void setup();
		void update();
		void draw();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void resized(int w, int h);

		ofxONI oni;
		
		int numOfPlayres;
		ofxCvGrayscaleImage players[MAX_PLAYERS];
		ofTrueTypeFont franklin;

};

#endif
