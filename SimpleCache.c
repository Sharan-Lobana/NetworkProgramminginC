#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define REQ_SZ 1000000

void error(char *msg)
{
	perror(msg);
	exit(0);
}

void getmimetype(char *r, int len, char mimet[100])
{
	int i = 0;
	char *temp = "Content-Type:";
	bzero(mimet, 100);
	int found = 0;
	while(1)
	{	
		found = 0;
		if(r[i] == 'C')
		{
			found = 1;
			for(int j = 0; j < strlen(temp); j++)
			{
				if(r[i] == temp[j])
				{
					i++;
				}
				else
				{
					found = 0;
					break;
				}
			}
		}

		if(found == 1)
		{	
			int ind = 0;
			i++;
			while(r[i] != ';')
			{
				mimet[ind] = r[i];
				i++;
				ind++;
			}

			return;
		}

		if(r[i] != 'C')
			i++;
		if(i >= len)
			break;
	}

	return;	
}

char *extractfilename(char *url, int len)
{
	if(len > 4)
	{
		int offset = 0;

		if(url[0] == 'w' && url[1] == 'w' && url[2] == 'w' && url[3] == '.')
		{
			offset = 4;
		}
		else
		{
			int ind = 0;
			while(url[ind] != ':' && ind < len)
			{
				ind++;
			}
			if(ind < len)
			{
	
				offset = ind+3;
				if(url[offset] == 'w' && url[offset+1] == 'w' && url[offset+2] == 'w' && url[offset+3] == '.')
					offset += 4;
			}

		}

		int i = offset; 
		while(i < len && url[i] != '/')
		{
			i++;
		}

		int filenamelength = len - i;
		char *filename = (char *)malloc(sizeof(char)*(filenamelength+1));

		int index = 0;
		while(i < len)
		{
			filename[index] = url[i];
			index++;
			i++;
		}
		filename[filenamelength] = '\0';
		return filename;
	}

	return "No file specified";
}

char *extracthostname(char *url, int len)
{
	char hostname[256];	//Temporary buffer to store the hostname;
	int hostnamelen;
	bzero((char *)&hostname, 256);

	if(len > 4)
	{
		int offset = 0;

		if(url[0] == 'w' && url[1] == 'w' && url[2] == 'w' && url[3] == '.')
		{
			offset = 4;
		}
		else
		{
			int ind = 0;
			while(url[ind] != ':' && ind < len)
			{
				ind++;
			}
			if(ind < len)
			{
				offset = ind+3;
				if(url[offset] == 'w' && url[offset+1] == 'w' && url[offset+2] == 'w' && url[offset+3] == '.')
					offset += 4;
			}

		}

		int i = 0; 
		while(i+offset < len && url[i+offset] != '/')
		{
			hostname[i] = url[i+offset];
			i++;
		}
		hostname[i] = '\0';
		hostnamelen = strlen(hostname);
		char *c = (char *) malloc((hostnamelen+1)*sizeof(char));
		for(int i = 0; i < hostnamelen; i++)
		{
			c[i] = hostname[i];
		}
		c[hostnamelen] = '\0';

		return c;
	}

	return "Illegal URL";
	
}

