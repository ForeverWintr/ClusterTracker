//register a jpeg image of my robotics arena, given the coordinates of
//the four corners. 
//ATTENTION: By convention, this program assumes X is
//the vertical axis, with (0, 0) in the top left corner

#include <vector>
#include <iostream>
#include <cmath>
#include "jpeg.h"

using namespace std;

struct point{
  int x, y;
};
struct line{
  point a, b, midpoint;
  double slope, length, yintercept;
};
struct corners {
  point TL, TR, BR, BL; //values for 4 corners
};

corners findConstants(corners orig, int &neww, int &newh, 
		      double c[8]);
corners getinput();
line createline(point, point);//return a line
void toArray(Picture &p, vector<vector<vector<unsigned int> > > &array);
void regstr(vector<vector<vector<unsigned int> > > &orig,
		 vector<vector<vector<unsigned int> > > &reg, 
		 int neww, int newh, double c[8]);
void lookaround(int m[4], int c, vector<vector<vector<unsigned int> > > &orig, 
		double x, double y);
void toPicture(Picture &p, vector<vector<vector<unsigned int> > > &array);
double interpolate(double x, double y, int m[]);

//the following function is adapted from an example given by "iamthwee"
//at <http://www.daniweb.com/forums/thread41803.html>
void gauss(float A [4][5],    // coefficients and constants   //
	   double result[4],                                 ///
	   bool& err);                                      ////
////////////////////////////////////////////////////////////////


int main(int argc, char **argv) {
  
  vector<vector<vector<unsigned int> > > array, reg;
  Picture p, newp;
  corners originalCs, newCs;
  int neww, newh;
  double c[8];
  
  if (!p.open(argv[1])) {
    cerr<<"ERROR!  Unable to open file "<<argv[1]<<endl;
    return 2;
  }

  
  //Test values for here: 155 2 774 10 0 296 959 299
  //new test values: 196 155 206 775 470 3 475 954 
  originalCs=getinput();
  
  newCs=findConstants(originalCs, neww, newh, c);

  toArray(p, array);
  cout<<"got here"<<endl;
  
  regstr(array, reg, neww, newh, c);

  toPicture(newp, reg);

  newp.save(argv[2]);

  return 0;
}

