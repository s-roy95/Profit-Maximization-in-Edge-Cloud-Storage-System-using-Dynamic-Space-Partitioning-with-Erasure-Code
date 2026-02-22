#include<bits/stdc++.h>
//#include<windows.h>
#include<unistd.h>
#include<chrono>
using namespace std;

#define no_eS 30
#define no_aP  100
#define aP_to_eS_lb 3
#define aP_to_eS_ub 5
#define c_eS_lb_dis 25 
#define c_eS_ub_dis 30
#define eS_eS_lb_dis 10 
#define eS_eS_ub_dis 15
#define aP_eS_lb_dis 3 
#define aP_eS_ub_dis 5

#define HIGH_P 10 	//percentage of high priority tasks
#define MID_P 20	//percentage of mid priority tasks
#define LOW_P 70 	//percentage of low priority tasks

#define HIGH_Deadline 30
#define MID_Deadline 40
#define LOW_Deadline 50

#define HIGH_Profit 20
#define MID_Profit 10
#define LOW_Profit 5

#define MIN_BATCH 5
#define MAX_BATCH 20

#define CDATA 30//percentage of common data between two consecutive batches

#define NETWORK_FAILURE_RATE 0.17 // 10% network failures

int NUM_req =10000;
#define no_movies 5000
#define max_datachunks 24
#define req_datachunks 4
#define eS_blockcount 20

double pri_pub_ratio[no_eS];


double total_profit=0;

//<distance,eS> to store distance between each aP and eS
vector<pair<int,int>> aP_eS_finalDis[no_aP];

int c_eS_dis[no_eS];
int eS_eS_dis[no_eS][no_eS];
int aP_eS_dis[no_aP][no_eS];
int eS_aP_dis[no_eS][no_aP];
double aP_eS_prob[no_aP][no_eS];
double eS_aP_prob[no_eS][no_aP];
int eS_blocks[no_eS][eS_blockcount];
int access_time[no_eS][eS_blockcount];

int aP_reqCnt[no_aP];
int eS_aP_blockStart[no_eS][no_aP];
int eS_aP_blockEnd[no_eS][no_aP];

int timer=0;
int hit_count_SE=0;
int profit_SE=0;
int hit_count_NSE=0;
int profit_NSE=0;

class Req {
	public: 
    int id;
	int arrival_time;
	int aP;
    int cid;
	int deadline;
	int profit;
	
    Req(int _id,int _arrival_time,int _aP,int _cid,int _deadline,int _profit)
    {
        id=_id;arrival_time=_arrival_time;aP=_aP;cid=_cid;deadline=_deadline;profit=_profit;
    }
};



vector<Req*> R;




