#include <cstdio>
#include <cstring>
#include <sys/time.h>
#include <sys/resource.h>
#include "io_ud.h"

//const int inf = 2000000000;

using namespace std;

long long n;
long long m;
char *txtName, *binName;

void copyFile(char * sName, char * dName)
{
	char * buf = (char *)malloc(bufSize);
	size_t size;
	
	FILE * sFile = fopen(sName, "rb");
	FILE * dFile = fopen(dName, "wb");
	
	while (size = fread(buf, 1, bufSize, sFile))
		fwrite(buf, 1, size, dFile);
	fclose(sFile);
	fclose(dFile);	
	free(buf);
}

void binarize(char ** argv)
{
//	n = atoi(argv[1]);
	txtName = argv[1];
	binName = (char*)malloc(1+strlen(txtName) + 4);
	strcpy(binName, txtName);
	strcat(binName, ".bin");

	FILE *txtFile = fopen(txtName, "r");
	outBuf binBuf(binName);

	m = 0;
	fscanf(txtFile, "%lld", &n);
	edge tmp;
	int xy, x, y, w;

	while (fscanf(txtFile, "%d%d%d", &x, &y, &w) != EOF)
	{
		tmp.x = x; tmp.y = y; tmp.w = w;
		binBuf.insert(tmp);
		m++;
	}
	fclose(txtFile);

}

void deDu(char* sName, long long &m)
{
	char* tmpName = (char*)malloc(1+strlen(sName)+6);
	strcpy(tmpName, sName);
	strcat(tmpName, ".tmpdu");
	long long mm = 0;
	
	inBuf sBuf(sName);
	sBuf.start();
	outBuf tmpBuf(tmpName);
	long long i = 0;
	int px = -1, py = -1;
	for (edge tmp; i < m; i++)
	{
		sBuf.nextEdge(tmp);

		if (tmp.x == px && tmp.y == py) continue;
		px = tmp.x; py = tmp.y;
		if (tmp.x == tmp.y) continue;
		mm++;
		tmpBuf.insert(tmp);
	}
	m = mm;
	remove(sName);
	rename(tmpName, sName);	
}

void swapEdge(char *binName)
{
	inBuf edgeBuf(binName);
	edgeBuf.start();
	
	char* tmpName = (char*)malloc(1+strlen(binName) + 4);
	strcpy(tmpName, binName);
	strcat(tmpName, ".tmp");
	outBuf tmpBuf(tmpName);
	
	edge tmp;
	int xy;
	while (!edgeBuf.isEnd)
	{
		edgeBuf.nextEdge(tmp);
		xy = tmp.x; tmp.x = tmp.y; tmp.y = xy;
//		swap(tmp.x, tmp.y);
		tmpBuf.insert(tmp);
	}
	
	remove(binName);
	rename(tmpName, binName);

	free(tmpName);	
}


void labelX(char * degName, char *binName, long long &m)
{
	inBuf edgeBuf(binName), degBuf(degName);
	edgeBuf.start(); degBuf.start();
	
	char* tmpName = (char*)malloc(1+strlen(binName)+4);
	strcpy(tmpName, binName);
	strcat(tmpName, ".tmp");
	outBuf tmpBuf(tmpName);
	
	edge tmp, tmpDeg;
	tmpDeg.x = -1;
	
	while (!edgeBuf.isEnd)
	{
		edgeBuf.nextEdge(tmp);
		
		while (tmpDeg.x < tmp.x)
			degBuf.nextEdge(tmpDeg);
		tmpBuf.insert(tmpDeg.y, tmp.y, tmp.w);

	}
	
	remove(binName);
	rename(tmpName, binName);

	free(tmpName);	
	
}


void rankSwapDeg(char * degName)
{
	inBufL degBuf(degName);
	degBuf.start();
	
	char* tmpName = (char*)malloc(1+strlen(degName)+4);
	strcpy(tmpName, degName);
	strcat(tmpName, ".tmp");
	outBuf tmpBuf(tmpName);
	
	edgeL tmp;
	for (int i = 0, xx; i < n ; i++)
	{
		degBuf.nextEdge(tmp);
		tmpBuf.insert(tmp.y, n-1-i, 0);
		
	}
	
	remove(degName);
	rename(tmpName, degName);

	free(tmpName);	
	
	
}





