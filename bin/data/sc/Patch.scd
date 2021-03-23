
~appPath = PathName(thisProcess.nowExecutingPath).pathOnly;

/*-------------------SERVER OPTIONS---------------------*/

~initS = {
	Server.default = s = Server.local;
	Server.default.options.inDevice= "Soundflower (2ch)";
	Server.default.options.outDevice= "Built-in Output";
	Server.default.options.memSize = (256*32);
	Server.default.options.blockSize = 64;
	Server.default.options.numWireBufs = 512;
	Server.default.options.numOutputBusChannels=2;
	Server.default.options.numInputBusChannels=2;
	Server.default.options.sampleRate = 44100;

};


~initS.value;


s.waitForBoot(
	{
		~frames = 4096;

		~ifft = Buffer.alloc(s, ~frames, 1, bufnum: 84);
		~ifft.setn(0, 0.0.dup(~frames));
		~panL = Buffer.alloc(s, ~frames, 1, bufnum: 85);
		~panL.setn(0, 0.0.dup(~frames));
		~panR = Buffer.alloc(s, ~frames, 1, bufnum: 86);
		~panR.setn(0, 0.0.dup(~frames));
		~tiltU = Buffer.alloc(s, ~frames, 1, bufnum: 87);
		~tiltU.setn(0, 0.0.dup(~frames));
		~tiltD = Buffer.alloc(s, ~frames, 1, bufnum: 88);
		~tiltD.setn(0, 0.0.dup(~frames));
		~mods = Buffer.alloc(s, 5, 1, bufnum: 89);
		~mods.setn(0, 0.0.dup(5));

		a=Env([0, 1, 0]*2, [60, ~frames/2-60]/~frames, \lin);
		~envBuf  = Buffer.alloc(s, ~frames/2, 1, bufnum: 91);
		~envBuf.loadCollection( a.asSignal(~frames/2) );

		~filelo = "/Users/nebbia/CITÉ_DES_ARTS_2019/Sounds/ans88x3_2-2fb-grm_grain.wav.scpv";
		~filehi = "/Users/nebbia/CITÉ_DES_ARTS_2019/Sounds/ans88x3_2-2fb-grm_purity.wav.scpv";
		~anslo = Buffer.read(s, ~filelo);
		~anshi = Buffer.read(s, ~filehi);




		SynthDef(\fogNewsAns88x3, {

			arg

			frames = ~frames, ansloBuf = ~anslo, anshiBuf = ~anshi,
			ifftBuf = ~ifft, envBuf = ~envBuf,
			panLBuf = ~panL.bufnum, panRBuf = ~panR.bufnum,
			tiltUBuf = ~tiltU.bufnum, tiltDBuf = ~tiltD.bufnum,
			modsBuf = ~mods.bufnum,
			inbus = 0;

			var
			colors, anslo, anshi, ifft, panl, panr, tiltu, tiltd, sigl, sigr, sigu, sigd,
			l, r, u, d,
			low, hi, lohi, white, lohiwhite,
			convo, convosig, mono, sig,
			amp = BufRd.kr(1, modsBuf, 0),
			lohimorph = BufRd.kr(1, modsBuf, 1),
			hishift = BufRd.kr(1, modsBuf, 2),
			whitemorph = BufRd.kr(1, modsBuf, 3),
			convomix = BufRd.kr(1, modsBuf, 4),
			volume = BufRd.kr(1, modsBuf, 5),
			input = Mix.ar( In.ar(inbus, 2) );

			BufWr.kr( 4096, ifftBuf, 0);
			colors = PV_BufRd( LocalBuf.new(frames), ifftBuf, 0 );
			colors = PV_MagMap(colors, envBuf);

			colors = PV_Compander( colors, 2, 1.2, 0.4);

			ifft = PV_Copy( colors, LocalBuf.new(frames) );

			anslo = PV_BufRd( LocalBuf.new(frames), ansloBuf, 0.03 );
			anshi = PV_BufRd( LocalBuf.new(frames), anshiBuf, 0.88 );

			low = PV_MagMul(anslo, colors );
			hi = PV_MagMul(anshi, ifft );
			hi = PV_MagShift(hi, 0.75, 512*hishift );
			lohi = PV_Morph(low, hi, lohimorph );

			lohi = PV_MagSquared(lohi );
			// lohi = PV_MagSmooth(lohi );
			lohi = PV_Diffuser(lohi );



			mono =IFFT(lohi, 0)*100;


			white = HPF.ar(
				BMoog.ar(WhiteNoise.ar( 0.2 ), (50+(whitemorph*65)).midicps, 0.4 ),
				5000
			);

			convo = AmplitudeMod.kr(input, 0.1, 0.5) ;

			convosig = PitchShift.ar(
				LPCAnalyzer.ar( input,
					PitchShift.ar(mono, 0.2, 2, 0, 0.1),
					256,  10, 0.95, 1)
				, 0.2, 1, 0, 0.1)*0.1;

			mono =  mono + (white*whitemorph) + (convosig*convomix*8);

			sig = mono * convo;
			sig = Limiter.ar(sig*2, 0.8, 0.1)* ((1-amp) * -70).dbamp;
			sig = Limiter.ar(sig, 0.9, 0.1);

			SendReply.kr(Impulse.kr(1), '/scSynth', 1);

			Out.ar(0, sig.dup);

		}).add;



		~fog = Synth.tail(s, \fogNewsAns88x3, [

			\frames, ~frames, \ansloBuf, ~anslo, \anshiBuf, ~anshi,
			\ifftBuf, ~ifft, \envBuf, ~envBuf,
			\panLBuf, ~panL.bufnum, \panRBuf, ~panR.bufnum,
			\tiltUBuf, ~tiltU.bufnum, \tiltDBuf, ~tiltD.bufnum,
			\modsBuf, ~mods.bufnum,
			\inbus, 2
		]);

		s.meter;
		s.plotTree;
});

