#include <iostream>
#include <fstream>
#include <vector>

struct access{
        unsigned long long address;
        std::string instr;
};

struct accessInfo{
	int valid;
	int tag;
};

void directMapped(std::vector<access> &inputList, int blockSize, std::ofstream &fout);
void setAssociative(std::vector<access> &inputList, int blockSize, std::ofstream &fout);
void fullyAssociative(std::vector<access> &inputList, int blockSize, std::ofstream &fout);
void hotCold(std::vector<access> &inputList, int blockSize, std::ofstream &fout);
void setAssocStoreMiss(std::vector<access> &inputList, int blockSize, std::ofstream &fout);
void setAssocPrefetch(std::vector<access> &inputList, int blockSize, std::ofstream &fout);
void prefetchMiss(std::vector<access> &inputList, int blockSize, std::ofstream &fout);

int main(int argc, char* argv[]){
        
	unsigned long long addr;
	std::string behavior;
	std::vector<access> inputList;
	std::string input;
	std::string output;
	if(argc >= 2){
	        input = argv[1];
		output = argv[2];
	}
	
	std::ifstream infile(input);
	while(infile >> behavior >> std::hex >> addr){
	        struct access temp;
		temp.address = addr;
		temp.instr = behavior;
		inputList.push_back(temp);
	}
	infile.close();

	int blockSize = 32;	
	std::ofstream fout(output);
	directMapped(inputList, blockSize, fout);
	setAssociative(inputList, blockSize, fout);
	fullyAssociative(inputList, blockSize, fout);
	hotCold(inputList, blockSize, fout);
	setAssocStoreMiss(inputList, blockSize, fout);
	setAssocPrefetch(inputList, blockSize, fout);
	prefetchMiss(inputList, blockSize, fout);
	fout.close();
}

void directMapped(std::vector<access> &inputList, int blockSize, std::ofstream &fout){
	int hits = 0;
	int accesses = 0;
	int cacheSize = 32;
	int tag;
	int index;
	int blockAddress;
	struct accessInfo initialVal;
	initialVal.valid = 0;
	initialVal.tag = 0;

	while(cacheSize <= 1024){
		std::vector<accessInfo> direct;
		for(int i=0; i<cacheSize; i++){
			direct.push_back(initialVal);
		}
		for(unsigned int i=0; i<inputList.size(); i++){
			blockAddress = inputList.at(i).address/blockSize;
			index = blockAddress%cacheSize;
			tag = blockAddress/cacheSize;
			if(direct.at(index).valid == 0){
				direct.at(index).valid = 1;
				direct.at(index).tag = tag;
			}
			else if(direct.at(index).valid == 1 && direct.at(index).tag == tag){
				hits++;
			}
			else{
				direct.at(index).tag = tag;
			}
			accesses++;
		}
		fout << hits << "," << accesses << ";";
		if(cacheSize != 1024){
		        fout << " ";
		}
		hits = 0;
		accesses = 0;
		if(cacheSize == 32){
			cacheSize = 128;
		}
		else if(cacheSize == 128){
			cacheSize = 512;
		}
		else{
			cacheSize *= 2;
		}
	}
	fout << std::endl;
}

void setAssociative(std::vector<access> &inputList, int blockSize, std::ofstream &fout){
	int hits = 0;
	int accesses = 0;
	int LRU = 0;
	int associativity = 2;	
	int cacheSize = 512;
	int sets, tag, index, blockAddress;
	bool inserted = false;
	struct accessInfo initialVal;
	initialVal.valid = 0;
	initialVal.tag = 0;

	while(associativity <= 16){
		std::vector<std::vector<accessInfo>> setAssoc;
		sets = cacheSize/(associativity);		
		for(int i=0; i<sets; i++){
			std::vector<accessInfo> indivSet;
			for(int j=0; j<associativity; j++){
				indivSet.push_back(initialVal);	
			}
			setAssoc.push_back(indivSet);
		}
		for(int i=0; i<inputList.size(); i++){
			blockAddress = inputList.at(i).address/blockSize;
			index = blockAddress%sets;
			tag = blockAddress/sets;
			for(int j=0; j<associativity; j++){
				if(setAssoc.at(index).at(j).tag == 0 && !inserted){
					setAssoc.at(index).at(j).tag = tag;	
					setAssoc.at(index).at(j).valid = LRU;
					LRU++;
					inserted = true;
				}				
				else if(setAssoc.at(index).at(j).tag == tag && !inserted){
					setAssoc.at(index).at(j).valid = LRU;
					LRU++;
					hits++;
					inserted = true;
				}
				
			}
			if(!inserted){
				int victim = 0;
				for(int j=1; j<associativity; j++){
					if(setAssoc.at(index).at(j).valid < setAssoc.at(index).at(victim).valid){
						victim = j;
					}
				}
				setAssoc.at(index).at(victim).tag = tag;
				setAssoc.at(index).at(victim).valid = LRU;
				LRU++;
			}
			inserted = false;
			accesses++;
		}
		fout << hits << "," << accesses << ";";
		if(associativity != 16){
		        fout << " ";
		}
		hits = 0;
		accesses = 0;
		LRU = 0;
		associativity *= 2;
	}
	fout << std::endl;
}