void relabelV()
{
	char * outName = binName;
	char * inName = (char *)malloc(1+strlen(binName) + 3);
	sprintf(inName, "%s.in", binName);
	char * outTmpName = (char *)malloc(1+strlen(outName) + 4);
	sprintf(outTmpName, "%s.tmp", outName);
	char * inTmpName = (char *)malloc(1+strlen(inName) + 4);
	sprintf(inTmpName, "%s.tmp", inName);	
	char * delName = (char *)malloc(1+strlen(binName) + 4);
	sprintf(delName, "%s.del", binName);

	char *degName = (char*)malloc(1+strlen(txtName) + 4);
	sprintf(degName, "%s.deg", txtName);	
	
	xSort(outName, m, false);
printf("%lld\n", checkSize(outName));
	deDu(outName, m);

printf("%lld\n", checkSize(outName));
	copyFile(outName, inName);	
printf("%lld\n", checkSize(inName));
	swapEdge(inName);
	xSort(inName, m, false);

	long long nn = n, mm = m, ntmp;
	
	long long delSize = (n-1)/8+1;
	if (delSize > memSize) delSize = memSize;
	unsigned char * memDel = (unsigned char *)malloc(delSize);

	bool isFirst = 1;
	
	int maxn = -1, minn = n;
	while (1)
	{
		edge ei, eo, eii, eoo;
		printf("n nn %lld %lld\n", n, nn);
		ntmp = nn;
		long long d[3] = {0,0,0};
		{
			inBuf inEdge(inName), outEdge(outName);
			outBuf delBuf(delName);
			outBufL degBuf(degName);
			inEdge.start(); outEdge.start();
			for (int i = 0, ci, co; i < n; i++)
			{
				for (ci = 0; !inEdge.isEnd && i == inEdge.buf[inEdge.cnt].x; ci++)
				{
					eii = ei;
					inEdge.nextEdge(ei);
					maxn = max(ei.x, maxn);					maxn = max(ei.y, maxn);					minn = min(ei.x, minn);					minn = min(ei.y, minn);
				}
				for (co = 0; !outEdge.isEnd && i == outEdge.buf[outEdge.cnt].x; co++)
				{
					eoo = eo;
					outEdge.nextEdge(eo);
					maxn = max(eo.x, maxn);					maxn = max(eo.y, maxn);					minn = min(eo.x, minn);					minn = min(eo.y, minn);
				}
				
				if (ci+co < 3) d[ci+co]++;
				
				if (0)
				if (ci + co == 1 || ci == 2 && co == 0 || co == 2 && ci == 0 ) 
				{
					nn--;

					if (ci > 0) delBuf.insert(i, ei.y, -ei.w);
					if (ci > 1) delBuf.insert(i, eii.y, -eii.w);
					if (co > 0) delBuf.insert(i, eo.y, eo.w);
					if (co > 1) delBuf.insert(i, eoo.y, eoo.w);
				}
				degBuf.insert(ci*(long long)co, i, 0);
				
			}
		}
		
		
		printf("degree 0 1 2 %lld %lld %lld\n", d[0], d[1], d[2]);
		nn -= d[0];

		if (nn == ntmp) break;
		isFirst = 0;
		
		for (long long di = 0, delta = memSize * 8; di < n; di += delta)
		{
			inBuf inEdge(inName), outEdge(outName), delBuf(delName);	
			memset(memDel, 0, delSize);
			int ss = 0;
			for (delBuf.start();!delBuf.isEnd;)
			{
				delBuf.nextEdge(ei);				
				if (ei.x >= di && ei.x < di+delta) 
				{
					memDel[ei.x/8] |= (1<<(ei.x%8));
				}
			}
			
			{
				outBuf inTmpEdge(inTmpName), outTmpEdge(outTmpName);
				for (inEdge.start(); !inEdge.isEnd;)
				{
					inEdge.nextEdge(ei);
					if (!( ei.x >= di && ei.x < di+delta &&  (memDel[ei.x/8]&(1<<(ei.x%8)))!=0 || 
							ei.y >= di && ei.y < di+delta && (memDel[ei.y/8]&(1<<(ei.y%8)))!=0 ))
						inTmpEdge.insert(ei);
				}

				for (outEdge.start(); !outEdge.isEnd;)
				{
					outEdge.nextEdge(ei);
					
					if (!( ei.x >= di && ei.x < di+delta && ((memDel[ei.x/8]&(1<<(ei.x%8)))!=0) ) 
					  && !(ei.y >= di && ei.y < di+delta && ((memDel[ei.y/8]&(1<<(ei.y%8)))!=0) ))
						outTmpEdge.insert(ei);
				}
			}
			
			remove(inName); rename(inTmpName, inName);
			remove(outName); rename(outTmpName, outName);
		}

		break;
	
	}
	
	free(memDel); 

	xSortL(degName, n);

	rankSwapDeg(degName);

	xSort(degName, n, false);

	m = checkSize(outName);
	printf("nn m %lld %lld\n", nn, m );
	
	labelX(degName, outName, m);

	swapEdge(outName);
	xSort(outName, m, false);
	labelX(degName, outName, m);

	swapEdge(outName);
	xSort(outName, m, false);
	
	free(outTmpName); 
	free(inName); 
	free(inTmpName);

	free(degName);

}

void init(char * binName, char *oldOutName, char *oldInName, long long &iCnt)
{
	inBuf edgeBuf(binName);

	outBuf oldOutBuf(oldOutName), oldInBuf(oldInName);
	
	edgeBuf.start();

	int xy;
	for (edge e; !edgeBuf.isEnd;)
	{
		edgeBuf.nextEdge(e);
		if (e.x < e.y)
		{
			iCnt++;
			xy = e.x; e.x = e.y; e.y = xy;
			oldInBuf.insert(e);
		}
		else oldOutBuf.insert(e);
		
	}

}


void filter(char * xName)
{
	inBuf xBuf(xName);
	char *tmpName = (char*) malloc(1+strlen(xName) + 4);
	strcpy(tmpName, xName);
	strcat(tmpName, ".tmp");
	outBuf tmpBuf(tmpName);
	
	xBuf.start();
	for (edge e; !xBuf.isEnd;)
	{
		xBuf.nextEdge(e);
		if (e.x <= e.y) continue;
		tmpBuf.insert(e);
	}
//	return;
	remove(xName);
	rename(tmpName, xName);
	free(tmpName);

}





//genNew(newOutName, gInName, canOutName, isEnd);


