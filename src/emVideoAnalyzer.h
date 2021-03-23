#pragma once

#include "ofMain.h"
#include "spline.h"


class emVideoAnalyzer : public ofThread {
    
    
    
    //----------------------------------------------------PUBLIC//----------------------------------------------------
    
    public :
    
    
    bool ready = true;
    
    string debug = "pre-start";
    
    //----------------------------------------------------
    //    void run() {
    //        startThread();
    //    }
    
    void setup( int w, int h) {
        
        width = w;
        height = h;
        nBands = 1024;
        
        presences.resize(256, 0.0f);
        brightnesses.resize(256, 0.0f);
        saturations.resize(256, 0.0f);
        posXs.resize(256, 0.0f);
        posYs.resize(256, 0.0f);
        
        velocities.resize(256, 0.0f);
        angles.resize(256, 0.0f);
        
        outpresences.resize(nBands, 0.0f);
        outbrightnesses.resize(nBands, 0.0f);
        outsaturations.resize(nBands, 0.0f);
        outposXs.resize(nBands, 0.0f);
        outposYs.resize(nBands, 0.0f);
        
        averageColor = ofColor::black;
        
        ready = true;
        
        startThread();
        
        //debug = "started";
        
    }
    
    //----------------------------------------------------
    
    
    void newFrame( ofPixels pixels, int nB) {
        
        nBands = nB;
        pixelsCopy = pixels;
        ready = false;
        
        //debug = "new frame";
        
    }
    
    
    //----------------------------------------------------
    
    bool getReady() { return ready; }
    
    
    
    
    //----------------------------------------------------
    
    void setHuesIndexes(vector<double> indexes) {
        huesIndexes = indexes;
    }
    
    
    //----------------------------------------------------
    
    
    
    vector<float> getPresences(float cont, float amp, float smooth) {
        //presencesTCont = tCont;
        presencesCont = cont;
        presencesAmp = amp;
        presencesSmooth = smooth;
        
        return outpresences;
    }
    
    vector<float> getBrightnesses(float cont, float amp, float smooth) {
        //brightnessesTCont = tCont;
        brightnessesCont = cont;
        brightnessesAmp = amp;
        brightnessesSmooth = smooth;
        
        return outbrightnesses;
    }
    
    vector<float> getSaturations(float cont, float amp, float smooth) {
        //saturationssTCont = tCont;
        saturationsCont = cont;
        saturationsAmp = amp;
        saturationsSmooth = smooth;
        
        return outsaturations;
    }
    
    vector<float> getPositionsX(float cont, float amp, float smooth) {
        //positionsTCont = tCont;
        panCont = cont;
        panAmp = amp;
        panSmooth = smooth;
        
        return outposXs;
    }
    
    vector<float> getPositionsY(float cont, float amp, float smooth) {
        //positionsTCont = tCont;
        tiltCont = cont;
        tiltAmp = amp;
        tiltSmooth = smooth;
        
        return outposYs; }
    
    float getBlack(int thresh, float cont, float amp, float smooth) {
        //blackTCont = tCont;
        blackThresh = thresh;
        blackCont = cont;
        blackAmp = amp;
        blackSmooth = smooth;
        
        return black;
    }
    
    float getNeutral(int thresh, float cont, float amp, float smooth) {
        //neutralTCont = tCont;
        neutralThresh = thresh;
        neutralCont = cont;
        neutralAmp = amp;
        neutralSmooth = smooth;
        
        return neutral;
    }
    
    float getWhite(int thresh, float cont, float amp, float smooth) {
        //whiteTCont = tCont;
        whiteThresh = thresh;
        whiteCont = cont;
        whiteAmp = amp;
        whiteSmooth = smooth;
        
        return white;
    }
    
    float getLightness(float cont, float amp, float smooth) {
        //lightnessTCont = tCont;
        lightnessCont = cont;
        lightnessAmp = amp;
        lightnessSmooth = smooth;
        
        return lightness;
    }
    
    vector<float> getVelocities(float cont, float amp, float smooth) {
        //velocitiesTCont = tCont;
        velocitiesCont = cont;
        velocitiesAmp = amp;
        velocitiesSmooth = smooth;
        
        return velocities;
    }
    
    vector<float> getAngles(float cont, float amp, float smooth) {
        //anglesTCont = tCont;
        anglesCont = cont;
        anglesAmp = amp;
        anglesSmooth = smooth;
        
        return angles;
    }
    
