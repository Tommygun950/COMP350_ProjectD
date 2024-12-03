//ProjectB Functions
void printString(char*);
void printChar(char*);
char* readString(char*);
char* readSector(char*, int);
void handleInterrupt21(int ax, char* bx, int cx, int dx);
//ProjectC Functions
int directoryExists(char* directory_sector, int* file_entry, char* possible_filename);
void readFile(char* filename, char* output_buffer, int* sectorsRead);
void executeProgram(char* name);
void terminate();
//ProjectD Functions
void writeSector(char* buffer, int sector);
void deleteFile(char* filename);
void writeFile(char* buffer, char* filename, int numberOfSectors);

//////////////////////////////////////////////////////////////////////////////////////////
//main function
void main(){
	makeInterrupt21();

	interrupt(0x21, 8, "this is a test massage", "testmg", 3);

	interrupt(0x21, 4, "shell", 0,0); //loads and executes shell.
}
//////////////////////////////////////////////////////////////////////////////////////////
//Prints string to terminal.
void printString(char* chars){
	int i=0;
	while (chars[i] != 0x0){ //0x0 signifies the last character in the string.
		char al = chars[i]; //AL register is the place holder for each character.
		char ah = 0xe; //AH register is always 0xe.
		int ax = ah*256 + al; //AX is always equal to AH*256+AL.
		interrupt(0x10,ax,0,0,0);//0,0,0 are registers BX,CX,DX but are not used.
		++i;
	}
}

//Prints a single character to the terminal.
void printChar(char* c){	
	char al = c;
	char ah = 0xe;
	int ax = ah*256 + al;
	interrupt(0x10,ax,0,0,0);
}

//Takes input fro mthe screen and sends it back out (like echo command).
char* readString(char* array_input)
{
	char keyboard_input = 0x0;

	int i = 0;
	while (i < 80) { //The character array is limited to 80 elements. 
		//0x16 is how BIOS reads from interrupt.
		keyboard_input = (char) interrupt(0x16,0,0,0,0); 

		//0xd is enter and notifies the system that's the end of the string.
		if (keyboard_input == 0xd)
			break;
		//Makes sure if backspace is pressed, it won't store it.
		if (keyboard_input == 0x8 && i == 0)
			continue;
		//0x8 is the backspace key, and will remove the last element.
		if (keyboard_input == 0x8){
			char space = ' ';
			interrupt(0x10, 0xe * 256 + keyboard_input,0,0,0);
			interrupt(0x10, 0xe * 256 + space,0,0,0);
			interrupt(0x10, 0xe * 256 + keyboard_input,0,0,0);
			//the 2nd backspace removes it from the screen.
			--i;
			continue;
		}

		array_input[i] = keyboard_input;
		interrupt(0x10, 0xe * 256 + keyboard_input,0,0,0);
		++i;
	}
	array_input[i] = 0xa; //0xa is for a line feed.
	array_input[i + 1] = 0x0; //0x0 signifies the end of the string.

	interrupt(0x10, 0xe * 256 + 0xd,0,0,0); //Outputs each entire element from the array.
	interrupt(0x10, 0xe * 256 + 0xa,0,0,0); //Outputs line feed character.

	return array_input;
}

//Reads from a file and outputs it to the screen.
char* readSector(char* buffer, int sector){
	int ah = 2; //Tells BIOS to read a sector rather than write.
	int al = 1; //Leave as 1, its the number of sectors to read.
	int ax = ah * 256 + al;

	char* bx = buffer; //address where the data should be stored to.
	
	int ch = 0; //track number.
	int cl = sector + 1; //relative sector number.
	int cx = ch * 256 + cl; 

	int dh = 0; //head number.
	int dx = dh * 256 + 0x80;

	interrupt(0x13, ax, bx, cx, dx);
	
	return buffer;
}

//Our own defined interrupt that can be called within kernel.c.
void handleInterrupt21(int ax, char* bx, int cx, int dx){
	switch(ax){ //AX is the # that determines which function to run.
		case 0: printString(bx); break; //if ax is 0, it'll printString.
		case 1: readString(bx); break; //if ax is 1, it'll readString.
		case 2: readSector(bx, cx); break; //if ax is 2, it'll read sector.
		case 3: readFile(bx, cx, dx); break; //if ax is 3, it'll readFile.
		case 4: executeProgram(bx); break; //if ax is 4, it'll executeProgram.
		case 5: terminate(); break; //if ax is 5, it'll terminate.
		case 6: writeSector(bx, cx); break; //If ax is 5, it'll writeSector.
		case 7: deleteFile(bx); break; //if ax is 7, it'll deleteFile.
		case 8: writeFile(bx, cx, dx); break; //if ax is 8, it'll writeFile.
		default: printString("ERROR: AX is invalid."); break; //if ax isn't anything above, it'll print and error.
	}
}

//Will return either 1 or 0 (true or false) if a file exists in the directory.
int directoryExists(char* directory_sector, int* file_entry, char* possible_filename){
	int i = 0; //counter
	int letters = 0; //counter for correct number of letters in the loop. 
	
	//Steps through the directory in increments of 32.
	for(*file_entry=0; *file_entry<512; *file_entry+=32){
		//Compares the first 6 letters in the filename.
		while(i<6){
			//If the letter isn't the same.
			if(directory_sector[*file_entry + i] != possible_filename[i]){
				break;
			}
			//If the letter is the same.
			else{
				++letters;
			}
			++i;
		}
		//If all 6 letters are the same, return true.
		if(letters == 6){
			return 1;
		}
	}
	//If all 6 letters don't match, return false.
	return 0;
}

