#include "seg.h"
#include "inode.h"
#include <fstream>
//#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <bitset>

#define DISK_BLOCK_SIZE 1024
#define FILE_SIZE 1048576
#define NUM_IMAPS_PER_BLOCK 256

using namespace std;

//Global Variables
////////////////////
Seg segment; //Template for imports
int imap[10240]; //IMAP for drive directory
int map_count;

static bitset<64> bits;

vector<string> toBeWritten;

int cr_vals[10240];
int cr_count = 0;

string fnb[10240][2];

//Function pre-initializations
//////////////////////////////

//Drive initialization
void initDrive();
    int initializeSegFiles();
    void readCheckRegion();

//Runtime functions
int runSystem();
    void import(string, string);
	void writeToDrive();
	void writeMap();
    void list();
    void remove(string);
    int shutdown();
    void cat(string);
    void display(string, int, int);
    void overwrite(string , int , int , char );

////////////////////////////////
//Initializing drive
////////////////////////////////

//Create the drive, read in imap from checkpoint if existing drive detected
void initDrive(){
    if(mkdir("DRIVE", 0777)==-1){ //Tries to create new folder
        readCheckRegion(); //DRIVE detected read check region
        return;
    }
    if(initializeSegFiles()==-1){
        fprintf(stderr, "Error initializing SEGMENT FILES.\n");
        return;
    }
    return;
}

//Helper funtion to create the segment files in DRIVE/
int initializeSegFiles(){
    for(int i = 0; i < 64; i++){
        char buff[18];
        strcpy(buff, ("./DRIVE/SEGMENT" + to_string(i)).c_str());
        cout << buff << endl;
        char segmentBuffer[1024*1024+1];
        memset(segmentBuffer, '0', 1024*1024);
        segmentBuffer[1024*1024] = '\0';
        //cout << segmentBuffer << endl;
        FILE * f;
        f = fopen(buff, "w");
        //cout << segmentBuffer << endl;
        fputs(segmentBuffer, f);
        fclose(f);

    }
    return 0;
}

//Read Check Region
void readCheckRegion(){
    int arr [10240];
    int buff[256];
   ifstream in;
   string line;
   in.open("./DRIVE/CHECKPOINT_REGION");
   int i = 0;
   while(getline(in, line)){
    arr[i] = stoi(line);
    //cout << arr[i] << endl;
    i++;
   }
   in.close();
   for(int j = 0; j < i; j++){
    for(int k = 0; k < 256; k++){
        buff[k] = 0;
    }
    int segNum = arr[j] / 1024;
    int blockNum = arr[j] % 1024;
    string segName = "./DRIVE/SEGMENT" + to_string(segNum);
    FILE * f;
    f = fopen(segName.c_str(), "r");
    for(int l = 0; l < 1024*1024; l++){
        segment.data[l/1024][l%1024] = fgetc(f);
    }
    //cout << blockNum << endl;
    memcpy(buff, &segment.data[blockNum], 1024);

    for(int k = 0; k < 256; k++){
        //cout << buff[k] << endl;
        if(buff[k] != 0){
            imap[map_count] = buff[k];

            segment.currentDataBlock = buff[k]+1;

            int seg = buff[k] / 1024;
            int bloc = buff[k] % 1024;
            int address = bloc * 1024;

            string ts = "./DRIVE/SEGMENT" + to_string(seg);

            Inode inode;
            inode.fileSize = 0;

            FILE * inp;
            inp = fopen(ts.c_str(), "r");
            char tmps[1024*1024];


            for(int i = 0; i < 1024*1024; i++){
                tmps[i] = fgetc(inp);
                //cout << tmps[i];
            }
            cout << sizeof(Inode) << endl;
            memcpy(&inode, &tmps[imap[map_count]*1024], sizeof(Inode));


            fnb[map_count][0] = inode.fileName;
            fnb[map_count][1] = to_string(inode.fileSize);
            cout << inode.fileName << endl;
            cout << inode.fileSize << endl;
            for(int i = 0; i < 128; i++){
                //cout << inode.dataBlockPointers[i] << endl;

            }
            //cout << imap[map_count] << endl;
            map_count++;
        }
    }
   }

}