void generate_Infrastructure()
{    
    cout << "\n=== Generating Infrastructure with 10% Network Failures ===\n";
    
    //generating cloud to edgeServer distance WITH 10% FAILURES
    FILE* fptr;
    fptr=fopen("c_eS_dis.txt","w");
    if(fptr==NULL)
    {
        cout<<"File Creation failed\n";
        exit(0);
    }

    // First generate all connections
    vector<int> temp_c_eS(no_eS);
    for(int i=0;i<no_eS;i++)
    {
        int x=(rand()%(c_eS_ub_dis-c_eS_lb_dis+1))+c_eS_lb_dis;
        temp_c_eS[i] = x;
    }
    
    // Introduce 10% failures in Cloud-to-ES
    int total_c_eS = no_eS;
    int to_remove_c_eS = (int)(total_c_eS * NETWORK_FAILURE_RATE);
    vector<int> indices(no_eS);
    for(int i=0;i<no_eS;i++) indices[i] = i;
    random_shuffle(indices.begin(), indices.end());
    
    int removed_c_eS = 0;
    for(int i=0; i<to_remove_c_eS && i<indices.size(); i++)
    {
        temp_c_eS[indices[i]] = -1;
        removed_c_eS++;
    }
    
    // Write to file
    for(int i=0;i<no_eS;i++)
    {
        fprintf(fptr,"%d ", temp_c_eS[i]);
    }
    fclose(fptr);
    cout << "Cloud-to-ES: Generated with " << removed_c_eS << " failures out of " 
         << total_c_eS << " connections (" << (removed_c_eS*100.0/total_c_eS) << "%)\n";

    //generating edgeServer to edgeServer distance WITH 10% FAILURES
    fptr=fopen("eS_eS_dis.txt","w");
    if(fptr==NULL)
    {
        cout<<"File Creation failed\n";
        exit(0);
    }

    // First generate all ES-ES connections
    vector<vector<int>> temp_eS_eS(no_eS, vector<int>(no_eS));
    vector<pair<int,int>> all_connections;
    
    for(int i=0;i<no_eS;i++)
    {
        for(int j=0;j<no_eS;j++)
        {
            int x=(rand()%(eS_eS_ub_dis-eS_eS_lb_dis+1))+eS_eS_lb_dis;
            temp_eS_eS[i][j] = x;
            if(i != j && i < j) // Only count each pair once
            {
                all_connections.push_back({i,j});
            }
        }
    }
    
    // Introduce 10% failures in ES-to-ES (bidirectional)
    int total_eS_eS = all_connections.size();
    int to_remove_eS_eS = (int)(total_eS_eS * NETWORK_FAILURE_RATE);
    random_shuffle(all_connections.begin(), all_connections.end());
    
    int removed_eS_eS = 0;
    for(int i=0; i<to_remove_eS_eS && i<all_connections.size(); i++)
    {
        int src = all_connections[i].first;
        int dst = all_connections[i].second;
        temp_eS_eS[src][dst] = -1;
        temp_eS_eS[dst][src] = -1; // Make it symmetric
        removed_eS_eS++;
    }
    
    // Write to file
    for(int i=0;i<no_eS;i++)
    {
        for(int j=0;j<no_eS;j++)
        {
            fprintf(fptr,"%d ", temp_eS_eS[i][j]);
        }
        fprintf(fptr,"\n");
    }
    fclose(fptr);
    cout << "ES-to-ES: Generated with " << removed_eS_eS << " failures out of " 
         << total_eS_eS << " connections (" << (removed_eS_eS*100.0/total_eS_eS) << "%)\n";

    //generating accessPoint to edgeServer distance WITH 10% FAILURES
    fptr=fopen("aP_eS_dis.txt","w");
    if(fptr==NULL)
    {
        cout<<"File Creation failed\n";
        exit(0);
    }

    vector<vector<int>> temp_aP_eS(no_aP, vector<int>(no_eS, -1));
    vector<pair<int,int>> ap_es_connections;
    map<int, int> ap_connection_count;
    
    deque<int> connected_eS; //to make sure aP connect to diff eS
    for(int i=0;i<no_aP;i++)
    {
        int connect_limit=(rand()%(aP_to_eS_ub-aP_to_eS_lb+1))+aP_to_eS_lb;
        set<int> eS_indexes;
        for(int j=0;j<connect_limit;j++)
        {
            //just to ensure a fair distribution of eS accross diff aP
            if(connected_eS.size()==no_eS)
            {
                for(int k=0;k<no_eS/2;k++) connected_eS.pop_front();
            }
            int x=(rand()%no_eS);
            int index=find(connected_eS.begin(),connected_eS.end(),x)-connected_eS.begin();
            if(index==connected_eS.size())
            {
                eS_indexes.insert(x);
                connected_eS.push_back(x);
            } 
            else j--;
        }
        
        ap_connection_count[i] = eS_indexes.size();
        
        for(int j=0;j<no_eS;j++)
        {
            if(eS_indexes.find(j)!=eS_indexes.end())
            {
                int x=(rand()%(aP_eS_ub_dis-aP_eS_lb_dis+1))+aP_eS_lb_dis;
                temp_aP_eS[i][j] = x;
                ap_es_connections.push_back({i,j});
            }
        }
    }
    
    // Introduce 10% failures in AP-to-ES (ensuring each AP keeps at least 1 connection)
    int total_aP_eS = ap_es_connections.size();
    int to_remove_aP_eS = (int)(total_aP_eS * NETWORK_FAILURE_RATE);
    random_shuffle(ap_es_connections.begin(), ap_es_connections.end());
    
    int removed_aP_eS = 0;
    for(int i=0; i<to_remove_aP_eS && i<ap_es_connections.size(); i++)
    {
        int ap = ap_es_connections[i].first;
        int es = ap_es_connections[i].second;
        
        // Only remove if this AP has more than 1 connection
        if(ap_connection_count[ap] > 1)
        {
            temp_aP_eS[ap][es] = -1;
            ap_connection_count[ap]--;
            removed_aP_eS++;
        }
    }
    
    // Write to file
    for(int i=0;i<no_aP;i++)
    {
        for(int j=0;j<no_eS;j++)
        {
            fprintf(fptr,"%d ", temp_aP_eS[i][j]);
        }
        fprintf(fptr,"\n");
    }
    fclose(fptr);
    cout << "AP-to-ES: Generated with " << removed_aP_eS << " failures out of " 
         << total_aP_eS << " connections (" << (removed_aP_eS*100.0/total_aP_eS) << "%)\n";
    cout << "=== Infrastructure Generation Complete ===\n\n";
}  