void genNew(char * newName, char * inName, char * outAnsName, bool &isEnd)
{
	int perMem = memSize / 2 / sizeof(edge);
	edge * mem = (edge *) malloc(perMem * sizeof(edge) );
	
	int iperMem = memSize / 2 / sizeof(edgeS);
	edgeS * imem = (edgeS *) malloc(iperMem * sizeof(edgeS) );
	int ioldCnt, iallCnt, iduCnt;
	
	char * outTmpName = (char *)malloc(1+strlen(outAnsName) + 4);
	sprintf(outTmpName, "%s.tmp", outAnsName);

	inBuf newEdge(newName), inEdge(inName);
	
	newEdge.start();	inEdge.start();
	
	int phead, ptail, lasthead = 0, lasttail = 0, lasthead_ = 0, lasttail_ = 0, xy, ix;
	int xid, yid, mid, xxid, yyid, tid, stid;
	
	bool isCon;
	
	for (edge en, eo, ei;!newEdge.isEnd || lasthead < lasthead_/* || lastmid < lasttail*/; )
	{
		printf("%d %d %d %d\n", !inEdge.isEnd, !newEdge.isEnd, lasthead < lasthead_, lasttail_ < lasttail);

		phead = 0; ptail = perMem;
		
		for (int i = lasthead; i < lasthead_; i++)
			mem[phead++] = mem[i];
		for (int i = lasttail-1; i >= lasttail_; i--)
			mem[--ptail] = mem[i];
		
		printf("%d %d, %d %d, %d %d\n", phead, ptail, lasthead, lasthead_, lasttail, lasttail_);
		
		while (phead < ptail && !newEdge.isEnd)
		{
			if (!inEdge.isEnd && inEdge.buf[inEdge.cnt].x < newEdge.buf[newEdge.cnt].x)
			{
				if (phead > 0 && inEdge.buf[inEdge.cnt].x == mem[phead-1].x)
					inEdge.nextEdge(mem[--ptail]);
				else inEdge.nextEdge();
			}
			else
			{
				newEdge.nextEdge(mem[phead++]);
			}
		}
		
		if (phead > 0)
			while  (phead < ptail && !inEdge.isEnd && inEdge.buf[inEdge.cnt].x == mem[phead-1].x )
				inEdge.nextEdge(mem[--ptail]);

		printf("%d %d, %d %d, %d %d\n", phead, ptail, lasthead, lasthead_, lasttail, lasttail_);
		
		
		
		if (!newEdge.isEnd && phead > 0 && newEdge.buf[newEdge.cnt].x == mem[phead-1].x)
		{
			if (ptail < perMem && mem[phead-1].x == mem[ptail].x)
			{
				printf("error phead ptail\n");
				for (int i = phead - 10; i < phead; i++)
					printf("%d %d %d\n", mem[i].x, mem[i].y, mem[i].w);
				for (int i = ptail; i < perMem; i++)
					printf("%d %d %d   ==\n", mem[i].x, mem[i].y, mem[i].w);
				exit(0);
			}
						
			lasthead_ = phead;
			while (phead > 0 && newEdge.buf[newEdge.cnt].x == mem[phead-1].x)
				phead--;
			lasthead = phead;
			lasttail = lasttail_ = 0;
		}
		else if (!inEdge.isEnd && ptail < perMem && inEdge.buf[inEdge.cnt].x == mem[ptail].x)
		{
			lasttail_ = ptail;
			while (ptail < perMem && inEdge.buf[inEdge.cnt].x == mem[ptail].x)
				ptail++;
			lasttail = ptail;
			lasthead_ = phead;
			while (phead > 0 && inEdge.buf[inEdge.cnt].x == mem[phead-1].x)
				phead--;
			lasthead = phead;
		}
		else
		{
			lasthead_ = lasthead = lasttail_ = lasttail = 0;
		}

		printf("%d %d, %d %d, %d %d\n", phead, ptail, lasthead, lasthead_, lasttail, lasttail_);

		for (int i = ptail; i < perMem; i++)
		{
			xy = mem[i].x; mem[i].x = mem[i].y; mem[i].y = xy;
		}
		sort(mem+ptail, mem+perMem);
		tid = ptail;
		
		isCon = (!newEdge.isEnd || lasthead < lasthead_);

		swap(outTmpName, outAnsName);
				
		{
			inBuf outTmpEdge(outTmpName);
			outBuf outAnsEdge(outAnsName);
			outTmpEdge.start();

			while (!outTmpEdge.isEnd)
			{
				timer tm;
				tm.restart();

				if (tid < perMem && outTmpEdge.buf[outTmpEdge.cnt].x > mem[tid].x)
				{
					ix = mem[tid].x;
					ioldCnt = iduCnt = iallCnt = 0;
				}
				else
				{
					ioldCnt = iperMem; iallCnt = 0;
					ix = outTmpEdge.buf[outTmpEdge.cnt].x;

					while (!outTmpEdge.isEnd && ix == outTmpEdge.buf[outTmpEdge.cnt].x)
					{
						outTmpEdge.nextEdge(imem[iallCnt]);
											
						if (imem[iallCnt].w > 0)
						{
							imem[--ioldCnt] = imem[iallCnt];
						}
						if (iallCnt >= ioldCnt) 
						{
							printf("error inner! %d %d\n", ioldCnt, iallCnt); exit(0);
						}
						iallCnt++;
					}

					iduCnt = iallCnt;

					for (int i = ioldCnt; i < iperMem; i++)
					{
						for (xid = 0, yid = phead - 1, mid = -1; xid <= yid; )
							if (mem[(xid+yid)>>1].x == imem[i].x) 
							{
								mid = ((xid+yid)>>1);
								yid = mid-1;
							}	
							else if (mem[(xid+yid)>>1].x > imem[i].x) yid = ((xid+yid)>>1)-1;
							else xid = ((xid+yid)>>1)+1;

						if (mid == -1) continue;

						for (int j = mid; j < phead && mem[j].x == imem[i].x; j++)
						{

						
							for (xid = 0, yid = iallCnt - 1; xid <= yid;)
								if (imem[(xid+yid)>>1].x == mem[j].y)
								{
									if ( abs(imem[(xid+yid)>>1].w) > mem[j].w+imem[i].w ) imem[(xid+yid)>>1].w = -(mem[j].w+imem[i].w);
									break;
								}
								else if (imem[(xid+yid)>>1].x > mem[j].y) yid = ((xid+yid)>>1)-1;
								else xid = ((xid+yid)>>1)+1;
								
							if (xid > yid)
							{ 
								if (iduCnt >= ioldCnt)
								{
									printf("error inner! %d %d %d\n", ioldCnt, iallCnt, iperMem);
									exit(0);
								}
								imem[iduCnt].x = mem[j].y;
								imem[iduCnt++].w = -(mem[j].w + imem[i].w);
								if (iduCnt == ioldCnt)
								{
									sort(imem, imem+iduCnt);
									iallCnt = 1;
									for (int k = 1; k < iduCnt; k++)
										if (imem[k].x != imem[k-1].x) imem[iallCnt++] = imem[k];
									iduCnt = iallCnt;
								}
							}

						}
						
						
					}

				
					sort(imem, imem+iduCnt);
					iallCnt = 1;
					for (int k = 1; k < iduCnt; k++)
						if (imem[k].x != imem[k-1].x) imem[iallCnt++] = imem[k];				
					iduCnt = iallCnt;
				}
				
				
				if (tid >= perMem || ix != mem[tid].x) stid = -1;
				else
				{
					stid = tid;
					tid++;
					while (tid < perMem && mem[tid].x == mem[tid-1].x) tid++;
				}
				
				
				if (stid != -1)
				for (int i = stid; i < perMem && mem[i].x == ix; i++)
				{

					
					for (xid = 0, yid = phead-1, mid = -1; xid <= yid;)
						if (mem[(xid+yid)>>1].x == mem[i].y) 
						{
							mid = ((xid+yid)>>1);
							yid = mid-1;
						}
						else if (mem[(xid+yid)>>1].x > mem[i].y) yid = ((xid+yid)>>1)-1;
						else xid = ((xid+yid)>>1)+1;


					if (mid != -1)
					{
						for (int j = mid; j < phead && mem[j].x == mem[i].y; j++ )
						{
							if (ix > mem[j].y)
							{
							
							
								for (xxid = 0, yyid = iallCnt - 1; xxid <= yyid; )
									if (imem[(xxid+yyid)>>1].x == mem[j].y)
									{
										if (abs(imem[(xxid+yyid)>>1].w) > mem[j].w + mem[i].w) imem[(xxid+yyid)>>1].w = -(mem[j].w + mem[i].w);
										break;
									}
									else if (imem[(xxid+yyid)>>1].x > mem[j].y) yyid = ((xxid+yyid)>>1)-1;
									else xxid = ((xxid+yyid)>>1)+1;


								if (xxid > yyid)
								{								
									if (iduCnt >= iperMem)
									{
										printf("error inner\n" );
									}
									
									imem[iduCnt].x = mem[j].y;
									imem[iduCnt++].w = -(mem[i].w + mem[j].w);
									if (iduCnt == iperMem)
									{
										sort(imem, imem+iduCnt);
										iallCnt = 1;
										for (int k = 1; k < iduCnt; i++)
											if (imem[k].x != imem[k-1].x) imem[iallCnt++] = imem[k];
										iduCnt = iallCnt;
									}
								}

							}
							
						}
					}

											
																
				}

				
				sort(imem, imem+iduCnt);
	
				if (iduCnt > 0) 
				{
					if (isCon || imem[0].w < 0)
					{
						outAnsEdge.insert(ix, imem[0].x, imem[0].w);
						if (imem[0].w < 0) isEnd = 0;
					}
				}

				for (int k = 1; k < iduCnt; k++)
				{
					if (imem[k].x != imem[k-1].x)
					if (isCon || imem[k].w < 0)
					{
						outAnsEdge.insert(ix, imem[k].x, imem[k].w);
						if (imem[k].w < 0) isEnd = 0;
					}
				}
				
			}
						
		}
		
		
	}
	
	remove(outTmpName);
	
	if (strlen(outAnsName) > strlen(outTmpName))
	{
		rename(outAnsName, outTmpName);
		free(outAnsName);
	}
	else
	{
		free(outTmpName);
	}
	free(mem);
	free(imem);	
	
}


