//register image and equalize intensity 
//ATTENTION: By convention, this program assumes X is
//the vertical axis, with (0, 0) in the top left corner

#include <vector>
#include <iostream>
#include <cmath>
#include <iomanip>
#include "jpeg.h"
#include <cstring>
#include <cassert>
#include <fstream>
 
//define some shortcuts for multidimensional vector creation
#define VVV(t) vector<vector<vector<t> > >
#define VV(t) vector<vector<t> >

//the following defines are from forum user David M. at  http://tinyurl.com/29yy5hq
#define PI 3.1415926
#define DEGTORAD(Deg) ((Deg * PI) / 180.0)
#define RADTODEG(Rad) ((180.0 * Rad) / PI)

using namespace std;

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

corners findConstants(corners orig, int &neww, int &newh, 
		      double c[8]);
line createline(point, point);//return a line
void regstr(vector<vector<vector<double> > > &orig, 
	    vector<vector<vector<double> > > &reg, 
	    corners origCs);
	    //int neww, int newh, double c[8]);
void lookaround(int m[4], int c, vector<vector<vector<double> > > &orig, 
		double x, double y);
double interpolate(double x, double y, int m[]);
void toArray(Picture &p, vector<vector<vector<double> > > &array);
void toPicture(Picture &p, vector<vector<vector<double> > > &array);
void toHSI(vector<vector<vector<double> > > &rgb);
void toRGB(vector<vector<vector<double> > > &rgb);
void usage();
double calcM(vector<vector<vector<double> > > &hsi, vector<vector<double > > &M);
void mult(vector<vector<vector<double> > > &hsi, vector<vector<double > > &M);
int cubeCount(VVV(double) &thresh);
void dim(VVV(double) &thresh, int h, int w);
bool threshold(VVV(double) &hsi, const int &cubes, int div);

//the following function is adapted from an example given by "iamthwee"
//at <http://www.daniweb.com/forums/thread41803.html>
void gauss(float A [4][5], double result[4], bool& err);


//take as input a list of coordinates
//read images to register from std input
//register background
//create equalization array 
//if image is not background, apply registration array. hmm.
int main(int argc, char **argv) {
  Picture p, out;
  vector<vector<vector<double > > > img, regd;
  vector<vector<double > > bg;
  string filename= " ", newfile="_processed", dimensions="dimensions.txt";
  double avgI;

  if(argc !=9){
    cerr<<argv[0]<<": Error, 8 coordinates required, "<<argc-1<<" supplied"<<endl;
    usage();
    return 1;
  } 
  corners origCs(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), 
	    atoi(argv[5]), atoi(argv[6]), atoi(argv[7]), atoi(argv[8]));

  //handle background
  cout<<argv[0]<<" is reading images."<<endl;
  cin>>filename;
  //filename="../testimg.jpg";
  if (!p.open(const_cast<char *>(filename.c_str()))) {
    cerr<<"ERROR!  Unable to open file "<<filename<<endl;
    return 2;
  }
  
  cout<<"registering "<<filename<<endl;
  toArray(p, img);

  regstr(img, regd, origCs);
  
  dimensions.insert(filename.length()-4, newfile);
  cout<<"New dimensions are H: "<<regd.size()<<", W: "<<regd[0].size()<<
    ". Saved to "<<dimensions<<endl;
  
  ofstream dimf;
  dimf.open(dimensions.c_str());
  dimf<<regd.size()<<" "<<regd[0].size()<<endl;
  dimf.close();

  //uncoment the following lines to save the registered background image
  //  toPicture(p, regd);
  //p.save("regbg.jpg", 100);
 
  toHSI(regd);
  avgI=calcM(regd, bg); //bg is now a 2d array of things to add to intensity 
  cout<<"  Background array calculated. Average intensity is "<<avgI<<endl;

  cin>>filename;
  while(filename != "done"){
    Picture p, outp;
    if (!p.open(const_cast<char *>(filename.c_str()))) {
      cerr<<"ERROR!  Unable to open file "<<filename<<endl;
      return 2;
    }
    cout<<"registering "<<filename<<endl;
    toArray(p, img);
    
    //each of the below commands performs a different operation on the
    //image.  to see the saved images of intermediate steps in
    //processing, comment out some of the commands
    
    //register
    regstr(img, regd, origCs);

    //convert to HSI
    toHSI(regd);

    //do intensity equalization
    mult(regd, bg);

    //threshold
    //  cout<<"Thresholding... ";
    //threshold(regd, 6, 128);
    //cout<<endl<<"Cubes found: "<<cubeCount(regd)<<endl;
    
    //convert back to rgb
    toRGB(regd);
    
    //return to picture format
    toPicture(outp, regd);
    
    filename.insert(filename.length()-4, newfile);
    cout<<"saving "<<filename<<endl;
    outp.save(const_cast<char *>(filename.c_str()));

    cin>>filename;
  }  
}

