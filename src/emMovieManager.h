#pragma once

#include "ofMain.h"

class emMovieManager: public ofThread {



//--------------------------------------------------------------------------------------------------------

////////////////////////////////////////////PUBLIC

//--------------------------------------------------------------------------------------------------------

public :

	string state = "Stopped";

	string currentStream = "";
    
    bool recHAP = false;

//----------------------------------------------------


	void setup(string workPath, string dataPath, string streamsListPath) {


		this->recPath = workPath + "Record/";
		this->playPath = workPath + "Play/";
		this->rescuePath = workPath + "Rescue/";
		this->errorPath = workPath + "Error/";
		this->archivePath = workPath + "Archive/";
		this->dataPath = dataPath;
		this->pathToStreamsListFile = streamsListPath;


		ofLogNotice() << "[:::MOVIE MANAGER:::] : SETUP";
		ofLogNotice() << "[:::MOVIE MANAGER:::] : Rec Path : " << recPath;
		ofLogNotice() << "[:::MOVIE MANAGER:::] : Play Path : " << playPath;
		ofLogNotice() << "[:::MOVIE MANAGER:::] : Rescue Path : " << rescuePath;
		ofLogNotice() << "[:::MOVIE MANAGER:::] : Error Path : " << errorPath;
		ofLogNotice() << "[:::MOVIE MANAGER:::] : Archive Path : " << archivePath;

        
		refreshStreamsList();

        
		recordRequested = true;
		removeRequested = false;
		streamSelector = 0;
		name = streamsList.at(streamSelector);
		link = streamsList.at(streamSelector + 1);
		recTimer = 0;
        
	}

//----------------------------------------------------

	void setStreams( string pathToStreamsFile) {

		pathToStreamsListFile = pathToStreamsFile;

		ofLogNotice() << "[:::MOVIE MANAGER:::] : Streams : " << ofToString(pathToStreamsListFile);
	}
    
    void setRecHAP( bool rechap) {
        
        recHAP = rechap;
        
        ofLogNotice() << "[:::MOVIE MANAGER:::] : HAP mode : " << ofToString(recHAP);
    }
    
    

	void setDuration(int dur) {

		this-> duration = dur;

		ofLogNotice() << "[:::MOVIE MANAGER:::] : Duration : " << ofToString(duration);


	}

	void setMaxLoadTime(int dur) {

		this-> maxLoadTime = dur;

		ofLogNotice() << "[:::MOVIE MANAGER:::] : maxLoadTime : " << ofToString(maxLoadTime);


	}

	void setMinDuration(float dur) {

		this-> minDuration = dur;

		ofLogNotice() << "[:::MOVIE MANAGER:::] : minDuration : " << ofToString(minDuration);


	}

	void setMaxFrameLate(int dur) {

		this-> maxFrameLate = dur;

		ofLogNotice() << "[:::MOVIE MANAGER:::] : frameLate : " << ofToString(maxFrameLate);


	}


	void setFirstFrameLate(int dur) {

		this-> firstFrameLate = dur;

		ofLogNotice() << "[:::MOVIE MANAGER:::] : First frameLate : " << ofToString(firstFrameLate);


	}

	void setInterRec(int inter) {

		this-> interRecTime = inter * 60;

		ofLogNotice() << "[:::MOVIE MANAGER:::] : Inter Record Time : " << ofToString(interRecTime);


	}

	void setMaxNumFiles(int max) {

		this-> maxNumFiles = max;

		ofLogNotice() << "[:::MOVIE MANAGER::] : Max num Files : " << ofToString(maxNumFiles);


	}

	void setMaxNumArchive(int max) {

		this-> maxNumArchive = max;

		ofLogNotice() << "[:::MOVIE MANAGER:::] : Max num archived files : " << ofToString(maxNumArchive);


	}

	void keepErrorfile(bool keep) {

		this -> keepError = keep;

		ofLogNotice() << "[:::MOVIE MANAGER:::] : Keep error files  : " << ofToString(keepError);
	}



