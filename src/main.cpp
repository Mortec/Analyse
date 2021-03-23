#include "ofMain.h"
#include "ofApp.h"
#include "ofAppGLFWWindow.h"
#include "ofxWatchdog.h"

//========================================================================
int main( ){
    
    
    int count = 0;
    
    while (count < 5) {
        ofSleepMillis(1000);
        count +=1 ;
    }
    

    
    CGDisplayCount displayCount;
    CGDirectDisplayID displays[32];
    CGGetActiveDisplayList(32, displays, &displayCount);
    int numDisplays = 1;//displayCount;
    
    cout<<displayCount<<" display(s) detected."<<endl;
    
    ofGLFWWindowSettings settings;
    settings.width = 1280;
    settings.height = 800;
    //settings.setPosition(ofVec2f(0,0));
    settings.resizable = true;
    settings.monitor=0;
    if (numDisplays > 1) {
        settings.windowMode = OF_FULLSCREEN;
    }
    
    shared_ptr<ofAppBaseWindow> mainWindow = ofCreateWindow(settings);
    mainWindow -> setVerticalSync(false);
    
    
    if (numDisplays > 1) {
        
        settings.width = 1280*2;
        settings.height = 720;
        //settings.setPosition(ofVec2f(0,0));
        settings.resizable = true;
        settings.shareContextWith = mainWindow;
        if (numDisplays > 1) {
            settings.monitor=1;
            settings.windowMode = OF_FULLSCREEN;
        }
        else settings.monitor=0;
        
        
        shared_ptr<ofAppBaseWindow> secondWindow = ofCreateWindow(settings);
        secondWindow -> setVerticalSync(false);
        
        ofSetFrameRate(30);
        
        shared_ptr<ofApp> mainApp(new ofApp);
        ofAddListener(secondWindow->events().draw,mainApp.get(),&ofApp::drawSecondScreen);
        
        ofRunApp(mainWindow, mainApp);
        ofRunMainLoop();
    }
    
    else ofRunApp(new ofApp());
    

    
    

}

/**
 #include "ofMain.h"
 #include "ofApp.h"
 
 //========================================================================
 int main( ){
 
 // Get screen widths and heights from Quartz Services
 // See https://developer.apple.com/library/mac/documentation/GraphicsImaging/Reference/Quartz_Services_Ref/index.html
 
 CGDisplayCount displayCount;
 CGDirectDisplayID displays[32];
 
 // Grab the active displays
 CGGetActiveDisplayList(32, displays, &displayCount);
 int numDisplays= displayCount;
 
 // If two displays present, use the 2nd one. If one, use the first.
 int whichDisplay= numDisplays-1;
 
 int displayHeight= CGDisplayPixelsHigh ( displays[whichDisplay] );
 int displayWidth= CGDisplayPixelsWide ( displays[whichDisplay] );
 
 /////////////// this is a bit slow to do  /////////////
 // move 2nd display to right of primary display and align display tops
 if(numDisplays > 0){
 CGDisplayConfigRef displayConfig;
 CGBeginDisplayConfiguration ( &displayConfig );
 CGConfigureDisplayOrigin ( displayConfig, displays[1], CGDisplayPixelsWide(displays[0]), 0 );
 CGCompleteDisplayConfiguration ( displayConfig, kCGConfigureForAppOnly );
 }
 /
 
 
// //////// instead let's just moving our window to wherever our display is living:

CGRect displayBounds= CGDisplayBounds ( displays[whichDisplay] );

ofSetupOpenGL(displayWidth, displayHeight, OF_FULLSCREEN);            // <-------- setup the GL context
// that OF_FULLSCREEN makes the window as big as the primary display, but we want it to be as big as whichever we're using
ofSetWindowShape(displayWidth, displayHeight);
// move onto our display.
ofSetWindowPosition(displayBounds.origin.x, displayBounds.origin.y);

// print display info.
cout<<numDisplays<<" display(s) detected."<<endl<<"Using display "<<whichDisplay<<" ("<<displayWidth<<"x"<<displayHeight<<")."<<endl;

// this kicks off the running of my app
// can be OF_WINDOW or OF_FULLSCREEN
// pass in width and height too:

ofRunApp(new ofApp());
}
 **/