void load_Infrastructure_Details()
{
    //Storing cloud to edgeServer distances from file to vector
    FILE* fptr;
    fptr=fopen("c_eS_dis.txt","r");
    if(fptr==NULL)
    {
        cout<<"File Opening failed\n";
        exit(0);
    }

    for(int i=0;i<no_eS;i++)
    {
        fscanf(fptr,"%d",&c_eS_dis[i]);
        //cout<<c_eS_dis[i]<<" ";
    } 
    fclose(fptr);
    
    //Storing edgeServer to edgeServer distances from file to vector
   // fptr=fopen("infrastructure_details\\eS_eS_dis.txt","r");
    fptr=fopen("eS_eS_dis.txt","r");
    if(fptr==NULL)
    {
        cout<<"File Opening failed\n";
        exit(0);
    }

    for(int i=0;i<no_eS;i++)
    {
        for(int j=0;j<no_eS;j++)
        {
            fscanf(fptr,"%d",&eS_eS_dis[i][j]);
            //cout<<eS_eS_dis[i][j]<<" ";
        }
        //cout<<"\n";
    }
    fclose(fptr);

    //Storing accessPoint to edgeServer distances from file to vector
   // fptr=fopen("infrastructure_details\\aP_eS_dis.txt","r");
   fptr=fopen("aP_eS_dis.txt","r");
    if(fptr==NULL)
    {
        cout<<"File opening failed\n";
        exit(0);
    }

    for(int i=0;i<no_aP;i++)
    {
        for(int j=0;j<no_eS;j++)
        {
            fscanf(fptr,"%d",&aP_eS_dis[i][j]);
            //cout<<aP_eS_dis[i][j]<<" ";
        }
        //cout<<"\n";
    }
    fclose(fptr);

    //Storing edgeServer to accessPoint distances from file to vector
    for(int i=0;i<no_eS;i++)
    {
        for(int j=0;j<no_aP;j++)
        {
            eS_aP_dis[i][j]=aP_eS_dis[j][i];
        }
    }
}

void probability_Calculation()
{
    //accessPoint to edgeServer probability calculation
    memset(aP_eS_prob,0,sizeof(aP_eS_prob)); //marking probility 0 for edgeServers apart from neighbours
    for(int i=0;i<no_aP;i++)
    {
        double total=0;
        //taking inverse of distance and storing back to probibility array
        for(int j=0;j<no_eS;j++)
        {
            if(aP_eS_dis[i][j]!=-1) //if this eS connected then only calculate its probability
            {
                double value=1.0/1.0*aP_eS_dis[i][j];
                aP_eS_prob[i][j]=round(value*1000)/1000; //its actually round(1/dis * 1000) / 1000 so to round it to three decimal places
                total+=aP_eS_prob[i][j];
            } 
        }
        //caluating actual probability and storing back to probability array
        for(int j=0;j<no_eS;j++)
        {
            if(aP_eS_dis[i][j]!=-1)
            {
                double value=aP_eS_prob[i][j]/total;
                aP_eS_prob[i][j]=round(value*1000)/1000;
            }
            //cout<<aP_eS_prob[i][j]<<" ";
        }
       // cout<<"\n";
    }
    
    //cout<<"\n";
    //edgeServer to accessPoint probability calculation
    memset(eS_aP_prob,0,sizeof(eS_aP_prob)); //marking probility 0 for edgeServers apart from neighbours
    for(int i=0;i<no_eS;i++)
    {
        double total=0;
        //taking inverse of distance and storing back to probibility array
        for(int j=0;j<no_aP;j++)
        {
            if(eS_aP_dis[i][j]!=-1) //if this eS connected then only calculate its probability
            {
                double value=1.0/1.0*eS_aP_dis[i][j];
                eS_aP_prob[i][j]=round(value*1000)/1000; //its actually round(1/dis * 1000) / 1000 so to round it to three decimal places
                total+=eS_aP_prob[i][j];
            }
        }
        //caluating actual probability and storing back to probability array
        for(int j=0;j<no_aP;j++)
        {
            if(eS_aP_dis[i][j]!=-1)
            {
                double value=eS_aP_prob[i][j]/total;
                eS_aP_prob[i][j]=round(value*1000)/1000;
            }
            //cout<<eS_aP_prob[i][j]<<" ";
        }
        //cout<<"\n";
    }
    //cout<<"\n";
}

