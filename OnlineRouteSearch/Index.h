/**
Author: Hongwei Liang
Description: The Index.h and Index.cpp files are mainly for loading inputs, building offline indices, and retrieving 
	query related subindices and POI candidates in the online phase.
**/

#ifndef INDEX_H_INCLUDED
#define INDEX_H_INCLUDED

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <deque>
#include <string.h>
#include <map>
#include <unordered_map>
#include <unordered_set>

using namespace std;

typedef int wtype; //data type for edge weight and budget.
typedef float decmtype; //data type for feature rating, keyword preference weight, threshold.


/***Structure Definitions************************/
struct Hop2Label {
	int ancid;  // the pivot node id
	wtype dist; // the distance from the source node to the pivot node

	Hop2Label(int aid, wtype dis) {
		ancid = aid;
		dist = dis;
	}
};

struct FtrNodePair{
	int nodeId;  //POI id having this feature
	decmtype ftrVal; //feature value on this feature

	FtrNodePair(int nid, decmtype val){
	    nodeId = nid;
	    ftrVal = val;
	}
};

typedef unordered_map<int, vector<FtrNodePair> > KwdsMap; //for FI index: <keyword_id, list of nodes having this feature>


struct UserQuery{
	int startPOI;
	int endPOI;
	wtype budget;
	decmtype thres;  //threshold for feature value, i.e., theta
	vector <int> keywds;
	vector<decmtype> keywds_weights;
	decmtype alpha;  //can change to a vector of alpha if we want to specify different alpha for each feature

	UserQuery(int sid, int eid, wtype b, decmtype thr, vector<int> kwds, vector<decmtype> kwds_wt, decmtype al){
		startPOI = sid;
		endPOI = eid;
		budget = b;
		thres = thr;
		keywds = kwds;
		keywds_weights = kwds_wt;
		alpha = al;
	}
};


/***Global variables Declaration************************/
extern string root;  //directory of the dataset
extern long long node_num;	//number of POI
extern vector<vector<Hop2Label> > HI_orig; // HI index: stores the list of 2 hop labels for each POI
extern KwdsMap FI; //FI index
extern vector< vector<decmtype> > POIFtMx;  // POI-Feature  matrix
extern int fSpaceSz; //feature space size
extern vector<wtype> stayTimeVec;   //A more general graph can use double
extern wtype **travelTime; // 2-d matrix for pair-wise least traveling cost
extern wtype INF; //a large enough integer
extern vector<wtype> nodeCost;  //each cost equals to stay cost + the cost of (min_in_edge + min_out_edge)/2, used for upper bound estimation
extern vector<wtype> tourCostsVec; //each cost equals to travelTime[source][i] + stay cost of i + travelTime[i][destination], only useful in the Greedy algorithm


//Parameters for each trip query, defined as globals for easy access across files
extern int g_startNode;
extern int g_endNode;
extern wtype g_budget;
extern decmtype g_val_thres;
extern vector<int> g_kwds;
extern vector<decmtype> g_kwds_wts;
extern decmtype g_alpha;  //power law factor.  if 0, simply sum; if very large, simply max
extern vector<double> PLCoeff; //diminishing factor vector

extern int g_s_pos, g_t_pos; //the cut-off positions for source and destination POIs
extern unordered_map<int, int> HI_Q_Pos; //subindex HI_Q, but we only record the cut-off positions of HI, instead of storing the labels again
extern wtype **cmptdPair; // matrix for optimizing the distance computation, records the threshold less than which remaining budget the pair is not reachable

/***Functions Declaration************************/
bool LbAscend(Hop2Label label1, Hop2Label label2); //sort the 2-hop labels in ascending order as distance
void LoadHop2Labels(string labelFileStr, string remainFileStr); //use the data structure and functions in io_ud.h
void PrintHop2Labels(string of_str);
int LoadLocationKeywords(string poiKwdsFileStr);
int LoadInvertedKeywords(string ivtKwdsFileStr);
int LoadStayTime(string stayFileStr);
void LoadInputs(string data_header);
void initPowerLaw(decmtype param); //compute the diminishing factors for each rank
wtype CompTransitCost(int sId, int t_Id, int s_pos, int t_pos);  //least traveling cost computation
int TruncateHop2Label(vector<Hop2Label>& inputLabel, wtype bdgt); //truncate 2-hop labels by budget to speed up traveling cost computation
int RetrieveCandidates(UserQuery uq);


#endif // INDEX_H_INCLUDED
