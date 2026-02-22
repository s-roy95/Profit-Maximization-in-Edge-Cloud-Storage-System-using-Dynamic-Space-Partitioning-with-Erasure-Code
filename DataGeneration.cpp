#include <iostream>
#include<cstdlib>
#include<vector>
#include<algorithm>
#include<fstream>
#include<iomanip>
#include<queue>

using namespace std;




#define HIGH_P 70 	//percentage of high priority tasks
#define MID_P 20	//percentage of mid priority tasks
#define LOW_P 10 	//percentage of low priority tasks

#define HIGH_Deadline 70
#define MID_Deadline 100
#define LOW_Deadline 130

#define HIGH_Profit 20
#define MID_Profit 10
#define LOW_Profit 5

#define CDATA 0 //percentage of common data between two consecutive batches

ofstream ReqTrace("RequestTrace_0R.txt"); //output file name


#define NUM_AP 16
#define NUM_CHUNK 10 	





#define NUM_ES 8
#define ES_CAP 10
#define MAX_DIST 10
#define NUM_Req 10000
#define MAXSIMTIME 100000

#define LOCAL_ACCESS_TIME 2
#define SHARER_ACCESS_TIME 5
#define Miss2Cloud  30

#define LOCAL_SHARE 0.8
#define FWD 1

#define DEADLINE 0
#define PROFIT   1
#define SORT 0

#define DEBUG 1
#define MIN_DEADLINE 10

int dm[NUM_AP][NUM_ES];
int sim_time;

//ofstream ReqTraceSched("RequestTraceSched.txt");


class Req {
	public: 
	int arrival_time;
	int cid;
	int time_elapsed;
	int request_id;
	int deadline;
	int finish_time;
	int AP;
	int profit;
	Req(int ar, int _cid, int rid, int dl, int ft, int _AP, int _profit){
		arrival_time=ar;cid=_cid;request_id=rid;
		deadline=ar+dl;
		finish_time=ft,AP=_AP; profit=_profit;
		time_elapsed=0;
	}
	void set_time_elapsed(int amount){
		time_elapsed+=amount;
	}

};


vector< Req * > R; 

bool mysort(Req *A, Req *B){
	if (SORT==DEADLINE) return A->deadline>B->deadline; 
	if (SORT==PROFIT) return A->profit>B->profit; 
}	

class AP_Class{
	public: 
		int AP_ID;
		int dist_min_dist_es;
		int num_min_dist_es;
		vector< Req *> Q;
		void Schedule(); 
		void setAP_ID(int APid){AP_ID=APid;}
		void SortReqs(){
			sort(Q.begin(), Q.end(),mysort);
		}
};


class ES_Class{

     public:

     int hit, miss;
     int DC[ES_CAP];
     int LRU[ES_CAP];
     int Freq[ES_CAP];     
     int ES_ID;
     void Clear(){
	     for(int i=0;i<ES_CAP;i++) {DC[i]=-1;LRU[i]=-1;Freq[i]=-1; hit=0;miss=0;}
     }
     void Schedule(){}
     void Place(int _cid, int local);
     int Access(Req *R, int LocalAccess);
     bool Check(int CID);
};

ES_Class  ES[NUM_ES];
AP_Class AP[NUM_AP];

void ES_Class::Place(int _cid, int local){
	     int start, end;
	 	 if (DEBUG) cout<<"Before Placed "<<_cid<<" :"; 
		 if (DEBUG) for(int i=0;i<ES_CAP;i++) cout<<" "<<DC[i]; 

	     if (local) { //Replace from local area
	       start=0; end=(int)(ES_CAP*LOCAL_SHARE);
	     } else{
		     start=(int)(ES_CAP*LOCAL_SHARE);
		     end=ES_CAP;
	     }
	       int iLRU=start, vLRU=LRU[start];
	       if(DEBUG) cout<<"\nplace local="<<local<<" start= "<<start<<" end= " <<end<<endl;
	       //USe LRU 
	       for(int i=start;i<end;i++){
		    if (LRU[i]==-1) { iLRU=i; break; }  
		    if(vLRU<LRU[i]) {iLRU=i;vLRU=LRU[i];}    
	       }
	       DC[iLRU]=_cid;
	       LRU[iLRU]=sim_time;
	       Freq[iLRU]=1;
	 	 if (DEBUG) cout<<"\nAfter Placed:"; 
		 if (DEBUG) for(int i=0;i<ES_CAP;i++) cout<<" "<<DC[i]; 
		 if (DEBUG) cout<<"\n";
	       // Write Code for Zips Law of Replacement	 
     }


