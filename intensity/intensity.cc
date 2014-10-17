//equalize intensity 
//ATTENTION: By convention, this program assumes X is
//the vertical axis, with (0, 0) in the top left corner

#include <vector>
#include <iostream>
#include <cmath>
#include <iomanip>
//#include <cimage>
#include "jpeg.h"
 
//the following defines are from forum user David M. at  http://tinyurl.com/29yy5hq
#define PI 3.1415926
#define DEGTORAD(Deg) ((Deg * PI) / 180.0)
#define RADTODEG(Rad) ((180.0 * Rad) / PI)


using namespace std;

void toArray(Picture &p, vector<vector<vector<double> > > &array);
void toPicture(Picture &p, vector<vector<vector<double> > > &array);
void toHSI(vector<vector<vector<double> > > &rgb);
void toRGB(vector<vector<vector<double> > > &rgb);

int main(int argc, char **argv)
{
  vector<vector<vector<double> > > empty;
  
  Picture base;
  
  if (!base.open(argv[1])) {
    cerr<<"ERROR!  Unable to open file "<<argv[1]<<endl;
    return 2;
  }

  toArray(base, empty);
  toHSI(empty);

  //test area

  //find avg intensity of empty
  double avg=0;
  int t=0, w=0;
  for(t=0; t<empty.size(); t++)
    for(w=0; w<empty[t].size(); w++){
      //cout<<"empty at ("<<t<<", "<<w<<"): "<<empty[t][w][2] <<". arena: "<<arena[t][w][2]<<endl;
      avg+= empty[t][w][2];
      //arena[t][w][2] = 97.7437;
    }
  avg= avg/(t*w);
  cout<<"avg: "<<avg<<" t*w: "<<t*w<<endl;

  //then, for each pixel in empty, what must it be multiplied by to get avg?
  //trying instead what must be added to it
  cout<<empty.size()<<" "<<empty[0].size()<<" ";
  for(t=0; t<empty.size(); t++)
    for(w=0; w<empty[t].size(); w++){
      double m= avg-empty[t][w][2];
      //cout<<setprecision(5)<<m<<" ";
      //cout<<avg<<" / "<<empty[t][w][2]<<" = "<<m<<" so, ";
      //cout<<arena[t][w][2]<<" * "<<m<<" = ";
      empty[t][w][2] += m;
      //cout<<arena[t][w][2]<<endl;
    }
  
  toRGB(empty);
  toPicture(base, empty);
  base.save("out.jpg", 100);
  return 0;
}

//create an rgbarray from an hsi one
void toRGB(vector<vector<vector<double> > > &array){
 
  for(int t=0; t<array.size(); t++)
    for(int w=0; w<array[t].size(); w++){
      double r, g, b;
      double h=array[t][w][0], s=array[t][w][1], i=array[t][w][2];
      
      //      cout << "h = " << h << endl;
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
      
      //      cout << r << ", " << g << ", " << b << endl;
    }
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

      //intensity
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


//creates a vector of vectors of vectors containing the jpeg image. rows are height
void toArray(Picture &p, vector<vector<vector<double> > > &array){
   for(int h=0; h<p.getheight(); h++){
     vector<vector<double> > row;
    for(int w=0; w<p.getwidth(); w++){
      int r,g,b;
      vector<double> pixel;

      p.getpixel(w,h,r,g,b);
      pixel.push_back(r);
      pixel.push_back(g);
      pixel.push_back(b);
      row.push_back(pixel);
    }
    array.push_back(row);
   }
}