void eS_initialization()
{
   //cout<<"in eS_initialization\n";
	FILE* fptr;
    fptr=fopen("eS_initialization.txt","w");
    if(fptr==NULL)
    {
        cout<<"File Creation failed\n";
        exit(0);
    }
    
    //for split then prob of private space in ES will vary from 0.5 to 0.9 and for no split it is 0 
   
   for(int i=0;i<no_eS;i++)
    {
		pri_pub_ratio[i]=(rand()%5+5)/10.0;
		fprintf(fptr,"%lf ",pri_pub_ratio[i]);
	}
	fprintf(fptr,"\n");
	
    for(int i=0;i<no_eS;i++)
    {
        for(int j=0;j<eS_blockcount;j++)
        {
			int x=rand()%max_datachunks;
            eS_blocks[i][j]=x;
            fprintf(fptr,"%d ",x);
        }
        fprintf(fptr,"\n");
    }
    fclose(fptr);
}

void load_eS_initialization()
{
	//cout<<"in load_eS_initialization\n";
	FILE* fptr;
    fptr=fopen("eS_initialization.txt","r");
    if(fptr==NULL)
    {
        cout<<"File Opening failed\n";
        exit(0);
    }
    
    //for no split which means 100% public or shared space 
   /* for(int i=0;i<no_eS;i++)
    {
		fscanf(fptr,"%lf",&pri_pub_ratio[i]);
		//cout<<pri_pub_ratio[i]<<" ";
	}*/
	
	//for split which means prob of private space in ES will vary from 0.5 to 0.9 
    for(int i=0;i<no_eS;i++)
    {
		fscanf(fptr,"%lf",&pri_pub_ratio[i]);
		//cout<<pri_pub_ratio[i]<<" ";
	}
	
    for(int i=0;i<no_eS;i++)
    {
        for(int j=0;j<eS_blockcount;j++)
        {
            fscanf(fptr,"%d",&eS_blocks[i][j]);
            //cout<<eS_blocks[i][j]<<" ";
        }
        //cout<<"\n";
    }
    fclose(fptr);
}

void Generate_Request()
{
	FILE* fptr;
    fptr=fopen("Requests.txt","w");
    if(fptr==NULL)
    {
        cout<<"File Creation failed\n";
        exit(0);
    }
    
    int prev_batch_movie[max_datachunks];
    for(int i=0;i<max_datachunks;i++) prev_batch_movie[i]=0;
    
    int batch_no=0;
    int req_in_batch=0;
    
    int arrival_time=0;
    for(int i=0;i<NUM_req;i++)
    {
		int id=i;
		int aP=rand()%no_aP;
		int cid;
		if(req_in_batch==0)
		{
			batch_no++;
			req_in_batch=(rand()%(MAX_BATCH-MIN_BATCH+1))+MIN_BATCH;
			for(int j=0;j<max_datachunks;j++) prev_batch_movie[j]=0;
		}
		int prob_no=rand()%100;
		if(prob_no<CDATA)
		{
			for(int j=0;j<max_datachunks;j++)
			{
				if(prev_batch_movie[j]==1)
				{
					cid=j;
					break;
				}
			}
		}
		else
		{
			cid=rand()%max_datachunks;
			prev_batch_movie[cid]=1;
		}
		
		int temp_val=rand()%100;
		int deadline,profit;
		if(temp_val<HIGH_P)
		{
			deadline=HIGH_Deadline;
			profit=HIGH_Profit;
		}
		else if(temp_val<HIGH_P+MID_P)
		{
			deadline=MID_Deadline;
			profit=MID_Profit;
		}
		else
		{
			deadline=LOW_Deadline;
			profit=LOW_Profit;
		}
		
        fprintf(fptr,"%d %d %d %d %d %d\n",id,arrival_time,aP,cid,deadline,profit);
        req_in_batch--;
    }
    fclose(fptr);
}

