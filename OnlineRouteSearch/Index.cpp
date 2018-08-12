/**
Author: Hongwei Liang
Description: The Index.h and Index.cpp files are mainly for loading inputs, building offline indices, and retrieving 
	query related subindices and POI candidates in the online phase.
**/

#include "Index.h"
#include "io_ud.h"

/***Global variables ************************/
string root = "/cs/hongweil/SubmodularRoute/Data/input_data/"; //dir linux
long long num_node; //number of POI
vector<vector<Hop2Label> > HI_orig; // HI index: stores the list of 2 hop labels for each POI
KwdsMap FI; //FI index
vector< vector<decmtype> > POIFtMx;  // POI-Feature  matrix
int fSpaceSz; //feature space size
vector<wtype> stayTimeVec;   //A more general graph can use double
wtype **travelTime; // 2-d matrix for pair-wise least traveling cost
wtype INF = 65535;  //a large enough integer
vector<wtype> nodeCost; //each cost equals to stay cost + the cost of (min_in_edge + min_out_edge)/2, used for upper bound estimation
vector<wtype> tourCostsVec; //each cost equals to travelTime[source][i] + stay cost of i + travelTime[i][destination], only useful in the Greedy algorithm

//Parameters for each trip query, defined as globals for easy access
int g_startNode;
int g_endNode;
wtype g_budget;
decmtype g_val_thres;
vector<int> g_kwds;
vector<decmtype> g_kwds_wts;
decmtype g_alpha;  //power law factor.  if 0, simply sum; if very large, simply max
vector<double> PLCoeff; //diminishing factor vector

int g_s_pos, g_t_pos; //the cut-off positions for source and destination POIs
unordered_map<int, int> HI_Q_Pos;  //subindex HI_Q, but we only record the cut-off positions of HI, instead of storing the labels again
wtype **cmptdPair; // matrix for optimizing the distance computation, records the threshold less than which remain budget the pair is not reachable


/**Functions Definitions*****************************************/
bool LbAscend(Hop2Label label1, Hop2Label label2) { //sort the 2-hop labels in ascending order as distance
	return label1.dist < label2.dist;
}

void LoadHop2Labels(string edgeFileNm) {  //use the data structure and functions in io_ud.h
	/**load original index**/
	const char * edgeNmConst = edgeFileNm.c_str();
	char * edgeName = new char[strlen(edgeNmConst)+1];
	strcpy(edgeName,edgeNmConst);

	char * degName = (char*)malloc(1+strlen(edgeName) + 50);
	sprintf(degName, "%s.deg", edgeName);
	char * outName = (char*)malloc(1+strlen(edgeName) + 50);
	sprintf(outName, "%s.out", edgeName);

	inBufL degBuf(degName);
	inBufS outLabel(outName);

	num_node = checkB(degName)/sizeof(edgeL);
	cout << "In total " << num_node << " nodes." << endl;

	edgeL * deg = (edgeL *)malloc(sizeof(edgeL)*num_node);
	edgeS * labelout = (edgeS*)malloc(checkB(outName));

	vector<int> pivotMap(num_node);

	degBuf.start();
	for (int i = 0; i < num_node; i++){
		degBuf.nextEdge(deg[i]);
		//cout << "load deg i = " << i << "; deg[i].x = " << ((deg[i].x<<32)>>32)
		//	<< " deg[i].y = " << deg[i].y << " deg[i].w = " << deg[i].w << endl;
		pivotMap[((deg[i].x<<32)>>32)] = i;
	}

	outLabel.start();
	for (int j = 0; !outLabel.isEnd; j++){
		outLabel.nextEdge(labelout[j]);
		//cout << "label j = " << j << "; labelout[j].x = " << labelout[j].x
		//	 << " labelout[j].w = " << labelout[j].w << endl;
	}

	/**rebuild indices in our format**/
	HI_orig.resize(num_node);  //will be used later
	edgeS *label;

	for(int i = 0; i < num_node; i++) {
		vector<Hop2Label> newLabelList;
		newLabelList.push_back(Hop2Label(i, 0) );
		int num = 0;

		label = labelout + deg[i].y; //deg[i].y indicates offset

		while(label[num].x!=-1) {
			Hop2Label newLabel = Hop2Label(pivotMap[label[num].x], label[num].w);
			newLabelList.push_back(newLabel);
			num++;
		}
		stable_sort(newLabelList.begin(), newLabelList.end(), LbAscend);  //ascending order as distance
		HI_orig[i] = newLabelList; // add all (ancid, dist) entries for a node
	}

	//-- free pointers
	free(deg);
	free(labelout);
	free(outName);
	free(degName);
	free(edgeName);
}