bool checkDu(char * fName)
{
	inBuf fBuf(fName);
	edge e, pre;
	pre.x = pre.y = -1;
	
	int cnt = 0;
	for (fBuf.start(); !fBuf.isEnd;)
	{
		fBuf.nextEdge(e);
		if (e.x == pre.x && e.y == pre.y) 
		{
			printf("%d %d %d\n", e.x, e.y, e.w);
			return 1;
		}
		pre = e;
	}
	return 0;
}


//prune(canOutName, oldInName, oldOutTmpName, newOutName, isEnd);


void prune(char * dataName, char * oldName, char * scanName, char * allName, char * newName, bool & isEnd, int maxDeg, int & maxDegTmp)
{	
	
	printf("Prune can old scan %lld %lld %lld\n", checkSize(dataName), checkSize(oldName), checkSize(scanName) );
	
	inBuf dataBuf(dataName), scanBuf(scanName), oldBuf(oldName);
	outBuf allBuf(allName), newBuf(newName);

	int dhead, dtail, sCnt, sxid, syid, mid, sid;
	int edgePerMem = memSize/2/sizeof(edge);
	edge * memData = (edge*)malloc(edgePerMem * sizeof(edge));
	edge * memScan = (edge*)malloc(edgePerMem * sizeof(edge));

	int minx, maxx, miny, maxy;
	
	int lasthead_ = 0, lasthead = 0, lasttail_ = 0, lasttail = 0, ix;
	

	for (dataBuf.start(), oldBuf.start(); !dataBuf.isEnd || !oldBuf.isEnd || lasthead < lasthead_ || lasttail_ < lasttail;)
	{
		dhead = 0; dtail = edgePerMem;

		for (int i = lasthead; i < lasthead_; i++)
			memData[dhead++] = memData[i];
		for (int i = lasttail-1; i >= lasttail_; i--)
			memData[--dtail] = memData[i];
		
		while( dhead < dtail && (!oldBuf.isEnd || !dataBuf.isEnd) )
		{
			if (dataBuf.isEnd || 
			!oldBuf.isEnd && (oldBuf.buf[oldBuf.cnt].x < dataBuf.buf[dataBuf.cnt].x || 
			oldBuf.buf[oldBuf.cnt].x == dataBuf.buf[dataBuf.cnt].x && oldBuf.buf[oldBuf.cnt].y < dataBuf.buf[dataBuf.cnt].y)) 
				oldBuf.nextEdge(memData[dhead++]);			
			else if (oldBuf.isEnd || 
			!dataBuf.isEnd && (dataBuf.buf[dataBuf.cnt].x < oldBuf.buf[oldBuf.cnt].x || 
			oldBuf.buf[oldBuf.cnt].x == dataBuf.buf[dataBuf.cnt].x && dataBuf.buf[dataBuf.cnt].y < oldBuf.buf[oldBuf.cnt].y))			
			{
				dataBuf.nextEdge(memData[--dtail]);
				if (memData[dtail].w < 0) memData[dtail].w = -memData[dtail].w;
				else dtail++;
			}
			else 
			{
				if (oldBuf.buf[oldBuf.cnt].w <= abs(dataBuf.buf[dataBuf.cnt].w))
				{
					oldBuf.nextEdge(memData[dhead++]);
					dataBuf.nextEdge();
				}
				else
				{
					dataBuf.nextEdge(memData[--dtail]);
					if (memData[dtail].w < 0) memData[dtail].w = -memData[dtail].w;
					else dtail++;
					oldBuf.nextEdge(memData[dhead++]);
				}
			}
		}
		
				
		if (dhead > 0 && dtail < edgePerMem) ix = max(memData[dhead-1].x, memData[dtail].x);
		else if (dhead > 0) ix = memData[dhead-1].x;
		else ix = memData[dtail].x;
		

		
		printf("%d : %d %d %d %d %d %d\n", ix, dhead, dtail, lasthead, lasthead_, lasttail, lasttail_);
		
		lasthead_ = lasthead = lasttail_ = lasttail = 0;

		if (dhead == dtail && (!dataBuf.isEnd && dataBuf.buf[dataBuf.cnt].x == ix || !oldBuf.isEnd && oldBuf.buf[oldBuf.cnt].x == ix) )
		{
			lasthead_ = dhead;
			while (dhead > 0 && memData[dhead-1].x == ix) dhead--;
			lasthead = dhead;
			lasttail_ = dtail;
			while (dtail < edgePerMem && memData[dtail].x == ix) dtail++;
			lasttail = dtail;
		}
		
		fseeko64(scanBuf.inFile, 0, SEEK_SET);
		
		for (scanBuf.start();!scanBuf.isEnd;)
		{
			minx = miny = n;
			maxx = maxy = -1;
			for (sCnt = 0; sCnt < edgePerMem && !scanBuf.isEnd;sCnt++)
			{
				scanBuf.nextEdge(memScan[sCnt]);
				if (memScan[sCnt].x < minx) minx = memScan[sCnt].x;
				else if (memScan[sCnt].x > maxx) maxx = memScan[sCnt].x;
				if (memScan[sCnt].y < miny) miny = memScan[sCnt].y;
				else if (memScan[sCnt].y > maxy) maxy = memScan[sCnt].y;
			}
			
			timer tm1;

			int ii = 0;
			for (int j = edgePerMem-1, jj; j >= dtail; j--)
			if (memData[j].x > 0 && memData[j].y >= minx && memData[j].y <= maxx)			
			{
				
				while (ii < dhead && memData[ii].x < memData[j].x) ii++;
				if (ii >= dhead) break;
				if (memData[ii].x > memData[j].x) continue;
				
				for (sid = sCnt, sxid = 0, syid = sCnt - 1; sxid <= syid;)
				{
					c2++;
					mid = ((sxid+syid)>>1);
					if (memData[j].y == memScan[mid].x)
					{
						sid = mid;
						syid = mid-1;
					}
					else if (memData[j].y < memScan[mid].x)
						syid = mid-1;
					else sxid = mid+1;					
				}
				
				if (sid == sCnt || memScan[sid].y >= memData[j].x) continue;
				
				for (int i = ii;i < dhead && memData[i].x == memData[j].x && memData[i].y < memData[j].y; i++)
				{
					c2++;
					while (sid < sCnt && memScan[sid].x == memData[j].y && memScan[sid].y < memData[i].y) 
					{
						c2++;
						sid++;
					}
					if (sid == sCnt ||  memScan[sid].x != memData[j].y || memScan[sid].y >= memData[j].x) break;
					if (memScan[sid].y == memData[i].y && memData[j].w - memData[i].w >= memScan[sid].w)
					{
						c3++;
						memData[j].x = -memData[j].x;
						break;
					}						
				}
				
			}
			
			
			tPruneCPU += tm1.getTime();
		}
		
		
		int prex = -1, preCnt = 0;
		
		for (int i = 0, j = edgePerMem-1; i < dhead || j >= dtail;)
		{
			if (i < dhead && j >= dtail)
			{
				if (memData[i].x < abs(memData[j].x) || memData[i].x == abs(memData[j].x) && memData[i].y < memData[j].y) 
				{
					if (memData[i].x != prex)
					{
						maxDegTmp = max(maxDegTmp, preCnt);
						preCnt = 1;
						prex = memData[i].x;
					}
					else preCnt++;
					allBuf.insert(memData[i++]);
				}
				else
				{
					if (memData[j].x > 0) 
					{
						if (memData[j].x != prex)
						{
							maxDegTmp = max(maxDegTmp, preCnt);
							preCnt = 1;
							prex = memData[j].x;
						}
						else preCnt++;
						allBuf.insert(memData[j]);
						newBuf.insert(memData[j]);
						isEnd = 0;
					}
					j--;
				}
			}
			else if (i < dhead)
			{
					if (memData[i].x != prex)
					{
						maxDegTmp = max(maxDegTmp, preCnt);
						preCnt = 1;
						prex = memData[i].x;
					}
					else preCnt++;
				allBuf.insert(memData[i++]);
			}
			else
			{
				if (memData[j].x > 0) 
				{
					if (memData[j].x != prex)
					{
						maxDegTmp = max(maxDegTmp, preCnt);
						preCnt = 1;
						prex = memData[j].x;
					}
					else preCnt++;
					allBuf.insert(memData[j]);
					newBuf.insert(memData[j]);
					isEnd = 0;
				}
				j--;
			}
			
		}
		
		maxDegTmp = max(maxDegTmp, preCnt);
		
	}
	
	free(memData);
	free(memScan);
	
}