    ofColor getAverageColor() {
        
        return averageColor;
    }
    
    float getAveragehue(float avhsmooth){
        averageHueSmooth = avhsmooth;
        return averageHue;
    }
    
    float getAverageBrightness(float avbamp, float avbsmooth) {
        averageBrightnessAmp = avbamp;
        averageBrightnessSmooth = avbsmooth;
        return averageBrightness;
        
    }
    
    float getAverageSaturation(float avsamp, float avssmooth) {
        averageSaturationAmp = avsamp;
        averageSaturationSmooth = avssmooth;
        return averageSaturation;
    }
    
    float getRed() {
        return R;
    }
    
    float getGreen() {
        return V;
    }
    
    float getBlue() {
        return B;
    }
    
    
    
    //----------------------------------------------------PRIVATE//----------------------------------------------------
    
    private :
    
    int width = 150;
    
    int height = 100;
    
    int nBands = 1024;
    
    float redR = 0.02;
    int redI = 10;
    
    float blueR = 0.02;
    int blueI = 10;
    
    vector <double> huesIndexes;
    
    ofPixels pixelsCopy;
    
    vector<float> presences;
    vector<float> brightnesses;
    vector<float> saturations;
    vector<float> posXs;
    vector<float> posYs;
    
    vector<float> velocities;
    vector<float> angles;
    
    
    vector<float> outpresences;
    vector<float> outbrightnesses;
    vector<float> outsaturations;
    vector<float> outposXs;
    vector<float> outposYs;
    
    vector<float> outvelocities;
    vector<float> outangles;
    
    
    float presencesCont = 0.0f;
    float brightnessesCont = 0.0f;
    float saturationsCont = 0.0f;
    float positionsCont = 0.0f;
    float velocitiesCont = 0.0f;
    float anglesCont = 0.0f;
    float panCont = 0.0f;
    float tiltCont = 0.0f;
    
    float presencesAmp = 1.0f;
    float brightnessesAmp = 1.0f;
    float saturationsAmp = 1.0f;
    float positionsAmp = 1.0f;
    float velocitiesAmp = 1.0f;
    float anglesAmp = 1.0f;
    float panAmp = 1.0f;
    float tiltAmp = 1.0f;
    
    float presencesSmooth = 0.5f;
    float brightnessesSmooth = 0.5f;
    float saturationsSmooth = 0.5f;
    float positionsSmooth = 0.5f;
    float velocitiesSmooth = 0.5f;
    float anglesSmooth = 0.5f;
    float panSmooth = 0.5f;
    float tiltSmooth= 0.5f;
    
    float black = 0.0f;
    float neutral = 0.0f;
    float white = 0.0f;
    
    float blackThresh = 3.0f;
    float neutralThresh = 3.0f;
    float whiteThresh = 252.0f;
    
    float blackCont = 0.0f;
    float neutralCont = 0.0f;
    float whiteCont = 0.0f;
    
    float blackAmp = 1.0f;
    float neutralAmp = 1.0f;
    float whiteAmp = 1.0f;
    
    float blackSmooth = 0.5f;
    float neutralSmooth = 0.5f;
    float whiteSmooth = 0.5f;
    
    ofColor averageColor;
    
    float R = 0.0f;
    float V = 0.0f;
    float B = 0.0f;
    
    float lightness = 0.0f;
    
    float lightnessCont = 0.0f;
    float lightnessAmp = 1.0f;
    float lightnessSmooth = 0.0f;
    
    float averageHue = 0.0f;
    float averageHueSmooth = 0.5f;
    
    float averageBrightness = 0.0f;
    float averageBrightnessAmp = 1.0f;
    float averageBrightnessSmooth = 0.5f;
    
    float averageSaturation = 0.0f;
    float averageSaturationAmp = 1.0f;
    float averageSaturationSmooth = 0.0f;
    
    
    
