#pragma once

#include "ofMain.h"

#include "ofxOsc.h"
#include "ofxNetwork.h"
#include "ofxSyphon.h"
#include "ofxAudioAnalyzer.h"
#include "ofxImGui.h"
#include "ofxMidi.h"
#include "ofxAudioUnit.h"


#include "emMovieManager.h"
#include "emVideoAnalyzer.h"
#include "emRemoteHapPlayer.h"
#include "emSCInterface.h"

#include "spline.h"



class ofApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();
    void drawSecondScreen(ofEventArgs & args);
    
    void drawVideoAnalysis(int x, int y, int w , int h);
    void drawAudioAnalysis(int x, int y, int w , int h);

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	void exit();
    
    ofParameter<int> autoShutDown = 0;
    ofParameter<int> shutDownHour = 23;
    ofParameter<int> shutDownMinute = 0;

    int iround(float v);
    
	ofParameter<bool> configVisible;
	ofParameter<bool> settingsVisible;
	ofParameter<bool> streamsGuiVisible;


	void setupPrint();
    
    bool setupComplete = false;



//////////////////////////////////////////////////////////////////////////////////////////

	//APPEARANCE

//////////////////////////////////////////////////////////////////////////////////////////
    
    
    
    
    string datapath;
    
    //GUI
    ofxImGui::Gui gui;
    
    bool drawGui();
    
    ofxImGui::Settings guiSettings;
    ImGuiWindowFlags window_flags = 0;

    bool guiVisible;
    
    bool showFrameRate = false;
    
	ofParameter<ofColor> backgroundColor;

	ofParameter<ofColor> subFrameColor;
	ofParameter<ofColor> frameColor;
	ofParameter<ofColor> overFrameColor;


	ofParameter<ofColor> subFontColor;
	ofParameter<ofColor> fontColor;
	ofParameter<ofColor> overFontColor;

	ofParameter<float> subStroke;
	ofParameter<float> stroke;
	ofParameter<float> overStroke;

	ofParameter<ofTrueTypeFont> subFont;
	ofParameter<ofTrueTypeFont> mainFont;
	ofParameter<ofTrueTypeFont> overFont;


	ofParameter<float> subFontSize;
	ofParameter<float> mainFontSize;
	ofParameter<float> overFontSize;

	ofParameter<int> previewWidth;
	ofParameter<int> previewHeight;
	ofParameter<int> videoWidth;
	ofParameter<int> videoHeight;
	ofParameter<int> thumbsWidth;
	ofParameter<int> thumbsHeight;
    
    

//////////////////////////////////////////////////////////////////////////////////////////

	//CONFIG

//////////////////////////////////////////////////////////////////////////////////////////

    
    

	ofParameter<int> frameRate;

	ofParameter<string> workPath;
	string playPath;
	string rescuePath;
	string errorPath;
	string archivePath;
	string printPath;


	ofParameter<string> pathToVids;
	ofParameter<string> pathToStreamsListFile;
	ofParameter<string> pathToDirectStreamsListFile;

	/*******************************/


	ofParameter<int> oscSyncInputPort;
	ofParameter<int> oscSyncOutputPort;

	void setupConfig();
	void loadConfig();
	void saveConfig();


//////////////////////////////////////////////////////////////////////////////////////////

	//VIDEO STUFF