//pick a threshold for the image based on the number of objects
bool threshold(VVV(double) &hsi, const int &cubes, int div){
  //threshold the image
  //VVV(double) temp=hsi;
  for(int t=0; t<hsi.size(); t++)
    for(int w=0; w<hsi[t].size(); w++){
      if(hsi[t][w][2] > div)
	hsi[t][w][2]=255;
      else
	hsi[t][w][2]=0;
    }
  /*int cc=cubeCount(temp);
  temp.clear();

  //this doesn't work because the actual image is never altered
  //if number of cubes is not right, try again with a different threshold
  if(cc < cubes){ 
    threshold(hsi, cubes, div+1); cout<<"+";
  }
  else if(cc > cubes){ 
    threshold(hsi, cubes, div-1); cout<<"-";
  }
  if(cubes == cc)
  return true;*/
}
  
//return the number of cubes in the image
int cubeCount(VVV(double) &thresh){
  int cubes=0;
  for(int h=1; h<thresh.size()-2; h++)
    for(int w=1; w<thresh[h].size()-2; w++){
      if(thresh[h][w][2]==255){
	cubes++;
	dim(thresh, h, w);
      }
    }
  return cubes;
}

//recursively "dim" (mark as visited) connected pixels
void dim(VVV(double) &thresh, int h, int w){
  //if we aren't looking at a white pixel we shouldn't be here
  assert(thresh[h][w][2] == 255);
  //dim current pixel
  thresh[h][w][2]=100;
  //look all around
  for(int nh=h-1; nh<=h+1; nh++)
    for(int nw=w-1; nw<=w+1; nw++)
      if(thresh[nh][nw][2]==255)
	dim(thresh, nh, nw);
}

//add m to hsi array intensities
void mult(vector<vector<vector<double> > > &hsi, vector<vector<double > > &M){
  if(hsi.size() != M.size() || hsi[0].size() != M[0].size())
    cerr<<"MULTIPLICATION ERROR: M and HSI arrays have different dimensions."<<endl;
  else{
    for(int t=0; t<hsi.size(); t++)
      for(int w=0; w<hsi[t].size(); w++)
	hsi[t][w][2]+=M[t][w];
    //	hsi[t][w][2]*=M[t][w];
  }
}

//create an rgbarray from an hsi one
void toRGB(vector<vector<vector<double> > > &array){
   for(int t=0; t<array.size(); t++)
    for(int w=0; w<array[t].size(); w++){
      double r, g, b;
      double h=array[t][w][0], s=array[t][w][1], i=array[t][w][2];
      
      if(h < 120 && h >= 0){
	b= i*(1-s);
	r= i*(1+ ( (s*cos(DEGTORAD(h))) / cos(DEGTORAD(60) - DEGTORAD(h)) ));
	g= 3*i-(r+b);
      }

      if(120 <= h && h < 240){
	r= i*(1-s);
	g= i*(1+ ( (s*cos(DEGTORAD(h)-DEGTORAD(120))) / cos(DEGTORAD(180) - DEGTORAD(h)) ));
	b= 3*i-(r+g);
      }

      if(240 <= h && h <= 360){
	g= i*(1-s);
	b= i*(1+ ( (s*cos(DEGTORAD(h)-DEGTORAD(240))) / cos(DEGTORAD(300) - DEGTORAD(h)) ));
	r= 3*i-(g+b);
      }
      
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
      
      array[t][w][0]=r;
      array[t][w][1]=g;
      array[t][w][2]=b;
     }
}