void Load_Requests()
{
	FILE* fptr;
    fptr=fopen("Requests.txt","r");
    if(fptr==NULL)
    {
        cout<<"File Opening failed\n";
        exit(0);
    }
    
    for(int i=0;i<NUM_req;i++)
    {
		int id,arrival_time,aP,cid,deadline,profit;
        fscanf(fptr,"%d %d %d %d %d %d",&id,&arrival_time,&aP,&cid,&deadline,&profit);
        Req* temp=new Req(id,arrival_time,aP,cid,deadline,profit);
        R.push_back(temp);
    }
    fclose(fptr);
}




void LRU(int curr_eS,int cid,set<int>&data_chunks,int start,int end)
{
    //this is used to bring coded block of data items from cloud to edgeServer if not found when PutRequest2aP2 is called
    //fetching remiaining datachunks from Cloud to edgeServer
    while(data_chunks.size()!=req_datachunks)
    {
        //selecting the LRU for replacement
        int LRU_block=start;
        int LRU_time=access_time[curr_eS][start];
        for(int i=start;i<=end;i++)
        {
            if(access_time[curr_eS][i]<LRU_time)
            {
                LRU_time=access_time[curr_eS][i];
                LRU_block=i;
            }
        }
        //replacing the LRU block 
        for(int i=0;i<max_datachunks;i++)
        {
            if(data_chunks.find(i)==data_chunks.end())
            {
                eS_blocks[curr_eS][LRU_block]=i;
                access_time[curr_eS][LRU_block]=timer;
                timer++;
                data_chunks.insert(i);
                break;
            }
        }
    }
}

void search_in_eS(int curr_eS,int cid,set<int>&data_chunks,int aP)
{
    for(int i=0;i<eS_blockcount;i++)
    {
        if(eS_blocks[curr_eS][i]<max_datachunks and eS_blocks[curr_eS][i]==cid)
        {
            data_chunks.insert(eS_blocks[curr_eS][i]);
            access_time[curr_eS][i]=timer;
            timer++;
        }
        if(data_chunks.size()==req_datachunks) return;
    }
}

void PutRequest2aP_nosplit_erasure(Req *R)
{
    int id=R->id;
    int arrival_time=R->arrival_time;
    int aP=R->aP;
    int cid=R->cid;
    int deadline=R->deadline;
    int profit=R->profit;
    
    
  /*  int columnWidth = 15;
    
    cout << setw(columnWidth) << "id"
              << setw(columnWidth) << "arrival time"
              << setw(columnWidth) << "aP"
              << setw(columnWidth) << "cid"
              << setw(columnWidth) << "deadline"
              << setw(columnWidth) << "profit"
              << "\n";
    
    cout << setw(columnWidth) << id
              << setw(columnWidth) << arrival_time
              << setw(columnWidth) << aP
              << setw(columnWidth) << cid
              << setw(columnWidth) << deadline
              << setw(columnWidth) << profit
              << "\n";*/
    

    //search in edgeServers based on distance
    set<int> data_chunks;
    int time_taken=0;
    for(int j=0;j<no_eS;j++)
    {
        int dis=aP_eS_finalDis[aP][j].first;
        int curr_eS=aP_eS_finalDis[aP][j].second;
        time_taken=max(time_taken,dis);
        search_in_eS(curr_eS,cid,data_chunks,aP);
        if(data_chunks.size()==req_datachunks)
        {
          //  cout<<"Request satisfied with EdgeServers only\n";
            //cout<<"Time taken = "<<time_taken<<"\n";
            if(time_taken<=deadline)
            {
                hit_count_NSE++;
                profit_NSE+=profit;
                //cout<<"Status : Hit\n";
            } 
            //else cout<<"Status : Miss\n";
            return;
        }
    }

 //select the edge server connected to aP based upon probability and random data generation
    int temp_val=rand()%1000;
    int selected_eS=0;
    int low_range=0;
    for(int i=0;i<no_eS;i++)
    {
        if(aP_eS_prob[aP][i]!=0) //accessPoint is conntected to this egde Server 
        {
            selected_eS=i; //
            int high_range=low_range+(aP_eS_prob[aP][i]*1000);
            if(temp_val>=low_range and temp_val<=high_range)
            {
                selected_eS=i;
                break;
            }
            low_range=high_range+1;
        }
    }


    //if data_chunks search not fulfilled then finally get datachunk from cloud
    
    //edge server selected, now for aP whether its private block or public block are replaced 
    bool private_rep=false; //private replacement
    float temp=rand()%11;
    temp/=10.0;
    if(temp<=pri_pub_ratio[selected_eS]) private_rep=true;

    if(private_rep)
    {
        LRU(selected_eS,cid,data_chunks,eS_aP_blockStart[selected_eS][aP],eS_aP_blockEnd[selected_eS][aP]);
         time_taken=max(time_taken,c_eS_dis[selected_eS]+eS_aP_dis[selected_eS][aP]);
       // cout<<"\nRequest satisfied by cloud\n";
        //cout<<"Time taken = "<<time_taken<<"\n";
    }
    else
    {
        int pub_block_count_sno=floor(eS_blockcount*pri_pub_ratio[selected_eS]); //public block count start number
        
        LRU(selected_eS,cid,data_chunks,pub_block_count_sno,eS_blockcount-1);
         time_taken=max(time_taken,c_eS_dis[selected_eS]+eS_aP_dis[selected_eS][aP]);
       // cout<<"\nRequest satisfied by cloud\n";
        //cout<<"Time taken = "<<time_taken<<"\n";
    }

    if(time_taken<=deadline)
    {
        hit_count_NSE++;
        profit_NSE+=profit;
        //cout<<"Status : Hit\n";
    } 
    //else cout<<"Status : Miss\n";
}