//////////////////////////////////////////////////////////////////////////////////////////



	//VIDEO PARAMETERS
    
    ofParameter<int> videoInputOption;
    ofParameter<int> videoOutputOption;
    
    ofParameter<int> videoUsbInputOption;
    ofParameter<string> camName;

	vector<string> videoInputOptions = {"Movies", "USB", "Syphon", "Broadcast", "SoundGradient", "None"};
	vector<string> videoOutputOptions = {"none", "Broadcast", "Sound_Gradient_Texture"};

	ofParameter<string> videoInputName;
	ofParameter<string> videoOutputName;
	ofParameter<string> syphon_1_OutputName;
	ofParameter<string> syphon_2_OutputName;
    
    ofParameter<int> varispeedOption;
    ofParameter<int> varispeedOn;
    ofParameter<float> varispeedIntensity;
    ofParameter<float> varispeedRef;
    float movieSpeed;
    
    void updateMovieSpeed();
    

	ofParameter<int> videoOutWidth;
	ofParameter<int> videoOutHeight;

	ofParameter<string> oscVideoDataOutputAddr;
	ofParameter<int> oscVideoDataOutputPort;

	ofParameter<int> videoAnalyseOn;
    ofParameter<int> nBandsAnalysis;
	ofParameter<int> videoProcessingOn;


	ofParameter<int> sendVideoOsc;
	ofParameter<int> sendToSyphon;


	//APPEARANCE
    ofParameter<float> videoCrop;
	ofParameter<float> zoomFactor;
	ofParameter<float> zoomPosX;
	ofParameter<float> zoomPosY;


	//tuning
    ofParameter<float> temperature;
	ofParameter<float> contrast;
	ofParameter<float> brightness;
	ofParameter<float> saturation;
    
    
    //VIDEO MAPPING
    vector<string> videoArrayOptions = {"Presences", "Brightnesses", "Saturations", "Horizontal Positions", "Vertical Positions", "None"};
    
    vector<string> videoUnaryOptions = { "Average Hue", "Average Brighness", "Average Saturation", "White Presence", "Gray Presence", "Black Presence", "None"};
    
    ofParameter<int> videoStartC;
    ofParameter<int> videoEndC;
    
    ofParameter<int> videoStartF;
    ofParameter<int> videoEndF;
    
    ofParameter<int> nBandsHisto;
    
    
    //-----------------------------MIDI
    
    ofParameter<int> midiOutOn;
    
    
    ofParameter<string> midiOutputName;
    vector<string> midiOuputNames = {"bla", "blabla"};
    ofParameter<int> midiOutputOption;
    vector<string> midiOutputs;
    ofParameter<int> midiChannel;
    
    ofParameter<int> midiScaleSize;
    ofParameter<int> midiScaleKey;
    string notesNames[12] = {"A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#"};
    vector<ofParameter<int>> midiScale;
    ofParameter<int> midiScaleOption;
    ofParameter<string> midiScaleName;
    vector<string> scales;
    vector<int> midiNotes;
    vector<bool> midiKeyboard;
    
    ofParameter<int> midiTriggerOption;
    vector<ofParameter<float>> midiThresholds;
    vector<float> midiTriggersBuf;
    vector<bool> midiTriggers;
    vector<bool> lastmidiTriggers;
    
    ofParameter<int> midiOctaveOption;
    ofParameter<int> midiOctaveIntensity;
    ofParameter<int> midiOctaveRef;
    vector<float> midiOctaves;
    
    ofParameter<int> midiVelocityOption;
    ofParameter<float> midiVelocitiesIntensity;
    ofParameter<float> midiVelocitiesRef;
    vector<float> midiVelocities;
    
    ofParameter<int> midiAfterTouchOption;
    ofParameter<float> midiAfterTouchesIntensity;
    ofParameter<float> midiAfterTouchesRef;
    vector<float> midiAfterTouches;
    
    ofParameter<int> midiCcNum1;
    ofParameter<int> midiCcOption1;
    ofParameter<float> midiCcIntensity1;
    ofParameter<float> midiCcRef1;
    float midiCc1;
    
    ofParameter<int> midiCcNum2;
    ofParameter<int> midiCcOption2;
    ofParameter<float> midiCcIntensity2;
    ofParameter<float> midiCcRef2;
    float midiCc2;
    
    void updateMidiList();
    void updateMidiNotes();
    void updateMidi();
    
    
    vector<float> midiPresences;
    vector<float> midiBrightnesses;
    vector<float> midiSaturations;
    vector<float> midiXs;
    vector<float> midiYs;
    
    ofxMidiOut midiOut;
    
    
    void drawMidi(float x, float y,  float w, float h);
    void drawMidiKeyboard(float x, float y,  float w, float h);
    
    void refreshMidiPortList();
    void allNotesOff();
    void sendMidi();
    void changeMidiPort(int p);
    
    vector<int> currentNotes;
    
    
    //-------------------------------SC_PANNEL
    
    emSCInterface sc; //boot server
    
    ofxOscSender scOscSender;
    
    ofParameter<string> scServerAddr;
    ofParameter<int> scServerPort;
    ofParameter<string> scInputName;
    ofParameter<string> scOutputName;
    ofParameter<int> scSamplerate;
    ofParameter<int> scMemSize;
    ofParameter<int> scBlockSize;
    ofParameter<int> scNumInputs;
    ofParameter<int> scNumOutputs;
    
    ofParameter<int> autoStartSc;
    void makeServer();
    void startSc();
    void reStartSc();
    void quitSc();
    bool scStarted = false;
    
    ofParameter<int> watchSc;
    
    ofParameter<int> iFFTbufNum;
    ofParameter<int> iFFTphasesBufNum;
    ofParameter<string> iFFTpatchName;

    void loadIFFTpatch();

    ofParameter<string> synthPatchName;
    
    ofParameter<int> synthFreqBufnum;
    ofParameter<int> synthGateBufnum;
    ofParameter<int> synthVolsBufnum;

    ofParameter<int> synthMod1BufNum;
    ofParameter<int> synthMod2BufNum;
    ofParameter<int> synthMod3BufNum;
    ofParameter<int> synthMod4BufNum;
    
    vector<ofParameter<string>> scSynthStaticsLabels;
    //statics : h1n, h2n, h3n, lfotype, lforate, A, D, S, R
    vector<ofParameter<string>> scSynthModsLabels;
    //mods : vel, h1mod, h2mod, h3mod, lfoMod
    
    void loadSynth();
    
    //-------------------------------SC
    
    //freqBuf, h1nBuf, h2nBuf, h3nBuf, h1pBuf, h2pBuf, h3pBuf, lfoTypeBuf, lofoRateBuf;
    //trigBuf, velBuf, h1modBuf, h2ModBuf, h3modBuf, lfoModBuf, volBuf;
    //aBuf, dBuf, sBuf, rBuf;
    
    
    ofParameter<int> scSynthOn;
        
    vector<ofParameter<float>> scFreqs;
    ofParameter<float> hNumber1;
    ofParameter<float> hNumber2;
    ofParameter<float> hNumber3;
    ofParameter<float> hPhase1;
    ofParameter<float> hPhase2;
    ofParameter<float> hPhase3;
    
    ofParameter<int> lfoOption;
    vector<string> lfoOptions = {"sin", "square", "saw", "tri", "rnd"};
    ofParameter<float> lfoRate;
    ofParameter<int> scLfoOption;
    ofParameter<float> scLfoIntensities;
    ofParameter<float> scLfoRefs;
    vector<float> lfoMods;
    
    ofParameter<int> scTriggerOption;
    vector<ofParameter<float>> scThresholds;
    vector<float> scTriggersBuf;
    vector<bool> scTriggers;
    
    ofParameter<int> scVelocitiesOption;
    ofParameter<float> scVelocitiesIntensities;
    ofParameter<float> scVelocitiesRefs;
    vector<float> scVelocities;
    
    ofParameter<int> scH1Option;
    ofParameter<float> scH1Intensities;
    ofParameter<float> scH1Refs;
    vector<float> h1Mods;
    
    ofParameter<int> scH2Option;
    ofParameter<float> scH2Intensities;
    ofParameter<float> scH2Refs;
    vector<float> h2Mods;
    
    
    vector<ofParameter<float>> scVolumes;
    
    ofParameter<float> scAtt;
    ofParameter<float> scDec;
    ofParameter<float> scSus;
    ofParameter<float> scRel;
    
    ofParameter<int> scMasterVolOption;
    ofParameter<float> scMasterVolIntensity;
    ofParameter<float> scMasterVolRef;
    float scMasterVolume;

   
    vector<float> scPresences;
    vector<float> scBrightnesses;
    vector<float> scSaturations;
    vector<float> scXs;
    vector<float> scYs;
    
    void updateSynth();
    void drawSC(float x, float y,  float w, float h);
    void drawSCkeyboard(float x, float y,  float w, float h);
    void sendOSCsynth();
    
    //------------------------------iFFT
    
    
    void updateIFFT();
    void sendOSCifft();
    float getVideoMod( int modOption );
    float getVideoDataSample( int option, int index );
    
    ofParameter<int> iFFTon;
    
    vector<string> magOptions =  {"Presences", "Brightnesses", "Saturations", "Pan", "Tilt"};
    ofParameter<int> iFFTmagOption;
    ofParameter<int> iFFTphaseOption;
    
    vector<float> mags;
    vector<float> phases;
    vector<float> pan;
    vector<float> tilt;
    
    
    ofParameter<int> iFFTampModOption;
    ofParameter<float> iFFTampModIntensity;
    ofParameter<float> iFFTampRef;
    float iFFTamp;
    
    ofParameter<int> iFFTloHiModOption;
    ofParameter<float> iFFTloHiModIntensity;
    ofParameter<float> iFFTloHiRef;
    float iFFTloHi;
    
    
    ofParameter<int> iFFThiShiftModOption;
    ofParameter<float> iFFThiShiftModIntensity;
    ofParameter<float> iFFThiShiftRef;
    float iFFThiShift;
    
    ofParameter<int> iFFTwhiteMorphModOption;
    ofParameter<float> iFFTwhiteMorphModIntensity;
    ofParameter<float> iFFTwhiteMorphRef;
    float iFFTwhiteMorph;
    
    ofParameter<int> iFFTconvoMixModOption;
    ofParameter<float> iFFTconvoMixModIntensity;
    ofParameter<float> iFFTconvoMixRef;
    float iFFTconvoMix;
    
    
    void videoUnaryOptionSelector(int option);
    void videoArrayOptionSelector(int option);
    void videoArrayMidiOptionSelector(int option);
    void videoArrayScOptionSelector(int option);
    
    void drawIFFT(float x, float y, float w, float h);
    
    
	//-------------------------------------VIDEO DATA SETTINGS

	ofParameter<int> skipPix;

	
	ofParameter<float> rotation;
	ofParameter<float> histoCont;
	ofParameter<float> histoAmp;
	ofParameter<float> histoSmooth;

	ofParameter<float> luSatCont;
	ofParameter<float> luSatAmp;
	ofParameter<float> luSatSmooth;

	ofParameter<float> luCont;
	ofParameter<float> luAmp;
	ofParameter<float> luSmooth;

	ofParameter<float> satCont;
	ofParameter<float> satAmp;
	ofParameter<float> satSmooth;

	ofParameter<float> xyCont;
	ofParameter<float> xyAmp;
	ofParameter<float> xySmooth;
    
    ofParameter<float> panCont;
    ofParameter<float> panAmp;
    ofParameter<float> panSmooth;
    
    ofParameter<float> tiltCont;
    ofParameter<float> tiltAmp;
    ofParameter<float> tiltSmooth;

	ofParameter<float> rvbcmjnCont;
	ofParameter<float> rvbcmjnAmp;
	ofParameter<float> rvbcmjnSmooth;
    
    ofParameter<float> veloCont;
    ofParameter<float> veloAmp;
    ofParameter<float> veloSmooth;
    ofParameter<float> anglesCont;
    ofParameter<float> anglesAmp;
    ofParameter<float> anglesSmooth;
    
    ofParameter<int> blackThresh;
    ofParameter<float> blackCont;
    ofParameter<float> blackAmp;
    ofParameter<float> blackSmooth;
    
    ofParameter<int> neutralThresh;
    ofParameter<float> neutralCont;
    ofParameter<float> neutralAmp;
    ofParameter<float> neutralSmooth;
    
    ofParameter<int> whiteThresh;
    ofParameter<float> whiteCont;
    ofParameter<float> whiteAmp;
    ofParameter<float> whiteSmooth;
    
    ofParameter<float> lightnessCont;
    ofParameter<float> lightnessAmp;
    ofParameter<float> lightnessSmooth;
    
    ofParameter<float> averageHueSmooth;
    
    ofParameter<float> averageBrightnessAmp;
    ofParameter<float> averageBrightnessSmooth;
    
    ofParameter<float> averageSaturationAmp;
    ofParameter<float> averageSaturationSmooth;
    
    float averageHue = 0.0f;
    float averageBrightness = 0.0f;
    float averageSaturation = 0.0f;
    
    
    
    