    //----------------------------------------------------THREADED//----------------------------------------------------
    
    
    
    
    void threadedFunction() {
        
        while (isThreadRunning() ) {
            
            
            
            if (ready == false) {
                
                if (pixelsCopy.getWidth() != width || pixelsCopy.getHeight() != height) {
                    pixelsCopy.resize(width, height, OF_INTERPOLATE_NEAREST_NEIGHBOR);
                }
                
                analyze( nBands );
                
                ready = true;
            }
            else { sleep(5); }
        }
        
    }
    
    
    //----------------------------------------------------//----------------------------------------------------
    
    
    void analyze( int n) {
        
        long a = ofGetElapsedTimeMillis();
        
        float numPixels = (float)(width * height);
        
        float colorCount = 0.0f;
        
        float newBlack = 0.0f;
        float newNeutral = 0.0f;
        float newWhite = 0.0f;
        
        float newR = 0.0f;
        float newV = 0.0f;
        float newB = 0.0f;
        
        vector<float> newPresences(256, 0.0f);
        vector<float> newBrightnesses(256, 0.0f);
        vector<float> newSaturations(256, 0.0f);
        vector<float> newPosXs(256, width/2.0f );
        vector<float> newPosYs(256, height/2.0f );
        
        /*----------------------------------SORTING*/
        
        
        
        for (int i = 0; i < pixelsCopy.getWidth(); i ++) {
            
            for (int j = 0; j < pixelsCopy.getHeight(); j ++) {
                
                ofColor c = pixelsCopy.getColor(i, j);
                
                newR = newR + (float)c.r;
                newV = newV + (float)c.g;
                newB = newB + (float)c.b;
                
                float h = (float)c.getHue();
                float s = (float)c.getSaturation();
                float b = (float)c.getBrightness();
                
                if (b <= blackThresh) {
                    newBlack += 1.0f;
                }
                
                else if (b >= whiteThresh && s <= neutralThresh) {
                    newWhite += 1.0f;
                }
                
                else if (s <= neutralThresh) {
                    
                    newNeutral += 1.0f;
                    
                }
                
                else {
                    
                    int index = rint(h);
                    
                    newPresences[index] = newPresences[index] + 1.0f;
                    newBrightnesses[index] = newBrightnesses[index] + b;
                    newSaturations[index] = newSaturations[index] + s;
                    
                    newPosXs[index] = newPosXs[index] + i ;
                    newPosYs[index] = newPosYs[index] + j ;
                    
                    colorCount =  colorCount + 1.0f;
                    
                }
                
            }
        }
        
        if (colorCount == 0) colorCount =  1.0f;
        
        /*----------------------------------SMOOTHING*/
        //        float max = 0.0f;
        //
        //        for (int i = 0; i <  presences.size(); i++){
        //            if (newPresences.at(i) >= max) max = presences.at(i);
        //        }
        //
        //        float ratio;
        //        max /= colorCount;
        //        if (max == 0.0) ratio = 1.0f; else ratio = 1.0f/max;
        //
        
        
        //for (int index = 0; index < nBands; index++) {
        for (int index = 0; index < 256; index++) {
            
            float b, s, x, y;
            
            float p = newPresences[index];
            
            if (p<=0) {
                b = 0.0f;
                s = 0.0f;
                x = 0.5f;
                y = 0.5f;
            }
            else{
                b = (newBrightnesses[index] / p ) / 255.0f;
                s =  (newSaturations[index] / p ) / 255.0f;
                x = (newPosXs[index] / p ) / width;
                y = (newPosYs[index] / p ) / height;
            }
            
            presences[index] = dynamize(presences[index], pow(p / colorCount, 0.5), 0, 1, presencesCont, presencesAmp, presencesSmooth);
            
            brightnesses[index] = dynamize(brightnesses[index], pow(b, 0.5), 0, 1, brightnessesCont, brightnessesAmp, brightnessesSmooth);
            
            saturations[index] = dynamize(saturations[index], pow(s, 0.5), 0, 1, saturationsCont, saturationsAmp, saturationsSmooth);
            
            x = ofMap(x, 0.0f, 1.0f, -1.0f, 1.0f);
            x *= panAmp;
            posXs[index] = smooth(posXs[index], x, panSmooth);
            posXs[index] = ofClamp(posXs[index], -1.0f, 1.0f);
            
            y = ofMap(y, 0.0f, 1.0f, -1.0f, 1.0f);
            y *= tiltAmp;
            posYs[index] = smooth(posYs[index], y, tiltSmooth);
            posYs[index] = ofClamp(posYs[index], -1.0f, 1.0f);
            
        }
        
        
        
        
        
        black = dynamize(black, newBlack/numPixels, 0, 1, blackCont, blackAmp, blackSmooth);
        neutral = dynamize(neutral, newNeutral/numPixels, 0, 1, neutralCont, neutralAmp, neutralSmooth);
        white = dynamize(white, newWhite/numPixels, 0, 1, whiteCont, whiteAmp, whiteSmooth);
        
        R = newR/numPixels; //dynamize(R , newR/numPixels, 0, 255.0f, lightnessCont, lightnessAmp, lightnessSmooth);
        V = newV/numPixels; //dynamize(V , newV/numPixels, 0, 255.0f, lightnessCont, lightnessAmp, lightnessSmooth);
        B = newB/numPixels; //dynamize(B , newB/numPixels, 0, 255.0f, lightnessCont, lightnessAmp, lightnessSmooth);
        
        lightness = ( R + V + B)/3.0f/255.0f;
        
        averageColor.set(R, V, B, 255.0f);
        
        averageHue = smooth(averageHue, averageColor.getHue()/255.0f, averageHueSmooth);
        averageBrightness = dynamize(averageBrightness, averageColor.getBrightness()/255.0f, 0, 1, 0, averageBrightnessAmp, averageBrightnessSmooth);
        averageSaturation = dynamize(averageSaturation, averageColor.getSaturation()/255.0f, 0, 1, 0, averageSaturationAmp, averageSaturationSmooth);
        
        averageColor.setHsb( averageHue, averageBrightness, averageSaturation);
        
        outpresences = interpolate(presences, n, true, 0, 1);
        outbrightnesses = interpolate(brightnesses, n, false, 0, 1);
        outsaturations = interpolate(saturations, n, false, 0, 1);
        outposXs = interpolate(posXs, n, true, -1, 1);
        outposYs = interpolate(posYs, n, true, -1, 1);
        
        
        
        a = ofGetElapsedTimeMillis() - a; //(~5-8 ms)
        
        debug = ofToString( averageColor );
        
    }
    