void PutRequest2aP_split_erasure(Req *R)
{
    int id=R->id;
    int arrival_time=R->arrival_time;
    int aP=R->aP;
    int cid=R->cid;
    int deadline=R->deadline;
    int profit=R->profit;
    
    
    int columnWidth = 15;
    
    cout << setw(columnWidth) << "id"
              << setw(columnWidth) << "arrival time"
              << setw(columnWidth) << "aP"
              << setw(columnWidth) << "cid"
              << setw(columnWidth) << "deadline"
              << setw(columnWidth) << "profit"
              << "\n";
    
    cout << setw(columnWidth) << id
              << setw(columnWidth) << arrival_time
              << setw(columnWidth) << aP
              << setw(columnWidth) << cid
              << setw(columnWidth) << deadline
              << setw(columnWidth) << profit
              << "\n";
    

    //search in edgeServers based on distance
    set<int> data_chunks;
    int time_taken=0;
    for(int j=0;j<no_eS;j++)
    {
        int dis=aP_eS_finalDis[aP][j].first;
        int curr_eS=aP_eS_finalDis[aP][j].second;
        time_taken=max(time_taken,dis);
        search_in_eS(curr_eS,cid,data_chunks,aP);
        if(data_chunks.size()==req_datachunks)
        {
          //  cout<<"Request satisfied with EdgeServers only\n";
            cout<<"Time taken = "<<time_taken<<"\n";
            if(time_taken<=deadline)
            {
                hit_count_SE++;
                profit_SE+=profit;
                cout<<"Status : Hit\n";
            } 
            else cout<<"Status : Miss\n";
            return;
        }
    }

 //select the edge server connected to aP based upon probability and random data generation
    int temp_val=rand()%1000;
    int selected_eS=0;
    int low_range=0;
    for(int i=0;i<no_eS;i++)
    {
        if(aP_eS_prob[aP][i]!=0) //accessPoint is conntected to this egde Server 
        {
            selected_eS=i; //
            int high_range=low_range+(aP_eS_prob[aP][i]*1000);
            if(temp_val>=low_range and temp_val<=high_range)
            {
                selected_eS=i;
                break;
            }
            low_range=high_range+1;
        }
    }


    //if data_chunks search not fulfilled then finally get datachunk from cloud
    
    //edge server selected, now for aP whether its private block or public block are replaced 
    bool private_rep=false; //private replacement
    float temp=rand()%11;
    temp/=10.0;
    if(temp<=pri_pub_ratio[selected_eS]) private_rep=true;

    if(private_rep)
    {
        LRU(selected_eS,cid,data_chunks,eS_aP_blockStart[selected_eS][aP],eS_aP_blockEnd[selected_eS][aP]);
         time_taken=max(time_taken,c_eS_dis[selected_eS]+eS_aP_dis[selected_eS][aP]);
       // cout<<"\nRequest satisfied by cloud\n";
        cout<<"Time taken = "<<time_taken<<"\n";
    }
    else
    {
        int pub_block_count_sno=floor(eS_blockcount*pri_pub_ratio[selected_eS]); //public block count start number
        
        LRU(selected_eS,cid,data_chunks,pub_block_count_sno,eS_blockcount-1);
         time_taken=max(time_taken,c_eS_dis[selected_eS]+eS_aP_dis[selected_eS][aP]);
       // cout<<"\nRequest satisfied by cloud\n";
        cout<<"Time taken = "<<time_taken<<"\n";
    }

    if(time_taken<=deadline)
    {
        hit_count_SE++;
        profit_SE+=profit;
        cout<<"Status : Hit\n";
    } 
    else cout<<"Status : Miss\n";
}

