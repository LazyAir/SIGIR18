#include <cstdio>
#include <cstring>
#include <algorithm>
#include <vector>
#include <map>
#include <sys/time.h>
#include <iostream>

using namespace std;

#define show(x) cout<<#x<<" "<<(x)<<endl

//typedef signed char wtype;
typedef int wtype;

const long long memSize = 4LL * 1024 * 1024 * 1024 ;//6 * 12 ; //1024 * 4;
const int bufSize = 1024 * 3 * 256 * 32;//2 * 12 ;
//const int intPerMem = memSize / 2 / sizeof(int);
const int llPerBuf = bufSize / sizeof(long long);
const int fileNameLen = 128;

long long iCnt = 0, oCnt = 0;
double tRead = 0, tWrite = 0;


int iter = 0, allCnt, nxCnt, nyCnt, ilCnt, ihCnt, olCnt, ohCnt;
long long pCnt = 0, c1 = 0, c2 = 0, c3 = 0, c4 = 0, c22 = 0, lenCnt = 0, largeDisk = 0, sumCnt = 0;


double tBi, tRe, tInit, tGen, tRun, tPrune, tMerge1, tMerge2, tGenS = 0, tPruneS = 0, tMerge1S = 0, tMerge2S = 0,tSortS = 0, tSort1 = 0, tSort2 = 0, tQuery, tPruneSort = 0, tPruneCPU = 0, tSortMem = 0;
double tBP = 0;

int ixBuf, oxBuf, iyBuf, oyBuf, ixMerge, oxMerge, iyMerge, oyMerge, 
	iSort = 0, oSort = 0, iSort1 = 0, oSort1 = 0, iSort2 = 0, oSort2 = 0;


bool fTest = 0, fBP = 0, fxBuf = 0, fyBuf = 0, fmerge = 0, fmerge2 = 0, fdeb = 1, fiter = 0, fnon = 0, findex = 0, fnew = 0, ftime = 1, fio = 0, fprune = 0, fEq = 0, fCheck = 0;

//const int lenSize = 5, lowBit = 65535;
//const int lenPrune[lenSize + 1] = {0, 2, 3, 4, 6, 9};



struct timer
{
	timeval stime, etime;
	
	timer()
	{
		gettimeofday(&stime, NULL);
	}
	void restart()
	{
		gettimeofday(&stime, NULL); 
	}

/*	void end()
	{
		gettimeofday(&etime, NULL);
	}
*/
	double getTime()
	{
		gettimeofday(&etime, NULL);
		return (double)etime.tv_sec-stime.tv_sec + 
			((double)etime.tv_usec-stime.tv_usec)/1000000.0;
		
	}
	
};

struct edgeS
{
	int x;
	wtype w;
	bool operator < (const edgeS & edgeTmp) const
	{
		return x < edgeTmp.x || x == edgeTmp.x && abs(w) < abs(edgeTmp.w) ||
				x == edgeTmp.x && abs(w) == abs(edgeTmp.w) && w > edgeTmp.w;
	}
}__attribute__((packed));



struct edge
{
	int x, y;
	wtype w;
	edge (){}
	edge (int xx, int yy, wtype ww)
	{
		x = xx; y = yy; w = ww;
	}	
	bool operator < (const edge & edgeTmp) const
	{
		return x < edgeTmp.x || x == edgeTmp.x && y < edgeTmp.y || 
		x == edgeTmp.x && y == edgeTmp.y && abs(w) < abs(edgeTmp.w);
	}

}__attribute__((packed));

long long checkSize(char *fileName)
{
	FILE * pFile = fopen (fileName,"rb");
	fseeko64(pFile, 0, SEEK_END); 
	long long ans = ftello64(pFile)/sizeof(edge);
	fclose(pFile);
	return ans;
}