void pruneSmall(char * canName, char * oldName, char * scanName, char * allName, char * newName, bool & isEnd, int maxDeg, int & maxDegTmp)
{
	
	printf("PruneSmall can old scan %lld %lld %lld\n", checkSize(canName), checkSize(oldName), checkSize(scanName) );
	
	inBuf canEdge(canName), oldEdge(oldName), scanEdge(scanName) ;
	outBuf allEdge(allName), newEdge(newName);
	
	int iperMem = maxDeg;
	int perMem = (memSize - iperMem*(long long)sizeof(edgeS))/sizeof(edge);
	edge *mem = (edge *)malloc(perMem * sizeof(edge));
	edgeS *imem = (edgeS *)malloc(iperMem * (long long)sizeof(edgeS));

	int phead, ptail, lasthead_ = 0, lasthead = 0, lasttail = 0, lasttail_ = 0, prehead, prex = -1, degCnt = 0;
	int sxid, syid, sid, ix;
	
	long long lastPos;
	edge laste;
	
	for (canEdge.start(), oldEdge.start();!canEdge.isEnd || !oldEdge.isEnd || lasttail_ < lasttail ;)
	{

		lastPos = ftello64(oldEdge.inFile) - sizeof(edge)*(long long)(oldEdge.bufLen-oldEdge.cnt);

		prehead = phead = 0; ptail = perMem;
		
printf("%d %d %d %d %d %d (%lld)\n", phead, ptail, lasthead, lasthead_, lasttail, lasttail_, lastPos/sizeof(edge));
		
		if (lasttail_ < lasttail)
		{
			if (lasthead != lasthead_)
			{
				edge eo;
				eo.x = eo.y = -1;
				while (eo.x != mem[lasthead_-1].x || eo.y != mem[lasthead_-1].y)
				{
					if (oldEdge.isEnd)
					{
						printf("error oldEdge\n");
						exit(0);
					}
					oldEdge.nextEdge(eo);
				}	
			}
			
			for (int i = lasthead; i < lasthead_; i++)
				mem[phead++] = mem[i];
			for (int i = lasttail-1; i >= lasttail_; i--)
				mem[--ptail] = mem[i];				
		}
		else
		{
			while (!canEdge.isEnd)
				if (canEdge.buf[canEdge.cnt].w < 0)	
				{
					canEdge.nextEdge(mem[--ptail]);
					prehead = phead;
					break;
				}
				else canEdge.nextEdge();
		}
		

		if (ptail == perMem)
		{
			edge eo;
			while (!oldEdge.isEnd)
			{
				oldEdge.nextEdge(eo);
				allEdge.insert(eo);
				if (eo.x != prex)
				{
					maxDegTmp = max(maxDegTmp, degCnt);
					degCnt = 1;
					prex = eo.x;
				}
				else degCnt++;
			}
			maxDegTmp = max(maxDegTmp, degCnt);
			
			break;

		}
		


		while (!oldEdge.isEnd && phead < ptail)
		{

			if (oldEdge.buf[oldEdge.cnt].x == mem[ptail].x && oldEdge.buf[oldEdge.cnt].y <= mem[ptail].y) 
			{
				if (oldEdge.buf[oldEdge.cnt].y < mem[ptail].y)
				{
					oldEdge.nextEdge(mem[phead++]);
				}
				else 
				{
					if (-mem[ptail].w >= oldEdge.buf[oldEdge.cnt].w)
					{
						ix = mem[ptail].x;
						ptail++;
						bool ff = 1;
						while (!canEdge.isEnd)
							if (canEdge.buf[canEdge.cnt].w < 0)	
							{
								ff = 0;							
								canEdge.nextEdge(mem[--ptail]);
								if (ix != mem[ptail].x) phead = prehead;
								break;
							}
							else canEdge.nextEdge();
										
						if (ff) break;
					}
					else oldEdge.nextEdge();
				}
			}
			else if (oldEdge.buf[oldEdge.cnt].x < mem[ptail].x)
			{
				oldEdge.nextEdge();
			}
			else
			{
				bool ff = 1;
				while (!canEdge.isEnd)
					if (canEdge.buf[canEdge.cnt].w < 0)	
					{
						ff = 0;
						canEdge.nextEdge(mem[--ptail]);
						prehead = phead;
						break;
					}
					else canEdge.nextEdge();
				if (ff) break;
			}
		}

		
printf("%d : %d %d(%d) %d %d %d %d\n", mem[ptail].x, phead, ptail, perMem-ptail, lasthead, lasthead_, lasttail, lasttail_);
		
		if (!oldEdge.isEnd) laste = oldEdge.buf[oldEdge.cnt];
		else laste.x = laste.y = -1;
		
		if (!oldEdge.isEnd && phead == ptail && ptail < perMem && oldEdge.buf[oldEdge.cnt].x == mem[ptail].x)
		{
			lasthead_ = phead;
			while (phead > 0 && mem[phead-1].x == mem[ptail].x) phead--;
			lasthead = phead;

			if (lasthead_ != lasthead) laste = mem[lasthead];

			lasttail_ = ptail;
			while (ptail < perMem && mem[ptail].x == mem[lasttail_].x) ptail++;
			lasttail = ptail;
		}
		else lasthead = lasthead_ = lasttail = lasttail_ = 0;
		
printf("%d : %d %d(%d) %d %d %d %d\n", mem[ptail].x, phead, ptail, perMem-ptail, lasthead, lasthead_, lasttail, lasttail_);
		
		for (int i = ptail, xy; i < perMem; i++)
		{
			xy = mem[i].x;	mem[i].x = mem[i].y; mem[i].y = xy;
		}	
		sort(mem+ptail, mem+perMem);
		
			
		fseeko64(scanEdge.inFile, 0, SEEK_SET);
		
		
		timer tm;
		
		scanEdge.start(); 
		for (int i = ptail, si; i < perMem && !scanEdge.isEnd;)
		{
			ix = mem[i].x;
			while (!scanEdge.isEnd && scanEdge.buf[scanEdge.cnt].x < mem[i].x)
				scanEdge.nextEdge();
			
				
			if (scanEdge.isEnd || scanEdge.buf[scanEdge.cnt].x > ix) 
			{
				i++;
				continue;
			}
			
			
			for (si = 0; !scanEdge.isEnd && scanEdge.buf[scanEdge.cnt].x == ix; )
			{
				if (si == iperMem) 
				{
					printf("error! iperMem\n");
					exit(0);
				}
				scanEdge.nextEdge(imem[si++]);
			}
						
			for (; i < perMem && mem[i].x == ix; i++)
			{
					
				for (sid = -1, sxid = 0, syid = phead-1; sxid <= syid;)
				{
					c2++;
					if ( mem[(sxid+syid)>>1].x == mem[i].y )
					{
						sid = ((sxid+syid)>>1);
						syid = sid-1;						
					}
					else if ( mem[(sxid+syid)>>1].x > mem[i].y )
					{
						syid = ((sxid+syid)>>1)-1;				
					}
					else sxid = ((sxid+syid)>>1)+1;				
				}
				
				
				if (sid == -1) continue;
				
			
				for (int jj = 0, j = sid; j < phead && mem[j].x == mem[i].y && mem[j].y < mem[i].x; j++)
				{
					
					c2++;
					while (jj < si && imem[jj].x < mem[j].y) 
					{
						c2++;
						jj++;
					}
					if (jj == si) break;
					
					if (imem[jj].x == mem[j].y && mem[j].w+imem[jj].w <= -mem[i].w)
					{
						c3++;
						mem[i].w = -mem[i].w;
						break;
					}
								
				}				
				
			}
			
			
		}
		
		tPruneCPU += tm.getTime();
		
		fseeko64(oldEdge.inFile, lastPos, SEEK_SET);
		
		
		for (int i = ptail, xy; i < perMem; i++)
		{
			xy = mem[i].x;	mem[i].x = mem[i].y;	mem[i].y = xy;
		}
		
		sort(mem+ptail, mem+perMem);
		edge eo;
		oldEdge.start();
		int i;
		for (i = ptail; !oldEdge.isEnd && (oldEdge.buf[oldEdge.cnt].x != laste.x || oldEdge.buf[oldEdge.cnt].y != laste.y);)
		{
			oldEdge.nextEdge(eo);
			while (i < perMem && mem[i] < eo)
			{
				if (mem[i].x == eo.x && mem[i].y == eo.y) 
				{
//					printf("%d %d %d %d\n", eo.x, eo.y, eo.w, mem[i].w);
					eo.w = -abs(eo.w);
				}
				if (mem[i].w < 0)
				{
					mem[i].w = -mem[i].w;
					allEdge.insert(mem[i]);
					if (mem[i].x != prex)
					{
						maxDegTmp = max(maxDegTmp, degCnt);
						degCnt = 1;
						prex = mem[i].x;
					}
					else degCnt++;
					newEdge.insert(mem[i]);
					isEnd = 0;
				}
				i++;
			}
			if (eo.w > 0)
			{
				allEdge.insert(eo);			
				if (eo.x != prex)
				{
					maxDegTmp = max(maxDegTmp, degCnt);
					degCnt = 1;
					prex = eo.x;
				}
				else degCnt++;
			}
			
		}
		
		if (!oldEdge.isEnd)
			while (i < perMem && mem[i] < oldEdge.buf[oldEdge.cnt])
			{
				if (mem[i].w < 0)
				{
					mem[i].w = -mem[i].w;
					allEdge.insert(mem[i]);
					if (mem[i].x != prex)
					{
						maxDegTmp = max(maxDegTmp, degCnt);
						degCnt = 1;
						prex = mem[i].x;
					}
					else degCnt++;
					newEdge.insert(mem[i]);
					isEnd = 0;
				}	
				i++;
			}
		
		maxDegTmp = max(maxDegTmp, degCnt);
		
	
	}
	
	maxDegTmp = max(maxDegTmp, degCnt);
	
	
	free(imem);
	free(mem);
}