//create new image, and given points in that image, find where they came from 
void regstr(vector<vector<vector<double> > > &orig,
	    vector<vector<vector<double> > > &reg, 
	    corners origCs){
//int neww, int newh, double c[8]){
  int neww, newh;
  double c[8];
  corners newCs = findConstants(origCs, neww, newh, c);


  int r[4], g[4], b[4];
  reg.empty();
  reg.resize(newh);

  //create new image, rows are height
  for(int h=0; h<newh; h++){
    reg[h].resize(neww);
    for(int w=0; w<neww; w++){
      //int r=0,g=0,b=0;
     
      reg[h][w].resize(3);
      double x=c[0]*h + c[1]*w + c[2]*h*w + c[3];
      double y=c[4]*h + c[5]*w + c[6]*h*w + c[7];

      lookaround(r, 0, orig, x, y);
      lookaround(g, 1, orig, x, y);
      lookaround(b, 2, orig, x, y);

      reg[h][w][0]=int(interpolate(x, y, r)+0.5);
      reg[h][w][1]=int(interpolate(x, y, g)+0.5);
      reg[h][w][2]=int(interpolate(x, y, b)+0.5);
    }
  }
}

double interpolate(double x, double y, int m[]){
  //round down to find u, v
  int u=int(floor(x)), v=int(floor(y));
  int A, B, C, D;

  //equations generated by maple
  A= -v*m[3]-m[0]+v*m[1]+m[2]*v-m[0]*v+m[1];
  B= -m[3]*u+m[1]*u+u*m[2]-m[0]*u+m[2]-m[0];
  C= m[3]-m[1]-m[2]+m[0];
  D= v*m[3]*u+m[0]*u-v*m[1]*u-u*m[2]*v+v*m[0]*u-m[2]*v+m[0]*v-m[1]*u+m[0];
  
  return A*x + B*y + C*x*y +D;
}

//return the four nearest neighbors 
//Seg fault occurs here when v is passed in as 960
//I don't think v should ever be 960, but it is set in an advanced math thing
//that I don't understand. 
void lookaround(int m[4], int c, vector<vector<vector<double> > > &orig, 
		double u, double v){
  int x=int(floor(u)), y=int(floor(v));

  //the following is a patch to ensure that y doesn't go out of bounds.
  if(y >= orig[0].size())
    y--;
//y=orig[0].size()-1;
  if(x >= orig.size())
    x--;

  //find constants for interpolation
  
  //  if(y >= 960)
    //cout<<"y is "<<y<<"! here comes a seg fault!"<<endl;
  //cout<<"looking around ("<<u<<", "<<v<<")"<<endl;
  // cout<<"orig["<<x<<"]["<<y<<"]["<<c<<"]  = "<<endl;
  //cout<<orig[x][y][c]<<endl; 
  //cout<<"orig size is height = "<<orig.size()<<", width = "<<orig[0].size()<<endl;
  if(x < 0 && y <0)
    m[0]=0;
  else
    m[0]=int(orig[x][y][c]+0.5); 

  if(y+1 >= orig[0].size())
    m[2]=0;
  else{
    m[2]=int(orig[x][y+1][c]+0.5);
  }
  if(x+1 >= orig.size())
     m[1]=0;
  else
    m[1]=int(orig[x+1][y][c]+0.5);
  if(x+1 >= orig.size() || y+1 >= orig[0].size())
    m[3]=0;
  else{
    //cout<<"orig["<<x+1<<"]["<<y+1<<"]["<<c<<"] + 0.5 = "<<endl;
    m[3]=int(orig[x+1][y+1][c]+0.5);// sometimes this segfaults, sometimes it doesn't
    //cout<<"noseg"<<endl;
  }

}

