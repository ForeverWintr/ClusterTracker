//find corners of arena, use unsharp mask?
//input: filename of image
//output: coords in form (tl tr bl br) x y x y x y x y

//notes:

//http://svr-www.eng.cam.ac.uk/~er258/work/fast.html //try this algorythm?
//ideal threshold after sobel seems to be somewhere around 25

#include <vector>
#include <iostream>
#include <cmath>
#include <iomanip>
#include "jpeg.h"
#include <cstring>

using namespace std;

#define VVV(t) vector<vector<vector<t> > >

struct point{
  int x, y;
};
struct line{
  line(point A, point B){
    a=A;
    b=B;
    slope=(a.y - b.y)/(a.x - b.x); //m
    length=sqrt(pow(static_cast<double>(a.y-b.y),2)+pow(static_cast<double>(a.x-b.x),2));
    midpoint.x=(a.x+b.x)/2;
    midpoint.y=(a.y+b.y)/2;
    yintercept=(slope*a.x-a.y)*-1;} //y=mx+b or -b=mx-y or -yintercept=slope*x-y 
  point a, b, midpoint;
  double slope, length, yintercept;
};
struct corners{
  corners(int tlh, int tlw, int trh, int trw, int blh, int blw, int brh, int brw){
    TL.x=tlh; TL.y=tlw; TR.x=trh; TR.y=trw; BR.x=brh; BR.y=brw; BL.x=blh; BL.y=blw; }
  point TL, TR, BR, BL; //values for 4 corners
};

void toArray(Picture &p, vector<vector<vector<double> > > &rgb);
void toPicture(Picture &p, vector<vector<vector<double> > > &rgb);
void edgy(VVV(double) &rgb, const int &colour);
void applyMask(VVV(double) &rgb, const int &colour, vector<vector<double> > mask);
void threshold(VVV(double) &rgb, const int &colour, const int&thresh);
void traverse(VVV(double) &rgb, const int &colour);
bool onEdge(VVV(double) &array, int h, int w, int mask);
int moveLeft(VVV(double) &rgb, const int &colour, int h, int w);
void dilate(VVV(double) &rgb, const int &colour);

int main(int argc, char **argv) {
  Picture p;
  VVV(double) rgb; //VVV #defined above
  vector<vector<double > > mask;
  
  //open image
  if (!p.open(argv[1])) {
    cerr<<"ERROR!  Unable to open file "<<argv[1]<<endl;
    return 2;
  }
  //convert to array
  toArray(p, rgb);

  cout<<"Smoothing..."<<endl;
  //initialize mask
  int msize=3;
  mask.resize(msize);
  for(int h=0; h<msize; h++){
    mask[h].resize(msize);
    for(int w=0; w<msize; w++){
      mask[h][w]=1.0/9.0;
    }
  }
  applyMask(rgb, 0, mask);    
  applyMask(rgb, 0, mask);
  applyMask(rgb, 0, mask);
  applyMask(rgb, 0, mask); 
  applyMask(rgb, 0, mask);  
  
  cout<<"Detecting Edges..."<<endl;
  //edge detection
  edgy(rgb, 0);
  //edgy(rgb, 1);
  //edgy(rgb, 2);

  //threshold
  threshold(rgb, 0, 128);

  //do some erosion and dilation stuff first
  dilate(rgb, 0);

  //traverse(rgb, 0);
  
  //tests
  /* for(int h=0; h<rgb.size(); h++)
    for(int w=0; w<rgb[h].size(); w++)
      if(rgb[h][w][0] != 0 && rgb[h][w][0] != 255)
	cout<<"rgb["<<h<<"]["<<w<<"] = "<<rgb[h][w][0]<<endl;
  */

  //to picture and print
  toPicture(p, rgb);
  p.save("out.jpg", 100);
}
	
//perform dilation on image
void dilate(VVV(double) &rgb, const int& colour){
  VVV(double) temp=rgb;
  bool se[5][5];

  for(int i=0; i<5; i++)
    for(int j=0; j<5; j++)
      se[i][j]=true;

  //for(int h=0; h<rgb.size(); h++)
  //for(int w=0; w<rgb[h].size(); w++){
  
}