bool ES_Class::Check(int CID){
      	for(int i=0;i<ES_CAP;i++){
	   if(DC[i]==CID) return 1;
   }
   return 0;
}
int ES_Class::Access(Req *R, int LocalAccess){
         //Search Locally 
	 for(int i=0;i<ES_CAP;i++){
		 if(DC[i]==R->cid) {
	    	     LRU[i]=sim_time;
		     Freq[i]++;
		     if (LocalAccess) R->finish_time=R->time_elapsed+R->arrival_time+LOCAL_ACCESS_TIME;
		     else R->finish_time=R->time_elapsed+R->arrival_time+SHARER_ACCESS_TIME;
		     hit++;
		     return 1;
		 } 
	 }
	 // Miss happened: Send to Central or Forward2Neigbours or Rejects
	 miss++;
	 if (DEBUG) cout<<"Miss Occurred at R"<<R->request_id<<" ES_ID= "<<ES_ID<<" \n";
	 //if (R->deadline) 
	 if(!FWD) { //Always do locally replacement
	 	 if (DEBUG) cout<<"Before Placed "<<R->cid<<" :"; 
		 if (DEBUG) for(int i=0;i<ES_CAP;i++) cout<<" "<<DC[i]; 
	     	Place(R->cid,1); //1 for local
	 	 if (DEBUG) cout<<"\nAfter Placed:"; 
		 if (DEBUG) for(int i=0;i<ES_CAP;i++) cout<<" "<<DC[i]; 
		 if (DEBUG) cout<<"\n";
		R->finish_time=R->time_elapsed+R->arrival_time+Miss2Cloud;
	     }

	 if(FWD && LocalAccess) {
		//miss can be redirected to two nearby ESs 
		int left, right;
	        bool X, Y;	
		left=ES_ID-1;if (left==-1) left =NUM_ES-1;
		right=ES_ID+1;if (right==NUM_ES) right =0;
		X=ES[left].Check(R->cid); Y=ES[right].Check(R->cid); //Check for avalibalbity in Left ES or right ES
							   //Assume You have meta-data of both the neibour Servers
		if (X&&Y) {
			   if((rand()%100)>50) ES[left].Access(R,1); else ES[right].Access(R,1); return 0;
			  }
		if (X) { ES[left].Access(R,1);return 0;}	
		if (Y) { ES[right].Access(R,1);return 0;}

	        if ((rand()%100)>(LOCAL_SHARE*100)){ //Forward miss to Sharer with probablity 1-LOCAL_SHARE
			if((rand()%100)>50) ES[left].Access(R,0); //non-local access
			else ES[right].Access(R,0); // non-local access
		}
		else {
		     Place(R->cid,1); //Place locally	
		     R->finish_time=R->time_elapsed+R->arrival_time+Miss2Cloud;
	     }
	 }
	 if(FWD && !LocalAccess) {
		Place(R->cid,0); //0 Place in one SharedPart of the ES
		R->finish_time=R->time_elapsed+SHARER_ACCESS_TIME+R->arrival_time+Miss2Cloud;
	 }
	 return 0;
} 


void AP_Class::Schedule() {
	if (SORT) SortReqs();
	//if (Greedy) Sort();
	for(int i=0;i<Q.size();i++) {
		Q[i]->set_time_elapsed(dist_min_dist_es);
		ES[num_min_dist_es].Access(Q[i],1);//1 for local access
	}
	Q.clear();       	
}	