//return new corners, constants, and new width and height
corners findConstants(corners orig, int &neww, int &newh, 
		      double c[8]){
  bool err;
  //  corners reg;
  float x[4][5], y[4][5];
  double c14[4], c58[4];
  line bottom(orig.BR, orig.BL);
  
  neww=int(ceil(bottom.length)), newh=int(ceil(bottom.length*10/16)); //this line needs to be changed depending on whether x is height or width	\
  
  corners reg(0,0, 0,neww-1, newh-1,0, newh-1,neww-1); 
  /*reg.TL.x=0;
  reg.TL.y=0;
  
  reg.TR.x=0;
  reg.TR.y=neww-1;

  reg.BL.x=newh-1;
  reg.BL.y=0;

  reg.BR.x=newh-1;
  reg.BR.y=neww-1; // -1 because the image starts at (0, 0)
  */

  //set up system of equations
  //TL
  x[0][0] = reg.TL.x;
  x[0][1] = reg.TL.y;
  x[0][2] = reg.TL.x*reg.TL.y;
  x[0][3] = 1;
  x[0][4] = orig.TL.x;

  //TR
  x[1][0] = reg.TR.x;
  x[1][1] = reg.TR.y;
  x[1][2] = reg.TR.x*reg.TR.y;
  x[1][3] = 1;
  x[1][4] = orig.TR.x;

  //BL
  x[2][0] = reg.BL.x;
  x[2][1] = reg.BL.y;
  x[2][2] = reg.BL.x*reg.BL.y;
  x[2][3] = 1;
  x[2][4] = orig.BL.x;

  //BR
  x[3][0] = reg.BR.x;
  x[3][1] = reg.BR.y;
  x[3][2] = reg.BR.x*reg.BR.y;
  x[3][3] = 1;
  x[3][4] = orig.BR.x;

  y[0][0] = reg.TL.x;
  y[0][1] = reg.TL.y;
  y[0][2] = reg.TL.x*reg.TL.y;
  y[0][3] = 1;
  y[0][4] = orig.TL.y;

  //TR
  y[1][0] = reg.TR.x;
  y[1][1] = reg.TR.y;
  y[1][2] = reg.TR.x*reg.TR.y;
  y[1][3] = 1;
  y[1][4] = orig.TR.y;

  //BL
  y[2][0] = reg.BL.x;
  y[2][1] = reg.BL.y;
  y[2][2] = reg.BL.x*reg.BL.y;
  y[2][3] = 1;
  y[2][4] = orig.BL.y;

  //BR
  y[3][0] = reg.BR.x;
  y[3][1] = reg.BR.y;
  y[3][2] = reg.BR.x*reg.BR.y;
  y[3][3] = 1;
  y[3][4] = orig.BR.y;

  gauss(x, c14, err);
  gauss(y, c58, err);

  //test for here: 31 183 44 751 275 48 278 908
  //cout<<"c1: "<<c14[0]<<" c2: "<<c14[1]<<" c3: "<<c14[2]<<" c4: "<<c14[3]<<endl;
  //cout<<"c5: "<<c58[0]<<" c6: "<<c58[1]<<" c7: "<<c58[2]<<" c8: "<<c58[3]<<endl;

  int i=0;
  while(i<4){
    c[i]=c14[i];
    i++;
  }
  while(i<8){
    c[i]=c58[i-4];
    i++;
  }

  return reg;
}

//calculate matrix of values to add to intensity in order to make it equal to the average. 
double calcM(vector<vector<vector<double> > > &hsi, vector<vector<double > >&M){
  M.empty();
  M.resize(hsi.size());
  
  //calculate average intensity
  double I=0;
  int h, w;
  for(h=0; h<hsi.size(); h++)
    for(w=0; w<hsi[h].size(); w++){
      I+=hsi[h][w][2];
    }
  I/=(h*w);
 
  // i*x=I, find x
  for(h=0; h<hsi.size(); h++){
    M[h].resize(hsi[h].size());
    for(w=0; w<hsi[h].size(); w++)
      M[h][w]= I - hsi[h][w][2];
    //M[h][w]= I / hsi[h][w][2];
  }
  return I;
}

//convert an hsi array to an rgb one
void toHSI(vector<vector<vector<double> > > &array){
  double h, s, i;

  for(int t=0; t<array.size(); t++)
    for(int w=0; w<array[t].size(); w++){
      double r=array[t][w][0], g=array[t][w][1], b=array[t][w][2];

      //intensity
      i= (r+g+b)/3;

      //saturation
      if (r + g + b != 0) {
	s= 1- ( (3*min(r, min(g, b))) / (r+g+b) );
      } else {
	s = 0;
      }

      //hue
      if (!(r == g && g == b)) {
	h= acos((0.5*((r-g)+(r-b))) / 
		sqrt(pow((r-g),2.0) + ((r-b) * (g-b))));
      } else {
	h = 0;
      }
      h=RADTODEG(h);
      if(b > g)
	h= 360 - h;

      // cout<<"r: "<<r<<", g: "<<g<<", b: "<<b<<". h: "<<h
      //  <<", s: "<<s<<", i: "<<i<<endl;

      array[t][w][0]=h;
      array[t][w][1]=s;
      array[t][w][2]=i;
      
    }
}

