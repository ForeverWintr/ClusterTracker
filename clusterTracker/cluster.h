//Classes for clustertracker
#ifndef CLUSTER_H
#define CLUSTER_H

#include<iostream>
#include<iomanip>
#include<fstream>
#include<cmath>
#include<vector>

using namespace std;

#define NUMCUBES 200

class point{//x is width, not like in img proc
public:
  point(int a=0, int b=0) :x(a), y(b){}
  friend ostream& operator<<(ostream &ostr, const point &p){ostr<<"("<<p.x<<", "<<p.y<<")";
    return ostr;}
  point operator+(const point &i){return point(i.x+x, i.y+y);}
  point& operator+=(const point &i){x+=i.x; y+=i.y; return *this;}
  point operator/(const int &i){return point(int(x/i +0.5), int(y/i+0.5));}
  point& operator/=(const int &i){x=int(x/i+0.5); y=int(y/i+0.5); return *this;}
  friend bool operator==(const point& p, const point& q){return (q.x==p.x && q.y == p.y);}
  int x, y;
};

class cube {
public:
  //cube() : middle(0,0), area(0), isonedge(0), incluster(0) {}// compile fails w/this line
 cube(point m, int a=0, bool e=false) : middle(m), isonedge(e), area(a), incluster(false) {}//{ middle=m; area=a; isonedge=e;}
 cube(int a=0, bool e=false) :area(a), isonedge(e), incluster(false) {}
  friend ostream& operator<<(ostream& Ostr, const cube& c) {Ostr<<c.middle; return Ostr;}
  friend bool operator==(const cube& c, const cube& e){return e.middle==c.middle;}
  point middle;
  int area;
  bool isonedge;
  bool incluster;
};

struct line{
  line() : slope(0), length(-1), yintercept(0){}
  line(cube one, cube two){
    a=one; b=two;
    int ax=a.middle.x, bx=b.middle.x, ay=a.middle.y, by=b.middle.y;
    if(ax == bx && ay == by)
      cout<<"cube at ("<<ax<<", "<<ay<<") has no line to itself"<<endl;
    else{
      if(ax==bx) //line is vertical and has no slope
	slope=701337;
      else
	slope=(ay-by) / (ax-bx);//m
      length=sqrt(pow(double(ay-by), 2)+pow(double(ax-bx),2));
      yintercept=(slope*ax-ay)*-1;//y=mx+b or -b=mx-y or -yintercept=slope*x-y
    }
  }
 friend ostream& operator<<(ostream& Ostr, const line& l) {
   Ostr<<l.a<<fixed<<setprecision (1)<<" --/"<<l.length<<"/-- "<<l.b; return Ostr;}
  cube a, b;
  double slope, length, yintercept;
};

class cluster {
  point location;
  vector<cube> cubes; //all the cubes in the cluster
  // tightness; //average dist between cubes?
 public:
 cluster() :location(0,0) {}
  void add(const cube& c); //add a cube to the cluster
  void update(); //update location, tightness, etc
  int size() {return cubes.size();}
  bool contains(const cube& c);
  point loc() {return location;}
  int x() {return location.x;}
  int y() {return location.y;}
};

//does the cluster contain a cube already?
bool cluster::contains(const cube& c){
  for(int i=0; i<cubes.size(); i++)
    if(c == cubes[i])
      return true;
  return false;
}

//update the dependant vars in cluster
void cluster::update(){
  //do some averaging
  location=0; //tightness=0;
  for(int i=0; i<size(); i++){
    location+=cubes[i].middle;
  }
  location/=size();
}

void cluster::add(const cube& c){
  cubes.push_back(c);
  update();
}

class slice {
 public:
  slice() :objects(-1), perimcubes(0), clustercubes(0){}
  void print(const int&);
  void printv(const int&);
  int objects;
  cube block[NUMCUBES];
  int perimcubes, clustercubes;
  vector<cluster> clust;
  // double avgsize;
};

//print slice info in csv format
void slice::print(const int& maxclust){
  int i=0;
  cout<<objects<<","<<perimcubes<<","<<clust.size()<<","<<clustercubes<<",";
  for(i=0; i<clust.size(); i++){ //print info on clusters
    cout<<clust[i].size()<<","<<clust[i].x()<<","<<clust[i].y()<<",";
  }
  //fill in extra zeroes
  for(i; i<maxclust; i++)
    cout<<0<<","<<0<<","<<0<<",";
  //  cout<<endl;
}

//print slice info with labels
void slice::printv(const int& maxclust){
  int i;
  cout<<"Objects: "<<objects<<", perimcubes: "<<perimcubes<<", clusters: "<<clust.size()<<", clustercubes: "<<clustercubes<<", ";
  for(i=0; i<clust.size(); i++){ //print info on clusters
    cout<<"size of cluster "<<i<<": "<<clust[i].size()
	<<", x: "<<clust[i].x()<<", y: "<<clust[i].y()<<". ";
  }
  //fill in extra zeroes
  for(i; i<maxclust; i++)
    cout<<0<<","<<0<<","<<0<<",";
  cout<<endl;
}


#endif