void allot_blocks_aP_in_eS(int total,vector<int>&v,int eS)
{
	int start_block=0;
	int end_block;
	for(int i=0;i<v.size();i++)
	{
		int aP=v[i];
		int req_count=aP_reqCnt[i];
		int blocks_alloted=floor((req_count*eS_blockcount*pri_pub_ratio[eS])/total);
		eS_aP_blockStart[eS][aP]=start_block;
		eS_aP_blockEnd[eS][aP]=start_block+blocks_alloted-1;
		start_block=blocks_alloted;
	}
}

void update_eS_aP_blockSpace()
{
	vector<int> v;
	int total=0;
	for(int i=0;i<no_eS;i++)
	{
		for(int j=0;j<no_aP;j++)
		{
			if(eS_aP_dis[i][j]!=-1)
			{
				v.push_back(j);
				total+=aP_reqCnt[j]+1;  //we are adding 1 to avoid the case when no request arrived at some aP(like initially)
			}
		}
		allot_blocks_aP_in_eS(total,v,i);
		v.clear();
		total=0;
	}
	
}

void Simulate_nosplit_erasure()
{
    cout<<"No Split Erasure Code method started\n";
    sleep(3);
    //initialize request count on each aP as zero initially
	for(int i=0;i<no_aP;i++) aP_reqCnt[i]=0;
	
	 memset(access_time,0,sizeof(access_time));
	
    for(int i=0;i<NUM_req;i++)
    {
        if(i%1000==0) update_eS_aP_blockSpace();
        PutRequest2aP_nosplit_erasure(R[i]);
        aP_reqCnt[R[i]->aP]++;
        //cout<<"\n\n";
    }
    cout<<"\n";
}

void Simulate_split_erasure()
{
    cout<<"Split Erasure Code method started\n";
    sleep(3);
    //initialize request count on each aP as zero initially
	for(int i=0;i<no_aP;i++) aP_reqCnt[i]=0;
  
  memset(access_time,0,sizeof(access_time));
  
    for(int i=0;i<NUM_req;i++)
    {
        if(i%1000==0) update_eS_aP_blockSpace();
         PutRequest2aP_split_erasure(R[i]);
        aP_reqCnt[R[i]->aP]++;
        cout<<"\n\n";
    }
    /*cout<<"\n";
    cout<<"Split with Erasure Code Result \n";
    cout<<"Total request : "<<NUM_req<<"\n";
    cout<<"Total hit count : "<<hit_count_SE<<"\n";
    cout<<"Total miss count : "<<NUM_req-hit_count_SE<<"\n";
    cout<<"Hit % : "<<(hit_count_SE*100)/NUM_req<<"\n";
    cout<<"Total Profit : "<<profit_SE<<"\n";
    cout<<"\n\n";*/
}