struct inBuf
{
	edge *buf;
	int cnt, bufLen;
	FILE *inFile;
	bool isEnd;
	int perBuf;
	inBuf()
	{
		perBuf = bufSize / sizeof(edge);
		inFile = NULL;
	}
	inBuf(char *fileName)
	{
		perBuf = bufSize / sizeof(edge);
		init(fileName, false);
	}
	void init(char *fileName, bool isMem)
	{
		if (isMem) perBuf = memSize/5/sizeof(edge);
		buf = (edge*)malloc(sizeof(edge) * perBuf);
		inFile = fopen(fileName, "rb");
	}
	
/*	void nextInt(int & ans)
	{
		fread(&ans, sizeof(int), 1, inFile);
	}
*/	
	void start()
	{
		timer tm;
		cnt = 0;
		iCnt++;
		bufLen = fread(buf, sizeof(edge), perBuf, inFile);
		isEnd = (bufLen == 0);
		tRead += tm.getTime();
	}
	~inBuf()
	{
		if (inFile != NULL)	fclose(inFile);
		free(buf);
	}
	void nextEdge()
	{
		if (cnt < bufLen - 1) cnt++;
		else 
		{
			timer tm;
			iCnt++;
			bufLen = fread(buf, sizeof(edge), perBuf, inFile);
			cnt = 0;
			isEnd = (bufLen == 0);
			tRead += tm.getTime();
		}
	}
	void nextEdge(edge & tmpEdge)
	{
		if (cnt < bufLen - 1)
		{
			tmpEdge = buf[cnt++];
		}
		else 
		{
			timer tm;
			iCnt++;
			tmpEdge = buf[cnt];
			bufLen = fread(buf, sizeof(edge), perBuf, inFile);
			cnt = 0;
			isEnd = (bufLen == 0);
			tRead += tm.getTime();
		}
	}
	void nextEdge(edgeS & tmpEdge)
	{
		if (cnt < bufLen - 1)
		{
			tmpEdge.x = buf[cnt].y;
			tmpEdge.w = buf[cnt++].w;
		}
		else 
		{
			timer tm;
			iCnt++;
			tmpEdge.x = buf[cnt].y;
			tmpEdge.w = buf[cnt].w;
			bufLen = fread(buf, sizeof(edge), perBuf, inFile);
			cnt = 0;
			isEnd = (bufLen == 0);
			tRead += tm.getTime();
		}
	}
	
/*	long long checkSize()
	{
		long long tmp = ftell(inFile);
		fseeko64(inFile, 0, SEEK_END); 
		long long ans = ftell(inFile);
		fseeko64(inFile, tmp, SEEK_SET);
		return ans;
	}
*/	
	
};


//char * p1, * p2;

struct outBuf
{
	edge *buf;
	int cnt;
	FILE *outFile;
	int perBuf;
	long long * sCnt;
	outBuf()
	{
		perBuf = bufSize/sizeof(edge);
		buf = (edge*)malloc(perBuf * sizeof(edge));
		cnt = 0;
		outFile = NULL;
	}
	outBuf(char *fileName)
	{
		perBuf = bufSize/sizeof(edge);
		buf = (edge*)malloc(perBuf * sizeof(edge));
		cnt = 0;
		outFile = fopen(fileName, "wb");
	}
	outBuf(char *fileName, long long & sCnt_)
	{
		sCnt = &sCnt_;
		perBuf = memSize/5/sizeof(edge);
		buf = (edge*)malloc(perBuf * sizeof(edge));
		cnt = 0;
		outFile = fopen(fileName, "wb");
	}
	
	~outBuf()
	{
		flush();
		fflush(outFile);
		if (outFile != NULL) fclose(outFile);
		free(buf);
	}
	void insert(edge & x)
	{
		buf[cnt++] = x;
		if (cnt == perBuf) flush();
	}
	void insert(int x, int y, wtype w)
	{
		buf[cnt].x = x;
		buf[cnt].y = y;
		buf[cnt++].w = w;
		if (cnt == perBuf) flush();
	}

		
	void flush()
	{
		if (perBuf != bufSize/sizeof(edge) && cnt > 0)
		{
			sort(buf, buf+cnt);
			int newCnt = 1;
			for (int px = buf[0].x, py = buf[0].y, i = 1; i < cnt; i++)
			{
				if (px == buf[i].x && py == buf[i].y) continue;
				buf[newCnt++] = buf[i];
				px = buf[i].x; py = buf[i].y;
			}
			*sCnt -= cnt - newCnt;
			cnt = newCnt;
		}
			
		timer tm;
		oCnt++;
		fwrite(buf, sizeof(edge), cnt, outFile);
		cnt = 0;
		tWrite += tm.getTime();
	}
	
};







