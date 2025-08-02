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
#define aP_eS_lb_dis 2 
#define aP_eS_ub_dis 3

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

//ofstream ReqTrace("RequestTrace.txt"); //output file name

int NUM_req =100000;
#define no_movies 50000
#define max_datachunks 24
#define req_datachunks 4
#define eS_blockcount 20

#define pri_pub_ratio 0.7

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
    //generating cloud to edgeServer distance
    FILE* fptr;
    fptr=fopen("infrastructure_details\\c_eS_dis.txt","w");
    if(fptr==NULL)
    {
        cout<<"File Creation failed\n";
        exit(0);
    }

    for(int i=0;i<no_eS;i++)
    {
        int x=(rand()%(c_eS_ub_dis-c_eS_lb_dis+1))+c_eS_lb_dis;
        fprintf(fptr,"%d ",x);
    }
    fclose(fptr);

    //generating edgeServer to edgeServer distance
    fptr=fopen("infrastructure_details\\eS_eS_dis.txt","w");
    if(fptr==NULL)
    {
        cout<<"File Creation failed\n";
        exit(0);
    }

    for(int i=0;i<no_eS;i++)
    {
        for(int j=0;j<no_eS;j++)
        {
            int x=(rand()%(eS_eS_ub_dis-eS_eS_lb_dis+1))+eS_eS_lb_dis;
            fprintf(fptr,"%d ",x);
        }
        fprintf(fptr,"\n");
    }
    fclose(fptr);

    //generating accessPoint to edgeServer distance
    fptr=fopen("infrastructure_details\\aP_eS_dis.txt","w");
    if(fptr==NULL)
    {
        cout<<"File Creation failed\n";
        exit(0);
    }

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
        for(int j=0;j<no_eS;j++)
        {
            if(eS_indexes.find(j)!=eS_indexes.end())
            {
                int x=(rand()%(aP_eS_ub_dis-aP_eS_lb_dis+1))+aP_eS_lb_dis;
                fprintf(fptr,"%d ",x);
            }
            else
            {
                fprintf(fptr,"%d ",-1);
            }
        }
        fprintf(fptr,"\n");
    }
    fclose(fptr);
}

void load_Infrastructure_Details()
{
    //Storing cloud to edgeServer distances from file to vector
    FILE* fptr;
    fptr=fopen("infrastructure_details\\c_eS_dis.txt","r");
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
    fptr=fopen("eS_to_eS.txt","r");
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
   fptr=fopen("aP_to_eS.txt","r");
    if(fptr==NULL)
    {
        cout<<"File Creation failed\n";
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
       // cout<<"\n";
    }
}

void eS_initialization()
{
    //randomaly putting movies along with block no in edge Servers
    
    for(int i=0;i<no_eS;i++)
    {
        //set<int> data_chunks;
        for(int j=0;j<eS_blockcount;j++)
        {
            int movie_no=rand()%no_movies; //first 2 digit represent movie no 
            int datachuck_no=rand()%max_datachunks; //last 2 digit represents block_no
            int chunk=(movie_no*100)+datachuck_no; //done to fit it in 4 digit integer
            eS_blocks[i][j]=chunk;
        }
        //cout<<"\n";
    }
	
	//store eS_initialization in file for further use it during nosplit erasure method is called, since same eS_intialization is required
	FILE* fptr=fopen("eS_initialization","w");
	if(fptr==NULL)
	    {
		cout<<"File Creation failed\n";
		exit(0);
	    }

	for(int i=0;i<no_eS;i++)
	{
		for(int j=0;j<eS_blockcount;j++)
		{
			fprintf(fptr,"%d ",eS_blocks[i][j]);
		}
		fprintf(fptr,"\n");
	}
	fclose(fptr);
	
    //set all block access time to zero initially
    memset(access_time,0,sizeof(access_time));
}

void load_eS_initialization()
{
	FILE* fptr=fopen("eS_initialization","r");
	if(fptr==NULL)
	    {
		cout<<"File Opening failed\n";
		exit(0);
	    }

	for(int i=0;i<no_eS;i++)
	{
		for(int j=0;j<eS_blockcount;j++)
		{
			fscanf(fptr,"%d",&eS_blocks[i][j]);
			//cout<<eS_blocks[i][j]<<" ";
		}
		//fprintf(fptr,"\n");
		//cout<<"\n";
	}
	fclose(fptr);
	timer=0;
	memset(access_time,0,sizeof(access_time));
}