void Store_and_View_Result()
{
	//storing Split with Erasure Code Result in file
	FILE* fptr=fopen("result.txt","w");
	fprintf(fptr,"No Split with Erasure Code Result \n");
	fprintf(fptr,"Total Request  ");
	fprintf(fptr,"%d\n",NUM_req);
	fprintf(fptr,"Total Hit Count  ");
	fprintf(fptr,"%d\n",hit_count_SE);
	fprintf(fptr,"Total Miss Count  ");
	fprintf(fptr,"%d\n",NUM_req-hit_count_SE);
	fprintf(fptr,"Hit %  ");
	fprintf(fptr,"%d\n",(hit_count_SE*100)/NUM_req);
	fprintf(fptr,"Profit  ");
	fprintf(fptr,"%d\n",profit_SE);
	fprintf(fptr,"\n");
	
	//storing NoSplit with Erasure Code Result in file
	fprintf(fptr,"Split with Erasure Code Result \n");
	fprintf(fptr,"Total Request  ");
	fprintf(fptr,"%d\n",NUM_req);
	fprintf(fptr,"Total Hit Count  ");
	fprintf(fptr,"%d\n",hit_count_NSE);
	fprintf(fptr,"Total Miss Count ");
	fprintf(fptr,"%d\n",NUM_req-hit_count_NSE);
	fprintf(fptr,"Hit %  ");
	fprintf(fptr,"%d\n",(hit_count_NSE*100)/NUM_req);
	fprintf(fptr,"Profit  ");
	fprintf(fptr,"%d\n",profit_NSE);
	
	//printing comparative result
	cout << setw(30) << left << " No Split Erasure Code Result" << " Split Erasure Code Result\n\n";
	cout << setw(17) << left << "Total Request : " << setw(13) << NUM_req;
	cout << setw(17) << left << "Total Request : " << setw(13) << NUM_req << "\n";

	cout << setw(17) << left << "Total Hit count : " << setw(13) << hit_count_SE;
	cout << setw(17) << left << "Total Hit count : " << setw(13) << hit_count_NSE << "\n";

	cout << setw(17) << left << "Total Miss count : " << setw(13) << (NUM_req - hit_count_SE);
	cout << setw(17) << left << "Total Miss count : " << setw(13) << (NUM_req - hit_count_NSE) << "\n";

	cout << setw(17) << left << "Hit % : " << setw(13) << fixed << setprecision(2) << (static_cast<double>(hit_count_SE) * 100 / NUM_req);
	cout << setw(17) << left << "Hit % : " << setw(13) << fixed << setprecision(2) << (static_cast<double>(hit_count_NSE) * 100 / NUM_req) << "\n";

	cout << setw(17) << left << "Total Profit : " << setw(13) << profit_SE;
	cout << setw(17) << left << "Total Profit : " << setw(13) << profit_NSE << "\n";
}   

void aP_eS_finalDis_calculation()
{
	for(int i=0;i<no_aP;i++)
	{
		//for each aP calculating final min distance to each eS represented as j
		for(int j=0;j<no_eS;j++)
		{
			//if directly connnected means this is min, so directly update this
			if(aP_eS_dis[i][j]!=-1)
			{
				//here we stored as {distance,eS}
				aP_eS_finalDis[i].push_back({aP_eS_dis[i][j],j});
				continue;
			}
			//otherwise check each possibilty of through which eS it is nearest in total 2 hops
			int dis=INT_MAX;
			for(int k=0;k<no_eS;k++)
			{
				if(aP_eS_dis[i][k]!=-1 && eS_eS_dis[k][j]!=-1)
				{
					dis=min(dis,aP_eS_dis[i][k]+eS_eS_dis[k][j]);
				}	
			}
			aP_eS_finalDis[i].push_back({dis,j});
		}
	}
	
	//sorting based on the distance
	for(int i=0;i<no_aP;i++)
	{
		sort(aP_eS_finalDis[i].begin(),aP_eS_finalDis[i].end());
	}
	//printing final aP_eS_dis 
	/*for(int i=0;i<no_aP;i++)
	{
		for(int j=0;j<no_eS;j++)
		{
			cout<<aP_eS_finalDis[i][j].first<<" "<<aP_eS_finalDis[i][j].second<<"|";
		}
		cout<<"\n";
	}*/
}


int main()
{
  srand((unsigned)time(0));
  generate_Infrastructure();
  load_Infrastructure_Details();
  probability_Calculation();
  eS_initialization();
  load_eS_initialization();
  Generate_Request();
  Load_Requests();
    
  aP_eS_finalDis_calculation();
    
  Simulate_nosplit_erasure();
  load_eS_initialization();
  Simulate_split_erasure();
  Store_and_View_Result();
  cout<<"\n"<<"total_profit="<<total_profit<<"\n";
  return 0;
}
