#include "io_ud.h"
#include <vector>
using namespace std;

long long n;

int queryCnt;

char * txtName, * degName, * outName, *inName;

edgeL * deg;
edgeS * labelout, *labelin;
edgeS * labelx, * labely;

bool fc = 0;

int query(int x, int y)
{
	if (x == y) return 0;
	int xx = x, yy = y;

	x = ((deg[xx].x<<32)>>32);
	y = ((deg[yy].x<<32)>>32);
	
/*	if (fc)
	{
	printf("%d %d\n", x, y);	printf("%lld %lld %lld %lld\n", deg[xx].y, deg[xx].w, deg[yy].y, deg[yy].w);
	}
*/	
	if (x > y)
	{
		labelx = labelout + deg[xx].w;
		labely = labelin + deg[yy].y;
	}
	else
	{
		int xy = x; x = y; y = xy;
		labelx = labelin + deg[yy].y;
		labely = labelout + deg[xx].w;
	}
	
/*	if (fc)
	{
	printf("%d %d\n", x, y);		for (int i = 0; i < 500 && labelx[i].x > -1; i++)		printf("%d %d %d\n", x, labelx[i].x, labelx[i].w);for (int i = 0; i < 500 && labely[i].x > -1; i++)		printf("%d %d %d\n", y, labely[i].x, labely[i].w);
	}
*/	
	int ans = 10000, i = 0, j = 0;
	
/*	
	while (labelx[i].x != -1 || labely[j].x != -1)
	{
		if (labelx[i].x == y) ans = ans>labelx[i].w?labelx[i].w:ans;
		if (labely[j].x == x) ans = ans>labely[j].w?labely[j].w:ans;

		if (labelx[i].x == labely[j].x) 
		{
			ans = ans>(labelx[i].w + labely[j].w)?(labelx[i].w + labely[j].w):ans;
			i++;
			j++;
		}
		else if (labelx[i].x != -1 && (labely[j].x == -1 || labelx[i].x < labely[j].x) ) i++;
		else j++;
	}
	return ans;
*/
	
	if (labelx[i].x != -1 && labely[j].x != -1)
	while (labelx[i].x < y)
	{
//		printf("---%d %d %d %d\n", labelx[i].x, labelx[i].w, labely[j].x, labely[j].w);
		if (labelx[i].x == labely[j].x) 
		{
			ans = ans>(labelx[i].w + labely[j].w)?(labelx[i].w + labely[j].w):ans;
			if (labelx[++i].x == -1) break;
			if (labely[++j].x == -1) break;
		}
		else if (labelx[i].x < labely[j].x)
		{
			if (labelx[++i].x == -1) break;
		}
		else if (labely[++j].x == -1) break;
	}
	
	while (labelx[i].x != -1 && labelx[i].x < y) i++;
	if (labelx[i].x == y) ans = ans>labelx[i].w?labelx[i].w:ans;

	
	return ans;
}


void loadIndex()
{
	inBufL degBuf(degName);
	inBufS inLabel(inName), outLabel(outName);
	
	n = checkB(degName)/sizeof(edgeL);

	deg = (edgeL *)malloc(sizeof(edgeL)*n);
	labelin = (edgeS*)malloc(checkB(inName));
	labelout = (edgeS*)malloc(checkB(outName));

	printf("%lld\n", n);

	degBuf.start();
	for (int i = 0; i < n; i++)
		degBuf.nextEdge(deg[i]);

	inLabel.start();
	for (int i = 0; !inLabel.isEnd; i++)
		inLabel.nextEdge(labelin[i]);
	
	outLabel.start();
	for (int i = 0; !outLabel.isEnd; i++)
		outLabel.nextEdge(labelout[i]);
			
}

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
				int ii = i;
				while (i > -1)
				{
					printf("%d %lld\n", i, ((deg[i].x<<32)>>32));
					i = pre[i];
				}
				
				
				fc = 1;
				query(x, ii);
				
				exit(0);
			}
			
		}
		printf("%lf (ms)\n", ss*1000/((ii+1)*n));	
	}	
}



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
	
	loadIndex();
	
	printf("load %lf (ms)\n", tm.getTime()*1000);
	

	double sumTime = 0;
	
//	for (int i = 0; i < 5; i++) printf("%d %d\n", labelin[i].x, labelin[i].w);
		
	queryCnt = 1000;
	if (argc > 2) queryCnt = atoi(argv[2]);

//	check();	return 0;	fc = 1;
	
	int cnt = 0;
	for (int i = 0; i < queryCnt; i++)
	{
		int x = rand()%n, y = rand()%n;
//		x = 15280, y = 1040;
		tm.restart();
		int ans = query(x, y);
		
//		printf("%d %d %d\n", x, y, ans);
		sumTime += tm.getTime();
	}
	
	printf("avg %lf (ms)\n", sumTime*1000/queryCnt);
	
	free(degName);
	free(inName);
	free(outName);
	
	free(deg);
	free(labelin);
	free(labelout);
	
	return 0;
}














