#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void playSound();
int getNum(FILE* x,int bytesPS,int endian);
int* makeQueue(int size);
int queueSwap(int* data,int push);
char* movingAvg(FILE* x,int bytesPS,int frameSize,int dataLength);
int getInt(FILE* x,int bytesPS,int endian);

#define LITTLE_ENDIAN  0
#define BIG_ENDIAN  1

int main(void){

	char fileName[256];
	char riff[5];
	char dataHeader[5];
	int bytesPerSample;
	int dataLength = 0;
	int frameSize = 100;
	char* newSignal;

	sprintf(fileName,"C:\\Users\\branc\\OneDrive\\Desktop\\dnb\\drum1.wav");
	printf("Now opening %s\n",fileName);
	FILE *file = fopen(fileName,"rb");//opens .wav file
	FILE *output = fopen("output1.wav","wb");//creates writable output file

	//playSound();

	//RIFF
	for(int i=0;i<4;i++){
		riff[i] = fgetc(file);
	}
	riff[4] = '\0';//adds null to create string
	printf("%s\n",riff);

	//BYTES PER SAMPLE
	fseek(file,34,SEEK_SET);//jumps to location of bytesPerSamples
	bytesPerSample = fgetc(file) / 8;
	fgetc(file);//skip a byte
	printf("Bytes per sample:");
	printf("%d\n",bytesPerSample);

	//DATA HEADER
	for(int i=0;i<4;i++){
		dataHeader[i] = fgetc(file);
	}
	dataHeader[4] = '\0';//adds null to create string
	printf("%s\n",dataHeader);

	//CHUNK 2 SIZE
	dataLength = getInt(file,4,0);//chunk2size info is 4 bytes
	printf("File size: %u kb\n",dataLength/1000);

	//FRAME SIZE
	printf("Enter desired frame length:\n");
	fflush(stdout);
	scanf(" %d",&frameSize);

	newSignal = movingAvg(file,bytesPerSample,frameSize,dataLength);

	fseek(file,0,SEEK_SET);

	printf("Writing...");
	fflush(stdout);

	for(int i = 0;i < 44;i++){
		int temp = fgetc(file);
		fprintf(output,"%c",temp);
	}

	for(int i = 0;i < dataLength;i++){
		fprintf(output,"%c",newSignal[i]);
	}

	fflush(output);

	fclose(file);
	fclose(output);

	return 0;
}

void playSound(  ) {
    char command[256];
    sprintf(command,"powershell -c (New-Object Media.SoundPlayer \"C:\\Users\\branc\\OneDrive\\Desktop\\dnb\\drum1.wav\").PlaySync()");
    /* play sound */
    system(command);
}

char* movingAvg(FILE* x,int bytesPS,int frameSize,int dataLength){

	char* newSignal = malloc(dataLength);
	int amplitudes[dataLength / bytesPS];
	int* frame;
	int avg = 0;
	int temp = 0;

	printf("Filtering... ");
	fflush(stdout);

	frame = makeQueue(frameSize);

	for(int i = 0;i < frameSize;i++){
		queueSwap(frame,getNum(x,bytesPS,0));
		fflush(stdout);
	}//loads frame with first samples

	for(int i = 2;i < frameSize + 2;i++){
		avg += frame[i];
	}//loads avg with numbers in frame

	for(int i = 0;i < dataLength / bytesPS - frameSize;i++){
		amplitudes[i] = avg / frameSize;
		temp = getNum(x,bytesPS,0);
		avg -= queueSwap(frame,temp);
		avg += temp;
	}//this is the moving average filter

	for(int i = 0;i < sizeof(amplitudes) / sizeof(int);i++){
		temp = amplitudes[i];

		for(int j = 0;j < bytesPS;j++){
			newSignal[i * bytesPS + j] = temp % 256;
			temp /= 256;
		}

	}//encodes new waveform, little endian

	printf("Complete\n");
	fflush(stdout);

	return newSignal;
}

int getNum(FILE* x,int bytesPS,int endian){
	short number = 0;
	int temp[bytesPS];
	if(endian == LITTLE_ENDIAN){
		for(int i = bytesPS - 1;i >= 0;i--){
			temp[i] = fgetc(x);
		}
		for(int i = 0;i < bytesPS;i++){
			number *= 256;
			number += temp[i];
		}
	}
	else if(endian == BIG_ENDIAN){
		for(int i = 0;i < bytesPS;i++){
			number *= 256;
			number += fgetc(x);
		}
	}
	else{
		printf("Error\n");
	}
	return number;
}//decodes short, 16bit

int getInt(FILE* x,int bytesPS,int endian){
	int number = 0;
	int temp[bytesPS];
	if(endian == LITTLE_ENDIAN){
		for(int i = bytesPS - 1;i >= 0;i--){
			temp[i] = fgetc(x);
		}
		for(int i = 0;i < bytesPS;i++){
			number *= 256;
			number += temp[i];
		}
	}
	else if(endian == BIG_ENDIAN){
		for(int i = 0;i < bytesPS;i++){
			number *= 256;
			number += fgetc(x);
		}
	}
	else{
		printf("Error\n");
	}
	return number;
}//decodes int, 32bit

int* makeQueue(int size){
	int* data = malloc(size * sizeof(int) + 2);
	data[0] = size + 2;//size of array
	data[1] = 2;//index of first in queue
	return data;
}

int queueSwap(int* data,int push){
	int pull = data[data[1]];

	data[data[1]] = push;
	data[1]++;

	if(data[1] >= data[0]){
		data[1] = 2;
	}//returns to beginning of array

	return pull;
}