int main(int argc, char *argv[])
{
	int sockfd, portno;	//socket_file_descriptor, portnumber of server

	struct sockaddr_in serv_addr;
	struct hostent *server;
	char request[REQ_SZ];
	char *url = argv[1];
	int len = strlen(url);
	if(argc < 3)
	{
		if(argc < 2)
		{
			fprintf(stderr, "Usage: %s hostname port\n", argv[0]);
			exit(0);
		}
		else
		{
			fprintf(stderr, "Usage: %s %s port\n",argv[0], url);
			exit(0);
		}
	}

	char *hostname = extracthostname(url, len);
	char *filename = extractfilename(url, len);
	portno = atoi(argv[2]);

	printf("\nDomain %s\n", hostname);
	printf("\nPort Number: %d\n", portno);
	printf("\nPort Number in network byte order: %d\n", htons(portno));
	printf("\nFilename: %s\n", filename);

	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);	//Instantiate the socket and get socket_file_descriptor

	if(sockfd < 0)
	{
		error("\nError opening socket.\n");
		exit(0);	
	}

	server = gethostbyname(hostname);	//Get the hostent instance 

	if(!server)
	{
		fprintf(stderr,"\nERR No such host or gethostbyname() failed.\n");
		exit(0);
	}
	else
		printf("\nHost server successfully found by gethostbyname()\n");

	bzero((char *)&serv_addr, sizeof(serv_addr));	//Initialize the string with zeros

	serv_addr.sin_family = AF_INET;
	bcopy((char *) server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);	//Copy port number after converting from host to network byte order

	printf("\nServer h_name is %s\n",server->h_name);

	if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		error("\nERROR connecting to specified server.\n");
		exit(0);
	}
	else
	{
		printf("\nConnection established to the server with port number %d\r\n", htons(atoi(argv[2])));

		bzero(request, REQ_SZ);	//Initialize request character array with '\0'

		sprintf(request, "GET %s HTTP/1.1\n", url);	//Copy the given string to request array
		printf("\n%s", request);	

		char* rootCacheDirectory = "/tmp/networkprogramming/";	//Path to the temporary cached files root directory
	    
	    int filenamelen = len < 255 ? len: 255;	//Maximum allowed filename length is 255 characters in LINUX

	    //TODO 
	    //Truncate the url to have length less than 256 for converting it to filename with proper extension

	    char* cachedfilename = (char*)malloc(filenamelen+1);	//CachedFilename 
	    
	    //Replace all the forward slashes and colons with underscores to construct the filename
	    for(int i = 0; i < filenamelen; i++)
	    {
	    	if(url[i] == '/' || url[i] == ':')
	    		cachedfilename[i] = '_';
	    	else
	    		cachedfilename[i] = url[i];
	    }

	    cachedfilename[filenamelen] = '\0';

	    char cachedfilepath[1000];	//Complete path to the cached file

	    bzero(cachedfilepath, 1000);	//Initialize the character array with '\0'
	    strncpy(cachedfilepath, rootCacheDirectory,strlen(rootCacheDirectory));
	    strcat(cachedfilepath,cachedfilename);

	    //Check if the file already exists in the CACHE
	    if(access(cachedfilepath, F_OK) != -1)
	    {
	    	printf("\nThe file exists in the system CACHE.\n");
	    	printf("\nThe complete path to the file is %s\n", cachedfilepath);
	    	printf("\nRetreiving the existing file without any request to the server.\n");
	    	
		} 
		else
		{
			if(send(sockfd, request, strlen(request), 0) < 0)
				printf("\nError encountered while sending the request.\n");
			else
				printf("\nRequest for fetching html sent successfully.\n");
			
			bzero(request, REQ_SZ);
		    int received = recv(sockfd, request, REQ_SZ-1, 0);

		    if(!received)
		    {
		    	printf("\n%s\n", "Empty response received");
		    	exit(0);
		    }
		    else if(received < 0)
		    {
		    	printf("\n%s\n", "Error while receiving response");
		    	exit(0);
		    }
		    else
		    {
		    	printf("\n%d %s\n",received,"Number of bytes recieved");
		    }

		    printf("\n The cached file path will be %s\n", cachedfilepath);
		    FILE *file = fopen(cachedfilepath, "ab+");
		    printf("Cached file opened successfully.Beginning to write to the file.\n");
		    request[REQ_SZ-1] = '\0';
		    char mimetype[100];
		    getmimetype(request, strlen(request), mimetype);
		    printf("The mime type has been retreived successfully\n");
		    printf("The mime type is %s\n", mimetype);
		    int success = fputs(request, file);
		    printf("Attempting to copy the response to the opened file.\n");
		    if(success >= 0)
		    	printf("The response has been successfully written to a file in /tmp/networkprogramming\n");
		    else
		    	printf("Error writing the response to a temporary file.\n");
		}

		char command[4110];	//Command to open the file
    	bzero(command, 4100);
    	strcpy(command, "gnome-open ");
    	strcat(command, cachedfilepath);	//Maximum allowed filepath length on linux is 4096 characters
    	if(system(command) != -1)
    	{
    		printf("\nFile opened successfully.\n");
    	}
    	else
    	{
    		printf("\nError opening file.\n");
    	}
    	
    	exit(0);
	    close(sockfd);  
	    return (EXIT_SUCCESS);
	}
}