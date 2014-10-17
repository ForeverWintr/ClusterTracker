
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
