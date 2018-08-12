/**
Author: Hongwei Liang
Description: The RouteSearch.h and RouteSearch.cpp files are for implementing the route search algorithms, including PACER, PACER_SC and Greedy.
**/

#ifndef ROUTESEARCH_H_INCLUDED
#define ROUTESEARCH_H_INCLUDED

#include <iostream>
#include <fstream>
#include <string>
#include <sys/time.h>
#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <iterator>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <queue>
#include <stack>
#include <math.h>
#include <random>

#include "Index.h"

using namespace std;
using std::size_t;
using std::hash;

/***Structure Definitions************************/

struct state{  //the information of each record in C_L
	vector<int> path; //route
	wtype time; //used budget
	int lastID; //ending poi

	state(vector<int> p, wtype t, int l) {
		path = p;
		time = t;
		lastID = l;
	}
};


typedef std::vector<state> stateList;  //C_L

typedef multiset<decmtype, std::greater<decmtype> > DscSet; //a (descending) ordered set of feature ratings on the POIs in the route
typedef unordered_map<int, DscSet> FDscSetMap; // a map <key: feature id, value: the ordered set of feature ratings for this feature>

struct TripSetFeature{
	decmtype totalAggRating; //Gain of a compact state
	unordered_map<int, decmtype> ftrAggRatings;  // phi_k for each feature k: <feature k, aggregated rating for k>
	FDscSetMap fasm;  //intermediate ordered sets of POI ratings for all features, for the purpose of increment computation

	TripSetFeature(){
	}

	TripSetFeature(decmtype r, unordered_map<int, decmtype> ftr, FDscSetMap fm){
		totalAggRating = r;
		ftrAggRatings = ftr;
		fasm = fm;
	}

};

struct CmpctNode{  //compact node
	TripSetFeature tsf;
	stateList sl;

	CmpctNode(){
	}
};


struct SCNode{ //compact node with state collapse
	TripSetFeature tsf;
	vector<int> path;
	wtype time;
	int lastID;

	SCNode(){
	}
};


//-- topTrips
struct topTrips{
	decmtype aggRating;
	wtype cost;
	vector<int> path;
	unordered_map<int, decmtype> ftrAggRatings;

	topTrips(decmtype r, wtype c, vector<int> p, unordered_map<int, decmtype> f){
		aggRating = r;
		cost = c;
		path = p;
		ftrAggRatings = f;
	}
};

typedef std::vector<int> intVector;

struct HashFunc{  //for hashing a set of POIs (intVector)
	size_t operator()(const intVector &key) const{

		size_t seed = 0;

		for(auto it = key.begin(); it != key.end(); ++it){
			seed ^= hash<int>()(*it) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}

		return seed;
	}
};

struct margiRat{ //for computing the upper bound of marginal gain
	int id;
	decmtype margiGain;
	decmtype margiRatio;

	margiRat(int i, decmtype g, decmtype r){
		id = i;
		margiGain = g;
		margiRatio = r;
	}

	margiRat(){
		margiGain = 0;
		margiRatio = 0;
	}
};


/*** Global Variables Declaration**********************************/
extern long long search_space; //for computing the search space, i.e., number of examined routes
extern priority_queue<topTrips> topKqueue; //priority query for the topK results


/***Functions Declaration************************/
long long getCurrentTime();
TripSetFeature CompAggRating(TripSetFeature exFsm, int newPOI); //Incrementally compute the Gain for a set of POIs
bool operator < (topTrips a, topTrips b);
void PACER(vector<int> stack_tail, vector<int> used_nodes); //Method 1: the optimal route search algorithm PACER
void PACER_SC(vector<int> stack_tail, vector<int> used_nodes); //Method 2: the PACER algorithm with state collapse heuristics
void Greedy();  //Method 3: the greedy algorithm, only produces 1 route
void RecTrips(string method, int K);


#endif // ROUTESEARCH_H_INCLUDED