//reconstruct IO/buf... to be done

void compress(char * longName, char * shortName, char * degName)
{
	inBuf longBuf(longName);
	outBufS shortBuf(shortName);
	outBufL degBuf(degName);
	
	longBuf.start();
	int prex = 0;
	long long cnt = 0;
	degBuf.insert(cnt, 0, 0);
	for (edge e; !longBuf.isEnd; )
	{
		longBuf.nextEdge(e);
		while (prex < e.x) 
		{
			shortBuf.insert(-1, 0);
			cnt++;
			degBuf.insert(cnt, 0, 0);
			prex++;
		}
		shortBuf.insert(e.y, e.w);
		sumCnt++;
		cnt++;
	}
	while (prex < n) 
	{
		shortBuf.insert(-1, 0);
		if (prex == n-1) break;
		cnt++;
		degBuf.insert(cnt, 0, 0);
		prex++;
	}
}

void appendInOut(char * degName, char * degInName, char * degOutName, char * ansName)
{
	inBuf degBuf(degName);
	inBufL degInBuf(degInName), degOutBuf(degOutName);
	outBufL ansBuf(ansName);
	
	degBuf.start(); degInBuf.start(); degOutBuf.start();
	edge e;
	edgeL d1, d2;
	long long tmp;
	
	for (int i = 0; i < n; i++)
	{
		degBuf.nextEdge(e);
		degInBuf.nextEdge(d1);
		degOutBuf.nextEdge(d2);
		tmp = e.y;
		tmp = (e.x | (tmp<<32LL));
		ansBuf.insert(tmp, d1.x, d2.x);
	}
	
	
}