	void removeRequest(string pathToFile, string message) {

		removeRequested = true;

		movieToRemovePath = pathToFile;

		ofLogNotice() << "[:::MOVIE MANAGER:::] : remove request from MAIN  : " << movieToRemovePath;
        ofLogNotice() << "[:::MOVIE MANAGER:::] :  " << message;
	}


	void archiveRequest(string pathToFile) {

		archiveRequested = true;

		movieToArchivePath = pathToFile;

		ofLogNotice() << "[:::MOVIE MANAGER:::] : archiveRequested  : " << movieToArchivePath;
	}



//----------------------------------------------------

	void runRec() {

		startThread();
	}

//----------------------------------------------------

	void stop() {
		//waitForThread();
		state = "Stopped";
		currentStream = "";
		stopThread();
	}




//--------------------------------------------------------------------------------------------------------

////////////////////////////////////////////PRIVATE

//--------------------------------------------------------------------------------------------------------
private :

	bool keepError;
	float minDuration;
	int maxLoadTime;
	int maxFrameLate;
	int firstFrameLate;
	bool first;
	int duration;
	float interRecTime;
	int maxNumFiles;
	int maxNumArchive;

	bool recordRequested;

	bool removeRequested;

	bool archiveRequested;

	ofVideoPlayer movie;

	string errorPath;
	string recPath;
	string playPath;
	string rescuePath;
	string archivePath;

	string dataPath;

	string movieToRemovePath;
	string movieToArchivePath;


	string pathToStreamsListFile;

	vector<string> streamsList;

	string name;
	string link;

	int streamSelector = 0;

	int recTimer = 0;

	int TESTLOAD = 0;
	int TESTDUR = 1;
	int TESTFRAME = 2;
	int BAD = 3;
	int GOOD = 4;

//----------------------------------------------------


	void threadedFunction() {

		ofLogNotice() << "[:::MOVIE MANAGER:::] : STARTING THREAD : " << "TIMER = " + ofToString(recTimer);

		while (isThreadRunning()) {

			//ofLogNotice() << "[:::MOVIE MANAGER:::] : THREAD RUNNING..." << "TIMER = "+ofToString(recTimer);
			//-------------------

			if (removeRequested) {

				string message = "[:::MOVIE MANAGER:::] : Remove requested from Main : SOUND ISSUE OR BLACK FRAME";

				eraseMovie(movieToRemovePath, message);

				removeRequested = false;
			}


			if ( (recTimer >= interRecTime) && (recordRequested == false) ) {

				recordRequested = true;

			}


			//-------------------
            
			if (recordRequested) {
                
				name = streamsList.at(streamSelector);
				link = streamsList.at(streamSelector + 1);


				string moviePath = record(name, link);

				if ( !recHAP ) testMovie(moviePath);
                else {
                    movePlay(moviePath);
                    recordRequested = false;
                }
                
				refresh();

				streamSelector = (streamSelector + 2) % ( streamsList.size() );

				recTimer = -1;

			}

			recTimer += 1;

			lock();
			state = "Sleeping";
			currentStream = "";
			unlock();

			sleep(1000);
		}
	}



//--------------------------------------------------------------


	void refreshStreamsList() {

		ofLogNotice() << "[:::MOVIE MANAGER:::] : update Streams list";

		streamsList.clear();

		ofBuffer buffer = ofBufferFromFile(pathToStreamsListFile );

		for (auto line : buffer.getLines() ) {
			streamsList.push_back(line);
		}


		ofLogNotice() << "[:::::MOVIE MANAGER:::::] : File list : " << pathToStreamsListFile;
		//ofLogNotice() << ofToString( streamsList );


		ofXml streams;

		streams.addChild("STREAMS");

		for (int i = 0; i < streamsList.size(); i += 2 ) {

			//ofLogNotice() << "Listed stream : " << streamsList.at(i);
			//ofLogNotice() << "Url : " << streamsList.at(i + 1) << "\n";

			ofXml temp;
			temp.addChild("Stream");
			temp.setAttribute("id", ofToString(i / 2) );
			temp.addValue("Name", streamsList.at(i) );
			temp.addValue("Url", streamsList.at(i + 1) );
			streams.addXml(temp);

		}

		streams.save("lists/Streams.xml");

	}



//----------------------------------------------------