void PrintTrace()
{
	int size=R.size();
	FILE* fptr=fopen("RequestTrace.txt","w");
	if(fptr==NULL)
	{
		cout<<"PrintTrace File Opening Failed"<<"\n";
		return;
	}
	for(int i=0;i<size;i++)
	{
		fprintf(fptr,"%d %d %d %d %d %d\n",R[i]->id,R[i]->arrival_time,R[i]->aP,R[i]->cid,R[i]->deadline,R[i]->profit);
	}
	fclose(fptr);
}

void Generate_Request()
{
	
    //creating extra vector to store and sort so that it will not affect no_split erasure code method
    vector<Req*> uniqueReq;
    int repeatCount = (NUM_req* CDATA) / 100;
    int uniqueCount = NUM_req - repeatCount;
    int id=0;
    int arrival_time=0;
    
    for (int i = 0; i < uniqueCount; i++)
     {
        // Generate unique random values for each field
        int aP = random() % no_aP; 
        int cid = random() % no_movies;  
        int randint = random() % 100;
        if (randint >= 0 && randint <= HIGH_P)
        {
            R.push_back(new Req(id, arrival_time, aP, cid, HIGH_Deadline, HIGH_Profit));
            uniqueReq.push_back(new Req(id, arrival_time, aP, cid, HIGH_Deadline, HIGH_Profit));
        }
        else if (randint > HIGH_P && randint <= MID_P + HIGH_P)
        {
            R.push_back(new Req(id, arrival_time,aP, cid, MID_Deadline, LOW_Profit));
            uniqueReq.push_back(new Req(id, arrival_time,aP, cid, MID_Deadline, LOW_Profit));
        }
        else
        {
            R.push_back(new Req(id, arrival_time, aP, cid, LOW_Deadline, MID_Profit));
            uniqueReq.push_back(new Req(id, arrival_time, aP, cid, LOW_Deadline, MID_Profit));
        }
	if(i%5==0 and i!=0) arrival_time+=random()%10;
	id++;
    }
    
     // Sort unique requests by aP and cid
    std::sort(uniqueReq.begin(), uniqueReq.end(), [](const Req* a, const Req* b) {
        return std::tie(a->aP, a->cid) < std::tie(b->aP, b->cid);
    });
    
    for (int i = 0; i < repeatCount; i++) {
        int aP = uniqueReq[i % uniqueCount]->aP;  
        int cid = uniqueReq[i % uniqueCount]->cid;  
        int deadline = uniqueReq[i % uniqueCount]->deadline;  
        int profit =  uniqueReq[i % uniqueCount]->profit;  
	if(i%5==0) arrival_time=arrival_time+random()%10;
        R.push_back(new Req(id++, arrival_time, aP, cid, deadline, profit));
    }
    // Clean up uniqueRequests vector
    for (Req* req : uniqueReq) {
        delete req;
    }
    PrintTrace();
}


void Load_Requests()
{
	
 unsigned int number_of_lines = 0;
	FILE* fptr=fopen("RequestTrace.txt","r");
	if(fptr==NULL)
	    {
		cout<<"File Opening failed\n";
		exit(0);
	    }
	   int ch;
	    while (EOF != (ch=getc(fptr)))
		if ('\n' == ch)
		    ++number_of_lines;
	//   printf("%u\n", number_of_lines);
	    NUM_req=number_of_lines-1;
	    fclose(fptr);
	    FILE* fptr1=fopen("RequestTrace.txt","r");
	if(fptr1==NULL)
	    {
		cout<<"File Opening failed\n";
		exit(0);
	    }
	    int id;
    int arrival_time;
    int aP;
    int cid;
    int deadline;
    int profit;
    for(int i=0;i<NUM_req;i++)
    {
    	fscanf(fptr1,"%d",&id);
    	fscanf(fptr1,"%d",&arrival_time);
    	fscanf(fptr1,"%d",&aP);
        fscanf(fptr1,"%d",&cid);
        fscanf(fptr1,"%d",&deadline);
    	fscanf(fptr1,"%d",&profit);
    	R.push_back(new Req(id,arrival_time,aP, cid, deadline, profit));
    	total_profit+=profit;
    }
    fclose(fptr1);
}

