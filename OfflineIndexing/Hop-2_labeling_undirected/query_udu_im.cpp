#include "io_ud.h"
#include <vector>
using namespace std;

long long n;

int queryCnt;

char * txtName, * degName, * outName;

edgeL * deg;
edgeS * labelout;
edgeS * label_long, * label_short;

vector<int> pivotMap;

bool fc = 1;

int query(int xx, int yy)
{
	if (xx == yy) return 0;
	//int xx = x, yy = y;

	int x_deg, y_deg;

	x_deg = ((deg[xx].x<<32)>>32);
	y_deg = ((deg[yy].x<<32)>>32);
	
// /*	
	if (fc)
	{
	printf("x_deg = %d y_deg = %d\n", x_deg, y_deg);	printf("deg[xx].x = %lld deg[xx].y = %lld deg[xx].w = %lld  deg[yy].x = %lld deg[yy].y = %lld deg[yy].w = %lld\n", deg[xx].x, deg[xx].y, deg[xx].w, deg[yy].x, deg[yy].y, deg[yy].w);
	}
// */	

	if (x_deg > y_deg)
	{
		label_long = labelout + deg[xx].w;
		label_short = labelout + deg[yy].y;
	}
	else
	{
		int xy = x_deg; x_deg = y_deg; y_deg = xy;   // exchange x_deg and y_deg
		label_long = labelout + deg[yy].y;
		label_short = labelout + deg[xx].w;
	}
	

	
	int ans = 100000, i = 0, j = 0;
	
	
	
	cout << "labels for longger :";
	int kk = 0;
	while (label_long[kk].x != -1){
		cout << "("  << pivotMap[label_long[kk].x] << " "<< label_long[kk].w << ") ";
		++kk;
	}
	cout << endl;
		
	cout << "labels for shorter :";
	kk = 0;
	while (label_short[kk].x != -1){
		cout << "("  << pivotMap[label_short[kk].x] << " "<< label_short[kk].w << ") ";
		++kk;
	}
	cout << endl;

	
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
	
	if (label_long[i].x != -1 && label_short[j].x != -1)
	while (label_long[i].x < y_deg)
	{
		//cout << "i = " << i << endl;
		//printf("---%d %d %d %d\n", label_long[i].x, label_long[i].w, label_short[j].x, label_short[j].w);
		if (label_long[i].x == label_short[j].x) 
		{
			ans = ans>(label_long[i].w + label_short[j].w)?(label_long[i].w + label_short[j].w):ans;
			if (label_long[++i].x == -1) break;
			if (label_short[++j].x == -1) break;
		}
		else if (label_long[i].x < label_short[j].x)
		{
			if (label_long[++i].x == -1) break;
		}
		else if (label_short[++j].x == -1) break;
	}
	
	while (label_long[i].x != -1 && label_long[i].x < y_deg) i++;
	if (label_long[i].x == y_deg) ans = ans>label_long[i].w?label_long[i].w:ans;
	
		
	return ans;
}


void loadIndex()
{
	inBufL degBuf(degName);
	inBufS outLabel(outName);
	
	n = checkB(degName)/sizeof(edgeL);

	deg = (edgeL *)malloc(sizeof(edgeL)*n);
	labelout = (edgeS*)malloc(checkB(outName));

	pivotMap.resize(n);
	
	printf("%lld\n", n);

	degBuf.start();
	for (int i = 0; i < n; i++){
		degBuf.nextEdge(deg[i]);
		//cout << "load deg i = " << i << "; deg[i].x = " << ((deg[i].x<<32)>>32)
		//	<< " deg[i].y = " << deg[i].y << " deg[i].w = " << deg[i].w << endl;
		pivotMap[((deg[i].x<<32)>>32)] = i;
	}
	
	outLabel.start();
	for (int i = 0; !outLabel.isEnd; i++){
		outLabel.nextEdge(labelout[i]);
		//cout << "label i = " << i << "; labelout[i].x = " << labelout[i].x 
		//	 << " labelout[i].w = " << labelout[i].w << endl;
	}
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
		a[y].push_back(make_pair(x,w));
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

*/


void reindex(){
	/* store the number of labels of each vertex */
	int *labelnum = (int*)malloc(sizeof(int)*n);
	edgeS *label;
	for(int i=0; i<n; i++) {
		int num = 0;
		label = labelout + deg[i].y;
		while(label[num].x!=-1) {
			num++;
		}
		labelnum[i] = num;
	}

	/* read vertices and total number of labels */
	int *nodes = (int*)malloc(sizeof(int)*n);
	int labelnumber = 0;
	for(int i=0; i<n; i++) {
		labelnumber += labelnum[i];
	}

	/* read labels  */
	edge *labels = (edge*)malloc(sizeof(edge)*labelnumber);
	int labindex = 0;
	for(int i=0; i<n; i++) {
		cout << "labels for " << i << ": ";
		label = labelout + deg[i].y;
		for(int j=0; j<labelnum[i]; j++) {
			edge newlab;
			newlab.x = i;
			newlab.y = pivotMap[label[j].x];
			newlab.w = label[j].w;
			cout << "(" << i << " " << newlab.y << " " << newlab.w << ") ";
			labels[labindex] = newlab;
			labindex++;
		}
		cout << endl<< endl<< endl<< endl;
	}
	

}

int main(int argc, char ** argv)
{
	txtName = argv[1];
	degName = (char*)malloc(1+strlen(txtName) + 50);
	sprintf(degName, "%s.deg", txtName);
	
	outName = (char*)malloc(1+strlen(txtName) + 50);
	sprintf(outName, "%s.out", txtName);

	timer tm;
	
	loadIndex();
	
	reindex();
	
	printf("load %lf (ms)\n", tm.getTime()*1000);
	

	double sumTime = 0;
	
		
	queryCnt = 10;
	if (argc > 2) queryCnt = atoi(argv[2]);

//	check();	return 0;	fc = 1;
	
	srand(time(NULL));
	int cnt = 0;
	for (int i = 0; i < queryCnt; i++)
	{
		int x = rand()%n, y = rand()%n;
		tm.restart();
		int ans = query(x, y);
		
		cout << "(x y) = " << x << " " << y << "; ans = " << ans << endl << endl;
		sumTime += tm.getTime();
	}
	
	printf("avg %lf (ms)\n", sumTime*1000/queryCnt);
	
	free(degName);
	free(outName);
	
	free(deg);
	free(labelout);
	
	return 0;
}