void fullyAssociative(std::vector<access> &inputList, int blockSize, std::ofstream &fout){
	int hits = 0;
	int accesses = 0;
	int cacheSize = 512;
	int LRU = 1;
	int blockAddress;	
	bool inserted = false;	
	struct accessInfo initialVal;
	initialVal.valid = 0;
	initialVal.tag = 0;

	std::vector<accessInfo> full;
	for(int i=0; i<cacheSize; i++){
		full.push_back(initialVal);
	}
	for(int i=0; i<inputList.size(); i++){
		blockAddress = inputList.at(i).address/blockSize;
		for(int j=0; j < full.size(); j++){
			if(full.at(j).tag == 0 && !inserted){
				full.at(j).tag = blockAddress;	
				full.at(j).valid = LRU;
				LRU++;
				inserted = true;
			}				
			else if(full.at(j).tag == blockAddress && !inserted){
				full.at(j).valid = LRU;
				inserted = true;				
				hits++;
				LRU++;
			}
		}
		if(!inserted){
			int victim = 0;
			for(int j=1; j<cacheSize; j++){
				if(full.at(j).valid < full.at(victim).valid){
					victim = j;
				}
			}
			full.at(victim).tag = blockAddress;
			full.at(victim).valid = LRU;
			LRU++;
		}
		inserted = false;
		accesses++;
	}
	fout << hits << "," << accesses << ";";
	fout << std::endl;
}

void hotCold(std::vector<access> &inputList, int blockSize, std::ofstream &fout){
        int hits = 0;
	int accesses = 0;
	int cacheSize = 512;
	int LRU;
	int blockAddress;
	bool inserted = false;
	struct accessInfo initialVal;
	initialVal.valid = 0;
	initialVal.tag = 0;

	std::vector<int> hotColdList;
	for(int i=0; i<511; i++){
	        hotColdList.push_back(0);
	}
	for(int i=2; i<514; i++){
	        hotColdList.push_back(i);
	}
	std::vector<accessInfo> full;
	for(int i=0; i<cacheSize; i++){
	        full.push_back(initialVal);
	}
	for(int i=0; i<inputList.size(); i++){
	        blockAddress = inputList.at(i).address/blockSize;
		for(int k=0; k < full.size(); k++){
		        if(full.at(k).tag == blockAddress && !inserted){
		                inserted = true;
				LRU = k+511;
				hits++;
			}
		}
		if(!inserted){
		        int j=0;
   		        while(j < 511){
  			        if(hotColdList.at(j) == 1) j = (2*j)+1;
				else j = (2*j)+2;
			}
			int victIndex = hotColdList.at(j)-2;
			full.at(victIndex).tag = blockAddress;
			while(j != 0){
			        j = (j-1)/2;
				
				if(hotColdList.at(j) == 0) hotColdList.at(j) = 1;
				else hotColdList.at(j) = 0;
			}
		}
		else{
		        while(LRU != 0){
			        LRU = (LRU-1)/2;
				
				if(hotColdList.at(LRU) == 0) hotColdList.at(LRU) = 1;
				else hotColdList.at(LRU) = 0;
			}

		}
		inserted = false;
		accesses++;
	}
	fout << hits << "," << accesses << ";";
	fout << std::endl;
}