	void refreshPlaylist() {

		ofLogNotice() << "[:::MOVIE MANAGER:::] : update Play list";

		ofDirectory dir( playPath );

		if (dir.exists()) {
            
            if (recHAP) dir.allowExt("MOV");
			else dir.allowExt("mp4");

			dir.listDir();

			vector<ofFile> files = dir.getFiles();

			sort (files.begin(), files.end(), sortFileDate );

			if (dir.size() > maxNumFiles) {

				for (int i = 0; i < ( files.size() - maxNumFiles ); i++) {

					moveRescue( files.at( i ) );

				}

			}

			ofXml playlist;
			playlist.addChild("PLAYLIST");

			dir.listDir();
			files.clear();
			files = dir.getFiles();

			for (int i = 0; i < files.size(); i++) {

				string path = files.at(i).getAbsolutePath();

				//ofLogNotice() << "File found : " << path;

				ofXml temp;
				temp.addChild("Movie");
				temp.setAttribute("id", ofToString(i) );
				temp.addValue("path", path );
				playlist.addXml(temp);

			}

			playlist.save("lists/Playlist.xml");

		}

		else {
			ofLogNotice() << "[:::MOVIE MANAGER:::] : PLAY DIRECTORY DOES NOT EXIST : CREATNG IT";

			ofDirectory::createDirectory( playPath, false, false);
		}


	}
//----------------------------------------------------

	void refreshRescuelist() {
		ofLogNotice() << "[:::MOVIE MANAGER:::] : update Rescue list";

		ofDirectory dir( rescuePath );

		if (dir.exists()) {

			//only show MOV & mp4 files
            if (recHAP) dir.allowExt("MOV");
            else dir.allowExt("mp4");

			//populate the directory object
			dir.listDir();

			vector<ofFile> files = dir.getFiles();

			sort (files.begin(), files.end(), sortFileDate);

			if (dir.size() > maxNumFiles) {

				for (int i = 0; i < (files.size() - maxNumFiles); i++) {

					moveArchive( files.at( i ) );

				}

			}

			ofXml rescuelist;

			rescuelist.addChild("RESCUE_LIST");

			dir.listDir();
			files.clear();
			files = dir.getFiles();

			//go through and print out all the paths + fill the xml file
			for (int i = 0; i < files.size(); i++) {

				string path = files.at(i).getAbsolutePath();

				//ofLogNotice() << "File found : " << path;

				ofXml temp;
				temp.addChild("Rescue_Movie");
				temp.setAttribute("id", ofToString(i) );
				temp.addValue("path", path );
				rescuelist.addXml(temp);

			}

			rescuelist.save("lists/Rescuelist.xml");

		}

		else {
			ofLogNotice() << "[:::MOVIE MANAGER:::] : RESCUE DIRECTORY DOES NOT EXIST : CREATNG IT";

			ofDirectory::createDirectory( rescuePath, false, false);
		}


	}


//----------------------------------------------------

	void refreshArchivelist() {

		ofLogNotice() << "[:::MOVIE MANAGER:::] : update Archive list";

		ofDirectory dir( archivePath );

		if (dir.exists()) {

			//only show MOV & mp4 files
            if (recHAP) dir.allowExt("MOV");
            else dir.allowExt("mp4");

			//populate the directory object
			dir.listDir();

			vector<ofFile> files = dir.getFiles();

			sort (files.begin(), files.end(), sortFileDate);

			if (dir.size() > maxNumArchive) {

				for (int i = 0; i < ( files.size() - maxNumArchive ); i++) {

					eraseArchive( files.at( i ) );

				}

			}

			ofXml archivelist;

			archivelist.addChild("ARCHIVE");

			dir.listDir();
			files.clear();
			files = dir.getFiles();
			//go through and print out all the paths + fill the xml file
			for (int i = 0; i < files.size(); i++) {

				string path = files.at(i).getAbsolutePath();

				//ofLogNotice() << "File found : " << path;

				ofXml temp;
				temp.addChild("Archived_Movie");
				temp.setAttribute("id", ofToString(i) );
				temp.addValue("path", path );
				archivelist.addXml(temp);

			}

			archivelist.save("lists/Archive.xml");

		}

		else {
			ofLogNotice() << "[:::MOVIE MANAGER:::] : ARCHIVE DIRECTORY DOES NOT EXIST : CREATNG IT";

			ofDirectory::createDirectory( archivePath, false, false);
		}


	}

//----------------------------------------------------

