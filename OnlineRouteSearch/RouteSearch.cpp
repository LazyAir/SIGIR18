/**
Author: Hongwei Liang
Description: The RouteSearch.h and RouteSearch.cpp files are for implementing the route search algorithms, including PACER, PACER_SC and Greedy.
**/

#include "RouteSearch.h"

/*** Global Variables Definitions**********************************/
long long search_space = 0; //for computing the search space, i.e., number of examined routes

/***Initialized for each trip query***/
priority_queue<topTrips> topKqueue; //priority query for the topK results
vector<int> g_Ind;  //a list of the to be considered POI ids, g_ind[i] stores the real POI id
int g_IndSz; //size of the above
unordered_set<int> g_candSet; //candidate set
int g_K; //user input k 

unordered_map<intVector, CmpctNode, HashFunc > CNM; //the map for compact states: only will be used if the method is PACER
unordered_map<intVector, SCNode, HashFunc > SNM; //the map for collapsed compact states: only will be used if the method is PACER-SC

/**Functions Definitions*****************************************/
TripSetFeature CompAggRating(TripSetFeature exTsf, int newPOI){ //Incrementally compute the Gain for a set of POIs
    // this function does not implement the optimization for simply sum, in which case maintaining the order is an unnecessary overhead
    FDscSetMap exFsm = exTsf.fasm; //existing ordered sets of POI ratings for all features
    unordered_map<int, decmtype> exFars = exTsf.ftrAggRatings;
    FDscSetMap newFsm;
    unordered_map<int, decmtype> newFars;
    int inptKwdSz = g_kwds.size();
    newFsm.reserve(inptKwdSz);
    newFars.reserve(inptKwdSz);
    DscSet dss;
    decmtype totalAggRat = 0;
    vector<decmtype> pFVec = POIFtMx[newPOI];

    for(int j  = 0; j < inptKwdSz; j++){
        int k = g_kwds[j]; //get the real keyword k

        if(pFVec[k] >= g_val_thres){  //external global feature value threshold, i.e., theta
	    dss = exFsm[k];
            dss.insert(pFVec[k]);
            newFsm[k] = dss;

            decmtype fAggRat = 0;
            if(g_alpha > 4){  //we assume that when alpha > 4, it indicates the MAX aggregation
                fAggRat = *dss.begin();
            }else{ //compute the aggregated diminishing utility for keyword k
                int inx = 0;
                for(auto it_d = dss.begin(); it_d != dss.end(); it_d++){
                    fAggRat += (*it_d) * PLCoeff[inx];

                    ++inx;
                }
            }
            fAggRat = g_kwds_wts[j] * fAggRat;
            newFars[k] = fAggRat;
            totalAggRat += fAggRat;
        }else{
            newFsm[k] = exFsm[k];
            decmtype exfar = exFars[k];
            newFars[k] = exfar;
            totalAggRat += exfar;
        }
    }

    TripSetFeature tsf = TripSetFeature(totalAggRat, newFars, newFsm);
    return tsf;
}

bool operator < (topTrips a, topTrips b){//for priority queue of top-k results
    if(a.aggRating == b.aggRating){
        return a.cost < b.cost;
    }

    return a.aggRating > b.aggRating;
}

bool operator < (margiRat a, margiRat b){ //for priority queue of marginal ratio
    return a.margiRatio < b.margiRatio;
}

void reportSet(vector<int>& used_nodes){  //for testing
    for(int i = 0; i < used_nodes.size(); i++){
        cout << "+" << used_nodes[i];
    }
    cout << endl;
}


