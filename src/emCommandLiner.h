#pragma once

#include "ofMain.h"



class emCommandLiner : public ofThread {
    
    
    
    //----------------------------------------------------
    public :
    
    void line(string line) {
        commandLine = line;
        commandDone = false;
        startThread();
    }
    
    string getAnswer(){
        return fromSystem;
    }
    
    
    //----------------------------------------------------
    private :
    
    
    void threadedFunction() {

        ofLogNotice() << "[:::COMMAND SENT:::] : " << commandLine;
        
        fromSystem = ofSystem(commandLine);
        
        ofLogNotice() << "[:::COMMAND RETURN:::] : " << fromSystem;
        
        commandDone = true;
        
        //stopThread();

    }
    
    //----------------------------------------------------
    string commandLine = "";
    
    bool commandDone = false;
    
    string fromSystem = "null";
    
    //----------------------------------------------------
    /*
    vector<string> getLocalIPs()
    {
        vector<string> result;
        
#ifdef TARGET_WIN32
        
        string commandResult = ofSystem("ipconfig");
        //ofLogVerbose() << commandResult;
        
        for (int pos = 0; pos >= 0; )
        {
            pos = commandResult.find("IPv4", pos);
            
            if (pos >= 0)
            {
                pos = commandResult.find(":", pos) + 2;
                int pos2 = commandResult.find("\n", pos);
                
                string ip = commandResult.substr(pos, pos2 - pos);
                
                pos = pos2;
                
                if (ip.substr(0, 3) != "127") // let's skip loopback addresses
                {
                    result.push_back(ip);
                    //ofLogVerbose() << ip;
                }
            }
        }
        
#else
        
        string commandResult = ofSystem("ifconfig");
        
        for(int pos = 0; pos >= 0; )
        {
            pos = commandResult.find("inet ", pos);
            
            if(pos >= 0)
            {
                int pos2 = commandResult.find("netmask", pos);
                
                string ip = commandResult.substr(pos+5, pos2-pos-6);
                
                pos = pos2;
                
                if(ip.substr(0, 3) != "127") // let's skip loopback addresses
                {
                    result.push_back(ip);
                    //ofLogVerbose() << ip;
                }
            }
        }
        
#endif
        
        return result;
    }
    */
    
};