void PrintHop2Labels(string of_str){   //for testing
	const char * of_nm = of_str.c_str();
	ofstream hopLbOutfile;
	hopLbOutfile.open(of_nm);

	int sz = HI_orig.size();
	for(int i = 0; i < sz; i++){
		hopLbOutfile << i << ": ";

		vector<Hop2Label> lb_list = HI_orig[i];

		for(int j = 0; j < lb_list.size(); j++){
			hopLbOutfile << "(" << lb_list[j].ancid  << " " << lb_list[j].dist << ") ";
		}
		hopLbOutfile << endl;
	}

	hopLbOutfile.close();
}

int LoadLocationKeywords(string poiKwdsFileStr){
	const char * poiKwdsFile = poiKwdsFileStr.c_str();
	ifstream poiKwds_read(poiKwdsFile);
	if (!poiKwds_read){
		cout << "False: file " << poiKwdsFileStr << "can not be found";
		return false;
	}

	POIFtMx.resize(num_node);

	string line;
	char sepCol[] = ":";
	char *p;
	while( getline(poiKwds_read, line) ){
		int sz = line.size();
		const char * str_char = line.c_str();
		int split = strcspn(str_char, sepCol);
		int poi = atoi(line.substr(0,split).c_str() );
		string rest = line.substr(split + 1,sz - 1);
		vector<decmtype> ftrVec;

		p = strtok(&rest[0], " ");
		while(p != NULL){
			decmtype val;
			val = atof(p);
			ftrVec.push_back(val);
			p=strtok(NULL, " ");
		}

		POIFtMx[poi] = ftrVec;
	}

	fSpaceSz = POIFtMx[0].size();

	poiKwds_read.close();
	return 0;

}

int LoadInvertedKeywords(string ivtKwdsFileStr){
	const char * ivtKwdsFile = ivtKwdsFileStr.c_str();
	ifstream ivtKwds_read(ivtKwdsFile);
	if (!ivtKwds_read){
		cout << "False: file " << ivtKwdsFileStr << "can not be found";
		return false;
	}

	string line;
	char sepCol[] = ":";
	char *p;
	while( getline(ivtKwds_read, line) ){
		int sz = line.size();
		const char * str_char = line.c_str();
		int split = strcspn(str_char, sepCol);
		int kwd = atoi(line.substr(0,split).c_str() );
		string rest = line.substr(split + 1,sz - 1);
		vector<FtrNodePair> fnpVec;

		p = strtok(&rest[0], " ");

		int i = 0;
		while(p != NULL){
			int id;  //do not need to be initialized
			decmtype val;

			if(i % 2 == 0){
				id = atoi(p);
			}else{
				val = atof(p);
				FtrNodePair fnp = FtrNodePair(id, val);
				fnpVec.push_back(fnp);
			}

			p=strtok(NULL, " ");
			++i;
		}

		FI[kwd] = fnpVec;
	}

	ivtKwds_read.close();
	return 0;
}

int LoadStayTime(string stayFileStr){
	stayTimeVec.resize(num_node);
	const char * stayFile = stayFileStr.c_str();
	ifstream stay_read(stayFile);
	if (!stay_read){
		cout << "False: file " << stayFileStr << "can not be found";
		return false;
	}

	string line;
	char sepCol[] = ":";
	while( getline(stay_read, line) ){
		int sz = line.size();
		const char * str_char = line.c_str();
		int split = strcspn(str_char, sepCol);

		int node_id = atoi(line.substr(0,split).c_str() );
		wtype stay_time = atoi(line.substr(split + 1,sz - 1).c_str() );  //--- change to atof() if weight type is not int
		//wtype stay_time = atof(line.substr(split + 1,sz - 1).c_str() ); 

		stayTimeVec[node_id] = stay_time;
	}

	stay_read.close();
	return 0;
}