//////////////////////////////VIDEO DATA CONTAINER
    
    ofVideoGrabber audioVideoCamera;
    ofVideoGrabber videoCamera;
    
    void changeVideoCam(int n);
     
    void initCams();
    
    
    ofTexture blackTex;
    
    ofPixels analyzedPixels;
    
    float brightnessTest;

    
    ofTexture *videoTex;
    emVideoAnalyzer videoAnalyzer;
    
    ofParameter<int> analysisWidth;
    ofParameter<int> analysisHeight;
    
    vector<float> presences;
    vector<float> brightnesses;
    vector<float> saturations;
    vector<float> posXs;
    vector<float> posYs;
    
    vector<float> velocities;
    vector<float> angles;
    
    float black = 0.0f;
    float neutral = 0.0f;
    float white = 0.0f;
    
    float lightness = 0.0f;

    ofColor averageColor;
    
	float R = 0.0f;
	float V = 0.0f;
	float B = 0.0f;
    
	float L = 0.0f;
	float C = 0.0f;
	float M = 0.0f;
	float J = 0.0f;
	float N = 0.0f;
    


	int numThumbs;
	int thumbsCursor;
	int frameCounter;
	int modulo;
	ofPixels thumbs;

	vector <ofTexture> thumbsTex;

	ofPixels colorGraph;
	ofPixels lastColorGraph;
	ofPixels tempColorGraph;

	ofTexture colorGraphTex;