void xSort(char * _sName, long long & m, bool deDu)
{
if (fdeb)	printf("sort begin %lld\n", m);
if (m == 0) return;
	timer tm, tm1;
	tm.restart();
	char *sName = (char*)malloc(1+strlen(_sName));
	strcpy(sName, _sName);
	char* tmpName = (char*)malloc(1+strlen(sName)+4);
	strcpy(tmpName, sName);
	strcat(tmpName, ".tmp");
	
	int cnt, edgePerMem = memSize/sizeof(edge), edgePerBuf = bufSize/sizeof(edge);

	edge * memEdge = (edge*)malloc(edgePerMem * (long long)sizeof(edge));
	
	double tin = 0;
	if (deDu)
	{
		long long tmpM = m;
		outBuf tmpBuf(tmpName);
		inBuf usEdge(sName);
		usEdge.start();
		m = 0;
		while (!usEdge.isEnd)
		{
			cnt = 0;
			memEdge[0].x = 0;
			while (cnt < edgePerMem && !usEdge.isEnd)
				usEdge.nextEdge(memEdge[cnt++]);
			timer tTmp;
			sort(memEdge, memEdge+cnt);
			tin += tTmp.getTime();
			int px = -1, py = -1;
			for (int i = 0; i < cnt; i++)
			{
				if (memEdge[i].x == px && memEdge[i].y == py) continue;
				m++;
				tmpBuf.insert(memEdge[i]);
				px = memEdge[i].x; py = memEdge[i].y;
			}
		}
		
		swap(tmpName, sName);
printf("ok deDu %lld %lf %lf %lf\n", m, (m+0.0)/tmpM, tm.getTime(), tin);
	
	}


	{
		outBuf tmpBuf(tmpName);
		inBuf usEdge(sName);
		usEdge.start();
		while (!usEdge.isEnd)
		{
			cnt = 0;
			while (cnt < edgePerMem && !usEdge.isEnd)
				usEdge.nextEdge(memEdge[cnt++]);
				
			timer tTmp;
			sort(memEdge, memEdge+cnt);
			tin += tTmp.getTime();
			for (int i = 0; i < cnt; i++)
				tmpBuf.insert(memEdge[i]);
		}
	}
	
	tSort1 += tm1.getTime();
	
	int iCntTmp, oCntTmp;
	iSort1 += (iCntTmp=iCnt);
	oSort1 += (oCntTmp=oCnt);
	
printf("ok %lf %lf\n", tm.getTime(), tin);
	
	if (m <= edgePerMem)
	{
		if (1+strlen(tmpName) > 1+strlen(sName))
		{
			remove(sName);
			rename(tmpName, sName);
		}
		else
		{
			remove(sName);
		}
		free(sName);	
		free(tmpName);	
		free(memEdge);

		tSortS += tm.getTime();
		if (fdeb)printf("m edgePerMem time %lld %d %lf\n", m, edgePerMem, tm.getTime());
		return;
	}
	
	
	timer tm2;

	
	int h[edgePerMem/edgePerBuf][2], hh, cntBuf[edgePerMem/edgePerBuf];
	
	long long posFile[edgePerMem/edgePerBuf];
	int ii = 0;
	for (long long len = edgePerMem; len < m; len *= edgePerMem/edgePerBuf)
	{
		swap(sName, tmpName);
		inBuf sBuf(sName);
		outBuf tmpBuf(tmpName);
		
//show(len);
		
		for (long long lastPos = 0; lastPos < m; lastPos += len * (edgePerMem/edgePerBuf) )
		{
			cnt = 0;
			int maxi;
			for (int i = 0; i < edgePerMem/edgePerBuf && lastPos+i*len < m; i++ )
			{
				maxi = i;

//printf("%d\n", (lastPos+i*len));

				fseeko64(sBuf.inFile, (lastPos+i*len)*sizeof(edge), SEEK_SET);
				posFile[i] = 0;
				sBuf.start();
				h[hh=i+1][0] = cnt;
				h[i+1][1] = i;
				while (!sBuf.isEnd && cnt - h[hh][0] < edgePerBuf)
					sBuf.nextEdge(memEdge[cnt++]);

				cntBuf[i] = cnt;
				for (int j = hh, jj; j > 1; j=jj)
				{
					jj = (j>>1);
					if (memEdge[h[j][0]] < memEdge[h[jj][0]])
					{
						swap(h[j][0], h[jj][0]);
						swap(h[j][1], h[jj][1]);
					}
					else break;
				}
				cntBuf[i] = cnt;
			}
			maxi++;
			while (hh > 0)
			{
				int minEdge = h[1][0], pi = h[1][1];
				tmpBuf.insert(memEdge[minEdge]);
				
				if (minEdge < cntBuf[pi]-1 || posFile[pi] < (len-1)/edgePerBuf)
				{
					if (minEdge < cntBuf[pi]-1) h[1][0] = minEdge+1;
					else
					{
						posFile[pi]++;
						fseeko64(sBuf.inFile, (lastPos + pi*len + posFile[pi]*edgePerBuf)*sizeof(edge), SEEK_SET);
						sBuf.start();
						cntBuf[pi] = h[1][0] = pi*edgePerBuf;
	 					while (!sBuf.isEnd && cntBuf[pi] - h[1][0] < edgePerBuf 
		 					&& edgePerBuf * posFile[pi] + cntBuf[pi] - h[1][0] < len)
		 				sBuf.nextEdge(memEdge[cntBuf[pi]++]);

						if (sBuf.isEnd || edgePerBuf * posFile[pi] + cntBuf[pi] - h[1][0] >= len) posFile[pi] = (len-1)/edgePerBuf;
						
					}
				}
				else 
				{
					h[1][0] = h[hh][0]; h[1][1] = h[hh--][1];					
				}
					
				for (int j = 1, jj; (j<<1) <= hh; j = jj)
				{
					jj = (j<<1);
					if (jj < hh && memEdge[h[jj+1][0]] < memEdge[h[jj][0]]) jj++;
					if (memEdge[h[jj][0]] < memEdge[h[j][0]]) 
					{
						swap(h[j][0], h[jj][0]);
						swap(h[j][1], h[jj][1]);
					}
					else break;
				}											
			}	
		}		
//		break;	
	}
//	tSort += tm.getTime();
	
//	printf("%s\n%s\n", tmpName, sName);
//	return;
	
	if (1+strlen(tmpName) > 1+strlen(sName))
	{
		remove(sName);
		rename(tmpName, sName);
	}
	else
	{
		remove(sName);
	}
	free(sName);	
	free(tmpName);	
	free(memEdge);

	tSort2 += tm2.getTime();
	tSortS += tm.getTime();
	iSort2 += iCnt - iCntTmp;
	oSort2 += oCnt - oCntTmp;
	iSort += iCnt;
	oSort += oCnt;
	if (fdeb)printf("m edgePerMem time %lld %d %lf\n", m, edgePerMem, tm.getTime());
	
//	if (iter > 1)	{	deDu(_sName, m);	printf("\nm edgePerMem time %d %d %lf\n\n", m, edgePerMem, tm.getTime());	}
}