//Will take a character array containing a file name and reads the file into a buffer.
void  readFile(char* filename, char* output_buffer, int* sectorsRead){
	int i = 0; //counter
	char directory_sector[512]; //load directory sector into 512 byte character array.

	int file_entry = 0;
	int* param = &file_entry;
	*sectorsRead = 0;
	readSector(directory_sector, 2);

	//If the filename exists.
	if(directoryExists(directory_sector, param, filename) == 1){ 
		while(directory_sector[*param + i] != 0){
			readSector(output_buffer, directory_sector[*param + 6 + i]);
			output_buffer += 512;
			++*sectorsRead;
			++i;
		}
	}
	//If the filename doesn't exist.
	else{
		*sectorsRead = 0;
	}
}

//Will take the name of a program, use readFile to locate it, and it'll execute it.
void executeProgram(char* program){
	char buffer[13312]; //13312 is the maximum size of a file.
	int sectorsRead;
	int i = 0;

	//Call readFile to load the file into a buffer.
	readFile(program, buffer, &sectorsRead);

	for( i=0; i<sectorsRead*512; i++){
		//Transfter file from buffer into memory at segment 0x2000.
		putInMemory(0x2000, i, buffer[i]);
	}
	launchProgram(0x2000); //This is a new routine found in kernel.asm.
}

//Will make an interupt 0x21 call to reload and execute shell.
void terminate(){
	char shellname[6];

	//copy letters in one at a time.
	shellname[0]='s';
	shellname[1]='h';
	shellname[2] = 'e';
	shellname[3] = 'l';
	shellname[4] = 'l';
	shellname[5] = '\0';

	executeProgram(shellname);
}

//Writes to a sector on disk.
void writeSector(char* buffer, int sector){
	int ah = 3;
	int al = 1; //# of sectors to write.
	int ax = ah * 256 + al;
	
	char* bx = buffer; 

	int ch = 0; //Track #.
	int cl = sector + 1; //Relative sector #.
	int cx = ch * 256 + cl;

	int dh = 0; //Head #.
	int dx = dh * 256 + 0x80;

	interrupt(0x13, ax, bx, cx, dx);
}

//Deletes a specified file.
void deleteFile(char* filename){
	char dir[512]; //Directory sector buffer.
	char map[512]; //Disk map buffer.
	int file_entry = 0; //Varaible for file index.

	//Reads directory and disk map.
	readSector(dir, 2);
	readSector(map, 1);

	//Search for file.
	if(directoryExists(dir, &file_entry, filename)){
		int i = 0;
		dir[file_entry] = 0x00;

		while(dir[file_entry + 6 + i] != 0){
			int sector_num = dir[file_entry + 6 + i];
			map[sector_num] = 0x00;
			i++;
		}

		//Writes modified dir and map back to disk.
		writeSector(dir, 2);
		writeSector(map, 1);
	}
}

//Writes data to a file by allocating free sectors.
void writeFile(char* buffer, char* filename, int numberOfSectors){
    //Necessary strings.
	char dir[512]; //directory buffer.
	char map[512]; //map buffer.
	char buffer[512]; //general-use buffer.

	//Necessary variables.
    int i, j;
    int file_entry; //Var to track entry.
	int dir_column; //Index for storing sectors #s in dir entry.
	int sector_ct = 0; //Count of sectors.
	int sector_current; //Counter for current sector.
	int sector_total; //Total number of sectors.
  
    int freeSectors[512]; 

    readSector(map, 1); 
    readSector(dir, 2); 
    
    // Finds free sectors and allocates them for the new file.
    for (sector_total = 3; sector_total < 512 && sector_ct < numberOfSectors; sector_total++) {
        if (map[sector_total] == '\0') { //Checks if sector is empty.
            map[sector_total] = 0xFF;
            sector_ct++;
            freeSectors[sector_ct - 1] = sector_total;
        }
    }

    // Validate if there enough free sectors.
    if (sector_ct < numberOfSectors) {
        return; //Breaks if not enough free sectors.
    }

	//Finds free drectory entry & stores the filename and allocated sector #s.
    for (file_entry = 0; file_entry < 512; file_entry += 32) { 
        if (dir[file_entry] == '\0') { //Checks if its free.
            for (i = 0; i < 6; i++) {
                if (filename[i] == '\0')
                    dir[file_entry + i] = '\0';
                else
                    dir[file_entry + i] = filename[i];
            }

            //Stores column index for sector #s in the dir entry.
            dir_column = file_entry + 6;

            //Stores allocated sector #s in the dir entry.
            for (i = 0; i < sector_ct && i < 26; i++)
                dir[dir_column + i] = freeSectors[i];

            break;
        }
    }
    
    //Writes the file data buffer into allocated sectors.
    for (i = 0; i < sector_ct && i < 26; i++) {
        sector_current = freeSectors[i];
        readSector(buffer, sector_current);

        for (j = 0; j < 512; j++)
            buffer[j] = buffer[j + (i * 512)];

        writeSector(buffer, sector_current);
    }

    //Updates with new map and dir.
    writeSector(map, 1);
    writeSector(dir, 2);
}
