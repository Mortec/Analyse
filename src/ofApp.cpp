
#include "ofApp.h"




//-------------------------------------------------------------------------------------------------------------------------

//::::	SETUP

//-------------------------------------------------------------------------------------------------------------------------



void ofApp::setup() {
    
    string time = ofGetTimestampString("%d-%m-%Y-%H-%M-%S");
    
    startTime = "startime : " + time;
    
    string exedir = ofSystem("pwd");
    
    exedir  = exedir.substr(0, exedir.size()-1);
    
    ofDirectory dir (exedir+"/data");
    
    
    if( dir.exists() ){
        ofSetDataPathRoot( exedir+"/data/" );
    }
    
    string user = ofSystem("whoami");
    
    user  = user.substr(0, user.size()-1);
    
    
    ofLogToFile("logs/" + user + "_" + time + ".txt" );
    
    ofLogNotice() << ":::: USER :::" << user << "\n";
    ofLogNotice() << ":::: EXEC DIR :::" << exedir << "\n";
    ofLogNotice() << ":::: DATAPATH :::" << ofToDataPath("") << "\n";
    
    datapath = exedir+"/data/";
    
    currentAudioOutput = ofSystem("/usr/local/bin/SwitchAudioSource -c");
    currentAudioOutput = currentAudioOutput.substr(0, currentAudioOutput.size()-1);
    
    ofLogNotice() << ofSystem("/usr/local/bin/SwitchAudioSource -s \"Soundflower (2ch)\"") << "\n";
    
    ofPixels blackPix;
    blackPix.allocate(50, 50, OF_PIXELS_RGB);
    blackPix.setColor(ofColor::black);
    
    blackTex.allocate(blackPix);
    blackTex.loadData(blackPix);
    
    videoTex = &blackTex;
    
    audioVideoTex = &blackTex;
    
    loadDefaultGuiParameters();
    
    initCams();
    
    setupConfig();
    setupAudio();
    setupVideo();
    setupPrint();
    setupAutomations();
    
    if ( HAPmode ){
        startHap();
        manager.setRecHAP(true);
    }
    

    
    
    this->gui.setup();
    setGrayStyle();
    this->guiVisible = true;
    
    scOscSender.setup(scServerAddr.get(), scServerPort.get() );
    
    lastVsource = videoInputOption;
    lastAsource = audioVideoInputOption;
    
    
    if (autoStartAudio.get() ){
        startAudioUnits( audioInputDevice, audioExtInputDevice, audioOutputDevice);
        loadAudioUnitsPresets(audioPresetName);
    }
    
    
    if(autoStartVcam.get() ) {
        if (videoInputOption.get() == 1){
            changeVideoCam(videoUsbInputOption);
            int tempo = 0;
            while (videoCamera.isInitialized() == false && tempo < 5500) {
                sleep(250);
                tempo+=250;
            }
        }
    }
    
    
    
    
    if(autoStartSc.get()) {
        startSc();
        ///sleep(1000);
    }
    
    
    if (autoPlay.get() ) play();
    
    
    if (autoRec.get() ) manager.runRec();
    
    
    if (autoPrint.get() ) {
        //autoprintstuff here
    }
    
    changeView(viewNum.get() );
    
    
    //if (toggleFullScreen.get() ) ofToggleFullscreen();
    
    
    
    
    ofLogNotice("[:::::MAIN:::::] SETUP COMPLETE");
    setupComplete = true;
    
}



//-------------------------------------------------------------------------------------------------------------------------


//::::	UPDATE


//-------------------------------------------------------------------------------------------------------------------------


void ofApp::update() {
    
    if (autoShutDown){
        string stHour = ofGetTimestampString("%H");
        int hour = ofToInt(stHour);
        string stMin = ofGetTimestampString("%M");
        int min = ofToInt(stMin);
        
        if (hour == shutDownHour && min == shutDownMinute) exit();
    }
    
    long a = ofGetElapsedTimeMillis();
    
    transferManagerSettings();
    
    
    //UPDATE_VIDEOS_SOURCES//-------------------------------------------------------
    
    //videoInputOptions = {"Movies", "USB", "Canon", "Syphon", "Broadcast", "SoundGradient", "None"};
    //audioVideoInputOptions = {"Movies", "USB", "Canon", "Syphon", "None"};
    
    if (videoInputOption==0 || audioVideoInputOption==0) {
        updateFile();
    }
    
    if (videoInputOption==1) {
        if (videoCamera.isInitialized() ) videoCamera.update();
    }
    
    if (audioVideoInputOption==1 && videoUsbInputOption != audioVideoUsbInputOption ) {
        if (audioVideoCam ->isInitialized() ) audioVideoCam ->update();
        
    }
    
    
    if (videoInputOption==2 || audioVideoInputOption==2) {
        //update Syphon
    }
    
    
    updateVideo();
    
    updateAudioVideo();
    
    
    if (videoAnalyseOn ) {
        getVideoData();
    }
    
    
    
    
    
    //UPDATE_COLORGRAPH//-------------------------------------------------------
    
    lastColorGraph.setColor(ofColor::black);
    
    for (int i = 0; i < nBandsHisto; i++) {
        
        int index = ofMap(i, 0, nBandsHisto-1, 0, nBandsAnalysis-1);
        
        int y = (int) ( ofMap( presences[index], 0, 1, 5, lastColorGraph.getHeight() - 5 ) );
        
        ofColor c;
        
        
        float hu = colorSpectrum.getColor(index, 0).getHue();
        c.setHsb(hu, saturations[index] * 255.0, brightnesses[index] * 255.0);
        
        lastColorGraph.setColor(1, y, c);
        lastColorGraph.setColor(1, y - 1, c);
        
    }
    
    colorGraph.cropTo( tempColorGraph, 0, 0, colorGraph.getWidth() - 1, colorGraph.getHeight());
    lastColorGraph.pasteInto(colorGraph, 0, 0);
    tempColorGraph.pasteInto(colorGraph, 1, 0);
    
    colorGraphTex.loadData(colorGraph);
    
    
    
    
    //UPDATE_THUMBNAIL//-------------------------------------------------------
    
    frameCounter++;
    
    /*
     thumbsWidth = (int)( ( ofGetWidth() - ( numThumbs * 10 ) ) / numThumbs );
     
     thumbsHeight = (int)( (float)thumbsWidth * ((float)videoHeight / (float)videoWidth) );
     
     if ( (frameCounter % (thumbsWidth + 10 * ofGetWidth() / 1280) ) == 0) {
     
     thumbs = analyzedPixels;
     
     thumbs.resize( (int)thumbsWidth.get(), (int)thumbsHeight.get() );
     
     thumbsTex.at(thumbsCursor).loadData(thumbs);
     
     thumbsCursor = (thumbsCursor + 1) % (numThumbs + 1);
     
     }
     */
    
    
    
    // CHANGE_MOVIE//-------------------------------------------------------
    
    if (HAPmode && silenceTimer < 50 ) { testSound(); }
    
    else {
        
        if ( autoPlay ) {
            
            if (playing) {
                
                int elapsedTimeSeconds = (int)(ofGetElapsedTimeMillis() / 1000) - timeFlag;
                
                if (elapsedTimeSeconds >= movieTempo * 60) {
                    
                    changeFileAuto();
                    
                    timeFlag = (int)(ofGetElapsedTimeMillis() / 1000);
                    
                    movieTempo = ofRandom(zapDurationMin, zapDurationMax);
                }
            }
        }
        
        
        if (audioVideoInputOption==0 ) detectError();
        
    }
    
    
    //UPDATE_AUDIO_DATA//-------------------------------------------------------
    
    testRms = pow( inputTap.getRMS(0), 0.5 );
    
    //if (HAPmode) { hap.updateRMS( testRms ); }
    
    
    
    //DO GRADIENT & BROADCAST//-------------------------------------------------------
    if (audioAnalyseOn) {
        
        
        static ofxAudioUnitTap::StereoSamples samples;
        
        outputTap.getSamples(samples);
        
        //        analyzeSamplesLeft(samples.left);
        
        //        if (stereoGradient) {
        //
        //            analyzeSamplesRight(samples.right);
        //
        //            soundGradientPix = doGradientStereo(aStartFreq.get(), aEndFreq.get(), cStartFreq.get(), cEndFreq.get(), mStartBand.get() , mEndBand.get() );
        //        }
        //
        //        else {
        //            soundGradientPix = doGradient(aStartFreq.get(), aEndFreq.get(), cStartFreq.get(), cEndFreq.get(), mStartBand.get() , mEndBand.get() );
        //
        //        }
        
        //        analyzeSamplesLeft(samples.left);
        //        if (stereoGradient) analyzeSamplesRight(samples.right);
        //        selectAudioData();
        //        interpolateAudioData();
        //        soundGradientPix = applyGradient();
        //        soundGradientTex.loadData(soundGradientPix);
        
        analyzeSamplesLeft(samples.left);
        analyzeSamplesRight(samples.right);
        selectAudioData();
        soundGradientPix = doGradientStereo(aStartFreq.get(), aEndFreq.get(), cStartFreq.get(), cEndFreq.get(), mStartBand.get() , mEndBand.get() );
        soundGradientTex.loadData(soundGradientPix);
        
        
        
        ///AUDIOGRAM
        for (int i = 0; i < audioGram.getHeight(); i++ ) {
            
            int index = iround(ofMap(i, 0, audioGram.getHeight()-1, 0, spectrumNorm.size()-1 ) );
            
            float l = spectrumNorm.at(index) * 255;
            
            audioGram.setColor( frameCounter%audioGram.getWidth(), audioGram.getHeight()- i, l);
        }
        
        
        audioGramTex.loadData( audioGram);
    }
    
    
    
    
    
    
    ///GRADIENT
    gradientFbo.begin();
    ofClear(0,0,0,1);
    avBCSA.begin();
    avBCSA.setUniform3f("avgluma", 0.62,0.62,0.62);
    avBCSA.setUniform1f("contrast", gradientContrast);
    avBCSA.setUniform1f("brightness", gradientBrightness);
    avBCSA.setUniform1f("saturation", ofClamp(gradientSaturation, 0, 5) );
    avBCSA.setUniform1f("alpha", gradientAlpha);
    avBCSA.setUniform1i("invert", invertGradient);
    
    avBCSA.setUniformTexture("image", soundGradientTex, 1);
    
    soundGradientTex.draw(0, gradientFbo.getHeight() , gradientFbo.getWidth(), -gradientFbo.getHeight() );
    avBCSA.end();
    gradientFbo.end();
    //////////////////
    
    
    
    
    ///BROADCAST
    broadcastFbo.begin();
    ofClear(0, 0, 0, 1);
    
    avBlend.begin();
    avBlend.setUniform1f( "contrast",	1.0f );
    avBlend.setUniform1f( "brightness",	0.0f );
    avBlend.setUniform1f( "blendmix",	audioVideoMix );
    avBlend.setUniform1i( "blendmode",	blendMode );
    
    if (swap) {
        avBlend.setUniformTexture("texBase",  gradientFbo.getTexture(), 0 );
        avBlend.setUniformTexture("texBlend", audioVideoFbo.getTexture(), 1 );
    }
    else {
        avBlend.setUniformTexture("texBase",   audioVideoFbo.getTexture(), 0 );
        avBlend.setUniformTexture("texBlend",   gradientFbo.getTexture(), 1 );
    }
    
    glBegin(GL_QUADS);
    
    glTexCoord2f(0, 0);
    glVertex3f(0, 0, 0);
    
    glTexCoord2f(1920, 0);
    glVertex3f(1920, 0, 0);
    
    glTexCoord2f(1920, 1080);
    glVertex3f(1920, 1080, 0);
    
    glTexCoord2f(0,1080);
    glVertex3f(0,1080, 0);
    
    glEnd();
    
    avBlend.end();
    
    broadcastFbo.end();
    /////////////////////
    
    
    
    
    ///-------------AUDIO SCREEN----------------///
    audioScreen.begin();
    ofClear(0, 0, 0, 1);
    
    ofFill();
    ofSetColor(ofColor::black);
    ofDrawRectangle(0, 0, audioScreen.getWidth(), audioScreen.getHeight() );
    
    ofSetColor(ofColor::white);
    
    /*
     audioVideoFbo.draw(36, 40, 450, 270);
     
     ofColor(audioColor);
     ofNoFill();
     drawWaveForm(560, 40, 340, 270);
     
     ofFill();
     drawSoundSpectrum(960, 40, 924, 260);
     
     
     
     ofSetColor(audioColor);
     //drawAudiogram(0, 360, 1920, 120);
     
     audioGramTex.draw( 1920 - (frameCounter%3840), 360, 1920, 120 );
     audioGramTex.draw( 1920 - (frameCounter + 1920)%3840, 360, 1920, 120 );
     
     drawPresencesChoice(36, 530, 230, 120);
     //ofDrawRectangle(306, 530, 40, 120 * *agPresence);////////////////////////
     drawHues(384, 530, 558, 120);
     
     drawBrightnessesChoice(36, 730, 230, 120);
     ofFill();
     ofSetColor(audioColor);
     ofDrawRectangle(306, 730+120, 40, -120 * agGlobalModders.at(globalBrightness));
     drawBrightnesses(384, 730, 558, 120);
     
     drawSaturationsChoice(36, 930, 230, 120);
     ofFill();
     ofSetColor(audioColor);
     ofDrawRectangle(306, 930+120, 40, -120 * agGlobalModders.at(globalSaturation));
     drawSaturations(384, 930, 558, 120);
     
     drawAlphasChoice(36, 1130, 230, 120);
     ofFill();
     ofSetColor(audioColor);
     ofDrawRectangle(306, 1130+120, 40, -120 * agGlobalModders.at(globalAlpha));
     drawAlphas(384, 1130, 558, 120);
     
     //drawBroadcast(980, 536, 906, 508);
     ofSetColor(ofColor::white);
     gradientFbo.draw(980, 536, 906, 508);
     */
    
    int width = iround(audioScreen.getWidth() );
    int height = iround(audioScreen.getHeight() );
    ofFill();
    
    drawSoundSpectrum(0, 0, width, height/2);
    
    audioGramTex.draw( width - (frameCounter%( width*2) ), height/2 , width, height/2 );
    
    audioGramTex.draw( width - (frameCounter + width )%( width*2 ), height/2 , width, height/2 );
    
    
    
    audioScreen.end();
    
    
    
    ///--------------------VIDEO SCREEN--------------------/////
    
    videoScreen.begin();
    //drawVideoAnalysis( 0,  0, 1920, 1080);
    
    ofClear(0, 0, 0, 1);
    
    ofFill();
    ofSetColor(ofColor::black);
    ofDrawRectangle(0, 0, audioScreen.getWidth(), audioScreen.getHeight() );
    
    ofSetColor(ofColor::white);
    ofFill();
    
    float space = videoScreen.getHeight() / nBandsHisto;
    
    for (int i = 0; i < nBandsHisto; i++) {
        
        int index = iround(ofMap(i, 0, nBandsHisto-1, 0, nBandsAnalysis-1));
        
        ofVec2f pos = { 0, i * space };
        
        ofColor c;
        
        float h = presences[index] * videoScreen.getWidth();
        
        float hu = colorSpectrum.getColor(index, 0).getHue();
        
        c.setHsb(hu, saturations[index] * 255.0, 255);//brightnesses[index] * 255.0);
        ofSetColor(c);
        ofDrawRectangle(pos.x, videoScreen.getHeight() - pos.y, h, space );
    }
    
    
    
    
    
    videoScreen.end();
    
    
    
    //UPDATE_INST//-------------------------------------------------------
    
    if (midiOutOn == 1){
        updateMidi();
        sendMidi();
    }
    
    if (iFFTon == 1){
        updateIFFT();
        sendOSCifft();
    }
    
    if(scSynthOn == 1){
        updateSynth();
        sendOSCsynth();
    }
    
    //    sendAudioOscData();
    //    sendVideoOscData();
    //    sendVideoToSyphon();
    
    
    
    a = ofGetElapsedTimeMillis() - a;
    
    
    
}




//-------------------------------------------------------------------------------------------------------------------------


//::::	DRAW


//-------------------------------------------------------------------------------------------------------------------------


void ofApp::draw() {
    
    int a = ofGetElapsedTimeMillis();
    if (videoAnalyseOn) {
        videoFbo.draw(0,0, analysisWidth,analysisHeight);
        ofImage im;
        im.grabScreen(0,0,analysisWidth,analysisHeight);
        analyzedPixels = im.getPixels();
    }
    a = ofGetElapsedTimeMillis() - a;
    
    //debug = ofToString(a);
    
    
    
    
    if (viewName.get() == "Analyse Video") {
        drawVideo();
        
    }
    
    if (viewName.get() == "Analyse Audio") {
        drawAudio();
        
    }
    
    
    if (viewName.get() == "Broadcast") {
        drawBroadcast(0, 0, ofGetWidth(), ofGetHeight() );
    }
    
    
    if (viewName.get() == "Print") {
        drawPrint();
    }
    
    if (viewName.get() == "Automations") {
        ofBackground(15, 15, 15);
    }
    
    
    
    drawGui();
    
    
    
}


void ofApp::drawSecondScreen(ofEventArgs & args){
    
    int w = ofGetScreenWidth()/2;
    int h = w *audioScreen.getHeight()/audioScreen.getWidth();
    
    
    ofPushStyle();
    ofBackground(0);
    
    
    
    videoScreen.draw(0,0, w, h);
    
    audioScreen.draw(w, 0, w, h);
    
    ofSetColor(white);
    ofDrawLine( w, 0, w, h);
    
    ofPopStyle();
    
    
}



bool ofApp::drawGui() {
    
    
    //    //set window properties
    static bool no_titlebar = false;
    static bool no_border = true;
    static bool no_resize = false;
    static bool no_move = true;
    static bool no_scrollbar = false;
    static bool no_collapse = true;
    static bool no_menu = true;
    static bool no_settings = false;
    static bool no_scroll_mouse = false;
    //static bool no_input = false;
    static float bg_alpha = -0.01f; // <0: default
    bool show = true;
    
    if (no_titlebar)  window_flags |= ImGuiWindowFlags_NoTitleBar;
    if (!no_border)   window_flags |= ImGuiWindowFlags_ShowBorders;
    if (no_resize)    window_flags |= ImGuiWindowFlags_NoResize;
    if (no_move)      window_flags |= ImGuiWindowFlags_NoMove;
    if (no_scrollbar) window_flags |= ImGuiWindowFlags_NoScrollbar;
    if (no_collapse)  window_flags |= ImGuiWindowFlags_NoCollapse;
    if (!no_menu)     window_flags |= ImGuiWindowFlags_MenuBar;
    if (no_settings) window_flags |= ImGuiWindowFlags_NoSavedSettings;
    //if (no_scroll_mouse) window_flags |= ImGuiWindowFlags_NoScrollWithMouse;
    //window_flags |= ImGuiWindowFlags_AlwaysVerticalScrollbar;
    //if (no_input) window_flags |=ImGuiWindowFlags_NoInputs;
    
    //    ImGui::Begin("My Title", &show, ImVec2(myWidth, myHeight), bg_alpha, window_flags);
    
    //ImGui::SetNextWindowSize(ImVec2(395,780), ImGuiSetCond_FirstUseEver);
    //setGrayStyle();
    
    guiSettings = ofxImGui::Settings();
    if (viewName.get() == "Automations") guiSettings.windowSize=ofVec2f(ofGetWidth()-15, ofGetHeight()-15);
    else guiSettings.windowSize=ofVec2f(390, ofGetHeight()-15);
    
    bool ok = true;
    this->gui.begin();
    {
        if (viewName.get() == "Analyse Video") {
            drawImGuiVideo();
        }
        
        if (viewName.get() == "Analyse Audio") {
            drawImGuiAudio();
        }
        
        if (viewName.get() == "Automations") {
            drawImGuiAutomation();
        }
    }
    this->gui.end();
    return guiSettings.mouseOverGui;
    
    
}


//-------------------------------------------------------------------------------------------------------------------------


//VIDEO STUFF


//-------------------------------------------------------------------------------------------------------------------------



void ofApp::updateVideo() {
    
    //vector<string> videoInputOptions = {"Movies", "USB", "Canon", "Syphon", "Broadcast", "SoundGradient", "None"};
    
    switch (videoInputOption){
            
            //MOVIES
        case 0 :
            
            if (HAPmode) {
                
                if (hapStarted){
                    videoTex = hap.getTexture();
                }
                else videoTex = &blackTex;
            
            }
            else if (playing) {
                if (moviePlayer.at(playerSelector).isLoaded() && moviePlayer.at(playerSelector).isFrameNew() && moviePlayer.at(playerSelector).getCurrentFrame()>1 ) {
                    videoTex = &moviePlayer.at(playerSelector).getTexture();
                }
                
            }
            
            
            break;
            
            //USB
        case 1 :
            if (videoCamera.isInitialized() ){
                if (videoCamera.isFrameNew() ) {
                    videoTex = &videoCamera.getTexture();
                }
            }
            break;
            
            
            //SYPHON
        case 2 :
            //getSyphonTex
            //videoTex = syphonVtex;
            videoTex = &blackTex;
            break;
            
            //BROADCAST
        case 3 :
            videoTex = &broadcastFbo.getTexture();
            
            break;
            
            //SOUNDGRADIENT
        case 4 :
            videoTex = &gradientFbo.getTexture();
            break;
            
            //NONE
        case 5 :
            //videoTex.clear();
            //videoTex = &blackTex;
            videoTex = &blackTex;
            break;
            
            
            
            
            
            
    }
    
    
    float w = videoFbo.getWidth() * zoomFactor ;
    float h = videoFbo.getHeight() * zoomFactor ;
    float x = videoFbo.getWidth()/2*(1 - (1-zoomFactor) * zoomPosX );
    float y =  videoFbo.getHeight()/2*(1 - (1-zoomFactor) * zoomPosY );
    
    if (videoTex != NULL && videoTex -> isAllocated() ){
        
        videoFbo.begin();
        
        ofClear(0, 0, 0, 1);
        
        avBCSA.begin();
        avBCSA.setUniform3f("avgluma", 0.62,0.62,0.62);
        //avBCSA.setUniform1f("gamma", gamma);
        avBCSA.setUniform1f("contrast", contrast);
        avBCSA.setUniform1f("brightness", brightness);
        avBCSA.setUniform1f("saturation", saturation);
        avBCSA.setUniform1f("alpha", 1.0);
        avBCSA.setUniformTexture("image", *videoTex, 1);
        
        videoTex -> setAnchorPercent(0.5, 0.5);
        videoTex -> draw(x, y, w, h);
        videoTex -> setAnchorPercent(0.0, 0.0);
        
        avBCSA.end();
        
        videoFbo.end();
    }
    
    
    
}









//--------------------------------------------------------------

void ofApp::getVideoData() {
    
    videoAnalyzer.lock();
    
    if ( videoAnalyzer.getReady() ) {
        
        
        presences = videoAnalyzer.getPresences(histoCont, histoAmp, histoSmooth);
        brightnesses = videoAnalyzer.getBrightnesses( luCont, luAmp, luSmooth );
        saturations = videoAnalyzer.getSaturations(satCont, satAmp, satSmooth);
        posXs = videoAnalyzer.getPositionsX(panCont, panAmp, panSmooth);
        posYs = videoAnalyzer.getPositionsY(tiltCont, tiltAmp, tiltSmooth);
        
        //videoAnalyzer.getVelocities(veloCont, veloAmp, veloSmooth);
        //videoAnalyzer.getAngles(anglesCont, anglesAmp, anglesSmooth);
        //videoAnalyzer.getLightness(lightnessCont, lightnessAmp, lightnessSmooth);
        
        white = videoAnalyzer.getWhite(whiteThresh, whiteCont, whiteAmp, whiteSmooth);
        neutral = videoAnalyzer.getNeutral(neutralThresh, neutralCont, neutralAmp, neutralSmooth);
        black = videoAnalyzer.getBlack(blackThresh, blackCont, blackAmp, blackSmooth);
        
        averageColor = videoAnalyzer.getAverageColor();
        averageHue = videoAnalyzer.getAveragehue(averageHueSmooth);
        averageBrightness = videoAnalyzer.getAverageBrightness(averageBrightnessAmp, averageBrightnessSmooth);
        averageSaturation = videoAnalyzer.getAverageSaturation(averageSaturationAmp, averageSaturationSmooth);
        
        videoAnalyzer.newFrame(analyzedPixels, nBandsAnalysis.get() );
        
    }
    
    videoAnalyzer.unlock();
    
}

//--------------------------------------------------------------




//-------------------------------------------------------------------------------------------------------------------------


//VIDEO DRAW


//-------------------------------------------------------------------------------------------------------------------------

void ofApp::drawSource(float x, float y, float width, float height) {
    
    ofSetHexColor(0xffffff);
    if (videoTex != NULL && videoTex->isAllocated() ){
        videoTex->setAnchorPercent(0, 0);
        videoTex->draw(x, y+5, width, height-5);
    }
    
    ofSetColor(fontColor);
    ofDrawBitmapString("Input Preview", x, y );
}


//--------------------------------------------------------------------------------------

void ofApp::drawSourceInfo(float x, float y, float width, float height) {
    
    ofSetColor(fontColor);
    
    ofFill();
    
    ofDrawBitmapString(startTime, x, y);
    
    ofDrawBitmapString(ofGetTimestampString("%d-%m-%Y-%H-%M-%S"), x, y + 20); //The default timestamp format is "%Y-%m-%d-%H-%M-%S-%i" (e.g. 2011-01-15-18-29-35-299).
    
    
    
    ofDrawBitmapString(fileName, x, y + 40 );
    
    ofDrawBitmapString("fps : " + ofToString(ofGetFrameRate()), x, y + 60);
    
    ofColor rc;
    if (recorderState == "Sleeping") { rc = ofColor::green;}
    if (recorderState == "Recording") { rc = ofColor::red;}
    if (recorderState == "Testing") { rc = ofColor::orange;}
    if (recorderState == "Stopped") { rc = ofColor::yellow;}
    
    ofSetColor(rc);
    ofDrawBitmapString("AutoRecord : " + recorderState, x + width - 200, y );
    ofDrawBitmapString(currentStream, x + width - 200, y + 20 );
    
}

//--------------------------------------------------------------------------------------


void ofApp::drawProcessed(float x, float y, float width, float height) {
    
    ofSetHexColor(0xffffff);
    videoFbo.draw(x, y + 5, width, height-5);
    
    //    ofSetColor(ofColor::black);
    //    ofDrawRectangle(x, y, width/2*videoCrop.get(), height);
    //    ofDrawRectangle(x+width, y, -width/2*videoCrop.get(), height);
    ofSetColor(fontColor);
    ofDrawBitmapString("Processed Video", x, y);
}


//--------------------------------------------------------------------------------------


void ofApp::drawThumbnails(float x, float y, float width, float height) {
    
    ofSetHexColor(0xffffff);
    
    int offset  = (ofGetWidth() + frameCounter) % (thumbsWidth + 10);
    
    float space = ofGetWidth() / 128.0f;
    
    for (int i = 0 ; i < ( numThumbs + 1); i++) {
        
        int index = (i + thumbsCursor) % (numThumbs + 1);
        
        thumbsTex.at(index).draw ( ( x + (space + width) * i ) - offset, y, width, height );
        
    }
}


//--------------------------------------------------------------------------------------

void ofApp::drawHisto(float x, float y, float width, float height) {
    
    ofFill();
    ofSetColor(ofColor::black);
    ofDrawRectangle(x, y, width, -height);
    
    float space = width / nBandsHisto;
    
    for (int i = 0; i < nBandsHisto; i++) {
        
        int index = iround(ofMap(i, 0, nBandsHisto-1, 0, nBandsAnalysis-1));
        
        ofVec2f pos = { x + i * space, y };
        
        float h = presences[index] * height;
        
        ofColor c;
        
        float hu = colorSpectrum.getColor(index, 0).getHue();
        
        c.setHsb(hu, 255.0, 255.0);
        ofSetColor(c);
        ofDrawRectangle(pos.x, pos.y, space, -h);
    }
    
    ofFill();
    ofSetColor(ofColor::black);
    ofDrawRectangle(x+width, y, width/16.0f, -height * black);
    
    ofNoFill();
    ofSetColor(frameColor);
    ofDrawRectangle(x+width, y, width/16.0f, -height * black);
    
    ofFill();
    ofSetColor(ofColor::gray);
    ofDrawRectangle(x+width + width/16.0f, y, width/16.0f, -height * neutral);
    
    ofFill();
    ofSetColor(ofColor::white);
    ofDrawRectangle(x+width + 2*width/16.0f, y, width/16.0f, -height * white);
    
    ofSetColor(frameColor);
    ofNoFill();
    ofDrawRectangle(x, y, width, -height);
    
    ofSetColor(fontColor);
    ofDrawBitmapString("Colors Presences", x, y - height -5);
}

//--------------------------------------------------------------------------------------

void ofApp::drawLuSats(float x, float y, float width, float height) {
    
    ofSetColor(ofColor::white);
    ofFill();
    
    float space = width / nBandsHisto;
    
    for (int i = 0; i < nBandsHisto; i++) {
        
        int index = iround(ofMap(i, 0, nBandsHisto-1, 0, nBandsAnalysis-1));
        
        ofVec2f pos = { x + i * space, y };
        
        ofColor c;
        
        float hu = colorSpectrum.getColor(index, 0).getHue();
        
        c.setHsb(hu, saturations[index] * 255.0, brightnesses[index] * 255.0);
        ofSetColor(c);
        ofDrawRectangle(pos.x, pos.y-height/2, space, -height/2);
    }
    
    float lx1 = ofMap(videoStartC.get(), 0, 1023, 0, width);
    float lx2 = ofMap(videoEndC.get(), 0, 1023, 0, width);
    
    line(x + lx1, y -height, x + lx1, y-height/4, 1, frameColor);
    line(x + lx2, y -height, x + lx2, y-height/4, 1, frameColor);
    
    string f1 = ofToString( ofMap(videoStartC, 0, 1023, 380, 850) )+ " THz";
    string f2 = ofToString( ofMap(videoEndC, 0, 1023, 380, 850) )+ " THz";
    
    ofSetColor(fontColor);
    ofDrawBitmapString(f1, x + lx1-10, y-10);
    ofDrawBitmapString(f2, x + lx2-10-50, y);
    
    ofSetColor(frameColor);
    ofNoFill();
    ofDrawRectangle(x, y-height/2, width, -height/2);
    
    ofSetColor(fontColor);
    ofDrawBitmapString("Colors Brightnesses & Saturations", x, y - height -5);
}

//--------------------------------------------------------------------------------------

void ofApp::drawSats(float x, float y, float width, float height) {
    
    ofSetColor(ofColor::white);
    ofFill();
    
    float space = width / nBandsHisto;
    
    int offset = (int)( rotation * nBandsHisto);
    
    for (int i = 0; i < nBandsHisto; i++) {
        
        int index = iround(ofMap(i, 0, nBandsHisto-1, 0, nBandsAnalysis-1));
        
        ofVec2f pos = { x + i * space, y };
        
        ofColor c;
        
        float hu = colorSpectrum.getColor(index, 0).getHue();
        
        c.setHsb(hu, saturations[index] * 255.0f, 255);
        ofSetColor(c);
        ofDrawRectangle(pos.x, pos.y, space, -height);
    }
    
    ofSetColor(frameColor);
    ofNoFill();
    ofDrawRectangle(x, y, width, -height);
}

//--------------------------------------------------------------------------------------

void ofApp::drawLus(float x, float y, float width, float height) {
    
    ofSetColor(ofColor::white);
    ofFill();
    
    float space = width / nBandsHisto;
    
    int offset = (int)( rotation * nBandsHisto);
    
    for (int i = 0; i < nBandsHisto; i++) {
        
        int index = iround(ofMap(i, 0, nBandsHisto-1, 0, nBandsAnalysis-1));
        
        ofVec2f pos = { x + i * space, y };
        
        ofColor c;
        
        float hu = colorSpectrum.getColor(index, 0).getHue();
        
        c.setHsb(hu, 255.0f, brightnesses[index] * 255.0f);
        ofSetColor(c);
        ofDrawRectangle(pos.x, pos.y, space, -height);
    }
    
    ofSetColor(frameColor);
    ofNoFill();
    ofDrawRectangle(x, y, width, -height);
    
}

//--------------------------------------------------------------------------------------

void ofApp::drawXyPositions(float x, float y, float width, float height) {
    
    
    
    
    ofFill();
    ofSetColor(ofColor::black);
    ofDrawRectangle(x, y, width, -height);
    
    line(x + width / 2, y, x + width / 2, y - height, stroke, subFrameColor);
    line(x, y - height / 2, x + width, y - height / 2, stroke, subFrameColor);
    
    ofEnableAlphaBlending();
    ofEnableSmoothing();
    
    for (int i = 0; i < presences.size()-1; i+=1) {
        
        int index = i;
        
        float posx =  posXs[index] * (width / 2.0f ) ;
        float posy = posYs[index] * (height / 2.0f );
        
        ofColor c;
        
        float hu = colorSpectrum.getColor(index, 0).getHue();
        float sat = saturations[index] * 255.0f;
        float br = brightnesses[index] * 255.0f;
        float alph = presences[index] * 255.0f;
        
        c.setHsb(hu, sat, br);
        c.a = alph;
        
        ofSetColor(c);
        //ofDrawLine(x + width/2, y - height/2, x + width/2 + posx, y - height/2 + posy);
        ofFill();
        ofDrawCircle( x + width/2 + posx, y - height/2 + posy, 2 );
        
        
    }
    
    ofDisableAlphaBlending();
    ofDisableSmoothing();
    
    frame(x, y, width, -height, stroke, frameColor, false);
    
    ofSetColor(fontColor);
    ofDrawBitmapString("Colors Positions", x, y - height -5);
}


//--------------------------------------------------------------------------------------

void ofApp::drawColorGraph(float x, float y, float width, float height) {
    
    colorGraphTex.draw(x,  y, width, height);
    
}





//-------------------------------------------------------------------------------------------------------------------------


// OUTPUT SYPHON, OSC, MIDI ETC....


//-------------------------------------------------------------------------------------------------------------------------


void ofApp::sendOSCifft() {
    
    ofxOscBundle bundle;
    
    ofxOscMessage pv;
    pv.setAddress("/b_setn");
    pv.addIntArg(iFFTbufNum.get()+0 );
    pv.addIntArg(0 );
    pv.addIntArg(2048 );
    pv.addFloatArg(4096);
    pv.addFloatArg(0.25);
    pv.addFloatArg(1);
    pv.addFloatArg(0.0);
    for (int i = 2 ; i < 1024; i++ ) {
        pv.addFloatArg(  mags.at(i) );
        pv.addFloatArg( phases.at(i) * 3.14159f );
    }
    bundle.addMessage(pv);
    
    ofxOscMessage panL;
    panL.setAddress("/b_setn");
    panL.addIntArg(iFFTbufNum.get()+1 );
    panL.addIntArg(0 );
    panL.addIntArg(2048 );
    panL.addFloatArg(4096);
    panL.addFloatArg(0.25);
    panL.addFloatArg(1);
    panL.addFloatArg(0.0);
    for (int i = 2 ; i < 1024; i++ ) {
        panL.addFloatArg(  1 - pan.at(i) );
        panL.addFloatArg( phases.at(i) * 3.14159f );
    }
    bundle.addMessage(panL);
    
    ofxOscMessage panR;
    panR.setAddress("/b_setn");
    panR.addIntArg(iFFTbufNum.get()+2 );
    panR.addIntArg(0 );
    panR.addIntArg(2048 );
    panR.addFloatArg(4096);
    panR.addFloatArg(0.25);
    panR.addFloatArg(1);
    panR.addFloatArg(0.0);
    for (int i = 2 ; i < 1024; i++ ) {
        panR.addFloatArg(  pan.at(i) );
        panR.addFloatArg( phases.at(i) * 3.14159f );
    }
    bundle.addMessage(panR);
    
    ofxOscMessage tiltU;
    tiltU.setAddress("/b_setn");
    tiltU.addIntArg(iFFTbufNum.get()+3 );
    tiltU.addIntArg(0 );
    tiltU.addIntArg(2048 );
    tiltU.addFloatArg(4096);
    tiltU.addFloatArg(0.25);
    tiltU.addFloatArg(1);
    tiltU.addFloatArg(0.0);
    for (int i = 2 ; i < 1024; i++ ) {
        tiltU.addFloatArg(  tilt.at(i) );
        tiltU.addFloatArg( phases.at(i) * 3.14159f );
    }
    bundle.addMessage(tiltU);
    
    ofxOscMessage tiltD;
    tiltD.setAddress("/b_setn");
    tiltD.addIntArg(iFFTbufNum.get()+4 );
    tiltD.addIntArg(0 );
    tiltD.addIntArg(2048 );
    tiltD.addFloatArg(4096);
    tiltD.addFloatArg(0.25);
    tiltD.addFloatArg(1);
    tiltD.addFloatArg(0.0);
    for (int i = 2 ; i < 1024; i++ ) {
        tiltD.addFloatArg(  1-tilt.at(i) );
        tiltD.addFloatArg( phases.at(i) * 3.14159f );
    }
    bundle.addMessage(tiltD);
    
    ofxOscMessage mods;
    mods.setAddress("/b_setn");
    mods.addIntArg(iFFTbufNum.get()+5 );
    mods.addIntArg(0 );
    mods.addIntArg(6 );
    mods.addFloatArg( iFFTamp );
    mods.addFloatArg( iFFTloHi );
    mods.addFloatArg( iFFThiShift );
    mods.addFloatArg( iFFTwhiteMorph );
    mods.addFloatArg( iFFTconvoMix );
    mods.addFloatArg( scMasterVolume );
    
    bundle.addMessage(mods);
    
    
    
    
    scOscSender.sendBundle(bundle);
}



void ofApp::sendOSCsynth() {
    
    ofxOscBundle bundle;
    
    ofxOscMessage freqmess;
    freqmess.setAddress("/b_setn");
    freqmess.addIntArg(synthFreqBufnum.get() );
    freqmess.addIntArg(0 );
    freqmess.addIntArg(16 );
    for (int i = 0 ; i < 16; i++ ) {
        freqmess.addFloatArg( scFreqs.at(i).get() );
    }
    bundle.addMessage(freqmess);
    
    ofxOscMessage gatemess;
    gatemess.setAddress("/b_setn");
    gatemess.addIntArg(synthGateBufnum.get() );
    gatemess.addIntArg(0 );
    gatemess.addIntArg(20 );
    for (int i = 0 ; i < 16; i++ ) {
        gatemess.addFloatArg( (float)scTriggers.at(i) );
    }
    gatemess.addFloatArg(scAtt);
    gatemess.addFloatArg(scDec);
    gatemess.addFloatArg(scSus);
    gatemess.addFloatArg(scRel);
    bundle.addMessage(gatemess);
    
    ofxOscMessage velmess;
    velmess.setAddress("/b_setn");
    velmess.addIntArg(synthMod1BufNum.get() );
    velmess.addIntArg(0 );
    velmess.addIntArg(16 );
    for (int i = 0 ; i < 16; i++ ) {
        velmess.addFloatArg( scVelocities.at(i) );
    }
    bundle.addMessage(velmess);
    
    ofxOscMessage h1mess;
    h1mess.setAddress("/b_setn");
    h1mess.addIntArg(synthMod2BufNum.get() );
    h1mess.addIntArg(0 );
    h1mess.addIntArg(18 );
    for (int i = 0 ; i < 16; i++ ) {
        h1mess.addFloatArg( h1Mods.at(i) );
    }
    h1mess.addFloatArg(hNumber1);
    h1mess.addFloatArg(hPhase1);
    bundle.addMessage(h1mess);
    
    ofxOscMessage h2mess;
    h2mess.setAddress("/b_setn");
    h2mess.addIntArg(synthMod3BufNum.get() );
    h2mess.addIntArg(0 );
    h2mess.addIntArg(18 );
    for (int i = 0 ; i < 16; i++ ) {
        h2mess.addFloatArg( h2Mods.at(i) );
    }
    h2mess.addFloatArg(hNumber2);
    h2mess.addFloatArg(hPhase2);
    bundle.addMessage(h2mess);
    
    ofxOscMessage lfomess;
    lfomess.setAddress("/b_setn");
    lfomess.addIntArg(synthMod4BufNum.get() );
    lfomess.addIntArg(0 );
    lfomess.addIntArg(18 );
    for (int i = 0 ; i < 16; i++ ) {
        lfomess.addFloatArg( lfoMods.at(i) );
    }
    lfomess.addFloatArg((float)scLfoOption);
    lfomess.addFloatArg(lfoRate);
    bundle.addMessage(lfomess);
    
    
    ofxOscMessage volmess;
    volmess.setAddress("/b_setn");
    volmess.addIntArg(synthVolsBufnum.get() );
    volmess.addIntArg(0 );
    volmess.addIntArg(16 );
    for (int i = 0 ; i < 16; i++ ) {
        volmess.addFloatArg( scVolumes.at(i).get() * scMasterVolume );
    }
    bundle.addMessage(volmess);
    
    scOscSender.sendBundle(bundle);
    
    
}

void ofApp::sendVideoOscData() {
    
}



//--------------------------------------------------------------
void ofApp::sendVideoToSyphon() {
    
    
    //videoServer_1.publishTexture(&processedTex);
    
    //videoServer_2.publishTexture(&soundGradientTex);
    
    
}


//-------------------------------------------------------------------------------------------------------------------------


//::::	DATA TRANSFER


//-------------------------------------------------------------------------------------------------------------------------



void ofApp::transferManagerSettings() {
    
    
    
    
    if (managerChange) {
        manager.lock();
        
        manager.setStreams(pathToStreamsListFile);
        
        manager.setDuration(autoRecTime);
        manager.setInterRec(interRecTime);
        
        manager.setMaxLoadTime(maxLoadTime);
        manager.setMinDuration(minRecDuration);
        manager.setMaxFrameLate(maxFrameLate);
        manager.setFirstFrameLate(firstFrameLate);
        
        
        manager.setMaxNumFiles(maxNumFiles);
        
        manager.setMaxNumArchive(maxArchive);
        
        manager.keepErrorfile(keepErrorFile);
        
        if (removeRequested) {
            manager.removeRequest(movieToRemovePath, removeMessage);
            removeRequested = false;
        }
        
        recorderState = manager.state;
        currentStream = manager.currentStream;
        
        
        manager.unlock();
        
        managerChange = false;
    }
    
    else {
        manager.lock();
        recorderState = manager.state;
        currentStream = manager.currentStream;
        manager.unlock();
    }
    
    
    
    
    
}




//--------------------------------------------------------------
//------------------------FRAME LINE ETC ...--------------------
//--------------------------------------------------------------


void ofApp::frame(float x, float y, float width, float height, float stroke, ofColor color, bool fill) {
    
    
    if (fill) {
        ofFill();
    }
    else {
        ofNoFill();
    }
    
    ofSetLineWidth(stroke);
    
    ofSetColor(color);
    
    ofDrawRectangle(x, y, width, height);
    
}

void ofApp::line(float x1, float y1, float x2, float y2, float stroke, ofColor color) {
    
    ofNoFill();
    //ofDrawLine(x1, y1, x2, y2);
    ofVec2f pos1 = {x1, y1};
    ofVec2f pos2 = {x2, y2};
    ofPath line;
    
    line.setStrokeWidth(stroke);
    line.setColor(color);
    line.arc(pos1, 0, 0, 0, 0);
    line.arc(pos2, 0, 0, 0, 0);
    line.close();
    
    line.draw();
    
}

void ofApp::circle(float x, float y, float radius, float stroke, ofColor color, bool fill) {
    
    ofVec2f center = {x, y};
    
    ofPath circle;
    circle.setStrokeWidth(stroke);
    circle.setFilled(fill);
    circle.setColor(color);
    circle.setFillColor(color);
    circle.arc(center, radius, radius, 0, 360);
    circle.close();
    
    circle.draw();
    
    //ofDrawCircle(x, y, radius);
    
}




//-------------------------------------------------------------------------------------------------------------------------


//AUDIO STUFF


//-------------------------------------------------------------------------------------------------------------------------



void ofApp::refreshAudioDeviceList() {
    
    ofLogNotice() << "Refreshing audio devices list.";
    
    audioInputDeviceList = input.getInputDevicesList();
    audioInputDeviceIDs = input.getInputDevicesIDs();
    audioOutputDeviceList = output.getOuputDevicesList();
    audioOutputDeviceIDs = output.getOuputDevicesIDs();
    
    
    ofLogNotice() << "INPUTS : ";
    ofLogNotice() << ofToString( audioInputDeviceList );
    ofLogNotice() << "OUTPUTS : ";
    ofLogNotice() << ofToString( audioOutputDeviceList );
    
    
}


//--------------------------------------------------------------------------------------
void ofApp::startAudioUnits(int inputId, int extinputId, int outputId){
    /*
     input
     ID[193]  	Name[Built-in Input]
     ID[47]  	Name[Soundflower (2ch)]
     ID[58]  	Name[Soundflower (64ch)]
     
     output
     ID[200]  	Name[Built-in Output]
     ID[47]  	Name[Soundflower (2ch)]
     ID[58]  	Name[Soundflower (64ch)]
     ID[38]  	Name[2Flower]
     */
    
    
    
    
    if (inputId >= audioInputDeviceList.size() ) inputId = 0;
    
    if (extinputId >= audioInputDeviceList.size() ) extinputId = 0;
    
    if (outputId >= audioOutputDeviceList.size() ) outputId = 0;
    
    //Setup
    
    inputTap.setBufferLength(bufferSize);
    outputTap.setBufferLength(bufferSize);
    
    hpf.setup('aufx', 'hpas','appl');
    
    lpf.setup('aufx', 'lpas','appl');
    
    pitchshift.setup('aufx', 'tmpt','appl');
    
    compressor.setup('aufx', 'dcmp','appl');
    
    
    inputMixer.setInputBusCount(2);
    //cout << inputMixer.getParameterList() << endl;
    
    input.connectTo(inputTap);
    inputTap.connectTo(inputMixer, 0); extInput.connectTo(inputMixer, 1);
    inputMixer.connectTo(hpf);
    
    hpf.connectTo(lpf);
    lpf.connectTo(pitchshift);
    pitchshift.connectTo(compressor);
    compressor.connectTo(outputTap);
    outputTap.connectTo(output);
    
    input.start( audioInputDeviceIDs[inputId] );
    extInput.start( audioInputDeviceIDs[extinputId]);
    output.start( audioOutputDeviceIDs[outputId] );
    
    //cout << output.getParameterList() << endl;
    
    //inputMixer.printParameterList();
    //hpf.printParameterList();
    //lpf.printParameterList();
    pitchshift.printParameterList(); //[0] Pitch [-2400 : 2400 : 0]
    //compressor.printParameterList(); //[6] Master Gain [-40 : 40 : 0]
    
    /*
     aufx bpas appl  -  Apple: AUBandpass
     aufx dcmp appl  -  Apple: AUDynamicsProcessor
     aufx dely appl  -  Apple: AUDelay
     aufx dist appl  -  Apple: AUDistortion
     aufx filt appl  -  Apple: AUFilter
     aufx greq appl  -  Apple: AUGraphicEQ
     aufx hpas appl  -  Apple: AUHipass
     aufx hshf appl  -  Apple: AUHighShelfFilter
     aufx lmtr appl  -  Apple: AUPeakLimiter
     aufx lpas appl  -  Apple: AULowpass
     aufx lshf appl  -  Apple: AULowShelfFilter
     aufx mcmp appl  -  Apple: AUMultibandCompressor
     aufx mrev appl  -  Apple: AUMatrixReverb
     aufx nbeq appl  -  Apple: AUNBandEQ
     aufx nsnd appl  -  Apple: AUNetSend
     aufx nutp appl  -  Apple: AUNewPitch
     aufx pmeq appl  -  Apple: AUParametricEQ
     aufx raac appl  -  Apple: AURoundTripAAC
     aufx rogr appl  -  Apple: AURogerBeep
     aufx sdly appl  -  Apple: AUSampleDelay
     aufx tmpt appl  -  Apple: AUPitch
     
     aumx 3dem appl  -  Apple: AUSpatialMixer
     aumx 3dmx appl  -  Apple: AUMixer3D
     aumx mcmx appl  -  Apple: AUMultiChannelMixer
     aumx mspl appl  -  Apple: AUMultiSplitter
     aumx mxmx appl  -  Apple: AUMatrixMixer
     aumx smxr appl  -  Apple: AUMixer
     aufc conv appl  -  Apple: AUConverter
     aufc defr appl  -  Apple: AUDeferredRenderer
     aufc ipto appl  -  Apple: AUNotQuiteSoSimpleTime
     aufc merg appl  -  Apple: AUMerger
     aufc nutp appl  -  Apple: AUNewTimePitch
     aufc splt appl  -  Apple: AUSplitter
     aufc tmpt appl  -  Apple: AUTimePitch
     aufc vari appl  -  Apple: AUVarispeed
     auou ahal appl  -  Apple: AudioDeviceOutput
     auou def  appl  -  Apple: DefaultOutputUnit
     auou genr appl  -  Apple: GenericOutput
     auou sys  appl  -  Apple: SystemOutputUnit
     auou vpio appl  -  Apple: AUVoiceProcessor
     augn afpl appl  -  Apple: AUAudioFilePlayer
     augn nrcv appl  -  Apple: AUNetReceive
     augn sspl appl  -  Apple: AUScheduledSoundPlayer
     augn ttsp appl  -  Apple: AUSpeechSynthesis
     auol tmpt appl  -  Apple: AUTimePitch
     auol vari appl  -  Apple: AUVarispeed
     */
    
    ofLogNotice() << "[::::MAIN::::] AudioUnits Started";
    
    
}
//--------------------------------------------------------------------------------------

void ofApp::stopAudioUnits() {
    
    input.stop();
    extInput.stop();
    
    ofLogNotice() << "[::::MAIN::::] AudioUnits Stopped";
    
}
//--------------------------------------------------------------------------------------

void ofApp::loadAudioUnitsPresets(string pName){
    
    //    string datapath = ofFilePath::getAbsolutePath( ofToDataPath("") );
    //    cout << datapath << endl;
    
    inputMixer.setInputVolume( extInputVol, 1);
    hpf.loadCustomPresetAtPath(  datapath + "au/hpf/" + pName + ".aupreset" );
    lpf.loadCustomPresetAtPath(  datapath + "au/lpf/" + pName + ".aupreset" );
    pitchshift.loadCustomPresetAtPath(   datapath + "au/pitchshift/" + pName + ".aupreset" );
    compressor.loadCustomPresetAtPath(  datapath + "au/compressor/" + pName + ".aupreset" );
    
    
    compressor.setParameter( 6, kAudioUnitScope_Global, volume);
    
    //output.setParameter(AudioUnitParameterID property, AudioUnitScope scope,  volume);
    
}

//--------------------------------------------------------------------------------------
void ofApp::saveAudioUnitsPresets(string pName){
    
    
    hpf.saveCustomPresetAtPath(  datapath + "au/hpf/" + pName + ".aupreset" );
    lpf.saveCustomPresetAtPath(  datapath + "au/lpf/" + pName + ".aupreset" );
    pitchshift.saveCustomPresetAtPath(   datapath + "au/pitchshift/" + pName + ".aupreset" );
    compressor.saveCustomPresetAtPath(  datapath + "au/compressor/" + pName + ".aupreset" );
    
}

//--------------------------------------------------------------------------------------

void ofApp::updateAudioData() {
    
    
    sampleLeft = audioDataBuffer.getSample(0, 0);
    sampleRight = audioDataBuffer.getSample(0, 1);
    
    int offset = (1280 + frameCounter) % 1280;
    
    soundGraphPointsL.at(offset) = sampleLeft;
    soundGraphPointsR.at(offset) = sampleRight;
    
}


void ofApp::interpolateAudioData() {
    
    i_presencesL1 = interpolate( spectrum, aStartFreq, aEndFreq, cStartFreq, cEndFreq);
    i_presencesL2 = interpolate( melBands, mStartBand, mEndBand, cStartFreq, cEndFreq);
    i_brightnessesL = interpolate( *audioBrightnessesL, 0, audioBrightnessesL->size(), cStartFreq, cEndFreq);
    i_saturationsL = interpolate( *audioSaturationsL, 0, audioSaturationsL->size(), cStartFreq, cEndFreq);
    i_alphasL = interpolate( *audioAlphasL, 0, audioAlphasL->size(), cStartFreq, cEndFreq);
    
    
    if (stereoGradient) {
        i_presencesR1 = interpolate(spectrum2, aStartFreq, aEndFreq, cStartFreq, cEndFreq);
        i_presencesR2 = interpolate( melBands2, mStartBand, mEndBand, cStartFreq, cEndFreq);
        i_brightnessesR = interpolate( *audioBrightnessesR, 0, audioBrightnessesR-> size(), cStartFreq, cEndFreq);
        i_saturationsR = interpolate( *audioSaturationsR, 0, audioSaturationsR->size() , cStartFreq, cEndFreq);
        i_alphasR = interpolate( *audioAlphasR, 0, audioAlphasR->size(), cStartFreq, cEndFreq);
        
    }
    
}


void ofApp::selectAudioData() {
    
    
    ///{"POWER", "PITCH_FREQ", "PITCH_SALIENCE","HFC", "SPECTRAL_COMPLEXITY", "CENTROID", "DISSONANCE", "ROLL_OFF", "ODD_TO_EVEN", "STRONG_PEAKS", "STRONG_DECAY", "NONE"
    
    
    ///global gradient
    agBrightness = agGlobalModders.at(globalBrightness);
    agSaturation = agGlobalModders.at(globalSaturation);
    agAlpha = agGlobalModders.at(globalAlpha);
    agContrast = agGlobalModders.at(globalContrast);
    
    gradientBrightness = agBrightness * globalBrightnessAdjust + refBrightness;
    gradientSaturation = agSaturation * globalSaturationAdjust + refSaturation;
    gradientAlpha = agAlpha * globalAlphaAdjust + refAlpha;
    gradientContrast = agContrast * globalContrastAdjust + refContrast;
    
    ///audio video input
    avBMod = agGlobalModders.at(avBModOption);
    avSMod = agGlobalModders.at(avSModOption);
    avAMod = agGlobalModders.at(avAModOption);
    avCMod = agGlobalModders.at(avCModOption);
    
    avB = avBMod*avBModIntensity + audioVideoBrightness;
    avC = avCMod*avCModIntensity + audioVideoContrast;
    avS = avSMod*avSModIntensity + audioVideoSaturation;
    avA = avAMod*avAModIntensity + audioVideoAlpha;
    
    
    ///"SPECTRUM", "MEL_BANDS", "MFCC", "HPCP", "TRISTIMLUS", "NONE"
    
    audioPresencesL1 = &spectrumNorm;
    audioPresencesR1 = &spectrumNorm2;
    
    
    
    audioPresencesL2 = &melBandsNorm;
    audioPresencesR2 = &melBandsNorm2;
    
    
    switch(perBandBrightness){
        case 0:
            audioBrightnessesL = &spectrumNorm;
            audioBrightnessesR = &spectrumNorm2;
            break;
            
        case 1:
            audioBrightnessesL = &melBandsNorm;
            audioBrightnessesR = &melBandsNorm2;
            break;
            
        case 2:
            audioBrightnessesL = &mfccNorm;
            audioBrightnessesR = &mfccNorm2;
            break;
        case 3:
            audioBrightnessesL = &hpcpNorm;
            audioBrightnessesR = &hpcpNorm2;
            break;
        case 4:
            audioBrightnessesL = &tristimulusNorm;
            audioBrightnessesR = &tristimulusNorm2;
            break;
        case 5:
            audioBrightnessesL = &zeroFloats;
            audioBrightnessesR = &zeroFloats;
            break;
    }
    
    switch(perBandSaturation){
        case 0:
            audioSaturationsL = &spectrumNorm;
            audioSaturationsR = &spectrumNorm2;
            break;
            
        case 1:
            audioSaturationsL = &melBandsNorm;
            audioSaturationsR = &melBandsNorm2;
            break;
            
        case 2:
            audioSaturationsL = &mfccNorm;
            audioSaturationsR = &mfccNorm2;
            break;
        case 3:
            audioSaturationsL = &hpcpNorm;
            audioSaturationsR = &hpcpNorm2;
            break;
        case 4:
            audioSaturationsL = &tristimulusNorm;
            audioSaturationsR = &tristimulusNorm2;
            break;
        case 5:
            audioSaturationsL= &zeroFloats;
            audioSaturationsR = &zeroFloats;
            break;
    }
    
    
    switch(perBandAlpha){
        case 0:
            audioAlphasL = &spectrumNorm;
            audioAlphasR = &spectrumNorm2;
            break;
            
        case 1:
            audioAlphasL = &melBandsNorm;
            audioAlphasR = &melBandsNorm2;
            break;
            
        case 2:
            audioAlphasL = &mfccNorm;
            audioAlphasR = &mfccNorm2;
            break;
        case 3:
            audioAlphasL = &hpcpNorm;
            audioAlphasR = &hpcpNorm2;
            break;
        case 4:
            audioAlphasL = &tristimulusNorm;
            audioAlphasR = &tristimulusNorm2;
            break;
        case 5:
            audioAlphasL= &zeroFloats;
            audioAlphasR = &zeroFloats;
            break;
    }
    
    
    
    
}
//--------------------------------------------------------------------------------------
ofPixels ofApp::applyGradient() {
    
    ofPixels grad;
    ofPixels grad2;
    
    colorPoints.clear();
    colorPoints2.clear();
    
    
    for (int i = cStartFreq; i < cEndFreq; i++) {
        
        
        //mono
        int numPoints = (int)( 20*( i_presencesL1.at(i)*(1-perBandPresenceAdjust) + i_presencesL2.at(i)*perBandPresenceAdjust ) );
        float hue =    lookupHues.at(i);
        float brightness = (   i_brightnessesL.at(i)  )* 255.0f;
        float saturation = (   i_saturationsL.at(i)   )* 255.0f;
        float alpha = (   i_alphasL.at(i)   )* 255.0f;
        
        hue = ofClamp(hue, 0.0f, 255.0f);
        brightness = ofClamp(brightness, 0.0f, 255.0f);
        saturation = ofClamp(saturation, 0.0f, 255.0f);
        alpha = ofClamp(alpha, 0.0f, 255.0f);
        
        for (int j = 0; j < numPoints; j++) {
            colorPoints.push_back(ofVec4f(hue, saturation, brightness, alpha) );
        }
        
        
        //stereo
        if(stereoGradient ) {
            int numPoints2 = (int)( 20*(i_presencesR1.at(i)*(1-perBandPresenceAdjust) + i_presencesR2.at(i)*perBandPresenceAdjust) );
            float hue2 =    lookupHues.at(i);
            float brightness2 = (   i_brightnessesR.at(i)  )* 255.0f;;
            float saturation2 = (   i_saturationsR.at(i)   )* 255.0f;
            float alpha2 = (   i_alphasR.at(i)   )* 255.0f;
            
            hue2 = ofClamp(hue2, 0.0f, 255.0f);
            brightness2 = ofClamp(brightness2, 0.0f, 255.0f);
            saturation2 = ofClamp(saturation2, 0.0f, 255.0f);
            alpha2 = ofClamp(alpha2, 0.0f, 255.0f);
            
            for (int j = 0; j < numPoints2; j++) {
                colorPoints2.push_back(ofVec4f(hue2, saturation2, brightness2, alpha2) );
            }
        }
        
    }
    
    
    //mono
    grad.allocate(2, colorPoints.size(), OF_PIXELS_RGBA);
    for (int i = 0; i < colorPoints.size(); i++) {
        ofColor c;
        c.setHsb(colorPoints.at(i).x, colorPoints.at(i).y, colorPoints.at(i).z, colorPoints.at(i).w);
        grad.setColor(0, i, c);
        
    }
    
    grad.resize(2, 1080, OF_INTERPOLATE_NEAREST_NEIGHBOR);
    
    
    //stereo
    if ( stereoGradient ) {
        grad2.allocate(1, colorPoints2.size(), OF_PIXELS_RGBA);
        for (int i = 0; i < colorPoints2.size(); i++) {
            ofColor c;
            c.setHsb(colorPoints2.at(i).x, colorPoints2.at(i).y, colorPoints2.at(i).z, colorPoints2.at(i).w);
            grad2.setColor(0, i, c);
            
        }
        
        grad2.resize(1, 1080, OF_INTERPOLATE_NEAREST_NEIGHBOR);
        
        grad2.pasteInto(grad, 1, 0);
    }
    
    
    
    return grad;
    
    
}



//--------------------------------------------------------------------------------------



//ANALYZE LEFT SAMPLES
void ofApp::analyzeSamplesLeft( vector<float> samplesLeft ) {
    
    
    audioAnalyzerLeft.analyze(samplesLeft, 0, sampleRate);
    
    //-:get Values:
    rms     = audioAnalyzerLeft.getValue(RMS, 0, rmsSmooth);
    power   = audioAnalyzerLeft.getValue(POWER, 0, powerSmooth);
    pitchFreq = audioAnalyzerLeft.getValue(PITCH_FREQ, 0, pitchFreqSmooth);
    pitchConf = audioAnalyzerLeft.getValue(PITCH_CONFIDENCE, 0, pitchConfSmooth);
    pitchSalience  = audioAnalyzerLeft.getValue(PITCH_SALIENCE, 0, pitchSalienceSmooth);
    inharmonicity   = audioAnalyzerLeft.getValue(INHARMONICITY, 0, inharmonicitySmooth);
    hfc = audioAnalyzerLeft.getValue(HFC, 0, hfcSmooth);
    specComp = audioAnalyzerLeft.getValue(SPECTRAL_COMPLEXITY, 0, specCompSmooth);
    centroid = audioAnalyzerLeft.getValue(CENTROID, 0, centroidSmooth);
    rollOff = audioAnalyzerLeft.getValue(ROLL_OFF, 0, rollOffSmooth);
    oddToEven = audioAnalyzerLeft.getValue(ODD_TO_EVEN, 0, oddToEvenSmooth);
    strongPeak = audioAnalyzerLeft.getValue(STRONG_PEAK, 0, strongPeakSmooth);
    strongDecay = audioAnalyzerLeft.getValue(STRONG_DECAY, 0, strongDecaySmooth);
    
    //Normalized values for graphic meters:
    pitchFreqNorm   = audioAnalyzerLeft.getValue(PITCH_FREQ, 0, pitchFreqSmooth, TRUE);
    hfcNorm     = audioAnalyzerLeft.getValue(HFC, 0, hfcSmooth, TRUE);
    specCompNorm = audioAnalyzerLeft.getValue(SPECTRAL_COMPLEXITY, 0, specCompSmooth, TRUE);
    centroidNorm = audioAnalyzerLeft.getValue(CENTROID, 0, centroidSmooth, TRUE);
    rollOffNorm  = audioAnalyzerLeft.getValue(ROLL_OFF, 0, rollOffSmooth, TRUE);
    oddToEvenNorm   = audioAnalyzerLeft.getValue(ODD_TO_EVEN, 0, oddToEvenSmooth, TRUE);
    strongPeakNorm  = audioAnalyzerLeft.getValue(STRONG_PEAK, 0, strongPeakSmooth, TRUE);
    strongPeakNorm*= strongPeakAmp;
    strongDecayNorm = audioAnalyzerLeft.getValue(STRONG_DECAY, 0, strongDecaySmooth, TRUE);
    
    dissonance = audioAnalyzerLeft.getValue(DISSONANCE, 0, dissonanceSmooth);
    
    
    
    
    
    spectrum = audioAnalyzerLeft.getValues(SPECTRUM, 0, spectrumSmooth);
    
    for (int i = 0; i < spectrum.size(); i++){
        float scaledValue = ofMap(spectrum[i], DB_MIN, DB_MAX,0.0,1.0, true);
        //float scaledValue = ofMap(spectrum.at(i), DB_MIN, DB_MAX, 0, 1, true);
        
        float cFac = ofMap(i, 0, spectrum.size(), -1, 1);
        cFac = cFac * compensation;
        if (scaledValue > 0) spectrumNorm.at(i) = ofClamp(scaledValue+cFac, 0, 1) ;
        else spectrumNorm.at(i) = 0;
    }
    
    melBands = audioAnalyzerLeft.getValues(MEL_BANDS, 0, melBandsSmooth);
    for (int i = 0; i < melBands.size(); i++){
        float scaledValue = ofMap(melBands.at(i), DB_MIN, DB_MAX, 0, 1, true);
        melBandsNorm.at(i) = scaledValue;
    }
    
    mfcc = audioAnalyzerLeft.getValues(MFCC, 0, mfccSmooth);
    for (int i = 0; i < mfcc.size(); i++){
        float scaledValue = ofMap(mfcc.at(i), 0, MFCC_MAX_ESTIMATED_VALUE, 0, 1, true);
        mfccNorm.at(i) = scaledValue;
    }
    
    hpcp = audioAnalyzerLeft.getValues(HPCP, 0, hpcpSmooth);
    hpcpNorm = hpcp;
    
    tristimulus = audioAnalyzerLeft.getValues(TRISTIMULUS, 0, tristimulusSmooth);
    tristimulusNorm = tristimulus;
    
    //    multiPitches = audioAnalyzerLeft.getValues(MULTI_PITCHES, 0);//, multiPitchesSmooth);
    //    saliencePeaks = audioAnalyzerLeft.getSalienceFunctionPeaks(0, multiPitchesSmooth);
    //    multiPitchesSaliences.resize(12);//saliencePeaks.size() );
    //
    //    for (int i = 0; i < multiPitchesSaliences.size(); i++) {
    //        multiPitchesSaliences[i] = 0;
    //        //multiPitchesSaliences[i] = saliencePeaks[i].value;
    //    }
    
    isOnset = audioAnalyzerLeft.getOnsetValue(0);
    
    
    agGlobalModders.clear();
    agGlobalModders.push_back(power);
    agGlobalModders.push_back(pitchFreqNorm);
    agGlobalModders.push_back(pitchSalience);
    agGlobalModders.push_back(hfcNorm);
    agGlobalModders.push_back(specCompNorm);
    agGlobalModders.push_back(centroidNorm);
    agGlobalModders.push_back(dissonance);
    agGlobalModders.push_back(rollOffNorm);
    agGlobalModders.push_back(oddToEvenNorm);
    agGlobalModders.push_back(strongPeakNorm);
    agGlobalModders.push_back(strongDecayNorm);
    agGlobalModders.push_back(0.0f);
    
}

//---------------ANALYZE RIGHT SAMPLES
void ofApp::analyzeSamplesRight( vector<float> samplesRight) {
    
    audioAnalyzerRight.analyze(samplesRight, 0, sampleRate);
    
    
    //-:get Values:
    rms2     = audioAnalyzerRight.getValue(RMS, 0, rmsSmooth);
    power2   = audioAnalyzerRight.getValue(POWER, 0, powerSmooth);
    pitchFreq2 = audioAnalyzerRight.getValue(PITCH_FREQ, 0, pitchFreqSmooth);
    pitchConf2 = audioAnalyzerRight.getValue(PITCH_CONFIDENCE, 0, pitchConfSmooth);
    pitchSalience2  = audioAnalyzerRight.getValue(PITCH_SALIENCE, 0, pitchSalienceSmooth);
    inharmonicity2   = audioAnalyzerRight.getValue(INHARMONICITY, 0, inharmonicitySmooth);
    hfc2 = audioAnalyzerRight.getValue(HFC, 0, hfcSmooth);
    specComp2 = audioAnalyzerRight.getValue(SPECTRAL_COMPLEXITY, 0, specCompSmooth);
    centroid2 = audioAnalyzerRight.getValue(CENTROID, 0, centroidSmooth);
    rollOff2 = audioAnalyzerRight.getValue(ROLL_OFF, 0, rollOffSmooth);
    oddToEven2 = audioAnalyzerRight.getValue(ODD_TO_EVEN, 0, oddToEvenSmooth);
    strongPeak2 = audioAnalyzerRight.getValue(STRONG_PEAK, 0, strongPeakSmooth);
    strongDecay2 = audioAnalyzerRight.getValue(STRONG_DECAY, 0, strongDecaySmooth);
    
    //Normalized values for graphic meters:
    pitchFreqNorm2   = audioAnalyzerRight.getValue(PITCH_FREQ, 0, pitchFreqSmooth, TRUE);
    hfcNorm2     = audioAnalyzerRight.getValue(HFC, 0, hfcSmooth, TRUE);
    specCompNorm2 = audioAnalyzerRight.getValue(SPECTRAL_COMPLEXITY, 0, specCompSmooth, TRUE);
    centroidNorm2 = audioAnalyzerRight.getValue(CENTROID, 0, centroidSmooth, TRUE);
    rollOffNorm2  = audioAnalyzerRight.getValue(ROLL_OFF, 0, rollOffSmooth, TRUE);
    oddToEvenNorm2   = audioAnalyzerRight.getValue(ODD_TO_EVEN, 0, oddToEvenSmooth, TRUE);
    strongPeakNorm2  = audioAnalyzerRight.getValue(STRONG_PEAK, 0, strongPeakSmooth, TRUE);
    strongPeakNorm2 *= strongPeakAmp;
    strongDecayNorm2 = audioAnalyzerRight.getValue(STRONG_DECAY, 0, strongDecaySmooth, TRUE);
    
    dissonance2 = audioAnalyzerRight.getValue(DISSONANCE, 0, dissonanceSmooth);
    
    
    
    
    
    spectrum2 = audioAnalyzerRight.getValues(SPECTRUM, 0, spectrumSmooth);
    for (int i = 0; i < spectrum2.size(); i++){
        float scaledValue = ofMap(spectrum2.at(i), DB_MIN, DB_MAX, 0, 1, true);
        //spectrumNorm2.at(i) = scaledValue;
        
        float cFac = ofMap(i, 0, spectrum2.size(), -1, 1);
        cFac = cFac * compensation;
        if (scaledValue > 0) spectrumNorm2.at(i) = ofClamp(scaledValue+cFac, 0, 1) ;
        else spectrumNorm2.at(i) = 0;
    }
    
    melBands2 = audioAnalyzerRight.getValues(MEL_BANDS, 0, melBandsSmooth);
    for (int i = 0; i < melBands2.size(); i++){
        float scaledValue = ofMap(melBands2.at(i), DB_MIN, DB_MAX, 0, 1, true);
        melBandsNorm2.at(i) = scaledValue;
    }
    
    mfcc2 = audioAnalyzerRight.getValues(MFCC, 0, mfccSmooth);
    for (int i = 0; i < mfcc2.size(); i++){
        float scaledValue = ofMap(mfcc2.at(i), 0, MFCC_MAX_ESTIMATED_VALUE, 0, 1, true);
        mfccNorm2.at(i) = scaledValue;
    }
    
    hpcp2 = audioAnalyzerRight.getValues(HPCP, 0, hpcpSmooth);
    for (int i = 0; i < hpcp2.size(); i++){
        float scaledValue = ofMap(hpcp2.at(i), DB_MIN, DB_MAX, 0, 1);
        hpcpNorm2.at(i) = scaledValue;
    }
    
    tristimulus2 = audioAnalyzerRight.getValues(TRISTIMULUS, 0, tristimulusSmooth);
    for (int i = 0; i < tristimulus2.size(); i++){
        float scaledValue = ofMap(tristimulus2.at(i), DB_MIN, DB_MAX, 0, 1);
        tristimulusNorm2.at(i) = scaledValue;
    }
    
    //    multiPitches2 = audioAnalyzer.getValues(MULTI_PITCHES, 1);//, multiPitchesSmooth);
    //    saliencePeaks2 = audioAnalyzer.getSalienceFunctionPeaks(1, multiPitchesSmooth);
    //    multiPitchesSaliences2.resize(12);//saliencePeaks.size() );
    //
    //    for (int i = 0; i < multiPitchesSaliences2.size(); i++) {
    //        multiPitchesSaliences2[i] = 1;
    //        //multiPitchesSaliences[i] = saliencePeaks[i].value;
    //    }
    
    isOnset2 = audioAnalyzerRight.getOnsetValue(0);
    
    
}



//-------------------------------------------------------------------------------------------------------------------------


//AUDIO DRAWINGS


//-------------------------------------------------------------------------------------------------------------------------


void ofApp::drawAudioData( int x, int y, int width, int height) {
    
    
    int mw = 250;
    int graphH = 75;
    int xpos = 20;
    int ypos = 20;
    int pady = 50;
    int padx = 350;
    
    ofFill();
    
    
    ofPushMatrix();
    ofTranslate(x - xpos, y - ypos);
    
    /*
     float value, valueNorm;
     
     //	ofSetColor(audioFontColor);
     //	value = rms;
     string strValue = "RMS: " + ofToString(value, 2);
     //	ofDrawBitmapString(strValue, xpos, ypos);
     //	ofSetColor(audioColor);
     //	ofDrawRectangle(xpos, ypos + 5, value * mw, 10);
     
     //	ypos += pady;
     ofSetColor(audioFontColor);
     value = power;
     strValue = "Power: " + ofToString(value, 2);
     ofDrawBitmapString(strValue, xpos, ypos);
     ofSetColor(audioColor);
     ofDrawRectangle(xpos, ypos + 5, value * mw, 10);
     
     ypos += pady;
     ofSetColor(audioFontColor);
     value = pitchFreq;
     valueNorm = pitchFreqNorm;
     strValue = "Pitch Frequency: " + ofToString(value, 2) + " hz.";
     ofDrawBitmapString(strValue, xpos, ypos);
     ofSetColor(audioColor);
     ofDrawRectangle(xpos, ypos + 5, valueNorm * mw, 10);
     
     ypos += pady;
     ofSetColor(audioFontColor);
     value = pitchConf;
     strValue = "Pitch Confidence: " + ofToString(value, 2);
     ofDrawBitmapString(strValue, xpos, ypos);
     ofSetColor(audioColor);
     ofDrawRectangle(xpos, ypos + 5, value * mw, 10);
     
     ypos += pady;
     ofSetColor(audioFontColor);
     value = pitchSalience;
     strValue = "Pitch Salience: " + ofToString(value, 2);
     ofDrawBitmapString(strValue, xpos, ypos);
     ofSetColor(audioColor);
     ofDrawRectangle(xpos, ypos + 5, value * mw, 10);
     
     //	ypos += pady;
     //	ofSetColor(audioFontColor);
     //	value = inharmonicity;
     //	strValue = "Inharmonicity: " + ofToString(value, 2);
     //	ofDrawBitmapString(strValue, xpos, ypos);
     //	ofSetColor(audioColor);
     //	ofDrawRectangle(xpos, ypos + 5, value * mw, 10);
     
     ypos += pady;
     ofSetColor(audioFontColor);
     value = hfc;
     valueNorm = hfcNorm;
     strValue = "HFC: " + ofToString(value, 2);
     ofDrawBitmapString(strValue, xpos, ypos);
     ofSetColor(audioColor);
     ofDrawRectangle(xpos, ypos + 5, valueNorm * mw, 10);
     
     ypos += pady;
     ofSetColor(audioFontColor);
     value = specComp;
     valueNorm = specCompNorm;
     strValue = "Spectral Complexity: " + ofToString(value, 2);
     ofDrawBitmapString(strValue, xpos, ypos);
     ofSetColor(audioColor);
     ofDrawRectangle(xpos, ypos + 5, valueNorm * mw, 10);
     
     ypos += pady;
     ofSetColor(audioFontColor);
     value = centroid;
     valueNorm = centroidNorm;
     strValue = "Centroid: " + ofToString(value, 2);
     ofDrawBitmapString(strValue, xpos, ypos);
     ofSetColor(audioColor);
     ofDrawRectangle(xpos, ypos + 5, valueNorm * mw, 10);
     
     ypos += pady;
     ofSetColor(audioFontColor);
     value = dissonance;
     strValue = "Dissonance: " + ofToString(value, 2);
     ofDrawBitmapString(strValue, xpos, ypos);
     ofSetColor(audioColor);
     ofDrawRectangle(xpos, ypos + 5, value * mw, 10);
     
     ypos += pady;
     ofSetColor(audioFontColor);
     value = rollOff;
     valueNorm = rollOffNorm;
     strValue = "Roll Off: " + ofToString(value, 2);
     ofDrawBitmapString(strValue, xpos, ypos);
     ofSetColor(audioColor);
     ofDrawRectangle(xpos, ypos + 5, valueNorm * mw , 10);
     
     ypos += pady;
     ofSetColor(audioFontColor);
     value = oddToEven;
     valueNorm = oddToEvenNorm;
     strValue = "Odd To Even Harmonic Energy : " + ofToString(value, 2);
     ofDrawBitmapString(strValue, xpos, ypos);
     ofSetColor(audioColor);
     ofDrawRectangle(xpos, ypos + 5, valueNorm * mw, 10);
     
     ypos += pady;
     ofSetColor(audioFontColor);
     value = strongPeak;
     valueNorm = strongPeakNorm;
     strValue = "Strong Peak: " + ofToString(value, 2);
     ofDrawBitmapString(strValue, xpos, ypos);
     ofSetColor(audioColor);
     ofDrawRectangle(xpos, ypos + 5, valueNorm * mw, 10);
     
     ypos += pady;
     ofSetColor(audioFontColor);
     value = strongDecay;
     valueNorm = strongDecayNorm;
     strValue = "Strong Decay: " + ofToString(value, 2);
     ofDrawBitmapString(strValue, xpos, ypos);
     ofSetColor(audioColor);
     ofDrawRectangle(xpos, ypos + 5, valueNorm * mw, 10);
     
     ypos += pady;
     ofSetColor(audioFontColor);
     value = isOnset;
     strValue = "Onsets: " + ofToString(value);
     ofDrawBitmapString(strValue, xpos, ypos);
     ofSetColor(audioColor);
     ofDrawRectangle(xpos, ypos + 5, value * mw, 10);
     
     ofPopMatrix();
     */
    
    
    
    
    
    //-Vector Values Algorithms:
    
    ofFill();
    
    ofPushMatrix();
    
    ofTranslate(padx, 0);
    
    
    int yoffset = graphH + pady;
    ypos = y;
    
    ofSetColor(audioFontColor);
    ofDrawBitmapString("Spectrum: ", 0, ypos);
    ofPushMatrix();
    ofTranslate(0, ypos);
    ofSetColor(audioColor);
    float bin_w = (float) mw / spectrum.size();
    for (int i = 0; i < spectrum.size(); i++) {
        float scaledValue = ofMap(spectrum[i], DB_MIN, DB_MAX, 0.0, 1.0, true);//clamped value
        float bin_h = -1 * (scaledValue * graphH);
        ofDrawRectangle(i * bin_w, graphH, bin_w, bin_h);
    }
    float lx1 = ofMap(aStartFreq.get(), 0, 1023, 0, mw);
    float lx2 = ofMap(aEndFreq.get(), 0, 1023, 0, mw);
    
    line(lx1, y + graphH, lx1, y, 1, overAudioColor);
    line(lx2, y + graphH, lx2, y, 1, overAudioColor);
    
    string f1 = ofToString( ofMap(aStartFreq, 0, 1023, 20, 22050) )+ "Hz";
    string f2 = ofToString( ofMap(aEndFreq, 0, 1023, 20, 22050) )+ "Hz";
    
    ofDrawBitmapString(f1, lx1-10, y+graphH);
    ofDrawBitmapString(f2, lx2-10, y+graphH);
    
    ofPopMatrix();
    
    
    
    
    
    /*
     ypos += yoffset;
     ofSetColor(audioFontColor);
     ofDrawBitmapString("Mel Bands: ", 0, ypos);
     ofPushMatrix();
     ofTranslate(0, ypos);
     ofSetColor(audioColor);
     bin_w = (float) mw / melBands.size();
     for (int i = 0; i < melBands.size(); i++) {
     float scaledValue = ofMap(melBands[i], DB_MIN, DB_MAX, 0.0, 1.0, true);//clamped value
     float bin_h = -1 * (scaledValue * graphH);
     ofDrawRectangle(i * bin_w, graphH, bin_w, bin_h);
     }
     ofPopMatrix();
     
     
     ypos += yoffset;
     ofSetColor(audioFontColor);
     ofDrawBitmapString("MFCC: ", 0, ypos);
     ofPushMatrix();
     ofTranslate(0, ypos);
     ofSetColor(audioColor);
     bin_w = (float) mw / mfcc.size();
     for (int i = 0; i < mfcc.size(); i++) {
     float scaledValue = ofMap(mfcc[i], 0, MFCC_MAX_ESTIMATED_VALUE, 0.0, 1.0, true);//clamped value
     float bin_h = -1 * (scaledValue * graphH);
     ofDrawRectangle(i * bin_w, graphH, bin_w, bin_h);
     }
     ofPopMatrix();
     
     ypos += yoffset;
     ofSetColor(audioFontColor);
     ofDrawBitmapString("HPCP: ", 0, ypos);
     ofPushMatrix();
     ofTranslate(0, ypos);
     ofSetColor(audioColor);
     bin_w = (float) mw / hpcp.size();
     for (int i = 0; i < hpcp.size(); i++) {
     //float scaledValue = ofMap(hpcp[i], DB_MIN, DB_MAX, 0.0, 1.0, true);//clamped value
     float scaledValue = hpcp[i];
     float bin_h = -1 * (scaledValue * graphH);
     ofDrawRectangle(i * bin_w, graphH, bin_w, bin_h);
     }
     ofPopMatrix();
     
     //TRISTIMULS
     ypos += yoffset;
     ofSetColor(audioFontColor);
     ofDrawBitmapString("Tristimulus: ", 0, ypos);
     ofPushMatrix();
     ofTranslate(0, ypos);
     ofSetColor(audioColor);
     bin_w = (float) mw / tristimulus.size();
     for (int i = 0; i < tristimulus.size(); i++) {
     //float scaledValue = ofMap(hpcp[i], DB_MIN, DB_MAX, 0.0, 1.0, true);//clamped value
     float scaledValue = tristimulus[i];
     float bin_h = -1 * (scaledValue * graphH);
     ofDrawRectangle(i * bin_w, graphH, bin_w, bin_h);
     }
     ofPopMatrix();
     
     
     //MULTIPITCHES
     ypos += yoffset;
     ofSetColor(audioFontColor);
     ofDrawBitmapString("Multi Pitches: ", 0, ypos);
     ofPushMatrix();
     ofTranslate(0, ypos);
     ofSetColor(audioColor);
     bin_w = (float) mw / saliencePeaks.size() + 20;
     for (int i = 0; i < saliencePeaks.size(); i++) {
     //float scaledValue = ofMap(multiPitches[i], DB_MIN, DB_MAX, 0.0, 1.0, true);//clamped value
     //float pitchVal = -1 * multiPitches[i] / 2000.0;
     //float pitchVal = 1;
     float pitchVal = -1 * saliencePeaks[i].value;
     ofDrawRectangle(i * bin_w, graphH, bin_w, pitchVal * graphH);
     
     ofDrawBitmapString(ofToString(saliencePeaks[i].bin) + "c", i * bin_w, graphH + 10);
     }
     ofPopMatrix();
     
     //debug = ofToString(multiPitches);
     */
    
    
    
    ofPopMatrix();
    
    
    
    
}




void ofApp::drawSoundSpectrum(float x, float y, float w, float h) {
    
    ofSetColor(subAudioColor);
    ofDrawLine(x, y + h, x+w, y + h );
    
    ofSetColor(audioColor);
    
    float bin_w = (float) w / spectrum.size();
    
    
    for (int i = 0; i < spectrum.size(); i++) {
        float scaledValue = spectrumNorm.at(i);//ofMap(spectrum[i], DB_MIN, DB_MAX,0.0,1.0, true);         //scaledValue = pow(10, spectrum[i]/20 );
        float bin_h = -1 * (scaledValue * h);
        ofDrawRectangle(x + i * bin_w, y + h, bin_w, bin_h);
    }
    
    
}


//--------------------------------------------------------------
//------------------------FIRST AUDIO DRAWING-------------------
//--------------------------------------------------------------

void ofApp::drawSoundGraph(float x, float y, float width, float height) {
    
    soundGraphTex.draw(x, y, width, height);
    
    
}

//--------------------------------------------------------------

void ofApp::drawSoundGraphPoints(float x, float y, float width, float height) {
    
    int offset = (1280 + frameCounter) % 1280;;
    
    ofSetColor(audioColor);
    
    ofSetLineWidth(1);
    
    ofNoFill();
    
    glPushMatrix();
    
    glTranslatef(x, y, 0);
    
    ofBeginShape();
    for (int i = 0; i < soundGraphPointsL.size(); i++) {
        
        int index = (i + offset) % soundGraphPointsL.size();
        float x1 = ofMap(i, 0, soundGraphPointsL.size(), 0, width);
        float y1 = ofMap(soundGraphPointsL.at(index), -1, 1, 0, height/2);
        ofVertex(x1, y1);
    }
    ofEndShape();
    
    
    ofBeginShape();
    for (int i = 0; i < soundGraphPointsR.size(); i++) {
        
        int index = (i + offset) % soundGraphPointsR.size();
        float x2 = ofMap(i, 0, soundGraphPointsR.size(), 0, width);
        float y2 = ofMap(soundGraphPointsR.at(index), -1, 1, 0, height/2);
        ofVertex(x2, y2 + height / 2);
    }
    ofEndShape();
    
    glPopMatrix();
    
    frame(x, y, width, -height, stroke, frameColor, false);
    
}

//--------------------------------------------------------------

void ofApp::drawWaveForm(float x, float y, float width, float height) {
    
    ofPolyline L = inputTap.getLeftWaveform(width, height/2);
    ofPolyline R = inputTap.getLeftWaveform(width, height/2);
    
    ofSetColor(audioColor);
    ofNoFill();
    ofPushMatrix();
    ofTranslate(x, y);
    L.draw();
    ofTranslate(0, height/2);
    R.draw();
    ofPopMatrix();
    
    ofDrawRectangle(x, y, width, height);
    ofDrawRectangle(x, y, width, height/2);
}



//--------------------------------------------------------------

void ofApp::drawRMS(float x, float y, float width, float height) {
    
    ofSetColor(audioColor);
    ofFill();
    ofDrawRectangle(x, y, width, -height * testRms);
    
    frame(x, y, width, -height, stroke, frameColor, false);
    
}








//-------------------------------------------------------------------------------------------------------------------------


//::::AUDIO OSC


//-------------------------------------------------------------------------------------------------------------------------

void ofApp::sendAudioOscData() {
    
    ofxOscBundle b;
    
    
    ofxOscMessage rmsMess;
    rmsMess.setAddress("/Analyse/rms");
    rmsMess.addFloatArg(rms);
    
    ofxOscMessage specCompMess;
    specCompMess.setAddress("/Analyse/Spectral_Complexity");
    specCompMess.addFloatArg(specComp);
    
    
    b.addMessage(rmsMess);
    b.addMessage(specCompMess);
    
    audioOscSender.sendBundle(b);
    
}



//-------------------------------------------------------------------------------------------------------------------------


//::::GLOBAL DRAW


//-------------------------------------------------------------------------------------------------------------------------




void ofApp::drawVideo() {
    
    ofPushStyle();
    ofBackground(backgroundColor);
    float width = ofGetWidth()/3-15;
    float height = ofGetHeight()/4-15;
    int x = 420;
    int y = height + 30;
    
    drawXyPositions(x, y, width, height);
    
    
    y+=height/2+30;
    drawHisto(x, y, width, height/2);
    
    
    y+=height/2+30;
    drawLuSats(x, y, width, height/2);
    
    y = 15;
    x+= width + 30 + 100;
    drawSource( x, y +15 , width-width/4, height);
    
    y += height + 30;
    drawProcessed( x, y+15 , width-width/4, height);
    
    x = 420;
    y+= height + 70;
    drawIFFT(x, y, width, height*5/3);
    
    //y+= height + 30;
    //drawMidi(x, y, width, height/2);
    
    x+=width+30;
    drawSC(x, y, width, height/2);
    
    //colorSpectrumTex.draw(x, y, videoWidth, videoHeight/2 );
    
    ofPopStyle();
}

void ofApp::drawVideoAnalysis(int x, int y, int w,  int h){
    
    int cols = 3;
    int pad = 15;
    
    ofPushStyle();
    ofBackground(backgroundColor);
    float width = (w - ((cols+1)*pad) ) /cols;
    float height = 2*width/3;
    int xx = 15;
    int yy = 20;
    
    
    
    drawSource( xx, yy, width, height);
    xx += width + 15;
    drawProcessed( xx, yy , width, height);
    xx += width + 15;
    drawXyPositions(xx, yy+height, width, height);
    
    
    xx= 15;
    yy+=height/2 + 100;
    drawHisto(xx, yy + height/2, width*3 + 2*pad - 95, height/2);
    
    yy+=height/2 + 30;
    drawLuSats(xx, yy + height/2, width*3 + 2*pad, height/2);
    
    yy+= + 30;
    drawIFFT(xx, yy + height/2, width*3 + 2*pad, height);
    
    
    //yy += height/2 + 30;
    drawSC(xx, yy + height/2, width*3 + 2*pad, height/2);
    
    //colorSpectrumTex.draw(x, y, videoWidth, videoHeight/2 );
    ofPopStyle();
    
    
}


void ofApp::drawAudio() {
    
    ofBackground(backgroundColor);
    
    ofPushStyle();
    
    
    
    float vw = (float)(ofGetWidth()-40)/3;
    float vh = vw / 1.6f;
    
    float x = vw;
    float y = 15;
    
    
    //SOUND SPECTRUM
    ofSetColor(audioFontColor);
    ofDrawBitmapString("Sound Spectrum", x , y);
    
    y+=15;
    drawSoundSpectrum(x, y, vw -20, (int)vh/3.0f);
    
    
    float lx1 = ofMap(aStartFreq.get(), 0, 1023, 0, vw-20);
    float lx2 = ofMap(aEndFreq.get(), 0, 1023, 0, vw-20);
    
    line(x + lx1, y, x + lx1, y + vh/3.0f, 1, overAudioColor);
    line(x + lx2, y, x + lx2, y + vh/3.0f, 1, overAudioColor);
    
    y+=15;
    string f1 = ofToString( ofMap(aStartFreq, 0, 1023, 20, 22050) )+ " Hz";
    string f2 = ofToString( ofMap(aEndFreq, 0, 1023, 20, 22050) )+ " Hz";
    
    ofSetColor(audioFontColor);
    ofDrawBitmapString(f1, x + lx1-10, y + vh/3.0f);
    ofDrawBitmapString(f2, x + lx2-10, y + vh/3.0f + 15);
    
    
    
    
    
    //COLOR SPECTRUM
    y+= vh/3.0f + 50;
    ofSetColor(audioFontColor);
    ofDrawBitmapString("Color Spectrum", x , y);
    
    y+=15;
    colorSpectrumTex.draw(x, y, vw-20, (int)vh/4.0f );
    
    
    
    //y+=10;
    lx1 = ofMap(cStartFreq.get(), 0, 1023, 0, vw-20);
    lx2 = ofMap(cEndFreq.get(), 0, 1023, 0, vw-20);
    line(x + lx1, y, x + lx1, y + vh/4.0f, 1, frameColor);
    line(x + lx2, y, x + lx2, y + vh/4.0f, 1, frameColor);
    
    f1 = ofToString( ofMap(cStartFreq, 0, 1023, 380, 850) )+ " THz";
    f2 = ofToString( ofMap(cEndFreq, 0, 1023, 380, 850) )+ " THz";
    
    ofDrawBitmapString(f1, x + lx1-10, y + vh/4.0f + 15);
    ofDrawBitmapString(f2, x + lx2-10, y + vh/4.0f + 30);
    
    
    
    
    
    
    //PRESENCES & HUES
    y += vh/4.0f + 60;
    ofSetColor(audioFontColor);
    ofDrawBitmapString("Hues presence modulation", x , y);
    y+= 15;
    drawHues(x, y, vw-20, (int)vh/8.0f);
    
    //BRIGHTNESSES
    y += vh/8.0f + 60;
    ofSetColor(audioFontColor);
    ofDrawBitmapString("Brightnesses modulation", x , y);
    y+= 15;
    drawBrightnesses(x, y, vw-20, (int)vh/8.0f);
    
    //SATURATIONS
    y += vh/8.0f + 60;
    ofSetColor(audioFontColor);
    ofDrawBitmapString("Saturations modulation", x , y);
    y+= 15;
    drawSaturations(x, y, vw-20, (int)vh/8.0f);
    
    //ALPHAS
    y += vh/8.0f + 60;
    ofSetColor(audioFontColor);
    ofDrawBitmapString("Alphas parameter modulation", x , y);
    y+= 15;
    drawAlphas(x, y, vw-20, (int)vh/8.0f);
    
    
    
    
    
    
    x += vw + 30 ;
    
    // VIDEO PREVIEW
    y =15;
    ofSetColor(audioFontColor);
    ofDrawBitmapString("Video Preview", x , y);
    y +=15;
    audioVideoFbo.draw(x, y, vw/2, vh/2);
    
    // SOUND GRADIENT
    y += vh/2 + 15;
    ofSetColor(audioFontColor);
    ofDrawBitmapString("Gradient", x , y);
    y+=15;
    ofSetColor(ofColor::black);
    ofDrawRectangle(x, y, vw, vh);
    ofSetColor(255);
    gradientFbo.draw(x, y, vw, vh);
    
    
    
    
    // BLEND
    
    y+=15 + vh;
    ofSetColor(audioFontColor);
    ofDrawBitmapString("Blend", x , y);
    y+=15;
    drawBroadcast( x, y , vw, vh);
    
    
    ofPopStyle();
    
    
}




void ofApp::drawPresences(int x, int y, int w, int h){
    
    float wi =  (float)w/soundGradientPix.getHeight() ;
    
    for (int i = 0; i < soundGradientPix.getHeight(); i++) {
        
        ofColor(c);
        
        float hue = colorSpectrum.getColor(0, i).getHue();
        
        c.setHsb(hue, 255.0f, 255.0f);
        //ofFill();
        ofSetColor(c);
        ofDrawRectangle( x + wi*i, y+h , wi, -h);
        
    }
    
}

void ofApp::drawHues(int x, int y, int w, int h){
    
    float wi =  (float)w/soundGradientPix.getHeight() ;
    
    for (int i = 0; i < soundGradientPix.getHeight(); i++) {
        ofColor(c);
        float hue = soundGradientPix.getColor(0, i).getHue();
        c.setHsb(hue, 255.0f, 255.0f);
        //ofFill();
        ofSetColor(c);
        ofDrawRectangle( x + wi*i, y+h , wi, -h);
        
    }
    
    
}


void ofApp::drawSaturations(int x, int y, int w, int h){
    
    float wi =  (float)w/soundGradientPix.getHeight() ;
    
    for (int i = 0; i < soundGradientPix.getHeight(); i++) {
        ofColor(c);
        float sat = soundGradientPix.getColor(0, i).getSaturation();
        c.setHsb(0.0f, sat, 128.0f);
        //ofFill();
        ofSetColor(c);
        ofDrawRectangle( x + wi*i, y+h , wi, -h);
        
    }
}


void ofApp::drawBrightnesses(int x, int y, int w, int h){
    
    
    float wi =  (float)w/soundGradientPix.getHeight() ;
    
    for (int i = 0; i < soundGradientPix.getHeight(); i++) {
        ofColor(c);
        float b = soundGradientPix.getColor(0, i).getBrightness();
        c.setHsb(0.0f, 0.0f, b);
        //ofFill();
        ofSetColor(c);
        ofDrawRectangle( x + wi*i, y+h , wi, -h);
        
    }
    
    
    
}


void ofApp::drawAlphas(int x, int y, int w, int h){
    ofFill();
    //ofSetColor(ofColor::gray);
    //ofDrawRectangle(x, y, w, h);
    float wi =  (float)w/soundGradientPix.getHeight() ;
    
    for (int i = 0; i < soundGradientPix.getHeight(); i++) {
        ofColor(c);
        c = soundGradientPix.getColor(0, i);
        float a = c.a;
        c.setHsb(0.0f, 255.0f, 255.0f, a);
        //ofFill();
        ofSetColor(c);
        ofDrawRectangle( x + wi*i, y+h , wi, -h);
        
    }
    
    
}


void ofApp::drawPresencesChoice(int x, int y, int w, int h){
    
    int s = melBandsNorm.size();
    float ww = w/s;
    
    ofFill();
    ofSetColor(audioColor);
    
    for  (int i = 0; i < s; i++) {
        
        ofDrawRectangle(x +(ww*i), y+h, ww, -h * melBandsNorm.at(i) );
        
    }
    
    frame(x, y, w, h, stroke, frameColor, false);
    
}


void ofApp::drawBrightnessesChoice(int x, int y, int w, int h){
    
    int s = audioBrightnessesL -> size();
    float ww = w/s;
    
    ofFill();
    ofSetColor(audioColor);
    for  (int i = 0; i < s; i++) {
        
        ofDrawRectangle(x +(ww*i), y+h, ww, -h * audioBrightnessesL->at(i) );
        
    }
    
    frame(x, y, w, h, stroke, frameColor, false);
    
}

void ofApp::drawSaturationsChoice(int x, int y, int w, int h){
    
    int s = audioSaturationsL -> size();
    float ww = w/s;
    
    ofFill();
    ofSetColor(audioColor);
    
    for  (int i = 0; i < s ; i++) {
        
        ofDrawRectangle(x +(ww*i), y+h, ww, -h * audioSaturationsL->at(i) );
        
    }
    
    frame(x, y, w, h, stroke, frameColor, false);
    
    
}
void ofApp::drawAlphasChoice(int x, int y, int w, int h){
    
    
    int s = audioAlphasL -> size();
    float ww = w/s;
    
    ofFill();
    ofSetColor(audioColor);
    ofFill();
    for  (int i = 0; i < s; i++) {
        
        ofDrawRectangle(x +(ww*i), y+h, ww, -h * audioAlphasL->at(i) );
        
    }
    
    frame(x, y, w, h, stroke, frameColor, false);
    
    
}




void ofApp::drawBroadcast(float x, float y, float width, float height) {
    
    ofPushStyle();
    
    ofSetColor(ofColor::black);
    ofDrawRectangle(x, y, width, height);
    
    ofSetColor(255);
    
    broadcastFbo.draw(x, y, width, height);
    
    ofSetColor(ofColor::black);
    ofDrawRectangle(x, y, width/2*audioVideoCrop.get(), height);
    ofDrawRectangle(x+width, y, -width/2*audioVideoCrop.get(), height);
    
    ofSetColor(ofColor::white);
    if (showFrameRate && viewName.get() == "Broadcast") ofDrawBitmapString( ofToString(ofGetFrameRate() ), 50, 50);
    
    ofPopStyle();
    
}



//-------------------------------------------------------------------------------------------------------------------------


//::::	AUTOMATIONS


//-------------------------------------------------------------------------------------------------------------------------






//--------------------------------------------------------------

void ofApp::changeFile(int iD) {
    
    refreshPlaylist();
    
    if (isFiles) {
        fileRequested = true;
        nextMovieLoaded = false;
        
        nextiD = iD % (playlist.size() );
        
        string nextPath = playlist.at(nextiD);
        
        //moviePlayer.at( (playerSelector + 1 ) % 2 ).setPlayer( ofPtr<ofBaseVideoPlayer>(new ofQTKitPlayer)  );
        moviePlayer.at( (playerSelector + 1 ) % 2 ).close();
        moviePlayer.at( (playerSelector + 1 ) % 2 ) = ofVideoPlayer();
        moviePlayer.at( (playerSelector + 1 ) % 2 ).setLoopState(OF_LOOP_NORMAL);
        moviePlayer.at( (playerSelector + 1 ) % 2 ).setPixelFormat(OF_PIXELS_NATIVE);
        moviePlayer.at( (playerSelector + 1 ) % 2 ).setUseTexture(true);
        moviePlayer.at( (playerSelector + 1 ) % 2 ).load( nextPath);
        
        ofLogNotice() << "\n";
        ofLogNotice() << "Attempt to load : " << nextPath;
        ofLogNotice() << "\n";
        
    }
    
    else {
        ofLogNotice() << "\n";
        ofLogNotice() << "Changefile requested but directory is empty.";
        ofLogNotice() << "\n";
        
    }
    
}

//--------------------------------------------------------------

void ofApp::changeFileAuto() {
    
    refreshPlaylist();
    
    if (isFiles) {
        fileRequested = true;
        nextMovieLoaded = false;
        
        
        if (randomMovie.get() ) {
            
            nextiD = (int)ofRandom(0, playlist.size() - 1 );
            
        } else {
            nextiD = (nextiD + 1) % (playlist.size() );
        }
        
        
        string nextPath = playlist.at(nextiD);
        moviePlayer.at( (playerSelector + 1 ) % 2 ).setLoopState(OF_LOOP_NORMAL);
        moviePlayer.at( (playerSelector + 1 ) % 2 ).setPixelFormat(OF_PIXELS_NATIVE);
        moviePlayer.at( (playerSelector + 1 ) % 2 ).setUseTexture(true);
        
        moviePlayer.at( (playerSelector + 1 ) % 2 ).load( nextPath);
        
        
        ofLogNotice() << "\n";
        ofLogNotice() << "Attempt to load : " << nextPath;
        ofLogNotice() << "\n";
    }
    
    else {
        ofLogNotice() << "\n";
        ofLogNotice() << "Auto Changefile requested but directory is empty.";
        ofLogNotice() << "\n";
        
    }
    
    
}

//--------------------------------------------------------------
void ofApp::updateMovieSpeed() {
    
    float mod;
    
    switch (varispeedOption) {
            
        case 0 :
            mod = power;
            break;
        case 1 :
            mod = pitchFreqNorm;
            break;
        case 2 :
            mod = pitchSalience;
            break;
        case 3 :
            mod = hfcNorm;
            break;
        case 4 :
            mod = specCompNorm ;
            break;
        case 5 :
            mod = centroidNorm ;
            break;
        case 6 :
            mod = dissonance ;
            break;
        case 7 :
            mod = rollOffNorm ;
            break;
        case 8 :
            mod = oddToEvenNorm ;
            break;
        case 9 :
            mod = strongPeakNorm ;
            break;
        case 10 :
            mod = strongDecayNorm ;
            break;
        case 11 :
            mod = 0.0f;
            break;
            
            
    }
    
    movieSpeed = ofClamp(varispeedRef + mod * varispeedIntensity, 0.2f, 2.0f);
    
    if (HAPmode) hap.setSpeed(movieSpeed);
    
    _pitchCompensation = (1-movieSpeed) * pitchCompensation * 2400;
    
    
    //pitchshift.setParameter(<#AudioUnitParameterID property#>, <#AudioUnitScope scope#>, <#AudioUnitParameterValue value#>);
    
    pitchshift.setParameter(kTimePitchParam_Pitch, kAudioUnitScope_Global, _pitchCompensation);
    
    
    
}

//--------------------------------------------------------------
void ofApp::updateFile() {
    
    if ( HAPmode ) {
        
        hap.update();
        
        fileName = hap.getFileName();
        
        if (!hap.alive) restartHap();
        
    }
    
    else {
        
        if (playing) {
            
            if (moviePlayer.at(playerSelector).isLoaded() ) {
                moviePlayer.at(playerSelector).update();
            }
            
        }
        
        
        if (fileRequested) {
            
            
            if (moviePlayer.at( (playerSelector + 1 ) % 2 ).isLoaded() && (nextMovieLoaded == false) ) {
                
                ofLogNotice() << "\n";
                ofLogNotice() << "File loaded : " << playlist.at(nextiD);
                ofLogNotice() << "\n";
                
                moviePlayer.at( (playerSelector + 1) % 2 ).play();
                
                nextMovieLoaded = true;
                
                
            }
            
            
            if (nextMovieLoaded == true) {
                moviePlayer.at( (playerSelector + 1 ) % 2 ).update();
                ofLogNotice() << "Waiting first frame....";
            }
            
            
            if ( moviePlayer.at( (playerSelector + 1 ) % 2 ).isFrameNew()  &&  (nextMovieLoaded == true) ) {
                
                ofFile file( playlist.at(nextiD) );
                
                movieNum = nextiD;
                
                fileName = file.getFileName();
                
                moviePlayer.at(playerSelector).stop();
                
                moviePlayer.at( playerSelector ).close();
                
                playerSelector = (playerSelector + 1 ) % 2;
                
                fileRequested = false;
                
                if (!playing) {
                    moviePlayer.at( playerSelector ).setPaused(true);
                }
                
                ofLogNotice() << "\n";
                ofLogNotice() << "First frame OK Switching to : " << fileName;
                ofLogNotice() << "\n";
                //cout << moviePlayer.at( playerSelector ).getPixelFormat() << endl;
            }
            
        }
        
    }

}





//----------------------------------------------------

static bool sortFileDate (ofFile i, ofFile j) {
    
    std::time_t a = std::filesystem::last_write_time(i);
    std::time_t b = std::filesystem::last_write_time(j);
    
    return ( a < b );
    
}

//--------------------------------------------------------------

void ofApp::refreshPlaylist() {
    
    ofLogNotice() << "\n";
    ofLogNotice() << "[::::MAIN::::] : Update Playlist";
    ofLogNotice() << "\n";
    
    ofXml movies;
    
    movies.addChild("MOVIES");
    
    ofDirectory test( playPath );
    
    if (test.exists()) {
        
        //only show mpg & mp4 files
        test.allowExt("MP4");
        test.allowExt("MPG");
        test.allowExt("mp4");
        test.allowExt("mpg");
        test.allowExt("MOV");
        
        //populate the directory object
        test.listDir();
        
        if ( test.size() < minNumFiles ) {
            
            pathToVids = rescuePath;
            appState = "Rescue";
        }
        
        else {
            pathToVids = playPath;
            appState = "Normal";
        }
        
        ofDirectory playDir(pathToVids.get() );
        
        playDir.allowExt("MP4");
        playDir.allowExt("MPG");
        playDir.allowExt("mp4");
        playDir.allowExt("mpg");
        playDir.allowExt("MOV");
        
        playDir.listDir();
        
        vector<ofFile> files = playDir.getFiles();
        
        sort (files.begin(), files.end(), sortFileDate );
        
        
        playlist.clear();
        
        playlist.assign(playDir.size(), string() );
        
        playNames.clear();
        playNames.assign(playDir.size(), string() );
        
        movies.addChild("MovieFiles");
        
        
        
        for (int i = 0; i < files.size(); i++) {
            
            string path = files.at(i).getAbsolutePath();
            
            ofLogNotice() << "File found : " << path;
            
            playlist.at(i) = path;
            playNames.at(i) = files.at(i).getFileName();
            
            ofXml temp;
            temp.addChild("Movie");
            temp.setAttribute("id", ofToString(i) );
            temp.addValue("path", playlist.at(i) );
            
            movies.addXml(temp);
            
            
        }
        
        movies.save("lists/Movies.xml");
        
        if (files.size() == 0) {isFiles = false;}
        else { isFiles = true; }
        
    }
    
    else {
        ofLogNotice() << "FILES DIRECTORY DOES NOT EXIST.";
        test.create();
    }
    
}




//--------------------------------------------------------------


void ofApp::refreshStreamsList() {
    
    ofLogNotice() << "\n";
    ofLogNotice() << "[::::MAIN::::] : Update Streams list";
    ofLogNotice() << "\n";
    
    streamsList.clear();
    
    ofBuffer buffer = ofBufferFromFile(pathToStreamsListFile.get() );
    
    for (auto line : buffer.getLines() ) {
        streamsList.push_back(line);
    }
    
    
    ofLogNotice() << "Found streams list : " << pathToStreamsListFile;
    //ofLogNotice() << ofToString( streamsList );
    
    streamsNames.clear();
    streamsNames.assign(streamsList.size() / 2, string() );
    streamsUrls.clear();
    streamsUrls.assign(streamsList.size() / 2, string() );
    
    ofXml streams;
    
    streams.addChild("STREAMS");
    
    
    for (int i = 0; i < streamsList.size(); i += 2 ) {
        
        // ofLogNotice() << "Listed stream : " << streamsList.at(i);
        // ofLogNotice() << "Url : " << streamsList.at(i + 1) << "\n";
        
        
        streamsNames.at(i / 2) = streamsList.at(i);
        streamsUrls.at(i / 2) = streamsList.at(i + 1);
        
        ofXml temp;
        temp.addChild("Stream");
        temp.setAttribute("id", ofToString(i / 2) );
        temp.addValue("Name", streamsList.at(i) );
        temp.addValue("Url", streamsList.at(i + 1) );
        streams.addXml(temp);
        
        
    }
    
    
    
    streams.save("lists/Streams_Main.xml");
    
}


//--------------------------------------------------------------

void ofApp::initCams(){
    
    
    camList = videoCamera.listDevices();
    
    camNames.resize(camList.size() );
    
    ofXml cameras;
    cameras.addChild("Cameras");
    
    for (int i = 0; i < camList.size(); i++) {
        if (camList[i].bAvailable) {
            ofLogNotice() << camList[i].id << ": " << camList[i].deviceName;// << "->" << ofToString(camList[i].formats);
            camNames.at(i) = camList[i].deviceName;
            ofXml temp;
            temp.addChild("Camera");
            temp.setAttribute("id", ofToString(i) );
            temp.addValue("Name", camList[i].deviceName );
            cameras.addXml(temp);
        }
        else {
            ofLogNotice() << camList[i].id << ": " << camList[i].deviceName << " - unavailable ";
            camNames.at(i) = "unavailable";
        }
    }
    
    cameras.save("lists/Cameras.xml");
    
}


//--------------------------------------------------------------


void ofApp::changeVideoCam(int n) {
    initCams();
    if (camList.size() >=1 ) {
        
        if (videoUsbInputOption.get() >= camList.size() ) videoUsbInputOption = 0;
        
        if (videoCamera.isInitialized() ) videoCamera.close();
        
        videoCamera.setDeviceID(videoUsbInputOption);
        videoCamera.setDesiredFrameRate(25);
        videoCamera.setPixelFormat(OF_PIXELS_NATIVE);
        //videoCamera.setup(1920, 1080, true);
        videoCamera.setup(3000, 2000, true);//1980x1024 (4/3)
        
        vCamName = camNames.at(videoUsbInputOption);
    }
    
}

//--------------------------------------------------------------


void ofApp::changeAudioVideoCam(int n) {
    initCams();
    if (camList.size() >=1 ) {
        if (audioVideoUsbInputOption>= camList.size() ) audioVideoUsbInputOption = 0;
        
        if (audioVideoCamera.isInitialized() ) audioVideoCamera.close();
        
        if(audioVideoUsbInputOption != videoUsbInputOption){
            audioVideoCamera.setDeviceID(audioVideoUsbInputOption);
            audioVideoCamera.setDesiredFrameRate(25);
            audioVideoCamera.setPixelFormat(OF_PIXELS_NATIVE);
            audioVideoCamera.setup(1920, 1080, true);
            audioVideoCam = &audioVideoCamera;
        }
        else {
            
            if (!videoCamera.isInitialized() ){
                videoCamera.setDeviceID(videoUsbInputOption);
                videoCamera.setDesiredFrameRate(25);
                videoCamera.setPixelFormat(OF_PIXELS_NATIVE);
                videoCamera.setup(1920, 1080, true);
            }
            
            audioVideoCam = &videoCamera;
        }
        
        avCamName = camNames.at(audioVideoUsbInputOption);
    }
    
}


//--------------------------------------------------------------
void ofApp::changeStream(int streamId) {
    
    refreshStreamsList();
    
}


//--------------------------------------------------------------

void ofApp::play() {
    
    if ( HAPmode ) {
        
        hap.play();
        
    }
    
    else {
        if (isFiles) {
            if ( moviePlayer.at(playerSelector).isLoaded() ) {
                moviePlayer.at(playerSelector).play();
                playing = true;
            } else {
                changeFile( movieNum );
                playing = true;
            }
        }
    }
    
    
}


//--------------------------------------------------------------

void ofApp::pause() {
    
    if ( HAPmode ) {
        
        hap.pause();
        
    }
    
    else {
        if (isFiles) {
            moviePlayer.at(playerSelector).setPaused(true);
            playing = false;
        }
    }
    
}




//--------------------------------------------------------------


void ofApp::stop() {
    
    if ( HAPmode ) {
        
        hap.stop();
        
    }
    
    else {
        if (isFiles) {
            moviePlayer.at(playerSelector).stop();
            playing = false;
        }
        
    }
    
}





//--------------------------------------------------------------

void ofApp::exit() {
    

    
    //saveConfig();
    
    //scOscSender.disableBroadcast();
    
    quitSc();
    
    manager.stop();
    

    if ( HAPmode ) {
        endHap();
    }
    
    else {
        moviePlayer.at(playerSelector).stop();
        moviePlayer.at(playerSelector).close();
        moviePlayer.at((playerSelector + 1) % 2).close();
    }
    
    videoCamera.close();
    audioVideoCamera.close();
    
    videoAnalyzer.stopThread();
    
    audioAnalyzerLeft.exit();
    ofSleepMillis(100);
    audioAnalyzerRight.exit();
    ofSleepMillis(100);
    
    midiOut.closePort();
    
    ofLogNotice() << ofSystem("/usr/local/bin/SwitchAudioSource -s \""+ currentAudioOutput + "\"") << "\n";
    
    string time = ofGetTimestampString("%d-%m-%Y-%H-%M-%S");
    
    ofLogNotice() << "";
    ofLogNotice() << "";
    ofLogNotice() << time;
    
    //ofExit();
    OF_EXIT_APP(0);
    
}






//--------------------------------------------------------------

void ofApp::recStream(int iD) {
    
    //#osascript : osascript -e 'tell application "Terminal" to do script "./streamRecStda.sh /DATA/VIDEOS/EVAPORATIONS/STREAMVIDS/ CNN_ http://www.wiiz.tv/cnn.php 1"'
    
    
    string name = streamsNames.at(iD);
    string url = streamsUrls.at(iD);
    string duration = ofToString(recTime.get() );
    
    string args = pathToVids.get() + " " + name + "__" + " " + url + " " + duration;
    
    string script = ofToDataPath("") + "scripts/./streamRecStda.sh";
    
    string command = "osascript -e 'tell application \"Terminal\" to do script \"" + script;
    string commandLine = command + " " + args + "\"'";
    
    // string command = "open -a Terminal " + script;
    // string commandLine = command + " " + args + " &";
    
    ofLogNotice() << commandLine;
    
    
    string commandReturn1 = ofSystem(commandLine);
    string commandReturn2 = ofSystem("osascript -e 'tell application \"Terminal\" to  activate'");
    
    ofLogNotice() << commandReturn1;
    ofLogNotice() << commandReturn2;
    
}


//--------------------------------------------------------------

void ofApp::checkWorkingDir() {
    
    string path = workPath.get();
    
    ofDirectory check(path );
    
    if (!check.exists() || path == "" ) {
        
        ofFileDialogResult openResult = ofSystemLoadDialog("Select a working directory.");
        path = ofFilePath::getEnclosingDirectory(openResult.getPath(), true);
        
    }
    
    ofDirectory play(path + "Play/");
    ofDirectory rec(path + "Record/");
    ofDirectory rescue(path + "Rescue/");
    ofDirectory error(path + "Error/");
    ofDirectory archive(path + "Archive/");
    ofDirectory print(path + "Print/");
    
    
    if (!play.exists() ) {play.create(); }
    if (!rec.exists() ) {rec.create(); }
    if (!rescue.exists() ) {rescue.create(); }
    if (!error.exists() ) {error.create(); }
    if (!archive.exists() ) {archive.create(); }
    if (!print.exists() ) {print.create(); }
    
    //workPath = path;
    playPath = path + "Play/";
    rescuePath = path + "Rescue/";
    errorPath = path + "Error/";
    archivePath = path + "Archive/";
    printPath = path + "Print/";
    
    ofLogNotice() << "WORKING_DIRECTORY_IS_:" << path;
    
    //saveConfig();
    
}

//--------------------------------------------------------------

void ofApp::checkStreamsList() {
    
    //pathToStreamsListFile = ofToDataPath("") + "lists/RECORD_URLS.txt";
    
    ofFile list(pathToStreamsListFile.get() );
    
    if (!list.exists() ) {
        ofFileDialogResult openResult = ofSystemLoadDialog("Select the file containing the streams list");
        pathToStreamsListFile = openResult.getPath();
    }
    
    
    
}

//--------------------------------------------------------------

void ofApp::detectError() {
    
    string message;
    
    if (playing && appState == "Normal") {
        /*
         brightnessTest = 0.0f;
         ofPixels pixTest;
         pixTest.allocate(50, 50, OF_IMAGE_COLOR);
         
         if (moviePlayer.at(playerSelector).isInitialized()) {
         moviePlayer.at(playerSelector).getPixels().resizeTo(pixTest,  OF_INTERPOLATE_NEAREST_NEIGHBOR);
         }
         for (int i = 0; i < pixTest.getWidth(); i++) {
         for (int j = 0; j< pixTest.getHeight(); j++) {
         
         brightnessTest += pixTest.getColor(i, j).getBrightness();
         
         }
         }
         */
        
        if ( testRms == 0.0f  ) { sTimer += 1; message = "NO SOUND ERROR : " + ofToString(sTimer); }
        else { sTimer = 0.0f; }
        
        
        //if ( brightnessTest == 0.0f  ) { bTimer += 1; message = "BLACK FRAME ERROR : " + ofToString(bTimer); }
        //else {bTimer = 0.0f; }
        
        
        if ( (bTimer / 25.0f > blackTimer) || (sTimer / 25.0f > silenceTimer) ) {
            
            movieToRemovePath = playlist.at(movieNum);
            
            removeRequested = true;
            removeMessage = message;
            
            managerChange = true;
            
            sTimer = 0.0f;
            bTimer = 0.0f;
            
            changeFile( nextiD + 1);
            
        }
        
    }
    
}



void ofApp::testSound() {
    
    string message;
    static int badcount = 0;
    
    if ( testRms == 0.0f  ) { message = "NO SOUND ERROR : " + ofToString(sTimer); }
    else { sTimer = ofGetElapsedTimeMillis(); }
    
    if ( (ofGetElapsedTimeMillis()-sTimer)/1000.0f > silenceTimer ) {
        badcount++;
        sTimer = ofGetElapsedTimeMillis();
    }
    
    if ( badcount >= 4 ) exit();
    

}


//--------------------------------------------------------------

void ofApp::eraseMovie( string path, string message) {
    
    
    ofFile file(path);
    ofLogNotice() << "[:::::MAIN:::::] : Erasing file : " << file.getAbsolutePath();
    ofLogNotice() << "[:::::MAIN:::::] : ==> " << message;
    if (keepErrorFile) {
        string name = file.getFileName();
        file.moveTo(errorPath + name, false, true);
    } else {file.remove(); }
    
    
}




//-------------------------------------------------------------------------------------------------------------------------


//::::	PRINT STUFF


//-------------------------------------------------------------------------------------------------------------------------


void ofApp::print() {
    
    // ofCairoRenderer pdf;
    // pdf.setup("snapshot.pdf", ofCairoRenderer::PDF);
    
    // //pdf.viewport(ofRectangle(0, 0, 2970, 2100) );
    // pdf.setupGraphicDefaults();
    // pdf.setBackgroundColor(ofColor::white);
    
    // pdf.setFillMode(OF_FILLED);
    // pdf.setColor(ofColor::black);
    
    // int numPoints = bufferSize;
    
    // for (int i = 0; i < numPoints; i++) {
    // 	float sample = audioBuffer[i * 2];
    // 	float x = ofMap(i, -1, 1, 0, pdf.getViewportWidth() );
    // 	float y = ofMap(sample, -1, 1, 0, pdf.getViewportHeight());
    // 	pdf.drawCircle(x, y, 0, 1);
    // }
    // pdf.close();
    
    
}


void ofApp::drawPrint() {
    
    ofPushStyle();
    
    ofBackground(backgroundColor);
    
    drawSourceInfo(5 + previewWidth + 5, 15, ofGetWidth() - 10 - previewWidth, previewHeight );
    
    drawSource(5, 5, previewWidth, previewHeight);
    
    drawBroadcast(5, 5 + previewHeight + 5, videoWidth, videoHeight);
    
    drawHisto(5 + videoWidth + 5, 5 + previewHeight + 5 + videoHeight / 2, videoWidth, videoHeight / 2);
    
    drawLuSats(5 + videoWidth + 5, 5 + previewHeight + 5 + videoHeight, videoWidth, videoHeight / 2.2);
    
    drawXyPositions((5 + videoWidth) * 2 + 5, 5 + previewHeight + 5 + videoHeight, videoWidth, videoHeight);
    
    drawThumbnails(0, 5 + previewHeight + 5 + videoHeight + 5, thumbsWidth, thumbsHeight);
    
    drawColorGraph(ofGetWidth() - 1,  5 + previewHeight + 5 + videoHeight + 5 + thumbsHeight + 5 + 100, -ofGetWidth() - thumbsWidth, -100);
    
    drawWaveForm(5, 5 + previewHeight + 5 + videoHeight + 5 + thumbsHeight + 5 + 100 + 5 + videoHeight, videoHeight, videoHeight);
    
    drawRMS(5 + videoHeight + 10 , 5 + previewHeight + 5 + videoHeight + 5 + thumbsHeight + 5 + 100 + 5 + videoHeight, videoWidth / 8, videoHeight);
    
    drawSoundGraphPoints( 0,  ofGetHeight(), ofGetWidth(), -90);
    
    ofPopStyle();
}



//-------------------------------------------------------------------------------------------------------------------------


//::::	START FUNCTIONS


//-------------------------------------------------------------------------------------------------------------------------






//-------------------------------------------------------------------------------------------------------------------------


//::::	CONFIG SETUP SAVE


//-------------------------------------------------------------------------------------------------------------------------


void ofApp::setupConfig() {
    
    
    workPath.set("WORKING_DIRECTORY", "/EVAPORATION/");
    pathToStreamsListFile.set("STREAMS_LIST", ofToDataPath("") + "lists/" + "RECORD_URLS.txt");
    pathToDirectStreamsListFile.set("DIRECT_STREAMS_LIST", ofToDataPath("") + "lists/" + "DIRECT_URLS.txt");
    
    videoPresetName.set("VIDEO_PRESET", "DEFAULT");
    audioPresetName.set("AUDIO_PRESET", "DEFAULT");
    
    HAPmode.set("HAPmode", 0);
    
    
    viewNum.set("VIEW_NUMBER", 0);
    viewName.set("VIEW", viewOptions.at(0) );
    
    
    
    ofDirectory check(ofToDataPath("") );
    
    if (check.exists() ) {
        string path = ofToDataPath("") + "set/" + "SetupConfig.xml";
        
        ofLogNotice() << "DEBUG SETUP PATH : " << path;
        
        ofFile test(path);
        
        if (test.exists() ) {
            
            ofXml config;
            
            ofBuffer buffer = test.readToBuffer();
            
            config.loadFromBuffer( buffer.getText() );
            
            
            workPath = config.getValue(workPath.getName() );
            
            pathToStreamsListFile = ofToDataPath("") + "lists/" + config.getValue(pathToStreamsListFile.getName() );
            pathToDirectStreamsListFile = ofToDataPath("") + "lists/" + config.getValue(pathToDirectStreamsListFile.getName() );
            
            videoPresetName = config.getValue(videoPresetName.getName() );
            audioPresetName = config.getValue(audioPresetName.getName() );
            
            
            //            automationsPresetName = config.getValue(automationsPresetName.getName() );
            //            printPresetName = config.getValue(printPresetName.getName() );
            //
            //            oscSyncInputPort = config.getIntValue(oscSyncInputPort.getName() );
            //            oscSyncOutputPort = config.getIntValue(oscSyncOutputPort.getName() );
            
            
            HAPmode = config.getIntValue(HAPmode.getName() );
            
            ofLogNotice() << "[::::CONFIFURATION_SETUP::::] : HAPmode : " << HAPmode;
            
            viewNum = config.getIntValue(viewNum.getName() );
            viewName = "start";//config.getValue(viewName.getName() );
            
            
        }
        
    }
    
    
    checkWorkingDir();
    checkStreamsList();
    
    ofLogNotice() << "[::::CONFIFURATION_SETUP::::] : LOADED";
}


//--------------------------------------------------------------------------------------


void ofApp::saveConfig() {
    
    ofXml config;
    
    config.addChild("SETUP");
    
    config.addValue(workPath.getName(), workPath.get() );
    
    ofFile file(pathToStreamsListFile.get() );
    config.addValue(pathToStreamsListFile.getName(), file.getFileName() );
    
    ofFile file_(pathToDirectStreamsListFile.get());
    config.addValue(pathToDirectStreamsListFile.getName(), file_.getFileName() );
    
    config.addValue(videoPresetName.getName(), videoPresetName.get() );
    config.addValue(audioPresetName.getName(), audioPresetName.get() );
    
    config.addValue(HAPmode.getName(), HAPmode.get() );
    
    config.addValue(viewNum.getName(), viewNum.get() );
    config.addValue(viewName.getName(), viewName.get() );
    
    config.save("set/SetupConfig.xml");
    
    ofLogNotice() << "[::::CONFIGURATION::::] : SAVED";
    
}



//-------------------------------------------------------------------------------------------------------------------------


//::::	VIDEO SETUP LOAD SAVE


//-------------------------------------------------------------------------------------------------------------------------



void ofApp::setupVideo() {
    
    
    /*-------------------DEFAULT SETTINGS--------------------*/
    
    //SETTINGS
    videoInputOption.set("Video_Input",0, 0, videoInputOptions.size()-1 );
    videoUsbInputOption.set("Camera", 0);
    
    oscVideoDataOutputAddr.set("VIDEO_OSC_OUT_ADDR", "127.0.0.1");
    oscVideoDataOutputPort.set("VIDEO_OSC_OUT_PORT", 57120);
    
    //PROCESSING
    contrast.set("Contrast", 1.0f, 0.0f, 5.0f);
    brightness.set("Brightness", 1.0f, 0.0f, 5.0f);
    saturation.set("Saturation", 1.0f, 0.0f, 5.0f);
    zoomFactor.set("Zoom_Factor", 2.0f, 1.0f, 4.0f);
    zoomPosX.set("ZoomPos_X", 0.0f, -1.0f, 1.0);
    zoomPosY.set("ZommPos_Y", 0.0f, -1.0f, 1.0);
    videoCrop.set("Video_Crop", 0.0f, 0.0f, 1.0f);
    
    
    //DATA SETTINGS dEFAULT
    videoAnalyseOn.set("Video_Analysis", 1);
    
    videoStartC.set("Start_Color_f", 0, 0, 1023);
    videoEndC.set("End_Color_f", 1023, 0, 1023);
    videoStartF.set("Start_Sound_f", 0, 0, 1023);
    videoEndF.set("End_Sound_f", 1023, 0, 1023);
    
    analysisWidth.set("ANALYSIS_WIDTH", 120, 10, 640);
    analysisHeight.set("ANALYSIS_HEIGHT", 80, 10, 640);
    nBandsAnalysis.set("N_BANDS_ANALYSIS", 1024, 12, 1024);
    nBandsHisto.set("n_Bands_Histo", 256, 12, 256);
    
    histoCont.set("Presence_Contrast", 0.0f, -1.0f, 1.0f);
    histoAmp.set("Presence_Amp", 5.0f, 0.0f, 5.0f);
    histoSmooth.set("Presence_Smooth", 0.9f, 0, 1.0f);
    
    luCont.set("Brightness_Contrast", 0.0f, -1.0f, 1.0f);
    luAmp.set("Brightness_Amp", 1.0f, 0.0f, 5.0f);
    luSmooth.set("Brightness_Smooth", 0.9f, 0.0f, 1.0f);
    
    satCont.set("Saturation_Contrast", 0.0f, -1.0f, 1.0f);
    satAmp.set("Saturation_Amp", 1.0f, 0.0f, 5.0f);
    satSmooth.set("Saturation_Smooth", 0.9f, 0.0f, 1.0f);
    
    
    xyCont.set("Positions_Contrast", 0.0f, -1.0f, 1.0f);
    xyAmp.set("Positions_Amp", 1.8f, 0.0f, 5.0f );
    xySmooth.set("Position_Smooth", 0.95f, 0.0f, 1.0f);
    
    panCont.set("Pan_Contrast", 0.0f, -1.0f, 1.0f);
    panAmp.set("Pan_Amp", 1.8f, 0.0f, 5.0f );
    panSmooth.set("Pan_Smooth", 0.95f, 0.0f, 1.0f);
    
    tiltCont.set("Tilt_Contrast", 0.0f, -1.0f, 1.0f);
    tiltAmp.set("Tilt_Amp", 1.8f, 0.0f, 5.0f );
    tiltSmooth.set("Tilt_Smooth", 0.95f, 0.0f, 1.0f);
    
    whiteThresh.set("WhitePresences_Threshold", 245, 0, 255);
    whiteCont.set("WhitePresence_Contrast", 0.0f, -1.0f, 1.0f);
    whiteAmp.set("WhitePresence_Amp", 1.0f, 0.0f, 10.0f);
    whiteSmooth.set("WhitePresence_SMooth", 0.0f, 0.0f, 1.0f);
    
    neutralThresh.set("NeutralPresences_Threshold", 15, 0, 255);
    neutralCont.set("GrayPresence_Contrast", 0.0f, -1.0f, 1.0f);
    neutralAmp.set("GrayPresence_Amp", 1.0f, 0.0f, 10.0f);
    neutralSmooth.set("GrayPresence_SMooth", 0.0f, 0.0f, 1.0f);
    
    blackThresh.set("BlackPresences_Threshold", 15, 0, 255);
    blackCont.set("BlackPresence_Contrast", 0.0f, -1.0f, 1.0f);
    blackAmp.set("BlackPresence_Amp", 1.0f, 0.0f, 10.0f);
    blackSmooth.set("BlackPresence_SMooth", 0.0f, 0.0f, 1.0f);
    
    lightnessCont.set("Lightness_Contrast", 0.0f, -1.0f, 1.0f);
    lightnessAmp.set("Lightness_Amp", 1.0f, 0.0f, 5.0f);
    lightnessSmooth.set("Lightness_Smooth", 0.0f, 0.0f, 1.0f);
    
    averageHueSmooth.set("Average_Hue_Smooth", 0.5f, 0.0f, 1.0f);
    
    averageBrightnessAmp.set("Average_Brightness_Amp", 1.0f, 0.5f, 2.0f);
    averageBrightnessSmooth.set("Average_Brightness_Smooth", 0.5f, 0.0f, 1.0f);
    
    averageSaturationAmp.set("Average_Saturation_Amp", 1.0f, 0.5f, 2.0f);
    averageSaturationSmooth.set("Average_Saturation_Smooth", 0.5f, 0.0f, 1.0f);
    
    //MIDI SETTINGS DEFAULT
    midiOutOn.set("Midi_Out_On_Off", 0, 0, 1);
    midiOutputOption.set("Midi_Output", 0);
    midiChannel.set("Midi_Channel", 1, 1, 16);
    midiScaleSize.set("Num_Notes", 12, 3, 12);
    midiScaleOption.set("Scale", 0);
    midiScaleKey.set("Key", 0, -11, 11);
    midiScale.resize(12);
    for (int i=0; i<midiScale.size(); i++) {
        midiScale.at(i).set("Note_"+ofToString(i), i, 0, 11);
    }
    midiThresholds.resize(12);
    for (int i=0; i<midiThresholds.size(); i++) {
        midiThresholds.at(i).set("Thresh_"+ofToString(i), 0.1f, 0.0f, 1.0f);
    }
    midiTriggerOption.set("Midi_Triggers", 0, 0, videoArrayOptions.size()-1);
    
    midiOctaveOption.set("Midi_Octave", 0, 0, videoArrayOptions.size()-1 );
    midiOctaveIntensity.set("Midi_Octave_Intensity", 0, 0, 7);
    midiOctaveRef.set("Midi_Octave_ref", 5, 2, 8);
    
    midiVelocityOption.set("Midi_Velocities", 0, 0, videoArrayOptions.size()-1);
    midiVelocitiesIntensity.set("Midi_Velocities_Intensity", 0.0f, -1.0f, 1.0f);
    midiVelocitiesRef.set("Midi_Velocities_ref", 0.0f, 0.0f, 1.0f);
    
    midiAfterTouchOption.set("Midi_AfterTouches", 0, 0, videoArrayOptions.size()-1);
    midiAfterTouchesIntensity.set("Midi_AfterTouches_Intensity", 0.0f, -1.0f, 1.0f);
    midiAfterTouchesRef.set("Midi_AfterTouches_ref", 0.0f, 0.0f, 1.0f);
    
    midiCcNum1.set("Midi_CC1_Num", 12, 0, 127);
    midiCcOption1.set("Midi_CC1", 0, 0, videoUnaryOptions.size()-1);
    midiCcIntensity1.set("Midi_CC1_Intensity", 0.0f, -1.0f, 1.0f);
    midiCcRef1.set("Midi_CC1_ref", 0.0f, 0.0f, 1.0f);
    
    midiCcNum2.set("Midi_CC2_Num", 13, 0, 127);
    midiCcOption2.set("Midi_CC2", 0, 0, videoUnaryOptions.size()-1);
    midiCcIntensity2.set("Midi_CC2_Intensity", 0.0f, -1.0f, 1.0f);
    midiCcRef2.set("Midi_CC2_ref", 0.0f, 0.0f, 1.0f);
    
    
    
    //-------------------------------SC_PANNEL
    scServerAddr.set("sc_Server_Addr", "127.0.0.1");
    scServerPort.set("sc_Server_Port", 57110);
    scInputName.set("sc_Input_Device", "Apple Inc.: Built-in Input");
    scOutputName.set("sc_Output_Device", "Apple Inc.: Built-in Output");
    scSamplerate.set("sc_Samplerate", 44100);
    scMemSize.set("sc_Memsize", 8192);
    scBlockSize.set("sc_Blocksize", 256);
    scNumInputs.set("sc_NumInput", 2);
    scNumOutputs.set("sc_NumOutput", 2);
    
    watchSc.set("Watch_sc", 0);
    
    //iFFT
    
    iFFTbufNum.set("iFFT_BufNum", 70);
    iFFTphasesBufNum.set("iFFT_Phases_BufNum", 71);
    
    iFFTpatchName.set("iFFT_PatchName", "iFFT_evaporation");
    
    //Synth
    scSynthOn.set("SC_Synth_On_Off", 0, 0, 1);
    synthPatchName.set("Synth_PatchName", "Piano_Evaporation");
    synthFreqBufnum.set("Synth_Freq_BufNum", 77);
    synthGateBufnum.set("Synth_Gate_BufNum", 78);
    synthMod1BufNum.set("Synth_Mod1_Buf", 79);
    synthMod2BufNum.set("Synth_Mod2_Buf", 80);
    synthMod3BufNum.set("Synth_Mod3_Buf", 81);
    synthMod4BufNum.set("Synth_Mod4_Buf", 82);
    synthVolsBufnum.set("Synth_Vols_BufNum", 83);
    
    vector<string> mods = {"Velocities", "H1_mod", "H2_mod","LFO_mod"};
    scSynthModsLabels.resize(mods.size() );
    for (int i = 0; i < mods.size(); i++) {
        scSynthModsLabels.at(i).set("Synth_ModLabel_"+ofToString(i), mods.at(i) );
    }
    
    vector<string> modsparams =
    {"H1_n", "H1_p", "H2_n", "H2_p1", "LfoType", "LfoRate"};
    scSynthStaticsLabels.resize(modsparams.size() );
    for (int i = 0; i < modsparams.size(); i++) {
        scSynthStaticsLabels.at(i).set("Synth_Mod_ParameterLabel_"+ofToString(i), modsparams.at(i) );
    }
    
    //------------------------------SC__iFFT
    
    iFFTon.set("iFFT_On_Off", 0, 0, 1);

    iFFTmagOption.set("iFFTmag_Option", 0, 0, magOptions.size()-1 );
    iFFTphaseOption.set("iFFTphase_Option", 0, 0, magOptions.size()-1 );
    
    iFFTloHiModOption.set("iFFTloHiMod_Option", 0, 0, videoUnaryOptions.size()-1  );
    iFFTloHiModIntensity.set("iFFTloHiMod_Intensity", 0, -1.0f, 1.0f);
    iFFTloHiRef.set("iFFTloHi_morph", 0.5f, 0.0f, 1.0f);
    
    iFFThiShiftModOption.set("iFFThiShiftMod_Option", 0, 0, videoUnaryOptions.size()-1  );
    iFFThiShiftModIntensity.set("iFFThiShiftMod_Intensity", 0, -0.75f, 0.75f);
    iFFThiShiftRef.set("iFFT_hiShift", 0.0f, -1.0, 1.0f);
    
    iFFTwhiteMorphModOption.set("iFFTwhiteMorph_Option", 0, 0, videoUnaryOptions.size()-1  );
    iFFTwhiteMorphModIntensity.set("iFFTwhiteMorph_Intensity", 0, -1.0f, 1.0f);
    iFFTwhiteMorphRef.set("iFFT_whiteMorph", 0.0f, 0.0f, 1.0f);
    
    iFFTampModOption.set("iFFTampMod_Option", 0, 0, videoUnaryOptions.size()-1  );
    iFFTampModIntensity.set("iFFTampMod_Intensity", 0, -1.0f, 1.0f);
    iFFTampRef.set("iFFT_amp", 1.0f, 0.0f, 5.0f);

    iFFTconvoMixModOption.set("iFFTconvoMixMod_Option", 0, 0, videoUnaryOptions.size()-1  );
    iFFTconvoMixModIntensity.set("iFFTconvoMixMod_Intensity", 0, -1.0f, 1.0f);
    iFFTconvoMixRef.set("iFFT_convoMix", 1.0f, 0.0f, 5.0f);

    
    //-----------------------------SUPER_C_patch
    scFreqs.resize(16);
    for (int i = 0; i<16;i++){
        scFreqs.at(i).set("sc_Freq_"+ofToString(i), 65*2^i, 0.0f, 20000.0f);
    }
    scVolumes.resize(16);
    for (int i = 0; i < 16; i++){
        scVolumes.at(i).set("Vol_"+ofToString(i), 0.5f, 0.0f, 4.0f);
    }
    
    scTriggerOption.set("sc_Trigger_Option", 0, 0, videoArrayOptions.size()-1);
    scThresholds.resize(16);
    for (int i = 0; i < 16; i++){
        scThresholds.at(i).set("sc_Thresh_"+ofToString(i), 0.5f, 0.0f, 1.0f);
    }
    scVelocitiesOption.set("sc_Velocity_Option", 0, 0, videoArrayOptions.size()-1);
    scVelocitiesIntensities.set("sc_Velocity_Intensity", 0.0f, -1.0f, 1.0f);
    scVelocitiesRefs.set("sc_Velocity_Ref", 0.0f, 0.0f, 1.0f);
    
    hNumber1.set("sc_H1_n", 2, 0, 12);
    hPhase1.set("sc_P1_n", 1/2.0f, 0, 12);
    scH1Option.set("sc_H1mod_Option", 0, 0, videoArrayOptions.size()-1);
    scH1Intensities.set("sc_H1mod_Intensity", 0.0f, -1.0f, 1.0f);
    scH1Refs.set("sc_H1mod_Ref", 0.0f, 0.0f, 1.0f);
    
    hNumber2.set("sc_H2_n", 5, 0, 12);
    hPhase2.set("sc_P2_n", 1/5.0f, 0, 12);
    scH2Option.set("sc_H2mod_Option", 0, 0, videoArrayOptions.size()-1);
    scH2Intensities.set("sc_H2mod_Intensity", 0.0f, -1.0f, 1.0f);
    scH2Refs.set("sc_H2mod_Ref", 0.0f, 0.0f, 1.0f);
    
    lfoOption.set("sc_LFO_Type", 0, 0, 4);
    lfoRate.set("sc_LFO_Rate", 0.5f, 0.0f, 1.0f);
    scLfoOption.set("sc_LFOmod_Option", 0, 0, videoArrayOptions.size()-1);
    scLfoIntensities.set("sc_LFOmod_Intensity", 0.0f, -1.0f, 1.0f);
    scLfoRefs.set("sc_LFOmod_Ref", 0.0f, 0.0f, 1.0f);
    
    scAtt.set("sc_Atk", 0.05f, 0.01f, 5.0f);
    scDec.set("sc_Dec", 0.1f, 0.01f, 5.0f);
    scSus.set("sc_Sus", 0.9f, 0.0f, 1.0f);
    scRel.set("sc_Rel", 3.0f, 0.01f, 5.0f);
    
    scMasterVolOption.set("sc_Master_VolMod_Option", 0, 0, videoUnaryOptions.size()-1 );
    scMasterVolIntensity.set("sc_Master_VolMod_Intensity", 0.0f, -1.0f, 1.0f);
    scMasterVolRef.set("sc_Master_Vol", 0.9f, 0.0f, 4.0f);
    
    
    ofLogNotice() << "[:::: SETUP : VIDEO DEFAULT PARAMETERS DONE ::::]";
    
    
    
    /*-------------------LOAD XML--------------------*/
    
    
    refreshMidiPortList();
    
    refreshVideoPresets();
    
    loadVideoPreset(videoPresetName);
    
    changeMidiPort(midiOutputOption);
    
    
    /*-------------------INIT VIDEO STUFF--------------------*/
    
    videoAnalyzer.setHuesIndexes(huesIndexes);
    
    videoAnalyzer.setup(analysisWidth.get(), analysisHeight.get() );
    
    analyzedPixels.allocate(analysisWidth, analysisHeight, OF_PIXELS_RGB);
    analyzedPixels.setColor(ofColor::black);
    
    presences.resize(1024, 0.0f);
    brightnesses.resize(1024, 0.0f);
    saturations.resize(1024, 0.0f);
    posXs.resize(1024, 0.0f);
    posYs.resize(1024, 0.0f);
    
    velocities.resize(1024, 0.0f);
    angles.resize(1024, 0.0f);
    
    R = 0.0f; V = 0.0f; B = 0.0f; L = 0.0f;
    C =  0.0f; M = 0.0f; J = 0.0f; N = 0.0f;
    
    midiNotes.resize(12, 0);
    midiTriggersBuf.resize(12, 0.0f);
    midiTriggers.resize(12, false);
    midiPresences.resize(12, 0.0f);
    midiBrightnesses.resize(12, 0.0f);
    midiSaturations.resize(12, 0.0f);
    midiXs.resize(12, 0.0f);
    midiYs.resize(12, 0.0f);
    currentNotes.resize(12, 0);
    

    mags.resize(1024, 0.0f);
    phases.resize(1024, 0.0f);
    pan.resize(1024, 0.0f);
    tilt.resize(1024, 0.0f);

    
    scPresences.resize(16, 0.0f);
    scBrightnesses.resize(16, 0.0f);
    scSaturations.resize(16, 0.0f);
    scXs.resize(16, 0.0f);
    scYs.resize(16, 0.0f);
    
    scTriggersBuf.resize(16, 0.0f);
    scTriggers.resize(16, false);
    scVelocities.resize(16, 0.0f);
    h1Mods.resize(16, 0.0f);
    h2Mods.resize(16, 0.0f);
    lfoMods.resize(16, 0.0f);
    
    
    videoScreen.allocate(1280, 720, GL_RGBA); //3840x1200 (2x1920x1200)
    videoFbo.allocate(1920, 1080, GL_RGBA);
    
    memoryFFT.allocate(1280, 1024, OF_PIXELS_RGB);
    
    lastVsource = videoInputOption;
    lastVcam = 99;
    
    

    
    
    
    ofLogNotice() << "[:::: SETUP : INIT VIDEO STUFF  ::::]";
    
    
    
}


//--------------------------------------------------------------------------------------


void ofApp::refreshVideoPresets() {
    
    
    ofDirectory dir( ofToDataPath("") + "presets/" );
    
    dir.allowExt("vp");
    
    dir.listDir();
    
    videoPresetsNames.resize(dir.size() );
    
    
    
    for (int i = 0; i < dir.size(); i++ ) {
        ofFile file = dir.getFile(i);
        videoPresetsNames.at(i) = file.getBaseName();
        
    }
    
    ;
}


//--------------------------------------------------------------------------------------


void ofApp::loadVideoPreset( string name ) {
    
    string presetPath = ofToDataPath("") + "presets/" + name + ".vp";
    
    ofFile file(presetPath);
    
    if (file.exists() ) {
        
        ofLogNotice() << "[:::: LOADING VIDEO PRESET ::::] : " + name;
        
        ofXml preset;
        preset.load(presetPath);
        
        //SETTINGS
        string root = "/VIDEO_SETTINGS/";
        videoInputOption = preset.getIntValue(root + videoInputOption.getName() );
        videoUsbInputOption = preset.getIntValue(root + videoUsbInputOption.getName() );
        
        oscVideoDataOutputAddr = preset.getValue(root + oscVideoDataOutputAddr.getName() );
        oscVideoDataOutputPort = preset.getIntValue(root + oscVideoDataOutputPort.getName() );
        
        contrast = preset.getFloatValue(root + contrast.getName() );
        brightness = preset.getFloatValue(root + brightness.getName() );
        saturation = preset.getFloatValue(root + saturation.getName() );
        
        zoomFactor = preset.getFloatValue(root + zoomFactor.getName() );
        zoomPosX = preset.getFloatValue(root + zoomPosX.getName() );
        zoomPosY = preset.getFloatValue(root + zoomPosY.getName() );
        videoCrop = preset.getFloatValue(root + videoCrop.getName() );
        
        
        //DATA SETTINGS
        root = "/VIDEO_DATA_SETTINGS/";
        videoAnalyseOn = preset.getBoolValue(root + videoAnalyseOn.getName() );
        
        videoStartC = preset.getIntValue(root + videoStartC.getName() );
        videoEndC = preset.getIntValue(root + videoEndC.getName() );
        videoStartF = preset.getIntValue(root + videoStartF.getName() );
        videoEndF = preset.getIntValue(root + videoEndF.getName() );
        
        analysisWidth = preset.getIntValue(root + analysisWidth.getName() );
        analysisHeight = preset.getIntValue(root + analysisHeight.getName() );
        
        nBandsAnalysis = preset.getIntValue(root + nBandsAnalysis.getName() );
        nBandsHisto = preset.getIntValue(root + nBandsHisto.getName() );
        
        histoCont = preset.getFloatValue(root + histoCont.getName() );
        histoAmp  = preset.getFloatValue(root + histoAmp.getName() );
        histoSmooth = preset.getFloatValue(root + histoSmooth.getName() );
        
        luCont = preset.getFloatValue(root + luCont.getName() );
        luAmp  = preset.getFloatValue(root + luAmp.getName() );
        luSmooth = preset.getFloatValue(root + luSmooth.getName() );
        
        satCont = preset.getFloatValue(root + satCont.getName() );
        satAmp = preset.getFloatValue(root + satAmp.getName() );
        satSmooth = preset.getFloatValue(root + satSmooth.getName() );
        
        xyCont = preset.getFloatValue(root + xyCont.getName() );
        xyAmp = preset.getFloatValue(root + xyAmp.getName() );
        xySmooth = preset.getFloatValue(root + xySmooth.getName() );
        
        panCont = preset.getFloatValue(root + panCont.getName() );
        panAmp = preset.getFloatValue(root + panAmp.getName() );
        panSmooth = preset.getFloatValue(root + panSmooth.getName() );
        
        tiltCont = preset.getFloatValue(root + tiltCont.getName() );
        tiltAmp = preset.getFloatValue(root + tiltAmp.getName() );
        tiltSmooth = preset.getFloatValue(root + tiltSmooth.getName() );
        
        whiteThresh = preset.getIntValue(root + whiteThresh.getName() );
        whiteCont = preset.getFloatValue(root + whiteCont.getName() );
        whiteAmp = preset.getFloatValue(root + whiteAmp.getName() );
        whiteSmooth = preset.getFloatValue(root + whiteSmooth.getName() );
        
        neutralThresh = preset.getIntValue(root + neutralThresh.getName() );
        neutralCont = preset.getFloatValue(root + neutralCont.getName() );
        neutralAmp = preset.getFloatValue(root + neutralAmp.getName() );
        neutralSmooth = preset.getFloatValue(root + neutralSmooth.getName() );
        
        blackThresh = preset.getIntValue(root + blackThresh.getName() );
        blackCont = preset.getFloatValue(root + blackCont.getName() );
        blackAmp = preset.getFloatValue(root + blackAmp.getName() );
        blackSmooth = preset.getFloatValue(root + blackSmooth.getName() );
        
        lightnessCont = preset.getFloatValue(root + lightnessCont.getName() );
        lightnessAmp = preset.getFloatValue(root + lightnessAmp.getName() );
        lightnessSmooth = preset.getFloatValue(root + lightnessSmooth.getName() );
        
        averageHueSmooth = preset.getFloatValue(root + averageHueSmooth.getName() );
        
        averageBrightnessAmp = preset.getFloatValue(root + averageBrightnessAmp.getName() );
        averageBrightnessSmooth = preset.getFloatValue(root + averageBrightnessSmooth.getName() );
        
        averageSaturationAmp = preset.getFloatValue(root + averageSaturationAmp.getName() );
        averageSaturationSmooth = preset.getFloatValue(root + averageSaturationSmooth.getName() );
        
        
        //MIDI
        root = "/MIDI/";
        midiOutputOption= preset.getIntValue(root + midiOutputOption.getName() );
        midiChannel= preset.getIntValue(root + midiChannel.getName() );
        midiScaleSize= preset.getIntValue(root + midiScaleSize.getName() );
        midiScaleKey= preset.getIntValue(root + midiScaleKey.getName() );
        for (int i=0; i<midiScale.size(); i++) {
            midiScale.at(i)= preset.getIntValue(root + midiScale.at(i).getName() );
        }
        for (int i=0; i<midiThresholds.size(); i++) {
            midiThresholds.at(i)= preset.getFloatValue(root + midiThresholds.at(i).getName() );
        }
        midiTriggerOption= preset.getFloatValue(root + midiTriggerOption.getName() );
        
        midiOctaveOption= preset.getIntValue(root + midiOctaveOption.getName() );
        midiOctaveIntensity= preset.getFloatValue(root + midiOctaveIntensity.getName() );
        midiOctaveRef= preset.getFloatValue(root + midiOctaveRef.getName() );
        
        midiVelocityOption= preset.getIntValue(root + midiVelocityOption.getName() );
        midiVelocitiesIntensity= preset.getFloatValue(root + midiVelocitiesIntensity.getName() );
        midiVelocitiesRef= preset.getFloatValue(root + midiVelocitiesRef.getName() );
        
        midiAfterTouchOption= preset.getIntValue(root + midiAfterTouchOption.getName() );
        midiAfterTouchesIntensity= preset.getFloatValue(root + midiAfterTouchesIntensity.getName() );
        midiAfterTouchesRef= preset.getFloatValue(root + midiAfterTouchesRef.getName() );
        
        midiCcNum1= preset.getIntValue(root + midiCcNum1.getName() );
        midiCcOption1= preset.getIntValue(root + midiCcOption1.getName() );
        midiCcIntensity1= preset.getFloatValue(root + midiCcIntensity1.getName() );
        midiCcRef1= preset.getFloatValue(root + midiCcRef1.getName() );
        
        midiCcNum2= preset.getIntValue(root + midiCcNum2.getName() );
        midiCcOption2= preset.getIntValue(root + midiCcOption2.getName() );
        midiCcIntensity2= preset.getFloatValue(root + midiCcIntensity2.getName() );
        midiCcRef2= preset.getFloatValue(root + midiCcRef2.getName() );
        
        
        //SC_PANEL
        root = "/SC_PANEL/";
        
        scServerAddr = preset.getValue(root + scServerAddr.getName() );
        scServerPort = preset.getIntValue(root + scServerPort.getName() );
        scInputName = preset.getValue(root + scInputName.getName() );
        scOutputName = preset.getValue(root + scOutputName.getName() );
        scSamplerate = preset.getIntValue(root + scSamplerate.getName() );
        scMemSize = preset.getIntValue(root + scMemSize.getName() );
        scBlockSize = preset.getIntValue(root + scBlockSize.getName() );
        scNumInputs = preset.getIntValue(root + scNumInputs.getName() );
        scNumOutputs = preset.getIntValue(root + scNumOutputs.getName() );
        
        watchSc = preset.getFloatValue(root + watchSc.getName() );
        
        //iFFT
        
        iFFTpatchName = preset.getValue(root + iFFTpatchName.getName() );
        iFFTbufNum = preset.getIntValue(root + iFFTbufNum.getName() );
        iFFTphasesBufNum = preset.getIntValue(root + iFFTphasesBufNum.getName() );
        
        
        //Synth
        
        synthPatchName = preset.getValue(root + synthPatchName.getName() );
        synthFreqBufnum = preset.getIntValue(root + synthFreqBufnum.getName() );
        synthGateBufnum = preset.getIntValue(root + synthGateBufnum.getName() );
        synthMod1BufNum = preset.getIntValue(root + synthMod1BufNum.getName() );
        synthMod2BufNum = preset.getIntValue(root + synthMod2BufNum.getName() );
        synthMod3BufNum = preset.getIntValue(root + synthMod3BufNum.getName() );
        synthMod4BufNum = preset.getIntValue(root + synthMod4BufNum.getName() );
        synthVolsBufnum = preset.getIntValue(root + synthVolsBufnum.getName() );
        
        for (int i = 0; i < scSynthModsLabels.size(); i++) {
            scSynthModsLabels.at(i) = preset.getValue(root + scSynthModsLabels.at(i).getName() );
        }
        
        for (int i = 0; i < scSynthStaticsLabels.size(); i++) {
            scSynthStaticsLabels.at(i) = preset.getValue(root + scSynthStaticsLabels.at(i).getName() );
        }
        
        //iFFT
        
        root = "/iFFT/";
        
        iFFTon = preset.getIntValue(root + iFFTon.getName() );

        iFFTmagOption = preset.getIntValue( root + iFFTmagOption.getName() );
        iFFTphaseOption = preset.getIntValue( root + iFFTphaseOption.getName() );
        
        iFFTampModOption = preset.getIntValue( root + iFFTampModOption.getName() );
        iFFTampModIntensity = preset.getFloatValue( root + iFFTampModIntensity.getName() );
        iFFTampRef = preset.getFloatValue( root + iFFTampRef.getName() );
        
        iFFTloHiModOption = preset.getIntValue( root + iFFTloHiModOption.getName() );
        iFFTloHiModIntensity = preset.getFloatValue( root + iFFTloHiModIntensity.getName() );
        iFFTloHiRef = preset.getFloatValue( root + iFFTloHiRef.getName() );
        
        iFFThiShiftModOption = preset.getIntValue( root + iFFThiShiftModOption.getName() );
        iFFThiShiftModIntensity = preset.getFloatValue( root + iFFThiShiftModIntensity.getName() );
        iFFThiShiftRef = preset.getFloatValue( root + iFFThiShiftRef.getName() );
        
        iFFTwhiteMorphModOption = preset.getIntValue( root + iFFTwhiteMorphModOption.getName() );
        iFFTwhiteMorphModIntensity = preset.getFloatValue( root + iFFTwhiteMorphModIntensity.getName() );
        iFFTwhiteMorphRef = preset.getFloatValue( root + iFFTwhiteMorphRef.getName() );
        
        iFFTconvoMixModOption = preset.getIntValue( root + iFFTconvoMixModOption.getName() );
        iFFTconvoMixModIntensity = preset.getFloatValue( root + iFFTconvoMixModIntensity.getName() );
        iFFTconvoMixRef = preset.getFloatValue( root + iFFTconvoMixRef.getName() );

        
        //SC_SYNTH
        root = "/SC_SYNTH/";
        
        
        scSynthOn = preset.getIntValue( root + scSynthOn.getName() );
        
        for (int i = 0; i<16;i++){
            scFreqs.at(i) = preset.getFloatValue(root + scFreqs.at(i).getName() );
        }
        for (int i = 0; i < 16; i++){
            scVolumes.at(i) = preset.getFloatValue(root + scVolumes.at(i).getName() );
        }
        
        scTriggerOption = preset.getIntValue( root + scTriggerOption.getName() );
        
        for (int i = 0; i < 16; i++){
            scThresholds.at(i) = preset.getFloatValue( root + scThresholds.at(i).getName() );
        }
        
        scVelocitiesOption = preset.getIntValue( root + scVelocitiesOption.getName() );
        scVelocitiesIntensities = preset.getFloatValue( root + scVelocitiesIntensities.getName() );
        scVelocitiesRefs = preset.getFloatValue( root + scVelocitiesRefs.getName() );
        
        hNumber1 = preset.getFloatValue( root + hNumber1.getName() );
        hPhase1 = preset.getFloatValue( root + hPhase1.getName() );
        scH1Option = preset.getIntValue( root + scH1Option.getName() );
        scH1Intensities = preset.getFloatValue( root + scH1Intensities.getName() );
        scH1Refs = preset.getFloatValue( root + scH1Refs.getName() );
        
        hNumber2 = preset.getFloatValue( root + hNumber2.getName() );
        hPhase2 = preset.getFloatValue( root + hPhase2.getName() );
        scH2Option = preset.getIntValue( root + scH2Option.getName() );
        scH2Intensities = preset.getFloatValue( root + scH2Intensities.getName() );
        scH2Refs = preset.getFloatValue( root + scH2Refs.getName() );
        
        lfoOption = preset.getIntValue( root + lfoOption.getName() );
        lfoRate = preset.getFloatValue( root + lfoRate.getName() );
        scLfoOption = preset.getIntValue( root + scLfoOption.getName() );
        scLfoIntensities = preset.getFloatValue( root + scLfoIntensities.getName() );
        scLfoRefs = preset.getFloatValue( root + scLfoRefs.getName() );
        
        scAtt = preset.getFloatValue( root + scAtt.getName() );
        scDec = preset.getFloatValue( root + scDec.getName() );
        scSus = preset.getFloatValue( root + scSus.getName() );
        scRel = preset.getFloatValue( root + scRel.getName() );
        
        scMasterVolOption = preset.getIntValue(root + scMasterVolOption.getName() );
        scMasterVolIntensity = preset.getFloatValue(root + scMasterVolIntensity.getName() );
        scMasterVolRef = preset.getFloatValue( root + scMasterVolRef.getName() );
        
        videoPresetName = name;
        
        
        for (int i = 0; i < videoPresetsNames.size(); i++) {
            if (videoPresetsNames.at(i) == name ) {
                videoPresetNumber = i;
                break;
                
            }
        }
        
        
        changeMidiPort(midiOutputOption);
        
        ofLogNotice() << "[::::VIDEO_PRESET_LOADED::::] : " + videoPresetName.get();
        
    }
    
    
    
    
}


//--------------------------------------------------------------------------------------


void ofApp::saveVideoPreset( string name) {
    
    ofXml preset;
    preset.addChild("VIDEO_SETUP");
    
    ofXml set;
    set.addChild("VIDEO_SETTINGS");
    set.addValue(videoInputOption.getName(), videoInputOption.get() );
    set.addValue(videoUsbInputOption.getName(), videoUsbInputOption.get() );
    
    set.addValue(oscVideoDataOutputAddr.getName(), oscVideoDataOutputAddr.get() );
    set.addValue(oscVideoDataOutputPort.getName(), oscVideoDataOutputPort.get() );
    
    set.addValue(contrast.getName(), contrast.get() );
    set.addValue(brightness.getName(), brightness.get() );
    set.addValue(saturation.getName(), saturation.get() );
    set.addValue(zoomFactor.getName(), zoomFactor.get() );
    set.addValue(zoomPosX.getName(), zoomPosX.get() );
    set.addValue(zoomPosY.getName(), zoomPosY.get() );
    
    preset.addXml(set);
    
    ofXml datset;
    datset.addChild("VIDEO_DATA_SETTINGS");
    datset.addValue(videoAnalyseOn.getName(), videoAnalyseOn.get() );
    
    datset.addValue(videoStartC.getName(), videoStartC.get() );
    datset.addValue(videoEndC.getName(), videoEndC.get() );
    datset.addValue(videoStartF.getName(), videoStartF.get() );
    datset.addValue(videoEndF.getName(), videoEndF.get() );
    
    datset.addValue(analysisWidth.getName(), analysisWidth.get() );
    datset.addValue(analysisHeight.getName(), analysisHeight.get() );
    datset.addValue(nBandsAnalysis.getName(), nBandsAnalysis.get() );
    datset.addValue(nBandsHisto.getName(), nBandsHisto.get() );
    
    datset.addValue(histoCont.getName(), histoCont.get() );
    datset.addValue(histoAmp.getName(), histoAmp.get() );
    datset.addValue(histoSmooth.getName(), histoSmooth.get() );
    
    datset.addValue(luCont.getName(), luCont.get() );
    datset.addValue(luAmp.getName(), luAmp.get() );
    datset.addValue(luSmooth.getName(), luSmooth.get() );
    
    datset.addValue(satCont.getName(), satCont.get() );
    datset.addValue(satAmp.getName(), satAmp.get() );
    datset.addValue(satSmooth.getName(), satSmooth.get() );
    
    datset.addValue(xyCont.getName(), xyCont.get() );
    datset.addValue(xyAmp.getName(), xyAmp.get() );
    datset.addValue(xySmooth.getName(), xySmooth.get() );
    
    datset.addValue(panCont.getName(), panCont.get() );
    datset.addValue(panAmp.getName(), panAmp.get() );
    datset.addValue(panSmooth.getName(), panSmooth.get() );
    
    datset.addValue(tiltCont.getName(), tiltCont.get() );
    datset.addValue(tiltAmp.getName(), tiltAmp.get() );
    datset.addValue(tiltSmooth.getName(), tiltSmooth.get() );
    
    datset.addValue(whiteThresh.getName(), whiteThresh.get() );
    datset.addValue(whiteCont.getName(), whiteCont.get() );
    datset.addValue(whiteAmp.getName(), whiteAmp.get() );
    datset.addValue(whiteSmooth.getName(), whiteSmooth.get() );
    
    datset.addValue(neutralThresh.getName(), neutralThresh.get() );
    datset.addValue(neutralCont.getName(), neutralCont.get() );
    datset.addValue(neutralAmp.getName(), neutralAmp.get() );
    datset.addValue(neutralSmooth.getName(), neutralSmooth.get() );
    
    datset.addValue(blackThresh.getName(), blackThresh.get() );
    datset.addValue(blackCont.getName(), blackCont.get() );
    datset.addValue(blackAmp.getName(), blackAmp.get() );
    datset.addValue(blackSmooth.getName(), blackSmooth.get() );
    
    datset.addValue(lightnessCont.getName(), lightnessCont.get() );
    datset.addValue(lightnessAmp.getName(), lightnessAmp.get() );
    datset.addValue(lightnessSmooth.getName(), lightnessSmooth.get() );
    
    
    datset.addValue(averageHueSmooth.getName(), averageHueSmooth.get() );
    
    datset.addValue(averageBrightnessAmp.getName(), averageBrightnessAmp.get() );
    datset.addValue(averageBrightnessSmooth.getName(), averageBrightnessSmooth.get() );
    
    datset.addValue(averageSaturationAmp.getName(), averageSaturationAmp.get() );
    datset.addValue(averageSaturationSmooth.getName(), averageSaturationSmooth.get() );
    
    
    preset.addXml(datset);
    
    //MIDI
    ofXml midi;
    midi.addChild("MIDI");
    midi.addValue(midiOutputOption.getName(), midiOutputOption.get() );
    midi.addValue(midiChannel.getName(), midiChannel.get() );
    midi.addValue(midiScaleOption.getName(), midiScaleOption.get() );
    midi.addValue(midiScaleSize.getName(), midiScaleSize.get() );
    midi.addValue(midiScaleKey.getName(), midiScaleKey.get() );
    for (int i=0; i<midiScale.size(); i++) {
        midi.addValue(midiScale.at(i).getName(), midiScale.at(i).get() );
    }
    for (int i=0; i<midiThresholds.size(); i++) {
        midi.addValue(midiThresholds.at(i).getName(), midiThresholds.at(i).get() );
    }
    midi.addValue(midiTriggerOption.getName(), midiTriggerOption.get() );
    
    midi.addValue(midiOctaveOption.getName(), midiOctaveOption.get()  );
    midi.addValue(midiOctaveIntensity.getName(), midiOctaveIntensity.get() );
    midi.addValue(midiOctaveRef.getName(), midiOctaveRef.get() );
    
    midi.addValue(midiVelocityOption.getName(), midiVelocityOption.get() );
    midi.addValue(midiVelocitiesIntensity.getName(), midiVelocitiesIntensity.get() );
    midi.addValue(midiVelocitiesRef.getName(), midiVelocitiesRef.get() );
    
    midi.addValue(midiAfterTouchOption.getName(), midiAfterTouchOption.get() );
    midi.addValue(midiAfterTouchesIntensity.getName(), midiAfterTouchesIntensity.get() );
    midi.addValue(midiAfterTouchesRef.getName(), midiAfterTouchesRef.get() );
    
    midi.addValue(midiCcNum1.getName(), midiCcNum1.get() );
    midi.addValue(midiCcOption1.getName(), midiCcOption1.get() );
    midi.addValue(midiCcIntensity1.getName(), midiCcIntensity1.get() );
    midi.addValue(midiCcRef1.getName(), midiCcRef1.get() );
    
    midi.addValue(midiCcNum2.getName(), midiCcNum2.get() );
    midi.addValue(midiCcOption2.getName(), midiCcOption2.get() );
    midi.addValue(midiCcIntensity2.getName(), midiCcIntensity2.get() );
    midi.addValue(midiCcRef2.getName(), midiCcRef2.get() );
    
    preset.addXml(midi);
    
    //SC_PANEL
    ofXml scPanel;
    scPanel.addChild("SC_PANEL");
    scPanel.addValue(scServerAddr.getName(), scServerAddr.get() );
    scPanel.addValue(scServerPort.getName(), scServerPort.get() );
    scPanel.addValue(scInputName.getName(), scInputName.get() );
    scPanel.addValue(scOutputName.getName(), scOutputName.get() );
    scPanel.addValue(scSamplerate.getName(), scSamplerate.get() );
    scPanel.addValue(scMemSize.getName(), scMemSize.get() );
    scPanel.addValue(scBlockSize.getName(), scBlockSize.get() );
    scPanel.addValue(scNumInputs.getName(), scNumInputs.get() );
    scPanel.addValue(scNumOutputs.getName(), scNumOutputs.get() );
    
    scPanel.addValue(watchSc.getName(), watchSc.get() );
    
    //iFFT
    scPanel.addValue(iFFTbufNum.getName(), iFFTbufNum.get() );
    scPanel.addValue(iFFTphasesBufNum.getName(), iFFTphasesBufNum.get() );
    scPanel.addValue(iFFTpatchName.getName(), iFFTpatchName.get() );
    
    //Synth
    scPanel.addValue( synthPatchName.getName(), synthPatchName.get() );
    scPanel.addValue(synthFreqBufnum.getName(), synthFreqBufnum.get() );
    scPanel.addValue(synthGateBufnum.getName(), synthGateBufnum.get() );
    scPanel.addValue(synthMod1BufNum.getName(), synthMod1BufNum.get() );
    scPanel.addValue(synthMod2BufNum.getName(), synthMod2BufNum.get() );
    scPanel.addValue(synthMod3BufNum.getName(), synthMod3BufNum.get() );
    scPanel.addValue(synthMod4BufNum.getName(), synthMod4BufNum.get() );
    scPanel.addValue(synthVolsBufnum.getName(), synthVolsBufnum.get() );
    
    for (int i = 0; i < scSynthModsLabels.size(); i++) {
        scPanel.addValue(scSynthModsLabels.at(i).getName(), scSynthModsLabels.at(i).get()  );
    }
    
    for (int i = 0; i < scSynthStaticsLabels.size(); i++) {
        scPanel.addValue(scSynthStaticsLabels.at(i).getName(), scSynthStaticsLabels.at(i).get()  );
    }
    
    preset.addXml(scPanel);
    
    //SC_iFFT
    ofXml ifft;
    ifft.addChild("iFFT");
    ifft.addValue(iFFTon.getName(), iFFTon.get() );
    ifft.addValue(iFFTmagOption.getName(), iFFTmagOption.get() );
    ifft.addValue(iFFTphaseOption.getName(), iFFTphaseOption.get() );
    
    ifft.addValue(iFFTampModOption.getName(), iFFTampModOption.get() );
    ifft.addValue(iFFTampModIntensity.getName(), iFFTampModIntensity.get() );
    ifft.addValue(iFFTampRef.getName(), iFFTampRef.get() );
    
    ifft.addValue(iFFTloHiModOption.getName(), iFFTloHiModOption.get() );
    ifft.addValue(iFFTloHiModIntensity.getName(), iFFTloHiModIntensity.get() );
    ifft.addValue(iFFTloHiRef.getName(), iFFTloHiRef.get() );
    
    ifft.addValue(iFFThiShiftModOption.getName(), iFFThiShiftModOption.get() );
    ifft.addValue(iFFThiShiftModIntensity.getName(), iFFThiShiftModIntensity.get() );
    ifft.addValue(iFFThiShiftRef.getName(), iFFThiShiftRef.get() );
    
    ifft.addValue(iFFTwhiteMorphModOption.getName(), iFFTwhiteMorphModOption.get() );
    ifft.addValue(iFFTwhiteMorphModIntensity.getName(), iFFTwhiteMorphModIntensity.get() );
    ifft.addValue(iFFTwhiteMorphRef.getName(), iFFTwhiteMorphRef.get() );
    
    ifft.addValue(iFFTconvoMixModOption.getName(), iFFTconvoMixModOption.get() );
    ifft.addValue(iFFTconvoMixModIntensity.getName(), iFFTconvoMixModIntensity.get() );
    ifft.addValue(iFFTconvoMixRef.getName(), iFFTconvoMixRef.get() );
    
    
    
    preset.addXml(ifft);
    
    
    //SC_SYNTH
    ofXml synth;
    synth.addChild("SC_SYNTH");
    
    
    synth.addValue(scSynthOn.getName(), scSynthOn.get() );
    
    for (int i = 0; i<16;i++){
        synth.addValue(scFreqs.at(i).getName(), scFreqs.at(i).get() );
    }
    
    for (int i = 0; i < 16; i++){
        synth.addValue(scVolumes.at(i).getName(), scVolumes.at(i).get());
    }
    
    
    synth.addValue(scTriggerOption.getName(), scTriggerOption.get() );
    for (int i = 0; i < 16; i++){
        synth.addValue(scThresholds.at(i).getName(), scThresholds.at(i).get() );
    }
    
    synth.addValue(scVelocitiesOption.getName(), scVelocitiesOption.get() );
    synth.addValue( scVelocitiesIntensities.getName(), scVelocitiesIntensities.get() );
    synth.addValue(scVelocitiesRefs.getName(), scVelocitiesRefs.get() );
    
    synth.addValue(hNumber1.getName(), hNumber1.get() );
    synth.addValue(hPhase1.getName(), hPhase1.get() );
    synth.addValue(scH1Option.getName(), scH1Option.get() );
    synth.addValue(scH1Intensities.getName(), scH1Intensities.get() );
    synth.addValue(scH1Refs.getName(), scH1Refs.get() );
    
    synth.addValue(hNumber2.getName(), hNumber2.get() );
    synth.addValue(hPhase2.getName(), hPhase2.get() );
    synth.addValue(scH2Option.getName(), scH2Option.get() );
    synth.addValue(scH2Intensities.getName(), scH2Intensities.get() );
    synth.addValue(scH2Refs.getName(), scH2Refs.get() );
    
    synth.addValue(lfoOption.getName(), lfoOption.get() );
    synth.addValue(lfoRate.getName(), lfoRate.get() );
    synth.addValue(scLfoOption.getName(), scLfoOption.get() );
    synth.addValue(scLfoIntensities.getName(), scLfoIntensities.get() );
    synth.addValue(scLfoRefs.getName(), scLfoRefs.get() );
    
    synth.addValue(scAtt.getName(), scAtt.get() );
    synth.addValue(scDec.getName(), scDec.get() );
    synth.addValue(scSus.getName(), scSus.get() );
    synth.addValue(scRel.getName(), scRel.get() );
    synth.addValue(scMasterVolRef.getName(), scMasterVolRef.get() );
    
    preset.addXml(synth);
    
    
    
    
    string path =  "presets/" + name + ".vp";
    
    preset.save(path);
    
    videoPresetName = name;
    
    refreshVideoPresets();
    
    for (int i = 0; i < videoPresetsNames.size(); i++) {
        if (videoPresetsNames.at(i) == name ) {
            videoPresetNumber = i;
            break;
            
        }
    }
    
    
    
    ofLogNotice() << "[::::VIDEO_PRESET_SAVED::::]  : " + videoPresetName.get();
}


//-------------------------------------------------------------------------------------------------------------------------

void ofApp::refreshMidiPortList() {
    
    ofxMidiOut::listPorts();
    midiOuputNames = ofxMidiOut::getPortList();
    
    bool found = false;
    for (int i = 0; i < midiOuputNames.size(); i++) {
        
        if (midiOutputName.get() == midiOuputNames.at(i)) {
            midiOutputOption = i;
            found = true;
            break;
        }
        
    }
    
    if (!found){
        midiOutputOption = 0;
        midiOutputName = midiOuputNames.at(0);
        
    }
    
    
}

//-------------------------------------------------------------------------------------------------------------------------


void ofApp::changeMidiPort(int p) {
    midiOut.closePort();
    midiOut.openPort(p);
    midiOutputOption = p;
    midiOutputName = midiOuputNames.at(p);
}

//-------------------------------------------------------------------------------------------------------------------------
void ofApp::allNotesOff() {
    for (int i = 0; i < 128; i++) {
        midiOut.sendNoteOff(midiChannel,i, 0);
    }
}

void ofApp::sendMidi() {
    
    for (int i = 0; i < midiScale.size(); i++) {
        
        int newTrigger = (int)( midiTriggersBuf.at(i)>=midiThresholds.at(i) );
        int trigger = (int)midiTriggers.at(i);
        
        if (newTrigger > trigger ) {
            //noteOn
            int note = midiScale.at(i) + (midiOctaveRef*12) + (iround( (midiOctaves.at(i)-0.5)*2*midiOctaveIntensity )*12 );
            int velocity = iround(midiVelocities.at(i)*127 );
            midiOut.sendNoteOn(midiChannel, note,  velocity);
            currentNotes.at(i) = note;
        }
        
        if ( newTrigger < trigger ) {
            //noteOff
            midiOut.sendNoteOff(midiChannel, currentNotes.at(i), 0);
            
        }
        
        if(newTrigger == 1){
            //afterTouches
            int polytouch = rint(midiAfterTouches.at(i)*127);
            midiOut.sendPolyAftertouch(midiChannel, currentNotes.at(i), polytouch);
            midiTriggers.at(i)=true;
        }
        else midiTriggers.at(i)=false;
        
        
        
        
    }
    
    midiOut.sendControlChange(midiChannel, midiCcNum1, midiCc1);
    midiOut.sendControlChange(midiChannel, midiCcNum2, midiCc2);
    
    
    
}



//-------------------------------------------------------------------------------------------------------------------------


//::::	AUDIO SETUP LOAD & SAVE


//-------------------------------------------------------------------------------------------------------------------------

void ofApp::setupAudio() {
    
    //-------------------------------------------------//AUDIO SETTINGS
    
    
    sampleRate.set("Samplerate", 44100);
    bufferSize.set("Buffersize", 2048);
    numBuf.set("Num_Buffer", 2);
    
    audioInputDeviceName.set("Audio_Input", "Soundflower (2ch)");
    audioInputDevice.set("Audio_Input_Num", 1);
    
    audioExtInputDeviceName.set("Ext_Audio_Input", "Built-in Input");
    audioExtInputDevice.set("Ext_Audio_Input_Num", 0);
    
    audioOutputDeviceName.set("Audio_Output", "Built-in Output");
    audioOutputDevice.set("Audio_Output_Num", 0);
    
    extInputVol.set("Ext_Input_Vol", 0, 0, 2);
    
    
    
    sendAudioOsc.set("Send_OSC", false);
    oscAudioDataOutputAddr.set("OSC_Out_Addr", "127.0.0.1");
    oscAudioDataOutputPort.set("OSC_Out_Port", 57130);
    
    //    audioProcessingOn.set("Compressor_On_Off", 1);
    //    //eq
    //    lowcut_0_1.set("LowCut",20,  20, 1000);
    //    highcut_0_2.set("HighCut",20,  1, 20);
    //    lowgain_0_3.set("LowGain",0, 0, 10);
    //    midgain_0_4.set("MidGain",0, 0, 10);
    //    highgain_0_5.set("HighGain",0, 0, 10);
    //    //comp
    //    preAmp.set("Pre_Amp", 1.0f, 1.0f, 4.0f);
    //    threshold.set("Threshold", 0.7f, 0.0f, 1.0f);
    //    ratio.set("Ratio", 3.5f, 1.0f, 5.0f);
    //    makeUp.set("Make_Up", 3.0f, 0.0f, 4.0f);
    
    volume.set("Volume", 0.0f, -40, 40);
    //    //pitchshift
    
    pitchCompensation.set("PitchCompensation", 0, 0, 2);
    
    //    pitchshift_2_0.set("PitchShift", 0, 0 , 1);
    
    
    
    //---------------------------------------------//AUDIO DATA SETTINGS
    
    audioAnalyseOn.set("Analysis_On_Off", 1);
    stereoGradient.set("Stereo_Gradient", 0);
    
    rmsAmp.set("RMS_Amp", 8.0f, 1.0f, 20.0f);
    rmsSmooth.set("RMS_Smooth", 0.7f, 0.0f, 1.0f);
    powerSmooth.set("Power_Smooth", 0.7f, 0.0f, 1.0f);
    pitchFreqSmooth.set("PichFreq_Smooth", 0.7f, 0.0f, 1.0f);
    pitchConfSmooth.set("PitchConfidence_SMooth", 0.7f, 0.0f, 1.0f);
    pitchSalienceSmooth.set("Pitch_Salience_SMooth", 0.7f, 0.0f, 1.0f);
    hfcSmooth.set("HFC_Smooth", 0.7f, 0.0f, 1.0f);
    specCompSmooth.set("Spectral_Complexity_Smooth", 0.7f, 0.0f, 1.0f);
    centroidSmooth.set("Centroid_Smooth", 0.7f, 0.0f, 1.0f);
    inharmonicitySmooth.set("Inharmonicity_Smooth", 0.7f, 0.0f, 1.0f);
    dissonanceSmooth.set("Dissonance_Smooth", 0.7f, 0.0f, 1.0f);
    rollOffSmooth.set("Roll_Off_Smooth", 0.7f, 0.0f, 1.0f);
    oddToEvenSmooth.set("Odd_To_Even_Smooth", 0.7f, 0.0f, 1.0f);
    strongPeakSmooth.set("Strong_Peak_Smooth", 0.7f, 0.0f, 1.0f);
    strongPeakAmp.set("Strong_Peak_Amp", 2.0f, 0.5f, 4.0f);
    strongDecaySmooth.set("Strong_Decay_Smooth", 0.7f, 0.0f, 1.0f);
    
    spectrumSmooth.set("Spectrum_Smooth", 0.7f, 0.0f, 1.0f);
    melBandsSmooth.set("MEL_Bands_Smooth", 0.7f, 0.0f, 1.0f);
    mfccSmooth.set("MFCC_Smooth", 0.7f, 0.0f, 1.0f);
    hpcpSmooth.set("HPCP_Smooth", 0.7f, 0.0f, 1.0f);
    tristimulusSmooth.set("Tristimulus_Smooth", 0.7f, 0.0f, 1.0f);
    multiPitchesSmooth.set("MultiPitches_Smooth", 0.7f, 0.0f, 1.0f);
    
    
    //---------------------------------------------//AUDIO MAPPING SETTINGS
    aStartFreq.set("A_START_F", 0, 0, 1023);
    aEndFreq.set("A_END_F", 500, 3, 1023);
    cStartFreq.set("C_START_F", 0, 0, 1023);
    cEndFreq.set("C_END_F", 1023, 3, 1023);
    mStartBand.set("M_START_B", 0, 0, 23);
    mEndBand.set("M_END_B", 20, 3, 23);
    
    globalRotation.set("Global_Rotation", 11, 0, 11);
    globalBrightness.set("Global_Brightness", 11, 0, 11);
    globalSaturation.set("Global_Saturation", 11, 0, 11);
    globalAlpha.set("Global_Alpha", 11, 0, 11);
    globalContrast.set("Global_Contrast", 11, 0, 11);
    
    globalRotationAdjust.set("Global_Rotation_Intensity", 0.0f, -2.0f, 2.0f);
    globalBrightnessAdjust.set("Global_Brightness_Intensity", 0.0f, -2.0f, 2.0f);
    globalSaturationAdjust.set("Global_Saturation_Intensity", 0.0f, -2.0f, 2.0f);
    globalAlphaAdjust.set("Global_Alpha_Intensity", 0.0f, -2.0f, 2.0f);
    globalContrastAdjust.set("Global_Contrast_Intensity", 0.0f, -2.0f, 2.0f);
    
    perBandPresence.set("Per_Band_Presence_1", 0, 0, 5);
    perBandPresence2.set("Per_Band_Presence_2", 1, 0, 5);
    compensation.set("Compensation", 0, 0, 1);
    perBandRotation.set("Per_Band_Rotation", 5, 0, 5);
    perBandBrightness.set("Per_Band_Brightness", 2, 0, 5);
    perBandSaturation.set("Per_Band_Saturation", 5, 0, 5);
    perBandAlpha.set("Per_Band_Alpha", 5, 0, 5);
    
    perBandPresenceAdjust.set("Per_Band_Presence_Ratio", 0.0f, 0.0f, 1.0f);
    perBandRotationAdjust.set("Per_Band_Rotation_Intensity", 0.0f, -2.0f, 2.0f);
    perBandBrightnessAdjust.set("Per_Band_Brightness_Intensity", 0.0f, -2.0f, 2.0f);
    perBandSaturationAdjust.set("Per_Band_Saturation_Intensity", 0.0f, -2.0f, 2.0f);
    perBandAlphaAdjust.set("Per_Band_Alpha_Intensity", 0.0f, -2.0f, 2.0f);
    
    
    audioVideoInputOption.set("Video_Input", 2, 0, videoInputOptions.size() );
    audioVideoUsbInputOption.set("USB_Cam", 0, 0, camNames.size() );
    
    audioVideoBrightness.set("Brightness", 0.0f, 0.0f, 5.0f);
    audioVideoContrast.set("Contrast", 0.0f, 0.0f, 5.0f);
    audioVideoSaturation.set("Saturation", 0.0f, 0.0f, 5.0f);
    audioVideoAlpha.set("Alpha", 0.0f, 0.0f, 2.0f);
    
    avBModOption.set("Video_Brightness_Option", 12, 0, 12);
    avCModOption.set("Video_Contrast_Option", 12, 0, 12);
    avSModOption.set("Video_Saturation_Option", 12, 0, 12);
    avAModOption.set("Video_Alpha_Option", 12, 0, 12);
    
    avBModIntensity.set("Video_BrightnessMod_Intensity", 0.0f, -2.0f, 2.0f);
    avCModIntensity.set("Video_ContrastMod_Intensity", 0.0f, -2.0f, 2.0f);
    avSModIntensity.set("Video_SaturationMod_Intensity", 0.0f, -2.0f, 2.0f);
    avAModIntensity.set("Video_AlphaMod_Intensity", 0.0f, -2.0f, 2.0f);
    
    audioVideoZoom.set("Zoom", 1.0f, 0.3f, 4.0f);
    audioVideoPositionX.set("Position_X", 0.0f, -1.0f, 1.0f);
    audioVideoPositionY.set("Position_Y", 0.0f, -1.0f, 1.0f);
    audioVideoCrop.set("Crop", 0.0f, 0.0f, 1.0f);
    
    varispeedOption.set("Varispeed_Option", 0, 0, 11);
    varispeedOn.set("Varispeed_On_Off", 0, 0, 1);
    varispeedIntensity.set("Varispeed_Intensity", 0, -1.0f, 1.0f);
    varispeedRef.set("Varispeed_Ref", 1.0f, 0.1f, 2.0f);
    
    
    blendMode.set("Blend_Mode", 0, 0, blendModeOptions.size() );
    swap.set("Swap", false);
    audioVideoMix.set("Mix", 0.0f, 0.0f, 1.0f);
    
    //gradient
    refRotation.set("Gradient_Rotation", 0.0f, 0.0f, 2.0f);
    refBrightness.set("Gradient_Brightness", 1.0f, 0.0f, 2.0f);
    refSaturation.set("Gradient_Saturation", 1.0f, 0.0f, 2.0f);
    refAlpha.set("Gradient_Alpha", 0.5f, 0.0f, 2.0f);
    refContrast.set("Gradient_Contrast", 0.0f, 0.0f, 5.0f);
    invertGradient.set("Gradient_Inversion", 0, 0, 1);
    
    audioGuiView.set("AUDIO_GUI_VIEW", 1, 0, 3);
    
    
    ofLogNotice() << "[:::: SETUP : DEFAULT AUDIO PARAMETER LOADED ::::]";
    
    
    
    refreshAudioDeviceList();
    
    loadAudioPresetsList();
    
    loadAudioPreset(audioPresetName);
    
    
    //-------------------------------------------//INIT AUDIO STUFF
    
    
    audioAnalyseIsOn = audioAnalyseOn.get();
    audioProcessingIsOn = audioProcessingOn.get();
    
    _sampleRate = sampleRate.get();
    _bufferSize = bufferSize.get();
    _numBuf = numBuf;
    
    memoryFFT.allocate(1280, 1024, OF_PIXELS_RGB);
    
    audioDataBuffer.allocate(bufferSize, 2);
    audioDataBuffer.set(0.0f);
    
    //    transferAudioBuffer.allocate(bufferSize, 2);
    //    transferAudioBuffer.set(0.0f);
    //    audioOutputBuffer.allocate(bufferSize, 2);
    //    audioOutputBuffer.set(0.0f);
    
    //audioAnalyzer.setup(sampleRate.get(), bufferSize.get(), 2);
    //audioAnalyzer.setActive(0, MULTI_PITCHES, FALSE);
    //audioAnalyzer.setActive(0, PITCH_SALIENCE_FUNC_PEAKS, FALSE);
    //audioAnalyzer.setSalienceFunctionPeaksParameters(0, 6);
    
    audioAnalyzerLeft.setup(sampleRate.get(), bufferSize.get(), 1);
    audioAnalyzerRight.setup(sampleRate.get(), bufferSize.get(), 1);
    
    spectrumNorm.resize(1025);
    spectrumNorm2.resize(1025);
    melBandsNorm.resize(24);
    melBandsNorm2.resize(24);
    mfccNorm.resize(13);
    mfccNorm2.resize(13);
    hpcpNorm.resize(12);
    hpcpNorm2.resize(12);
    tristimulusNorm.resize(3);
    tristimulusNorm2.resize(3);
    
    zeroFloats.resize(12);
    for (int i = 0; i < 12; i++)  {  zeroFloats.at(i) = 0; }
    
    
    //gradientSetup
    
    colorSpectrum = getSpectrum(1024, false);
    colorSpectrumTex.allocate(1024, 1, GL_RGB);
    colorSpectrumTex.loadData(colorSpectrum);
    lookupHues = getHueSpectrum(colorSpectrum);
    huesIndexes = getHuesToSpectrum(colorSpectrum);
    
    audioVideoFbo.allocate(1920, 1080, GL_RGBA);
    
    gradientFbo.allocate(1920, 1080, GL_RGBA);
    
    broadcastFbo.allocate(1920, 1080, GL_RGBA);
    
    audioScreen.allocate(1280, 720, GL_RGBA); // (2x1280x7Ž0)
    
    audioGram.allocate(1280, 520, OF_PIXELS_GRAY);
    audioGramTex.allocate( audioGram, GL_LUMINANCE);
    
    soundGradientPix.allocate(2, 1080, OF_PIXELS_RGBA);
    soundGradientTex.allocate(2, 1080, GL_RGBA);
    
    avBCSA.load("shaders/bcsa.vert", "shaders/bcsa.frag");
    avBlend.load("shaders/blend.vert", "shaders/blend.frag");
    
    
    audioVideoCam = &audioVideoCamera;
    lastAsource = audioVideoInputOption;
    
    lastAcam = 99;
    
    
    ofLogNotice() << "[:::: SETUP : AUDIO OBJECTS LOADED ::::] : ";
    
    
    
    
    //END SETUP AUDIO
}


//--------------------------------------------------------------------------------------

void ofApp::loadAudioPresetsList() {
    
    ofDirectory dir( ofToDataPath("") + "presets/" );
    
    dir.allowExt("ap");
    
    dir.listDir();
    
    audioPresetsNames.resize(dir.size() );
    
    for (int i = 0; i < dir.size(); i++ ) {
        ofFile file = dir.getFile(i);
        audioPresetsNames.at(i) = file.getBaseName();
        ofLogNotice() << "Preset found :" << audioPresetsNames.at(i);
        
    }
    
}


//--------------------------------------------------------------------------------------

void ofApp::refreshAudioPresets() {
    
    ofDirectory dir( ofToDataPath("") + "presets/" );
    
    dir.allowExt("ap");
    
    dir.listDir();
    
    audioPresetsNames.clear();
    audioPresetsNames.resize(dir.size() );
    
    for (int i = 0; i < dir.size(); i++ ) {
        ofFile file = dir.getFile(i);
        audioPresetsNames.at(i) = file.getBaseName();
        
    }
    
    
}


//--------------------------------------------------------------------------------------


void ofApp::saveAudioPreset(string name) {
    
    ofLogNotice() << "[:::: SAVING AUDIO PRESET ::::] : " + name;
    
    ofXml preset;
    preset.addChild("AUDIO_SETUP");
    
    ofXml set;
    set.addChild("AUDIO_SETTINGS");
    set.addValue(sampleRate.getName(), sampleRate.get() );
    set.addValue(bufferSize.getName(), bufferSize.get() );
    set.addValue(numBuf.getName(), numBuf.get() );
    
    set.addValue(audioInputDeviceName.getName(), audioInputDeviceName.get() );
    set.addValue(audioInputDevice.getName(), audioInputDevice.get() );
    set.addValue(audioExtInputDeviceName.getName(), audioExtInputDeviceName.get() );
    set.addValue(audioExtInputDevice.getName(), audioExtInputDevice.get() );
    set.addValue(audioOutputDeviceName.getName(), audioOutputDeviceName.get() );
    set.addValue(audioOutputDevice.getName(), audioOutputDevice.get() );
    
    set.addValue(extInputVol.getName(), extInputVol.get() );
    
    
    //set.addValue(autoStartAudio.getName(), autoStartAudio.get() );
    
    set.addValue(sendAudioOsc.getName(), sendAudioOsc.get() );
    set.addValue(oscAudioDataOutputAddr.getName(), oscAudioDataOutputAddr.get() );
    set.addValue(oscAudioDataOutputPort.getName(), oscAudioDataOutputPort.get() );
    
    //    set.addValue(audioProcessingOn.getName(), audioProcessingOn.get() );
    //
    //    //eq
    //    set.addValue(lowcut_0_1.getName(), lowcut_0_1.getName()  );
    //    set.addValue(highcut_0_2.getName(), highcut_0_2.getName() );
    //    set.addValue(lowgain_0_3.getName(), lowgain_0_3.getName() );
    //    set.addValue(midgain_0_4.getName(), midgain_0_4.getName() );
    //    set.addValue(highgain_0_5.getName(), highgain_0_5.getName() );
    //    //comp
    //    set.addValue(preAmp.getName(), preAmp.get() );
    //    set.addValue(threshold.getName(), threshold.get() );
    //    set.addValue(ratio.getName(), ratio.get() );
    //    set.addValue(makeUp.getName(), makeUp.get() );
    set.addValue(volume.getName(), volume.get() );
    //    //pitchshift
    set.addValue(pitchCompensation.getName(), pitchCompensation.getName() );
    //    set.addValue(pitchshift_2_0.getName(), pitchshift_2_0.getName() );
    
    preset.addXml(set);
    
    ofXml datset;
    datset.addChild("AUDIO_DATA_SETTINGS");
    datset.addValue(audioAnalyseOn.getName(), audioAnalyseOn.get() );
    datset.addValue(stereoGradient.getName(), stereoGradient.get() );
    datset.addValue(rmsAmp.getName(), rmsAmp.get() );
    datset.addValue(rmsSmooth.getName(), rmsSmooth.get() );
    datset.addValue(powerSmooth.getName(), powerSmooth.get() );
    datset.addValue(pitchFreqSmooth.getName(), pitchFreqSmooth.get() );
    datset.addValue(pitchConfSmooth.getName(), pitchFreqSmooth.get() );
    datset.addValue(pitchSalienceSmooth.getName(), pitchSalienceSmooth.get() );
    datset.addValue(hfcSmooth.getName(), hfcSmooth.get() );
    datset.addValue(specCompSmooth.getName(), specCompSmooth.get() );
    datset.addValue(centroidSmooth.getName(), centroidSmooth.get() );
    datset.addValue(inharmonicitySmooth.getName(), inharmonicitySmooth.get() );
    datset.addValue(dissonanceSmooth.getName(), dissonanceSmooth.get() );
    datset.addValue(rollOffSmooth.getName(), rollOffSmooth.get() );
    datset.addValue(oddToEvenSmooth.getName(), oddToEvenSmooth.get() );
    datset.addValue(strongPeakSmooth.getName(), strongPeakSmooth.get() );
    datset.addValue(strongPeakAmp.getName(), strongPeakAmp.get() );
    datset.addValue(strongDecaySmooth.getName(), strongDecaySmooth.get() );
    
    datset.addValue(spectrumSmooth.getName(), spectrumSmooth.get() );
    datset.addValue(melBandsSmooth.getName(), melBandsSmooth.get() );
    datset.addValue(mfccSmooth.getName(), mfccSmooth.get() );
    datset.addValue(hpcpSmooth.getName(), hpcpSmooth.get() );
    datset.addValue(tristimulusSmooth.getName(), tristimulusSmooth.get() );
    datset.addValue(multiPitchesSmooth.getName(), multiPitchesSmooth.get() );
    
    preset.addXml(datset);
    
    ofXml mapping;
    mapping.addChild("MAPPING");
    mapping.addValue(aStartFreq.getName(), aStartFreq.get() );
    mapping.addValue(aEndFreq.getName(), aEndFreq.get() );
    mapping.addValue(cStartFreq.getName(), cStartFreq.get() );
    mapping.addValue(cEndFreq.getName(), cEndFreq.get() );
    mapping.addValue(mStartBand.getName(), mStartBand.get() );
    mapping.addValue(mEndBand.getName(), mEndBand.get() );
    
    
    mapping.addValue(globalRotation.getName(), globalRotation.get() );
    mapping.addValue(globalRotationAdjust.getName(), globalRotationAdjust.get() );
    mapping.addValue(globalBrightness.getName(), globalBrightness.get() );
    mapping.addValue(globalBrightnessAdjust.getName(), globalBrightnessAdjust.get() );
    mapping.addValue(globalSaturation.getName(), globalSaturation.get() );
    mapping.addValue(globalSaturationAdjust.getName(), globalSaturationAdjust.get() );
    mapping.addValue(globalAlpha.getName(), globalAlpha.get() );
    mapping.addValue(globalAlphaAdjust.getName(), globalAlphaAdjust.get() );
    mapping.addValue(globalContrast.getName(), globalContrast.get() );
    mapping.addValue(globalContrastAdjust.getName(), globalContrastAdjust.get() );
    
    mapping.addValue(perBandPresence.getName(), perBandPresence.get() );
    mapping.addValue(perBandPresence2.getName(), perBandPresence2.get() );
    mapping.addValue(perBandPresenceAdjust.getName(), perBandPresenceAdjust.get() );
    mapping.addValue(compensation.getName(), compensation.get() );
    
    mapping.addValue(perBandRotation.getName(), perBandRotation.get() );
    mapping.addValue(perBandRotationAdjust.getName(), perBandRotationAdjust.get() );
    mapping.addValue(perBandBrightness.getName(), perBandBrightness.get() );
    mapping.addValue(perBandBrightnessAdjust.getName(), perBandBrightnessAdjust.get() );
    mapping.addValue(perBandSaturation.getName(), perBandSaturation.get() );
    mapping.addValue(perBandSaturationAdjust.getName(), perBandSaturationAdjust.get() );
    mapping.addValue(perBandAlpha.getName(), perBandAlpha.get() );
    mapping.addValue(perBandAlphaAdjust.getName(), perBandAlphaAdjust.get() );
    
    preset.addXml(mapping);
    
    ofXml broad;
    broad.addChild("BLENDING");
    broad.addValue(audioVideoInputOption.getName(), audioVideoInputOption.get() );
    broad.addValue(audioVideoUsbInputOption.getName(), audioVideoUsbInputOption.get() );
    
    broad.addValue(audioVideoBrightness.getName(), audioVideoBrightness.get() );
    broad.addValue(audioVideoContrast.getName(), audioVideoContrast.get() );
    broad.addValue(audioVideoSaturation.getName(), audioVideoSaturation.get() );
    broad.addValue(audioVideoAlpha.getName(), audioVideoAlpha.get() );
    
    broad.addValue(avBModOption.getName(), avBModOption.get() );
    broad.addValue(avCModOption.getName(), avCModOption.get() );
    broad.addValue(avSModOption.getName(), avSModOption.get() );
    broad.addValue(avAModOption.getName(), avAModOption.get() );
    
    broad.addValue(avBModIntensity.getName(), avBModIntensity.get() );
    broad.addValue(avCModIntensity.getName(), avCModIntensity.get() );
    broad.addValue(avSModIntensity.getName(), avSModIntensity.get() );
    broad.addValue(avAModIntensity.getName(), avAModIntensity.get() );
    
    broad.addValue(audioVideoZoom.getName(), audioVideoZoom.get() );
    broad.addValue(audioVideoPositionX.getName(), audioVideoPositionX.get() );
    broad.addValue(audioVideoPositionY.getName(), audioVideoPositionY.get() );
    broad.addValue(audioVideoCrop.getName(), audioVideoCrop.get() );
    
    broad.addValue(varispeedOption.getName(), varispeedOption.get() );
    broad.addValue(varispeedOn.getName(), varispeedOn.get() );
    broad.addValue(varispeedIntensity.getName(), varispeedIntensity.get() );
    broad.addValue(varispeedRef.getName(), varispeedRef.get() );
    broad.addValue(pitchCompensation.getName(), pitchCompensation.get() );
    
    broad.addValue(blendMode.getName(), blendMode.get() );
    broad.addValue(swap.getName(), swap.get() );
    broad.addValue(audioVideoMix.getName(), audioVideoMix.get() );
    
    broad.addValue(refRotation.getName(), refRotation.get() );
    broad.addValue(refBrightness.getName(), refBrightness.get() );
    broad.addValue(refSaturation.getName(), refSaturation.get() );
    broad.addValue(refAlpha.getName(), refAlpha.get() );
    broad.addValue(refContrast.getName(), refContrast.get() );
    broad.addValue(invertGradient.getName(), invertGradient.get() );
    
    preset.addXml(broad);
    
    string path = "presets/" + name + ".ap";
    
    preset.save(path);
    
    
    audioPresetName = name;
    
    refreshAudioPresets();
    
    for (int i = 0; i < audioPresetsNames.size(); i++) {
        if (audioPresetsNames.at(i) == name ) {
            audioPresetNumber = i;
            break;
            
        }
    }
    
    
    
    ofLogNotice() << "[::::AUDIO_SETUP::::] : SAVED = " + audioPresetName.get();
}


//--------------------------------------------------------------------------------------


void ofApp::loadAudioPreset(string name) {
    
    
    string presetPath = ofToDataPath("") + "presets/" + name + ".ap";
    
    ofFile file(presetPath);
    
    if (file.exists() ) {
        
        ofLogNotice() << "[:::: LOADING AUDIO PRESET ::::] : " + name;
        
        ofXml preset;
        preset.load(presetPath);
        
        
        //AUDIO SETTINGS
        sampleRate = preset.getIntValue("/AUDIO_SETTINGS/"+sampleRate.getName() );
        bufferSize = preset.getIntValue("/AUDIO_SETTINGS/" + bufferSize.getName() );
        numBuf = preset.getIntValue("/AUDIO_SETTINGS/" + numBuf.getName() );
        
        
        audioInputDeviceName = preset.getValue("/AUDIO_SETTINGS/" + audioInputDeviceName.getName() );
        for (int i=0; i<audioInputDeviceList.size(); i++ ) {
            if (audioInputDeviceList.at(i) == audioInputDeviceName.get() ) audioInputDevice = i;
        }
        
        audioExtInputDeviceName = preset.getValue("/AUDIO_SETTINGS/" + audioExtInputDeviceName.getName() );
        for (int i=0; i<audioInputDeviceList.size(); i++ ) {
            if (audioInputDeviceList.at(i) == audioExtInputDeviceName.get() ) audioExtInputDevice = i;
        }
        
        audioOutputDeviceName = preset.getValue("/AUDIO_SETTINGS/" + audioOutputDeviceName.getName() );
        for (int i=0; i<audioOutputDeviceList.size(); i++ ) {
            if (audioOutputDeviceList.at(i) == audioOutputDeviceName.get() ) audioOutputDevice = i;
        }
        
        extInputVol = preset.getFloatValue("/AUDIO/SETTINGS/" + extInputVol.getName() );
        
        //autoStartAudio = preset.getBoolValue("/AUDIO_SETTINGS/" + autoStartAudio.getName() );
        
        sendAudioOsc = preset.getBoolValue("/AUDIO_SETTINGS/" + sendAudioOsc.getName() );
        oscAudioDataOutputAddr = preset.getValue("/AUDIO_SETTINGS/" + oscAudioDataOutputAddr.getName() );
        oscAudioDataOutputPort = preset.getIntValue("/AUDIO_SETTINGS/" + oscAudioDataOutputPort.getName() );
        
        
        //        audioProcessingOn = preset.getIntValue("/AUDIO_SETTINGS/" + audioProcessingOn.getName() );
        //
        //        //eq
        //        lowcut_0_1 = preset.getFloatValue("/AUDIO_SETTINGS/" + lowcut_0_1.getName() );
        //        highcut_0_2 = preset.getFloatValue("/AUDIO_SETTINGS/" + highcut_0_2.getName() );
        //        lowgain_0_3 = preset.getFloatValue("/AUDIO_SETTINGS/" + lowgain_0_3.getName() );
        //        midgain_0_4 = preset.getFloatValue("/AUDIO_SETTINGS/" + midgain_0_4.getName() );
        //        highgain_0_5 = preset.getFloatValue("/AUDIO_SETTINGS/" + highgain_0_5.getName() );
        //        //comp
        //        preAmp = preset.getFloatValue("/AUDIO_SETTINGS/" + preAmp.getName() );
        //        threshold = preset.getFloatValue("/AUDIO_SETTINGS/" + threshold.getName() );
        //        ratio = preset.getFloatValue("/AUDIO_SETTINGS/" + ratio.getName() );
        //        makeUp = preset.getFloatValue("/AUDIO_SETTINGS/" + makeUp.getName() );
        
        volume = preset.getFloatValue("/AUDIO_SETTINGS/" + volume.getName() );
        
        //        //pitcshift
        
        pitchCompensation = preset.getFloatValue("/AUDIO_SETTINGS/" + pitchCompensation.getName() );
        
        //        pitchshift_2_0 = preset.getFloatValue("/AUDIO_SETTINGS/" + pitchshift_2_0.getName() );
        
        
        
        //AUDIO DATA SETTINGS
        audioAnalyseOn = preset.getIntValue("/AUDIO_DATA_SETTINGS/" + audioAnalyseOn.getName() );
        stereoGradient = preset.getIntValue("/AUDIO_DATA_SETTINGS/" + stereoGradient.getName() );
        
        rmsAmp = preset.getFloatValue("/AUDIO_DATA_SETTINGS/" + rmsAmp.getName() );
        rmsSmooth = preset.getFloatValue("/AUDIO_DATA_SETTINGS/" + rmsSmooth.getName() );
        powerSmooth = preset.getFloatValue("/AUDIO_DATA_SETTINGS/" + powerSmooth.getName() );
        pitchFreqSmooth = preset.getFloatValue("/AUDIO_DATA_SETTINGS/"+ pitchFreqSmooth.getName() );
        pitchConfSmooth = preset.getFloatValue("/AUDIO_DATA_SETTINGS/"+ pitchConfSmooth.getName() );
        pitchSalienceSmooth = preset.getFloatValue("/AUDIO_DATA_SETTINGS/" + pitchSalienceSmooth.getName() );
        hfcSmooth = preset.getFloatValue("/AUDIO_DATA_SETTINGS/" + hfcSmooth.getName() );
        specCompSmooth = preset.getFloatValue("/AUDIO_DATA_SETTINGS/" + specCompSmooth.getName() );
        centroidSmooth = preset.getFloatValue("/AUDIO_DATA_SETTINGS/" + centroidSmooth.getName() );
        inharmonicitySmooth = preset.getFloatValue("/AUDIO_DATA_SETTINGS/" + inharmonicitySmooth.getName() );
        dissonanceSmooth = preset.getFloatValue("/AUDIO_DATA_SETTINGS/" + dissonanceSmooth.getName() );
        rollOffSmooth = preset.getFloatValue("/AUDIO_DATA_SETTINGS/" + rollOffSmooth.getName() );
        oddToEvenSmooth = preset.getFloatValue("/AUDIO_DATA_SETTINGS/" + oddToEvenSmooth.getName() );
        strongPeakSmooth = preset.getFloatValue("/AUDIO_DATA_SETTINGS/" + strongPeakSmooth.getName()  );
        strongPeakAmp = preset.getFloatValue("/AUDIO_DATA_SETTINGS/" + strongPeakAmp.getName()  );
        strongDecaySmooth = preset.getFloatValue("/AUDIO_DATA_SETTINGS/" + strongDecaySmooth.getName() );
        
        spectrumSmooth = preset.getFloatValue("/AUDIO_DATA_SETTINGS/" + spectrumSmooth.getName() );
        melBandsSmooth = preset.getFloatValue("/AUDIO_DATA_SETTINGS/" + melBandsSmooth.getName() );
        mfccSmooth = preset.getFloatValue("/AUDIO_DATA_SETTINGS/" + mfccSmooth.getName() );
        hpcpSmooth = preset.getFloatValue("/AUDIO_DATA_SETTINGS/" + hpcpSmooth.getName() );
        tristimulusSmooth = preset.getFloatValue("/AUDIO_DATA_SETTINGS/" + tristimulusSmooth.getName() );
        multiPitchesSmooth = preset.getFloatValue("/AUDIO_DATA_SETTINGS/" + multiPitchesSmooth.getName() );
        
        //AUDIO MAPPING
        aStartFreq = preset.getIntValue("/MAPPING/" + aStartFreq.getName() );
        aEndFreq = preset.getIntValue("/MAPPING/" + aEndFreq.getName() );
        cStartFreq = preset.getIntValue("/MAPPING/" + cStartFreq.getName() );
        cEndFreq = preset.getIntValue("/MAPPING/" + cEndFreq.getName() );
        mStartBand = preset.getIntValue("/MAPPING/" + mStartBand.getName() );
        mEndBand = preset.getIntValue("/MAPPING/" + mEndBand.getName() );
        
        globalRotation = preset.getIntValue( "/MAPPING/" + globalRotation.getName() );
        globalBrightness = preset.getIntValue( "/MAPPING/" + globalBrightness.getName() );
        globalContrast = preset.getIntValue( "/MAPPING/" + globalContrast.getName() );
        globalSaturation = preset.getIntValue( "/MAPPING/" + globalSaturation.getName() );
        globalAlpha = preset.getIntValue( "/MAPPING/" + globalAlpha.getName() );
        
        
        globalRotationAdjust = preset.getFloatValue( "/MAPPING/" + globalRotationAdjust.getName() );
        globalBrightnessAdjust  = preset.getFloatValue( "/MAPPING/" + globalBrightnessAdjust.getName() );
        globalContrastAdjust  = preset.getFloatValue( "/MAPPING/" + globalContrastAdjust.getName() );
        globalSaturationAdjust = preset.getFloatValue( "/MAPPING/" + globalSaturationAdjust.getName() );
        globalAlphaAdjust = preset.getFloatValue( "/MAPPING/" + globalAlphaAdjust.getName() );
        
        //perBandPresence = preset.getIntValue( "/MAPPING/" + perBandPresence.getName() );
        //perBandPresence2 = preset.getIntValue( "/MAPPING/" + perBandPresence2.getName() );
        compensation = preset.getFloatValue( "/MAPPING/" + compensation.getName() );
        perBandRotation = preset.getIntValue( "/MAPPING/" + perBandRotation.getName() );
        perBandBrightness = preset.getIntValue( "/MAPPING/" + perBandBrightness.getName() );
        perBandSaturation = preset.getIntValue( "/MAPPING/" + perBandSaturation.getName() );
        perBandAlpha = preset.getIntValue( "/MAPPING/" + perBandAlpha.getName() );
        
        perBandPresenceAdjust = preset.getFloatValue( "/MAPPING/" + perBandPresenceAdjust.getName() );
        perBandRotationAdjust = preset.getFloatValue( "/MAPPING/" + perBandRotationAdjust.getName() );
        perBandBrightnessAdjust = preset.getFloatValue( "/MAPPING/" + perBandBrightnessAdjust.getName() );
        perBandSaturationAdjust = preset.getFloatValue( "/MAPPING/" + perBandSaturationAdjust.getName() );
        perBandAlphaAdjust = preset.getFloatValue( "/MAPPING/" + perBandAlphaAdjust.getName() );
        
        //BLENDING
        int a = audioVideoInputOption;
        audioVideoInputOption = preset.getIntValue("/BLENDING/" + audioVideoInputOption.getName() );
        if(audioVideoInputOption == 2) {
            if (a==0 && setupComplete) stop();
        }
        if(audioVideoInputOption == 0) {
            if (a==2 && setupComplete) hap.stop();
        }
        
        audioVideoUsbInputOption = preset.getIntValue("/BLENDING/" + audioVideoUsbInputOption.getName() );
        
        audioVideoBrightness = preset.getFloatValue("/BLENDING/" + audioVideoBrightness.getName() );
        audioVideoContrast = preset.getFloatValue("/BLENDING/" + audioVideoContrast.getName() );
        audioVideoSaturation = preset.getFloatValue("/BLENDING/" + audioVideoSaturation.getName() );
        audioVideoAlpha = preset.getFloatValue("/BLENDING/" + audioVideoAlpha.getName() );
        
        avBModOption = preset.getIntValue("/BLENDING/" + avBModOption.getName() );
        avCModOption = preset.getIntValue("/BLENDING/" + avCModOption.getName() );
        avSModOption = preset.getIntValue("/BLENDING/" + avSModOption.getName() );
        avAModOption = preset.getIntValue("/BLENDING/" + avAModOption.getName() );
        
        avBModIntensity = preset.getFloatValue("/BLENDING/" + avBModIntensity.getName() );
        avCModIntensity = preset.getFloatValue("/BLENDING/" + avCModIntensity.getName() );
        avSModIntensity = preset.getFloatValue("/BLENDING/" + avSModIntensity.getName() );
        avAModIntensity = preset.getFloatValue("/BLENDING/" + avAModIntensity.getName() );
        
        audioVideoZoom = preset.getFloatValue("/BLENDING/" + audioVideoZoom.getName() );
        audioVideoPositionX = preset.getFloatValue("/BLENDING/" + audioVideoPositionX.getName() );
        audioVideoPositionY = preset.getFloatValue("/BLENDING/" + audioVideoPositionY.getName() );
        audioVideoCrop = preset.getFloatValue("/BLENDING/" + audioVideoCrop.getName() );
        
        varispeedOption = preset.getIntValue("/BLENDING/" + varispeedOption.getName() );
        varispeedOn = preset.getIntValue("/BLENDING/" + varispeedOn.getName() );
        varispeedIntensity = preset.getFloatValue("/BLENDING/" + varispeedIntensity.getName() );
        varispeedRef = preset.getFloatValue("/BLENDING/" + varispeedRef.getName() );
        pitchCompensation = preset.getFloatValue("/BLENDING/" + pitchCompensation.getName() );
        
        blendMode = preset.getIntValue("/BLENDING/" + blendMode.getName() );
        swap = preset.getBoolValue("/BLENDING/" + swap.getName());
        audioVideoMix = preset.getFloatValue("/BLENDING/" + audioVideoMix.getName() );
        
        refRotation = preset.getFloatValue( "/BLENDING/" + refRotation.getName() );
        refBrightness = preset.getFloatValue( "/BLENDING/" + refBrightness.getName() );
        refContrast = preset.getFloatValue( "/BLENDING/" + refContrast.getName() );
        refSaturation = preset.getFloatValue( "/BLENDING/" + refSaturation.getName() );
        refAlpha = preset.getFloatValue( "/BLENDING/" + refAlpha.getName() );
        invertGradient == preset.getIntValue( "/BLENDING/" + invertGradient.getName() );
        
        audioPresetName = name;
        
        
        for (int i = 0; i < audioPresetsNames.size(); i++) {
            if (audioPresetsNames.at(i) == name ) {
                audioPresetNumber = i;
                break;
                
            }
        }
        
        
    }
    
    
    ofLogNotice() << "[::::AUDIO_SETUP PRESET LOADED::::] : " + audioPresetName.get();
    
}

//--------------------------------------------------------------------------------------------
// UPDATE AUDIO

void ofApp::updateAudioVideo() {
    
    //audioVideoInputOptions = {"Movies", "USB", "Canon", "Syphon", "None"};
    
    switch (audioVideoInputOption){
            
            //MOVIES
        case 0 :
            
            if ( HAPmode) {
                
                if (hapStarted){
                    if (varispeedOn == 1 ) {
                        updateMovieSpeed();
                    }
                    hap.updateRMS( testRms );
                    audioVideoTex = hap.getTexture();
                }
                else audioVideoTex = &blackTex;
                
            }
            
            else if (playing) {
                if (moviePlayer.at(playerSelector).isLoaded() && moviePlayer.at(playerSelector).isFrameNew()) {
                    audioVideoTex = &moviePlayer.at(playerSelector).getTexture();
                }
            }
            
            
            break;
            
            //USB
        case 1 :
            if ( audioVideoCam -> isInitialized() ){
                if (audioVideoCam -> isFrameNew() ) {
                    audioVideoTex = &audioVideoCam -> getTexture();
                }
            }
            break;
            
            
            //SYPHON
        case 2 :
            //audioVideoTex = &syphonVtex;
            audioVideoTex = &blackTex;
            break;
            
            
            //NONE
        case 3 :
            audioVideoTex = &blackTex;
            break;
            
            
            
            
    }
    
    
    float w = audioVideoFbo.getWidth() * audioVideoZoom.get() ;
    float h = audioVideoFbo.getHeight() * audioVideoZoom.get() ;
    float x = audioVideoFbo.getWidth()/2*(1 + (1-audioVideoZoom.get())*audioVideoPositionX);
    float y =  audioVideoFbo.getHeight()/2*(1 + (1-audioVideoZoom.get())*audioVideoPositionY);
    
    
    if (audioVideoTex != NULL && audioVideoTex -> isAllocated() ){
        audioVideoFbo.begin();
        ofClear(0, 0, 0, 1);
        avBCSA.begin();
        avBCSA.setUniform3f("avgluma", 0.62,0.62,0.62);
        avBCSA.setUniform1f("contrast", avC);
        avBCSA.setUniform1f("brightness", avB);
        avBCSA.setUniform1f("saturation", ofClamp(avS, 0, 5) );
        avBCSA.setUniform1f("alpha", avA);
        avBCSA.setUniformTexture("image", *audioVideoTex, 1);
        
        
        audioVideoTex -> setAnchorPercent(0.5, 0.5);
        audioVideoTex -> draw(x, y, w, h);
        audioVideoTex -> setAnchorPercent(0.0, 0.0);
        
        
        avBCSA.end();
        audioVideoFbo.end();
    }
    
    
    
    
    
}


//--------------------------------------------------------------------------------------------
//Audio Analysis Gui

void ofApp::drawImGuiAudio() {
    
    
    if (ofxImGui::BeginWindow(audioPresetName, guiSettings, window_flags))
        
    {
        if (ImGui::Button("Save") ) {
            saveAudioPreset(audioPresetName);
            saveAudioUnitsPresets(audioPresetName);
        }
        ImGui::SameLine();
        ImGui::Text("::::");
        ImGui::SameLine();
        if (ofxImGui::AddParameter(volume) ){
            compressor.setParameter( 6, kAudioUnitScope_Global, volume);
            //AudioUnitSetParameter(compressor, 6, kAudioUnitScope_Global, volume, 0);
        }
        
        //DEVICES
        ImGui::Text("");
        if (ImGui::Button(": HPF :")) hpf.showUI();
        ImGui::SameLine();
        if (ImGui::Button(": LPF :")) lpf.showUI();
        ImGui::SameLine();
        if (ImGui::Button(": PITCH :")) pitchshift.showUI();
        ImGui::SameLine();
        if (ImGui::Button(": COMP :")) compressor.showUI();
        ImGui::Text("");
        
        if (ImGui::CollapsingHeader("Sound Device") )
            
        {   ImGui::Indent();
            ImGui::Text("");
            if (ImGui::TreeNode("Audio Input") )
            {
                ImGui::Text("");
                if (ofxImGui::AddRadio(audioInputDevice, audioInputDeviceList) ) audioInputDeviceName = audioInputDeviceList.at(audioInputDevice.get() );
                ImGui::TreePop();
                
            }
            ImGui::Text("");
            if (ImGui::TreeNode("Ext. Audio Input") )
            {
                ImGui::Text("");
                if (ofxImGui::AddRadio(audioExtInputDevice, audioInputDeviceList) ) audioExtInputDeviceName = audioInputDeviceList.at(audioExtInputDevice.get() );
                if (ofxImGui::AddParameter(extInputVol) ){
                    inputMixer.setInputVolume( extInputVol, 1);
                }
                
                ImGui::TreePop();
                
            }
            ImGui::Text("");
            if (ImGui::TreeNode("Audio Output") )
            {
                ImGui::Text("");
                if (ofxImGui::AddRadio(audioOutputDevice, audioOutputDeviceList) ) audioOutputDeviceName = audioOutputDeviceList.at(audioOutputDevice.get() );
                ImGui::TreePop();
            }
            //                if (ImGui::TreeNode("Samplerate") ) {
            //                    ofxImGui::AddRadio(sampleRate, sampleRateOptions, 2);
            //                    ImGui::TreePop();
            //                }
            ImGui::Unindent();
            
            ImGui::Text("");
            
            //            if (ImGui::Button("Apply") ) {
            //                stopAudio();
            //                startAudio();
            //            };
        }
        
        //SOUND
        /*
         if (ImGui::CollapsingHeader("Sound Settings")){
         ImGui::Indent();
         ImGui::Text("");
         ofxImGui::AddRadio(audioProcessingOn, onOffOptions, 2);
         
         ImGui::Text("");
         ofxImGui::AddParameter(lowcut_0_1);
         ofxImGui::AddParameter(highcut_0_2);
         ofxImGui::AddParameter(lowgain_0_3);
         ofxImGui::AddParameter(midgain_0_4);
         ofxImGui::AddParameter(highgain_0_5);
         
         ImGui::Text("");
         ofxImGui::AddParameter(preAmp);
         ofxImGui::AddParameter(threshold);
         ofxImGui::AddParameter(ratio);
         ofxImGui::AddParameter(makeUp);
         
         ImGui::Text("");
         
         ImGui::Unindent();
         
         }
         */
        //DATAS
        if (ImGui::CollapsingHeader("Analysis") )
        {
            //                ofxImGui::AddParameter(this->background, false);
            //                ofxImGui::AddParameter(this->foreground);
            
            //ofxImGui::AddParameter(this->powerSmooth);
            ImGui::Text("");
            ofxImGui::AddRadio(audioAnalyseOn, onOffOptions, 2);
            ImGui::Text("");
            ImGui::Text("%.1f FPS (%.3f ms/frame)", ofGetFrameRate(), 1000.0f / ImGui::GetIO().Framerate);
            ImGui::Text("");
            ImGui::Indent();
            
            if (ImGui::TreeNode("Unary Datas") ) {
                
                ImGui::Text(" ");
                ImGui::Text("POWER : Instant power of the actual soundbuffer");
                ImGui::ProgressBar(power, ImVec2(0.0f,0.0f));
                
                ImGui::Text(" ");
                ImGui::Text("PITCH_FREQ : Fundamental frequency");
                char buf[16];
                sprintf(buf, "%d Hz", (int)( pitchFreq ) );
                ImGui::ProgressBar(pitchFreqNorm, ImVec2(0.0f,0.0f), buf );
                
                ImGui::Text(" ");
                ImGui::Text("PITCH_SALIENCE : How much the PITCH_FREQ is noticeable");
                ImGui::ProgressBar(pitchSalience, ImVec2(0.0f,0.0f));
                
                ImGui::Text(" ");
                ImGui::Text("HFC : High Frequency Coef");
                //sprintf(buf, "%d _", (int)( hfc ) );
                ImGui::ProgressBar(hfcNorm, ImVec2(0.0f,0.0f));
                
                ImGui::Text(" ");
                ImGui::Text("SPECTRAL_COMPLEXITY");
                ImGui::ProgressBar(specCompNorm, ImVec2(0.0f,0.0f));
                
                ImGui::Text(" ");
                ImGui::Text("CENTROID : Spectrum center (perceived as brightness)");
                sprintf(buf, "%d Hz", (int)( centroid ) );
                ImGui::ProgressBar(centroidNorm, ImVec2(0.0f,0.0f), buf);
                
                //                ImGui::Text(" ");
                //                ImGui::Text("INHARMONICITY");
                //                ImGui::ProgressBar(inharmonicity, ImVec2(0.0f,0.0f));
                
                ImGui::Text(" ");
                ImGui::Text("DISSONANCE");
                ImGui::ProgressBar(dissonance, ImVec2(0.0f,0.0f));
                
                ImGui::Text(" ");
                ImGui::Text("ROLL_OFF : roll off frequency of the spectrum");
                sprintf(buf, "%d Hz", (int)( rollOff ) );
                ImGui::ProgressBar(rollOffNorm, ImVec2(0.0f,0.0f), buf);
                
                ImGui::Text(" ");
                ImGui::Text("ODD_TO_EVEN : odd and even harmonic energy ratio");
                ImGui::ProgressBar(oddToEvenNorm, ImVec2(0.0f,0.0f));
                
                ImGui::Text(" ");
                ImGui::Text("STRONG_PEAK");
                sprintf(buf, "%d dB", (int)( strongPeak ) );
                ImGui::ProgressBar(strongPeakNorm, ImVec2(0.0f,0.0f), buf);
                
                ImGui::Text(" ");
                ImGui::Text("STRONG_DECAY");
                sprintf(buf, "%d dB", (int)( strongDecay ) );
                ImGui::ProgressBar(strongDecayNorm, ImVec2(0.0f,0.0f), buf);
                ImGui::Text("");
                ImGui::TreePop();
            }
            
            ImGui::Text("");
            if (ImGui::TreeNode("Array Datas")){
                
                
                ImGui::Text(" ");
                ImGui::Text("SPECTRUM : 1024 bands fft");
                //                float* arr = spectrum.data();
                //                ImGui::PlotHistogram(" ", arr, spectrum.size(), 0, NULL, DB_MIN, DB_MAX, ImVec2(0,80) );
                float* arr = spectrumNorm.data();
                ImGui::PlotHistogram(" ", arr, spectrumNorm.size(), 0, NULL, 0, 1, ImVec2(0,80) );
                
                ImGui::Text(" ");
                ImGui::Text("MEL_BANDS : 24 bands bargraph");
                arr = melBands.data();
                ImGui::PlotHistogram(" ", arr, melBands.size(), 0, NULL, DB_MIN, DB_MAX, ImVec2(0,80) );
                
                ImGui::Text(" ");
                ImGui::Text("MFCC : 12 bands cepstrum analysis");
                arr = mfcc.data();
                ImGui::PlotHistogram(" ", arr, mfcc.size(), 0, NULL,  0, MFCC_MAX_ESTIMATED_VALUE, ImVec2(0,80) );
                
                ImGui::Text(" ");
                ImGui::Text("HPCP : Harmonic Pitch Class Profile from A to G#");
                arr = hpcp.data();
                ImGui::PlotHistogram(" ", arr, hpcp.size(), 0, NULL,  0, 1, ImVec2(0,80) );
                
                ImGui::Text(" ");
                ImGui::Text("TRISTIMULUS : 1st Harmo / 2nd+3rd+4th Harmo / >4th Harmo");
                arr = tristimulus.data();
                ImGui::PlotHistogram(" ", arr, tristimulus.size(), 0, NULL,  0, 1, ImVec2(0,80) );
                
                
                //                    ImGui::Text("MULTIPITCHES");
                //                    arr = multiPitchesSaliences.data();
                //                    ImGui::PlotHistogram(" ", arr, multiPitchesSaliences.size(), 0, NULL,  0, 1, ImVec2(0,80) );
                ImGui::Text("");
                ImGui::TreePop();
            }
            
            ImGui::Unindent();
            ImGui::Text("");
        }
        
        
        //MAPPING
        if (ImGui::CollapsingHeader("Mapping") )
        {
            ImGui::Indent();
            
            
            ofxImGui::AddRange("SoundFreq. Range", aStartFreq, aEndFreq);
            ofxImGui::AddRange("ColorFreq. Range", cStartFreq, cEndFreq);
            
            
            if (ImGui::TreeNode("Presences") )
            {ImGui::Indent();
                
                //ofxImGui::AddRadio( perBandPresence, perBandOptions, 3);
                imGuiAudioArraySelector(perBandPresence.get() );
                
                //ofxImGui::AddRadio( perBandPresence2, perBandOptions, 3);
                imGuiAudioArraySelector(perBandPresence2.get() );
                
                ImGui::Text("");
                ofxImGui::AddParameter(perBandPresenceAdjust);
                
                ImGui::Text("");
                ofxImGui::AddParameter(compensation);
                
                ImGui::Unindent();
                ImGui::TreePop();
            }
            
            
            if (ImGui::TreeNode("Brightnesses") )
            {ImGui::Indent();
                
                ImGui::Text("   ");
                ofxImGui::AddRadio( perBandBrightness, perBandOptions, 3);
                imGuiAudioArraySelector(perBandBrightness.get() );
                ImGui::Text("   ");
                ofxImGui::AddParameter(perBandBrightnessAdjust);
                ImGui::Text("   ");
                
                ImGui::Unindent();
                ImGui::TreePop();
            }
            
            if (ImGui::TreeNode("Saturations") )
            {ImGui::Indent();
                
                ImGui::Text("   ");
                ofxImGui::AddRadio( perBandSaturation, perBandOptions, 3);
                imGuiAudioArraySelector(perBandSaturation.get() );
                ImGui::Text("   ");
                ofxImGui::AddParameter(perBandSaturationAdjust);
                ImGui::Text("   ");
                
                ImGui::Unindent();
                ImGui::TreePop();
            }
            
            if (ImGui::TreeNode("Alphas") )
            {ImGui::Indent();
                
                ImGui::Text("   ");
                ofxImGui::AddRadio( perBandAlpha, perBandOptions, 3);
                imGuiAudioArraySelector(perBandAlpha.get() );
                ImGui::Text("   ");
                ofxImGui::AddParameter(perBandAlphaAdjust);
                ImGui::Text("   ");
                
                ImGui::Unindent();
                ImGui::TreePop();
            }
            
            
            
            ImGui::Unindent();
        }
        
        //GRADIENT
        if (ImGui::CollapsingHeader("Gradient") )
        {
            
            ImGui::Indent();
            ImGui::Text(" ");
            ofxImGui::AddRadio(stereoGradient, stereoOptions, 2);
            ImGui::Text(" ");
            if (ImGui::TreeNode("Brightness") ){
                
                ImGui::Text(" ");
                ofxImGui::AddRadio( globalBrightness, globalOptions, 3);
                imGuiAudioUnarySelector(globalBrightness);
                ofxImGui::AddParameter(globalBrightnessAdjust);
                ImGui::Text("");
                ofxImGui::AddParameter(refBrightness);
                ImGui::Text("");
                ImGui::TreePop();
                
            }
            
            if (ImGui::TreeNode("Saturation") ){
                ImGui::Text(" ");
                ofxImGui::AddRadio( globalSaturation, globalOptions, 3);
                imGuiAudioUnarySelector(globalSaturation);
                ofxImGui::AddParameter(globalSaturationAdjust);
                ImGui::Text("");
                ofxImGui::AddParameter(refSaturation);
                ImGui::Text("");
                ImGui::TreePop();
                
            }
            
            if (ImGui::TreeNode("Alpha") ){
                ImGui::Text(" ");
                ofxImGui::AddRadio( globalAlpha, globalOptions, 3);
                imGuiAudioUnarySelector(globalAlpha);
                ofxImGui::AddParameter(globalAlphaAdjust);
                ImGui::Text("");
                ofxImGui::AddParameter(refAlpha);
                ImGui::Text("");
                ImGui::TreePop();
                
            }
            
            
            if (ImGui::TreeNode("Contrast") ){
                
                ImGui::Text(" ");
                ofxImGui::AddRadio( globalContrast, globalOptions, 3);
                imGuiAudioUnarySelector(globalContrast);
                ofxImGui::AddParameter(globalContrastAdjust);
                ImGui::Text("");
                ofxImGui::AddParameter(refContrast);
                ImGui::Text("");
                ImGui::TreePop();
                
            }
            
            ImGui::Text("");
            ofxImGui::AddRadio(invertGradient, onOffOptions, 2 );
            ImGui::Text("");
            
            ImGui::Unindent();
            
        }
        
        //VIDEO
        if (ImGui::CollapsingHeader("Video") )
        {
            
            ImGui::Indent();
            
            ImGui::Text("");
            if (ImGui::TreeNode("Video Input") ){
                ImGui::Text("");
                if (ofxImGui::AddRadio(audioVideoInputOption, audioVideoInputOptions, 3) ) {
                    
                    audioVideoInputName = audioVideoInputOptions.at(audioVideoInputOption.get() );
                    
                    if (lastAsource == 0 && videoInputOption.get() !=0) stop();

                    if (lastAsource == 1 ){
                        
                        if (videoInputOption == 1 && audioVideoUsbInputOption.get() == videoUsbInputOption.get() );//donothing
                        
                        else audioVideoCamera.close();
                    }
                    
                    
                    lastAsource = audioVideoInputOption;
                }
                ImGui::Text("");
                audioVideoInputOptionSelector(audioVideoInputOption);
                ImGui::TreePop();
            }
            
            ImGui::Text("");
            
            
            
            
            if (ImGui::TreeNode("Zoom") ){
                ImGui::Text("");
                ofxImGui::AddParameter(audioVideoZoom);
                ofxImGui::AddParameter(audioVideoPositionX);
                ofxImGui::AddParameter(audioVideoPositionY);
                ImGui::Text("");
                ImGui::TreePop();
            }
            
            if (ImGui::TreeNode("Video Brightness") ){
                ImGui::Text("");
                ofxImGui::AddRadio(avBModOption, globalOptions, 3);
                ImGui::Text("");
                imGuiAudioUnarySelector(avBModOption);
                ofxImGui::AddParameter(avBModIntensity);
                ImGui::Text("");
                ofxImGui::AddParameter(audioVideoBrightness);
                ImGui::Text("");
                ImGui::TreePop();
            }
            
            if (ImGui::TreeNode("Video Contrast") ){
                ImGui::Text("");
                ofxImGui::AddRadio(avCModOption, globalOptions, 3);
                ImGui::Text("");
                imGuiAudioUnarySelector(avCModOption);
                ofxImGui::AddParameter(avCModIntensity);
                ImGui::Text("");
                ofxImGui::AddParameter(audioVideoContrast);
                ImGui::Text("");
                ImGui::TreePop();
            }
            
            if (ImGui::TreeNode("Video Saturation") ){
                ImGui::Text("");
                ofxImGui::AddRadio(avSModOption, globalOptions, 3);
                ImGui::Text("");
                imGuiAudioUnarySelector(avSModOption);
                ofxImGui::AddParameter(avSModIntensity);
                ImGui::Text("");
                ofxImGui::AddParameter(audioVideoSaturation);
                ImGui::Text("");
                
                ImGui::TreePop();
            }
            
            if (ImGui::TreeNode("Video Alpha") ){
                ImGui::Text("");
                ofxImGui::AddRadio(avAModOption, globalOptions, 3);
                ImGui::Text("");
                imGuiAudioUnarySelector(avAModOption);
                ofxImGui::AddParameter(avAModIntensity);
                ImGui::Text("");
                ofxImGui::AddParameter(audioVideoAlpha);
                ImGui::Text("");
                ImGui::TreePop();
            }
            
            ImGui::Text("");
            
            if (audioVideoInputOption == 0 || audioVideoInputOption == 2) {
                if (ImGui::TreeNode("Varispeed") ){
                    ImGui::Text("");
                    if (ofxImGui::AddRadio(varispeedOn, onOffOptions, 2) ){
                        if (varispeedOn == 0 && hapStarted) hap.setSpeed(1.0f);
                    }
                    ImGui::Text("");
                    if (varispeedOn == 1) {
                        ofxImGui::AddRadio(varispeedOption, globalOptions, 3);
                        ImGui::Text("");
                        imGuiAudioUnarySelector(varispeedOption);
                        ofxImGui::AddParameter(varispeedIntensity);
                        ofxImGui::AddParameter(varispeedRef);
                        ImGui::ProgressBar(movieSpeed);
                        ImGui::Text("");
                        ofxImGui::AddParameter(pitchCompensation);
                        //ofxImGui::AddParameter(pitchshift_2_0);
                        ImGui::Text("");
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::Text("");
            ImGui::Unindent();
            
        }
        //BLENDING
        if (ImGui::CollapsingHeader("Blending") )
        {
            
            //ImGui::Indent();
            
            //if (ImGui::TreeNode("Blending Mode") ){
            ImGui::Text("");
            ofxImGui::AddRadio(blendMode, blendModeOptions, 3);
            ImGui::Text("");
            if(ImGui::Button("Swap" ) ) {
                swap = !swap;
            }
            ImGui::SameLine();
            if (!swap) {ImGui::Text("Gradient / Video");}
            else {ImGui::Text("Video / Gradient");}
            
            ImGui::Text("");
            ofxImGui::AddParameter(audioVideoMix);
            ofxImGui::AddParameter(audioVideoCrop);
            ImGui::Text("");
            
            //    ImGui::TreePop();
            //}
            
            //ImGui::Unindent();
            
        }
        
        //LOADING & SAVING PRESETS
        
        if (ImGui::CollapsingHeader("Presets") )
        {
            ImGui::Text("");
            
            if (ImGui::Button("  Load  ") ) {
                loadAudioPreset(audioPresetsNames.at(audioPresetNumber) );
                loadAudioUnitsPresets(audioPresetsNames.at(audioPresetNumber));
            }
            
            ImGui::SameLine();
            if (ImGui::Button("  Save  ") ) {
                saveAudioUnitsPresets(audioPresetName.get() );
                saveAudioPreset(audioPresetName.get() );
                
            }
            ImGui::Text("");
            static char abuf[16] = "";
            if (ImGui::InputText("", abuf, 16) ){
            }
            ImGui::SameLine();
            if(ImGui::Button("< Save as...") ) {
                saveAudioPreset(ofToString(abuf));
                audioPresetName = ofToString(abuf);
                saveAudioUnitsPresets(audioPresetName.get() );
                refreshAudioPresets();
            }
            ImGui::Text("");
            ofxImGui::AddRadio(audioPresetNumber, audioPresetsNames, 3);
            ImGui::Text("");
            
        }
        
        
        
    }
    
    ofxImGui::EndWindow(guiSettings);
    
    
}



void ofApp::imGuiAudioArraySelector(int option) {
    
    float* arr;
    
    switch(option ) { //"SPECTRUM", "MEL_BANDS", "MFCC", "HPCP", "TRISTIMLUS", "MULTI_PITCHES", "NONE"
            
        case 0 :
            arr = spectrum.data();
            ImGui::PlotHistogram("SPECTRUM", arr, spectrum.size(), 0, NULL, DB_MIN, DB_MAX, ImVec2(0,80) );
            ofxImGui::AddParameter(spectrumSmooth);
            
            
            break;
            
        case 1 :
            arr = melBands.data();
            ImGui::PlotHistogram("MEL_BANDS", arr, melBands.size(), 0, NULL, DB_MIN, DB_MAX, ImVec2(0,80) );
            ofxImGui::AddRange("MEL Bands Range", mStartBand, mEndBand);
            ofxImGui::AddParameter(melBandsSmooth);
            
            break;
            
        case 2 :
            arr = mfcc.data();
            ImGui::PlotHistogram("MFCC", arr, mfcc.size(), 0, NULL, 0, MFCC_MAX_ESTIMATED_VALUE, ImVec2(0,80) );
            ofxImGui::AddParameter(mfccSmooth);
            
            break;
            
        case 3 :
            arr = hpcp.data();
            ImGui::PlotHistogram("HPCP", arr, hpcp.size(), 0, NULL, 0, 1, ImVec2(0,80) );
            ofxImGui::AddParameter(hpcpSmooth);
            
            break;
            
        case 4 :
            arr = tristimulus.data();
            ImGui::PlotHistogram("TRISTIMLUS", arr, tristimulus.size(), 0, NULL, 0, 1, ImVec2(0,80) );
            ofxImGui::AddParameter(tristimulusSmooth);
            
            break;
            
        case 5 :
            //            arr = multiPitchesSaliences.data();
            //            ImGui::PlotHistogram("MULTI_PITCHES", arr, multiPitchesSaliences.size(), 0, NULL, 0, 1, ImVec2(0,80) );
            //            ofxImGui::AddParameter(multiPitchesSmooth);
            
            break;
            
    }
    
    
    
}


void ofApp::imGuiAudioUnarySelector(int option) {
    
    //"POWER", "PITCH_FREQ", "PITCH_SALIENCE", "HFC", "SPECTRAL_COMPLEXITY", "CENTROID", "DISSONANCE", "ROLL_OFF", "ODD_TO_EVEN", "STRONG_PEAKS", "STRONG_DECAY", "NONE"
    char buf[16];
    
    switch (option) {
        case 0 :
            ImGui::ProgressBar(power, ImVec2(0.0f,0.0f));
            ofxImGui::AddParameter(powerSmooth);
            break;
            
        case 1 :
            
            sprintf(buf, "%d Hz", (int)( pitchFreq ) );
            ImGui::ProgressBar(pitchFreqNorm, ImVec2(0.0f,0.0f), buf );
            ofxImGui::AddParameter(pitchFreqSmooth);
            break;
            
        case 2 :
            ImGui::ProgressBar(pitchSalience, ImVec2(0.0f,0.0f));
            ofxImGui::AddParameter(pitchSalienceSmooth);
            break;
            
        case 3 :
            ImGui::ProgressBar(hfcNorm, ImVec2(0.0f,0.0f));
            ofxImGui::AddParameter(hfcSmooth);
            break;
            
        case 4 :
            ImGui::ProgressBar(specCompNorm, ImVec2(0.0f,0.0f));
            ofxImGui::AddParameter(specCompSmooth);
            break;
            
        case 5 :
            sprintf(buf, "%d Hz", (int)( centroid ) );
            ImGui::ProgressBar(centroidNorm, ImVec2(0.0f,0.0f), buf);
            ofxImGui::AddParameter(centroidSmooth);
            break;
            
            
        case 6 :
            ImGui::ProgressBar(dissonance, ImVec2(0.0f,0.0f));
            ofxImGui::AddParameter(dissonanceSmooth);
            break;
            
        case 7 :
            sprintf(buf, "%d Hz", (int)( rollOff ) );
            ImGui::ProgressBar(rollOffNorm, ImVec2(0.0f,0.0f), buf);
            ofxImGui::AddParameter(rollOffSmooth);
            break;
            
        case 8 :
            ImGui::ProgressBar(oddToEvenNorm, ImVec2(0.0f,0.0f));
            ofxImGui::AddParameter(oddToEvenSmooth);
            break;
            
        case 9 :
            sprintf(buf, "%d dB", (int)( strongPeak ) );
            ImGui::ProgressBar(strongPeakNorm, ImVec2(0.0f,0.0f), buf);
            ofxImGui::AddParameter(strongPeakSmooth);
            ofxImGui::AddParameter(strongPeakAmp);
            break;
            
        case 10 :
            sprintf(buf, "%d dB", (int)( strongDecay ) );
            ImGui::ProgressBar(strongDecayNorm, ImVec2(0.0f,0.0f), buf);
            ofxImGui::AddParameter(strongDecaySmooth);
            break;
            
        case 11 :
            break;
            
            
    }
    
    
    
    
}

void ofApp::audioVideoInputOptionSelector(int option){
    
    switch(audioVideoInputOption) {
            
        case 0:
            break;
            
        case 1:
            
            if(ofxImGui::AddRadio(audioVideoUsbInputOption, camNames) ){
                changeAudioVideoCam(audioVideoUsbInputOption);
            }
            break;
        case 2:
            
            break;
            
            
            
    }
    
}

//-----------//-----------//-----------//-----------//-----------//-----------VIDEO_GUI

void ofApp::drawImGuiVideo() {
    
    if (ofxImGui::BeginWindow(videoPresetName, guiSettings, window_flags)){
        
        if (ImGui::Button("Save") ) {
            saveVideoPreset(videoPresetName);
        }
        ImGui::SameLine();
        ImGui::Text("::::");
        ImGui::SameLine();
        if (ofxImGui::AddParameter(volume) ){
            compressor.setParameter( 6, kAudioUnitScope_Global, volume);
            //AudioUnitSetParameter(compressor, 6, kAudioUnitScope_Global, volume, 0);
        }
        
        
        //////
        if (ImGui::CollapsingHeader("Video Input") ) {
            ImGui::Text("");
            //ImGui::Indent();
            //if (ImGui::TreeNode("Video Input") )
            //{
            if (ofxImGui::AddRadio(videoInputOption, videoInputOptions, 3) ){
                
                videoInputName = videoInputOptions.at(videoInputOption.get() );
                
                if (lastVsource == 0 && audioVideoInputOption.get() !=0) stop();
                
                if (lastVsource == 1 ){
                    
                    if (audioVideoInputOption == 1 && audioVideoUsbInputOption == videoUsbInputOption );//donothing
                    
                    //else videoCamera.close();
                }
                
                if(videoInputOption == 1) {}
                
                lastVsource = videoInputOption;
                
            }
            videoInputOptionSelector(videoInputOption.get() );
            
            ImGui::Text("");
            
        }
        //////
        if (ImGui::CollapsingHeader("Video Settings") ) {
            ImGui::Text("Brightness");
            ofxImGui::AddParameter(brightness);
            ImGui::Text("Contrast");
            ofxImGui::AddParameter(contrast);
            ImGui::Text("Saturation");
            ofxImGui::AddParameter(saturation);
            ImGui::Text("Zoom");
            ofxImGui::AddParameter(zoomFactor);
            ImGui::Text("Horizontal Position");
            ofxImGui::AddParameter(zoomPosX);
            ImGui::Text("Vertical Position");
            ofxImGui::AddParameter(zoomPosY);
            ImGui::Text("");
            //ofxImGui::AddParameter(videoCrop);
            //ImGui::Text("");
            
        }
        
        
//        if (ImGui::CollapsingHeader("Analysis") ) {
//            ImGui::Text("");
//            ofxImGui::AddRadio(videoAnalyseOn, onOffOptions, 2);
//            ImGui::Text("");
//            ImGui::Text("%.1f FPS (%.3f ms/frame)", ofGetFrameRate(), 1000.0f / ImGui::GetIO().Framerate);
//            ImGui::Text("");
//            ImGui::Indent();
//            if (ImGui::TreeNode("Unary Datas") )
//            {
//                ImGui::Text("Average Hue");
//                ImGui::ProgressBar(averageHue );
//                ImGui::Text("Average Brightness");
//                ImGui::ProgressBar(averageBrightness );
//                ImGui::Text("Average Saturation");
//                ImGui::ProgressBar(averageSaturation );
//                ImGui::Text("White Presence");
//                ImGui::ProgressBar(white );
//                ImGui::Text("Neutral Presence");
//                ImGui::ProgressBar(neutral );
//                ImGui::Text("Black Presence");
//                ImGui::ProgressBar(black );
//                
//                ImGui::TreePop();
//                
//            }
//            ImGui::Text("");
//            if (ImGui::TreeNode("Array Datas") )
//            {
//                ImGui::Text(" ");
//                ImGui::Text("Hue Presences");
//                float* arr = presences.data();
//                ImGui::PlotHistogram(" ", arr, presences.size(), 0, NULL, 0.0f , 1.0f, ImVec2(0,80) );
//                
//                ImGui::Text(" ");
//                ImGui::Text("Brightnesses");
//                float* arr1 = brightnesses.data();
//                ImGui::PlotHistogram(" ", arr1, brightnesses.size(), 0, NULL, 0.0f , 1.0f, ImVec2(0,80) );
//                
//                ImGui::Text(" ");
//                ImGui::Text("Saturations");
//                float* arr2 = saturations.data();
//                ImGui::PlotHistogram(" ", arr2, saturations.size(), 0, NULL, 0.0f , 1.0f, ImVec2(0,80) );
//                
//                ImGui::Text(" ");
//                ImGui::Text("Horizontal Positions");
//                float* arr3 = posXs.data();
//                ImGui::PlotHistogram(" ", arr3, posXs.size(), 0, NULL, 0.0f , 1.0f, ImVec2(0,80) );
//                
//                ImGui::Text(" ");
//                ImGui::Text("Vertical Positions");
//                float* arr4 = posYs.data();
//                ImGui::PlotHistogram(" ", arr4, posYs.size(), 0, NULL, 0.0f , 1.0f, ImVec2(0,80) );
//                
//                ImGui::TreePop();
//            }
//            
//            ImGui::Unindent();
//            ImGui::Text("");
//        }
        
        //////MIDI
//        if (ImGui::CollapsingHeader("Midi") ) {
//            ImGui::Indent();
//            ImGui::Text("");
//            ofxImGui::AddRadio(midiOutOn, onOffOptions, 2   );
//            ImGui::Text("");
//            if (ImGui::TreeNode("Midi Output") )
//            {
//                ImGui::Text("");
//                ofxImGui::AddRadio(midiOutputOption, midiOuputNames, 3 );
//                ImGui::Text("");
//                ofxImGui::AddStepper(midiChannel);
//                ImGui::Text("");
//                ImGui::TreePop();
//            }
//            ImGui::Text("");
//            
//            if (ImGui::TreeNode("Scale") )
//            {
//                
//                ImGui::Text("");
//                ofxImGui::AddStepper(midiScaleSize);
//                ImGui::Text("");
//                ImGui::Text("Midi Scale");
//                //int h;
//                ImGui::PushItemWidth(150);
//                for (int i = 0; i < midiScaleSize; i++){
//                    
//                    ofxImGui::AddStepper(midiScale.at(i));ImGui::SameLine();ImGui::Text(notesNames[midiScale.at(i)].c_str());
//                    
//                }
//                ImGui::PopItemWidth();
//                ImGui::Text("");
//                ImGui::TreePop();
//            }
//            ImGui::Text("");
//            
//            if (ImGui::TreeNode("Gates") )
//            {
//                ofxImGui::AddRadio(midiTriggerOption, videoArrayOptions, 3);
//                videoArrayMidiOptionSelector(midiTriggerOption);
//                ImGui::Text("");
//                for (int i = 0; i < midiScaleSize; i++){
//                    //h = i%4;
//                    //if (h!=0) ImGui::SameLine();
//                    ofxImGui::AddParameter(midiThresholds.at(i));
//                }
//                ImGui::Text("");
//                ImGui::TreePop();
//            }
//            
//            ImGui::Text("");
//            
//            if (ImGui::TreeNode("Note On") )
//            {
//                ImGui::Text("");
//                ofxImGui::AddRadio(midiOctaveOption, videoArrayOptions, 3);
//                videoArrayMidiOptionSelector(midiOctaveOption);
//                ofxImGui::AddParameter(midiOctaveIntensity);
//                ofxImGui::AddParameter(midiOctaveRef);
//                ImGui::Text("");
//                ofxImGui::AddRadio(midiVelocityOption, videoArrayOptions, 3);
//                videoArrayMidiOptionSelector(midiVelocityOption);
//                ofxImGui::AddParameter(midiVelocitiesIntensity);
//                ofxImGui::AddParameter(midiVelocitiesRef);
//                ImGui::Text("");
//                ofxImGui::AddRadio(midiAfterTouchOption, videoArrayOptions, 3);
//                videoArrayMidiOptionSelector(midiAfterTouchOption);
//                ofxImGui::AddParameter(midiAfterTouchesIntensity);
//                ofxImGui::AddParameter(midiAfterTouchesRef);
//                ImGui::Unindent();
//                ImGui::Text("");
//                ImGui::TreePop();
//            }
//            ImGui::Text("");
//            if (ImGui::TreeNode("Control Change") )
//            {
//                ImGui::Text("");
//                ofxImGui::AddParameter(midiCcNum1);
//                ofxImGui::AddRadio(midiCcOption1, videoUnaryOptions, 3);
//                videoUnaryOptionSelector(midiCcOption1);
//                ofxImGui::AddParameter(midiCcIntensity1);
//                ofxImGui::AddParameter(midiCcRef1);
//                ImGui::Text("");
//                ofxImGui::AddParameter(midiCcNum2);
//                ofxImGui::AddRadio(midiCcOption2, videoUnaryOptions, 3);
//                videoUnaryOptionSelector(midiCcOption2);
//                ofxImGui::AddParameter(midiCcIntensity2);
//                ofxImGui::AddParameter(midiCcRef2);
//                ImGui::Text("");
//                ImGui::TreePop();
//            }
//            ImGui::Text("");
//            
//        }
        
        /////iFFT
        if (ImGui::CollapsingHeader("Inverse FFT") ) {
            ImGui::Text("");
            ofxImGui::AddRadio(videoAnalyseOn, onOffOptions, 2);
            ImGui::Text("");
            ImGui::Text("%.1f FPS (%.3f ms/frame)", ofGetFrameRate(), 1000.0f / ImGui::GetIO().Framerate);
            ImGui::Text("");

//            ofxImGui::AddRange("Color Frequencies Range", videoStartC, videoEndC);
            ofxImGui::AddRange("Sound Frequency Range", videoStartF, videoEndF);
            ImGui::Text("");
            ofxImGui::AddRadio(iFFTon, onOffOptions, 2);
            ImGui::Text("");

            ofxImGui::AddRadio(iFFTmagOption, videoArrayOptions, 3);
            videoArrayOptionSelector( iFFTmagOption );
            ImGui::Text("");
            
            ofxImGui::AddRadio(iFFTphaseOption, videoArrayOptions, 3);
            videoArrayOptionSelector( iFFTphaseOption );
            ImGui::Text("");
            ImGui::Text("");
            videoArrayOptionSelector( 3 );
            ImGui::Text("");
            ImGui::Text("");
            videoArrayOptionSelector( 4 );
            ImGui::Text("");
            ImGui::Text("");
            
            ofxImGui::AddRadio(iFFTloHiModOption, videoUnaryOptions, 3);
            videoUnaryOptionSelector(iFFTloHiModOption);
            ofxImGui::AddParameter(iFFTloHiModIntensity);
            ofxImGui::AddParameter(iFFTloHiRef);
            ImGui::ProgressBar(iFFTloHi);
            ImGui::Text("");
            ofxImGui::AddRadio(iFFThiShiftModOption, videoUnaryOptions, 3);
            videoUnaryOptionSelector(iFFThiShiftModOption);
            ofxImGui::AddParameter(iFFThiShiftModIntensity);
            ofxImGui::AddParameter(iFFThiShiftRef);
            ImGui::ProgressBar(iFFThiShift);
//            ImGui::Text("");
//            ofxImGui::AddRadio(iFFTwhiteMorphModOption, videoUnaryOptions, 3);
//            videoUnaryOptionSelector(iFFTwhiteMorphModOption);
//            ofxImGui::AddParameter(iFFTwhiteMorphModIntensity);
//            ofxImGui::AddParameter(iFFTwhiteMorphRef);
//            ImGui::ProgressBar(iFFTwhiteMorph);
            ImGui::Text("");
            ImGui::Text("");
            
            ofxImGui::AddRadio(iFFTconvoMixModOption, videoUnaryOptions, 3);
            videoUnaryOptionSelector(iFFTconvoMixModOption);
            ofxImGui::AddParameter(iFFTconvoMixModIntensity);
            ofxImGui::AddParameter(iFFTconvoMixRef);
            ImGui::ProgressBar(iFFTconvoMix);
            ImGui::Text("");
            ImGui::Text("");
            
            ofxImGui::AddRadio(iFFTampModOption, videoUnaryOptions, 3);
            videoUnaryOptionSelector(iFFTampModOption);
            ofxImGui::AddParameter(iFFTampModIntensity);
            ofxImGui::AddParameter(iFFTampRef);
            ImGui::ProgressBar(iFFTamp);
            ImGui::Text("");
            
            ImGui::Indent();
        }
        
        
        
        /////SC_SYNTH
        if (ImGui::CollapsingHeader("Piano Synth") ){
            ImGui::Text("");
            ofxImGui::AddRadio(scSynthOn, onOffOptions, 2);
            
            ImGui::Text("");
            if (ImGui::TreeNode("Frequencies") )
            {
                ImGui::Text("");
                for (int i = 0; i < 16; i+=2){
                    ofxImGui::AddRange("", scFreqs.at(i), scFreqs.at(i+1) );
                }
                ImGui::Text("");
                ImGui::TreePop();
            }
            
            ImGui::Text("");
            if (ImGui::TreeNode("Gates") )
            {
                ImGui::Text("");
                ofxImGui::AddRadio(scTriggerOption, videoArrayOptions, 3);
                ImGui::Text("");
                videoArrayScOptionSelector(scTriggerOption);
                ImGui::Text("");
                for (int i = 0; i < 16; i++){
                    //h = i%4;
                    //if (h!=0) ImGui::SameLine();
                    ofxImGui::AddParameter(scThresholds.at(i));
                }
                ImGui::Text("");
                
                ImGui::Text("");
                ImGui::TreePop();
            }
            
            ImGui::Text("");
            if (ImGui::TreeNode("Velocities") )
            {
                ImGui::Text("");
                ofxImGui::AddRadio(scVelocitiesOption, videoArrayOptions, 3);
                ImGui::Text("");
                videoArrayScOptionSelector(scVelocitiesOption);
                ofxImGui::AddParameter(scVelocitiesIntensities);
                ofxImGui::AddParameter(scVelocitiesRefs);
                ImGui::Text("");
                ImGui::TreePop();
            }
            
            ImGui::Text("");
            if (ImGui::TreeNode("Harmonic 1 Mods") )
            {
                ImGui::Text("");
                ofxImGui::AddParameter(hNumber1);
                ofxImGui::AddParameter(hPhase1);
                ImGui::Text("");
                ofxImGui::AddRadio(scH1Option, videoArrayOptions, 3);
                ImGui::Text("");
                videoArrayScOptionSelector(scH1Option);
                ofxImGui::AddParameter(scH1Intensities);
                ofxImGui::AddParameter(scH1Refs);
                ImGui::Text("");
                ImGui::TreePop();
            }
            
            ImGui::Text("");
            if (ImGui::TreeNode("Harmonic 2 Mods") )
            {
                ImGui::Text("");
                ofxImGui::AddParameter(hNumber2);
                ofxImGui::AddParameter(hPhase2);
                ImGui::Text("");
                ofxImGui::AddRadio(scH2Option, videoArrayOptions, 3);
                ImGui::Text("");
                videoArrayScOptionSelector(scH2Option);
                ofxImGui::AddParameter(scH2Intensities);
                ofxImGui::AddParameter(scH2Refs);
                ImGui::Text("");
                ImGui::TreePop();
            }
            
//            ImGui::Text("");
//            if (ImGui::TreeNode("LFO Mods") )
//            {
//                ImGui::Text("");
//                ofxImGui::AddRadio(lfoOption, lfoOptions, 3);
//                ImGui::Text("");
//                ofxImGui::AddParameter(lfoRate);
//                ImGui::Text("");
//                ofxImGui::AddRadio(scLfoOption, videoArrayOptions, 3);
//                ImGui::Text("");
//                videoArrayScOptionSelector(scLfoOption);
//                ofxImGui::AddParameter(scLfoIntensities);
//                ofxImGui::AddParameter(scLfoRefs);
//                ImGui::Text("");
//                ImGui::TreePop();
//            }
            
            
            ImGui::Text("");
            if (ImGui::TreeNode("VCA") )
            {
                ImGui::Text("");
                ofxImGui::AddParameter(scAtt);
                ofxImGui::AddParameter(scDec);
                ofxImGui::AddParameter(scSus);
                ofxImGui::AddParameter(scRel);
                ImGui::Text("");
                for (int i = 0; i < 16; i++){
                    //h = i%4;
                    //if (h!=0) ImGui::SameLine();
                    ofxImGui::AddParameter(scVolumes.at(i));
                }
                ImGui::Text("");
                ofxImGui::AddRadio(scMasterVolOption, videoUnaryOptions, 3);
                ImGui::Text("");
                videoUnaryOptionSelector(scMasterVolOption);
                ofxImGui::AddParameter(scMasterVolIntensity);
                ofxImGui::AddParameter(scMasterVolRef);
                ImGui::Text("");
                
                ImGui::ProgressBar(scMasterVolume);
                ImGui::Text("");
                ImGui::TreePop();
            }
            
            ImGui::Text("");
        }
        
        
        ///PRESETS
        if (ImGui::CollapsingHeader("Presets ") )
        {
            ImGui::Text("");
            
            if (ImGui::Button("Load ") ) {
                loadVideoPreset(videoPresetsNames.at(videoPresetNumber) );
            }
            
            ImGui::SameLine();
            if (ImGui::Button("Save ") ) {
                saveVideoPreset(videoPresetName.get() );
            }
            
            static char abuf[16] = "";
            if (ImGui::InputText("", abuf, 16, ImGuiInputTextFlags_CharsNoBlank) ){
            }
            ImGui::SameLine();
            if(ImGui::Button("< Save as... ") ) {
                saveVideoPreset( ofToString(abuf));
                videoPresetName =  ofToString(abuf);
                refreshVideoPresets();
            }
            
            
            
            ofxImGui::AddRadio(videoPresetNumber, videoPresetsNames, 3);
            
            
        }
        
        
        
    }
    
    ofxImGui::EndWindow(guiSettings);
    
    
}


//-----------
void ofApp::videoInputOptionSelector(int option){
    
    switch(videoInputOption) {
            
        case 0:
            break;
            
        case 1:
            
            if(ofxImGui::AddRadio(videoUsbInputOption, camNames) ){
                changeVideoCam(videoUsbInputOption.get() );
            }
            break;
        case 2:
            
            break;
            
            
            
    }
    
}

//-----------


void ofApp::videoArrayMidiOptionSelector(int option){
    float* arr;
    switch (option) {
        case 0:
            arr = midiPresences.data();
            ImGui::PlotHistogram("Presences", arr, midiPresences.size(), 0, NULL, 0.0f , 1.0f, ImVec2(0,80) );
            ofxImGui::AddParameter(histoAmp);
            ofxImGui::AddParameter(histoSmooth);
            break;
            
        case 1:
            arr = midiBrightnesses.data();
            ImGui::PlotHistogram("Brightnesses", arr, midiBrightnesses.size(), 0, NULL, 0.0f , 1.0f, ImVec2(0,80) );
            ofxImGui::AddParameter(luAmp);
            ofxImGui::AddParameter(luSmooth);
            break;
        case 2:
            arr = midiSaturations.data();
            ImGui::PlotHistogram("Saturations", arr, midiSaturations.size(), 0, NULL, 0.0f , 1.0f, ImVec2(0,80) );
            ofxImGui::AddParameter(satAmp);
            ofxImGui::AddParameter(satSmooth);
            break;
        case 3:
            arr = midiXs.data();
            ImGui::PlotHistogram("Horizontal Positions", arr, midiXs.size(), 0, NULL, 0.0f , 1.0f, ImVec2(0,80) );
            ofxImGui::AddParameter(xyAmp);
            ofxImGui::AddParameter(xySmooth);
            break;
        case 4:
            arr = midiYs.data();
            ImGui::PlotHistogram("Presences", arr, midiYs.size(), 0, NULL, 0.0f , 1.0f, ImVec2(0,80) );
            ofxImGui::AddParameter(xyAmp);
            ofxImGui::AddParameter(xySmooth);
            break;
        case 5:
            
            break;
            
    }
}

void ofApp::videoUnaryOptionSelector(int option){
    switch (option) {
        case 0:
            ImGui::ProgressBar(averageHue);
            ofxImGui::AddParameter(averageHueSmooth);
            break;
        case 1:
            ImGui::ProgressBar(averageBrightness);
            ofxImGui::AddParameter(averageBrightnessAmp);
            ofxImGui::AddParameter(averageBrightnessSmooth);
            break;
        case 2:
            ImGui::ProgressBar(averageSaturation);
            ofxImGui::AddParameter(averageSaturationAmp);
            ofxImGui::AddParameter(averageSaturationSmooth);
            break;
        case 3:
            ImGui::ProgressBar(white);
            ofxImGui::AddParameter(whiteThresh);
            ofxImGui::AddParameter(whiteAmp);
            ofxImGui::AddParameter(whiteSmooth);
            break;
        case 4:
            ImGui::ProgressBar(neutral);
            ofxImGui::AddParameter(neutralThresh);
            ofxImGui::AddParameter(neutralAmp);
            ofxImGui::AddParameter(neutralSmooth);
            break;
        case 5:
            ImGui::ProgressBar(black);
            ofxImGui::AddParameter(blackThresh);
            ofxImGui::AddParameter(blackAmp);
            ofxImGui::AddParameter(blackSmooth);
            break;
        case 6:
            
            break;
            
        default:
            break;
    }
    
    
    
}

//-----------


void ofApp::videoArrayScOptionSelector(int option){
    float* arr;
    switch (option) {
        case 0:
            arr = scPresences.data();
            ImGui::PlotHistogram("Presences", arr, scPresences.size(), 0, NULL, 0.0f , 1.0f, ImVec2(0,80) );
            ofxImGui::AddParameter(histoAmp);
            ofxImGui::AddParameter(histoSmooth);
            break;
            
        case 1:
            arr = scBrightnesses.data();
            ImGui::PlotHistogram("Brightnesses", arr, scBrightnesses.size(), 0, NULL, 0.0f , 1.0f, ImVec2(0,80) );
            ofxImGui::AddParameter(luAmp);
            ofxImGui::AddParameter(luSmooth);
            break;
        case 2:
            arr = scSaturations.data();
            ImGui::PlotHistogram("Saturations", arr, scSaturations.size(), 0, NULL, 0.0f , 1.0f, ImVec2(0,80) );
            ofxImGui::AddParameter(satAmp);
            ofxImGui::AddParameter(satSmooth);
            break;
        case 3:
            arr = scXs.data();
            ImGui::PlotHistogram("Horizontal Positions", arr, scXs.size(), 0, NULL, 0.0f , 1.0f, ImVec2(0,80) );
            ofxImGui::AddParameter(panAmp);
            ofxImGui::AddParameter(panSmooth);
            break;
        case 4:
            arr = scYs.data();
            ImGui::PlotHistogram("Vertical Positions", arr, scYs.size(), 0, NULL, 0.0f , 1.0f, ImVec2(0,80) );
            ofxImGui::AddParameter(tiltAmp);
            ofxImGui::AddParameter(tiltSmooth);
            break;
        case 5:
            
            break;
            
    }
}


//-----------


void ofApp::videoArrayOptionSelector(int option){
    float* arr;
    switch (option) {
        case 0:
            arr = presences.data();
            ImGui::PlotHistogram("Presences", arr, presences.size(), 0, NULL, 0.0f , 1.0f, ImVec2(0,80) );
            ofxImGui::AddParameter(histoAmp);
            ofxImGui::AddParameter(histoSmooth);
            break;
            
        case 1:
            arr = brightnesses.data();
            ImGui::PlotHistogram("Brightnesses", arr, brightnesses.size(), 0, NULL, 0.0f , 1.0f, ImVec2(0,80) );
            ofxImGui::AddParameter(luAmp);
            ofxImGui::AddParameter(luSmooth);
            break;
        case 2:
            arr = saturations.data();
            ImGui::PlotHistogram("Saturations", arr, saturations.size(), 0, NULL, 0.0f , 1.0f, ImVec2(0,80) );
            ofxImGui::AddParameter(satAmp);
            ofxImGui::AddParameter(satSmooth);
            break;
        case 3:
            arr = posXs.data();
            ImGui::PlotHistogram("Horizontal Positions", arr, posXs.size(), 0, NULL, 0.0f , 1.0f, ImVec2(0,80) );
            ofxImGui::AddParameter(panAmp);
            ofxImGui::AddParameter(panSmooth);
            break;
        case 4:
            arr = posYs.data();
            ImGui::PlotHistogram("Vertical Positions", arr, posYs.size(), 0, NULL, 0.0f , 1.0f, ImVec2(0,80) );
            ofxImGui::AddParameter(tiltAmp);
            ofxImGui::AddParameter(tiltSmooth);
            break;
        case 5:
            
            break;
            
    }
}



//-------------------------------------------------------------------------------------------------------------------------


//::::	AUTOMATIONS SETUP LOAD & SAVE


//-------------------------------------------------------------------------------------------------------------------------

void ofApp::setupAutomations() {
    
    
    /*------------------- DEFAULT --------------------*/
    
    //
    autoShutDown.set("AUTOSHUTDOWN", 0, 0, 1);
    shutDownHour.set("SHUTDOWN_HOUR", 23, 0, 23);
    shutDownMinute.set("SHUTDOWN_MINUTE", 0, 0, 59);
    toggleFullScreen.set("TOGGLE_FULLSCREEN", false);
    autoStartAudio.set("AUTOSTART_AUDIO", true);
    autoStartVcam.set("AUTOCAM", true);
    autoStartSc.set("AUTOSTART_SC", false);
    
    //AUTOPLAY
    autoPlay.set("AUTOPLAY", false);
    minNumFiles.set("MINIMUM_FILES", 1, 1, 20);
    zapDurationMin.set("ZAP_MINI", 0.5f, 0.15f, 1.0f);
    zapDurationMax.set("ZAP_MAXI", 1.0f, 0.15f, 1.0f);
    randomMovie.set("RANDOM_MOVIES", true);
    silenceTimer.set("SILENCE_TEST_S", 50.0f, 5.0f, 50.0f);
    blackTimer.set("BLACK_FRAME_TEST_S", 50.0f, 5.0f, 50.0f);

    
    //REC
    recTime.set("INSTANT_RECORD_DURATION", 1, 1, 10);
    
    //AUTOREC
    autoRec.set("AUTORECORD", false);
    autoRecTime.set("RECORD_DURATION_MIN", 3, 1, 10);
    interRecTime.set("RECORD_SLEEP_MIN", 15, 1, 30);
    maxLoadTime.set("LOAD_TEST_MS", 1000, 50, 2000);
    minRecDuration.set("DURATION_TEST", 0.5f, 0.0f, 1.0f);
    firstFrameLate.set("FIRST_FRAME_TEST_MS", 500, 50, 2000);
    maxFrameLate.set("FRAME_TEST_MS", 1000, 50, 2000);
    maxNumFiles.set("PLAYLIST_SIZE", 30, 5, 50);
    maxArchive.set("ARCHIVE_SIZE", 50, 5, 500);
    keepErrorFile.set("KEEP_ERROR_FILE", true);
    
    
    //PRINT
    autoPrint.set("AUTOPRINT", false);
    interPrintTime.set("PRINT_SLEEP_MIN", 30, 1, 60);
    
    
    ofLogNotice() << "[::::SETUP  AUTOMATIONS : LOADED::::]";
    
    
    /*------------------- LOAD XML --------------------*/
    
    loadAutomations();
    
    
    
    /*------------------- INIT AUTOMATION STUFF --------------------*/
    
    fileRequested = false;
    
    removeRequested = false;
    
    movieToRemovePath = "";
    
    
    timeFlag = 0;
    
    
    streamIndex = 0;
    
    movieTempo = ofRandom(zapDurationMin, zapDurationMax);
    
    pathToVids = playPath;
    
    playerSelector = 0;
    
    moviePlayer.assign( 2, ofVideoPlayer() );
    //moviePlayer.assign( 2, ofxHAPAVPlayer() );
    
    refreshPlaylist();
    
    if (isFiles) {
        fileName = playlist.at(0);
    } else {
        fileName = "NO_FILES";
    }
    
    nextiD = 0;
    nextMovieLoaded = false;
    
    
    
    sTimer = 0.0f;
    bTimer = 0.0f;
    black = 0;
    
    refreshStreamsList();
    
    managerChange = true;
    
    manager.setup(workPath, ofToDataPath(""), pathToStreamsListFile);
    
    transferManagerSettings();
    
}


//------------------------------------------------------------


void ofApp::loadAutomations() {
    
    ofFile file( ofToDataPath("") + "set/SetupAutomations.xml" );
    
    if (file.exists() ) {
        
        ofXml automations;
        
        automations.load("set/SetupAutomations.xml");
        toggleFullScreen = automations.getIntValue("/AUTOSTART/" + toggleFullScreen.getName() );
        autoStartAudio = automations.getIntValue("/AUTOSTART/" + autoStartAudio.getName() );
        autoStartVcam = automations.getIntValue("/AUTOSTART/" + autoStartVcam.getName() );
        autoStartSc = automations.getIntValue("/AUTOSTART/" + autoStartSc.getName() );
        autoPlay = automations.getIntValue("/AUTOSTART/" + autoPlay.getName() );
        autoRec = automations.getIntValue("/AUTOSTART/" + autoRec.getName() );
        autoPrint = automations.getIntValue("/AUTOSTART/" + autoPrint.getName() );
        
        autoShutDown = automations.getIntValue("/AUTOPLAY/" + autoShutDown.getName() );
        shutDownHour = automations.getIntValue("/AUTOPLAY/" + shutDownHour.getName() );
        shutDownMinute = automations.getIntValue("/AUTOPLAY/" + shutDownMinute.getName() );
        minNumFiles = automations.getIntValue("/AUTOPLAY/" + minNumFiles.getName() );
        zapDurationMin = automations.getFloatValue("/AUTOPLAY/" + zapDurationMin.getName() );
        zapDurationMax = automations.getFloatValue("AUTOPLAY/" + zapDurationMax.getName() );
        randomMovie = automations.getBoolValue("/AUTOPLAY/" + randomMovie.getName() );
        silenceTimer = automations.getIntValue("/AUTOPLAY/" + silenceTimer.getName() );
        blackTimer = automations.getIntValue("/AUTOPLAY/" + blackTimer.getName() );
  
        recTime = automations.getIntValue("/RECORD/" + recTime.getName() );
        
        autoRecTime = automations.getIntValue("/AUTORECORD/" + autoRecTime.getName() );
        interRecTime = automations.getIntValue("/AUTORECORD/" + interRecTime.getName() );
        maxLoadTime = automations.getIntValue("/AUTORECORD/" + maxLoadTime.getName() );
        minRecDuration = automations.getFloatValue("/AUTORECORD/" + minRecDuration.getName() );
        firstFrameLate = automations.getIntValue("/AUTORECORD/" + firstFrameLate.getName() );
        maxFrameLate = automations.getIntValue("/AUTORECORD/" + maxFrameLate.getName() );
        maxNumFiles = automations.getIntValue("/AUTORECORD/" + maxNumFiles.getName() );
        maxArchive = automations.getIntValue("/AUTORECORD/" + maxArchive.getName() );
        
        keepErrorFile = automations.getIntValue("/AUTORECORD/" + keepErrorFile.getName() );
        
        interPrintTime = automations.getIntValue("/PRINT/" + interPrintTime.getName() );
        
        ofLogNotice() << "[::::AUTOMATIONS::::] : LOADED";
        
    }
    
}


//------------------------------------------------------------


void ofApp::saveAutomations() {
    
    ofXml autom;
    autom.addChild("SETUP_AUTOMATIONS");
    
    
    ofXml start;
    start.addChild("AUTOSTART");
    start.addValue(toggleFullScreen.getName(), toggleFullScreen.get() );
    start.addValue(autoStartAudio.getName(), autoStartAudio.get() );
    start.addValue(autoStartVcam.getName(), autoStartVcam.get() );
    start.addValue(autoStartSc.getName(), autoStartSc.get() );
    start.addValue(autoPlay.getName(), autoPlay.get() );
    start.addValue(autoRec.getName(), autoRec.get() );
    start.addValue(autoPrint.getName(), autoPrint.get() );
    
    ofXml play;
    play.addChild("AUTOPLAY");
    play.addValue(autoShutDown.getName(), autoShutDown.get() );
    play.addValue(shutDownHour.getName(), shutDownHour.get() );
    play.addValue(shutDownMinute.getName(), shutDownMinute.get() );
    play.addValue(minNumFiles.getName(), minNumFiles.get() );
    play.addValue(zapDurationMin.getName(), zapDurationMin.get() );
    play.addValue(zapDurationMax.getName(), zapDurationMax.get() );
    play.addValue(randomMovie.getName(), randomMovie.get() );
    play.addValue(silenceTimer.getName(), silenceTimer.get() );
    play.addValue(blackTimer.getName(), blackTimer.get() );

    ofXml rec;
    rec.addChild("RECORD");
    rec.addValue(recTime.getName(), recTime.get() );
    
    ofXml autorec;
    autorec.addChild("AUTORECORD");
    autorec.addValue(autoRecTime.getName(), autoRecTime.get() );
    autorec.addValue(interRecTime.getName(), autoRecTime.get() );
    autorec.addValue(maxLoadTime.getName(), maxLoadTime.get() );
    autorec.addValue(minRecDuration.getName(), minRecDuration.get() );
    autorec.addValue(firstFrameLate.getName(), firstFrameLate.get() );
    autorec.addValue(maxFrameLate.getName(), maxFrameLate.get() );
    autorec.addValue(maxNumFiles.getName(), maxNumFiles.get() );
    autorec.addValue(maxArchive.getName(), maxArchive.get() );
    autorec.addValue(keepErrorFile.getName(), keepErrorFile.get() );
    
    ofXml print;
    print.addChild("PRINT");
    print.addValue(interPrintTime.getName(), interPrintTime.get() );
    
    autom.addXml(start);
    autom.addXml(play);
    autom.addXml(rec);
    autom.addXml(autorec);
    autom.addXml(print);
    
    autom.save("set/SetupAutomations.xml");
    
    ofLogNotice() << "[::::AUTOMATIONS::::] : SAVED";
}





//-------------------------------------------------------------------------------------------------------------------------


//::::	PRINT SETUP LOAD & SAVE


//-------------------------------------------------------------------------------------------------------------------------

void ofApp::loadDefaultGuiParameters() {
    
    /*-------------------DEFAULT GUI--------------------*/
    
    previewWidth = 160;
    previewHeight = 90;
    
    videoWidth = 320;
    videoHeight = 180;
    
    backgroundColor = ofColor(40, 40, 40);
    
    subFrameColor = ofColor(80, 80, 80);
    frameColor = ofColor(150, 150, 150);
    overFrameColor = ofColor(200, 200, 200);
    
    subAudioColor = ofColor::gray;
    audioColor = ofColor::white;
    overAudioColor = ofColor::red;
    
    subFontColor = 100;
    fontColor = ofColor::white;
    overFontColor = 200;
    
    subStroke = 0.5f;
    stroke = 1.0f;
    overStroke = 2.0f;
    
    
}

void ofApp::setupPrint() {
    
    
    //printGui  -> setVisible(false);
    
    
    //THUMBNAILS//-------------------------------------------------------
    
    frameCounter = 0;
    thumbsCursor = 0;
    numThumbs = 10;
    
    thumbsWidth = (int)( ( ofGetWidth() - ( numThumbs * 10 ) ) / numThumbs );
    
    thumbsHeight = (int)( (float)thumbsWidth * ((float)videoHeight / (float)videoWidth) );
    
    
    modulo = thumbsWidth + 10 * ofGetWidth() / 1280;
    
    thumbs.allocate( thumbsWidth, thumbsHeight, OF_PIXELS_RGB );
    thumbs.setColor(ofColor::black);
    
    thumbsTex.assign(numThumbs + 1, ofTexture() );
    
    
    for (int i = 0; i < numThumbs + 1; i++) {
        thumbsTex.at(i).allocate(thumbs);
        thumbsTex.at(i).loadData(thumbs);
    }
    
    ofLogNotice() <<  "[:::::::SETUP::::::::]-----------------------ThumbsNails";
    
    
    
    
    
    //VIDEO GRAPH//-------------------------------------------------------
    
    lastColorGraph.allocate(1, 105, OF_PIXELS_RGB);
    colorGraph.allocate( (int)ofGetWidth() + thumbsWidth, 105, OF_PIXELS_RGB);
    tempColorGraph.allocate( (int)ofGetWidth() + thumbsWidth - 1, 105, OF_PIXELS_RGB);
    
    colorGraphTex.allocate(colorGraph);
    
    lastColorGraph.setColor(ofColor::black);
    tempColorGraph.setColor(ofColor::black);
    colorGraph.setColor(ofColor::black);
    
    
    ofLogNotice() << "[:::::::SETUP::::::::]-----------------------ColorGraph";
    
    
    
    
    
    //SOUND GRAPH//-------------------------------------------------------
    
    soundGraphPointsL.resize(1280);
    soundGraphPointsR.resize(1280);
    
    lastSoundGraph.allocate(1, 105, OF_PIXELS_RGB);
    soundGraph.allocate( (int)ofGetWidth() + thumbsWidth, 105, OF_PIXELS_RGB);
    tempSoundGraph.allocate( (int)ofGetWidth() + thumbsWidth - 1, 105, OF_PIXELS_RGB);
    soundGraph.setColor(ofColor::black);
    
    soundGraphTex.allocate(soundGraph);
    
    lastSoundGraph.setColor(ofColor::black);
    tempSoundGraph.setColor(ofColor::black);
    soundGraph.setColor(ofColor::black);
    
    memoryLooper = 0;
    memoryRMS.resize(1280);
    
    ofLogNotice() << "[:::::::SETUP::::::::]-----------------------SoundGraph";
    
    
    
    
    
}



void ofApp::drawImGuiAutomation(){
    
    
    
    if (ofxImGui::BeginWindow("::::: AUTOMATIONS :::::", guiSettings, window_flags))
        
    {
        ImGui::Text("");
        if (ImGui::Button("::::: SAVE :::::") ) {
            saveAutomations();
        }
        
        ImGui::SameLine();
        if (ofxImGui::AddParameter(volume) ){
            compressor.setParameter( 6, kAudioUnitScope_Global, volume);
        }
        ImGui::Text("");
        
        
        static bool open = true;
        
        //DEVICES
        ImGui::OpenNextNode(true);
        if (ImGui::CollapsingHeader("::::: PLAY :::::", open) ){
            
            ImGui::Text("");
            if (ofxImGui::AddRadio(movieNum, playlist, 2) ) {
                if (HAPmode.get() ) {
                    hap.change(movieNum);
                }
                else {
                    changeFile(movieNum);
                }
            }
            ImGui::Text("");
            //            if (ImGui::TreeNode("Audio Output") )
            //            {
            //
            //                ImGui::TreePop();
            //            }
            //            ImGui::Unindent();
        }
        ImGui::OpenNextNode(true);
        if (ImGui::CollapsingHeader("::::: AUTOPLAY :::::", open) ){
            ImGui::Text("");
            if ( ofxImGui::AddRadio(autoPlay, onOffOptions, 2) ){
                if (HAPmode.get() ) {
                    hap.autoplay( autoPlay );
                }
            }
            ImGui::Text("");
            if ( ofxImGui::AddRange("Movie Duration Range (min)", zapDurationMin, zapDurationMax) ) {
                if (HAPmode.get() ) {
                    hap.setRangeDuration( iround(zapDurationMin * 60) ,  iround(zapDurationMax * 60)  );
                }
            }
            ImGui::Text("");
            if ( ofxImGui::AddRadio(randomMovie, onOffOptions, 2) ){
                if (HAPmode.get() ) {
                    hap.random( randomMovie );
                }
            }
            ImGui::Text("");
            if ( ofxImGui::AddParameter(silenceTimer) ) {
                if (HAPmode.get() ) {
                    hap.setMaxSilence(silenceTimer );
                }
            }
            ImGui::Text("");
            ofxImGui::AddParameter(blackTimer);
            ImGui::Text("");
            if ( ofxImGui::AddParameter(minNumFiles) ) {
                if (HAPmode.get() ) {
                    hap.setMinimumFile(minNumFiles );
                }
            }
            ImGui::Text("");
        }
        
        /*
        ImGui::OpenNextNode(true);
        if (ImGui::CollapsingHeader("::::: RECORD :::::", open) ){
            ImGui::Text("");
            ofxImGui::AddParameter(recTime);
            ImGui::Text("");
            if (ofxImGui::AddRadio(streamNum, streamsNames, 3) ) recStream(streamNum);
            ImGui::Text("");
        }
        ImGui::OpenNextNode(true);
        if (ImGui::CollapsingHeader("::::: AUTORECORD :::::", open) ){
            ImGui::Text("");
            if (ofxImGui::AddRadio(autoRec, onOffOptions, 2) ){
                if (autoRec) manager.runRec(); else manager.stop();
            }
//            ImGui::Text("");
//            ofxImGui::AddRadio(recHAP, onOffOptions, 2);
            ImGui::Text("");
            ofxImGui::AddParameter(autoRecTime);
            ImGui::Text("");
            ofxImGui::AddParameter(interRecTime);
            ImGui::Text("");
            ofxImGui::AddParameter(maxLoadTime);
            ImGui::Text("");
            ofxImGui::AddParameter(minRecDuration);
            ImGui::Text("");
            ofxImGui::AddParameter(firstFrameLate);
            ImGui::Text("");
            ofxImGui::AddParameter(maxFrameLate);
            ImGui::Text("");
            ofxImGui::AddParameter(maxNumFiles);
            ImGui::Text("");
            ofxImGui::AddParameter(maxArchive);
            ImGui::Text("");
            ofxImGui::AddRadio(keepErrorFile, onOffOptions, 2);
            ImGui::Text("");
            if (ImGui::Button(":: Apply Autorecord Options ::") ) managerChange = true;
            ImGui::Text("");
        }
        */
        
        ImGui::OpenNextNode(true);
        if (ImGui::CollapsingHeader("::::: AUTOSTART OPTIONS :::::") ){
            ImGui::Text("");
            ofxImGui::AddRadio(autoShutDown, onOffOptions, 2);
            ImGui::Text("");
            ofxImGui::AddParameter(shutDownHour);
            ImGui::Text("");
            ofxImGui::AddParameter(shutDownMinute);
            ImGui::Text("");
            ofxImGui::AddRadio(toggleFullScreen, onOffOptions, 2);
            ImGui::Text("");
            ofxImGui::AddRadio(autoStartAudio, onOffOptions, 2);
            ImGui::Text("");
            ofxImGui::AddRadio(autoStartVcam, onOffOptions, 2);
            ImGui::Text("");
            ofxImGui::AddRadio(autoStartSc, onOffOptions, 2);
            ImGui::Text("");
            ofxImGui::AddRadio(autoPlay, onOffOptions, 2);
            ImGui::Text("");
//            ofxImGui::AddRadio(autoRec, onOffOptions, 2);
//            ImGui::Text("");
            
            //ofxImGui::AddRadio(autoPrint, onOffOptions, 2);
            //ImGui::Text("");
            
            
        }
        
        
        ImGui::Text("");
        if (ImGui::Button("::::: SAVE :::::") ) {
            saveAutomations();
        }
        ImGui::Text("");
        
    }
    
    ofxImGui::EndWindow(guiSettings);
    
}
//-------------------------------------------------------------------------------------------------------------------------


//::::	KEY CONTROLS


//-------------------------------------------------------------------------------------------------------------------------


void ofApp::keyReleased(int key) {
    
    //cout << key << "\n";
    /* 1 -> 0
     38
     233
     34
     39
     
     40
     167
     232
     33
     231
     224
     4352
     
     4353
     */
    
    switch (key) {
            
        case 38 :
            changeView("Analyse Video");
            break;
            
        case 233 :
            changeView("Analyse Audio");
            break;
            
        case 34 :
            changeView("Broadcast");
            break;
            
        case 39 :
            changeView("Print");
            break;
            
        case 40 :
            changeView("Automations");
            break;
            
            
        case ' ':
            if (playing) { pause(); }
            else { play(); }
            break;
            
        case 'f':
            if(viewName.get() == "Broadcast"){
                
                showFrameRate = !showFrameRate;
            }
            break;
            
            
            
        case OF_KEY_RETURN:
            
            break;
            
            
        case OF_KEY_RIGHT :
            if (HAPmode.get() ) {
                hap.next();
            }
            else {
                movieNum = (movieNum + 1)%playlist.size();
                changeFile(movieNum);
            }
            break;
            
        case OF_KEY_LEFT :
            if (HAPmode.get() ) {
                hap.previous();
            }
            else {
                movieNum = (playlist.size() + movieNum - 1)%playlist.size();
                changeFile(movieNum);
            }
            break;
            
        case OF_KEY_UP :
            break;
            
            
        case OF_KEY_DOWN :
            
            ofLogNotice() << "\n";
            ofLogNotice() << "[ ::: DEBUG ::: ]" << "\n" << debug;
            ofLogNotice() << "\n";
            cout << "[ ::: DEBUG ::: ] "  << debug << "\n";
            break;
            
        case '$' :
            saveAutomations();
            saveConfig();
            
            break;
            
        case 'k' :
            
            break;
            
        case'r' :
            //cout << videoCamera.getWidth() << " : " << videoCamera.getHeight() << endl;
            refreshPlaylist();
            break;
            
        case's' :
            
            break;
            
        case'l' :
            
            break;
            
            
        case'd' :
            
            break;
            
        case'i' :
            
            break;
            
        case 'p' :
            //shoot = true;
            break;
    }
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ) {
    
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






//-------------------------------------------------------------------------------------------------------------------------


//::::	VIEW STUFF


//-------------------------------------------------------------------------------------------------------------------------


void ofApp::changeView(int newNumView) {
    
    newNumView = (viewOptions.size() + newNumView ) % ( viewOptions.size() );
    
    changeView(viewOptions.at(newNumView));
    
    viewNum = newNumView;
    
}


void ofApp::changeView(string newView) {
    
    
    if (newView == "Analyse Video") {
        
        ofSetWindowTitle("Analyse Video" );
        ofSetFullscreen(false);
        ofShowCursor();
        
        viewNum = 0;
        
    }
    
    if (newView == "Analyse Audio") {
        
        ofSetWindowTitle("Analyse Audio");
        ofSetFullscreen(false);
        ofShowCursor();
        
        viewNum = 1;
    }
    
    
    if (newView == "Broadcast") {
        
        ofSetWindowTitle("Broadcast");
        if (toggleFullScreen) ofSetFullscreen(true);
        ofHideCursor();
        
        viewNum = 2;
    }
    
    
    if (newView == "Print") {
        
        ofSetWindowTitle("Print");
        ofSetFullscreen(false);
        ofShowCursor();
        
        viewNum = 3;
    }
    
    
    if (newView == "Automations") {
        
        ofSetWindowTitle("Automations");
        ofSetFullscreen(false);
        ofShowCursor();
        
        viewNum = 4;
    }
    
    
    viewName = newView;
    
    
}





//-------------------------------------------------------------------------------------------------------------------------


//::::	SPECTRUM STUFF


//-------------------------------------------------------------------------------------------------------------------------


ofPixels ofApp::getSpectrum(size_t size, bool bicubic) {
    
    ofPixels spectrum;
    vector<double> hues ={0, 21.25, 42.5, 85, 127.5, 170, 190};
    vector<float> waveLength ={780, 595, 580, 550, 490, 465, 380};
    vector<double> indexes;
    indexes.resize(waveLength.size() );
    
    for (int i = 0; i < waveLength.size(); i++ ) {
        
        float v = ofMap(waveLength.at(i), 780, 380, 0, size-1);
        
        indexes[i] = (double)v;
        
    }
    
    tk::spline s;
    
    //s.set_boundary(spline::bd_type left, double left_value,spline::bd_type right, double right_value, bool force_linear_extrapolation)
    //s.set_boundary(1, 0, 2, 192, true);
    
    s.set_points(indexes, hues, bicubic);
    
    spectrum.allocate(size, 1, OF_PIXELS_RGB);
    
    for (int i = 0; i < size; i++) {
        ofColor c;
        float h = (float) s(i);
        if (h < 0) {h = 0; }
        c.setHsb(h, 255, 255);
        spectrum.setColor(i, 0, c);
    }
    
    return spectrum;
}


/*---------------------------------------------------------------*/

vector<double> ofApp::getHuesToSpectrum(ofPixels spectrum) {
    vector<double> indexes;
    int hue = -1;
    for (int i = 0; i  < spectrum.getWidth(); i++) {
        int newHue = iround(spectrum.getColor(i, 0).getHue() );
        if (newHue > hue) {
            indexes.push_back(i);
            hue = newHue;
        }
    }
    
    indexes.push_back(1023);
    
    ofLogNotice() << "Hues indexes : " << ofToString(indexes.size() );
    ofLogNotice() << "Indexes values : " << ofToString(indexes );
    
    return indexes;
}
/*---------------------------------------------------------------*/

vector<float> ofApp::getHueSpectrum(ofPixels spectrum) {
    
    vector<float> hues;
    
    for (int i = 0; i < spectrum.getWidth(); i++) {
        
        hues.push_back(spectrum.getColor(i, 0).getHue() );
    }
    
    return hues;
    
}

/*---------------------------------------------------------------*/

vector<float> ofApp::getColorFrequencies(size_t size) {
    
    vector<float> frequencies;
    double c = 2.9979 * 10e8;
    
    for (int i = 0; i < size; i++) {
        
        double w = ofMap(i, 0, size, 780, 380);
        
        float f = (float)(w / c);
        frequencies.push_back(f);
        
    }
    
    return frequencies;
    
}

/*---------------------------------------------------------------*/

vector<float> ofApp::getAudioFrequencies(size_t size) {
    
    vector<float> frequencies;
    
    float fOffset = 22050.0f / size;
    
    for (int i = 0; i < size; i++) {
        
        float f = ofMap(i, 0, size, 0.0f, 22050.0f);
        frequencies.push_back(f + fOffset);
        
    }
    
    return frequencies;
    
}
/*---------------------------------------------------------------*/
void frequencyToNote(float frequency, int& octave, int& note, float& cents) {
    float x = logf(frequency / 440.f) / logf(2.f) + 4.f;
    octave = floorf(x);
    x -= octave;
    x *= 12.f;
    note = roundf(x);
    x -= note;
    x *= 100.f;
    cents = x;
}
/*---------------------------------------------------------------*/
//0 = 8.1757989156
// float midi[127];
// int a = 440; // a is 440 hz...
// for (int x = 0; x < 127; ++x)
// {
//    midi[x] = (a / 32) * (2 ^ ((x - 9) / 12));
// }

float ofApp::cpsMidi(float f) {
    
    int midiNote;
    
    midiNote = (440 / 32) * pow(2 ,  (f - 9) / 12 );
    
    return midiNote;
    
}



/*---------------------------------------------------------------*/





/*---------------------------------------------------------------*/


//MONO//


ofPixels ofApp::doGradient(int fMin, int fMax, int cMin, int cMax, int mMin, int mMax) {
    
    float gRot, gBri, gSat, gAlpha;
    
    vector <double> presences, presences2, rotations, brightnesses, saturations, alphas;
    
    vector <double> presencesIndexes, presences2Indexes, rotationsIndexes, brightnessesIndexes, saturationsIndexes, alphasIndexes;
    
    
    
    ofPixels grad;
    
    /*-----------------------AUDIOVIDEO-----------------------*/
    
    switch (avBModOption) {
        case 0 :
            avBMod = power;
            break;
        case 1 :
            avBMod = pitchFreqNorm;
            break;
        case 2 :
            avBMod = pitchSalience;
            break;
        case 3 :
            avBMod = hfcNorm;
            break;
        case 4 :
            avBMod = specCompNorm ;
            break;
        case 5 :
            avBMod = centroidNorm ;
            break;
        case 6 :
            avBMod = dissonance ;
            break;
        case 7 :
            avBMod = rollOffNorm ;
            break;
        case 8 :
            avBMod = oddToEvenNorm ;
            break;
        case 9 :
            avBMod = strongPeakNorm ;
            break;
        case 10 :
            avBMod = strongDecayNorm ;
            break;
        case 11 :
            avBMod = 0.0f;
            break;
            
    }
    
    avBMod = avBMod*avBModIntensity + audioVideoBrightness;
    
    
    
    switch (avCModOption) {
        case 0 :
            avCMod = power;
            break;
        case 1 :
            avCMod = pitchFreqNorm;
            break;
        case 2 :
            avCMod = pitchSalience;
            break;
        case 3 :
            avCMod = hfcNorm;
            break;
        case 4 :
            avCMod = specCompNorm ;
            break;
        case 5 :
            avCMod = centroidNorm ;
            break;
        case 6 :
            avCMod = dissonance ;
            break;
        case 7 :
            avCMod = rollOffNorm ;
            break;
        case 8 :
            avCMod = oddToEvenNorm ;
            break;
        case 9 :
            avCMod = strongPeakNorm ;
            break;
        case 10 :
            avCMod = strongDecayNorm ;
            break;
        case 11 :
            avCMod = 0.0f;
            break;
            
    }
    
    avCMod = avCMod*avCModIntensity + audioVideoContrast;
    
    
    
    switch (avSModOption) {
        case 0 :
            avSMod = power;
            break;
        case 1 :
            avSMod = pitchFreqNorm;
            break;
        case 2 :
            avSMod = pitchSalience;
            break;
        case 3 :
            avSMod = hfcNorm;
            break;
        case 4 :
            avSMod = specCompNorm ;
            break;
        case 5 :
            avSMod = centroidNorm ;
            break;
        case 6 :
            avSMod = dissonance ;
            break;
        case 7 :
            avSMod = rollOffNorm ;
            break;
        case 8 :
            avSMod = oddToEvenNorm ;
            break;
        case 9 :
            avSMod = strongPeakNorm ;
            break;
        case 10 :
            avSMod = strongDecayNorm ;
            break;
        case 11 :
            avSMod = 0.0f;
            break;
            
    }
    
    avSMod = avSMod*avSModIntensity + audioVideoSaturation;
    
    
    
    switch (avAModOption) {
        case 0 :
            avAMod = power;
            break;
        case 1 :
            avAMod = pitchFreqNorm;
            break;
        case 2 :
            avAMod = pitchSalience;
            break;
        case 3 :
            avAMod = hfcNorm;
            break;
        case 4 :
            avAMod = specCompNorm ;
            break;
        case 5 :
            avAMod = centroidNorm ;
            break;
        case 6 :
            avAMod = dissonance ;
            break;
        case 7 :
            avAMod = rollOffNorm ;
            break;
        case 8 :
            avAMod = oddToEvenNorm ;
            break;
        case 9 :
            avAMod = strongPeakNorm ;
            break;
        case 10 :
            avAMod = strongDecayNorm ;
            break;
        case 11 :
            avAMod = 0.0f;
            break;
            
    }
    
    avAMod = avAMod*avAModIntensity + audioVideoAlpha;
    
    
    
    /*----------ROTATION-------------*/
    
    switch (globalRotation.get() ) {
        case 0 :
            gRot = power * globalRotationAdjust.get();
            break;
        case 1 :
            gRot = pitchFreqNorm * globalRotationAdjust.get();
            break;
        case 2 :
            gRot = pitchSalience * globalRotationAdjust.get();
            break;
        case 3 :
            gRot = hfcNorm * globalRotationAdjust.get();
            break;
        case 4 :
            gRot = specCompNorm * globalRotationAdjust.get();
            break;
        case 5 :
            gRot = centroidNorm * globalRotationAdjust.get();
            break;
        case 6 :
            gRot = dissonance * globalRotationAdjust.get();
            break;
        case 7 :
            gRot = rollOffNorm * globalRotationAdjust.get();
            break;
        case 8 :
            gRot = oddToEvenNorm * globalRotationAdjust.get();
            break;
        case 9 :
            gRot = strongPeakNorm * globalRotationAdjust.get();
            break;
        case 10 :
            gRot = strongDecayNorm * globalRotationAdjust.get();
            break;
        case 11 :
            gRot = 0.0f;
            break;
            
    }
    
    
    /*----------BRIGHTNESS-------------*/
    
    switch (globalBrightness.get() ) {
        case 0 :
            gBri = power * globalBrightnessAdjust.get();
            break;
        case 1 :
            gBri = pitchFreqNorm * globalBrightnessAdjust.get();
            break;
        case 2 :
            gBri = pitchSalience * globalBrightnessAdjust.get();
            break;
        case 3 :
            gBri = hfcNorm * globalBrightnessAdjust.get();
            break;
        case 4 :
            gBri = specCompNorm * globalBrightnessAdjust.get();
            break;
        case 5 :
            gBri = centroidNorm * globalBrightnessAdjust.get();
            break;
        case 6 :
            gBri = dissonance * globalBrightnessAdjust.get();
            break;
        case 7 :
            gBri = rollOffNorm * globalBrightnessAdjust.get();
            break;
        case 8 :
            gBri = oddToEvenNorm * globalBrightnessAdjust.get();
            break;
        case 9 :
            gBri = strongPeakNorm * globalBrightnessAdjust.get();
            break;
        case 10 :
            gBri = strongDecayNorm * globalBrightnessAdjust.get();
            break;
        case 11 :
            gBri = 0.0f;
            break;
    }
    
    
    /*----------SATURATION-------------*/
    
    switch (globalSaturation.get() ) {
        case 0 :
            gSat = power * globalSaturationAdjust.get();
            break;
        case 1 :
            gSat = pitchFreqNorm * globalSaturationAdjust.get();
            break;
        case 2 :
            gSat = pitchSalience * globalSaturationAdjust.get();
            break;
        case 3 :
            gSat = hfcNorm * globalSaturationAdjust.get();
            break;
        case 4 :
            gSat = specCompNorm * globalSaturationAdjust.get();
            break;
        case 5 :
            gSat = centroidNorm * globalSaturationAdjust.get();
            break;
        case 6 :
            gSat = dissonance * globalSaturationAdjust.get();
            break;
        case 7 :
            gSat = rollOffNorm * globalSaturationAdjust.get();
            break;
        case 8 :
            gSat = oddToEvenNorm * globalSaturationAdjust.get();
            break;
        case 9 :
            gSat = strongPeakNorm * globalSaturationAdjust.get();
            break;
        case 10 :
            gSat = strongDecayNorm * globalSaturationAdjust.get();
            break;
        case 11 :
            gSat = 0.0f;
            break;
    }
    
    /*----------ALPHA-------------*/
    
    switch (globalAlpha.get() ) {
        case 0 :
            gAlpha = power * globalAlphaAdjust.get();
            break;
        case 1 :
            gAlpha = pitchFreqNorm * globalAlphaAdjust.get();
            break;
        case 2 :
            gAlpha = pitchSalience * globalAlphaAdjust.get();
            break;
        case 3 :
            gAlpha = hfcNorm * globalAlphaAdjust.get();
            break;
        case 4 :
            gAlpha = specCompNorm * globalAlphaAdjust.get();
            break;
        case 5 :
            gAlpha = centroidNorm * globalAlphaAdjust.get();
            break;
        case 6 :
            gAlpha = dissonance * globalAlphaAdjust.get();
            break;
        case 7 :
            gAlpha = rollOffNorm * globalAlphaAdjust.get();
            break;
        case 8 :
            gAlpha = oddToEvenNorm * globalAlphaAdjust.get();
            break;
        case 9 :
            gAlpha = strongPeakNorm * globalAlphaAdjust.get();
            break;
        case 10 :
            gAlpha = strongDecayNorm * globalAlphaAdjust.get();
            break;
        case 11 :
            gAlpha = 0.0f;
            break;
    }
    
    
    
    /*----------VECTOR-------------*//*----------DATA-------------*/
    
    if (cMin > cMax){
        int c = cMin;
        cMin = cMax;
        cMax = c;
    }
    
    if ( (cMax-cMin) <=2) {cMax = cMin+3;}
    
    if (fMin > fMax){
        int f = fMin;
        fMin = fMax;
        fMax = f;
    }
    if ( (fMax-fMin) <=2) {fMax = fMin+3;}
    
    if (mMin > mMax){
        int m = mMin;
        mMin = mMax;
        mMax = m;
    }
    if ( (mMax-mMin) <=2) {mMax = mMin+3;}
    
    /*----------PRESENCES-------------*/
    
    tk::spline p;
    
    switch ( perBandPresence.get() ) {
        case 0:
            //presences.resize( (fMax-fMin + 1) );
            //presencesIndexes.resize( (fMax-fMin + 1) );
            for (int i = fMin; i < fMax; i++) {
                float val = spectrumNorm.at(i);
                presences.push_back( val * perBandPresenceAdjust.get() );
                presencesIndexes.push_back( ofMap( i, fMin, fMax, cMin, cMax) );
            }
            
            break;
        case 1:
            //presences.resize( melBands.size() );
            //presencesIndexes.resize( melBands.size() );
            for (int i = mMin; i < mMax; i++) {
                float val = ofMap(melBands[i], DB_MIN, DB_MAX, 0.0, 1.0, true);
                presences.push_back(val * perBandPresenceAdjust.get() );
                presencesIndexes.push_back( ofMap(i, mMin, mMax, cMin, cMax) );
            }
            break;
        case 2:
            //presences.resize( mfcc.size() );
            //presencesIndexes.resize( mfcc.size() );
            for (int i = 0; i < mfcc.size(); i++) {
                float val = ofMap(mfcc[i], 0, MFCC_MAX_ESTIMATED_VALUE, 0.0, 1.0, true);
                presences.push_back(val * perBandPresenceAdjust.get() );
                presencesIndexes.push_back( ofMap(i, 0, mfcc.size(), cMin, cMax) );
            }
            break;
        case 3:
            //presences.resize( hpcp.size() );
            //presencesIndexes.resize( hpcp.size() );
            for (int i = 0; i < hpcp.size(); i++) {
                presences.push_back(hpcp.at(i) * perBandPresenceAdjust.get() );
                presencesIndexes.push_back( ofMap(i, 0, hpcp.size(), cMin, cMax) );
            }
            break;
        case 4:
            //presences.resize( tristimulus.size() );
            //presencesIndexes.resize( tristimulus.size() );
            for (int i = 0; i < tristimulus.size(); i++) {
                presences.push_back(tristimulus.at(i) * perBandPresenceAdjust.get() );
                presencesIndexes.push_back( ofMap(i, 0, tristimulus.size(), cMin, cMax) );
            }
            break;
            //        case 5:
            //            //presences.resize( multiPitches.size() );
            //            //presencesIndexes.resize( multiPitches.size() );
            //            for (int i = 0; i < multiPitchesSaliences.size(); i++) {
            //                presences.push_back(multiPitchesSaliences.at(i) * perBandPresenceAdjust.get() );
            //                presencesIndexes.push_back( ofMap(i, 0, multiPitches.size(), cMin, cMax) );
            //            }
            //            break;
            //        case 6:
            //            //presences.resize( multiPitches.size() );
            //            //presencesIndexes.resize( multiPitches.size() );
            //            for (int i = fMin; i < fMax; i++) {
            //                presences.push_back(perBandPresenceAdjust.get() );
            //                presencesIndexes.push_back( ofMap(i, fMin, fMax, cMin, cMax) );
            //            }
            //            break;
            
        case 5 :
            for (int i = fMin; i < fMax; i++) {
                presences.push_back(0.0f );
                presencesIndexes.push_back( ofMap(i, fMin, fMax, cMin, cMax) );
            }
            break;
    }
    
    p.set_points(presencesIndexes, presences);
    
    
    /*----------PRESENCES2-------------*/
    
    tk::spline p2;
    
    switch ( perBandPresence2.get() ) {
        case 0:
            //presences.resize( (fMax-fMin + 1) );
            //presencesIndexes.resize( (fMax-fMin + 1) );
            for (int i = fMin; i < fMax; i++) {
                float val = spectrumNorm.at(i);
                presences2.push_back( val * (1-perBandPresenceAdjust.get()) );
                presences2Indexes.push_back( ofMap( i, fMin, fMax, cMin, cMax) );
            }
            
            break;
        case 1:
            //presences.resize( melBands.size() );
            //presencesIndexes.resize( melBands.size() );
            for (int i = mMin; i < mMax; i++) {
                float val = ofMap(melBands[i], DB_MIN, DB_MAX, 0.0, 1.0, true);
                presences2.push_back(val * (1-perBandPresenceAdjust.get()) );
                presences2Indexes.push_back( ofMap(i, mMin, mMax, cMin, cMax) );
            }
            break;
        case 2:
            //presences.resize( mfcc.size() );
            //presencesIndexes.resize( mfcc.size() );
            for (int i = 0; i < mfcc.size(); i++) {
                float val = ofMap(mfcc[i], 0, MFCC_MAX_ESTIMATED_VALUE, 0.0, 1.0, true);
                presences2.push_back(val * (1-perBandPresenceAdjust.get()) );
                presences2Indexes.push_back( ofMap(i, 0, mfcc.size(), cMin, cMax) );
            }
            break;
        case 3:
            //presences.resize( hpcp.size() );
            //presencesIndexes.resize( hpcp.size() );
            for (int i = 0; i < hpcp.size(); i++) {
                presences2.push_back(hpcp.at(i) * (1-perBandPresenceAdjust.get()) );
                presences2Indexes.push_back( ofMap(i, 0, hpcp.size(), cMin, cMax) );
            }
            break;
        case 4:
            //presences.resize( tristimulus.size() );
            //presencesIndexes.resize( tristimulus.size() );
            for (int i = 0; i < tristimulus.size(); i++) {
                presences2.push_back(tristimulus.at(i) * (1-perBandPresenceAdjust.get()) );
                presences2Indexes.push_back( ofMap(i, 0, tristimulus.size(), cMin, cMax) );
            }
            break;
            //        case 5:
            //            //presences.resize( multiPitches.size() );
            //            //presencesIndexes.resize( multiPitches.size() );
            //            for (int i = 0; i < multiPitchesSaliences.size(); i++) {
            //                presences2.push_back(multiPitchesSaliences.at(i) * (1-perBandPresenceAdjust.get()) );
            //                presences2Indexes.push_back( ofMap(i, 0, multiPitches.size(), cMin, cMax) );
            //            }
            //            break;
            //        case 6:
            //            //presences.resize( multiPitches.size() );
            //            //presencesIndexes.resize( multiPitches.size() );
            //            for (int i = fMin; i < fMax; i++) {
            //                presences2.push_back( 1-perBandPresenceAdjust.get() );
            //                presences2Indexes.push_back( ofMap(i, fMin, fMax, cMin, cMax) );
            //            }
            //            break;
            
        case 5 :
            for (int i = fMin; i < fMax; i++) {
                presences2.push_back(0.0f );
                presences2Indexes.push_back( ofMap(i, fMin, fMax, cMin, cMax) );
            }
            break;
    }
    
    p2.set_points(presences2Indexes, presences2);
    
    
    
    /*----------ROTATIONS-------------*/
    
    tk::spline r;
    
    switch ( perBandRotation.get() ) {
        case 0:
            //rotations.resize( fMax - fMin + 1);
            //rotationsIndexes.resize( fMax - fMin + 1);
            for (int i = fMin; i < fMax; i++) {
                float val = spectrumNorm.at(i);
                rotations.push_back(val * perBandRotationAdjust.get() );
                rotationsIndexes.push_back( ofMap(i, fMin, fMax, cMin, cMax) );
            }
            
            break;
        case 1:
            //rotations.resize( melBands.size() );
            //rotationsIndexes.resize( melBands.size() );
            for (int i = mMin; i < mMax; i++) {
                float val = ofMap(melBands[i], DB_MIN, DB_MAX, 0.0, 1.0, true);
                rotations.push_back(val * perBandRotationAdjust.get() );
                rotationsIndexes.push_back( ofMap(i, mMin,mMax, cMin, cMax) );
            }
            break;
        case 2:
            //rotations.resize( mfcc.size() );
            //rotationsIndexes.resize( mfcc.size() );
            for (int i = 0; i < mfcc.size(); i++) {
                float val = ofMap(mfcc[i], 0, MFCC_MAX_ESTIMATED_VALUE, 0.0, 1.0, true);
                rotations.push_back(val * perBandRotationAdjust.get() );
                rotationsIndexes.push_back( ofMap(i, 0, mfcc.size(), cMin, cMax) );
            }
            break;
        case 3:
            //rotations.resize( hpcp.size() );
            //rotationsIndexes.resize( hpcp.size() );
            for (int i = 0; i < hpcp.size(); i++) {
                rotations.push_back(hpcp.at(i) * perBandRotationAdjust.get() );
                rotationsIndexes.push_back( ofMap(i, 0, hpcp.size(), cMin, cMax) );
            }
            break;
        case 4:
            //rotations.resize( tristimulus.size() );
            //rotationsIndexes.resize( tristimulus.size() );
            for (int i = 0; i < tristimulus.size(); i++) {
                rotations.push_back(tristimulus.at(i) * perBandRotationAdjust.get() );
                rotationsIndexes.push_back( ofMap(i, 0, tristimulus.size(), cMin, cMax) );
            }
            break;
            
            //        case 5:
            //            //rotations.resize( multiPitches.size() );
            //            //rotationsIndexes.resize( multiPitches.size() );
            //            for (int i = 0; i < multiPitchesSaliences.size(); i++) {
            //                rotations.push_back(multiPitchesSaliences.at(i) * perBandRotationAdjust.get() );
            //                rotationsIndexes.push_back( ofMap(i, 0, multiPitchesSaliences.size(), cMin, cMax) );
            //            }
            //            break;
            //
            //        case 6:
            //            for (int i = fMin; i < fMax; i++) {
            //                rotations.push_back(perBandRotationAdjust.get() );
            //                rotationsIndexes.push_back( ofMap(i, fMin, fMax, cMin, cMax) );
            //            }
            //            break;
            
        case 5 :
            for (int i = fMin; i < fMax; i++) {
                rotations.push_back(0.0f );
                rotationsIndexes.push_back( ofMap(i, fMin, fMax, cMin, cMax) );
            }
            break;
    }
    
    
    r.set_points(rotationsIndexes, rotations);
    
    
    /**----------BRIGHTNESSES-------------*/
    
    tk::spline b;
    
    switch ( perBandBrightness.get() ) {
        case 0:
            //brightnesses.resize( fMax - fMin + 1);
            //brightnessesIndexes.resize( fMax - fMin + 1);
            for (int i = fMin; i < fMax; i++) {
                float val = spectrumNorm.at(i);
                brightnesses.push_back(val * perBandBrightnessAdjust.get() );
                brightnessesIndexes.push_back( ofMap(i, fMin, fMax, cMin, cMax) );
            }
            
            break;
        case 1:
            //brightnesses.resize( melBands.size() );
            //brightnessesIndexes.resize( melBands.size() );
            for (int i = mMin; i < mMax; i++) {
                float val = ofMap(melBands[i], DB_MIN, DB_MAX, 0.0, 1.0, true);
                brightnesses.push_back(val * perBandBrightnessAdjust.get() );
                brightnessesIndexes.push_back( ofMap(i, mMin, mMax, cMin, cMax) );
            }
            break;
        case 2:
            //brightnesses.resize( mfcc.size() );
            //brightnessesIndexes.resize( mfcc.size() );
            for (int i = 0; i < mfcc.size(); i++) {
                float val = ofMap(mfcc[i], 0, MFCC_MAX_ESTIMATED_VALUE, 0.0, 1.0, true);
                brightnesses.push_back(val * perBandBrightnessAdjust.get() );
                brightnessesIndexes.push_back( ofMap(i, 0, mfcc.size(), cMin, cMax) );
            }
            break;
        case 3:
            //brightnesses.resize( hpcp.size() );
            //brightnessesIndexes.resize( hpcp.size() );
            for (int i = 0; i < hpcp.size(); i++) {
                brightnesses.push_back(hpcp.at(i) * perBandBrightnessAdjust.get() );
                brightnessesIndexes.push_back( ofMap(i, 0, hpcp.size(), cMin, cMax) );
            }
            break;
        case 4:
            //brightnesses.resize( tristimulus.size() );
            //brightnessesIndexes.resize( tristimulus.size() );
            for (int i = 0; i < tristimulus.size(); i++) {
                brightnesses.push_back(tristimulus.at(i) * perBandBrightnessAdjust.get() );
                brightnessesIndexes.push_back( ofMap(i, 0, tristimulus.size(), cMin, cMax) );
            }
            break;
            //        case 5:
            //            //brightnesses.resize( multiPitches.size() );
            //            //brightnessesIndexes.resize( multiPitches.size() );
            //            for (int i = 0; i < multiPitchesSaliences.size(); i++) {
            //                brightnesses.push_back(multiPitchesSaliences.at(i) * perBandBrightnessAdjust.get() );
            //                brightnessesIndexes.push_back( ofMap(i, 0, multiPitchesSaliences.size(), cMin, cMax) );
            //            }
            //            break;
            
            //        case 6:
            //            for (int i = fMin; i < fMax; i++) {
            //                brightnesses.push_back(perBandBrightnessAdjust.get() );
            //                brightnessesIndexes.push_back( ofMap(i, fMin, fMax, cMin, cMax) );
            //            }
            //            break;
            
        case 5 :
            for (int i = fMin; i < fMax; i++) {
                brightnesses.push_back(0.0f );
                brightnessesIndexes.push_back( ofMap(i, fMin, fMax, cMin, cMax) );
            }
            break;
            
    }
    
    
    b.set_points(brightnessesIndexes, brightnesses);
    
    
    /**----------SATURATIONS-------------*/
    
    
    tk::spline s;
    
    switch ( perBandSaturation.get() ) {
        case 0:
            //saturations.resize( fMax-fMin + 1);
            //saturationsIndexes.resize( fMax-fMin + 1);
            for (int i = fMin; i < fMax; i++) {
                float val = spectrumNorm.at(i);
                saturations.push_back(val * perBandSaturationAdjust.get() );
                saturationsIndexes.push_back( ofMap(i, fMin, fMax, cMin, cMax) );
            }
            
            break;
        case 1:
            //saturations.resize( melBands.size() );
            //saturationsIndexes.resize( melBands.size() );
            for (int i = mMin; i < mMax; i++) {
                float val = ofMap(melBands[i], DB_MIN, DB_MAX, 0.0, 1.0, true);
                saturations.push_back(val * perBandSaturationAdjust.get() );
                saturationsIndexes.push_back( ofMap(i, mMin, mMax, cMin, cMax) );
            }
            break;
        case 2:
            //saturations.resize( mfcc.size() );
            //saturationsIndexes.resize( mfcc.size() );
            for (int i = 0; i < mfcc.size(); i++) {
                float val = ofMap(mfcc[i], 0, MFCC_MAX_ESTIMATED_VALUE, 0.0, 1.0, true);
                saturations.push_back(val * perBandSaturationAdjust.get() );
                saturationsIndexes.push_back( ofMap(i, 0, mfcc.size(), cMin, cMax) );
            }
            break;
        case 3:
            //saturations.resize( hpcp.size() );
            //saturationsIndexes.resize( hpcp.size() );
            for (int i = 0; i < hpcp.size(); i++) {
                saturations.push_back(hpcp.at(i) * perBandSaturationAdjust.get() );
                saturationsIndexes.push_back( ofMap(i, 0, hpcp.size(), cMin, cMax) );
            }
            break;
        case 4:
            //saturations.resize( tristimulus.size() );
            //saturationsIndexes.resize( tristimulus.size() );
            for (int i = 0; i < tristimulus.size(); i++) {
                saturations.push_back(tristimulus.at(i) * perBandSaturationAdjust.get() );
                saturationsIndexes.push_back( ofMap(i, 0, tristimulus.size(), cMin, cMax) );
            }
            break;
            //        case 5:
            //            //saturations.resize( multiPitches.size() );
            //            //saturationsIndexes.resize( multiPitches.size() );
            //            for (int i = 0; i < multiPitchesSaliences.size(); i++) {
            //                saturations.push_back(multiPitchesSaliences.at(i) * perBandSaturationAdjust.get() );
            //                saturationsIndexes.push_back( ofMap(i, 0, multiPitchesSaliences.size(), cMin, cMax) );
            //            }
            //            break;
            
            //        case 6:
            //            for (int i = fMin; i < fMax; i++) {
            //                saturations.push_back(perBandSaturationAdjust.get() );
            //                saturationsIndexes.push_back( ofMap(i, fMin, fMax, cMin, cMax) );
            //            }
            //            break;
            
        case 5 :
            for (int i = fMin; i < fMax; i++) {
                saturations.push_back(0.0f );
                saturationsIndexes.push_back( ofMap(i, fMin, fMax, cMin, cMax) );
            }
            break;
            
            
    }
    
    
    s.set_points(saturationsIndexes, saturations);
    
    
    
    /**----------ALPHAS-------------*/
    
    tk::spline a;
    
    switch ( perBandAlpha.get() ) {
        case 0:
            //alphas.resize( fMax-fMin + 1);
            //alphasIndexes.resize( fMax-fMin + 1);
            for (int i = fMin; i < fMax; i++) {
                float val = spectrumNorm.at(i);
                alphas.push_back(val * perBandAlphaAdjust.get() );
                alphasIndexes.push_back( ofMap(i, fMin, fMax, cMin, cMax) );
            }
            
            break;
        case 1:
            //alphas.resize( melBands.size() );
            //alphasIndexes.resize( melBands.size() );
            for (int i = mMin; i < mMax; i++) {
                float val = ofMap(melBands[i], DB_MIN, DB_MAX, 0.0, 1.0, true);
                alphas.push_back(val * perBandAlphaAdjust.get() );
                alphasIndexes.push_back( ofMap(i, mMin, mMax, cMin, cMax) );
            }
            break;
        case 2:
            //alphas.resize( mfcc.size() );
            //alphasIndexes.resize( mfcc.size() );
            for (int i = 0; i < mfcc.size(); i++) {
                float val = ofMap(mfcc[i], 0, MFCC_MAX_ESTIMATED_VALUE, 0.0, 1.0, true);
                alphas.push_back(val * perBandAlphaAdjust.get() );
                alphasIndexes.push_back( ofMap(i, 0, mfcc.size(), cMin, cMax) );
            }
            break;
        case 3:
            //alphas.resize( hpcp.size() );
            //alphasIndexes.resize( hpcp.size() );
            for (int i = 0; i < hpcp.size(); i++) {
                alphas.push_back(hpcp.at(i) * perBandAlphaAdjust.get() );
                alphasIndexes.push_back( ofMap(i, 0, hpcp.size(), cMin, cMax) );
            }
            break;
        case 4:
            //alphas.resize( tristimulus.size() );
            //alphasIndexes.resize( tristimulus.size() );
            for (int i = 0; i < tristimulus.size(); i++) {
                alphas.push_back(tristimulus.at(i) * perBandAlphaAdjust.get() );
                alphasIndexes.push_back( ofMap(i, 0, tristimulus.size(), cMin, cMax) );
            }
            break;
            //        case 5:
            //            //alphas.resize( multiPitches.size() );
            //            //alphasIndexes.resize( multiPitches.size() );
            //            for (int i = 0; i < multiPitchesSaliences.size(); i++) {
            //                alphas.push_back(multiPitchesSaliences.at(i) * perBandAlphaAdjust.get() );
            //                alphasIndexes.push_back( ofMap(i, 0, multiPitchesSaliences.size(), cMin, cMax) );
            //            }
            //            break;
            
            //        case 6:
            //            for (int i = fMin; i < fMax; i++) {
            //                alphas.push_back(perBandAlphaAdjust.get() );
            //                alphasIndexes.push_back( ofMap(i, fMin, fMax, cMin, cMax) );
            //            }
            //            break;
            
        case 5 :
            for (int i = fMin; i < fMax; i++) {
                alphas.push_back(0.0f );
                alphasIndexes.push_back( ofMap(i, fMin, fMax, cMin, cMax) );
            }
            break;
    }
    
    
    a.set_points(alphasIndexes, alphas);
    
    
    /*----------GRADIENT-------------*/
    
    colorPoints.clear();
    //colorPoints.resize(cMax-cMin);
    
    for (int i = cMin; i < cMax; i++) {
        
        int numPoints = (int)( 20*(p(i)+p2(i)) );
        float hue =    lookupHues.at(i) +  (  (float)r(i) + gRot + refRotation.get()  )* 255.0f;
        float brightness = (   (float)b(i)  + gBri  + refBrightness.get()   )* 255.0f;
        float saturation = (   (float)s(i)  + gSat  + refSaturation.get()   )* 255.0f;
        float alpha = (   (float)a(i)  + gAlpha  + refAlpha.get()   )* 255.0f;
        
        hue = ofClamp(hue, 0.0f, 255.0f);
        brightness = ofClamp(brightness, 0.0f, 255.0f);
        saturation = ofClamp(saturation, 0.0f, 255.0f);
        alpha = ofClamp(alpha, 0.0f, 255.0f);
        
        for (int j = 0; j < numPoints; j++) {
            colorPoints.push_back(ofVec4f(hue, saturation, brightness, alpha) );
        }
        
    }
    
    grad.allocate(1, colorPoints.size(), OF_PIXELS_RGBA);
    
    for (int i = 0; i < colorPoints.size(); i++) {
        ofColor c;
        c.setHsb(colorPoints.at(i).x, colorPoints.at(i).y, colorPoints.at(i).z, colorPoints.at(i).w);
        grad.setColor(0, i, c);
        
    }
    
    grad.resize(2, 1080, OF_INTERPOLATE_NEAREST_NEIGHBOR);
    
    
    return grad;
    
}



/*---------------------------------------------------------------*/
//STEREO//

ofPixels ofApp::doGradientStereo(int fMin, int fMax, int cMin, int cMax, int mMin, int mMax) {
    
    float gRot, gBri, gSat, gAlpha;
    
    vector <double> presences, presences2, rotations, brightnesses, saturations, alphas;
    vector <double> presencesb, presences2b, rotationsb, brightnessesb, saturationsb, alphasb;
    
    vector <double> presencesIndexes, presences2Indexes, rotationsIndexes, brightnessesIndexes, saturationsIndexes, alphasIndexes;
    
    
    
    
    
    /*-----------------------AUDIOVIDEO-----------------------*/
    
    switch (avBModOption) {
        case 0 :
            avBMod = power + power2;
            break;
        case 1 :
            avBMod = pitchFreqNorm + pitchFreqNorm2;
            break;
        case 2 :
            avBMod = pitchSalience + pitchSalience2;
            break;
        case 3 :
            avBMod = hfcNorm + hfcNorm2;
            break;
        case 4 :
            avBMod = specCompNorm + specCompNorm2;
            break;
        case 5 :
            avBMod = centroidNorm + centroidNorm2;
            break;
        case 6 :
            avBMod = dissonance + dissonance2;
            break;
        case 7 :
            avBMod = rollOffNorm + rollOffNorm2;
            break;
        case 8 :
            avBMod = oddToEvenNorm + oddToEvenNorm2;
            break;
        case 9 :
            avBMod = strongPeakNorm + strongPeakNorm2;
            break;
        case 10 :
            avBMod = strongDecayNorm + strongDecayNorm2;
            break;
        case 11 :
            avBMod = 0.0f;
            break;
            
    }
    
    avBMod = avBMod/2*avBModIntensity + audioVideoBrightness;
    
    
    
    switch (avCModOption) {
        case 0 :
            avCMod = power + power2;
            break;
        case 1 :
            avCMod = pitchFreqNorm + pitchFreqNorm2;
            break;
        case 2 :
            avCMod = pitchSalience + pitchSalience2;
            break;
        case 3 :
            avCMod = hfcNorm + hfcNorm2;
            break;
        case 4 :
            avCMod = specCompNorm + specCompNorm2;
            break;
        case 5 :
            avCMod = centroidNorm + centroidNorm2;
            break;
        case 6 :
            avCMod = dissonance + dissonance2;
            break;
        case 7 :
            avCMod = rollOffNorm + rollOffNorm2;
            break;
        case 8 :
            avCMod = oddToEvenNorm + oddToEvenNorm2;
            break;
        case 9 :
            avCMod = strongPeakNorm + strongPeakNorm2;
            break;
        case 10 :
            avCMod = strongDecayNorm + strongDecayNorm2;
            break;
        case 11 :
            avCMod = 0.0f;
            break;
            
    }
    
    avCMod = avCMod/2*avCModIntensity + audioVideoContrast;
    
    
    
    switch (avSModOption) {
        case 0 :
            avSMod = power + power2;
            break;
        case 1 :
            avSMod = pitchFreqNorm + pitchFreqNorm2;
            break;
        case 2 :
            avSMod = pitchSalience + pitchSalience2;
            break;
        case 3 :
            avSMod = hfcNorm + hfcNorm2;
            break;
        case 4 :
            avSMod = specCompNorm + specCompNorm2;
            break;
        case 5 :
            avSMod = centroidNorm + centroidNorm2;
            break;
        case 6 :
            avSMod = dissonance + dissonance2;
            break;
        case 7 :
            avSMod = rollOffNorm + rollOffNorm2;
            break;
        case 8 :
            avSMod = oddToEvenNorm + oddToEvenNorm2;
            break;
        case 9 :
            avSMod = strongPeakNorm + strongPeakNorm2;
            break;
        case 10 :
            avSMod = strongDecayNorm + strongDecayNorm2;
            break;
        case 11 :
            avSMod = 0.0f;
            break;
    }
    
    avSMod = avSMod/2*avSModIntensity + audioVideoSaturation;
    
    
    
    switch (avAModOption) {
        case 0 :
            avAMod = power + power2;
            break;
        case 1 :
            avAMod = pitchFreqNorm + pitchFreqNorm2;
            break;
        case 2 :
            avAMod = pitchSalience + pitchSalience2;
            break;
        case 3 :
            avAMod = hfcNorm + hfcNorm2;
            break;
        case 4 :
            avAMod = specCompNorm + specCompNorm2;
            break;
        case 5 :
            avAMod = centroidNorm + centroidNorm2;
            break;
        case 6 :
            avAMod = dissonance + dissonance2;
            break;
        case 7 :
            avAMod = rollOffNorm + rollOffNorm2;
            break;
        case 8 :
            avAMod = oddToEvenNorm + oddToEvenNorm2;
            break;
        case 9 :
            avAMod = strongPeakNorm + strongPeakNorm2;
            break;
        case 10 :
            avAMod = strongDecayNorm + strongDecayNorm2;
            break;
            case 11 :
            avAMod = 0.0f;
            break;
            
    }
    
    avAMod = avAMod/2*avAModIntensity + audioVideoAlpha;
    
    
    
    /*----------ROTATION-------------*/
    
    switch (globalRotation.get() ) {
        case 0 :
        gRot = power + power2;
            break;
        case 1 :
            gRot = pitchFreqNorm + pitchFreqNorm2;
            break;
        case 2 :
            gRot = pitchSalience + pitchSalience2;
            break;
        case 3 :
            gRot = hfcNorm + hfcNorm2;
            break;
        case 4 :
            gRot = specCompNorm + specCompNorm2;
            break;
        case 5 :
            gRot = centroidNorm + centroidNorm2;
            break;
        case 6 :
            gRot = dissonance + dissonance2;
            break;
        case 7 :
            gRot = rollOffNorm + rollOffNorm2;
            break;
        case 8 :
            gRot = oddToEvenNorm + oddToEvenNorm2;
            break;
        case 9 :
            gRot = strongPeakNorm + strongPeakNorm2;
            break;
        case 10 :
            gRot = strongDecayNorm + strongDecayNorm2;
            break;
        case 11 :
            gRot = 0.0f;
            break;
    }
    
    gRot = gRot/2*globalRotationAdjust.get();
    
    
    /*----------BRIGHTNESS-------------*/
    
    switch (globalBrightness.get() ) {
        case 0 :
        gBri = power + power2;
            break;
        case 1 :
            gBri = pitchFreqNorm + pitchFreqNorm2;
            break;
        case 2 :
            gBri = pitchSalience + pitchSalience2;
            break;
        case 3 :
            gBri = hfcNorm + hfcNorm2;
            break;
        case 4 :
            gBri = specCompNorm + specCompNorm2;
            break;
        case 5 :
            gBri = centroidNorm + centroidNorm2;
            break;
        case 6 :
            gBri = dissonance + dissonance2;
            break;
        case 7 :
            gBri = rollOffNorm + rollOffNorm2;
            break;
        case 8 :
            gBri = oddToEvenNorm + oddToEvenNorm2;
            break;
        case 9 :
            gBri = strongPeakNorm + strongPeakNorm2;
            break;
        case 10 :
            gBri = strongDecayNorm + strongDecayNorm2;
            break;
        case 11 :
            gBri = 0.0f;
            break;
    }
    
    gBri = gBri/2*globalBrightnessAdjust.get();
    
    /*----------SATURATION-------------*/
    
    switch (globalSaturation.get() ) {
        case 0 :
        gSat = power + power2;
            break;
        case 1 :
            gSat = pitchFreqNorm + pitchFreqNorm2;
            break;
        case 2 :
            gSat = pitchSalience + pitchSalience2;
            break;
        case 3 :
            gSat = hfcNorm + hfcNorm2;
            break;
        case 4 :
            gSat = specCompNorm + specCompNorm2;
            break;
        case 5 :
            gSat = centroidNorm + centroidNorm2;
            break;
        case 6 :
            gSat = dissonance + dissonance2;
            break;
        case 7 :
            gSat = rollOffNorm + rollOffNorm2;
            break;
        case 8 :
            gSat = oddToEvenNorm + oddToEvenNorm2;
            break;
        case 9 :
            gSat = strongPeakNorm + strongPeakNorm2;
            break;
        case 10 :
            gSat = strongDecayNorm + strongDecayNorm2;
            break;
        case 11 :
            gSat = 0.0f;
            break;
    }
    
    gSat = gSat/2*globalSaturationAdjust.get();
    
    /*----------ALPHA-------------*/
    
    switch (globalAlpha.get() ) {
        case 0 :
        gAlpha = power + power2;
            break;
        case 1 :
            gAlpha = pitchFreqNorm + pitchFreqNorm2;
            break;
        case 2 :
            gAlpha = pitchSalience + pitchSalience2;
            break;
        case 3 :
            gAlpha = hfcNorm + hfcNorm2;
            break;
        case 4 :
            gAlpha = specCompNorm + specCompNorm2;
            break;
        case 5 :
            gAlpha = centroidNorm + centroidNorm2;
            break;
        case 6 :
            gAlpha = dissonance + dissonance2;
            break;
        case 7 :
            gAlpha = rollOffNorm + rollOffNorm2;
            break;
        case 8 :
            gAlpha = oddToEvenNorm + oddToEvenNorm2;
            break;
        case 9 :
            gAlpha = strongPeakNorm + strongPeakNorm2;
            break;
        case 10 :
            gAlpha = strongDecayNorm + strongDecayNorm2;
            break;
        case 11 :
            gAlpha = 0.0f;
            break;
    }
    
    gAlpha = gAlpha/2*globalAlphaAdjust.get();
    
    
    /*----------VECTOR-------------*//*----------DATA-------------*/
    
    if (cMin > cMax){
        int c = cMin;
        cMin = cMax;
        cMax = c;
        }
    
    if ( (cMax-cMin) <=2) {cMax = cMin+3;}
    
    if (fMin > fMax){
        int f = fMin;
        fMin = fMax;
        fMax = f;
    }
    if ( (fMax-fMin) <=2) {fMax = fMin+3;}
    
    if (mMin > mMax){
        int m = mMin;
        mMin = mMax;
        mMax = m;
    }
    if ( (mMax-mMin) <=2) {mMax = mMin+3;}
    
    /*----------PRESENCES-------------*/
    
    tk::spline p, pb;
    
    switch ( perBandPresence.get() ) {
        case 0:
            //presences.resize( (fMax-fMin + 1) );
            //presencesIndexes.resize( (fMax-fMin + 1) );
            for (int i = fMin; i < fMax; i++) {
                float val = spectrumNorm.at(i);
                float val2 = spectrumNorm2.at(i);
                presences.push_back( val * perBandPresenceAdjust.get() );
                presencesb.push_back( val2 * perBandPresenceAdjust.get() );
                presencesIndexes.push_back( ofMap( i, fMin, fMax, cMin, cMax) );
            }
            
            break;
        case 1:
            //presences.resize( melBands.size() );
            //presencesIndexes.resize( melBands.size() );
            for (int i = mMin; i < mMax; i++) {
                float val = ofMap(melBands[i], DB_MIN, DB_MAX, 0.0, 1.0, true);
                float val2 = ofMap(melBands2[i], DB_MIN, DB_MAX, 0.0, 1.0, true);
                presences.push_back(val * perBandPresenceAdjust.get() );
                presencesb.push_back( val2 * perBandPresenceAdjust.get() );
                presencesIndexes.push_back( ofMap(i, mMin, mMax, cMin, cMax) );
            }
            break;
        case 2:
            //presences.resize( mfcc.size() );
            //presencesIndexes.resize( mfcc.size() );
            for (int i = 0; i < mfcc.size(); i++) {
                float val = ofMap(mfcc[i], 0, MFCC_MAX_ESTIMATED_VALUE, 0.0, 1.0, true);
                float val2 = ofMap(mfcc2[i], 0, MFCC_MAX_ESTIMATED_VALUE, 0.0, 1.0, true);
                presences.push_back(val * perBandPresenceAdjust.get() );
                presencesb.push_back( val2 * perBandPresenceAdjust.get() );
                presencesIndexes.push_back( ofMap(i, 0, mfcc.size(), cMin, cMax) );
            }
            break;
        case 3:
            //presences.resize( hpcp.size() );
            //presencesIndexes.resize( hpcp.size() );
            for (int i = 0; i < hpcp.size(); i++) {
                presences.push_back(hpcp.at(i) * perBandPresenceAdjust.get() );
                presencesb.push_back(hpcp2.at(i) * perBandPresenceAdjust.get() );
                presencesIndexes.push_back( ofMap(i, 0, hpcp.size(), cMin, cMax) );
            }
            break;
        case 4:
            //presences.resize( tristimulus.size() );
            //presencesIndexes.resize( tristimulus.size() );
            for (int i = 0; i < tristimulus.size(); i++) {
                presences.push_back(tristimulus.at(i) * perBandPresenceAdjust.get() );
                presencesb.push_back(tristimulus2.at(i) * perBandPresenceAdjust.get() );
                presencesIndexes.push_back( ofMap(i, 0, tristimulus.size(), cMin, cMax) );
            }
            break;
            //        case 5:
            //            //presences.resize( multiPitches.size() );
            //            //presencesIndexes.resize( multiPitches.size() );
            //            for (int i = 0; i < multiPitchesSaliences.size(); i++) {
            //                presences.push_back(multiPitchesSaliences.at(i) * perBandPresenceAdjust.get() );
            //                presencesIndexes.push_back( ofMap(i, 0, multiPitches.size(), cMin, cMax) );
            //            }
            //            break;
            //        case 6:
            //            //presences.resize( multiPitches.size() );
            //            //presencesIndexes.resize( multiPitches.size() );
            //            for (int i = fMin; i < fMax; i++) {
            //                presences.push_back(perBandPresenceAdjust.get() );
            //                presencesIndexes.push_back( ofMap(i, fMin, fMax, cMin, cMax) );
            //            }
            //            break;
            
        case 5 :
            for (int i = fMin; i < fMax; i++) {
                presences.push_back(0.0f );
                presencesb.push_back(0.0f );
                presencesIndexes.push_back( ofMap(i, fMin, fMax, cMin, cMax) );
            }
            break;
    }
    
    p.set_points(presencesIndexes, presences);
    pb.set_points(presencesIndexes, presencesb);
    
    
    /*----------PRESENCES2-------------*/
    
    tk::spline p2, p2b;
    
    switch ( perBandPresence2.get() ) {
        case 0:
        //presences.resize( (fMax-fMin + 1) );
            //presencesIndexes.resize( (fMax-fMin + 1) );
            for (int i = fMin; i < fMax; i++) {
                float val = spectrumNorm.at(i);
                float val2 = spectrumNorm2.at(i);
                presences2.push_back( val * (1-perBandPresenceAdjust.get()) );
                presences2b.push_back( val2 * (1-perBandPresenceAdjust.get()) );
                presences2Indexes.push_back( ofMap( i, fMin, fMax, cMin, cMax) );
            }
            
            break;
        case 1:
            //presences.resize( melBands.size() );
            //presencesIndexes.resize( melBands.size() );
            for (int i = mMin; i < mMax; i++) {
                float val = ofMap(melBands[i], DB_MIN, DB_MAX, 0.0, 1.0, true);
                float val2 = ofMap(melBands2[i], DB_MIN, DB_MAX, 0.0, 1.0, true);
                presences2.push_back(val * (1-perBandPresenceAdjust.get()) );
                presences2b.push_back(val2 * (1-perBandPresenceAdjust.get()) );
                presences2Indexes.push_back( ofMap(i, mMin, mMax, cMin, cMax) );
            }
            break;
        case 2:
            //presences.resize( mfcc.size() );
            //presencesIndexes.resize( mfcc.size() );
            for (int i = 0; i < mfcc.size(); i++) {
                float val = ofMap(mfcc[i], 0, MFCC_MAX_ESTIMATED_VALUE, 0.0, 1.0, true);
                float val2 = ofMap(mfcc2[i], 0, MFCC_MAX_ESTIMATED_VALUE, 0.0, 1.0, true);
                presences2.push_back(val * (1-perBandPresenceAdjust.get()) );
                presences2b.push_back(val2 * (1-perBandPresenceAdjust.get()) );
                presences2Indexes.push_back( ofMap(i, 0, mfcc.size(), cMin, cMax) );
            }
            break;
        case 3:
            //presences.resize( hpcp.size() );
            //presencesIndexes.resize( hpcp.size() );
            for (int i = 0; i < hpcp.size(); i++) {
                presences2.push_back(hpcp.at(i) * (1-perBandPresenceAdjust.get()) );
                presences2b.push_back(hpcp2.at(i) * (1-perBandPresenceAdjust.get()) );
                presences2Indexes.push_back( ofMap(i, 0, hpcp.size(), cMin, cMax) );
            }
            break;
        case 4:
            //presences.resize( tristimulus.size() );
            //presencesIndexes.resize( tristimulus.size() );
            for (int i = 0; i < tristimulus.size(); i++) {
                presences2.push_back(tristimulus.at(i) * (1-perBandPresenceAdjust.get()) );
                presences2b.push_back(tristimulus2.at(i) * (1-perBandPresenceAdjust.get()) );
                presences2Indexes.push_back( ofMap(i, 0, tristimulus.size(), cMin, cMax) );
            }
            break;
            //        case 5:
            //            //presences.resize( multiPitches.size() );
            //            //presencesIndexes.resize( multiPitches.size() );
            //            for (int i = 0; i < multiPitchesSaliences.size(); i++) {
            //                presences2.push_back(multiPitchesSaliences.at(i) * (1-perBandPresenceAdjust.get()) );
            //                presences2Indexes.push_back( ofMap(i, 0, multiPitches.size(), cMin, cMax) );
            //            }
            //            break;
            //        case 6:
            //            //presences.resize( multiPitches.size() );
            //            //presencesIndexes.resize( multiPitches.size() );
            //            for (int i = fMin; i < fMax; i++) {
            //                presences2.push_back( 1-perBandPresenceAdjust.get() );
            //                presences2Indexes.push_back( ofMap(i, fMin, fMax, cMin, cMax) );
            //            }
            //            break;
            
        case 5 :
            for (int i = fMin; i < fMax; i++) {
                presences2.push_back(0.0f );
                presences2b.push_back(0.0f );
                presences2Indexes.push_back( ofMap(i, fMin, fMax, cMin, cMax) );
            }
            break;
    }
    
    p2.set_points(presences2Indexes, presences2);
    p2b.set_points(presences2Indexes, presences2b);
    
    
    
    /*----------ROTATIONS-------------*/
    
    tk::spline r, rb;
    
    switch ( perBandRotation.get() ) {
        case 0:
            //rotations.resize( fMax - fMin + 1);
            //rotationsIndexes.resize( fMax - fMin + 1);
            for (int i = fMin; i < fMax; i++) {
                float val = spectrumNorm.at(i);
                float val2 = spectrumNorm2.at(i);
                rotations.push_back(val * perBandRotationAdjust.get() );
                rotationsb.push_back(val2 * perBandRotationAdjust.get() );
                rotationsIndexes.push_back( ofMap(i, fMin, fMax, cMin, cMax) );
            }
            
            break;
        case 1:
            //rotations.resize( melBands.size() );
            //rotationsIndexes.resize( melBands.size() );
            for (int i = mMin; i < mMax; i++) {
                float val = ofMap(melBands[i], DB_MIN, DB_MAX, 0.0, 1.0, true);
                float val2 = ofMap(melBands2[i], DB_MIN, DB_MAX, 0.0, 1.0, true);
                rotations.push_back(val * perBandRotationAdjust.get() );
                rotationsb.push_back(val2 * perBandRotationAdjust.get() );
                rotationsIndexes.push_back( ofMap(i, mMin,mMax, cMin, cMax) );
            }
            break;
        case 2:
            //rotations.resize( mfcc.size() );
            //rotationsIndexes.resize( mfcc.size() );
            for (int i = 0; i < mfcc.size(); i++) {
                float val = ofMap(mfcc[i], 0, MFCC_MAX_ESTIMATED_VALUE, 0.0, 1.0, true);
                float val2 = ofMap(mfcc2[i], 0, MFCC_MAX_ESTIMATED_VALUE, 0.0, 1.0, true);
                rotations.push_back(val * perBandRotationAdjust.get() );
                rotationsb.push_back(val2 * perBandRotationAdjust.get() );
                rotationsIndexes.push_back( ofMap(i, 0, mfcc.size(), cMin, cMax) );
            }
            break;
        case 3:
            //rotations.resize( hpcp.size() );
            //rotationsIndexes.resize( hpcp.size() );
            for (int i = 0; i < hpcp.size(); i++) {
                rotations.push_back(hpcp.at(i) * perBandRotationAdjust.get() );
                rotationsb.push_back(hpcp2.at(i) * perBandRotationAdjust.get() );
                rotationsIndexes.push_back( ofMap(i, 0, hpcp.size(), cMin, cMax) );
            }
            break;
        case 4:
            //rotations.resize( tristimulus.size() );
            //rotationsIndexes.resize( tristimulus.size() );
            for (int i = 0; i < tristimulus.size(); i++) {
                rotations.push_back(tristimulus.at(i) * perBandRotationAdjust.get() );
                rotationsb.push_back(tristimulus2.at(i) * perBandRotationAdjust.get() );
                rotationsIndexes.push_back( ofMap(i, 0, tristimulus.size(), cMin, cMax) );
            }
            break;
            
            //        case 5:
            //            //rotations.resize( multiPitches.size() );
            //            //rotationsIndexes.resize( multiPitches.size() );
            //            for (int i = 0; i < multiPitchesSaliences.size(); i++) {
            //                rotations.push_back(multiPitchesSaliences.at(i) * perBandRotationAdjust.get() );
            //                rotationsIndexes.push_back( ofMap(i, 0, multiPitchesSaliences.size(), cMin, cMax) );
            //            }
            //            break;
            //
            //        case 6:
            //            for (int i = fMin; i < fMax; i++) {
            //                rotations.push_back(perBandRotationAdjust.get() );
            //                rotationsIndexes.push_back( ofMap(i, fMin, fMax, cMin, cMax) );
            //            }
            //            break;
            
        case 5 :
            for (int i = fMin; i < fMax; i++) {
                rotations.push_back(0.0f );
                rotationsb.push_back(0.0f );
                rotationsIndexes.push_back( ofMap(i, fMin, fMax, cMin, cMax) );
            }
            break;
    }
    
    
    r.set_points(rotationsIndexes, rotations);
    rb.set_points(rotationsIndexes, rotationsb);
    
    
    /**----------BRIGHTNESSES-------------*/
    
    tk::spline b, bb;
    
    switch ( perBandBrightness.get() ) {
        case 0:
            //brightnesses.resize( fMax - fMin + 1);
            //brightnessesIndexes.resize( fMax - fMin + 1);
            for (int i = fMin; i < fMax; i++) {
                float val = spectrumNorm.at(i);
                float val2 = spectrumNorm2.at(i);
                brightnesses.push_back(val * perBandBrightnessAdjust.get() );
                brightnessesb.push_back(val2 * perBandBrightnessAdjust.get() );
                brightnessesIndexes.push_back( ofMap(i, fMin, fMax, cMin, cMax) );
            }
            
            break;
        case 1:
            //brightnesses.resize( melBands.size() );
            //brightnessesIndexes.resize( melBands.size() );
            for (int i = mMin; i < mMax; i++) {
                float val = ofMap(melBands[i], DB_MIN, DB_MAX, 0.0, 1.0, true);
                float val2 = ofMap(melBands2[i], DB_MIN, DB_MAX, 0.0, 1.0, true);
                brightnesses.push_back(val * perBandBrightnessAdjust.get() );
                brightnessesb.push_back(val2 * perBandBrightnessAdjust.get() );
                brightnessesIndexes.push_back( ofMap(i, mMin, mMax, cMin, cMax) );
            }
            break;
        case 2:
            //brightnesses.resize( mfcc.size() );
            //brightnessesIndexes.resize( mfcc.size() );
            for (int i = 0; i < mfcc.size(); i++) {
                float val = ofMap(mfcc[i], 0, MFCC_MAX_ESTIMATED_VALUE, 0.0, 1.0, true);
                float val2 = ofMap(mfcc2[i], 0, MFCC_MAX_ESTIMATED_VALUE, 0.0, 1.0, true);
                brightnesses.push_back(val * perBandBrightnessAdjust.get() );
                brightnessesb.push_back(val2 * perBandBrightnessAdjust.get() );
                brightnessesIndexes.push_back( ofMap(i, 0, mfcc.size(), cMin, cMax) );
            }
            break;
        case 3:
            //brightnesses.resize( hpcp.size() );
            //brightnessesIndexes.resize( hpcp.size() );
            for (int i = 0; i < hpcp.size(); i++) {
                brightnesses.push_back(hpcp.at(i) * perBandBrightnessAdjust.get() );
                brightnessesb.push_back(hpcp2.at(i) * perBandBrightnessAdjust.get() );
                brightnessesIndexes.push_back( ofMap(i, 0, hpcp.size(), cMin, cMax) );
            }
            break;
        case 4:
            //brightnesses.resize( tristimulus.size() );
            //brightnessesIndexes.resize( tristimulus.size() );
            for (int i = 0; i < tristimulus.size(); i++) {
                brightnesses.push_back(tristimulus.at(i) * perBandBrightnessAdjust.get() );
                brightnessesb.push_back(tristimulus2.at(i) * perBandBrightnessAdjust.get() );
                brightnessesIndexes.push_back( ofMap(i, 0, tristimulus.size(), cMin, cMax) );
            }
            break;
            //        case 5:
            //            //brightnesses.resize( multiPitches.size() );
            //            //brightnessesIndexes.resize( multiPitches.size() );
            //            for (int i = 0; i < multiPitchesSaliences.size(); i++) {
            //                brightnesses.push_back(multiPitchesSaliences.at(i) * perBandBrightnessAdjust.get() );
            //                brightnessesIndexes.push_back( ofMap(i, 0, multiPitchesSaliences.size(), cMin, cMax) );
            //            }
            //            break;
            
            //        case 6:
            //            for (int i = fMin; i < fMax; i++) {
            //                brightnesses.push_back(perBandBrightnessAdjust.get() );
            //                brightnessesIndexes.push_back( ofMap(i, fMin, fMax, cMin, cMax) );
            //            }
            //            break;
            
        case 5 :
            for (int i = fMin; i < fMax; i++) {
                brightnesses.push_back(0.0f );
                brightnessesb.push_back(0.0f );
                brightnessesIndexes.push_back( ofMap(i, fMin, fMax, cMin, cMax) );
            }
            break;
            
    }
    
    
    b.set_points(brightnessesIndexes, brightnesses);
    bb.set_points(brightnessesIndexes, brightnessesb);
    
    
    /**----------SATURATIONS-------------*/
    
    
    tk::spline s, sb;
    
    switch ( perBandSaturation.get() ) {
        case 0:
            //saturations.resize( fMax-fMin + 1);
            //saturationsIndexes.resize( fMax-fMin + 1);
            for (int i = fMin; i < fMax; i++) {
                float val = spectrumNorm.at(i);
                float val2 = spectrumNorm2.at(i);
                saturations.push_back(val * perBandSaturationAdjust.get() );
                saturationsb.push_back(val2 * perBandSaturationAdjust.get() );
                saturationsIndexes.push_back( ofMap(i, fMin, fMax, cMin, cMax) );
            }
            
            break;
        case 1:
            //saturations.resize( melBands.size() );
            //saturationsIndexes.resize( melBands.size() );
            for (int i = mMin; i < mMax; i++) {
                float val = ofMap(melBands[i], DB_MIN, DB_MAX, 0.0, 1.0, true);
                float val2 = ofMap(melBands2[i], DB_MIN, DB_MAX, 0.0, 1.0, true);
                saturations.push_back(val * perBandSaturationAdjust.get() );
                saturationsb.push_back(val2 * perBandSaturationAdjust.get() );
                saturationsIndexes.push_back( ofMap(i, mMin, mMax, cMin, cMax) );
            }
            break;
        case 2:
            //saturations.resize( mfcc.size() );
            //saturationsIndexes.resize( mfcc.size() );
            for (int i = 0; i < mfcc.size(); i++) {
                float val = ofMap(mfcc[i], 0, MFCC_MAX_ESTIMATED_VALUE, 0.0, 1.0, true);
                float val2 = ofMap(mfcc2[i], 0, MFCC_MAX_ESTIMATED_VALUE, 0.0, 1.0, true);
                saturations.push_back(val * perBandSaturationAdjust.get() );
                saturationsb.push_back(val2 * perBandSaturationAdjust.get() );
                saturationsIndexes.push_back( ofMap(i, 0, mfcc.size(), cMin, cMax) );
            }
            break;
        case 3:
            //saturations.resize( hpcp.size() );
            //saturationsIndexes.resize( hpcp.size() );
            for (int i = 0; i < hpcp.size(); i++) {
                saturations.push_back(hpcp.at(i) * perBandSaturationAdjust.get() );
                saturationsb.push_back(hpcp2.at(i) * perBandSaturationAdjust.get() );
                saturationsIndexes.push_back( ofMap(i, 0, hpcp.size(), cMin, cMax) );
            }
            break;
        case 4:
            //saturations.resize( tristimulus.size() );
            //saturationsIndexes.resize( tristimulus.size() );
            for (int i = 0; i < tristimulus.size(); i++) {
                saturations.push_back(tristimulus.at(i) * perBandSaturationAdjust.get() );
                saturationsb.push_back(tristimulus2.at(i) * perBandSaturationAdjust.get() );
                saturationsIndexes.push_back( ofMap(i, 0, tristimulus.size(), cMin, cMax) );
            }
            break;
            //        case 5:
            //            //saturations.resize( multiPitches.size() );
            //            //saturationsIndexes.resize( multiPitches.size() );
            //            for (int i = 0; i < multiPitchesSaliences.size(); i++) {
            //                saturations.push_back(multiPitchesSaliences.at(i) * perBandSaturationAdjust.get() );
            //                saturationsIndexes.push_back( ofMap(i, 0, multiPitchesSaliences.size(), cMin, cMax) );
            //            }
            //            break;
            
            //        case 6:
            //            for (int i = fMin; i < fMax; i++) {
            //                saturations.push_back(perBandSaturationAdjust.get() );
            //                saturationsIndexes.push_back( ofMap(i, fMin, fMax, cMin, cMax) );
            //            }
            //            break;
            
        case 5 :
            for (int i = fMin; i < fMax; i++) {
                saturations.push_back(0.0f );
                saturationsb.push_back(0.0f );
                saturationsIndexes.push_back( ofMap(i, fMin, fMax, cMin, cMax) );
            }
            break;
            
            
    }
    
    
    s.set_points(saturationsIndexes, saturations);
    sb.set_points(saturationsIndexes, saturationsb);
    
    
    
    /**----------ALPHAS-------------*/
    
    tk::spline a, ab;
    
    switch ( perBandAlpha.get() ) {
        case 0:
            //alphas.resize( fMax-fMin + 1);
            //alphasIndexes.resize( fMax-fMin + 1);
            for (int i = fMin; i < fMax; i++) {
                float val = spectrumNorm.at(i);
                float val2 = spectrumNorm2.at(i);
                alphas.push_back(val * perBandAlphaAdjust.get() );
                alphasb.push_back(val2 * perBandAlphaAdjust.get() );
                alphasIndexes.push_back( ofMap(i, fMin, fMax, cMin, cMax) );
            }
            
            break;
        case 1:
            //alphas.resize( melBands.size() );
            //alphasIndexes.resize( melBands.size() );
            for (int i = mMin; i < mMax; i++) {
                float val = ofMap(melBands[i], DB_MIN, DB_MAX, 0.0, 1.0, true);
                float val2 = ofMap(melBands2[i], DB_MIN, DB_MAX, 0.0, 1.0, true);
                alphas.push_back(val * perBandAlphaAdjust.get() );
                alphasb.push_back(val2 * perBandAlphaAdjust.get() );
                alphasIndexes.push_back( ofMap(i, mMin, mMax, cMin, cMax) );
            }
            break;
        case 2:
            //alphas.resize( mfcc.size() );
            //alphasIndexes.resize( mfcc.size() );
            for (int i = 0; i < mfcc.size(); i++) {
                float val = ofMap(mfcc[i], 0, MFCC_MAX_ESTIMATED_VALUE, 0.0, 1.0, true);
                float val2 = ofMap(mfcc2[i], 0, MFCC_MAX_ESTIMATED_VALUE, 0.0, 1.0, true);
                alphas.push_back(val * perBandAlphaAdjust.get() );
                alphasb.push_back(val2 * perBandAlphaAdjust.get() );
                alphasIndexes.push_back( ofMap(i, 0, mfcc.size(), cMin, cMax) );
            }
            break;
        case 3:
            //alphas.resize( hpcp.size() );
            //alphasIndexes.resize( hpcp.size() );
            for (int i = 0; i < hpcp.size(); i++) {
                alphas.push_back(hpcp.at(i) * perBandAlphaAdjust.get() );
                alphasb.push_back(hpcp2.at(i) * perBandAlphaAdjust.get() );
                alphasIndexes.push_back( ofMap(i, 0, hpcp.size(), cMin, cMax) );
            }
            break;
        case 4:
            //alphas.resize( tristimulus.size() );
            //alphasIndexes.resize( tristimulus.size() );
            for (int i = 0; i < tristimulus.size(); i++) {
                alphas.push_back(tristimulus.at(i) * perBandAlphaAdjust.get() );
                alphasb.push_back(tristimulus2.at(i) * perBandAlphaAdjust.get() );
                alphasIndexes.push_back( ofMap(i, 0, tristimulus.size(), cMin, cMax) );
            }
            break;
            //        case 5:
            //            //alphas.resize( multiPitches.size() );
            //            //alphasIndexes.resize( multiPitches.size() );
            //            for (int i = 0; i < multiPitchesSaliences.size(); i++) {
            //                alphas.push_back(multiPitchesSaliences.at(i) * perBandAlphaAdjust.get() );
            //                alphasIndexes.push_back( ofMap(i, 0, multiPitchesSaliences.size(), cMin, cMax) );
            //            }
            //            break;
            
            //        case 6:
            //            for (int i = fMin; i < fMax; i++) {
            //                alphas.push_back(perBandAlphaAdjust.get() );
            //                alphasIndexes.push_back( ofMap(i, fMin, fMax, cMin, cMax) );
            //            }
            //            break;
            
        case 5 :
            for (int i = fMin; i < fMax; i++) {
                alphas.push_back(0.0f );
                alphasb.push_back(0.0f );
                alphasIndexes.push_back( ofMap(i, fMin, fMax, cMin, cMax) );
            }
            break;
    }
    
    
    a.set_points(alphasIndexes, alphas);
    ab.set_points(alphasIndexes, alphasb);
    
    
    /*----------GRADIENT-------------*/
    
    ofPixels grad;
    ofPixels grad2;
    
    colorPoints.clear();
    colorPoints2.clear();
    
    
    for (int i = cMin; i < cMax; i++) {
        
        int numPoints = (int)( 10*(p(i)+p2(i)) );
        float hue =    lookupHues.at(i) +  (  (float)r(i) + gRot + refRotation.get()  )* 255.0f;
        float brightness = (   (float)b(i)  + gBri  + refBrightness.get()   )* 255.0f;
        float saturation = (   (float)s(i)  + gSat  + refSaturation.get()   )* 255.0f;
        float alpha = (   (float)a(i)  + gAlpha  + refAlpha.get()   )* 255.0f;
        
        hue = ofClamp(hue, 0.0f, 255.0f);
        brightness = ofClamp(brightness, 0.0f, 255.0f);
        saturation = ofClamp(saturation, 0.0f, 255.0f);
        alpha = ofClamp(alpha, 0.0f, 255.0f);
        
        for (int j = 0; j < numPoints; j++) {
            colorPoints.push_back(ofVec4f(hue, saturation, brightness, alpha) );
        }
        
        
        
        int numPoints2 = (int)( 20*(pb(i)+p2b(i)) );
        float hue2 =    lookupHues.at(i) +  (  (float)rb(i) + gRot + refRotation.get()  )* 255.0f;
        float brightness2 = (   (float)bb(i)  + gBri  + refBrightness.get()   )* 255.0f;
        float saturation2 = (   (float)sb(i)  + gSat  + refSaturation.get()   )* 255.0f;
        float alpha2 = (   (float)ab(i)  + gAlpha  + refAlpha.get()   )* 255.0f;
        
        hue2 = ofClamp(hue2, 0.0f, 255.0f);
        brightness2 = ofClamp(brightness2, 0.0f, 255.0f);
        saturation2 = ofClamp(saturation2, 0.0f, 255.0f);
        alpha2 = ofClamp(alpha2, 0.0f, 255.0f);
        
        for (int j = 0; j < numPoints2; j++) {
            colorPoints2.push_back(ofVec4f(hue2, saturation2, brightness2, alpha2) );
        }
        
    }
    
    grad.allocate(2, colorPoints.size(), OF_PIXELS_RGBA);
    for (int i = 0; i < colorPoints.size(); i++) {
        ofColor c;
        c.setHsb(colorPoints.at(i).x, colorPoints.at(i).y, colorPoints.at(i).z, colorPoints.at(i).w);
        grad.setColor(0, i, c);
        
    }
    
    grad2.allocate(1, colorPoints2.size(), OF_PIXELS_RGBA);
    for (int i = 0; i < colorPoints2.size(); i++) {
        ofColor c;
        c.setHsb(colorPoints2.at(i).x, colorPoints2.at(i).y, colorPoints2.at(i).z, colorPoints2.at(i).w);
        grad2.setColor(0, i, c);
        
    }
    
    grad2.resize(1, 1080, OF_INTERPOLATE_NEAREST_NEIGHBOR);
    
    grad.resize(2, 1080, OF_INTERPOLATE_NEAREST_NEIGHBOR);
    
    grad2.pasteInto(grad, 1, 0);
    
    
    return grad;
    
}



/*---------------------------------------------------------------*/


int ofApp::iround(float v) {
    
    return (int)(floor(v+0.5f) );
}

/*---------------------------------------------------------------*/

float ofApp::getVideoMod( int modOption) {
    
    switch ( modOption) {
            
        case 0:
            return averageHue;

        case 1:
            return averageBrightness;

        case 2:
            return averageSaturation;

            
        case 3:
            return white;

        case 4:
           return neutral ;

        case 5:
            return black;
            
        case 6:
            return 0.0f;
        
        default:
            return 0.0f;
            
    }

}

float ofApp::getVideoDataSample( int option,  int index ){
    

    switch(option){
            
        case 0:
            return ofClamp( presences.at(index) , 0, 1 );
            
        case 1:
            return ofClamp( brightnesses.at(index) , 0, 1 );
            
        case 2:
            return ofClamp( saturations.at(index) , 0, 1 );
            
        case 3:
            return (ofClamp( posXs.at(index), -1, 1 )+1)/2;

        case 4:
            return (ofClamp( posYs.at(index) , -1, 1 )+1)/2;
            
        default :
            return 0.0f;
            
    }
    
}

void ofApp::updateIFFT(){
    
    
    iFFTamp = getVideoMod( iFFTampModOption )*iFFTampModIntensity + iFFTampRef;
    iFFTloHi = getVideoMod( iFFTloHiModOption )*iFFTloHiModIntensity + iFFTloHiRef;
    iFFThiShift = getVideoMod( iFFThiShiftModOption )*iFFThiShiftModIntensity + iFFThiShiftRef;
    iFFTwhiteMorph = getVideoMod( iFFTwhiteMorphModOption )*iFFTwhiteMorphModIntensity + iFFTwhiteMorphRef;
    iFFTconvoMix = getVideoMod( iFFTconvoMixModOption )*iFFTconvoMixModIntensity + iFFTconvoMixRef;

    
    
    
    for (int i= 0; i<nBandsAnalysis; i++) {
        
        if (i < videoStartF || i > videoEndF) {
            mags.at(i) = 0.0f;
            phases.at(i) = 0.0f;
            pan.at(i) = 0.5f;
            tilt.at(i) = 0.5f;
        }
        else {
            
            int cindex = iround( ofMap(i, videoStartF, videoEndF, videoStartC, videoEndC) );
            
            mags.at(i) =  getVideoDataSample( iFFTmagOption, cindex );
            phases.at(i) = getVideoDataSample( iFFTphaseOption, cindex );
            pan.at(i) = getVideoDataSample( 3, cindex );
            tilt.at(i) = getVideoDataSample( 4, cindex );
        }
    }
    
    
}



/*---------------------------------------------------------------*/

void ofApp::drawIFFT(float x, float y, float w, float h) {
    
    float mw = w/1024;
    float mh;

    mh = (3*h/4)/4;
    
    ofSetColor(audioColor);
    
    for (int i = 0; i < 1024; i++) {
        
        float p = mags.at(i)*mh;
        float p1 = phases.at(i)*mh;
        float p2 = pan.at(i)*mh -mh/2;
        float p3 = tilt.at(i)*mh -mh/2;

        ofDrawRectangle(x + mw*i, mh + y, mw, -p);
        ofDrawRectangle(x + mw*i, 2*mh + y, mw, -p1);
        ofDrawRectangle(x + mw*i, 3*mh-mh/2 + y, mw, -p2);
        ofDrawRectangle(x + mw*i, 4*mh-mh/2 + y, mw, -p3);

    }
    
    float lx1 = ofMap(videoStartF.get(), 0, 1023, 0, w);
    float lx2 = ofMap(videoEndF.get(), 0, 1023, 0, w);
    
    line(x + lx1, y, x + lx1, y + 3.5*h/4, 1, overAudioColor);
    line(x + lx2, y, x + lx2, y + 3.5*h/4, 1, overAudioColor);
    
    
    string f1 = ofToString( ofMap(videoStartF, 0, 1023, 20, 22050) )+ " Hz";
    string f2 = ofToString( ofMap(videoEndF, 0, 1023, 20, 22050) )+ " Hz";
    
    
    ofSetColor(frameColor);
    ofNoFill();
    ofDrawRectangle(x, y, w, 3*h/4);
    
    
    line(x + lx1, y, x + lx1, y + 3.2*h/4, 1, overAudioColor);
    line(x + lx2, y, x + lx2, y + 3.2*h/4, 1, overAudioColor);
    
    ofSetColor(audioFontColor);
    ofDrawBitmapString(f1, x + lx1-10, y + 25 + 3.3*h/4-10);
    ofDrawBitmapString(f2, x + lx2-10-50, y + 25 + 3.3*h/4);
    
    ofSetColor(fontColor);
    ofDrawBitmapString("iFFT Datas", x, y -5);
    
}

/*---------------------------------------------------------------*/

void ofApp::updateSynth() {
    
    for (int i = 0; i < 16; i++){
        int index = iround( ofMap(i, 0, 15, videoStartC, videoEndC) );
        scPresences.at(i) = presences.at(index);
        scBrightnesses.at(i) = brightnesses.at(index);
        scSaturations.at(i) = saturations.at(index);
        scXs.at(i) = posXs.at(index);
        scYs.at(i) = posYs.at(index);
        
    }
    
    
    
    float scMasterVolMod;
    
    switch (scMasterVolOption) {
        case 0:
            scMasterVolMod = averageHue;
            break;
        case 1:
            scMasterVolMod = averageBrightness;
            break;
        case 2:
            scMasterVolMod = averageSaturation;
            break;
        case 3:
            scMasterVolMod = white;
            break;
        case 4:
            scMasterVolMod = neutral;
            break;
        case 5:
            scMasterVolMod = black;
            break;
        case 6:
            scMasterVolMod = 0.0f;
            break;
            
    }
    scMasterVolume = scMasterVolMod*scMasterVolIntensity + scMasterVolRef;
    
    
    
    switch(scTriggerOption) {
        case 0:
            scTriggersBuf = scPresences;
            break;
        case 1:
            scTriggersBuf = scBrightnesses;
            break;
        case 2:
            scTriggersBuf = scSaturations;
            break;
        case 3:
            scTriggersBuf = scXs;
            break;
        case 4:
            scTriggersBuf = scYs;
            break;
        case 5:
            for (int i = 0; i < 16; i++) {
                scTriggersBuf.at(i) = 0.0f;
            }
            break;
    }
    
    for (int i = 0 ; i < 16; i++ ) {
        if (scTriggersBuf.at(i) >= scThresholds.at(i) ) scTriggers.at(i)=true; else scTriggers.at(i) = false;
    }
    
    
    
    switch(scVelocitiesOption) {
        case 0:
            scVelocities = scPresences;
            break;
        case 1:
            scVelocities = scBrightnesses;
            break;
        case 2:
            scVelocities = scSaturations;
            break;
        case 3:
            scVelocities = scXs;
            break;
        case 4:
            scVelocities = scYs;
            break;
        case 5:
            for (int i = 0; i < 16; i++) {
                scVelocities.at(i) = 0.0f;
            }
            break;
    }
    
    for (int i = 0 ; i < 16; i++ ) {
        scVelocities.at(i) = scVelocities.at(i)*scVelocitiesIntensities + scVelocitiesRefs;
    }
    
    
    
    switch(scH1Option) {
        case 0:
            h1Mods = scPresences;
            break;
        case 1:
            h1Mods = scBrightnesses;
            break;
        case 2:
            h1Mods = scSaturations;
            break;
        case 3:
            h1Mods = scXs;
            break;
        case 4:
            h1Mods = scYs;
            break;
        case 5:
            for (int i = 0; i < 16; i++) {
                h1Mods.at(i) = 0.0f;
            }
            break;
    }
    
    for (int i = 0 ; i < 16; i++ ) {
        h1Mods.at(i) = h1Mods.at(i)*scH1Intensities + scH1Refs;
    }
    
    
    
    
    switch(scH2Option) {
        case 0:
            h2Mods = scPresences;
            break;
        case 1:
            h2Mods = scBrightnesses;
            break;
        case 2:
            h2Mods = scSaturations;
            break;
        case 3:
            h2Mods = scXs;
            break;
        case 4:
            h2Mods = scYs;
            break;
        case 5:
            for (int i = 0; i < 16; i++) {
                h2Mods.at(i) = 0.0f;
            }
            break;
    }
    
    for (int i = 0 ; i < 16; i++ ) {
        h2Mods.at(i) = h2Mods.at(i)*scH2Intensities + scH2Refs;
    }
    
    
    
    
    
    switch(scLfoOption) {
        case 0:
            lfoMods = scPresences;
            break;
        case 1:
            lfoMods = scBrightnesses;
            break;
        case 2:
            lfoMods = scSaturations;
            break;
        case 3:
            lfoMods = scXs;
            break;
        case 4:
            lfoMods = scYs;
            break;
        case 5:
            for (int i = 0; i < 16; i++) {
                lfoMods.at(i) = 0.0f;
            }
            break;
    }
    
    for (int i = 0 ; i < 16; i++ ) {
        lfoMods.at(i) = lfoMods.at(i)*scLfoIntensities + scLfoRefs;
    }
    
}



/*---------------------------------------------------------------*/



void ofApp::drawSC(float x, float y, float w, float h) {
    
    float mw = w / 16.0f;
    ofFill();
    for (int i = 0; i < 16; i++) {
        
        float hu = lookupHues.at( ofMap(i, 0, 15, videoStartC, videoEndC) );
        float s = scSaturations.at(i) * 255.0f;
        float b = scBrightnesses.at(i) * 255.0f;
        float p = scTriggersBuf.at(i) * h;
        
        ofColor c; c.setHsb(hu, s, b);
        ofSetColor(c);
        ofDrawRectangle(x + mw*i, y+h, mw, -p);
        ofSetColor(255);
        float t = scThresholds.at(i).get();
        ofDrawLine(x + mw*i, y+h-(t*h), x + mw*i + mw, y+h-(t*h) );
        
    }
    
    ofSetColor(frameColor);
    ofNoFill();
    ofDrawRectangle(x, y, w, h);
    
    ofSetColor(fontColor);
    ofDrawBitmapString("SC Synth  Triggers", x, y -5);
}





/*---------------------------------------------------------------*/

void ofApp::startSc(){
    
    //string command = ofToDataPath("") + "sc/./startSc.sh&";
    //ofSystem(command);
    
    sc.setup();
    
    scStarted = true;
}


/*---------------------------------------------------------------*/

void ofApp::quitSc(){
    if (scStarted) {
        //string command = ofToDataPath("") + "sc/./quitSc.sh&";
        //ofSystem(command);
        sc.quitSC();
        
    }
    
    
    
}


/*---------------------------------------------------------------*/

void ofApp::updateMidiNotes() {
    
    //midiNotes.clear();
    for (int i= 0; i < midiScale.size(); i++) {
        //if (midiScale.at(i).get() >=0) {
        midiNotes.at(i) = midiScale.at(i).get();
        //}
    }
}
/*---------------------------------------------------------------*/

void ofApp::updateMidi() {
    
    //updateMidiNotes();
    /*--------------*/
    
    midiPresences.resize(midiScaleSize );
    midiBrightnesses.resize(midiScaleSize );
    midiSaturations.resize(midiScaleSize );
    midiXs.resize(midiScaleSize );
    midiYs.resize(midiScaleSize );
    
    
    for (int i = 0; i<midiScaleSize; i++) {
        int index = iround( ofMap(i, 0, midiScaleSize-1, videoStartC, videoEndC) );
        midiPresences.at(i) = presences.at(index);
        midiBrightnesses.at(i) = brightnesses.at(index);
        midiSaturations.at(i) = saturations.at(index);
        midiXs.at(i) = posXs.at(index);
        midiYs.at(i) = posYs.at(index);
    }
    
    
    
    /*--------------*/
    
    switch(midiOctaveOption) {
        case 0:
            midiOctaves = midiPresences;
            break;
        case 1:
            midiOctaves = midiBrightnesses;
            break;
        case 2:
            midiOctaves = midiSaturations;
            break;
        case 3:
            midiOctaves = midiXs;
            break;
        case 4:
            midiOctaves = midiYs;
            break;
            
        case 5:
            for (int i = 0; i<  midiScaleSize; i++) midiOctaves.at(i)= 0.0f;
            break;
            
    }
    
    for (int i = 0; i <  midiScaleSize; i++) midiOctaves.at(i)= midiOctaveIntensity*midiOctaves.at(i) + midiOctaveRef;
    
    /*--------------*/
    
    switch(midiTriggerOption) {
        case 0:
            midiTriggersBuf = midiPresences;
            break;
        case 1:
            midiTriggersBuf = midiBrightnesses;
            break;
        case 2:
            midiTriggersBuf = midiSaturations;
            break;
        case 3:
            midiTriggersBuf = midiXs;
            break;
        case 4:
            midiTriggersBuf = midiYs;
            break;
            
        case 5:
            for (int i = 0; i<  midiScaleSize; i++) midiTriggersBuf.at(i)= 0.0f;
            break;
            
    }
    
    
    
    /*--------------*/
    
    switch(midiVelocityOption) {
        case 0:
            midiVelocities = midiPresences;
            break;
        case 1:
            midiVelocities = midiBrightnesses;
            break;
        case 2:
            midiVelocities = midiSaturations;
            break;
        case 3:
            midiVelocities = midiXs;
            break;
        case 4:
            midiVelocities = midiYs;
            break;
            
        case 5:
            for (int i = 0; i<  midiScaleSize; i++) midiVelocities.at(i)= 0.0f;
            break;
            
    }
    
    for (int i = 0; i<  midiScaleSize; i++) midiVelocities.at(i)= midiVelocitiesIntensity*midiVelocities.at(i) + midiVelocitiesRef;
    
    
    
    /*--------------*/
    
    switch(midiAfterTouchOption) {
        case 0:
            midiAfterTouches = midiPresences;
            break;
        case 1:
            midiAfterTouches = midiBrightnesses;
            break;
        case 2:
            midiAfterTouches = midiSaturations;
            break;
        case 3:
            midiAfterTouches = midiXs;
            break;
        case 4:
            midiAfterTouches = midiYs;
            break;
            
        case 5:
            for (int i = 0; i<  midiScaleSize; i++) midiAfterTouches.at(i)= 0.0f;
            break;
            
    }
    
    for (int i = 0; i<  midiScaleSize; i++) midiAfterTouches.at(i)= midiAfterTouchesIntensity*midiAfterTouches.at(i) + midiAfterTouchesRef;
    
    /*--------------*//*--------------*/
    
    switch(midiCcOption1) {
            
        case 0:
            midiCc1 = averageColor.getHue()/255.0f;
            break;
            
        case 1:
            midiCc1 = averageColor.getBrightness()/255.0f;
            break;
            
        case 2:
            midiCc1 = averageColor.getSaturation()/255.0f;
            break;
            
        case 3:
            midiCc1 = white;
            break;
            
        case 4:
            midiCc1 = neutral;
            break;
            
        case 5:
            midiCc1 = black;
            break;
            
        case 6:
            midiCc1 = 0.0f;
            break;
            
    }
    
    midiCc1 = midiCc1*midiCcIntensity1 + midiCcRef1;
    
    /*--------------*//*--------------*/
    
    switch(midiCcOption2) {
            
        case 0:
            midiCc2 = averageColor.getHue()/255.0f;
            break;
            
        case 1:
            midiCc2 = averageColor.getBrightness()/255.0f;
            break;
            
        case 2:
            midiCc2 = averageColor.getSaturation()/255.0f;
            break;
            
        case 3:
            midiCc2 = white;
            break;
            
        case 4:
            midiCc2 = neutral;
            break;
            
        case 5:
            midiCc2 = black;
            break;
            
        case 6:
            midiCc2 = 0.0f;
            break;
            
    }
    
    midiCc2 = midiCc2*midiCcIntensity2 + midiCcRef2;
    
    
}




void ofApp::drawMidi(float x, float y, float w, float h) {
    
    float mw = w / midiScaleSize;
    ofFill();
    for (int i = 0; i < midiScaleSize; i++) {
        
        int index = ofMap(i, 0, midiScaleSize-1, videoStartC, videoEndC);
        
        float hu = lookupHues.at( index );
        float s = midiSaturations.at(i);
        float b = midiBrightnesses.at(i);
        float p = midiTriggersBuf.at(i) * h;
        
        ofColor c; c.setHsb(hu, s*255, b*255);
        ofSetColor(c);
        ofDrawRectangle(x + mw*i, y+h, mw, -p);
        
        ofSetColor(190);
        float t = midiThresholds.at(i).get();
        ofDrawLine(x + mw*i, y+h-h*t, x + mw*i + mw, y+h-h*t);
        
    }
    
    ofSetColor(frameColor);
    ofNoFill();
    ofDrawRectangle(x, y, w, h);
    
    ofSetColor(fontColor);
    ofDrawBitmapString("Midi Triggers", x, y -5);
    
    
}


vector<float> ofApp::interpolate( vector<float> input, int startI, int endI, int startO, int endO ) {
    
    vector<float> output(1024);
    
    if (endI>1023) endI = 1023;
    if (endO>1023) endO = 1023;
    
    
    if ( (endO-startO) <= (endI-startI) ){
        
        for (int i = 0; i < 1024; i++){
            if (i<startO || i > endO) {
                output.at(i) = 0;
            }
            else {
                int index = iround( ofMap(i, startO, endO, startI, endI) );
                output.at(i) = input.at(index);
            }
        }
    }
    
    
    
    else {
        
        int asize = endI-startI;
        tk::spline s;
        vector<double> indexes; indexes.resize( asize );
        vector<double> in; in.resize( asize );
        
        for (int i = startI; i < endI; i++) {
            indexes.at(i) = ofMap(i, startI, endI-1, startO, endO-1 ) ;
            in.at(i) = (double)input.at(i) ;
        }
        
        s.set_points(indexes, in, true);
        
        for (int i = 0; i < 1024; i++){
            if (i<startO || i >= endO) {
                output.at(i) = 0;
            }
            else {
                output.at(i) = (float)s(i);
            }
        }
        
    }
    
    
    return output;
}


/*---------------------------------------------------------------*/

void ofApp::startHap() {
    
    if (!hapStarted) {
        
        hap.begin(ofToDataPath("") + "HAPp.app");
        ofSleepMillis(4000);
        
        hap.setup("127.0.0.1", 57130, "127.0.0.1", 57140);
        ofSleepMillis(500);
        
        hap.setupPlayer( workPath );
        ofSleepMillis(10);
        
        hap.autoplay(autoPlay);
        ofSleepMillis(10);
        
        hap.random(randomMovie);
        ofSleepMillis(10);
        
        hap.setMinimumFile(minNumFiles);
        ofSleepMillis(10);
        
        hap.setRangeDuration(iround(zapDurationMin*60), iround(zapDurationMax*60) );
        ofSleepMillis(10);
        
        hap.setMaxSilence(iround(silenceTimer.get()) );
        ofSleepMillis(10);
        
        hapStarted = true;
        
        ofLogNotice() << "[::::MAIN::::] : HAPp STARTED";
    }
}


void ofApp::restartHap() {

        hap.restart(ofToDataPath("") + "HAPp.app");
        ofSleepMillis(4000);
        
        //hap.setSender("127.0.0.1", 57130);
        ofSleepMillis(500);
        
        hap.setupPlayer( workPath );
        ofSleepMillis(10);
        
        hap.autoplay(autoPlay);
        ofSleepMillis(10);
        
        hap.random(randomMovie);
        ofSleepMillis(10);
        
        hap.setMinimumFile(minNumFiles);
        ofSleepMillis(10);
        
        hap.setRangeDuration(iround(zapDurationMin*60), iround(zapDurationMax*60) );
        ofSleepMillis(10);
        
        hap.setMaxSilence(iround(silenceTimer.get()) );
        ofSleepMillis(10);
    
        hap.play();
    
        hapStarted = true;
    
    ofLogNotice() << "[::::MAIN::::] : HAPp RESTARTED";

}



void ofApp::endHap() {

        hap.happExit();

}


void ofApp::setGrayStyle(){
    
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Text]                  = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.00f, 0.00f, 0.00f, 0.70f);
    style.Colors[ImGuiCol_ChildWindowBg]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.05f, 0.07f, 0.10f, 0.90f);
    style.Colors[ImGuiCol_Border]                = ImVec4(0.70f, 0.70f, 0.70f, 0.65f);
    style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.33f, 0.33f, 0.33f, 0.30f);
    style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.80f, 0.90f, 0.90f, 0.40f);
    style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.65f, 0.90f, 0.90f, 0.45f);
    style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.20f, 0.17f, 0.17f, 0.83f);
    style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.80f, 0.40f, 0.40f, 0.20f);
    style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.27f, 0.29f, 0.32f, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.80f);
    style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
    style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.63f, 0.63f, 0.63f, 0.30f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.82f, 0.82f, 0.82f, 0.40f);
    style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(1.00f, 1.00f, 1.00f, 0.40f);
    style.Colors[ImGuiCol_ComboBg]               = ImVec4(0.20f, 0.20f, 0.20f, 0.99f);
    style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.90f, 0.90f, 0.90f, 0.50f);
    style.Colors[ImGuiCol_SliderGrab]            = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
    style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.80f, 0.49f, 0.49f, 1.00f);
    style.Colors[ImGuiCol_Button]                = ImVec4(0.03f, 0.03f, 0.05f, 0.60f);
    style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
    style.Colors[ImGuiCol_Header]                = ImVec4(0.4f, 0.43f, 0.49f, 1.0f);
    style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.31f, 0.31f, 0.49f, 0.90f);
    style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.23f, 0.21f, 0.31f, 0.80f);
    style.Colors[ImGuiCol_Column]                = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    style.Colors[ImGuiCol_ColumnHovered]         = ImVec4(0.60f, 0.70f, 0.70f, 1.00f);
    style.Colors[ImGuiCol_ColumnActive]          = ImVec4(0.70f, 0.90f, 0.90f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
    style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
    style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
    style.Colors[ImGuiCol_CloseButton]           = ImVec4(0.90f, 0.90f, 0.90f, 0.50f);
    style.Colors[ImGuiCol_CloseButtonHovered]    = ImVec4(0.70f, 0.80f, 0.90f, 0.60f);
    style.Colors[ImGuiCol_CloseButtonActive]     = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
    style.Colors[ImGuiCol_PlotLines]             = ImVec4(1.00f, 0.80f, 0.33f, 1.00f);
    style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(0.90f, 0.02f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.40f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.50f, 0.63f, 0.78f, 0.35f);
    style.Colors[ImGuiCol_ModalWindowDarkening]  = ImVec4(0.10f, 0.10f, 0.10f, 0.35f);
    
    
    
}