////////////////////////////VIDEO FUNCTIONS

	void setupVideo();
	void loadVideoPreset(string name);
	void saveVideoPreset(string name);
	void refreshVideoPresets();
    
    void getVideoData();
    
    void updateVideo();
    


////////////////////////////VIDEO DRAWING FUNCTION

	void drawSourceInfo(float x, float y, float w, float h);
	void drawSource(float x, float y, float w, float h);
	void drawProcessed(float x, float y, float w, float h);
	void drawThumbnails(float x, float y, float w, float h);;
	void drawHisto(float x, float y, float w, float h);
	void drawLus(float x, float y, float w, float h);
	void drawSats(float x, float y, float w, float h);
	void drawLuSats(float x, float y, float w, float h);
	void drawXyPositions(float x, float y, float w, float h);

	void drawColorGraph(float x, float y, float w, float h);

	void frame(float x, float y, float width, float height, float stroke, ofColor color, bool fill);
	void line(float x1, float y1, float x2, float y2, float stroke, ofColor color);
	void circle(float x, float y, float radius, float stroke, ofColor color, bool fill);
    
    ofFbo videoFbo;
    
	////////////////////////////VIDEO GUI

    void drawImGuiVideo();
    bool showVideoSaveAs;
    

    vector<string> videoPresetsNames = {"empty", "empty", "empty"};

	ofParameter<string> videoPresetName;
    ofParameter<int> videoPresetNumber;

	ofxOscSender videoOscSender;

	void sendVideoOscData();

//////////////////////////////////////////////////////////////////////////////////////////

	//AUTOMATIONS

//////////////////////////////////////////////////////////////////////////////////////////

	ofParameter<int> autoPlay;

	ofParameter<int> autoRec;
    
    ofParameter<int> toggleFullScreen;


	ofParameter<bool> playing;

	ofParameter<bool> drawing;

	ofParameter<bool> drawingTech;
	ofParameter<bool> drawingPdf;


	//TRANSPORT
	ofParameter<float> movieStartPos;
	ofParameter<float> movieEndPos;
	ofParameter<bool> movieLoop;
	ofParameter<float> moviePos;
    
	ofParameter<string> automationsPresetName;


	//Cam
	string cameraName;
	ofXml cameras;
	vector<ofVideoDevice> camList;
    vector<string> camNames;
    
    ofParameter<string> vCamName;
    ofParameter<string> avCamName;
    
    ofParameter<int> autoStartVcam;
 
	//Files
	string fileName;
    
    ofParameter<int> movieNum = 0;

	vector<ofVideoPlayer> moviePlayer;
    //vector<ofxHAPAVPlayer> moviePlayer;

	vector<string> playlist;


	bool fileRequested;
	bool fileReady;
	bool loadFail;

	int nextiD;


	//Streams

	vector<string> streamsList;
	vector<string> streamsNames;
	vector<string> streamsUrls;

	int streamIndex;

	ofParameter<int> recTime;
	ofParameter<int> autoRecTime;
    ofParameter<int> recHAP;

    
	void setupAutomations();
	void loadAutomations();
	void saveAutomations();
    void drawImGuiAutomation();

	void refreshPlaylist();
	void refreshStreamsList();
	void refresh();


	void loadDefaultGuiParameters();
	void loadGuiParameters(string path);
	void saveGuiParameters(string path);


	void changeFile(int iD);
	void changeFileAuto();
	void updateFile();

	void changeStream(int iD);



	void play();
	void pause();
	void stop();


	bool isFiles = false;


