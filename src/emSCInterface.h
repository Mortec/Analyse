//
//  emSCInterface.h
//  Analyse
//
//  Created by E.M. on 09/10/2017.
//
//

#pragma once

#include "ofMain.h"

#define HOST "localhost"
#define PORTS 57110
#define PORTL 57120
#define PORTR 57130

// this needs to be threaded otherwise it blocks the main thread
class StartSC : public ofThread{
    
public:
    StartSC(){}
    
    void openSC(string s){
        scfile = s;
        threadStart();
    }
    
    void threadStart(){
        startThread();
    }
    
    
    void threadStop(){
        ofSystem("kill -s 2 $(ps -A | grep -m1 sclang | awk '{print $1}')");
        ofSystem("kill -s 2 $(ps -A | grep -m1 scsynth | awk '{print $1}')");
        stopThread();
    }
    
    
    void threadedFunction(){
        
        string command = ofToDataPath("") + "sc/./startSc.sh&";
        
        ofSystem(command);
    }
    
    
    ~StartSC(){threadStop();}
    
    
private:
    string scfile;
};




// The SuperCollider Interface --------------------------------------------------------
class emSCInterface : public ofThread{
    
public:
    
    void setup(){
        
        startsc.openSC("myscfile.scd");
        
        // open an outgoing connection to sclang
        sender.setup(HOST, PORTL);
        
        // listen on the given port
        receiver.setup(PORTR);
        
        threadStart();
    }
    
    void quitSC() {
        
        msg.clear();
        msg.setAddress("/EXIT");
        sender.sendMessage(msg);
    
    }
    
    
    ~emSCInterface(){
        threadStop();
    }

    
private:
    ofxOscSender sender;
    ofxOscReceiver receiver;
    ofxOscMessage msg;
    StartSC startsc;
    
    void threadStart(){
        startThread();   // blocking, verbose
    }
    
    void threadStop(){
         //       msg.clear();
         //       msg.setAddress("/quit");
        //        sender.setup(HOST, PORTS); // change to server port
        //        sender.sendMessage(msg);
        stopThread();
    }
    
    
    void threadedFunction(){
        
        while( isThreadRunning() != 0 ){
            
            // check for waiting messages
            while(receiver.hasWaitingMessages()){
                // get the next message
                ofxOscMessage m;
                receiver.getNextMessage(&m);
                string msg_string;
                msg_string = m.getAddress();
                msg_string += ": ";
                for(int i = 0; i < m.getNumArgs(); i++){
                    // get the argument type
                    msg_string += m.getArgTypeName(i);
                    msg_string += ":";
                    // display the argument - make sure we get the right type
                    if(m.getArgType(i) == OFXOSC_TYPE_INT32){
                        msg_string += ofToString(m.getArgAsInt32(i));
                    }
                    else if(m.getArgType(i) == OFXOSC_TYPE_FLOAT){
                        msg_string += ofToString(m.getArgAsFloat(i));
                    }
                    else if(m.getArgType(i) == OFXOSC_TYPE_STRING){
                        msg_string += m.getArgAsString(i);
                    }
                    else{
                        msg_string += "unknown";
                    }
                }
                
                ofLogNotice() << "[::::FROM SC::::] " + msg_string << endl;
            }
            
            ofSleepMillis(3);
        }
    }
    
    
    
};