	void refresh() {

		refreshStreamsList();
		refreshPlaylist();
		refreshRescuelist();
		refreshArchivelist();

	}



//----------------------------------------------------


	void testMovie(string path) {
		lock();
		state = "Testing";
		unlock();

		ofFile movieFile(path);

		if (movieFile.exists() ) {

			string moviePath = path;

			ofLogNotice() << "[:::MOVIE MANAGER:::] : " << "Testing " << moviePath;

			string message = "pretest message";

			int timer = 0;
			int timeFlag = 0;

			int state = TESTLOAD;



			timeFlag = ofGetElapsedTimeMillis();
            

			//---------------LOAD TEST
			ofLogNotice() << "[:::MOVIE MANAGER:::] : TESTING LOADING";
            
            movie.close();
            movie = ofVideoPlayer();
            movie.setLoopState(OF_LOOP_NONE);            
            movie.setPixelFormat(OF_PIXELS_NATIVE);
            movie.setUseTexture(false);
            movie.load(moviePath);

			while ( state == TESTLOAD)  {

				timer = ofGetElapsedTimeMillis()  - timeFlag;

				if (timer >= maxLoadTime) {

					state  = BAD;

					message = "COULDNT LOAD THIS FILE";
				}

				else if ( movie.isLoaded() ) {

					ofLogNotice() << "[:::MOVIE MANAGER:::] : " << "Movie loaded : " << ofToString(timer);
					state = TESTDUR;
				}

			}


			//---------------DURATION TEST
			ofLogNotice() << "[:::MOVIE MANAGER:::] : TESTING DURATION";
			if ( state == TESTDUR ) {

				float dur = movie.getDuration() ;

				if ( dur < (minDuration * duration) ) {

					state = BAD;
					message = "TOO SHORT : " + ofToString(duration);
				}

				else {
					ofLogNotice() << "[:::MOVIE MANAGER:::] : " << "Movie duration OK : " << ofToString(duration);
					movie.play();
					movie.setVolume(0.0f);
					state = TESTFRAME;
					timeFlag = ofGetElapsedTimeMillis();
				}

			}


			//---------------FRAME TEST

			ofLogNotice() << "[:::MOVIE MANAGER:::] : TESTING FRAME";

			first = true;

			while ( state == TESTFRAME) {

				movie.update();

				if (first) {

					if ( movie.isFrameNew() ) {
						message = "FIRST FRAME : " + ofToString(ofGetElapsedTimeMillis() - timeFlag);
						timeFlag = ofGetElapsedTimeMillis();
						first = false;
					}
					
					else {

						timer = ofGetElapsedTimeMillis() - timeFlag;

						if (timer >= firstFrameLate) {

							message = "FIRST FRAME TOO LATE : " + ofToString(timer);
							state = BAD;

						}
					}
				}

				else {
					if ( movie.isFrameNew() ) {
						timeFlag = ofGetElapsedTimeMillis();
					}

					timer = ofGetElapsedTimeMillis() - timeFlag;

					if (timer >= maxFrameLate) {

						message = "FRAMES TOO SLOW : " + ofToString(timer);
						state = BAD;

					}


				}

				if (movie.getPosition() > 0.98f ) {
					ofLogNotice() << "[:::MOVIE MANAGER:::] : " << "Frame OK : " << ofToString(timer);
					state = GOOD;

					movie.stop();
				}

				sleep(40);

			}


			//---------------CONCLUSION

			if (state == GOOD) {


				movie.closeMovie();

				ofLogNotice() << "[:::MOVIE MANAGER:::] : " << moviePath << " : SUCCESS";



				recordRequested = false;

				//move to Play
				ofFile file(moviePath);

				movePlay(file);

			}

			if (state == BAD) {
				ofLogNotice() << "[:::MOVIE MANAGER:::] : " << moviePath << " : TEST FAILED";

				recordRequested = true;


				eraseMovie( moviePath, message );

			}

		}


		else {

			ofLogNotice() << "[:::MOVIE MANAGER:::] : " << "Couldn't load  movie : RECORDING FAILED";
		}
	}


//----------------------------------------------------