////////////////////////////////
//Runtime
////////////////////////////////
int runSystem(){
    while(1){ //Run till shutdown

        //Prompt
        cout << "FILESYSTEM:~ ";

        //Take command line input
	    string input, instr, ar1, ar2, ar3, ar4, temp;
	    getline(cin, input);

        //Parse Input
		temp = input;
		instr = temp.substr(0, temp.find(" "));
		temp = temp.substr(temp.find(" ") + 1);
		if(temp.find(" ")&& temp != instr){
			ar1 = temp.substr(0, temp.find(" "));
			temp = temp.substr(temp.find(" ") + 1);

			if(temp.find(" ")){
				ar2 = temp.substr(0, temp.find(" "));
				temp = temp.substr(temp.find(" ") + 1);

				if(temp.find(" ")){
					ar3 = temp.substr(0, temp.find(" "));
					temp = temp.substr(temp.find(" ") + 1);
					if(temp.find(" ")){
						ar4 = temp.substr(0, temp.find(" "));
						temp = temp.substr(temp.find(" ") + 1);
					}
				}
			}
		}

        //Run Input
        if(instr == "import"){
            import(ar1, ar2);
        }
        else if(instr == "remove"){
            remove(ar1);
        }
        else if(instr == "list"){
            list();
        }
        else if(instr == "shutdown"){
            return shutdown();
        }
        else if(instr == "cat"){
            cat(ar1);
        }
        else if(instr == "display"){
            display(ar1, stoi(ar2), stoi(ar3));
        }
        else if(instr == "overwrite"){
            overwrite(ar1, stoi(ar2), stoi(ar3), ar4[0]);
        }
        else {
            cerr << "Invalid command enter new prompt.\n";
        }

    }
}

void import(string toRead, string name){
    ifstream in;
    int filesize = 0;
    in.open(toRead);
    in.seekg(0, in.end);
    filesize = in.tellg();
    in.close();
    toBeWritten.push_back(name);
    char fileBuffer[filesize];
    // for(int i = 0; i < 128; i++){
    //     for(int j =0; j< 1024; j++){

    //     }
    // }

	FILE * f;
    f = fopen(toRead.c_str(), "r");

    Inode inode;
    inode.fileSize = 0;
    for(int i = 0; i < name.length(); i++){
        inode.fileName[i] = name[i];
    }
    if(name.length() < 128){
        for(int i = name.length(); i<128; i++){
            inode.fileName[i] = '\0';
        }
    }
    inode.fileSize = filesize;

    fread(fileBuffer, sizeof(char), filesize, f);
    int num_blocks = filesize / 1024;
    int bytes_remaining = filesize % 1024;

    char testing[1024];


    for(int i = 0; i < num_blocks; i++){
        memcpy(&segment.data[segment.currentDataBlock][0], &fileBuffer[i*1024], 1024);
        inode.dataBlockPointers[i] = segment.currentDataBlock;
        cout << segment.currentDataBlock << endl;
        segment.currentDataBlock++;
        if(segment.currentDataBlock==1024){
            writeToDrive();
        }
    }
    if(bytes_remaining != 0){
        memcpy(&segment.data[segment.currentDataBlock][0], &fileBuffer[num_blocks*1024], bytes_remaining);
        inode.dataBlockPointers[num_blocks] = segment.currentDataBlock;
        cout << segment.currentDataBlock << endl;
        segment.currentDataBlock++;
        if(segment.currentDataBlock==1024){
            writeToDrive();
        }
    } else {

    }
    if(num_blocks<128){
        if(bytes_remaining != 0){
            num_blocks++;
        }
        for(int i = num_blocks; i < 128; i++){
            inode.dataBlockPointers[i] = 0;
        }
    }

    //inode to buffer
    memcpy(&segment.data[segment.currentDataBlock][0], &inode, sizeof(inode));


    ofstream out;
    out.open("out.txt");
    for(int i = 0; i < 1024; i++){
        for(int j = 0; j < 1024; j++){
            out << segment.data[i][j];
        }
    }


// Inode ino;
//     memcpy(&ino, &segment.data[segment.currentDataBlock][0], sizeof(inode));

    //cout << ino.fileName << " " << ino.fileSize << endl;
    // for(int i = 0; i < num_blocks + 1; i++){
        //cout << ino.dataBlockPointers[i] << endl;
    // }

    imap[map_count] = segment.currentDataBlock;
    fnb[map_count][0] = inode.fileName;
    fnb[map_count][1] = to_string(inode.fileSize);
    map_count++;
    segment.currentDataBlock++;
    if(segment.currentDataBlock==1024){
        writeToDrive();
    }
    if(map_count == 256){
        writeMap();
    }

}