void PACER(vector<int> stack_tail, vector<int> used_nodes){  //The optimal PACER algorithm; the parameters correspond to I and C^- in the pseudo code in the paper
    vector<int> curr_used_nodes; //the compact state C
    while(!stack_tail.empty()){
        int idx = stack_tail.back();
        stack_tail.pop_back();

        curr_used_nodes = used_nodes;
        curr_used_nodes.push_back(g_Ind[idx]);
        int nsz = curr_used_nodes.size();

        int newID;
        stateList newsl;
        TripSetFeature newtsf;
        newtsf.totalAggRating = 0;

        if(nsz == 1){ //corresponds to the first level of the compact states enumeration tree
            newID = g_Ind[idx];
            vector<int> path;
            path.push_back(newID); 
            wtype visitCost = travelTime[g_startNode][newID]; //this traveling time must be computed earlier in RetrieveCandidates phase and < INF
            visitCost += stayTimeVec[newID];
            //Don't need to test whether the remaining budget is enough to go to the destination. Now that the node is added to stack, it passes the candidate selection
            state newState = state(path, visitCost, newID); //a record in C_L
			
            search_space++;
			
            newsl.push_back(newState);
            TripSetFeature empttsf;
            newtsf = CompAggRating(empttsf, newID);

            //update top-k
            wtype tripCost = visitCost + travelTime[newID][g_endNode]; //must be computed earlier in RetrieveCandidates phase and < INF
            topTrips tk = topTrips(newtsf.totalAggRating, tripCost,
                           path, newtsf.ftrAggRatings);  //omit the source and destination in the path, but the cost is for the complete close route
            if(topKqueue.size() == g_K){
                topTrips atrip = topKqueue.top();
                if(tk.aggRating > atrip.aggRating){
                    topKqueue.pop();
                    topKqueue.push(tk);
                }else if(tk.aggRating == atrip.aggRating){
                    if(tripCost < atrip.cost){
                        topKqueue.pop();
                        topKqueue.push(tk);
                    }
                }
            }else{
                topKqueue.push(tk);
            } //update top-k

            CmpctNode newCmptNd;
            newCmptNd.sl = newsl;
            newCmptNd.tsf = newtsf;
            CNM[curr_used_nodes] = newCmptNd;  //add the new compact node with its information to the hash map

            vector<int> nextTail;  //corresponds to the prefix of i in I in the paper
            nextTail.resize(g_IndSz - idx - 1);
            for(int n = idx + 1; n < g_IndSz; n++){
                //nextTail.push_back(n);
                nextTail[n - idx - 1] = n;
            }
            PACER(nextTail, curr_used_nodes);

        }else{
            bool addSetFlag = false;
            vector<int> searchKey;  //search key C^{-i}
            searchKey.resize(nsz - 1);
            vector<int> listBestPath;
            vector<int> newPath;
            auto map_end = CNM.end();

            unordered_set<int> candSet;
            unordered_set<int> realReachable;
            for(int i = 0; i < nsz; i++){
                bool addListItemFlag = false;

                for(int j = 0; j < nsz; j++){  //get the search key C^{-i}
                    if(j < i){
                        searchKey[j] = curr_used_nodes[j];  
                    }

                    if(j > i){
                        searchKey[j - 1] = curr_used_nodes[j];
                    }
                }

                auto map_it = CNM.find(searchKey);
                if( map_it != map_end ){ //found the compact node
                    newID = curr_used_nodes[i];
                    wtype listMinCost = INF;
                    CmpctNode cptn = map_it->second;
                    stateList presl = cptn.sl;
					
		    /*******
			   cost dominance pruning; find the feasible route having the least cost, addListItemFlag = true
		    *******/
                    for(auto slit = presl.begin(); slit != presl.end(); slit++){
                        search_space++;
			int preLastID = (*slit).lastID;
                        wtype remainBgt = g_budget - (*slit).time;
                        if(remainBgt > cmptdPair[preLastID][newID]){
                            wtype visitCost = CompTransitCost(preLastID, newID, HI_Q_Pos[preLastID], HI_Q_Pos[newID])
                                        + stayTimeVec[newID];
                            //travelTime[newID][g_endNode] must be computed earlier in the nodes retrieval phase
                            if(visitCost + travelTime[newID][g_endNode] <= remainBgt ){ //really reachable
								wtype newCost = (*slit).time + visitCost;
                                if(newCost < listMinCost){
                                    newPath = (*slit).path;
                                    newPath.push_back(newID);
                                    listMinCost = newCost;
                                    listBestPath = newPath;
                                    addListItemFlag = true;
                                }
                            }else{
                                cmptdPair[preLastID][newID] = remainBgt; //for optimizing future computation
                            }
                        }
                    }// end C^{-i}_L list enumeration
                    if(addListItemFlag){
                        if(newtsf.totalAggRating == 0){ //if this set info hasn't been computed yet
                            newtsf = CompAggRating(cptn.tsf, newID);
                        }

                        /*******
                           feature-based upper bound pruning; if promising, addSetFlag = true
                        *******/

			if(topKqueue.size() == g_K){
                            wtype rmBgtAftrAdd = g_budget - listMinCost;
                            int newIdPos = TruncateHop2Label(HI_orig[newID], rmBgtAftrAdd);

                            //--retrieve the reachable POIs from newID
                            realReachable.clear();
                            realReachable.reserve(g_IndSz);

                            wtype smallest_fromNew = INF;
                            wtype smallest_toEnd = INF;

                            candSet = g_candSet;
                            for(int j = 0; j < listBestPath.size(); j++){
                                candSet.erase(listBestPath[j]);
                            }

			    /**The following is for finding the online bound as in the Theorem 4 in the paper**/
                            priority_queue<margiRat> margiRatQueue;
                            TripSetFeature addOne;
                            decmtype new_marginal = 0;
                            decmtype new_ratio = 0;

                            for(auto it = candSet.begin(); it != candSet.end(); it++ ){
                                wtype fromCost = CompTransitCost(newID, *it, newIdPos, HI_Q_Pos[*it]);

                                if(fromCost + stayTimeVec[*it] + travelTime[*it][g_endNode] <= rmBgtAftrAdd ){
                                    realReachable.insert(*it);

                                    addOne = CompAggRating(newtsf, *it);
                                    new_marginal = addOne.totalAggRating - newtsf.totalAggRating;
                                    new_ratio = new_marginal * 1.0 / nodeCost[*it];

                                    margiRat mgr = margiRat(*it, new_marginal, new_ratio);
                                    margiRatQueue.push(mgr);

                                    if(fromCost < smallest_fromNew){
                                        smallest_fromNew = fromCost;
                                    }
                                    if(travelTime[*it][g_endNode] < smallest_toEnd){
                                        smallest_toEnd = travelTime[*it][g_endNode];
                                    }
                                }

                            }

                            //--Find the minimum edge cost and node cost respectively for later early judgment
                            wtype setCost = (smallest_fromNew + smallest_toEnd)/2;

                            TripSetFeature collectedTsf = newtsf;
                            wtype mincost = INF;
                            for(auto it = realReachable.begin(); it != realReachable.end(); it++){
                                int nodeid = *it;
                                if(nodeCost[nodeid] < mincost){
                                    mincost = nodeCost[nodeid];
                                }
                            }

                            margiRat top_mgr;
                            //--find the top l-1 top marginal ratio POIs that total cost <= rmBgt
                            while(!margiRatQueue.empty()){
                                if(setCost + mincost > rmBgtAftrAdd){ //an optimization
                                    break;
                                }

                                //select the node that maximize the ratio
                                top_mgr = margiRatQueue.top();
                                margiRatQueue.pop();

                                //check whether the top one satisfy the remain budget constraint
                                if(setCost + nodeCost[top_mgr.id] <= rmBgtAftrAdd){
                                    collectedTsf.totalAggRating += top_mgr.margiGain;
                                    setCost += nodeCost[top_mgr.id];
                                }else{
                                    break;
                                }
                            }

                            //--find the l-th top marginal ratio POI that the budget constraint is just violated after adding it 
                            double lambda = 0;
                            if(!margiRatQueue.empty()){
                                top_mgr = margiRatQueue.top();
                                lambda = (rmBgtAftrAdd - setCost) * 1.000 / nodeCost[top_mgr.id];
                            }

                            //--estimate upper bound
                            decmtype upperBound = collectedTsf.totalAggRating + lambda * top_mgr.margiGain;
                            if(upperBound >= topKqueue.top().aggRating){
                                state newState = state(listBestPath, listMinCost, newID);
                                newsl.push_back(newState);
                                addSetFlag = true;
                            }
                        }else{ //top-k queue not full
                            state newState = state(listBestPath, listMinCost, newID);
                            newsl.push_back(newState);
                            addSetFlag = true;
                        }

                        /*******
                            finish feature pruning part
                        *******/
                    }
                } // end current searchKey
            }//end all search keys

            if(addSetFlag){ //the C_L for this compact node is not empty
                vector<int> nextTail;
                nextTail.resize(g_IndSz - idx - 1);

                wtype addEndListMinCost = INF;
                vector<int> addEndListBestPath;
                for(int i = 0; i < newsl.size(); i++){
                    state s = newsl[i];
                    wtype tripCost = s.time + travelTime[s.lastID ][g_endNode];
                    if(tripCost < addEndListMinCost){
                        addEndListMinCost = tripCost;
                        addEndListBestPath = s.path;
                    }
                }

                //try to update top-k
                topTrips tk = topTrips(newtsf.totalAggRating, addEndListMinCost,
                               addEndListBestPath, newtsf.ftrAggRatings);  //omit the source and destination in the path, but the cost is for the complete close route
                if(topKqueue.size() == g_K){
                    topTrips atrip = topKqueue.top();
                    if(tk.aggRating > atrip.aggRating){
                        topKqueue.pop();
                        topKqueue.push(tk);
                    }else if(tk.aggRating == atrip.aggRating){
                        if(addEndListMinCost < atrip.cost){
                            topKqueue.pop();
                            topKqueue.push(tk);
                        }
                    }
                }else{
                    topKqueue.push(tk);
                } //update top-k

                //add new set info
                CmpctNode newCmptNd;
                newCmptNd.sl = newsl;
                newCmptNd.tsf = newtsf;
                CNM[curr_used_nodes] = newCmptNd;

                for(int n = idx + 1; n < g_IndSz; n++){
                    //nextTail.push_back(n);
                    nextTail[n - idx - 1] = n;
                }
                PACER(nextTail, curr_used_nodes);
            }else{  // empty
                vector<int> nextTail;
                nextTail.reserve(g_IndSz - idx - 1);

                vector<int> existKey;
                existKey.resize(nsz);

                for(int n = idx + 1; n < g_IndSz; n++){
                    bool existFlag = false;
                    for(int i = 0; i < nsz; i++){ //nsz is the size of curr_used_nodes, don't need to consider i = nsz, since the program can be here only if the set is empty
                        for(int j = 0; j < nsz; j++){
                            if(j < i){
                                existKey[j] = curr_used_nodes[j];  //re-write existKey
                            }
                            if(j > i){
                                existKey[j - 1] = curr_used_nodes[j]; //re-write existKey
                            }
                        }
                        //existKey.push_back(g_Ind[n]);
                        existKey[nsz - 1] = g_Ind[n];

                        if(CNM.find(existKey) != map_end ){ //existKey must include g_Ind[n], since i = nsz is not considered
                            existFlag = true;
                            break;
                        }
                    }
                    if(existFlag){
                        nextTail.push_back(n);
                    }
                }

                PACER(nextTail, curr_used_nodes);
            }
        }// end else nsz > 1
    }
}



