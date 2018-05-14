/**
Author: Hongwei Liang
Description: main file and output.
**/

#include "Index.h"
#include "RouteSearch.h"


/**************Main function*************/
int main(int argc, char* argv[]){

    string if_nameHeader2Str(argv[1]); //dataset header, e.g., sg
	string method(argv[2]); //search method: P (PACER), S (PACER-SC), or G (Greedy)
    int s_id = atoi(argv[3]); //source POI
    int e_id = atoi(argv[4]); //destination POI
    decmtype pram = atof(argv[5]); // alpha for power law, e.g., 0.5
    int k_val = atoi(argv[6]); //top-k value, e.g., 10
    wtype budget = atoi(argv[7]); // the unit is minute, e.g., 360
    decmtype val_thres = atof(argv[8]);  //e.g., 2.5
    string ftrPrf(argv[9]); //preferred features and corresponding weights, e.g., 0_0.4_2_0.3_10_0.3 
    string ftrPrf_copy = ftrPrf;
    char *p;
    p = strtok(&ftrPrf[0], "_");
    vector<int> kwds; //e.g., {0, 2, 10}
    vector<decmtype> kwd_vals; //e.g., {0.4, 0.3, 0.3}, has to be summing to 1

    int i = 0;
    while(p != NULL)
    {
        int kwd;  //do not need to assign a value
        decmtype val;

        if(i % 2 == 0){
            kwd = atoi(p);
            kwds.push_back(kwd);
        }else{
            val = atof(p);
            kwd_vals.push_back(val);
        }

        p=strtok(NULL, "_");
        ++i;
    }

    //create output file
    string ss;
    char ch[50];


    string of_str = if_nameHeader2Str + "-" + method + "-";
    string lastLine = if_nameHeader2Str + "\t" + method + "\t";  //The last line output is for the ease of batch experiments. Use "tail -1" to retrieve the result for the query
    sprintf(ch, "%d", s_id);
    ss = ch;
    of_str = of_str + ss + "-";
    sprintf(ch, "%d", e_id);
    ss = ch;
    of_str = of_str + ss + "-";
    sprintf(ch, "%.1f", pram);
    ss = ch;
    of_str = of_str + ss + "-";
    lastLine = lastLine + ss + "\t";
    sprintf(ch, "%d", k_val);
    ss = ch;
    of_str = of_str + ss + "-";
    sprintf(ch, "%d", budget);
    ss = ch;
    of_str = of_str + ss + "-";
    lastLine = lastLine + ss + "\t";
    sprintf(ch, "%.2f", val_thres);
    ss = ch;
    of_str = of_str + ss + "-";
    lastLine = lastLine + ss + "\t";
    of_str = of_str + ftrPrf_copy +".txt";
    lastLine = lastLine + ftrPrf_copy + "\t";

    const char * of_nm = of_str.c_str();
    cout << "file: " << of_str << endl;
    ofstream outfile;
    outfile.open(of_nm);

    UserQuery uq = UserQuery(s_id, e_id, budget, val_thres, kwds, kwd_vals, pram);

    cout << "Data: " << if_nameHeader2Str << endl;
    outfile << "Data: " << if_nameHeader2Str << endl;
    LoadInputs(if_nameHeader2Str);

    cout << "Load done." << endl;
    cout << "s = " << s_id << ", t = " << e_id << ", alpha = " << pram << ", k = " << k_val
         << ", b = " << budget << ", thres = " << val_thres << endl;
    outfile << "s = " << s_id << ", t = " << e_id << ", alpha = " << pram << ", k = " << k_val
         << ", b = " << budget << ", thres = " << val_thres << endl;
    cout << "features: (";
    outfile << "features: (";
    for(int j = 0; j < kwds.size(); j++){
        cout << kwds[j] << " = " << kwd_vals[j] << "; ";
        outfile << kwds[j] << " = " << kwd_vals[j] << "; ";
    }
    cout << ")" << endl;
    outfile << ")" << endl;

	double time_be, time_retri, time_fi;

    time_be = (double)clock()/CLOCKS_PER_SEC;

	int numCandidates = RetrieveCandidates(uq);
    outfile << "# of final retrieved nodes: " << numCandidates << endl;

	time_retri = (double)clock()/CLOCKS_PER_SEC;
    cout << "Retrieve candidates done. Used time: " << time_retri - time_be  << " secs." << endl;
	
	string method_full = "";
	if(method == "P"){
		method_full = "PACER";
	}else if(method == "S"){
		method_full = "PACER-SC";
	}else if(method == "G"){
		method_full = "Greedy";
	}else{
		cout << "Incorrect method." << endl;
		return -1;
	}
	
	cout << "Search method: " << method_full << endl;
	outfile << "Search method: " << method_full << endl;
	
    RecTrips(method, k_val);

    time_fi = (double)clock()/CLOCKS_PER_SEC;
    cout << "Compute trips done. Used time: " << time_fi - time_retri  << " secs." << endl;
    outfile << "Time for computing trips: " << time_fi - time_retri  << " secs." << endl;
    sprintf(ch, "%.3f", time_fi - time_retri);
    ss = ch;
    lastLine = lastLine + ch + "\t";
    cout << "Total running time: " << time_fi - time_be  << " secs." << endl;
    cout << "Total search space: " << search_space << endl;
    outfile << "Total search space: " << search_space << endl;
    sprintf(ch, "%lld", search_space);
    ss = ch;
    lastLine = lastLine + ss + "\t";

	
    /*****output TOPK*****/
    int trip_rank = topKqueue.size();
    cout << endl << "**********" << endl;
    outfile << endl << "**********" << endl;
    cout  << "Top " << trip_rank << " results--" << endl;
    outfile  << "Top " << trip_rank << " results--" << endl;

    while(!topKqueue.empty()){
        topTrips tk = topKqueue.top();

        cout << trip_rank << ": ";
        outfile << trip_rank << ": ";
        cout << g_startNode << "-";
        outfile << g_startNode << "-";
        sprintf(ch, "%d", g_startNode);
        ss = ch;
        string tripRoute = ss + "-";

        for(int i = 0; i < tk.path.size(); i++){
            int POI_id = tk.path[i];

            string POI_kwds = "(";
            vector<decmtype> pFVec = POIFtMx[POI_id];
            for(int j  = 0; j < g_kwds.size(); j++){
                int k = g_kwds[j]; //get the real keyword k

                if(pFVec[k] >= g_val_thres){  //external global threshold
                    sprintf(ch, "%d", k);
                    ss = ch;
                    POI_kwds += ss + ":";
                    sprintf(ch, "%.2f", pFVec[k]);
                    ss = ch;
                    POI_kwds += ss + ", ";
                }
            }
            POI_kwds += ")";

            cout << POI_id << POI_kwds << "-";
            outfile << POI_id << POI_kwds << "-";
            sprintf(ch, "%d", tk.path[i]);
            ss = ch;
            tripRoute = tripRoute + ss + POI_kwds + "-";
        }


        cout << g_endNode << "; aggRating = " << tk.aggRating << "; cost = " << tk.cost << endl;
        outfile << g_endNode << "; aggRating = " << tk.aggRating << "; cost = " << tk.cost << endl;
        sprintf(ch, "%d", g_endNode);
        ss = ch;
        tripRoute = tripRoute + ss;


        if(topKqueue.size() == 1){
            sprintf(ch, "%.4f", tk.aggRating);
            ss = ch;
            lastLine = lastLine + ss + "\t";
            lastLine = lastLine + tripRoute + "\t";
            sprintf(ch, "%d", tk.path.size() + 2);
            ss = ch;
            lastLine = lastLine + ss + "\t";
            sprintf(ch, "%d", tk.cost);
            ss = ch;
            lastLine = lastLine + ss + "\t";
        }

        topKqueue.pop();
        --trip_rank;

    }

    sprintf(ch, "%d", numCandidates);
    ss = ch;
    lastLine = lastLine + ss;
    outfile << endl << lastLine << endl;

    cout << "Output done." << endl;
    outfile.close();

 	cout << "All done!" << endl;
    return 0;
}