void writeMap(){
	int buffer[256]; //block
    int index = 0;
    int i = 0;
    while(i < 10240 && index < 256){
        if(imap[i] != 0){
            buffer[index] = imap[i];
            cout << buffer[index] << endl;
            index++;
        }
        i++;
    }
    while(index < 256){
        buffer[index] = 0;
        index++;
    }

    memcpy(&segment.data[segment.currentDataBlock][0], buffer, 1024);

    int tester[256];
    for(int i = 0; i < 256; i++){
        //cout << tester[i] << endl;
    }

    cr_vals[cr_count] = segment.currentDataBlock;
    cr_count++;
    segment.currentDataBlock++;
}

void writeToDrive(){
    toBeWritten.clear();
    segment.currentSegment = segment.currentDataBlock / 1024;
    segment.driveFileName = "./DRIVE/SEGMENT" + to_string(segment.currentSegment);
    FILE * f;
    f = fopen(segment.driveFileName.c_str(), "w");
    fwrite(segment.data[0], sizeof(char), 1024*1024, f);
    fclose(f);


    segment.currentSegment++;
    if(segment.currentSegment == 64){
        cout << "Mem overload" << endl;
        exit(1);
    }
    segment.currentDataBlock += 8;
}

void list(){
    for(int i = 0; i < map_count; i++){
        if(imap[i] != 0){
            cout << "Name: " << fnb[i][0] << " | Size (bytes): " << fnb[i][1] << endl;
        }
    }
}

void remove(string toRemove){
    for(int i = 0; i < 10240; i++){
	if(imap[i]!=0){
		char b0[128];
		char b1[128];
		for(unsigned int j = 0; j < toRemove.length(); j++){
			b0[j] = toRemove[j];
			b1[j] = fnb[i][0][j];
		}

		if(strcmp(b0, b1)==0){
			imap[i] = 0;
			return;
		}
	}
    }
    cerr << "File not found please try again.\n";
}

int shutdown(){
    writeMap();
	writeToDrive();
    ofstream f;
    f.open("./DRIVE/FILE_NAME_MAP");
    for(int i = 0; i < 10240; i++){
        if(imap[i] != 0){
            f << fnb[i][0] << endl;
        }
    }
    f.close();
    f.open("./DRIVE/CHECKPOINT_REGION");
    for(int i = 0; i < cr_count; i++){
        f << cr_vals[i] << endl;
    }


    return 0;
}