void setAssocStoreMiss(std::vector<access> &inputList, int blockSize, std::ofstream &fout){
	int hits = 0;
	int accesses = 0;
	int LRU = 0;
	int associativity = 2;	
	int cacheSize = 512;
	int sets, tag, index, blockAddress;
	std::string type;
	bool inserted = false;	
	struct accessInfo initialVal;
	initialVal.valid = 0;
	initialVal.tag = 0;

	while(associativity <= 16){
		std::vector<std::vector<accessInfo>> setAssoc;
		sets = cacheSize/(associativity);		
		for(int i=0; i<sets; i++){
			std::vector<accessInfo> indivSet;
			for(int j=0; j<associativity; j++){
				indivSet.push_back(initialVal);	
			}
			setAssoc.push_back(indivSet);
		}
		for(int i=0; i<inputList.size(); i++){
			blockAddress = inputList.at(i).address/blockSize;
			type = inputList.at(i).instr;			
			index = blockAddress%sets;	
			tag = blockAddress/sets;
			for(int j=0; j<associativity; j++){
				if(setAssoc.at(index).at(j).tag == 0 && !inserted && type != "S"){
					setAssoc.at(index).at(j).tag = tag;	
					setAssoc.at(index).at(j).valid = LRU;
					LRU++;
					inserted = true;
				}				
				else if(setAssoc.at(index).at(j).tag == tag && !inserted){
					setAssoc.at(index).at(j).valid = LRU;
					LRU++;
					hits++;
					inserted = true;
				}
				
			}
			if(!inserted && type != "S"){
				int victim = 0;
				for(int j=1; j<associativity; j++){
					if(setAssoc.at(index).at(j).valid < setAssoc.at(index).at(victim).valid){
						victim = j;
					}
				}
				setAssoc.at(index).at(victim).tag = tag;
				setAssoc.at(index).at(victim).valid = LRU;
				LRU++;
			}
			inserted = false;
			accesses++;
		}
		fout << hits << "," << accesses << ";";
		if(associativity != 16){
		        fout << " ";
		}
		hits = 0;
		accesses = 0;
		LRU = 0;
		associativity *= 2;
	}
	fout << std::endl;
}

void setAssocPrefetch(std::vector<access> &inputList, int blockSize, std::ofstream &fout){
	int hits = 0;
	int accesses = 0;
	int LRU = 0;
	int associativity = 2;	
	int cacheSize = 512;
	int sets, tag, index, blockAddress, prefetched, pfTag, pfIndex;
	bool inserted = false;
	bool pfInsert = false;	
	struct accessInfo initialVal;
	initialVal.valid = 0;
	initialVal.tag = 0;

	while(associativity <= 16){
		std::vector<std::vector<accessInfo>> setAssoc;
		sets = cacheSize/(associativity);		
		for(int i=0; i<sets; i++){
			std::vector<accessInfo> indivSet;
			for(int j=0; j<associativity; j++){
				indivSet.push_back(initialVal);	
			}
			setAssoc.push_back(indivSet);
		}
		for(int i=0; i<inputList.size(); i++){
			blockAddress = inputList.at(i).address/blockSize;
			prefetched = (inputList.at(i).address+32)/blockSize;			
			index = blockAddress%sets;	
			tag = blockAddress/sets;
			pfIndex = prefetched%sets;
			pfTag = prefetched/sets;
			for(int j=0; j<associativity; j++){
				if(setAssoc.at(index).at(j).tag == 0 && !inserted){
					setAssoc.at(index).at(j).tag = tag;	
					setAssoc.at(index).at(j).valid = LRU;
					LRU++;
					inserted = true;
				}				
				else if(setAssoc.at(index).at(j).tag == tag && !inserted){
					setAssoc.at(index).at(j).valid = LRU;
					LRU++;
					hits++;
					inserted = true;
				}
			}
 			if(!inserted){
				int victim = 0;
				for(int j=1; j<associativity; j++){
					if(setAssoc.at(index).at(j).valid < setAssoc.at(index).at(victim).valid){
						victim = j;
					}
				}
				setAssoc.at(index).at(victim).tag = tag;
				setAssoc.at(index).at(victim).valid = LRU;
				LRU++;
			}
			for(int j=0; j<associativity; j++){
			        if(setAssoc.at(pfIndex).at(j).tag == 0 && !pfInsert){
					setAssoc.at(pfIndex).at(j).tag = pfTag;	
					setAssoc.at(pfIndex).at(j).valid = LRU;
					LRU++;
					pfInsert = true;
				}				
				else if(setAssoc.at(pfIndex).at(j).tag == pfTag && !pfInsert){
			                setAssoc.at(pfIndex).at(j).valid = LRU;
					LRU++;
					pfInsert = true;
				}
			}
			if(!pfInsert){
				int victim = 0;
				for(int j=1; j<associativity; j++){
					if(setAssoc.at(pfIndex).at(j).valid < setAssoc.at(pfIndex).at(victim).valid){
						victim = j;
					}
				}
				setAssoc.at(pfIndex).at(victim).tag = pfTag;
				setAssoc.at(pfIndex).at(victim).valid = LRU;
				LRU++;
			}
			inserted = false;
			pfInsert = false;
			accesses++;
		}
		fout << hits << "," << accesses << ";";
		if(associativity != 16){
		        fout << " ";
		}
		hits = 0;
		accesses = 0;
		LRU = 0;
		associativity *= 2;
	}
	fout << std::endl;
}

