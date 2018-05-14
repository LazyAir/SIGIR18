/**
The code in this io_ud.h file is from the authors of the following paper:
Jiang, Minhao, Ada Wai-Chee Fu, Raymond Chi-Wing Wong, and Yanyan Xu. "Hop doubling label indexing for 
point-to-point distance querying on scale-free networks." Proceedings of the VLDB Endowment 7, no. 12 (2014): 1203-1214.

It is for loading the 2-hop labels, which will only be used in the function LoadHop2Labels() in Index.cpp
**/

#include <cstdio>
#include <cstring>
#include <algorithm>
#include <vector>
#include <map>
#include <sys/time.h>
#include <iostream>
//#include <stdio.h>

using namespace std;

typedef int wtype; //data type for edge weight and budget, etc.

const int bufSize = 1024 * 3 * 256 * 32;//2 * 12 ;
long long iCnt = 0, oCnt = 0;
double tRead = 0, tWrite = 0;


struct timer{
	timeval stime, etime;
	
	timer(){
		gettimeofday(&stime, NULL);
	}
	void restart(){
		gettimeofday(&stime, NULL); 
	}

	double getTime(){
		gettimeofday(&etime, NULL);
		return (double)etime.tv_sec-stime.tv_sec + 
			((double)etime.tv_usec-stime.tv_usec)/1000000.0;
		
	}
};

struct edgeS{
	int x;
	wtype w;
	bool operator < (const edgeS & edgeTmp) const{
		return (x < edgeTmp.x) || (x == edgeTmp.x && abs(w) < abs(edgeTmp.w) ) ||
				( x == edgeTmp.x && abs(w) == abs(edgeTmp.w) && w > edgeTmp.w);
	}
}__attribute__((packed));


struct edgeL{
	long long x, y, w;
	bool operator < (const edgeL & edgeTmp) const{
		return (x < edgeTmp.x) || (x == edgeTmp.x && y < edgeTmp.y) || (x == edgeTmp.x && y == edgeTmp.y && w < edgeTmp.w);
	}
};


struct inBufL{
	edgeL *buf;
	int cnt, bufLen, perBuf;
	FILE *inFile;
	bool isEnd;
	inBufL()	{}
	inBufL(char *fileName){
		perBuf = bufSize / sizeof(edgeL);
		buf = (edgeL*)malloc(sizeof(edgeL) * perBuf);
		inFile = fopen(fileName, "rb");
	}
	void init(char *fileName){
		inFile = fopen(fileName, "rb");
	}

	void nextOne(edgeL & tmpEdge){
		fread(&tmpEdge, sizeof(edgeL), 1, inFile);
	}

	void start(){
		timer tm;
		cnt = 0;
		iCnt++;
		bufLen = fread(buf, sizeof(edgeL), perBuf, inFile);
		isEnd = (bufLen == 0);
		tRead += tm.getTime();
	}

 /*
	void fseek(long long x)
	{
		fseeko64(inFile, x*sizeof(edgeL), SEEK_SET);
	}
 */

	~inBufL(){
		if (inFile != NULL)	fclose(inFile);
		free(buf);
	}
	void nextEdge(edgeL & tmpEdge){
		if (cnt < bufLen - 1)
		{
			tmpEdge = buf[cnt++];
		}
		else
		{
			timer tm;
			iCnt++;
			tmpEdge = buf[cnt];
			bufLen = fread(buf, sizeof(edgeL), perBuf, inFile);
			cnt = 0;
			isEnd = (bufLen == 0);
			tRead += tm.getTime();
		}
	}

};

struct outBufL{
	edgeL * buf;
	int cnt, perBuf;
	FILE *outFile;
	outBufL() {}
	outBufL(char *fileName){
		perBuf = bufSize/sizeof(edgeL);
		buf = (edgeL*)malloc(sizeof(edgeL) * perBuf);
		cnt = 0;
		outFile = fopen(fileName, "wb");
	}
	~outBufL(){
		flush();
		fflush(outFile);
		if (outFile != NULL) fclose(outFile);
		free(buf);
	}
	void insert(long long x, long long y, long long w){
		buf[cnt].x = x;
		buf[cnt].y = y;
		buf[cnt++].w = w;
		if (cnt == perBuf) flush();
	}
	void insert(edgeL & x){
		buf[cnt++] = x;
		if (cnt == perBuf) flush();
	}

	void flush(){
		timer tm;
		oCnt++;
		fwrite(buf, sizeof(edgeL), cnt, outFile);
		cnt = 0;
		tWrite += tm.getTime();
	}

};

struct inBufS{
	edgeS *buf;
	int cnt, bufLen;
	FILE *inFile;
	bool isEnd;
	int perBuf;
	inBufS()	{}
	inBufS(char *fileName){
		perBuf = bufSize / sizeof(edgeS);
		buf = (edgeS*)malloc(sizeof(edgeS) * perBuf);
		inFile = fopen(fileName, "rb");
	}
	void init(char *fileName){
		buf = (edgeS*)malloc(sizeof(edgeS) * perBuf);
		inFile = fopen(fileName, "rb");
	}

	void start(){
		timer tm;
		cnt = 0;
		iCnt++;
		bufLen = fread(buf, sizeof(edgeS), perBuf, inFile);
		isEnd = (bufLen == 0);
		tRead += tm.getTime();

	}
 /*
	void fseek(long long x)
	{
		fseeko64(inFile, x*sizeof(edgeS), SEEK_SET);
	}
 */
	~inBufS(){
		if (inFile != NULL)	fclose(inFile);
		free(buf);
	}
	void nextEdge(edgeS & tmpEdge){
		if (cnt < bufLen - 1){
			tmpEdge = buf[cnt++];
		}
		else{
			timer tm;
			iCnt++;
			tmpEdge = buf[cnt];
			bufLen = fread(buf, sizeof(edgeS), perBuf, inFile);
			cnt = 0;
			isEnd = (bufLen == 0);
			tRead += tm.getTime();
		}
	}

};

struct outBufS{
	edgeS *buf;
	int cnt, perBuf;
	FILE *outFile;
	outBufS() {}
	outBufS(char *fileName){
		perBuf = bufSize/sizeof(edgeS);
		buf = (edgeS*)malloc(sizeof(edgeS) * perBuf);
		cnt = 0;
		outFile = fopen(fileName, "wb");
	}
	~outBufS(){
		flush();
		fflush(outFile);
		if (outFile != NULL) fclose(outFile);
		free(buf);
	}
	void insert(int x, wtype w){
		buf[cnt].x = x;
		buf[cnt++].w = w;
		if (cnt == perBuf) flush();
	}

	void flush(){
		timer tm;
		oCnt++;
		fwrite(buf, sizeof(edgeS), cnt, outFile);
		cnt = 0;
		tWrite += tm.getTime();
	}

};

long long checkB(char * fileName){
	FILE * pFile = fopen (fileName,"rb");
	fseek (pFile, 0, SEEK_END);
	long long ans =ftell(pFile)+0.0;
	fclose(pFile);
	return ans;
}
