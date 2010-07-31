//
//  FliteTTS.m
//  iPhone Text To Speech based on Flite
//
//  Copyright (c) 2010 Sam Foster
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
// 
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.
//
//  Author: Sam Foster <samfoster@gmail.com> <http://cmang.org>
//  Copyright 2010. All rights reserved.
//

#import "FliteTTS.h"
#import "flite.h"

/*ACL removing some voices to save space...
cst_voice *register_cmu_us_kal();
cst_voice *register_cmu_us_kal16();
cst_voice *register_cmu_us_slt();
cst_voice *register_cmu_us_awb();*/
cst_voice *register_cmu_us_rms();
cst_wave *sound;
cst_voice *voice;

@implementation FliteTTS

-(id)init
{
    self = [super init];
	flite_init();
	// Set a default voice
	//voice = register_cmu_us_kal();
	//voice = register_cmu_us_kal16();
	//voice = register_cmu_us_rms();
	//voice = register_cmu_us_awb();
	//voice = register_cmu_us_slt();
	[self setVoice:@"cmu_us_rms"];
    return self;
}

-(void)speakText:(NSString *)text
{
	NSMutableString *cleanString;
	cleanString = [NSMutableString stringWithString:@""];
	if([text length] > 1)
	{
		int x = 0;
		while (x < [text length])
		{
			unichar ch = [text characterAtIndex:x];
			[cleanString appendFormat:@"%c", ch];
			x++;
		}
	}
	if(cleanString == nil)
	{	// string is empty
		cleanString = [NSMutableString stringWithString:@""];
	}
	sound = flite_text_to_wave([cleanString UTF8String], voice);
	
	
	/*
	// copy sound into soundObj -- doesn't yet work -- can anyone help fix this?
	soundObj = [NSData dataWithBytes:sound length:sizeof(sound)]; // find out wy this doesn't work
	NSError *sAudioPlayerErr;
	AVAudioPlayer *sAudioPlayer = [[AVAudioPlayer alloc] initWithData:soundObj error:&sAudioPlayerErr];
	NSLog(@"%@", [sAudioPlayerErr localizedDescription]);
	[sAudioPlayer setDelegate:self];
	[sAudioPlayer prepareToPlay];
	[sAudioPlayer play];
	NSLog(@"%@", [sAudioPlayerErr localizedDescription]);
	*/
	
	NSArray *filePaths = NSSearchPathForDirectoriesInDomains (NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *recordingDirectory = [filePaths objectAtIndex: 0];
	// Pick a file name
	NSString *tempFilePath = [NSString stringWithFormat: @"%@/%s", recordingDirectory, "temp.wav"];
	// save wave to disk
	char *path;	
	path = (char*)[tempFilePath UTF8String];
	cst_wave_save_riff(sound, path);
	// Play the sound back.
	NSError *err;
	[audioPlayer stop];
	audioPlayer =  [[AVAudioPlayer alloc] initWithContentsOfURL:[NSURL fileURLWithPath:tempFilePath] error:&err];
	[audioPlayer setDelegate:self];
	[audioPlayer prepareToPlay];
	[audioPlayer play];
	// Remove file
	[[NSFileManager defaultManager] removeItemAtPath:tempFilePath error:nil];
	
}

-(void)setPitch:(float)pitch variance:(float)variance speed:(float)speed
{
	feat_set_float(voice->features,"int_f0_target_mean", pitch);
	feat_set_float(voice->features,"int_f0_target_stddev",variance);
	feat_set_float(voice->features,"duration_stretch",speed); 
}

-(void)setVoice:(NSString *)voicename
{
	/*ACL removing some voices to save space
   if([voicename isEqualToString:@"cmu_us_kal"]) {
		voice = register_cmu_us_kal();
	}
	else if([voicename isEqualToString:@"cmu_us_kal16"]) {
		voice = register_cmu_us_kal16();
	}
  else if([voicename isEqualToString:@"cmu_us_slt"]) {
    voice = register_cmu_us_slt();
  }
  else  if([voicename isEqualToString:@"cmu_us_awb"]) {
    voice = register_cmu_us_awb();
  } else */ if([voicename isEqualToString:@"cmu_us_rms"]) {
		voice = register_cmu_us_rms();
	}
	
}

-(void)stopTalking
{
	[audioPlayer stop];
}

@end
