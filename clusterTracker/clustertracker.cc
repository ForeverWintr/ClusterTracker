#include<iostream>
#include<iomanip>
#include<fstream>
#include<cmath>
#include<vector>

#include "cluster.h"

using namespace std;

#define NUMCUBES 200
#define NEIGHBORDIST 55 //arbitrary

//functions
int read(string, vector<slice>&, int); //returns number of slices or -1 on error
void usage(char *a) {cout<<"USAGE: "<<a<<
    " filename.txt number-of-images new-height new-width Min_Cluster_dist Min_Cluster_Cubes"<<endl;} //print usage
void clustersearch(cube& index, vector<line>& distances, vector<cube>& cubes);
void printcsv(vector<slice>& state);//print cluster info in csv format!
bool printslice(const int&, const vector<slice>&); //print a slice.
bool process(slice&, int &dist, int &h, int &w, int& mincubes);
bool onEdge(point &m, int &dist, int &h, int &w);

int main(int argc, char **argv) {
  
  vector<slice> state;

  if(argc != 7){
    usage(argv[0]);
    exit(1);
  }
  string file=argv[1];  
  int newh=atoi(argv[3]);
  int neww=atoi(argv[4]);
  int dist=atoi(argv[5]);
  int mincubes=atoi(argv[6]);

  state.resize(atoi(argv[2])+1);
  
  int slices=read(file, state, dist);
  //  cout<<slices<<" slices read"<<endl;
  

  //test area


  for(int i=1; i<state.size(); i++){
    if(!process(state[i], dist, newh, neww, mincubes))
      cout<<"slice "<<i<<" doesn't exist"<<endl;
  }
  printcsv(state);

}

//print output in csv format
void printcsv(vector<slice>& state){
  int seconds=0;
  int maxclusters=0;
  //find max number of clusters
  for(int i=0; i<state.size(); i++)
    if(state[i].clust.size() > maxclusters)
      maxclusters=state[i].clust.size();
  
  //cout<<"Maxclusters: "<<maxclusters<<endl;
  //print banner
  cout<<"Image,Time,Cubes,# of perimeter cubes,Clusters,Clustered Cubes,";
  for(int i=1; i<=maxclusters; i++)
    cout<<"Size "<<i<<",X"<<i<<",Y"<<i<<",";
  cout<<endl;
 
  for(int i=1; i<state.size(); i++){
    cout<<i<<","<<seconds<<",";
    state[i].print(maxclusters);
    cout<<endl;
    seconds+=30;
  }
}

//recursive cube search in cluster, takes list of edges shorter than threshold distance
//find index on LHS
//mark as visited, pushto cubes
//for each LHS entry, look at entries on RHS.
//keep track of cubes found
void clustersearch(cube& index, vector<line>& distances, cluster& cubes){
  //  cout<<"looking at cube: "<<index<<endl;
  if(!cubes.contains(index)){
    //cout<<"adding cube "<<index<<endl;
    index.incluster=true;
    cubes.add(index);
    
    //first scan lhs
    for(int i=0; i<distances.size(); i++){
      if(distances[i].a == index){//this scans lhs, need to also scan rhs
	//	if(distances[i].length != -1){
	  distances[i].a.incluster=true;
	  if(!distances[i].b.incluster){//if b is not in cluster yet
	    distances[i].b.incluster=true;
	    //  cout<<"recursing from lhs "<<distances[i].a<<" on "<<distances[i].b<<endl;
	    clustersearch(distances[i].b, distances, cubes);
	  }  
      }
    }
    //THEN scan rhs
    for(int i=0; i<distances.size(); i++){
      if(distances[i].b == index){//this scans rhs
	distances[i].b.incluster=true;
	  if(!distances[i].a.incluster){//if a is not in cluster yet
	    distances[i].a.incluster=true;
	    //   cout<<"recursing clustersearch from rhs on"<<distances[i].a<<endl;
	    clustersearch(distances[i].a, distances, cubes);
	  }  
      } 
    }
  }
  //  cout<<"exiting clustersearch"<<endl;
}

