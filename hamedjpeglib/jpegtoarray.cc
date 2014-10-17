
/*             _      | |        |  _      | |      |  | |
 *            / \     | |          / \     | |  /|  |  | |             \
 *      Q___|____\  __| | | . |__Q____\  __| | (_|__|__| |  Q_|__|__|___)
 *  ___/    :      /      |___|         /               ___/          .
 *               _/                   _/
 *
 */

//  Written by Hamed Ahmadi Nejad
//    ahmadinejad@ce.sharif.edu
//    comphamed@yahoo.com

#include <iostream>
#include "jpeg.h"

using namespace std;

int main(int argc, char **argv) {
  if (argc<3) {
    cerr<<"  Usage: jpegtoarray FILE VARNAME"<<endl;
    cerr<<"  For example, \"jpegtoarray bird.jpg bird\" would create"<<endl;
    cerr<<"  an array called \"bird\" containing the RGB values of "<<endl;
    cerr<<"  the image in bird.jpg"<<endl;
    cerr<<"  Output is written to stdout"<<endl;
    return 1;
  }

  Picture p;
  if (!p.open(argv[1])) {
    cerr<<"ERROR!  Unable to open file "<<argv[1]<<endl;
    return 2;
  }
  
  int i,j;

  cout<<"unsigned char "<<argv[2]<<"["<<p.getheight()
      <<"]["<<p.getwidth()<<"][3] = {"<<endl;
  for (i=0;i<p.getheight();i++) {
    cout<<"   { ";
    for (j=0;j<p.getwidth();j++) {
      int r,g,b;
      p.getpixel(j,i,r,g,b);
      cout<<"{"<<r<<","<<g<<","<<b<<"}";
      if (j+1<p.getwidth()) cout<<", ";
    }
    cout<<" }";
    if (i+1<p.getheight()) cout<<',';
    cout<<endl;
  }
  cout<<"  };"<<endl;
  
  return 0;
}