void getMinMax(char * sName)
{
	inBuf sBuf(sName);
	sBuf.start();
	int mi = 10000, ma = -1;
	for (edge e; !sBuf.isEnd;)
	{
		sBuf.nextEdge(e);
		if (e.w < mi) mi = e.w;
		if (e.w > ma) ma = e.w;
	}
	
	printf("MinMax %s %d %d\n", sName, mi, ma);

}

void showBuf(char * newOutName)
{
	return;
	printf("%s\n", newOutName);
	inBuf eBuf(newOutName);
	edge e;
	for (eBuf.start(); !eBuf.isEnd;)
	{
		eBuf.nextEdge(e);
		printf("%d %d %d\n", e.x, e.y, e.w);
	}
	printf("%s\n", newOutName);
}

void mergeCan(char * canName, char * oldName, char * ansName)
{
	inBuf canEdge(canName), oldEdge(oldName);
	
	outBuf ansEdge(ansName);
	
	canEdge.start();
	oldEdge.start();
	edge ec, eo;
	ec.x = eo.x = n;
	if (!canEdge.isEnd) canEdge.nextEdge(ec);
	if (!oldEdge.isEnd) oldEdge.nextEdge(eo);
	
	while (ec.x < n || eo.x < n)
	{
		if (ec.x < eo.x || ec.x == eo.x && ec.y < eo.y)
		{
			ansEdge.insert(ec);
			if (canEdge.isEnd) ec.x = n;
			else canEdge.nextEdge(ec);
		}
		else if (eo.x < ec.x || eo.x == ec.x && eo.y < ec.y)
		{
			ansEdge.insert(eo);
			if (oldEdge.isEnd) eo.x = n;
			else oldEdge.nextEdge(eo);
		}
		else
		{
			if (eo.w > abs(ec.w)) eo = ec;
			ansEdge.insert(eo);
			if (canEdge.isEnd) ec.x = n;
			else canEdge.nextEdge(ec);
			if (oldEdge.isEnd) eo.x = n;
			else oldEdge.nextEdge(eo);
		}
		
	}
	
	

}


int checkMaxDeg(char * sName)
{
	inBuf sBuf(sName);
	int prex = -1, cnt = 0, maxCnt = 0;
	sBuf.start();
	for (edge e; !sBuf.isEnd; sBuf.isEnd)
	{
		sBuf.nextEdge(e);
		if (e.x != prex) 
		{
			maxCnt = max(maxCnt, cnt);
			cnt = 1;
			prex = e.x;
		}
		else cnt++;
	}
	
	return max(maxCnt, cnt);	
}



void initPart(char * oldName, char * gName, char * newName, int iter)
{
	edge e;
	char * partName = (char *)malloc(1+strlen(oldName) + 5);
	sprintf(partName, "%s.part", oldName);

	printf("here %s %s %s\n", oldName, gName, newName);
	{
		inBuf oldEdge(oldName);
		outBuf partEdge(partName), gEdge(gName), newEdge(newName);
	
		for (oldEdge.start(); !oldEdge.isEnd;)
		{
			oldEdge.nextEdge(e);
			if (e.w <= iter+1) 
			{
				partEdge.insert(e);
				if (e.w == iter+1) newEdge.insert(e);
				if (e.w == 1) gEdge.insert(e);			
			}
		}
	}
	
	remove(oldName); rename(partName, oldName);
	
	free(partName);

}