//find clusters and edge cubes. return false if slice uninitialized
bool process(slice &img, int &dist, int &h, int &w, int& mincubes){
  if(img.objects < 0) //slice is uninitialized
    return false;
  int edgecount=0;
  int c=0;
  int ccubes=0;
  vector<line> distances;

  //create a vector of distances between cubes. i.e. a complete graph
  //with cubes as vertices
  for(int i=1; i<=img.objects; i++){
    if(img.block[i].isonedge = onEdge(img.block[i].middle, dist, h, w))
      edgecount++;
    for(int j=i+1; j<=img.objects; j++){
      //set all distances less than threshold dist to -1
      line d(img.block[i], img.block[j]); //floating point exception when x vals are same because horizontal line has undefined slope
      if(d.length > dist)
	d.length= -1;
      else //I just added this, no idea why i didn't sooner...
	distances.push_back(d);
      c++;
    }
  }
  img.perimcubes=edgecount;

  for(int i=0; i< distances.size(); i++){
    //    cout<<"Distances["<<i<<"]: "<<distances[i]<<endl;
    //just to make sure things were initialized correctly
    if(distances[i].a.incluster || distances[i].b.incluster){
      cout<<"ERROR: cube erroneously marked as cluster. Fixing."<<endl;
      distances[i].a.incluster=distances[i].b.incluster=false;
    }
  }

  //find cluster
  for(int i=0; i< distances.size(); i++){
    if(distances[i].length != -1){
      if(!distances[i].a.incluster){
	cluster tempclust;
	//	cout<<"-------Clustersearch start--------"<<endl;
	clustersearch(distances[i].a, distances, tempclust);
	//cout<<"cluster at: "<<tempclust.loc()<<" of size: "<<tempclust.size()<<endl;
	if(tempclust.size() >= mincubes){
	  img.clust.push_back(tempclust);
	  ccubes+=tempclust.size();
	}
	//	cout<<"distances["<<i<<"].a: "<<distances[i].a<<" is incluster? "<<distances[i].a.incluster<<endl;
	//tempclust.update();	
      }
    }
  }
  img.clustercubes=ccubes;
  return true;
}

//returns true if point is on perimeter
bool onEdge(point &m, int &dist, int &h, int &w){ 
  if(m.x <= dist || m.x >= w-dist 
     || m.y <= dist || m.y >= h-dist)
    return true;
  else 
    return false;
}

//print a slice, return true if successfull, false if slice doesn't exist
bool printslice(const int& num, vector<slice>& state){ 
  if(state.size() <= num)
    return false;
  cout<<endl<<"----------Slice "<<num<<"-----------"<<endl;
  cout<<state[num].objects<<" objects"<<", "
      <<state[num].perimcubes<<" perimeter cube(s)"<<endl;
    //      <<state[num].avgsize<<endl;
  for(int i=1; i<=state[num].objects; i++){
    cout<<"Cube "<<i<<": area="<<state[num].block[i].area<<", M=("
	<<state[num].block[i].middle.x<<", "<<state[num].block[i].middle.y
	<<"), on edge="<<state[num].block[i].isonedge
	<<endl;
  }
  return true;
}

//open file. takes filename, array of slices, distance less than which
//blocks are neighbors, returns number of slices read
int read(string filename, vector<slice> &state, int dist)
{
  ifstream file;
  char in[256];
  char * pfortok;
  int cube=0, area, x, y, s, sprev=1, objects=0, prevcubes=1;
  double ydif;
  point temp;

  //state[1].avgsize=0;
  file.open(filename.c_str());
  if(!file.good())
    return -1;

  file.getline(in,256); //skip first line
  //read until eof?
  while(! file.eof()){
    file.getline(in,256); //in contains a line
    
    //tokenize in based on tabs
    pfortok = strtok (in,"\t");
    if(pfortok != NULL){
      cube=atoi(pfortok);
      area = atoi(strtok (NULL, "\t"));
      x = int(0.5 + atoi(strtok (NULL, "\t"))); //should be a to double
      y = int(0.5 + atoi(strtok (NULL, "\t")));
 
      s= atoi(strtok (NULL, "\t"));
      
      if(sprev == s)
	objects++;
      else{ //we are looking at a new slice
	state[sprev].objects=objects;
	//state[sprev].avgsize=state[sprev].avgsize/objects;
	//state[s].avgsize=0;
	objects=1;
	prevcubes=cube;
      }
      cube=cube-prevcubes+1;
      
      sprev=s;
      pfortok = strtok (NULL, "\t");
      if(pfortok != NULL)//pfortok should now be null
	cout<<"ERROR: PFORTOK IS NOT NULL"<<endl;
      
      temp.x=x;
      temp.y=y;
      //ydif=distance(temp, bottom)*increment;
      //state[s].block[cube].sizeoffset=ydif;
      //  state[s].avgsize+=ydif;
      state[s].block[cube].area=area;
      state[s].block[cube].middle.x=x;
      state[s].block[cube].middle.y=y;
      
    }
  }
  state[s].objects=objects;
  return s;
}