void prefetchMiss(std::vector<access> &inputList, int blockSize, std::ofstream &fout){
        int hits = 0;
	int accesses = 0;
	int LRU = 0;
	int associativity = 2;
	int cacheSize = 512;
	int sets, tag, index, blockAddress, prefetched, pfTag, pfIndex;
	bool miss = false;
	bool inserted = false;
	bool pfInsert = false;
	struct accessInfo initialVal;
	initialVal.valid = 0;
	initialVal.tag = 0;

	while(associativity <= 16){
	        std::vector<std::vector<accessInfo>> setAssoc;
		sets = cacheSize/(associativity);
		for(int i=0; i<sets; i++){
		        std::vector<accessInfo> indivSet;
			for(int j=0; j<associativity; j++){
			        indivSet.push_back(initialVal);
			}
			setAssoc.push_back(indivSet);
		}
		for(int i=0; i<inputList.size(); i++){
		        blockAddress = inputList.at(i).address/blockSize;
			prefetched = (inputList.at(i).address+32)/blockSize;
			index = blockAddress%sets;
			tag = blockAddress/sets;
			pfIndex = prefetched%sets;
			pfTag = prefetched/sets;
			for(int j=0; j<associativity; j++){
			        if(setAssoc.at(index).at(j).tag == 0 && !inserted){
					setAssoc.at(index).at(j).tag = tag;	
					setAssoc.at(index).at(j).valid = LRU;
					LRU++;
					miss = true;					
					inserted = true;
				}				
				else if(setAssoc.at(index).at(j).tag == tag && !inserted){
				        setAssoc.at(index).at(j).valid = LRU;
					LRU++;
					hits++;
					inserted = true;
				}
			}
			if(!inserted) miss = true;
			if(!inserted){
			        int victim = 0;
				for(int j=1; j<associativity; j++){
				        if(setAssoc.at(index).at(j).valid < setAssoc.at(index).at(victim).valid){
					        victim = j;
					}
				}
				setAssoc.at(index).at(victim).tag = tag;
				setAssoc.at(index).at(victim).valid = LRU;
				LRU++;
			}
			if(miss == true){ 
			        for(int j=0; j<associativity; j++){
				        if(setAssoc.at(pfIndex).at(j).tag == 0 && !pfInsert){
 						setAssoc.at(pfIndex).at(j).tag = pfTag;
						setAssoc.at(pfIndex).at(j).valid = LRU;
						LRU++;				
						pfInsert = true;
					}				
					else if(setAssoc.at(pfIndex).at(j).tag == pfTag && !pfInsert){
					        setAssoc.at(pfIndex).at(j).valid = LRU;
						LRU++;
						pfInsert = true;
					}
				}
				if(!pfInsert){
				        int victim = 0;
					for(int j=1; j<associativity; j++){
					        if(setAssoc.at(pfIndex).at(j).valid < setAssoc.at(pfIndex).at(victim).valid){
						        victim = j;
						}
					}
					setAssoc.at(pfIndex).at(victim).tag = pfTag;
					setAssoc.at(pfIndex).at(victim).valid = LRU;
					LRU++;
				}
			}
			miss = false;
			inserted = false;
			pfInsert = false;
			accesses++;
		}
		fout << hits << "," << accesses << ";";
		if(associativity != 16){
		        fout << " ";
		}
		hits = 0;
		accesses = 0;
		LRU = 0;
		associativity *= 2;
	}
}

