void run()
{
	timer tRun;
	
	char * gInName = (char *)malloc(1+strlen(txtName) + 5);
	sprintf(gInName, "%s.gin", txtName);
	char * newInName = (char *)malloc(1+strlen(txtName) + 7);
	sprintf(newInName, "%s.newin", txtName);
	char * oldInName = (char *)malloc(1+strlen(txtName) + 7);
	sprintf(oldInName, "%s.oldin", txtName);
	char * canInName = (char *)malloc(1+strlen(txtName) + 7);
	sprintf(canInName, "%s.canin", txtName);
	char * oldInTmpName = (char *)malloc(1+strlen(txtName) + 10);
	sprintf(oldInTmpName, "%s.oldintmp", txtName);
	
	char * gOutName = (char *)malloc(1+strlen(txtName) + 5);
	sprintf(gOutName, "%s.gout", txtName);
	char * newOutName = (char *)malloc(1+strlen(txtName) + 7);
	sprintf(newOutName, "%s.newout", txtName);
	char * oldOutName = (char *)malloc(1+strlen(txtName) + 7);
	sprintf(oldOutName, "%s.oldout", txtName);
	char * canOutName = (char *)malloc(1+strlen(txtName) + 7);
	sprintf(canOutName, "%s.canout", txtName);
	char * oldOutTmpName = (char *)malloc(1+strlen(txtName) + 10);
	sprintf(oldOutTmpName, "%s.oldouttmp", txtName);
	
	bool isEnd = 0;
	
	long long inCnt = 0;

	init(binName, oldOutName, oldInName, inCnt);
	xSort(oldInName, inCnt, false);
	copyFile(oldInName, gInName);
	copyFile(oldInName, newInName);

	copyFile(oldOutName, gOutName);
	copyFile(oldOutName, newOutName);
	
	
//	iter = 10;	initPart(oldOutName, gOutName, newOutName, iter);	initPart(oldInName, gInName, newInName, iter);
	

	int maxOutDeg = checkMaxDeg(oldOutName), maxInDeg = checkMaxDeg(oldInName);
	
	tInit = tRun.getTime();
	
if (ftime) printf("tInit %lf\n", tInit);

	timer tTotal; 
	tTotal.restart();
	
	double tMerge = 0, tMergeS = 0;
	
	while (!isEnd)
	{
		
printf("\n\niteration %d begin\n", ++iter);	

//generate

printf("newOut/In oldOut/In\n%lld %lld %lld %lld %lld\n", checkSize(newOutName), checkSize(newInName), checkSize(oldOutName), checkSize(oldInName), checkSize(oldOutName) + checkSize(oldInName) );

		isEnd = 1;

		getMinMax(newOutName);		getMinMax(newInName);

tRun.restart();
		
		if (iter <= 10)
		{
			copyFile(gOutName, canOutName);
			copyFile(gInName, canInName);
			genNew(newOutName, gInName, canOutName, isEnd);
			genNew(newInName, gOutName, canInName, isEnd);
		}
		else
		{
			copyFile(oldOutName, canOutName);
			copyFile(oldInName, canInName);
			genNew(newOutName, oldInName, canOutName, isEnd);
			genNew(newInName, oldOutName, canInName, isEnd);
		}
		
		
tGenS += (tGen = tRun.getTime());
show(tGen);

tRun.restart();

{			long long tmpSize = checkSize(canOutName)+checkSize(canInName);			if (largeDisk < tmpSize) largeDisk = tmpSize;}

		if (isEnd) break;
tRun.restart();

		isEnd = 1;
		
		int maxOutDegTmp = maxOutDeg, maxInDegTmp = maxInDeg;
		
if (fmerge) printf("prune out begin\n");


		if (checkSize(canOutName) > checkSize(oldInName) ) 
			prune(canOutName, oldOutName, oldInName, oldOutTmpName, newOutName, isEnd, maxInDeg, maxOutDegTmp); 
		else pruneSmall(canOutName, oldOutName, oldInName, oldOutTmpName, newOutName, isEnd, maxInDeg, maxOutDegTmp);
		
		
		
if (fmerge) printf("prune out end\n");

tPrune = tRun.getTime();

show(tPrune);
printf("c1 c2 c3\n%lld %lld %lld\n", c1, c2, c3);


if (fmerge) printf("prune in begin\n");	
		
		if (checkSize(canInName) > checkSize(oldOutName)) 
			prune(canInName, oldInName, oldOutName, oldInTmpName, newInName, isEnd, maxOutDeg, maxInDegTmp);
		else pruneSmall(canInName, oldInName, oldOutName, oldInTmpName, newInName, isEnd, maxOutDeg, maxInDegTmp);

if (fmerge) printf("prune in end\n");	
		
		maxOutDeg = maxOutDegTmp;
		maxInDeg = maxInDegTmp;
		
		
		remove(oldInName); rename(oldInTmpName, oldInName);
		remove(oldOutName); rename(oldOutTmpName, oldOutName);

tPruneS += (tPrune = tRun.getTime());
show(tPrune);
printf("c1 c2 c3\n%lld %lld %lld\n", c1, c2, c3);	
		
		
printf("tGenS tPruneS tPruneCPU tSortS tMergeS tTotal\n%lf %lf %lf %lf %lf %lf\n", tGenS, tPruneS, tPruneCPU, tSortS, tMergeS, tTotal.getTime());
printf("tRead tWrite iCnt oCnt\n%lf %lf %lld %lld\n",tRead, tWrite, iCnt, oCnt);
printf("largeDisk %lld %lf MB\n", largeDisk, largeDisk*sizeof(edge)/1024.0/1024.0);
		
	}
	
	
printf("tGenS tPruneS tPruneCPU tSortS tMergeS tTotal\n%lf %lf %lf %lf %lf %lf\n", tGenS, tPruneS, tPruneCPU, tSortS, tMergeS, tTotal.getTime());
printf("%lf\t%lf\t%lf\t%lf\t%lf\t%lf\n", tGenS, tPruneS, tPruneCPU, tSortS, tMergeS, tTotal.getTime());
printf("largeDisk %lld %lf MB\n", largeDisk, largeDisk*sizeof(edge)/1024.0/1024.0);

	

printf("\n\nindex begin\n");

/*
{
	char * inName = (char *)malloc(1+strlen(txtName) + 12);
	sprintf(inName, "%s.oldintest", txtName);

	char * outName = (char *)malloc(1+strlen(txtName) + 12);	
	sprintf(outName, "%s.oldouttest", txtName);
	
	copyFile(oldInName, inName);
	copyFile(oldOutName, outName);
}
	
*/
	char * inName = (char *)malloc(1+strlen(txtName) + 8);
	sprintf(inName, "%s.labelin", txtName);

	char * outName = (char *)malloc(1+strlen(txtName) + 9);	
	sprintf(outName, "%s.labelout", txtName);

	char *degName = (char*)malloc(1+strlen(txtName) + 4);
	sprintf(degName, "%s.deg", txtName);
	
	char *degInName = (char*)malloc(1+strlen(txtName) + 7);
	sprintf(degInName, "%s.degin", txtName);

	char *degOutName = (char*)malloc(1+strlen(txtName) + 7);
	sprintf(degOutName, "%s.degout", txtName);

	char *tmpDegName = (char*)malloc(1+strlen(degName) + 4);
	sprintf(tmpDegName, "%s.tmp", degName);
	
	sumCnt = 0;
	compress(oldInName, inName, degInName);
	compress(oldOutName, outName, degOutName);
	
	swapEdge(degName);
	xSort(degName, n, false);
	appendInOut(degName, degInName, degOutName, tmpDegName);
	xSortL(tmpDegName, n);
	remove(degName); rename(tmpDegName, degName);
	
	printf("sumCnt indexSize %lld %lf MB\n", sumCnt, double((checkB(outName)+checkB(inName)+checkB(degName)) )/(1024.0*(double)1024) );
	
	
	free(newInName);
	free(oldInName);
	free(canInName);
	free(oldInTmpName);
		
	free(newOutName);
	free(oldOutName);
	free(canOutName);
	free(oldOutTmpName);
	
}















int main(int argc, char ** argv)
{

printf("%s\n", argv[1]);
	
	txtName = argv[1];

	tSortS = 0;
	timer tMain;
	binarize(argv);

	tBi = tMain.getTime();
if (ftime) printf("\n\ntBi %lf\n", tBi);
	
	tMain.restart();
	relabelV();

	tRe = tMain.getTime();

if (ftime) printf("tRe %lf\n", tRe);
	
	tMain.restart();
	run();
	tRun = tMain.getTime();
	
if (fdeb)printf("tRun %lf\n", tRun);
	
	free(binName);
	return 0;
}