//attempt to find the lines around arena
void traverse(VVV(double) &rgb, const int &colour){
  //look for line
  for(int h=0; h<rgb.size(); h++)
    for(int w=0; w<rgb[h].size(); w++){
      if(rgb[h][w][colour] == 255){ //is this a line? try to move left	int th=h, tw=w;
	//cout<<"trying to move left from ("<<h<<", "<<w<<")"<<endl;
	int dist=moveLeft(rgb, colour, h, w);
      }
    }
}

//recursively move left. keep a count of distance moved
//if it stopped not at an edge and dist is very large, it's probably a corner
int moveLeft(VVV(double) &rgb, const int &colour, int h, int w){
  int dist=1;
  //set red level down so that we don't look here again
  rgb[h][w][colour]-=100;

  //check out of bounds
  if(onEdge(rgb, h, w, 3))
    cout<<"on edge ("<<h<<", "<<w<<")"<<endl;
  else{
    //look to the left
    if(rgb[h-1][w+1][colour] == 255){
      dist+=moveLeft(rgb, colour, h-1, w+1);
    }
    else if(rgb[h][w+1][colour] == 255){
      dist+=moveLeft(rgb, colour, h, w+1);
    }   
    else if(rgb[h+1][w+1][colour] == 255){
      dist+=moveLeft(rgb, colour, h+1, w+1);
    }
  }
  if(dist >= 10)
    rgb[h][w][1]=255;
  return dist;
}

//return true if a 3*3 area around (h, w) is out of bounds. (i.e., we are on edge of image)
 bool onEdge(VVV(double) &array, int h, int w, int mask){
  int hsize=array.size(), wsize=array[0].size();
  if(h == int(mask/2) || h >= hsize-(int(mask/2)+1))
    return true;
  if(w == int(mask/2) || w >= wsize-(int(mask/2)+1))
    return true;
  return false;
}


//threshold on a certain colour. for now threshold is set at half
void threshold(VVV(double) &rgb, const int &colour, const int&thresh){
  for(int h=0; h<rgb.size(); h++)
    for(int w=0; w<rgb[h].size(); w++){
      if(rgb[h][w][colour] < thresh)
	rgb[h][w][colour]=0;
      else
	rgb[h][w][colour]=255;
    }
}

//apply a mask to specified colour of image
void applyMask(VVV(double) &rgb, const int &colour, vector<vector<double> > mask){
  int offset=int(mask.size()/2);
  VVV(double) temp=rgb;
  
  //read through image
  for(int h=offset; h<(rgb.size()-(2*offset)); h++)
    for(int w=offset; w<(rgb[h].size()-(2*offset)); w++){
      temp[h][w][colour]=0;
      temp[h][w][colour]+=(rgb[h-1][w-1][colour] * mask[0][0]);
      temp[h][w][colour]+=(rgb[h-1][w][colour] * mask[0][1]);
      temp[h][w][colour]+=(rgb[h-1][w+1][colour]* mask[0][2]);
      
      temp[h][w][colour]+=(rgb[h][w-1][colour] * mask[1][0]);
      temp[h][w][colour]+=(rgb[h][w][colour] * mask[1][1]);
      temp[h][w][colour]+=(rgb[h][w+1][colour] * mask[1][2]);
      
      temp[h][w][colour]+=(rgb[h+1][w-1][colour] * mask[2][0]);
      temp[h][w][colour]+=(rgb[h+1][w][colour] * mask[2][1]);
      temp[h][w][colour]+=(rgb[h+1][w+1][colour] * mask[2][2]);
    }
  rgb=temp;
}