void LoadInputs(string data_header){
	/***Load Hop-2 Labels***/
	string edgeFileNm = root + data_header + "_undir_edges.txt";

	double tstart, tstop;
	tstart = (double)clock()/CLOCKS_PER_SEC;
	LoadHop2Labels(edgeFileNm);
	tstop = (double)clock()/CLOCKS_PER_SEC;
	cout << "It took " << tstop-tstart << " second(s) to load original labels." << endl;

//  Output the 2-hop lables for testing
//  string labelOfStr = root + data_header + "_lb2text.txt";
//	PrintHop2Labels(labelOfStr);

	/***Load POI-Keywords matrix***/
	string poiKwdsFileStr = root + data_header + "_mat_loc_kwds.txt";
	LoadLocationKeywords(poiKwdsFileStr);

	/***Load Inverted Indices***/
	string ivtdFileStr = root + data_header + "_inverted_kwds.txt";
	LoadInvertedKeywords(ivtdFileStr);


	/***Load Stay time***/
	string stayFileStr = root + data_header + "_stayTime.txt";
	LoadStayTime(stayFileStr);

	/***Init Transit time and cmptPair***/
	travelTime = new wtype*[num_node];
	cmptdPair = new wtype*[num_node];
	for(int i = 0; i < num_node; i++)
	{
		travelTime[i] = new wtype[num_node];
		cmptdPair[i] = new wtype[num_node];
		for(int j = 0; j < num_node; j++){
			if(i != j){
				travelTime[i][j] = INF;
			}else{
				travelTime[i][j] = 0;
			}
			cmptdPair[i][j] = 0;

		}
	}
}

void initPowerLaw(decmtype alpha){ //compute the diminishing factors for each rank
	PLCoeff = vector<double>(30); //don't need so much in real use
	for(int i = 0; i < 30; i++){
		double coeff = pow(i+1, -alpha); //rank k, index k-1
		PLCoeff[i] = coeff;   
	}
}

wtype CompTransitCost(int s_Id, int t_Id, int s_pos, int t_pos){ //least traveling cost computation
	wtype res = INF;

	if(travelTime[s_Id][t_Id] != INF){
		res = travelTime[s_Id][t_Id]; // use early computed dist
	}else{
		unordered_map<int, wtype> match_map;

		int long_id, short_id, long_pos, short_pos;
		if(s_pos > t_pos){
			long_id = s_Id;
			short_id = t_Id;
			long_pos = s_pos;
			short_pos = t_pos;
		}else{
			long_id = t_Id;
			short_id = s_Id;
			long_pos = t_pos;
			short_pos = s_pos;
		}

		if(short_pos == 1){   // only one entry to itself
			Hop2Label shortLabel = HI_orig[short_id][0];
			for(int i = 0; i < long_pos; i++){
				Hop2Label longLabel = HI_orig[long_id][i];
				if(shortLabel.ancid == longLabel.ancid){
					res = shortLabel.dist + longLabel.dist;
					break;
				}
			}
		}else{
			//establish hash, longer to build
			for(int i = 0; i < long_pos; i++){
				Hop2Label longLabel = HI_orig[long_id][i];
				match_map[longLabel.ancid] = longLabel.dist;
			}
			// hash match, shorter to match
			for(int j = 0; j < short_pos; j++){
				Hop2Label shortLabel = HI_orig[short_id][j];
				if( match_map.find(shortLabel.ancid) != match_map.end()){
					wtype dist = match_map[shortLabel.ancid] + shortLabel.dist;
					if(dist < res){
						res = dist;
					}
				}
			}
		}

		if(res != INF){
			travelTime[s_Id][t_Id] = res;
			travelTime[t_Id][s_Id] = res;  //for undirected graph
		}else{
			cmptdPair[s_Id][t_Id] = INF;
		}
	}

	return res;
}

int TruncateHop2Label(vector<Hop2Label>& inputLabels, wtype bdgt){  //truncate 2-hop labels by budget to speed up traveling cost computation
	/* Only select the pivot labels with distance less than budget, returns the position */
	int label_sz = inputLabels.size();
	int pos = 0;
	for(; (pos < label_sz) && (inputLabels[pos].dist <= bdgt); pos++){}

		return pos;
}