//////////////////////////////////////////////////////////////////////////////////////////
    
    

	ofParameter<string> scPatchName;
	ofParameter<bool> startSC;
	ofParameter<bool> keepSCalive;


	ofParameter<string> workingDir;


	bool newFrame;

	string debug;

	string startTime;

	int movieTimeEllapsed;

	float movieTempo;

	int timeFlag;

	void print();

	void recStream(int iD);

	void checkWorkingDir();
	void checkStreamsList();

	ofParameter<int> keepErrorFile;

	/////////////////////////////////////TEMPORISATION SETTINGS
    
    string appState = "Startup";
    
    int playingMovie = 0;
    
	ofParameterGroup tempoSettings;
    

	ofParameter<float> zapDurationMin;
	ofParameter<float> zapDurationMax;

	emMovieManager manager;

	void transferManagerSettings();
    string removeMessage = "startup";
	ofParameter<int> randomMovie;

	ofParameter<int> maxNumFiles;
	ofParameter<int> minNumFiles;
    
    ofParameter<int> streamNum = 0;
    
	ofParameter<int> recDuration;
	ofParameter<int> interRecTime;

	ofParameter<int> maxArchive;
    
    
    //tests
	ofParameter<float> minRecDuration;
	ofParameter<int> maxLoadTime;
	ofParameter<int> maxFrameLate;
	ofParameter<int> firstFrameLate;

	ofParameter<int> startRecHour;
	ofParameter<int> stopRecHour;


	ofParameter<int> autoPrint;
	ofParameter<int> interPrintTime;
	ofParameter<float> printTimer;

	ofParameter<bool> drawTech;

	int playerSelector;


	bool nextMovieLoaded;




	void detectError();
    
    void testSound();
    
	ofParameter<float> silenceTimer;
	ofParameter<float> blackTimer;
	long sTimer;
	int bTimer;
	int blackTestValue;

	void eraseMovie( string path, string message );


	bool managerChange;

	bool removeRequested;

	string movieToRemovePath;

	bool refreshRequested;

	vector<string> playNames;

	string recorderState;
	string currentStream;

	bool showAutomations = false;


//////////////////////////////////////////////////////////////////////////////////////////

	//PRINT STUFF

//////////////////////////////////////////////////////////////////////////////////////////

	int memoryLooper;

	ofPixels memoryHisto;

	ofParameter<string> printPresetName;


/////////////////////////////////////////////////////////////////////////SYPHON STUFF
    
    ofParameter<int> HAPmode = 0;
    
    emRemoteHapPlayer hap;
    bool hapStarted = false;
    
    void startHap();
    void restartHap( );
    void endHap();

	ofxSyphonServer videoServer_1;
	ofxSyphonServer videoServer_2;
	ofxSyphonClient syphonClient;

	vector<string> syphonServerOptions = {"none", "Camera", "Stream", "Sound_Gradient_Texture"};

	ofParameter<string> syphonInputName;
	ofParameter<string> syphonInputApp;


	void sendVideoToSyphon();


//////////////////////////////////////////////////////////////////////////////////////////

	//AUDIO STUFF

