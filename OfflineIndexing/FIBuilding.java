import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Collections;
import java.util.HashMap;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.TreeMap;



public class FIBuilding {
	
	/**Func: Build inverted keyword-poi index
	 * @throws IOException */
	@SuppressWarnings("unchecked")
	void InvertedIndexing(String if_name, String of_name) throws IOException{
		
		BufferedReader br = new BufferedReader(new FileReader(new File(if_name)));		
		BufferedWriter wr = new BufferedWriter(new FileWriter(new File(of_name)));
		
		TreeMap<Integer, ArrayList<FtrNodePair> > ivtd_Map = new TreeMap<Integer, ArrayList<FtrNodePair> >();
		
		String line;
		
		while( (line = br.readLine() ) != null){
			String [] edge = line.split(":");
			int nodeId = Integer.parseInt( edge[0] );
			String [] kwdVals = edge[1].split(" ");
			
			int kwdSz = kwdVals.length; 

			for(int i = 0; i < kwdSz; i++){
				double val = Double.parseDouble( kwdVals[i] );
				
				if(val > 0){
					FtrNodePair fnp = new FtrNodePair(val, nodeId);	
					if( ivtd_Map.containsKey(i) ){
						ivtd_Map.get(i).add(fnp);					
					}else{			
						ArrayList<FtrNodePair> newList = new ArrayList<FtrNodePair> ();
						newList.add(fnp);
						ivtd_Map.put(i, newList);
					}
				}
			}		
		}
			
		br.close();
		
		for (Integer key: ivtd_Map.keySet()) {
			   ArrayList<FtrNodePair> ivtd_list = ivtd_Map.get(key);
			   Collections.sort(ivtd_list);
			   			   
			   wr.write(key + ":");

			   Iterator<FtrNodePair> iter = ivtd_list.iterator();
			   while(iter.hasNext()) {
				   FtrNodePair elem = iter.next();
				   wr.write(elem.getNodeId() + " " + elem.getFtrVal() + " ");
			   }
			   wr.write("\n");
		}
		
		wr.close();
	}


	
	/*************************
	 *  main for testing
	 * @param args
	 * @throws IOException
	 *************************/
	public static void main(String[] args) throws IOException{	
		
		FIBuilding fb = new FIBuilding();
		String dir = "";
		
		String if_name = dir + "as_mat_loc_kwds.txt";
		String of_name = dir + "as_inverted_kwds.txt";
		
		fb.InvertedIndexing(if_name, of_name);
 
		System.out.println("Done!");
	}	
}



/***Class for inverted list***************/
class FtrNodePair implements Comparable<Object>{
	double ftrVal;
	int nodeId;
	
	public FtrNodePair(double ftrVal, int nodeId) {
		super();
		this.ftrVal = ftrVal;
		this.nodeId = nodeId;
	}

	public double getFtrVal() {
		return ftrVal;
	}

	public void setFtrVal(double ftrVal) {
		this.ftrVal = ftrVal;
	}

	public int getNodeId() {
		return nodeId;
	}

	public void setNodeId(int nodeId) {
		this.nodeId = nodeId;
	}
	

	@Override
	public int compareTo(Object compareFtr) {
		// TODO Auto-generated method stub
		
		double compareVal = ((FtrNodePair)compareFtr).getFtrVal(); 
		
		double diff = compareVal - this.ftrVal;   //Descending order
		
		if(diff > 0){
			return 1;
		}else if (diff < 0){
			return -1;
		}else{
			return 0;
		}
	}	
}