void PACER_SC(vector<int> stack_tail, vector<int> used_nodes){ //the PACER algorithm with state collapse heuristics
    vector<int> curr_used_nodes;
    while(!stack_tail.empty()){
        int idx = stack_tail.back();
        stack_tail.pop_back();

        curr_used_nodes = used_nodes;
        curr_used_nodes.push_back(g_Ind[idx]);
        int nsz = curr_used_nodes.size();

        int newID;
        TripSetFeature newtsf;
        newtsf.totalAggRating = 0;

        if(nsz == 1){
            newID = g_Ind[idx ];
            vector<int> path;
            path.push_back(newID);
            wtype visitCost = travelTime[g_startNode][newID];
            visitCost += stayTimeVec[newID];

            search_space++;

            TripSetFeature empttsf;
            newtsf = CompAggRating(empttsf, newID);

            //update top-k
            wtype tripCost = visitCost + travelTime[newID][g_endNode]; 
            topTrips tk = topTrips(newtsf.totalAggRating, tripCost,
                           path, newtsf.ftrAggRatings);
            if(topKqueue.size() == g_K){
                topTrips atrip = topKqueue.top();
                if(tk.aggRating > atrip.aggRating){
                    topKqueue.pop();
                    topKqueue.push(tk);
                }else if(tk.aggRating == atrip.aggRating){
                    if(tripCost < atrip.cost){
                        topKqueue.pop();
                        topKqueue.push(tk);
                    }
                }
            }else{
                topKqueue.push(tk);
            } //update top-k

            SCNode newScNd;
            newScNd.tsf = newtsf;
            newScNd.path = path;
            newScNd.time = visitCost;
            newScNd.lastID = newID;
            SNM[curr_used_nodes] = newScNd;

            vector<int> nextTail;
            nextTail.resize(g_IndSz - idx - 1);
            for(int n = idx + 1; n < g_IndSz; n++){
                nextTail[n - idx - 1] = n;
            }
            PACER_SC(nextTail, curr_used_nodes);

        }else{
            bool addSetFlag = false;
            vector<int> searchKey;
            searchKey.resize(nsz - 1);
            wtype setMinCost = INF;
            vector<int> newPath;
            vector<int> setBestPath;
            auto map_end = SNM.end();

            unordered_set<int> candSet;
            unordered_set<int> realReachable;

            int setBestLast;
            
            for(int i = 0; i < nsz; i++){
                for(int j = 0; j < nsz; j++){
                    if(j < i){
                        searchKey[j] = curr_used_nodes[j];  //re-write searchKey
                    }

                    if(j > i){
                        searchKey[j - 1] = curr_used_nodes[j]; //re-write searchKey
                    }
                }

                auto map_it = SNM.find(searchKey);
                if( map_it != map_end ){
					search_space++;
					
                    newID = curr_used_nodes[i];
                    SCNode scn = map_it->second;
                    int preLastID = scn.lastID;
                    wtype remainBgt = g_budget - scn.time;
                    if(remainBgt > cmptdPair[preLastID][newID]){
                        wtype visitCost = CompTransitCost(preLastID, newID, HI_Q_Pos[preLastID], HI_Q_Pos[newID])
                                    + stayTimeVec[newID];
                        if(visitCost + travelTime[newID][g_endNode] <= remainBgt ){ //really reachable
                            wtype newCost = scn.time + visitCost;
                            if(newCost < setMinCost){
                                newPath = scn.path;
                                newPath.push_back(newID);

                                setMinCost = newCost;
                                setBestPath = newPath;
                                setBestLast = newID;
                                addSetFlag = true;

                                if(newtsf.totalAggRating == 0){ //this set info hasn't been computed yet
                                    newtsf = CompAggRating(scn.tsf, newID);
                                }
                            }
                        }else{
                            cmptdPair[preLastID][newID] = remainBgt;
                        }
                    }
                } // end current key
            }//end all keys

            if(addSetFlag){
                vector<int> nextTail;
                nextTail.resize(g_IndSz - idx - 1);

                //try to update top-k
                wtype tripCost = setMinCost + travelTime[setBestLast][g_endNode];

                topTrips tk = topTrips(newtsf.totalAggRating, tripCost,
                               setBestPath, newtsf.ftrAggRatings);
                if(topKqueue.size() == g_K){
                    topTrips atrip = topKqueue.top();
                    if(tk.aggRating > atrip.aggRating){
                        topKqueue.pop();
                        topKqueue.push(tk);
                    }else if(tk.aggRating == atrip.aggRating){
                        if(tripCost < atrip.cost){
                            topKqueue.pop();
                            topKqueue.push(tk);
                        }
                    }
                }else{
                    topKqueue.push(tk);
                } //update top-k

                /*******
                    feature based upper bound pruning, if promising, store info for this set and recursive call
                *******/

                if(topKqueue.size() == g_K){
                    wtype rmBgtAftrAdd = g_budget - setMinCost;
                    int newIdPos = TruncateHop2Label(HI_orig[setBestLast], rmBgtAftrAdd);
                    realReachable.clear();
                    realReachable.reserve(g_IndSz);

                    wtype smallest_fromNew = INF;
                    wtype smallest_toEnd = INF;

                    candSet = g_candSet;
                    for(int j = 0; j < setBestPath.size(); j++){
                        candSet.erase(setBestPath[j]);
                    }

                    priority_queue<margiRat> margiRatQueue;
                    TripSetFeature addOne;
                    decmtype new_marginal = 0;
                    decmtype new_ratio = 0;

                    for(auto it = candSet.begin(); it != candSet.end(); it++ ){
                        wtype fromCost = CompTransitCost(newID, *it, newIdPos, HI_Q_Pos[*it]);

                        if(fromCost + stayTimeVec[*it] + travelTime[*it][g_endNode] <= rmBgtAftrAdd ){
                            realReachable.insert(*it);

                            addOne = CompAggRating(newtsf, *it);
                            new_marginal = addOne.totalAggRating - newtsf.totalAggRating;
                            new_ratio = new_marginal * 1.0 / nodeCost[*it];

                            margiRat mgr = margiRat(*it, new_marginal, new_ratio);
                            margiRatQueue.push(mgr);

                            if(fromCost < smallest_fromNew){
                                smallest_fromNew = fromCost;
                            }
                            if(travelTime[*it][g_endNode] < smallest_toEnd){
                                smallest_toEnd = travelTime[*it][g_endNode];
                            }
                        }
                    }

                    //--Find the minimum edge cost and node cost respectively for later early judgment
                    wtype setCost = (smallest_fromNew + smallest_toEnd)/2;

                    TripSetFeature collectedTsf = newtsf;
                    wtype mincost = INF;
                    for(auto it = realReachable.begin(); it != realReachable.end(); it++){
                        int nodeid = *it;
                        if(nodeCost[nodeid] < mincost){
                            mincost = nodeCost[nodeid];
                        }
                    }

                    margiRat top_mgr;
                    //--find the top l-1 top marginal ratio POIs that total cost <= rmBgt
                    while(!margiRatQueue.empty()){
                        if(setCost + mincost > rmBgtAftrAdd){
                            break;
                        }

                        //select the node that maximize the ratio
                        top_mgr = margiRatQueue.top();
                        margiRatQueue.pop();

                        //check whether the top one satisfy the remain budget constraint
                        if(setCost + nodeCost[top_mgr.id] <= rmBgtAftrAdd){
                            collectedTsf.totalAggRating += top_mgr.margiGain;
                            setCost += nodeCost[top_mgr.id];
                        }else{
                            break;
                        }
                    }

                    //--find the l-th top marginal ratio POI that the budget constraint is just violated after adding it 
                    double lambda = 0;
                    if(!margiRatQueue.empty()){
                        top_mgr = margiRatQueue.top();
                        lambda = (rmBgtAftrAdd - setCost) * 1.000 / nodeCost[top_mgr.id];
                    }

                    //estimate upper bound
                    decmtype upperBound = collectedTsf.totalAggRating + lambda * top_mgr.margiGain;
                    if(upperBound >= topKqueue.top().aggRating){
                        //add new set info
                        SCNode newScNd;
                        newScNd.tsf = newtsf;
                        newScNd.path = setBestPath;
                        newScNd.time = setMinCost;
                        newScNd.lastID = setBestLast;
                        SNM[curr_used_nodes] = newScNd;

                        for(int n = idx + 1; n < g_IndSz; n++){
                            nextTail[n - idx - 1] = n;
                        }
                        PACER_SC(nextTail, curr_used_nodes);

                    }
                }else{ //top-k queue not full
                    //add new set info
                    SCNode newScNd;
                    newScNd.tsf = newtsf;
                    newScNd.path = setBestPath;
                    newScNd.time = setMinCost;
                    newScNd.lastID = setBestLast;
                    SNM[curr_used_nodes] = newScNd;

                    for(int n = idx + 1; n < g_IndSz; n++){
                        nextTail[n - idx - 1] = n;
                    }
                    PACER_SC(nextTail, curr_used_nodes);
                }

                /*******
                    finish the feature based upper bound pruning part
                *******/
            }else{
                vector<int> nextTail;
                nextTail.reserve(g_IndSz - idx - 1);

                vector<int> existKey;
                existKey.resize(nsz);

                for(int n = idx + 1; n < g_IndSz; n++){
                    bool existFlag = false;
                    for(int i = 0; i < nsz; i++){ //nsz is the size of curr_used_nodes, don't need to consider i = nsz, since program can only be here when the set is empty
                        for(int j = 0; j < nsz; j++){
                            if(j < i){
                                existKey[j] = curr_used_nodes[j];  //re-write existKey
                            }

                            if(j > i){
                                existKey[j - 1] = curr_used_nodes[j]; //re-write existKey
                            }
                        }
                        existKey[nsz - 1] = g_Ind[n];

                        if(SNM.find(existKey) != map_end ){ //existKey must include g_Ind[n], since i = nsz is not considered
                            existFlag = true;
                            break;
                        }
                    }
                    if(existFlag){
                        nextTail.push_back(n);
                    }
                }

                PACER_SC(nextTail, curr_used_nodes);
            }
        }// end else nsz > 1
    }
}


 
void Greedy(){   //the greedy algorithm
    unordered_set<int> wholeSet_copy = g_candSet;
    wtype g_tripCost = 0;
    vector<int> g_tripPath;
    TripSetFeature collectedTsf;
    collectedTsf.totalAggRating = 0;
    wtype mincost = INF;
    for(auto it = g_candSet.begin(); it != g_candSet.end(); it++){
        int nodeid = *it;
        if(nodeCost[nodeid] < mincost){
            mincost = nodeCost[nodeid];
        }
    }

    while(!g_candSet.empty()){
        //select the node that maximize the ratio
        if(g_tripCost + mincost > g_budget){
            break;
        }
        decmtype MaxMarginal = 0;
        int MaxMarginalId;
        wtype MaxNodeCost;
        TripSetFeature MaxTsf;
        for(auto wit = g_candSet.begin(); wit != g_candSet.end(); wit++){
            int nid = *wit;
            TripSetFeature newAgg = CompAggRating(collectedTsf, nid);
            decmtype marginal = (newAgg.totalAggRating - collectedTsf.totalAggRating)*1.0 / tourCostsVec[nid]; //notice the difference between tourCostsVec[nid] and nodeCost[nid]
            if(marginal > MaxMarginal){
                MaxMarginal = marginal;
                MaxMarginalId = nid;
                MaxTsf = newAgg;
            }
        }

        //enumerate all possible orders to compute the costs to select the min cost one
        //update g_tripPath and g_tripCost

        vector<int> bestPath;
        wtype leastTripCost = INF;
        int tripsz = g_tripPath.size();
        for(int i = 0; i < tripsz + 1; i++){ //select the position for insertion
            search_space++;
            vector<int> tempPath;
            wtype tempCost = 0;
            for(int j = 0; j < i; j++){
                tempPath.push_back(g_tripPath[j]);
            }
            tempPath.push_back(MaxMarginalId);
            for(int j = i; j < tripsz; j++){
                tempPath.push_back(g_tripPath[j]);
            }
            //g_endNode
            tempCost += CompTransitCost(g_startNode, tempPath[0], HI_Q_Pos[g_startNode], HI_Q_Pos[tempPath[0]]);
            for(int k = 0; k < tripsz + 1; k++){ //new trip
                int currID = tempPath[k];
                tempCost += stayTimeVec[tempPath[k]]; //stay cost
                if(k < tripsz){ //traveling cost
                    int nextID = tempPath[k + 1];
                    tempCost += CompTransitCost(currID, nextID, HI_Q_Pos[currID], HI_Q_Pos[nextID]);
                }
            }
            tempCost += CompTransitCost(tempPath[tripsz], g_endNode, HI_Q_Pos[tempPath[tripsz]], HI_Q_Pos[g_endNode]);

            if(tempCost <= g_budget){
                if(tempCost < leastTripCost){
                    leastTripCost = tempCost;
                    bestPath = tempPath;
                }
            }
        }

        //check whether the max one satisfy the remain budget constraint
        if(leastTripCost <= g_budget){
            collectedTsf = MaxTsf;
            g_tripCost = leastTripCost;
            g_tripPath = bestPath;
        }

        g_candSet.erase(MaxMarginalId);
    }

    //select a node that maximizes tsf.totalAggRating over whole set
    TripSetFeature emptyTsf;
    TripSetFeature singleMaxAgg;
    singleMaxAgg.totalAggRating = 0;
    int singleMaxId;
    for(auto rit = wholeSet_copy.begin(); rit != wholeSet_copy.end(); rit++){
        search_space++;
        int nid = *rit;
        TripSetFeature newAgg = CompAggRating(emptyTsf, nid);
        if(newAgg.totalAggRating > singleMaxAgg.totalAggRating){
            singleMaxId = nid;
            singleMaxAgg = newAgg;
        }
    }

    //compare the collected set aggRating and single max aggRating
    if(singleMaxAgg.totalAggRating > collectedTsf.totalAggRating){
        g_tripPath.clear();
        g_tripPath.push_back(singleMaxId);
        g_tripCost = CompTransitCost(g_startNode, singleMaxId, HI_Q_Pos[g_startNode], HI_Q_Pos[singleMaxId]);
        g_tripCost += stayTimeVec[singleMaxId];
        g_tripCost += CompTransitCost(singleMaxId, g_endNode, HI_Q_Pos[singleMaxId], HI_Q_Pos[g_endNode]);
        collectedTsf = singleMaxAgg;
    }

    topTrips tk = topTrips(collectedTsf.totalAggRating, g_tripCost,
                               g_tripPath, collectedTsf.ftrAggRatings);

    topKqueue.push(tk);	
}



void RecTrips(string method, int K){
    
	for(auto it = HI_Q_Pos.begin(); it != HI_Q_Pos.end(); it++ ){
		g_Ind.push_back(it->first); 
		g_candSet.insert(it->first);
	}
	
	if(method == "P" || method == "S"){
		g_K = K;
		vector<int> stack_tail; //a vector to model a stack
		g_IndSz = g_Ind.size();
		stack_tail.reserve(g_IndSz);
		for(int i = 0; i < g_IndSz; i++){
			stack_tail.push_back(i); //the pushed i is not real poi id, g_Ind[i] maps to the real POI id
		}
		vector<int> used_nodes;
		
		long long reserve_num = 100000000; // a large enough integer for the number of compact states in CNM or SNM
		if (method == "P"){
			CNM.reserve(reserve_num);
			PACER(stack_tail, used_nodes);   //the initial recursive call of PACER
		}else{
			SNM.reserve(reserve_num);
			PACER_SC(stack_tail, used_nodes);   //the initial recursive call of PACER_SC			
		}
	}else{ // method == "G"
		Greedy(); // Greedy() only produces 1 route
	}
}