int RetrieveCandidates(UserQuery uq){
	/***Check validity of query and retrieve nodes by keywords and the threshold***/
	g_startNode = uq.startPOI;
	g_endNode = uq.endPOI;
	g_budget = uq.budget;
	g_val_thres = uq.thres;
	g_kwds = uq.keywds;
	g_kwds_wts = uq.keywds_weights;
	g_alpha = uq.alpha;

	decmtype weightsum = 0;
	for(int i = 0; i < g_kwds_wts.size(); i++){
		weightsum += g_kwds_wts[i];
	}

	for(int i = 0; i < g_kwds.size(); i++){
		if(g_kwds[i] >= fSpaceSz){
			cout << "Some keywords are out of range" << endl;
			return -1;
		}
	}

	cout << "Weight sum = " << weightsum << endl;
	if(weightsum != 1.0 || g_kwds.size() != g_kwds_wts.size() ){
		cout << "The keywords/weight format is invalid" << endl;
		return -1;
	}

	wtype s2tCost = CompTransitCost(g_startNode, g_endNode, HI_orig[g_startNode].size(), HI_orig[g_endNode].size() );

	if(s2tCost >= g_budget){
		cout << "The destination is not reachable from source or the cost is larger than budget!" << endl;
		return -1;
	}

	
	initPowerLaw(g_alpha); //pre-compute the diminishing factors for each rank

	unordered_set<int> rela_nodes; //query related nodes
	int kwds_sz = g_kwds.size();

	for(int k = 0; k < kwds_sz; k++){
		vector<FtrNodePair> FI_Q_kwd= FI[g_kwds[k] ];
		int list_sz = FI_Q_kwd.size();
		for(int i = 0; (i < list_sz) && (FI_Q_kwd[i].ftrVal >= g_val_thres); i++){
			rela_nodes.insert( FI_Q_kwd[i].nodeId );
		}
	}

	rela_nodes.erase(g_startNode);  //remove the POI for source/destination
	rela_nodes.erase(g_endNode);

	/***Build HI_Q_Pos and retrieve the final candidates***/
	g_s_pos = TruncateHop2Label(HI_orig[g_startNode], g_budget);
	g_t_pos = TruncateHop2Label(HI_orig[g_endNode], g_budget);

	int numFinalRet = 0;
	tourCostsVec.resize(num_node);
	
	for(auto iter = rela_nodes.begin(); iter != rela_nodes.end(); iter++){
		int node_id = *iter;
		// Only select the pivot nodes less than budget
		int node_pos = TruncateHop2Label(HI_orig[node_id], g_budget);
		wtype cost_s_p = CompTransitCost(g_startNode, node_id, g_s_pos, node_pos);
		if(cost_s_p < g_budget){
			wtype cost_p_e = CompTransitCost(node_id, g_endNode, node_pos, g_t_pos);
			if(cost_p_e < g_budget){
				wtype tourCost = cost_s_p + stayTimeVec[node_id] + cost_p_e;
				tourCostsVec[node_id] = tourCost;
				if(tourCost <= g_budget){  //test if s -- p -- t <= budget
					HI_Q_Pos[node_id] = node_pos;
					numFinalRet++;
				}
			}
		}
	}
	
	cout << "# of final retrieved nodes: " << numFinalRet << endl;  //candidates size

	vector<int> candidates;
	for(auto it = HI_Q_Pos.begin(); it != HI_Q_Pos.end(); it++ ){
		candidates.push_back(it->first);
	}
	candidates.push_back(g_startNode);
	candidates.push_back(g_endNode);

	
	/***compute the nodeCost of each POI: stay cost + the cost of (min_in_edge + min_out_edge)/2 ***/
	int candi_sz = candidates.size();
	nodeCost.resize(num_node);

	for(int i = 0; i < candi_sz; i++){
		int src  = candidates[i];
		int src_pos = TruncateHop2Label(HI_orig[src], g_budget);
		int src_min, src_sec_min; //Since our input is an undirected graph, we find the min and second_min as the min_in and min_out
		src_min = INF;
		src_sec_min = INF;

		for(int j = 0; j < candi_sz; j++){
			if( j != i){
				int dest = candidates[j];
				int dest_pos = TruncateHop2Label(HI_orig[dest], g_budget);
				wtype s_t_cost = CompTransitCost(src, dest, src_pos, dest_pos);

				if(s_t_cost < src_sec_min){
					if(s_t_cost < src_min){
						src_sec_min = src_min;
						src_min = s_t_cost;
					}else{
						src_sec_min = s_t_cost;
					}
				}
			}
		}

		wtype smallest2cost = (src_min + src_sec_min)/2 + stayTimeVec[src];
		nodeCost[src] = smallest2cost;
	}

	return numFinalRet;
}