void PrintTrace(ofstream &RT){
	char St;
	RT<<"Rid arr_time   AP   chunk_id deadline profit\n";
	for(int i=0;i<NUM_Req;i++){
		RT<<i<<" "<<setw(5)<<R[i]->arrival_time<<" "<< setw(9)<<R[i]->AP
	     	<<" "<<setw(4) <<R[i]->cid<<" "<<setw(10)<<R[i]->deadline <<setw(10)<<R[i]->profit
		<<endl;	
	}

}
void Generate_Request(){
	int ar=0,rid=0,newcid;
	vector <int> History;
	for(int i=0;i<NUM_Req;i++){
		if (i<5) {
			newcid=rand()%NUM_CHUNK;
			History.push_back(newcid);
		}
		else {
		if((rand()%100)<100-CDATA) { 
			newcid=rand()%NUM_CHUNK;
			History.erase(History.begin());
			History.push_back(newcid);
		      } else newcid=History[rand()%5];
		}
		int randint = rand() % 100; 
		if( randint >= 0 && randint <= HIGH_P)
			R.push_back(new Req(ar, newcid, i, HIGH_Deadline, -1,rand()%NUM_AP, HIGH_Profit));
		else if (randint > HIGH_P && randint <= MID_P+HIGH_P)
			R.push_back(new Req(ar, newcid, i, MID_Deadline, -1,rand()%NUM_AP, LOW_Profit));

		else
			R.push_back(new Req(ar, newcid, i, LOW_Deadline, -1,rand()%NUM_AP, MID_Profit));


		ar=ar+rand()%10;
		rid++;
	}
	PrintTrace(ReqTrace);

}

void initialize_DistMatrix(){
	int min_dist, min_es,dist;
	if (DEBUG) cout<<"\n";
	for(int i=0;i<NUM_AP;i++) {
		AP[i].setAP_ID(i);
		min_dist=100000;min_es=-1;
                for(int j=0;j<NUM_ES;j++) {
                        dist = rand()%MAX_DIST + 1;
                        dm[i][j] =  dist;
			if(min_dist>dist) { min_dist=dist; min_es=j;}
                }
		AP[i].dist_min_dist_es=min_dist;
		AP[i].num_min_dist_es=min_es;
		if (DEBUG) cout<<"\nAP["<<i<<"].num_min_dist_es="<<AP[i].num_min_dist_es;
        }
	if (DEBUG) cout<<"\n";
}

void ResetSystem(){
	//Clear ES queue and Storages
	for(int i=0;i<NUM_ES;i++) { ES[i].Clear(); ES[i].ES_ID=i;}
	//Clear finish time of All requests
	for(int i=0;i<NUM_Req;i++) R[i]->finish_time=-1;
}

void PutRequest2APs(int sim_time){
	for(int i=0;i<NUM_Req;i++){
		if(R[i]->arrival_time==sim_time) 
			AP[R[i]->AP].Q.push_back(R[i]);
	}
}

void Simulate(){
      sim_time=0;
      int i, j;
      while(sim_time<MAXSIMTIME){
	PutRequest2APs(sim_time);	
	for(i=0;i<NUM_AP;i++) AP[i].Schedule(); 
	for(j=0;j<NUM_ES;j++) ES[j].Schedule(); 
	sim_time++;
      }
}

void PrintStat(){
	int TGR=0; int PGR=0; int MaxProfit=0;
	for(int i=0;i<NUM_Req;i++){
	  if(R[i]->finish_time <= R[i]->deadline) {
		  TGR++; PGR +=R[i]->profit;
	  }
	  MaxProfit +=R[i]->profit;
	}
	cout<<"[[ TGR = "<<TGR<< " ]] [[ RSize ="<<R.size() <<" ]] [[ PGR= "<<PGR<<", MaxProfit="<<MaxProfit<<" ]]\n";
	if (DEBUG) for(int j=0;j<NUM_ES;j++) 
		       cout<<"ES["<<j<<"] ==> (hit "<<ES[j].hit<<" miss "<<ES[j].miss<<")\n"; 
	

}

int main() {
	
	srand((unsigned) time(NULL));
	cout<<"My Work Started...\n";
	//initialize_DistMatrix();
	Generate_Request();


	//ResetSystem();
	//Simulate();
	//PrintStat();
	//PrintTrace(ReqTraceSched);

	cout<<"\nMy Work end here...\n";
	
	return 0;
}
