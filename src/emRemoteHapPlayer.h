//
//  emRemoteHapPlayer.h
//  HAPp
//
//  Created by E.M. on 16/11/2017.
//
//


#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxSyphon.h"


class emRemoteHapPlayer  {
    
    
    
    ///-------------------------------------------------PUBLIC
    
    
    public :
    
    string feedback = "";
    
    string fileName = "nothing";
    
    long pulseTimer;
    
    bool alive = true;
    
    void begin(string pathToHapp) {
        
        
        string cmd = "open -g " + pathToHapp +" &"; //e.g. /Applications/HAPp.app
        
        ofSystem(cmd);

        happClient.setup();
        
        happClient.set("HAPp_Texture", "HAPp");
        
        syphonBuffer.allocate(1920, 1080, GL_RGBA);
        
        
    }
    
    void setup(string rAddr, int rPort, string happAddr, int happPort) {
        
        receiver.setup(rPort);
        
        sender.setup(happAddr, happPort);

        //setSender(rAddr, rPort);
        
        pulseTimer = ofGetElapsedTimeMillis();
        
        alive = true;
        
    }
    
    
    
    void restart(string pathToApp) {
        
        string killFB = ofSystem("kill -s 2 $(ps -A | grep -m1 HAPp | awk '{print $1}')");
        
        ofLogNotice() << "[:: HAPp ::] " << killFB;
        
        string cmd = "open -g " + pathToApp +" &"; //e.g. /Applications/HAPp.app
        
        ofSystem(cmd);
        
        pulseTimer = ofGetElapsedTimeMillis();
        
        alive = true;
    
    }
    

    
    
    void update() {
        
        if ( receiver.hasWaitingMessages() ) dispatchOscMessage();
        
        syphonBuffer.begin();
        
        happClient.bind();
        ofClear(255, 255, 255, 0);
        happClient.draw(0,0, syphonBuffer.getWidth(), syphonBuffer.getHeight() );
        happClient.unbind();
        
        syphonBuffer.end();
        
        if ( (ofGetElapsedTimeMillis()-pulseTimer) > 30000 ) alive = false;
        else alive = true;
        
    }
    
    
    
    
    ofTexture* getTexture() {
        
        return &syphonBuffer.getTexture();
        
        
    }
    
    string getFileName(){
    
        return fileName;
    
    }
    
    
    
    void setReceiver(int port) {
        sendMessage("setReceiver", port);
    }
    
    void setSender(string addr, int port) {
        sendMessage("setSender", addr, port);
    }
    
    void setupPlayer(string path){
        sendMessage("setup", path);
    }
    
    void setPlayPath(string path){
        sendMessage("setPlayPath", path);
    }
    
    void setRescuePath(string path){
        sendMessage("setRescuePath", path);
    }
    
    
    
    void play() {
        sendMessage("play");
    }
    void pause() {
        sendMessage("pause");
    }
    void stop() {
        sendMessage("stop");
    }
    void next() {
        sendMessage("next");
    }
    void previous() {
        sendMessage("previous");
    }
    void change(int fNum) {
        sendMessage("change", fNum);
    }
    
    
    
    void setSpeed(float speed){
        sendMessage("setSpeed", speed);
    }
    void setPosition(float pos){
        sendMessage("setPosition", pos);
    }
    void setVolume(float vol){
        sendMessage("setVolume", vol);
    }
    
    
    
    void setLoop(int loopstate) {
        sendMessage("setLoop", loopstate);
    }
    void autoplay(int state) {
        sendMessage("autoplay", state);
    }
    void random(int state) {
        sendMessage("random", state);
    }
    void setRangeDuration(int min, int max) {
        sendMessage("setRangeDuration", min, max);
    }
    void setMinimumFile(int min) {
        sendMessage("setMinimumFile", min);
    }
    void setMaxSilence(int max) {
        sendMessage("setMaxSilence", max);
    }
    void updateRMS(float rms) {
        sendMessage("updateRMS", rms);
    }
    
    
    
    void happExit() {
        sendMessage("exit");
    }
    
    
    
    
    ///-------------------------------------------------PRIVATE
    
    
    private :
    
    ofxOscSender sender;
    ofxOscReceiver receiver;
    
    ofxSyphonClient happClient;
    
    ofFbo syphonBuffer;
    
    
    
    void sendMessage(string path) {
        
        ofxOscMessage m;
        m.setAddress("/HAPp/" + path);
        sender.sendMessage(m, false);
        
    }
    
    void sendMessage(string path, string str) {
        
        ofxOscMessage m;
        m.setAddress("/HAPp/" + path);
        m.addStringArg(str);
        sender.sendMessage(m, false);
        
    }
    
    void sendMessage(string path, string str, int arg) {
        
        ofxOscMessage m;
        m.setAddress("/HAPp/" + path);
        m.addStringArg(str);
        m.addIntArg(arg);
        sender.sendMessage(m, false);
        
    }
    
    void sendMessage(string path, int val) {
        
        ofxOscMessage m;
        m.setAddress("/HAPp/" + path);
        m.addIntArg(val);
        sender.sendMessage(m, false);
        
    }
    
    void sendMessage(string path, int val1, int val2) {
        
        ofxOscMessage m;
        m.setAddress("/HAPp/" + path);
        m.addIntArg(val1);
        m.addIntArg(val2);
        sender.sendMessage(m, false);
        
    }
    
    void sendMessage(string path, float val) {
        
        ofxOscMessage m;
        m.setAddress("/HAPp/" + path);
        m.addFloatArg(val);
        sender.sendMessage(m, false);
        
    }
    
    
    void dispatchOscMessage() {
        
        while( receiver.hasWaitingMessages() )
        {
            
            ofxOscMessage m;
            receiver.getNextMessage( &m );
            
            
            if ( m.getAddress() == "/fromHAPp" )
            {
                feedback = m.getArgAsString(0);
                ofLogNotice() << "[:: HAPp ::] " << feedback;
            }
            
            
            if ( m.getAddress() == "/fromHAPp/Play" )
            {
                feedback = m.getArgAsString(0);
                fileName = feedback;
                ofLogNotice() << "[:: HAPp ::] Playing : " << feedback;
            }
            
            
            
            
            if ( m.getAddress() == "/HAPpPulse" )
            {
                feedback = m.getArgAsString(0);
                ofLogNotice() << "[:: HAPp ::] " << feedback;
                pulseTimer = ofGetElapsedTimeMillis();
            }
        }

    }
    
    
    
    
    
};
/* END emRemoteHapPlayer_h */
