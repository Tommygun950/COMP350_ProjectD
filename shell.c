//PROJECTC BASIC FUNCTIONS
void type(char* input_filename);
void exec(char* input_filename);
//PROJECTC FUNCTIONS FOR VALIDATING INPUT
void parse_input(char input[], char* cmd, char* file1, char* file2);
int fileExists(char input[], char* exists);
//ProjectD FUNCTIONS
void dir();
void del(char* file);
void copy(char* file1, char* file2);
void create(char* input_filename);

int main(){
	while(1){
		char userInput[30]; //max user input, 30 characters.
		char cmd[15]; //max command length, 15 characters.
		char file1[15]; //max filename length, 15 characters.
		char file2[15]; //max filename length, 15 characters.

		char* cmd_type = "type";
		char* cmd_exec = "exec";
		char* cmd_dir = "dir";
		char* cmd_del = "del";
		char* cmd_copy = "copy";
		char* cmd_create = "create";

		//Outputs SHELL header to terminal and calls for user input.
		syscall(0, "\rSHELL> ");
		syscall(1, userInput);
		
		//parses command and filename from user input.
		parse_input(userInput, cmd, file1, file2);

		/*FOR DEBUGGING: prints out the parsed command and file*/
		syscall(0, "\r\n Command: ");
		syscall(0, cmd);
		syscall(0, "\r\n File 1: ");
	    syscall(0, file1);
		syscall(0, "\r\n File 2: ");
	    syscall(0, file2);
		syscall(0, "\r\n\n");	
		
		//If the command entered is <type>, use the type command.
		if(fileExists(cmd, cmd_type)){
			type(file1);
		}
		//If the command entered is <exec>, use the exec command.
		else if(fileExists(cmd, cmd_exec)){
			exec(file1);
		}
		//If the command entered is <dir>, use the dir command.
		else if(fileExists(cmd, cmd_dir)){
			dir();
		}
		//If the command entered is <del>, use the del command.
		else if(fileExists(cmd, cmd_del)){
			del(file1);
		}
		//If the command entered is <copy>, use the copy command.
		else if(fileExists(cmd, cmd_copy)){
			copy(file1, file2);
		}
		//If the command entered is <create>, use the create command.
		else if(fileExists(cmd, cmd_create)){
			create(file1);
		}
		//If they typed an invalid or non-existing command.
		else{
			syscall(0, "Bad command!\n\r");
		}
	}
}

//Will parse both the command and the file out of the input.
void parse_input(char input[], char* cmd, char* file1, char* file2){
	int i=0, j=0, k=0;

	//Extracts command.
	while(input[i] != ' ' && input[i] != '\0'){
		cmd[j++] = input[i++];
	}
	cmd[j] = '\0'; //end of command.

	//Skips blank spaces.
	while (input[i] == ' ') i++;

	//Extracts file1.
	j=0;
	while(input[i] != ' ' && input[i] != '\0'){
		file1[j++] =input[i++];
	}
	file1[j] = '\0'; //end of file1.

	// Skips blank spaces between file1 and file2.
    while(input[i] == ' ') i++;

    // Check if there's a second file (file2).
    if(input[i] != '\0') {
        // Extracts file2. 
        j = 0;
        while(input[i] != ' ' && input[i] != '\0'){
            file2[j++] = input[i++];
        }
        file2[j] = '\0'; // End of file2.
    } else {
        file2[0] = '\0'; // No second file provided, file2 empty.
    }
}

//Will return 0 or 1 (true or false) if the filename in the command input exists or not.
int fileExists(char input[], char* exists){
	int i =0;
	//While the input string and the existing files doesn't reach it's end.
	while(input[i] != '\0' && exists[i] != '\0'){ 
		if(input[i] != exists[i]){
			return 0; //False if the input string doesn't exist as a filename.
		}
		i++;
	}
	return 1; //If all of the characters are found in an existing file.
}

//If user types <type filename>, the shell should print out file.
void type(char* input_filename){
	char buffer[13312]; //allows for max-sized file.
	int sectorsRead;
	syscall(3, input_filename, buffer, &sectorsRead); //3 is readFile.

	if(sectorsRead > 0){
		syscall(0, buffer); //0 is printString.
	}
	else{
		syscall(0, "File not found.\r\n"); //0 is printString.
	}
}

//If user types <exec filename>, the shell will execute file.
void exec(char* input_filename){
	char buffer[13312]; //max file-size.
	int sectorsRead;
	syscall(3, input_filename, buffer, &sectorsRead); //3 is readFile.

	if(sectorsRead > 0){
		syscall(4, input_filename); //4 is executeProgram.
	}
	else{
		syscall(0, "File not found.\r\n"); //0 is printString.
	}
}

//Prints out the files/folders in directory.
void dir()
{
	char directory_buffer[512], file_buffer[12];
	int i = 0; //counter.

	int file_size = 0; //counter for file size.
	int file_entry = 0; //counter for file entry index.

	syscall(2, directory_buffer, 2); //2 is readSector.

	//Itereate through each file sector.
	for (file_entry = 0; file_entry < 512; file_entry += 32){
		if (directory_buffer[file_entry] != '\0') { //If it's not at it's end (\0).
			while(i < 6){
				file_buffer[i] = directory_buffer[file_entry + i];
				i++;
			}
			while (directory_buffer[file_entry + i] != '\0') {
				file_size += 512;
				i++;
			}
			syscall(0, file_buffer); //print out the file name.
			syscall(0, "\n\r"); //Move to the next line on the terminal.
			i = 0; //Reset counter.
		}
	}

}

//Deletes a specified file.
void del(char* file){
	syscall(7, file);
}

//Copies a file & it's contents.
void copy(char* file1, char* file2){
	char max_buffer[512 * 26]; //Max sector size times the max # of possible sectors.
	int sectorsRead;

	syscall(3, file1, max_buffer, &sectorsRead);//
	if(sectorsRead > 0){ //If the file was found.
		syscall(8, max_buffer, file2, sectorsRead);
		syscall(0, "File copied!\r\n"); //Tells user if file was copied.
	}
	else{
		syscall(0, "ERROR: File doesn't exist.\n"); //Reports error to user.
	}
}

//Creates a new file
void create(char* input_filename) {
    char string_space[80];
    char buffer[512 * 26]; //Max size for 26 sectors
    int sector_num = 0;  //Track number of sectors used
    int sector_ind = 0;   //Track the position in the buffer
    int i;

    syscall(0, "File Creator: Leave a line blank & press ENTER to finish:\r\n");

    while (1) {
        //Read a line from the user
        syscall(1, string_space);

        //Check if the user pressed Enter without typing anything (end of input).
        if (string_space[0] == '\r') {
            syscall(0, "End of input detected. Exiting loop.\r\n");
            break;
        }

        //Copy the input line to the buffer.
        for (i = 0; string_space[i] != '\r' && string_space[i] != '\0'; i++) {
            buffer[sector_ind++] = string_space[i];

            //Check for buffer overflow.
            if (sector_ind >= sizeof(buffer)) {
                syscall(0, "Error: File content exceeds maximum size.\r\n");
                return;
            }
        }

        //Add new line characters for formatting.
        buffer[sector_ind++] = '\r';
        buffer[sector_ind++] = '\n';

        //If we've filled a sector, increment sector counter.
        if (sector_ind % 512 == 0) {
            sector_num++;
        }
    }

    //Adjust final sector count if there's leftover data.
    if (sector_ind % 512 != 0) {
        sector_num++;
    }

    //Write the buffer to the file.
    syscall(8, buffer, input_filename, sector_num);

	//Print success.
    syscall(0, "File created successfully!\r\n");
}