//apply an edge detection mask to array
//attempting to use sobel edge detection
//colour is the colour to use
//smooth first?
void edgy(VVV(double) &rgb, const int &colour){
  int x[3][3], y[3][3];
  VVV(double) temp=rgb;

  //define Gx
  x[0][0]= -1; x[0][1]= 0; x[0][2]= 1;
  x[1][0]= -2; x[1][1]= 0; x[1][2]= 2;
  x[2][0]= -1; x[2][1]= 0; x[2][2]= 1;

  //define Gy
  y[0][0]=  1; y[0][1]=  2; y[0][2]=  1;
  y[1][0]=  0; y[1][1]=  0; y[1][2]=  0;
  y[2][0]= -1; y[2][1]= -2; y[2][2]= -1;


  //for each pixel in array, excluding edges:
  int h=1, w=1;
  for(h=1; h<rgb.size()-2; h++){
    for(w=1; w<rgb[h].size()-2; w++){
      double Gx=0, Gy=0;      
      //process mask centered on (h, w)
      //Gx
      Gx+= rgb[h-1][w-1][colour] * x[0][0];
      Gx+= rgb[h-1][w][colour]   * x[0][1];
      Gx+= rgb[h-1][w+1][colour] * x[0][2];

      Gx+= rgb[h][w-1][colour] * x[1][0];
      Gx+= rgb[h][w][colour]   * x[1][1];
      Gx+= rgb[h][w+1][colour] * x[1][2];
    
      Gx+= rgb[h+1][w-1][colour] * x[2][0];
      Gx+= rgb[h+1][w][colour]   * x[2][1];
      Gx+= rgb[h+1][w+1][colour] * x[2][2];

      //Gy
      Gy+= rgb[h-1][w-1][colour] * y[0][0];
      Gy+= rgb[h-1][w][colour]   * y[0][1];
      Gy+= rgb[h-1][w+1][colour] * y[0][2];
      
      Gy+= rgb[h][w-1][colour] * y[1][0];
      Gy+= rgb[h][w][colour]   * y[1][1];
      Gy+= rgb[h][w+1][colour] * y[1][2];
    
      Gy+= rgb[h+1][w-1][colour] * y[2][0];
      Gy+= rgb[h+1][w][colour]   * y[2][1];
      Gy+= rgb[h+1][w+1][colour] * y[2][2];
      //cout<<"rgb["<<h+1<<"]["<<w+1<<"]["<<colour<<"] = "<<rgb[h+1][w+1][colour]<<" x "<<y[2][3]<<" = "<<Gy<<endl; 
      
      //cannot do this in place!!

      temp[h][w][colour]= sqrt((Gx*Gx)+(Gy*Gy));
      //      cout<<"old red: "<<temp[h][w][colour]<<" - "<<rgb[h][w][colour];
      temp[h][w][colour]= temp[h][w][colour]-rgb[h][w][colour];
      temp[h][w][colour]*=5; //inflate red values, to make thresholding easier
      if(temp[h][w][colour] > 255)
	temp[h][w][colour]=255;
      else if(temp[h][w][colour] < 10)
	temp[h][w][colour]=0;
      //cout<<" = "<<temp[h][w][colour]<<endl<<endl;
      temp[h][w][1]=0;//temp[h][w][colour];
      temp[h][w][2]=0;//temp[h][w][colour];
    }
  }
  //cout<<w<<" * "<<h<<endl;
  rgb=temp;
}
//convert array to picture
void toPicture(Picture &p, vector<vector<vector<double> > > &rgb){
  //rows are height
  int height=rgb.size();
  int width=rgb[1].size();
  p.setsize(width, height);
  for(int h=0; h<p.getheight(); h++){
    for(int w=0; w<p.getwidth(); w++){
      int r=int(0.5 + rgb[h][w][0]), g=int(0.5+rgb[h][w][1]), b=int(0.5+rgb[h][w][2]);
      
      if(r>255)
	r=255;
      else if(r<0)
	r=0;

      if(g>255)
	g=255;
      else if(g<0)
	g=0;

      if(b>255)
	b=255;
      else if(b<0)
	b=0;
      
      p.setpixel(w, h, r, g, b);
    }
  }
}


//convert picture to array
void toArray(Picture &p, vector<vector<vector<double> > > &rgb){
  rgb.clear();
  rgb.resize(p.getheight());
  for(int h=0; h<p.getheight(); h++){
    rgb[h].resize(p.getwidth());
    for(int w=0; w<p.getwidth(); w++){
      int r,g,b;
      rgb[h][w].resize(3);

      p.getpixel(w,h,r,g,b);
      rgb[h][w][0]=r;
      rgb[h][w][1]=g;      
      rgb[h][w][2]=b;
    }
    
  }
}