void search_in_Rem_eS(int curr_eS,int cid,set<int>&data_chunks,int &time_taken,int selected_eS)
{
    for(int i=0;i<eS_blockcount;i++)
    {
        if(data_chunks.size()==req_datachunks) return; // required no of data chunks found
        int chunk=eS_blocks[curr_eS][i];
        if((chunk/100)==cid)
        {
            if(data_chunks.find(chunk)==data_chunks.end()) //curr not present in datachunks set
            {
                //since this block is accessed so assign its access time
                access_time[curr_eS][i]=++timer;

                data_chunks.insert(chunk);
                time_taken=max(time_taken,eS_eS_dis[curr_eS][selected_eS]);
            }
        }
    }
}


void search_in_eS(int curr_eS,int cid,set<int>&data_chunks,int aP)
{
    for(int i=0;i<eS_blockcount;i++)
    {
        if(data_chunks.size()==req_datachunks) return; // required no of data chunks found
        int chunk=eS_blocks[curr_eS][i];
        if((chunk/100)==cid)
        {
            if(data_chunks.find(chunk)==data_chunks.end()) //curr not present in datachunks set
            {
                //since this block is accessed so assign its access time
                access_time[curr_eS][i]=++timer;

                data_chunks.insert(chunk);
            }
        }
    }
}


void LRU(int eS,int cid,set<int>&data_chunks,int low_idx,int high_idx)
{
/*
    //printing edge server status before LRU replacement     
    int colWidth= 10;
    cout<<"Edge Server status before LRU replacement :\n";
    std::cout << std::left << std::setw(15) << "Datablocks";

    for(int j=0;j<eS_blockcount;j++)
    {
	cout<<setw(colWidth)<<eS_blocks[eS][j];
    }
    cout<<"\n";
    std::cout << std::left << std::setw(15) << "Access Time";
    for(int j=0;j<eS_blockcount;j++)
    {
	cout<<setw(colWidth)<<access_time[eS][j];
    }
*/

    //acutal implementation
    for(int i=0;i<max_datachunks;i++)
    {
        if(data_chunks.size()==req_datachunks) break;
        int replace_idx=low_idx;
        int chunk=(100*cid)+i;
        if(data_chunks.find(chunk)==data_chunks.end())
        {
            //insert this in our data_chunks set so that when size reaches req_datachunks size we stops
            data_chunks.insert(chunk);
            //search of least recently used index
            int least_time=INT_MAX;
            for(int j=low_idx;j<=high_idx;j++)
            {
                if(access_time[eS][j]<least_time)
                {
                    least_time=access_time[eS][j];
                    replace_idx=j;
                }
            }

            //put the required chunk on this eS at least recently used data block index replacing previos data chunk
            eS_blocks[eS][replace_idx]=chunk;
            access_time[eS][replace_idx]=++timer;
        }
    }
/*
    //printing edge server status after LRU replacement 

    cout<<"\nEdge Server status after LRU replacement :\n";
    std::cout << std::left << std::setw(15) << "Datablocks";
    for(int j=0;j<eS_blockcount;j++)
    {
	cout<<setw(colWidth)<<eS_blocks[eS][j];
    }
    cout<<"\n";
    std::cout << std::left << std::setw(15) << "Access Time";
    for(int j=0;j<eS_blockcount;j++)
    {
	cout<<setw(colWidth)<<access_time[eS][j];
    }
    */
}


void PutRequest2aP_nosplit_erasure(Req* curr)
{
    int id = curr->id;
	int arrival_time = curr->arrival_time;
	int aP = curr->aP;
    int cid = curr->cid;
	int deadline = curr->deadline;
	int profit = curr->profit;

    //printing request
    int columnWidth = 15;

    cout <<  setw(columnWidth) <<"id"
              << setw(columnWidth) << "arrival_time"
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
                hit_count_NSE++;
                profit_NSE+=profit;
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

    if(data_chunks.size()!=req_datachunks)
    {
    //temp_val is to decide whether its public or private replacement
    	float temp_val=rand()%11;
    	temp_val/=10.0;
    	
    	int pub_block_count_sno=floor(eS_blockcount*pri_pub_ratio); //public block count start number
    	
    	//private replacement
    	if(temp_val<=pri_pub_ratio) LRU(selected_eS,cid,data_chunks,0,pub_block_count_sno-1);
    	//public replacement
    	else LRU(selected_eS,cid,data_chunks,pub_block_count_sno,eS_blockcount-1);
    	
        time_taken=max(time_taken,c_eS_dis[selected_eS]+eS_aP_dis[selected_eS][aP]);
        //cout<<"\nRequest satisfied by cloud\n";
        cout<<"Time taken = "<<time_taken<<"\n";
    }

    if(time_taken<=deadline)
    {
        hit_count_NSE++;
        profit_NSE+=profit;
        cout<<"Status : Hit\n";
    } 
    else cout<<"Status : Miss\n";
}