    //----------------------------------------------------
    
    float dynamize(float value, float newValue, float min, float max, float cont, float amp, float smooth ) {
        
        // contrast formula
        // f = ( 259*(255+c) )/( 255*(259-c) ); newVal = f * (val-128) + 128;
        // normalized : formula = ( 259*(255+c*128) )/( 255*(259-c*128) )/255; newVal = f * (val-0.5) + 0.5;
        
        //compression :float factor = (inBuffer.getRMSAmplitude() - threshold) / ratio + threshold;
        
        //float factor = (value - threshold) / ratio + threshold;
        //if (abs(newValue) > threshold) newValue = factor * newValue;
        
        
        
        //contrast
        //        float fac = 259.0f / 255.0f;
        //        float cfactor = ( fac * (1 + cont) ) / (fac - cont);
        //        newValue = cfactor * (newValue - 0.5) + 0.5;
        
        //amp
        newValue *= amp;
        
        //smooth
        newValue = (value * smooth) + (newValue * (1 - smooth) );
        
        newValue = ofClamp( newValue, min, max);
        
        return newValue;
        
    }
    
    //----------------------------------------------------
    
    float smooth(float oldValue, float newValue, float smoothFactor){
        
        newValue = (oldValue * smoothFactor) + (newValue * (1 - smoothFactor) );
        
        return newValue;
        
    }
    
    //----------------------------------------------------
    
    vector<float> interpolate(vector<float> array, int n, bool bicubic, float min, float max){
        
        tk::spline s;
        //s.set_boundary(tk::spline::bd_type(1), 0.0, tk::spline::bd_type(2), 0.0);
        
        vector<double> input;
        input.resize(huesIndexes.size() );
        
        vector<float> output;
        output.resize(n);
        
        //rearrange to natural
        
        for (int i = huesIndexes.size(); i < array.size(); i++) {
            ofColor c;
            c.setHsb(i, 255, 255);
            int rIndex = ofMap(i, huesIndexes.size(), array.size()-1, redI, 0 );
            int bIndex = ofMap(i, huesIndexes.size(), array.size()-1,  huesIndexes.size()-blueI, huesIndexes.size() );
            array.at(rIndex) = array.at(rIndex) + ( array.at(i) * (float)c.r/255.0f*redR );
            array.at(bIndex) = array.at(bIndex) + ( array.at(i) * (float)c.b/255.0f*blueR );
        }
        
        
        for (int i = 0 ; i < huesIndexes.size(); i++){
            input.at(i) = (double)( array.at(i) );
        }
        
        //interpolate
        
        s.set_points(huesIndexes, input, bicubic);
        
        for (int i = 0; i < n; i++) {
            float index = ofMap(i, 0, n-1, 0, 1023);
            output.at(i) = ofClamp( (float)(s(index) ), min, max );
        }
        
        return output;
        
    }
    
    
    
    
    
    //--------------------END emVideoAnalyzer
};
