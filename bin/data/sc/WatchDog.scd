
/*-------------------ENVIRONMENT---------------------*/
/*---------------------------------------------------*/

~appPath = PathName(thisProcess.nowExecutingPath).pathOnly;


~report = {arg addr, str; var now;
	now = Date.getDate.format("%H:%M:%S");
	addr.sendMsg("/fromInstallation", (now++" : "++str) );
};

~logPath = Date.getDate.format(~appPath++"/logs/%d-%m-%Y__%H:%M:%S.txt");

~log = {arg str; var now;
	now = Date.getDate.format("%H:%M:%S");
	("echo \""++now++"\n"++str++"\n"++"\" >> "++~logPath).unixCmd;
};



/*-------counters-------*/

~counter = -60;
~inCount = 0;


/*-------patch boot------*/

~boot = {
	if ( s.serverRunning == false, {
		File.use((~appPath++"Patch.scd"),"r", {
			arg f; var str;
			str = f.readAllString;
			str.interpret;
		});
	});
};





/*------------------------CUSTOM---------------------*/
/*---------------------------------------------------*/
~tempo = 300; //seconds


/*-------debug adress------*/
//address to send report via OSC for status

//~mortecAddr = NetAddr.new("192.168.0.13", 57120);

~mortecAddr = NetAddr.new("127.0.0.1", 57120);



/*-------restart funcs------*/

~restartPatch = {
	s.freeAll;
	1.wait;
	s.quit;
	1.wait;
	"killall scsynth".systemCmd;
	1.wait;
	{Window.allWindows.do({arg i; i.close})}.defer;
	1.wait;
	~boot.value();
};

//~restartMaxFunc = { do the stuff here};



/*-------dog client settings------*/

~dogClients = List.new;

~dogClients.add( ['/scSynth', -60, "booting", ~restartPatch] );
//~dogClients.add( ['/max', -60, "booting", ~restartMaxFunc] );



/*-----communication------*/
~fromAnalyse = OSCFunc({s.freeAll;  s.quit; 0.exit; }, '/EXIT', recvPort:57120);


~dogClients.do({arg item, i;
	OSCFunc({~dogClients[i][1]=0}, item[0], recvPort:57120);
});


/*------------------------SETUP--------------------------*/
/*-------------------------------------------------------*/

~logtempo = 10;

~report.(~mortecAddr, "Starting Server, setting up synths.");
~log.("---------------------------------------\n Starting Server, loading up synths... \n---------------------------------------\n\n");

~boot.value;

~report.(~mortecAddr, "Starting SkipJacks");
~log.("/--------Starting SkipJacks.--------/\n\n");




//________________________SkipJack

~timer = 0;

SkipJack({

	~dogClients.do({
		arg item, i;
		var name, flag, state, restartFunc;

		name = item[0].asString;
		flag = item[1]; state = item[2]; restartFunc = item[3];

		if ( flag < 0, {
			item[2] = "booting";
			~logtempo = 10;
		});

		if ( (flag == 0) || (flag == 1), {
			item[2] = "beating";
			~logtempo = ~tempo;
		});

		if ( (flag > 1) && (flag < 30), {
			item[2] = "late";
			~logtempo = 10;
		});

		if ( flag >= 30, {
			item[2] = "frozen";
			restartFunc.value();
			~dogClients[i][1] = -60;
			~report.(~mortecAddr, name++" has frozen, restarting...");
			~log.("----------------------------------------------\n/***/ "++name++" has frozen, restarting / /***/\n----------------------------------------------\n\n");
			~logtempo = 10;
		});

		item[1] = item[1] + 1;

	});

	"FLAG".setenv("0");

	~timer = ~timer + 1;

	if ( ~timer >= ~logtempo, {
		~dogClients.do({
			arg item, i;
			var name, state;
			name = item[0].asString; state = item[2];
			~report.(~mortecAddr, name++" is "++state);
			~log.(name++" is "++state);
		});

		~timer = 0;
	});



},1, name: "watchDog", clock: SystemClock.sched(1););



//________________________end skipjack






/*-------------------for checkSClang script---------------------*/

/*
"FLAG".setenv("0");

( (~appPath++"./checkSClang.sh &").unixCmd ).postln;*/




/*-------------------LAST REPORT---------------------*/


~report.(~mortecAddr, "WhatchDog complete, Skipjack is running.");

~log.("----------------------------------------\nWhatchDog complete, SkipJack is running.\n----------------------------------------\n\n");


