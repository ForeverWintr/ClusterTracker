
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

#include "jpeg.h"

#include <iostream>
#include <stdlib.h>
using namespace std;

int main(int argc, char **argv) {

  Picture pic;

  if (argc<3) {
    cerr<<" Usage:  scale filename sizepercent "<<endl;
    cerr<<" \"scale cat.jpg 200\" would result in a cat twice as big as the original"<<endl;
    cerr<<" \"scale cat.jpg 50\" would result in a cat half the size of the original"<<endl;
    cerr<<" Output is saved to file \"scaled.jpg\""<<endl;
    return 1;    
  }
  
  Picture pic2;
  pic.open(argv[1]);
  int scale=atoi(argv[2]);
  
  pic2.setsize(pic.getwidth()*scale/100, pic.getheight()*scale/100);
  for (int x=0;x<pic2.getwidth();x++) 
    for (int y=0;y<pic2.getheight();y++) {
      int xa,ya;
      xa=x*100/scale; ya=y*100/scale;
      if (xa>pic.getwidth()) xa=pic.getwidth();
      if (ya>pic.getheight()) ya=pic.getheight();
      pic2.setpixel(x,y,pic.getpixel(xa,ya));
    }
  pic2.save("scaled.jpg",80,1,1);
  return 0;
}