//////////////////////////////////////////////////////////////////////////////////////////




	/*********************************************AUDIO SETTINGS*/

	string audioPresetPath;


	//vector<string> audioDeviceList;
    
    vector<string> audioInputDeviceList;
    vector<int> audioInputDeviceIDs;
    
    vector<string> audioOutputDeviceList;
    vector<int> audioOutputDeviceIDs;


	ofParameter<string> audioInputDeviceName;
    ofParameter<string> audioExtInputDeviceName;
	ofParameter<string> audioOutputDeviceName;
    
	ofParameter<int> audioInputDevice;
    ofParameter<int> audioExtInputDevice;
	ofParameter<int> audioOutputDevice;

	ofParameter<int> sampleRate;
	ofParameter<int> bufferSize;
	ofParameter<int> numBuf;

	vector<string> sampleRateOptions = {"44100", "48000"};
	vector<string> bufferSizeOptions = {"512", "1024", "2048"};
	vector<string> numBufOptions = {"2", "4", "8"};

	int _sampleRate;
	int _bufferSize;
	int _numBuf;



	ofParameter<int> autoStartAudio;
    
    ofParameter<int> audioVideoInputOption;
    vector<string> audioVideoInputOptions = {"Movies", "USB", "Syphon", "None"};
    ofParameter<string> audioVideoInputName;
    
    ofParameter<int> audioVideoUsbInputOption;
    
    ofVideoGrabber* audioVideoCam;
    void changeAudioVideoCam(int n);

    
    void audioVideoInputOptionSelector(int option);
    void videoInputOptionSelector(int option);
    int lastVcam;
    int lastAcam;
    int lastVsource;
    int lastAsource;
    
	ofxOscSender audioOscSender;
	ofParameter<string> oscAudioDataOutputAddr;
	ofParameter<int> oscAudioDataOutputPort;

	ofParameter<bool> sendAudioOsc;

    ofParameter<bool> swap;

	ofParameter<int> audioAnalyseOn;
	bool audioAnalyseIsOn;

	ofParameter<int> audioProcessingOn;
	bool audioProcessingIsOn;

    
    vector<string> onOffOptions = {"Off", "On"};
    
	/*********************************************AUDIO DATA CONTAINERS*/

	ofSoundBuffer audioDataBuffer;


	float sampleLeft;
	float sampleRight;

	vector<float> soundGraphPointsL;
	vector<float> soundGraphPointsR;

	ofPixels soundGraph;
	ofPixels lastSoundGraph;
	ofPixels tempSoundGraph;

	ofTexture soundGraphTex;

	ofPixels soundGradientPix;
	ofTexture soundGradientTex;
    
    ofxAudioAnalyzer audioAnalyzerLeft;
    ofxAudioAnalyzer audioAnalyzerRight;
    
    
    ofFbo audioScreen;
    ofFbo videoScreen;
    
	float rms;
	float power;
	float pitchFreq;
	float pitchFreqNorm;
	float pitchConf;
	float pitchSalience;
	float hfc;
	float hfcNorm;
	float specComp;
	float specCompNorm;
	float centroid;
	float centroidNorm;
	float inharmonicity;
	float dissonance;
	float rollOff;
	float rollOffNorm;
	float oddToEven;
	float oddToEvenNorm;
	float strongPeak;
	float strongPeakNorm;
	float strongDecay;
	float strongDecayNorm;

	vector<float> spectrum;
    vector<float> spectrumNorm;
	vector<float> melBands;
    vector<float> melBandsNorm;
	vector<float> mfcc;
    vector<float> mfccNorm;
	vector<float> hpcp;
    vector<float> hpcpNorm;
	vector<float> tristimulus;
    vector<float> tristimulusNorm;
	vector<float> multiPitches;
    vector<SalienceFunctionPeak> saliencePeaks;
    vector<float> multiPitchesSaliences;
	bool isOnset;
    
    vector<float> zeroFloats;


	float newrms;
	float midrms;
	float nextrms;
    float testRms;
    
    float rms2;
    float power2;
    float pitchFreq2;
    float pitchFreqNorm2;
    float pitchConf2;
    float pitchSalience2;
    float hfc2;
    float hfcNorm2;
    float specComp2;
    float specCompNorm2;
    float centroid2;
    float centroidNorm2;
    float inharmonicity2;
    float dissonance2;
    float rollOff2;
    float rollOffNorm2;
    float oddToEven2;
    float oddToEvenNorm2;
    float strongPeak2;
    float strongPeakNorm2;
    float strongDecay2;
    float strongDecayNorm2;
    
    vector<float> spectrum2;
    vector<float> spectrumNorm2;
    vector<float> melBands2;
    vector<float> melBandsNorm2;
    vector<float> mfcc2;
    vector<float> mfccNorm2;
    vector<float> hpcp2;
    vector<float> hpcpNorm2;
    vector<float> tristimulus2;
    vector<float> tristimulusNorm2;
    vector<float> multiPitches2;
    vector<SalienceFunctionPeak> saliencePeaks2;
    vector<float> multiPitchesSaliences2;
    bool isOnset2;


	vector<float> memoryRMS;
	ofPixels memoryFFT;

	ofPixels fftColors;

	ofParameter<int> fftMapMin = 0;
	ofParameter<int> fftMapMax = 512;
	ofParameter<float> colorMapMin = 0.0f;
	ofParameter<float> colorMapMax = 1.0f;


	/*********************************************AUDIO PARAMETERS*/


    
    ofParameter<float> volume;



	ofParameter<float> rmsAmp;
	ofParameter<float> rmsSmooth;
    
    
	ofParameter<float> powerSmooth;
	ofParameter<float> pitchFreqSmooth;
	ofParameter<float> pitchConfSmooth;
	ofParameter<float> pitchSalienceSmooth;
	ofParameter<float> hfcSmooth;
	ofParameter<float> specCompSmooth;
	ofParameter<float> centroidSmooth;
	ofParameter<float> inharmonicitySmooth;
	ofParameter<float> dissonanceSmooth;
	ofParameter<float> rollOffSmooth;
	ofParameter<float> oddToEvenSmooth;
	ofParameter<float> strongPeakSmooth;
	ofParameter<float> strongDecaySmooth;
    ofParameter<float> strongPeakAmp;


    ofParameter<float> spectrumSmooth;
    ofParameter<float> melBandsSmooth;
    ofParameter<float> mfccSmooth;
    ofParameter<float> hpcpSmooth;
    ofParameter<float> tristimulusSmooth;
    ofParameter<float> multiPitchesSmooth;
    
    
    //PLUGGINS
    
//    //EQ
//    ofParameter<float> lowcut_0_1;
//    ofParameter<float> highcut_0_2;
//    ofParameter<float> lowgain_0_3;
//    ofParameter<float> midgain_0_4;
//    ofParameter<float> highgain_0_5;
//    float _lowcut_0_1;
//    float _highcut_0_2;
//    float _lowgain_0_3;
//    float _midgain_0_4;
//    float _highgain_0_5;
//    //COMP
//    
//    //PITCH
    ofParameter<float> pitchCompensation;