void cat(string toList){
    int inMem = 0;
    for(int i = 0; i < 10240; i++){
        if(imap[i]!=0){
            char b0[128];
            char b1[128];
            for(unsigned int j = 0; j < toList.length(); j++){
                b0[j] = toList[j];
                b1[j] = fnb[i][0][j];
            }
            if(strcmp(b0, b1)==0){
                int seg = imap[i] / 1024;
                int block = imap[i] % 1024;
                for(int m = 0; m < toBeWritten.size(); m++){
                    if(strcmp(b0, toBeWritten[m].c_str())){
                        inMem = 1;
                    }
                }
                if(inMem == 1){
                    Inode inode;
                    char tmps[1024*1024];
                    for(int i = 0; i < 1024*1024; i++){
                        tmps[i] = segment.data[i/1024][i%1024];
                    }
                        memcpy(&inode, &tmps[imap[i]*1024], sizeof(Inode));
                    int index = 0;
                    for(int i = 0; i < 128; i++){
                        char buff[1024];
                        int currentBlock = inode.dataBlockPointers[i];
                        //cout << currentBlock << endl;
                        if(currentBlock != 0){
                            memcpy(buff, &tmps[currentBlock*1024], 1024);
                            for(int i = 0; i < 1024; i++){
                                cout << buff[i];
                            }
                        }
                    }
                } else {
                    string ts = "./DRIVE/SEGMENT" + to_string(seg);

                    Inode inode;

                    FILE * inp;
                    inp = fopen(ts.c_str(), "r");
                    char tmps[1024*1024];


                    for(int i = 0; i < 1024*1024; i++){
                        tmps[i] = fgetc(inp);
                        //cout << tmps[i];
                    }
                    memcpy(&inode, &tmps[imap[i]*1024], sizeof(Inode));
                    //cout << inode.fileName << endl;
                    for(int i = 0; i < 128; i++){
                        char buff[1024];
                        int currentBlock = inode.dataBlockPointers[i];
                        //cout << currentBlock << endl;
                        // if(currentBlock != 0){
                            memcpy(buff, &tmps[currentBlock*1024], 1024);
                            for(int i = 0; i < 1024; i++){
                                cout << buff[i];
                            }
                        // }
                    }
                }
            }
        }
    }
    cout << endl;
}
void display(string name, int num, int start){
    int inMem = 0;
    for(int i = 0; i < 10240; i++){
        if(imap[i]!=0){
            char b0[name.length()];
            char b1[name.length()];
            for(unsigned int j = 0; j < name.length(); j++){
                b0[j] = name[j];
                b1[j] = fnb[i][0][j];
                //cout << b0 << endl;
            // cout << strcmp(b0, b1) << endl;
            }
            if(strcmp(b0, b1)==0){
                char file[128 * 1024];
                int seg = imap[i] / 1024;
                int block = imap[i] % 1024;
                for(int m = 0; m < toBeWritten.size(); m++){
                    if(strcmp(b0, toBeWritten[m].c_str())){
                        inMem = 1;
                    }
                }
                if(inMem == 1){
                    Inode inode;
                    char tmps[1024*1024];
                    for(int i = 0; i < 1024*1024; i++){
                        tmps[i] = segment.data[i/1024][i%1024];
                    }
                        memcpy(&inode, &tmps[imap[i]*1024], sizeof(Inode));
                    //cout << inode.fileName << endl;
                    int index = 0;
                    for(int i = 0; i < 128; i++){
                        char buff[1024];
                        int currentBlock = inode.dataBlockPointers[i];
                        //cout << currentBlock << endl;
                        if(currentBlock != 0){
                            memcpy(buff, &tmps[currentBlock*1024], 1024);
                            for(int i = 0; i < 1024; i++){
                                file[index] = buff[i];
                                index++;
                            }
                        }
                    }
                    cout << endl;
                } else {
                    string ts = "./DRIVE/SEGMENT" + to_string(seg);
                    Inode inode;

                    FILE * inp;
                    inp = fopen(ts.c_str(), "r");
                    char tmps[1024*1024];

                    

                    //cout << "hi" << endl;
                    for(int i = 0; i < 1024*1024; i++){
                        tmps[i] = fgetc(inp);
                    }
                    memcpy(&inode, &tmps[imap[i]*1024], sizeof(Inode));
                    //cout << inode.fileName << endl;
                    
                    int index = 0;
                    for(int i = 0; i < 128; i++){
                        char buff[1024];
                        int currentBlock = inode.dataBlockPointers[i];
                        //cout << currentBlock << endl;
                        if(currentBlock != 0){
                            memcpy(buff, &tmps[currentBlock*1024], 1024);
                            for(int i = 0; i < 1024; i++){
                                file[index] = buff[i];
                                index++;
                            }
                        }
                    }
                }
                
                for(int z = start; z < start+num; z++){
                    if(z < 128*1024 && z >= 0){
                        cout << file[z];
                    }
                }
                cout << endl;

                return;
            }
        }
    }
}
void overwrite(string name, int num, int start, char c){
    int inMem = 0;
    for(int i = 0; i < 10240; i++){
        if(imap[i]!=0){
            char b0[name.length()];
            char b1[name.length()];
            for(unsigned int j = 0; j < name.length(); j++){
                b0[j] = name[j];
                b1[j] = fnb[i][0][j];
                //cout << b0 << endl;
            // cout << strcmp(b0, b1) << endl;
            }
            if(strcmp(b0, b1)==0){
                char file[128][1024];
                for(int i = 0; i < 128; i++){
                    for(int j = 0; j < 1024; j++){
                        file[i][j] = '0';
                    }
                }
                int seg = imap[i] / 1024;
                int block = imap[i] % 1024;
                for(int m = 0; m < toBeWritten.size(); m++){
                    if(strcmp(b0, toBeWritten[m].c_str())){
                        inMem = 1;
                    }
                }
                if(inMem == 1){
                    Inode inode;
                    char tmps[1024*1024];
                    for(int i = 0; i < 1024*1024; i++){
                        tmps[i] = segment.data[i/1024][i%1024];
                    }
                        memcpy(&inode, &tmps[imap[i]*1024], sizeof(Inode));
                    //cout << inode.fileName << endl;
                    int index = 0;
                    for(int i = 0; i < 128; i++){
                        char buff[1024];
                        int currentBlock = inode.dataBlockPointers[i];
                        if(currentBlock != 0){
                            memcpy(file[index], &tmps[currentBlock*1024], 1024);
                            index++;
                        } 
                    }
                    for(int z = start; z < start+num; z++){
                        if(z < 128*1024 && z >= 0){
                            file[z/1024][z%1024] = c;
                        }
                    }
                    for(int i = 0; i < index; i++){
                        int currentBlock = inode.dataBlockPointers[i];
                        if(currentBlock != 0){
                            memcpy(segment.data[currentBlock], file[i], 1024);
                            //cout << segment.data[currentBlock] << endl;
                        }
                    }
                    toBeWritten.push_back(inode.fileName);
                } else {
                    string ts = "./DRIVE/SEGMENT" + to_string(seg);
                    Inode inode;

                    FILE * inp;
                    inp = fopen(ts.c_str(), "r");
                    char tmps[1024*1024];

                    

                    //cout << "hi" << endl;
                    for(int i = 0; i < 1024*1024; i++){
                        tmps[i] = fgetc(inp);
                        //cout << tmps[i];
                    }
                    fclose(inp);
                    memcpy(&inode, &tmps[imap[i]*1024], sizeof(Inode));
                    //cout << inode.fileName << endl;
                    
                    int index = 0;
                    for(int i = 0; i < 128; i++){
                        char buff[1024];
                        int currentBlock = inode.dataBlockPointers[i];
                        if(currentBlock != 0){
                            memcpy(file[index], &tmps[currentBlock*1024], 1024);
                            index++;
                        } 
                    }
                    for(int z = start; z < start+num; z++){
                        if(z < 128*1024 && z >= 0){
                            file[z/1024][z%1024] = c;
                        }
                    }
                    for(int i = 0; i < 1024; i++){
                        memcpy(segment.data[i], &tmps[i*1024], 1024);
                    }
                    for(int i = 0; i < index; i++){
                        int currentBlock = inode.dataBlockPointers[i];
                        if(currentBlock != 0){
                            memcpy(segment.data[currentBlock], file[i], 1024);
                        }
                    }
                    toBeWritten.push_back(inode.fileName);
                
                    
                    
                }
                cout << endl;

            }
        }
    }
}

void cleanSegment(){
    for(int i = 0; i < 64; i++){

    }
}


////////////////////////////////
//Driver
////////////////////////////////

int main(){
    for(int i = 0; i < 64; i++){
        bits[i] = 0;
    }
    segment.currentDataBlock = 8;
    initDrive();
    return runSystem();
}
