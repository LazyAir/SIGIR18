#include "io_ud.h"
#include <vector>
using namespace std;

long long n;

int queryCnt;

char * txtName, * degName, * outName, *inName;

inBufS * labelx, * labely;

inBufS labelin, labelout;
inBufL deg;
edgeL ex, ey;
edgeS sx, sy;

FILE * f1, * f2;

timer tmm;

//int xy[100000][2];

long long insize, outsize;

int query(long long x, long long y)
{
	if (x == y) return 0;

	deg.fseek(x);
	deg.nextOne(ex);
	
	deg.fseek(y);
	deg.nextOne(ey);	
	
	x = ((ex.x<<32)>>32);
	y = ((ey.x<<32)>>32);
		
	if (x > y)
	{
		labelx = &labelout;
		labely = &labelin;
		labelout.fseek(ex.w);
		labelin.fseek(ey.y);
	}
	else
	{
		int xy = x; x = y; y = xy;
		labelx = &labelin;
		labely = &labelout;
		labelin.fseek(ey.y);
		labelout.fseek(ex.w);
	}

	labelx->start();
	labely->start();

	
	
	
	int ans = 10000;
	
	labelx->nextEdge(sx);
	labely->nextEdge(sy);
	
	if (sx.x != -1 && sy.x != -1)
	while (sx.x < y)
	{
		if (sx.x == sy.x) 
		{
			ans = ans>(sx.w + sy.w)?(sx.w + sy.w):ans;
			labelx->nextEdge(sx);
			labely->nextEdge(sy);
			if (sx.x == -1) break;
			if (sy.x == -1) break;
		}
		else if (sx.x < sy.x)
		{
			labelx->nextEdge(sx);
			if (sx.x == -1) break;
		}
		else 
		{
			labely->nextEdge(sy);
			if (sy.x == -1) break;
		}	
	}
	
	while (sx.x != -1 && sx.x < y) 
	{
		labelx->nextEdge(sx);
	}
	if (sx.x == y) ans = ans>sx.w?sx.w:ans;
	
	return ans;
}



/*
const int N = 66000;
int dis[N], h[N], hh, pos[N], pre[N];

vector<vector<pair<int,int> > > a;

void up(int x)
{
	for (int j;x > 1; x = j)
	{
		j = (x >> 1);
		if (dis[h[j]] <= dis[h[x]]) break;
		swap(h[j], h[x]);
		pos[h[j]] = j;
		pos[h[x]] = x;
	}
}

void down(int x)
{
	for (int j; (x << 1) <= hh; x = j)
	{
		j = (x << 1);
		if (j < hh && dis[h[j]] > dis[h[j+1]]) j++;
		if (dis[h[x]] <= dis[h[j]]) break;
		swap(h[j], h[x]);
		pos[h[j]] = j;
		pos[h[x]] = x;
	}
}





void check()
{
	FILE * inFile = fopen(txtName, "r");
	a.resize(n);
	for (int i = 0; i < n; i++)
		a[i].resize(0);
	int nn;
	fscanf(inFile, "%d", &nn);
	
	for (int x, y, w; fscanf(inFile, "%d%d%d", &x, &y, &w)!=EOF;)
	{
		a[x].push_back(make_pair(y,w));
	}
	
	timer tm;
	double ss = 0;
	for (int ii = 0; ii < queryCnt; ii++)
	{
		int x = rand()%n;
		printf("%d %d ", ii, x);
		memset(dis, -1, sizeof(dis));
		memset(pos, -1, sizeof(pos));
		dis[x] = 0;
		h[hh = 1] = x; pos[x] = 1;
		memset(pre, -1, sizeof(pre));
		
		while (hh > 0)
		{
			int p = h[1];
			h[1] = h[hh--]; pos[h[1]] = h[1]; down(1);
			
			for (int i = 0, s = a[p].size(), j, w; i < s; i++)
			{
				j = a[p][i].first; 
				w = a[p][i].second;
				if (dis[j] == -1 || dis[j] > w + dis[p])
				{
					pre[j] = p;
					dis[j] = w + dis[p];
					if (pos[j] == -1)
					{
						pos[j] = ++hh;
						h[hh] = j;
					}
					up(pos[j]);
				}
			}
		}
		
		
		
		for (int i = 0; i < n; i++)
		{
			tm.restart();
			int ans = query(x, i);
			ss += tm.getTime();
			if (ans == 10000) ans = -1;
			if (dis[i] != ans) 
			{
				printf("error %d %d %d %d\n", x, i, dis[i], ans);
//				int ii = i;				while (i > -1)				{					printf("%d %lld\n", i, ((deg[i].x<<32)>>32));					i = pre[i];				}				fc = 1;				query(x, ii);
				
				exit(0);
			}
			
		}
		printf("%lf (ms)\n", ss*1000/((ii+1)*n));	
	}	
}


*/

int main(int argc, char ** argv)
{
	txtName = argv[1];
	degName = (char*)malloc(1+strlen(txtName) + 50);
	sprintf(degName, "%s.deg", txtName);
	
	inName = (char*)malloc(1+strlen(txtName) + 50);
	sprintf(inName, "%s.labelin", txtName);
	outName = (char*)malloc(1+strlen(txtName) + 50);
	sprintf(outName, "%s.labelout", txtName);

	timer tm;

	n = checkB(degName)/sizeof(edgeL);
	
	deg.init(degName);
	labelin.perBuf = labelout.perBuf = 1000;	
	if (argc > 3) labelin.perBuf = labelout.perBuf = atoi(argv[3]);
	labelin.init(inName);
	labelout.init(outName);
	
	printf("load %lf (ms)\n", tm.getTime()*1000);

	double sumTime = 0;
	
	queryCnt = 1000;
	if (argc > 2) queryCnt = atoi(argv[2]);
	
	insize = checkB(inName);
	outsize = checkB(outName);

//	check();	return 0;
		
//	printf("%d %d %d\n", labelin.perBuf, labelout.perBuf, queryCnt);

	printf("%d\n", n);

	inBufS labelin(inName), labelout(outName);

	for (int i = 0; i < queryCnt; i++)
	{		
		int x = rand()%n, y = rand()%n;

		tm.restart();
		
		query(x, y);
		sumTime += tm.getTime();

//		printf("%d %d %d %d\n", i, x, y, ans);
	}
	
	printf("avg %lf (ms)\n", sumTime*1000/queryCnt);
	
	free(degName);
	free(inName);
	free(outName);
	
	return 0;
}