//    ofParameter<float> pitchshift_2_0;
//    ofParameter<float> tuning_2_8 = 0;
    
    
    float _pitchCompensation;
    float _pitchshift_2_0;
    

    // AUDIOGUI
    void drawImGuiAudio();
    bool showAudioSaveAs = false;

    
    void imGuiAudioUnarySelector(int option);
    void imGuiAudioArraySelector(int option);
    


	/*********************************************AUDIO TRANSFORMATION PARAMETERS*/


	/*********************************************AUDIO FUNCTIONS*/
    
    string currentAudioOutput;

	void setupAudio();

	void loadAudioPreset(string name);
    void loadAudioPresetsList();
	void saveAudioPreset(string name);
	void refreshAudioPresets();



	void updateAudioData();
    void interpolateAudioData();
    vector<float> i_presencesL1;
    vector<float> i_presencesL2;
    vector<float> i_brightnessesL;
    vector<float> i_saturationsL;
    vector<float> i_alphasL;
    
    vector<float> i_presencesR1;
    vector<float> i_presencesR2;
    vector<float> i_brightnessesR;
    vector<float> i_saturationsR;
    vector<float> i_alphasR;
    

    
    void selectAudioData();
    vector<float> * audioPresencesL1;
    vector<float> * audioPresencesL2;
    vector<float> * audioBrightnessesL;
    vector<float> * audioSaturationsL;
    vector<float> * audioAlphasL;
    
    vector<float> * audioPresencesR1;
    vector<float> * audioPresencesR2;
    vector<float> * audioBrightnessesR;
    vector<float> * audioSaturationsR;
    vector<float> * audioAlphasR;
    
    float  agPresence;
    float  agBrightness;
    float  agSaturation;
    float  agAlpha;
    float  agContrast;
    
    
    float gradientBrightness;
    float gradientSaturation;
    float gradientAlpha;
    float gradientContrast;
    
    
    ofPixels applyGradient();
    
  
    void analyzeSamplesLeft(vector<float> samplesLeft);
    void analyzeSamplesRight(vector<float> samplesRight);
    
    
    
	void recordAudioData();
	void drawAudioData(int x, int y, int width, int height);
	void sendAudioOscData();

	void getFFTColors();

	void drawFFT(float x, float y, float width, float height);
	void drawWaveForm(float x, float y, float width, float height);
	void drawRMS(float x, float y, float width, float height);
	void drawSoundGraph(float x, float y, float width, float height);
	void drawSoundGraphPoints(float x, float y, float width, float height);

	//AUDIO DEVICES MANAGEMENT
	void initAudioObjects();
	void refreshAudioDeviceList();

    
    //AUDIOUNIT STUFF
    
    void startAudioUnits(int input, int extinput, int output);
    void restartAudioUnits(int input, int extinput, int output);
    void stopAudioUnits();
    
    void loadAudioUnitsPresets(string pName);
    void saveAudioUnitsPresets(string pName);
    
    ofParameter<float> extInputVol;
    
    ofxAudioUnitInput input;
    ofxAudioUnitTap inputTap;
    
    ofxAudioUnitInput extInput;
    
    ofxAudioUnitMixer inputMixer;
    
    ofxAudioUnit hpf;
    ofxAudioUnit lpf;
    ofxAudioUnit pitchshift;
    ofxAudioUnit compressor;
    
    ofxAudioUnitTap outputTap;
    ofxAudioUnitOutput output;
    
    

	/*********************************************AUDIO GUI*/

	ofParameter<ofColor> backgroundAudioColor;
	ofParameter<ofColor> subAudioColor;
	ofParameter<ofColor> audioColor;
	ofParameter<ofColor> overAudioColor;
	ofParameter<ofColor> audioFontColor;

	/*********************************************AUDIO DAT GUI*/



	vector<string> audioPresetsNames = {"empty", "empty", "empty"};
    ofParameter<int> audioPresetNumber;
	ofParameter<string> audioPresetName;

	string presetsPath;
    
    
    ofParameter<int> blendMode;
    vector<string> blendModeOptions = {"Normal", "Lighten", "Darken", "Multiply", "Average", "Add", "Substract", "Difference","Negation", "Exclusion", "Screen", "Overlay", "SoftLight", "HardLight", "ColorDodge", "ColorBurn", "LinearDodge", "LinearBurn", "LinearLight", "VividLight", "PinLight", "HardMix", "Reflect", "Glow", "Phoenix", "BlendPhoenix", "RGB"};
 
    ofShader avBCSA;
    ofShader avBlend;
    
    ofParameter<float> audioVideoBrightness;
    ofParameter<float> audioVideoContrast;
    ofParameter<float> audioVideoSaturation;
    ofParameter<float> audioVideoAlpha;
    ofParameter<float> audioVideoZoom;
    ofParameter<float> audioVideoPositionX;
    ofParameter<float> audioVideoPositionY;
    ofParameter<float> audioVideoCrop;
    
    ofParameter<int> avBModOption;
    ofParameter<int> avCModOption;
    ofParameter<int> avSModOption;
    ofParameter<int> avAModOption;
    
    ofParameter<float> avBModIntensity;
    ofParameter<float> avCModIntensity;
    ofParameter<float> avSModIntensity;
    ofParameter<float> avAModIntensity;
    
    ofParameter<float> compensation;
    
    float avBMod = 1.0f;
    float avCMod = 1.0f;
    float avSMod = 1.0f;
    float avAMod = 1.0f;
    
    float avB = 1.0f;
    float avC =1.0f;
    float avS = 1.0f;
    float avA=1.0f;
    
    ofFbo audioVideoFbo;
    ofTexture *audioVideoTex;
    
    ofFbo gradientFbo;
    ofFbo broadcastFbo;
    
    void updateAudioVideo();