struct edgeL
{
	long long x, y, w;
	bool operator < (const edgeL & edgeTmp) const
	{
		return x < edgeTmp.x || x == edgeTmp.x && y < edgeTmp.y || x == edgeTmp.x && y == edgeTmp.y && w < edgeTmp.w;
	}
};


struct inBufL
{
	edgeL *buf;
	int cnt, bufLen, perBuf;
	FILE *inFile;
	bool isEnd;
	inBufL()	{}
	inBufL(char *fileName)
	{
		perBuf = bufSize / sizeof(edgeL);
		buf = (edgeL*)malloc(sizeof(edgeL) * perBuf);		
		inFile = fopen(fileName, "rb");
	}
	
	void start()
	{
		timer tm;
		cnt = 0;
		iCnt++;
		bufLen = fread(buf, sizeof(edgeL), perBuf, inFile);
		isEnd = (bufLen == 0);
		tRead += tm.getTime();
	}
	
	~inBufL()
	{
		if (inFile != NULL)	fclose(inFile);
		free(buf);
	}
	void nextEdge(edgeL & tmpEdge)
	{
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

struct outBufL
{
	edgeL * buf;
	int cnt, perBuf;
	FILE *outFile;
	outBufL() {}
	outBufL(char *fileName)
	{
		perBuf = bufSize/sizeof(edgeL);
		buf = (edgeL*)malloc(sizeof(edgeL) * perBuf);
		cnt = 0;
		outFile = fopen(fileName, "wb");
	}
	~outBufL()
	{
		flush();
		fflush(outFile);
		if (outFile != NULL) fclose(outFile);
		free(buf);
	}
	void insert(long long x, long long y, long long w)
	{
		buf[cnt].x = x;
		buf[cnt].y = y;
		buf[cnt++].w = w;
		if (cnt == perBuf) flush();
	}
	void insert(edgeL & x)
	{
		buf[cnt++] = x;
		if (cnt == perBuf) flush();
	}
	
	void flush()
	{
		timer tm;
		oCnt++;
		fwrite(buf, sizeof(edgeL), cnt, outFile);
		cnt = 0;
		tWrite += tm.getTime();
	}
	
};




void xSortL(char * _sName, long long & m)
{
if (fdeb)	printf("sortL begin %lld\n", m);
if (m == 0) return;
	timer tm, tm1;
	tm.restart();
	iCnt = oCnt = 0; 
	char *sName = (char*)malloc(1+strlen(_sName));
	strcpy(sName, _sName);
	char* tmpName = (char*)malloc(1+strlen(sName)+4);
	strcpy(tmpName, sName);
	strcat(tmpName, ".tmp");
	
	int cnt, edgePerMem = memSize/sizeof(edgeL), edgePerBuf = bufSize/sizeof(edgeL);
//printf("%d\n", sizeof(edge));

	edgeL * memEdge = (edgeL*)malloc(edgePerMem * (long long)sizeof(edgeL));
	
	double tin = 0;

	{
		outBufL tmpBuf(tmpName);
		inBufL usEdge(sName);
		usEdge.start();
		while (!usEdge.isEnd)
		{
			cnt = 0;
			while (cnt < edgePerMem && !usEdge.isEnd)
				usEdge.nextEdge(memEdge[cnt++]);
				
			timer tTmp;
			sort(memEdge, memEdge+cnt);
			tin += tTmp.getTime();
			for (int i = 0; i < cnt; i++)
				tmpBuf.insert(memEdge[i]);
		}
	}
	
	tSort1 += tm1.getTime();
	
	int iCntTmp, oCntTmp;
	iSort1 += (iCntTmp=iCnt);
	oSort1 += (oCntTmp=oCnt);
	
printf("ok %lf %lf\n", tm.getTime(), tin);
	
	if (m <= edgePerMem)
	{
		if (1+strlen(tmpName) > 1+strlen(sName))
		{
			remove(sName);
			rename(tmpName, sName);
		}
		else
		{
			remove(sName);
		}
		free(sName);	
		free(tmpName);	
		free(memEdge);

		tSortS += tm.getTime();
		if (fdeb)printf("m edgePerMem time %lld %d %lf\n", m, edgePerMem, tm.getTime());
		return;
	}
	
	
	timer tm2;

	
	int h[edgePerMem/edgePerBuf][2], hh, cntBuf[edgePerMem/edgePerBuf];
	
	long long posFile[edgePerMem/edgePerBuf];
	int ii = 0;
	for (long long len = edgePerMem; len < m; len *= edgePerMem/edgePerBuf)
	{
		swap(sName, tmpName);
		inBufL sBuf(sName);
		outBufL tmpBuf(tmpName);
		
//show(len);
		
		for (long long lastPos = 0; lastPos < m; lastPos += len * (edgePerMem/edgePerBuf) )
		{
			cnt = 0;
			int maxi;
			for (int i = 0; i < edgePerMem/edgePerBuf && lastPos+i*len < m; i++ )
			{
				maxi = i;

//printf("%d\n", (lastPos+i*len));

				fseeko64(sBuf.inFile, (lastPos+i*len)*sizeof(edgeL), SEEK_SET);
				posFile[i] = 0;
				sBuf.start();
				h[hh=i+1][0] = cnt;
				h[i+1][1] = i;
				while (!sBuf.isEnd && cnt - h[hh][0] < edgePerBuf)
					sBuf.nextEdge(memEdge[cnt++]);

				cntBuf[i] = cnt;
				for (int j = hh, jj; j > 1; j=jj)
				{
					jj = (j>>1);
					if (memEdge[h[j][0]] < memEdge[h[jj][0]])
					{
						swap(h[j][0], h[jj][0]);
						swap(h[j][1], h[jj][1]);
					}
					else break;
				}
				cntBuf[i] = cnt;
			}
			maxi++;
			while (hh > 0)
			{
				int minEdge = h[1][0], pi = h[1][1];
				tmpBuf.insert(memEdge[minEdge]);
				
				if (minEdge < cntBuf[pi]-1 || posFile[pi] < (len-1)/edgePerBuf)
				{
					if (minEdge < cntBuf[pi]-1) h[1][0] = minEdge+1;
					else
					{
						posFile[pi]++;
						fseeko64(sBuf.inFile, (lastPos + pi*len + posFile[pi]*edgePerBuf)*sizeof(edgeL), SEEK_SET);
						sBuf.start();
						cntBuf[pi] = h[1][0] = pi*edgePerBuf;
	 					while (!sBuf.isEnd && cntBuf[pi] - h[1][0] < edgePerBuf 
		 					&& edgePerBuf * posFile[pi] + cntBuf[pi] - h[1][0] < len)
		 				sBuf.nextEdge(memEdge[cntBuf[pi]++]);

						if (sBuf.isEnd || edgePerBuf * posFile[pi] + cntBuf[pi] - h[1][0] >= len) posFile[pi] = (len-1)/edgePerBuf;
						
					}
				}
				else 
				{
					h[1][0] = h[hh][0]; h[1][1] = h[hh--][1];					
				}
					
				for (int j = 1, jj; (j<<1) <= hh; j = jj)
				{
					jj = (j<<1);
					if (jj < hh && memEdge[h[jj+1][0]] < memEdge[h[jj][0]]) jj++;
					if (memEdge[h[jj][0]] < memEdge[h[j][0]]) 
					{
						swap(h[j][0], h[jj][0]);
						swap(h[j][1], h[jj][1]);
					}
					else break;
				}											
			}	
		}		
//		break;	
	}
//	tSort += tm.getTime();
	
//	printf("%s\n%s\n", tmpName, sName);
//	return;
	
	if (1+strlen(tmpName) > 1+strlen(sName))
	{
		remove(sName);
		rename(tmpName, sName);
	}
	else
	{
		remove(sName);
	}
	free(sName);	
	free(tmpName);	
	free(memEdge);

	tSort2 += tm2.getTime();
	tSortS += tm.getTime();
	iSort2 += iCnt - iCntTmp;
	oSort2 += oCnt - oCntTmp;
	iSort += iCnt;
	oSort += oCnt;
	if (fdeb)printf("m edgePerMem time %lld %d %lf\n", m, edgePerMem, tm.getTime());
	
//	if (iter > 1)	{	deDu(_sName, m);	printf("\nm edgePerMem time %d %d %lf\n\n", m, edgePerMem, tm.getTime());	}
}




struct inBufS
{
	edgeS *buf;
	int cnt, bufLen;
	FILE *inFile;
	bool isEnd;
	int perBuf;
	inBufS()	{}
	inBufS(char *fileName)
	{
		perBuf = bufSize / sizeof(edgeS);
		buf = (edgeS*)malloc(sizeof(edgeS) * perBuf);		
		inFile = fopen(fileName, "rb");
	}
	
	void start()
	{
		timer tm;
		cnt = 0;
		iCnt++;
		bufLen = fread(buf, sizeof(edgeS), perBuf, inFile);
		isEnd = (bufLen == 0);
		tRead += tm.getTime();
		
	}
	~inBufS()
	{
		if (inFile != NULL)	fclose(inFile);
		free(buf);
	}
	void nextEdge(edgeS & tmpEdge)
	{
		if (cnt < bufLen - 1)
		{
			tmpEdge = buf[cnt++];
		}
		else 
		{
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

struct outBufS
{
	edgeS *buf;
	int cnt, perBuf;
	FILE *outFile;
	outBufS() {}
	outBufS(char *fileName)
	{
		perBuf = bufSize/sizeof(edgeS);
		buf = (edgeS*)malloc(sizeof(edgeS) * perBuf);
		cnt = 0;
		outFile = fopen(fileName, "wb");
	}
	~outBufS()
	{
		flush();
		fflush(outFile);
		if (outFile != NULL) fclose(outFile);
		free(buf);
	}
	void insert(int x, wtype w)
	{
		buf[cnt].x = x;
		buf[cnt++].w = w;
		if (cnt == perBuf) flush();
	}
	
	void flush()
	{
		timer tm;
		oCnt++;
		fwrite(buf, sizeof(edgeS), cnt, outFile);
		cnt = 0;
		tWrite += tm.getTime();
	}
	
};






void toTxt(char *sName)
{
	
	char *dName = (char *)malloc(1+strlen(sName) + 15);
	sprintf(dName, "%s.to.txt", sName);
	
	inBuf sBuf(sName);
	sBuf.start();
	FILE * dFile = fopen(dName, "w");
	for (edge e;!sBuf.isEnd;)
	{
		sBuf.nextEdge(e);
		fprintf(dFile, "%d %d %d\n", e.x, e.y, e.w);
	} 
	fclose(dFile);
	
}




void toTxtL(char *sName)
{
	
	char *dName = (char *)malloc(1+strlen(sName) + 15);
	sprintf(dName, "%s.to.txt", sName);
	
	inBufL sBuf(sName);
	sBuf.start();
	FILE * dFile = fopen(dName, "w");
	for (edgeL e;!sBuf.isEnd;)
	{
		sBuf.nextEdge(e);
		fprintf(dFile, "%lld %lld %lld\n", e.x, e.y, e.w);
	} 
	fclose(dFile);
	
}