//create new image, and given points in that image, find where they came from 
void regstr(vector<vector<vector<unsigned int> > > &orig,
		 vector<vector<vector<unsigned int> > > &reg, 
		 int neww, int newh, double c[8]){
  int r[4], g[4], b[4];
  // reg.resize(newh);

  //create new image, rows are height
  for(int h=0; h<newh; h++){
    vector<vector<unsigned int> > row;
    //reg[h].resize(neww);
    for(int w=0; w<neww; w++){
      //int r=0,g=0,b=0;
      vector<unsigned int> pixel;
      //reg[h][w].resize(3);
      double x=c[0]*h + c[1]*w + c[2]*h*w + c[3];
      double y=c[4]*h + c[5]*w + c[6]*h*w + c[7];
      
      lookaround(r, 0, orig, x, y);
      lookaround(g, 1, orig, x, y);
      lookaround(b, 2, orig, x, y);

      //reg[w][h][0]=int(interpolate(x, y, r)+0.5);
      //reg[w][h][1]=int(interpolate(x, y, g)+0.5);
      //reg[w][h][2]=int(interpolate(x, y, b)+0.5);

      pixel.push_back(int(interpolate(x, y, r)+0.5));
      pixel.push_back(int(interpolate(x, y, g)+0.5));
      pixel.push_back(int(interpolate(x, y, b)+0.5));
      
      row.push_back(pixel);
    }
    reg.push_back(row);
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
void lookaround(int m[4], int c, vector<vector<vector<unsigned int> > > &orig, 
		double u, double v){
  int x=int(floor(u)), y=int(floor(v));
  //find constants for interpolation
 
  // cout<<"looking around ("<<u<<", "<<v<<")"<<endl;
  //cout<<"orig size is height = "<<orig.size()<<", width = "<<orig[0].size()<<endl;
  if(x < 0 && y <0)
    m[0]=0;
  else
    m[0]=orig[x][y][c]; 

  if(y+1 >= orig[0].size())
    m[2]=0;
  else{
    //cout<<"y+1="<<y+1<<endl;
    m[2]=orig[x][y+1][c];//seg fault is here when y=539 and x=202

  }
  if(x+1 >= orig.size())
     m[1]=0;
  else
    m[1]=orig[x+1][y][c];
  if(x+1 >= orig.size() || y+1 >= orig[0].size())
    m[3]=0;
  else
    m[3]=orig[x+1][y+1][c];

}
  
//return new corners, constants, and new width and height
corners findConstants(corners orig, int &neww, int &newh, 
		      double c[8]){
  bool err;
  corners reg;
  float x[4][5], y[4][5];
  double c14[4], c58[4];
  line bottom=createline(orig.BR, orig.BL);
  
  neww=int(ceil(bottom.length)), newh=int(ceil(bottom.length*10/16)); //this line needs to be changed depending on whether x is height or width	\
  
  reg.TL.x=0;
  reg.TL.y=0;
  
  reg.TR.x=0;
  reg.TR.y=neww-1;

  reg.BL.x=newh-1;
  reg.BL.y=0;

  reg.BR.x=newh-1;
  reg.BR.y=neww-1; // -1 because the image starts at (0, 0)

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

//read input from std in
corners getinput(){
  corners c;
  cout<<endl<<"Please enter x and y values for the 4 corners: "<<endl;
  cout<<"Top Left x: ";
  cin>>c.TL.x;
  cout<<"Top Left y: ";
  cin>>c.TL.y;
  cout<<"Top Right x: ";
  cin>>c.TR.x;
  cout<<"Top Right y: ";
  cin>>c.TR.y;
  cout<<"Bottom Left x: ";
  cin>>c.BL.x;
  cout<<"Bottom Left y: ";
  cin>>c.BL.y;
  cout<<"Bottom Right x: ";
  cin>>c.BR.x;
  cout<<"Bottom Right y: ";
  cin>>c.BR.y;
  cout<<endl;
  
  return c;
}

//turn a 3D array into a picture
void toPicture(Picture &p, vector<vector<vector<unsigned int> > > &array){
  //rows are height
  int height=array.size();
  int width=array[1].size();

  p.setsize(width, height);
  for(int h=0; h<p.getheight(); h++){
    for(int w=0; w<p.getwidth(); w++){
      int r=array[h][w][0], g=array[h][w][1], b=array[h][w][2];
      p.setpixel(w, h, r, g, b);
    }
  }

}


//creates a vector of vectors of vectors containing the jpeg image. rows are height
void toArray(Picture &p, vector<vector<vector<unsigned int> > > &array){
  // array.resize(p.getheight());
  for(int h=0; h<p.getheight(); h++){
    //array[h].resize(p.getwidth());
    vector<vector<unsigned int> > row;
    for(int w=0; w<p.getwidth(); w++){
      int r,g,b;
      //  array[h][w].resize(3);
      vector<unsigned int> pixel;

      p.getpixel(w,h,r,g,b);
      //array[w][h][0]=r;
      //array[w][h][1]=g;
      //array[w][h][2]=b;

      pixel.push_back(r);
      pixel.push_back(g);
      pixel.push_back(b);
      row.push_back(pixel);
    }
    array.push_back(row);
   }
}

//Returns a line between 2 points
// slope: y2 - y1/ x2 - x1
line createline(point a, point b){
  line temp;
  temp.a=a;
  temp.b=b;
  temp.slope=(a.y - b.y)/(a.x - b.x); //m
  temp.length=sqrt(pow(static_cast<double>(a.y-b.y),2)+pow(static_cast<double>(a.x-b.x),2));
  temp.midpoint.x=(a.x+b.x)/2;
  temp.midpoint.y=(a.y+b.y)/2;
  temp.yintercept=(temp.slope*a.x-a.y)*-1; //y=mx+b or -b=mx-y or -yintercept=slope*x-y

  return temp;
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