//////////////////////////////////////////////////////////////////////////////////////////

	//VIEW SWITCHING

//////////////////////////////////////////////////////////////////////////////////////////

	ofParameter<string> viewName;
	ofParameter<int> viewNum;
	vector<string> viewOptions = {"Analyse Video", "Analyse Audio", "Broadcast", "Print"};

    
    ofParameter<int> audioGuiView;
    ofParameter<int> videoGuiView;
    ofParameter<int> broadcastGuiView;


	void changeView(string newView);
	void changeView(int newNumView);
	void drawVideo();
	void drawAudio();
	void drawPrint();
	void drawBroadcast(float x, float y, float width, float height);


    void setGrayStyle();

//////////////////////////////////////////////////////////////////////////////////////////

	//SPECTRUM STUFF

//////////////////////////////////////////////////////////////////////////////////////////



    ofPixels getSpectrum(size_t size, bool bicubic);
	ofPixels colorSpectrum;
    ofTexture colorSpectrumTex;
	vector<float> getAudioFrequencies(size_t size);
    
    
    void drawSoundSpectrum(float x, float y, float w, float h);
    
  
//COLOR_TO_SOUND//////////////////////////////////////////////////////////////////////////

	vector<double> getHuesToSpectrum(ofPixels spectrum);
    vector<double> huesIndexes;

	vector<float> getColorFrequencies(size_t size);
	vector<float> colorFrequencies;

    void frequencyToNote(float frequency, int& octave, int& note, float& cents);
    
	float cpsMidi(float f);
    float midiCps(int midiNote);




//SOUND_TO_COLOR//////////////////////////////////////////////////////////////////////////

	vector<float> getHueSpectrum(ofPixels spectrum);
	vector<float> lookupHues;

	vector<float> generateMidiNotes( vector<float> frequencies);
	vector<float> lookUpMidiNotes;

	ofPoint midiToColor_octaver(int note, int velocity);


	vector<string> globalOptions =  {"POWER", "PITCH_FREQ", "PITCH_SALIENCE",
	                                 "HFC", "SPECTRAL_COMPLEXITY", "CENTROID", "DISSONANCE", "ROLL_OFF", "ODD_TO_EVEN", "STRONG_PEAKS", "STRONG_DECAY", "NONE"
	                                };

	vector<string> perBandOptions = {
		"SPECTRUM", "MEL_BANDS", "MFCC", "HPCP", "TRISTIMLUS", "NONE"
	};
    
    vector<float> agGlobalModders;
    
    
    ofPixels audioGram;
    ofTexture audioGramTex;
    
    ofParameter<int> aStartFreq;
    ofParameter<int> aEndFreq;
    ofParameter<int> cStartFreq;
    ofParameter<int> cEndFreq;
    ofParameter<int> mStartBand;
    ofParameter<int> mEndBand;
    
    ofParameter<float> refRotation;
    ofParameter<float> refBrightness;
    ofParameter<float> refSaturation;
    ofParameter<float> refAlpha;
    ofParameter<float> refContrast;

	ofParameter<int> globalRotation;
	ofParameter<int> globalBrightness;
	ofParameter<int> globalSaturation;
	ofParameter<int> globalAlpha;
    ofParameter<int> globalContrast;

	ofParameter<float> globalRotationAdjust;
	ofParameter<float> globalSaturationAdjust;
	ofParameter<float> globalBrightnessAdjust;
	ofParameter<float> globalAlphaAdjust;
    ofParameter<float> globalContrastAdjust;

	ofParameter<int> perBandPresence;
    ofParameter<int> perBandPresence2;
	ofParameter<int> perBandRotation;
	ofParameter<int> perBandSaturation;
	ofParameter<int> perBandBrightness;
	ofParameter<int> perBandAlpha;

	ofParameter<float> perBandPresenceAdjust;
	ofParameter<float> perBandRotationAdjust;
	ofParameter<float> perBandSaturationAdjust;
	ofParameter<float> perBandBrightnessAdjust;
	ofParameter<float> perBandAlphaAdjust;
    
    ofParameter<int> invertGradient;
    
    ofParameter<float> audioVideoMix;

    ofParameter<int> stereoGradient;
    vector<string> stereoOptions = {"Mono", "Stereo"};
    
    vector<ofVec4f> colorPoints;
    vector<ofVec4f> colorPoints2;
    
	ofPixels doGradient(int fMin, int fMax, int cMin, int cMax, int mMin, int mMax);
    ofPixels doGradientStereo(int fMin, int fMax, int cMin, int cMax, int mMin, int mMax);
    
    void drawPresences(int x, int y, int w, int h);
    void drawHues(int x, int y, int w, int h);
    void drawBrightnesses(int x, int y, int w, int h);
    void drawSaturations(int x, int y, int w, int h);
    void drawAlphas(int x, int y, int w, int h);
    
    
    void drawPresencesChoice(int x, int y, int w, int h);
    void drawBrightnessesChoice(int x, int y, int w, int h);
    void drawSaturationsChoice(int x, int y, int w, int h);
    void drawAlphasChoice(int x, int y, int w, int h);
    
    vector<float> interpolate( vector<float> input, int startI, int endI, int startO, int endO);
    

//END ofApp
//////////////////////////////////////////////////////////////////////////////////////////
};



