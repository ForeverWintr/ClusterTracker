// Parse coordinates spit out by image j. To get these coordinates, in
// imageJ make sure "Auto-Measure" is checked by double clicking the
// point tool, then click on the corners of the arena in a clockwise
// direction starting at the top left.

#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char** argv){
  if(argc != 2){
    cerr<<"Error: incorrect number of arguments supplied to parsecoords"<<endl;
    exit(1);
  }
  ifstream file;
  char in[256];
  char * pfortok;
  int cube=0, area, x, y, s, sprev=1, objects=0, prevcubes=1;
  int junk;

  //open file
  file.open(argv[1]);
  if(!file.good()){
    cerr<<"File read error in parsecoords"<<endl;
    exit(2);
  }

  file.getline(in,256); //skip first line
  //read until eof?
  while(! file.eof()){
    file.getline(in,256); //in contains a line

    //tokenize in based on tabs
    pfortok = strtok (in,"\t");
    if(pfortok != NULL){
      junk=atoi(pfortok);
      junk = atoi(strtok (NULL, "\t"));
      x = int(0.5 + atoi(strtok (NULL, "\t"))); //should be a to double
      y = int(0.5 + atoi(strtok (NULL, "\t")));
 
      junk = atoi(strtok (NULL, "\t"));
      junk = atoi(strtok (NULL, "\t"));
      junk= atoi(strtok (NULL, "\t"));
      
      pfortok = strtok (NULL, "\t");
      if(pfortok != NULL)//pfortok should now be null
	cout<<"ERROR: PFORTOK IS NOT NULL"<<endl;
      
      //save coordinates as y x
      cout<<y<<" "<<x<<endl;
      
    } 
  }

  return 0;
}