	string record(string name, string link) {

		lock();
		state = "Recording";
		currentStream = name;
		unlock();

		string date = ofGetTimestampString("%A_%d_%B_%Y__%HH%M-%Ss");

        string moviePath;
        
        if (recHAP) moviePath = recPath + name + "__" + date + ".MOV";
        else moviePath = recPath + name + "__" + date + ".mp4";

		string args = link + " " + moviePath + " " + ofToString(duration);
        
        
		string commandLine;
        
        if (recHAP) commandLine = dataPath + "scripts/" + "./streamRecHAP.sh " + args;
        
        else commandLine = dataPath + "scripts/" + "./streamRecStdb.sh " + args;

		ofLogNotice() << "[:::MOVIE MANAGER:::] : [::NEW:RECORDING::] : index " + ofToString(streamSelector);
		ofLogNotice() << commandLine;

		string commandReturn = ofSystem(commandLine);

		ofLogNotice() << commandReturn;

		//d=`date +%A"_"%d"_"%B"_"%Y"__"%H"H"%M"-"%S"s"`


		return moviePath;

	}



//----------------------------------------------------

	void eraseMovie(string path, string message) {

		ofFile file(path);
		ofLogNotice() << "[:::MOVIE MANAGER:::] : Erasing movie : " << file.getAbsolutePath();
		ofLogNotice() << "[:::MOVIE MANAGER:::] : ==> " << message;
		if (keepError) {
			string name = file.getFileName();
			file.moveTo(errorPath + name, false, true);
		} else {file.remove(); }

	}
//----------------------------------------------------

	void eraseArchive(ofFile file) {

		ofLogNotice() << "[:::MOVIE MANAGER:::] : Erasing movie from archive directory : " << file.getAbsolutePath();
		file.remove();

	}

//----------------------------------------------------

	void moveArchive(ofFile file) {

		ofLogNotice() << "[:::MOVIE MANAGER:::] : Moving movie to Archive : " << file.getAbsolutePath();
		string name = file.getFileName();
		file.moveTo(archivePath + name, false, true);
	}

//----------------------------------------------------

	void moveRescue(ofFile file) {

		ofLogNotice() << "[:::MOVIE MANAGER:::] : Moving movie to Rescue dir : " << file.getAbsolutePath();
		string name = file.getFileName();
		file.moveTo(rescuePath + name, false, true);
	}

//----------------------------------------------------

	void movePlay(ofFile file) {
		ofLogNotice() << "[:::MOVIE MANAGER:::] : Moving movie to Playlist : " << file.getAbsolutePath();

		string name = file.getFileName();
		file.moveTo(playPath + name, false, true);
	}
    
    void movePlay(string filePath) {
        ofLogNotice() << "[:::MOVIE MANAGER:::] : Moving movie to Playlist : " << filePath;
        ofFile file(filePath);
        
        string name = file.getFileName();
        file.moveTo(playPath + name, false, true);
    }
    
    



//----------------------------------------------------

	static bool sortFileDate (ofFile i, ofFile j) {

		std::time_t a = std::filesystem::last_write_time(i);
		std::time_t b = std::filesystem::last_write_time(j);

		return ( a < b );

	}

};