void PutRequest2aP_split_erasure(Req* curr)
{
    int id = curr->id;
	int arrival_time = curr->arrival_time;
	int aP = curr->aP;
    int cid = curr->cid;
	int deadline = curr->deadline;
	int profit = curr->profit;

    //printing request
    int columnWidth = 15;

    cout <<  setw(columnWidth) <<"id"
              << setw(columnWidth) << "arrival_time"
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
    if(temp<=pri_pub_ratio) private_rep=true;

    if(private_rep)
    {
        LRU(selected_eS,cid,data_chunks,eS_aP_blockStart[selected_eS][aP],eS_aP_blockEnd[selected_eS][aP]);
         time_taken=max(time_taken,c_eS_dis[selected_eS]+eS_aP_dis[selected_eS][aP]);
       // cout<<"\nRequest satisfied by cloud\n";
        cout<<"Time taken = "<<time_taken<<"\n";
    }
    else
    {
        int pub_block_count_sno=floor(eS_blockcount*pri_pub_ratio); //public block count start number
        
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
		int blocks_alloted=floor((req_count*eS_blockcount*pri_pub_ratio)/total);
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
        cout<<"\n\n";
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
    cout<<"\n";
    cout<<"Split with Erasure Code Result \n";
    cout<<"Total request : "<<NUM_req<<"\n";
    cout<<"Total hit count : "<<hit_count_SE<<"\n";
    cout<<"Total miss count : "<<NUM_req-hit_count_SE<<"\n";
    cout<<"Hit % : "<<(hit_count_SE*100)/NUM_req<<"\n";
    cout<<"Total Profit : "<<profit_SE<<"\n";
    cout<<"\n\n";
}

void Store_and_View_Result()
{
	//storing Split with Erasure Code Result in file
	FILE* fptr=fopen("result.txt","w");
	fprintf(fptr,"Split with Erasure Code Result \n");
	fprintf(fptr,"Total Request : ");
	fprintf(fptr,"%d\n",NUM_req);
	fprintf(fptr,"Total Hit Count : ");
	fprintf(fptr,"%d\n",hit_count_SE);
	fprintf(fptr,"Total Miss Count : ");
	fprintf(fptr,"%d\n",NUM_req-hit_count_SE);
	fprintf(fptr,"Hit % : ");
	fprintf(fptr,"%d\n",(hit_count_SE*100)/NUM_req);
	fprintf(fptr,"Profit : ");
	fprintf(fptr,"%d\n",profit_SE);
	fprintf(fptr,"\n");
	
	//storing NoSplit with Erasure Code Result in file
	fprintf(fptr,"No Split with Erasure Code Result \n");
	fprintf(fptr,"Total Request : ");
	fprintf(fptr,"%d\n",NUM_req);
	fprintf(fptr,"Total Hit Count : ");
	fprintf(fptr,"%d\n",hit_count_NSE);
	fprintf(fptr,"Total Miss Count : ");
	fprintf(fptr,"%d\n",NUM_req-hit_count_NSE);
	fprintf(fptr,"Hit % : ");
	fprintf(fptr,"%d\n",(hit_count_NSE*100)/NUM_req);
	fprintf(fptr,"Profit : ");
	fprintf(fptr,"%d\n",profit_NSE);
	
	//printing comparative result
	cout << setw(30) << left << "Split Erasure Code Result" << "No Split Erasure Code Result\n\n";
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
				if(aP_eS_dis[i][k]!=-1)
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
// load_Infrastructure_Details();
    probability_Calculation();
    eS_initialization();
	load_eS_initialization();
Generate_Request();
    Load_Requests();
    
   aP_eS_finalDis_calculation();
    
  Simulate_nosplit_erasure();
       // load_eS_initialization();
  //Simulate_split_erasure();
    Store_and_View_Result();
    cout<<"\n"<<"total_profit="<<total_profit<<"\n";
    return 0;
}