//convert array to picture
void toPicture(Picture &p, vector<vector<vector<double> > > &array){
  //rows are height
  int height=array.size();
  int width=array[1].size();
  p.setsize(width, height);
  for(int h=0; h<p.getheight(); h++){
    for(int w=0; w<p.getwidth(); w++){
      int r=int(0.5 + array[h][w][0]), g=int(0.5+array[h][w][1]), b=int(0.5+array[h][w][2]);
      
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
void toArray(Picture &p, vector<vector<vector<double> > > &array){
  array.clear();
  array.resize(p.getheight());
  for(int h=0; h<p.getheight(); h++){
    array[h].resize(p.getwidth());
    for(int w=0; w<p.getwidth(); w++){
      int r,g,b;
      array[h][w].resize(3);

      p.getpixel(w,h,r,g,b);
      array[h][w][0]=r;
      array[h][w][1]=g;      
      array[h][w][2]=b;
    }
    
  }
}

//print usage instructions
void usage(){
  cout<<"USAGE: regintensity [top left h] [top left w] [top right] [bottom left] [bottom right]"<<endl;
}

//////////////////////////////////////////////////////////////////////
//the following function is adapted from an example given by "iamthwee"
//at <http://www.daniweb.com/forums/thread41803.html>      

void gauss(float A [4] [5], // coefficients and constants
	   double result[4],
	   bool& err)
// Solve system of N linear equations with N unknowns
// using Gaussian elimination with scaled partial pivoting
// First N rows and N+1 columns of A contain the system
// with right-hand sides of equations in column N+1
// err returns true if process fails; false if it is successful
// original contents of A are destroyed
// solution appears in column N
{
   int indx[20];
   float scale[20];
   float maxRatio;
   int maxIndx;
   int tmpIndx;
   float ratio;
   float sum;
   int N=4;
   
   for (int i = 0; i < N; i++) indx[i] = i;	// index array initialization
   
   // determine scale factors
   
   for (int row = 0; row < N; row++)
   {
      scale[row] = abs(A[row][0]);
      for (int col = 1; col < N; col++)
      {
	 if (abs(A[row][col]) > scale[row]) scale[row] = abs(A[row][col]);
      }
   }
   
// forward elimination
   
   for (int k = 0; k < N; k++)
   {
      // determine index of pivot row
      maxRatio = abs(A[indx[k]] [k])/scale[indx[k]];
      maxIndx = k;
      for (int i = k+1; i < N; i++)
      {
	 if (abs(A[indx[i]] [k])/scale[indx[i]] > maxRatio)
	 {
	    maxRatio = abs(A[indx[i]] [k])/scale[indx[i]];
	    maxIndx = i;
	 }
      }
      if (maxRatio == 0) // no pivot available
      {
	 err = true;
	 return;
      }
      tmpIndx =indx[k]; indx[k]=indx[maxIndx]; indx[maxIndx] = tmpIndx;
      
      // use pivot row to eliminate kth variable in "lower" rows
      for (int i = k+1; i < N; i++)
      {
	 ratio = -A[indx[i]] [k]/A[indx[k]] [k];
	 for (int col = k; col <= N; col++)
	 {
	    A[indx[i]] [col] += ratio*A[indx[k]] [col];
	 }
      }
   }
   
   // back substitution
   
   for (int k = N-1; k >= 0; k--)
   {
      sum = 0;
      for (int col = k+1; col < N; col++)
      {
	 sum += A[indx[k]] [col] * A[indx[col]] [N];
      }
      A[indx[k]] [N] = (A[indx[k]] [N] - sum)/A[indx[k]] [k];
   }
   
/*
  cout << endl;
  for (int r=0; r<N; r++)
  {
  cout << indx[r];
  for (int c=0; c<=N; c++) cout<<"  " << A[r][c];
  cout << endl;
  }
*/
   for (int k = 0; k < N; k++) result[k] = A[indx[k]] [N];
}
