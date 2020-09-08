#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

int send_everything(int sock, char *buff, int *len);
int recv_everything(int sock, char** buff, size_t len);

int main(int argc, char *argv[])
{
	if(argc != 3)
	{  
      printf("Error: Expecting %s x.x.x.x PORT\n", argv[0]);
      exit(0);
	}
	
    // Creating server socket 
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
	
    // Forcefully attaching socket to the port (8080) cause it hangs otherwise when spamming
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
	
    // Convert PORT arg into int and do some error checking
    int port = atoi(argv[2]);
    if (port <= 0 | port > 65535)
    {
    	printf("Error: PORT was not accepted.\n");
    	exit(0);
    }
	
    // Set the server address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
	
    // Convert the x.x.x.x arg into an IP address and do some error checking
    if (inet_pton(AF_INET, argv[1], &(server_address.sin_addr)) < 1)
    {
    	printf("Error: IP was not accepted.\n");
    	exit(0);
    }
    server_address.sin_port = htons( port );
	
    // Bind the socket to our IP and port (8080)
    bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address));
	
    // Listen for client connections and queue up to 5, but can only handle 1 at a time
    listen(server_socket, 5);
	
	// Print what IP and Port were listening on
    printf("Server Listening on: %s:%d\n", inet_ntoa(server_address.sin_addr), ntohs(server_address.sin_port));
	
    // Set the client socket to talk back too.
	int client_socket;
	struct sockaddr_in client_address;
	int addrlen = sizeof(client_address);
    client_socket = accept(server_socket, (struct sockaddr *) &client_address, (socklen_t*) &addrlen);
    printf("Connection from: %s\n", inet_ntoa(client_address.sin_addr));
	
	
	/*
	FILE *client_stream;
	if ((client_stream = fdopen(client_socket, "r+w")) == NULL)
	{
		printf("File stream was not created");
		exit(0);
	}
	*/
	
    // Send server hello to NC
    char *server_message = "Hello welcome to the Server!\n";
    
    int server_message_length = strlen(server_message);
    if (send_everything(client_socket, server_message, &server_message_length) == -1)
    {
    	printf("Send everything error");
    }
    
	printf("Hello message sent\n");
	
	// Declare vars for the number guessing game
	int value_read, games = 0; // Declare variables were using
	
	// Propmt the client for a name up too 100 characters
	server_message = "Enter your name: ";
	server_message_length = strlen(server_message);
    if (send_everything(client_socket, server_message, &server_message_length) == -1)
    {
    	printf("Send everything error");
    }
	
	// Allocate memory for the player name

	char *player_name;
	player_name = NULL;
	size_t len = 0;
	int size_of_name = 0;
	
	while ((size_of_name = recv_everything(client_socket, &player_name, len)) == -2)
	{
		printf("Error on size\n");
		server_message = "That name is too long try again: ";
		server_message_length = strlen(server_message);
	    if (send_everything(client_socket, server_message, &server_message_length) == -1)
	    {
	    	printf("Send everything error");
	    }
	}
	
	printf("Player name size: %d\n", size_of_name);
	printf("The player is: %s\n", player_name);
	
	// Ask the players if they want to play a game, then compare what they say to Y, return 0 if all they hit is Y\n
	char server_message_prep[1024] = {0};
	snprintf(server_message_prep, size_of_name + 100, "Well \"%s\", do you want to play a game?\nEnter Y/N [case sensative]: ", player_name);
	server_message = server_message_prep;
	
	server_message_length = strlen(server_message);
    if (send_everything(client_socket, server_message, &server_message_length) == -1)
    {
    	printf("Send everything error\n");
    }
    
    char *client_message; // Set buffer for client messages
    client_message = NULL;
    
    size_of_name = recv_everything(client_socket, &client_message, 0);
   
	//value_read = read(client_socket , client_message, 1024);
	int while_check = strncmp("Y", client_message, 1);
	
	// While loop that starts the gameflow
	while (while_check == 0)
	{
		// Delcare stuff
		int attempts, rand_number, guess = 0;
		srand(time(0)); // Set the rand() seed
		rand_number = rand() % 100 + 1; // Seting the number to be guessed based off of what rand() returns
		
		snprintf(server_message_prep, sizeof server_message_prep, "Guess My Number \"%s\".\n", player_name);
		server_message = server_message_prep;
		
		server_message_length = strlen(server_message);
	    if (send_everything(client_socket, server_message, &server_message_length) == -1)
	    {
	    	printf("Send everything error\n");
	    }
		
		do
		{
			server_message = "Enter a guess bewtween 1 and 100: ";
			
			server_message_length = strlen(server_message);
		    if (send_everything(client_socket, server_message, &server_message_length) == -1)
		    {
		    	printf("Send everything error\n");
		    }
			value_read = read(client_socket , client_message, 1024); // Read the clients message in bytes and store it in client_message
			guess = atoi(client_message); // Convernt the clients input into an int
			attempts++; // Incrementing attempts by 1
			
			if (guess > rand_number)
			{
				server_message = "Too high!\n";
				server_message_length = strlen(server_message);
			    if (send_everything(client_socket, server_message, &server_message_length) == -1)
			    {
			    	printf("Send everything error\n");
			    }
			}
			else if (guess < rand_number)
			{
				server_message = "Too Low!\n";
				server_message_length = strlen(server_message);
			    if (send_everything(client_socket, server_message, &server_message_length) == -1)
			    {
			    	printf("Send everything error\n");
			    }
			}
			else
			{
				snprintf(server_message_prep, sizeof server_message_prep, "Correct the number was %d, you got it in %d guesses!\n", rand_number, attempts);
				server_message = server_message_prep;
				server_message_length = strlen(server_message);
			    if (send_everything(client_socket, server_message, &server_message_length) == -1)
			    {
			    	printf("Send everything error\n");
			    }
				games++;
			}
		}while (guess != rand_number);
		
		// Prompt the user if they want to play again, same check
		attempts = 0; // Reset the attempts number
		server_message = "Do you want to play again?\nEnter Y/N: ";
		server_message_length = strlen(server_message);
	    if (send_everything(client_socket, server_message, &server_message_length) == -1)
	    {
	    	printf("Send everything error\n");
	    }
		value_read = read(client_socket , client_message, 1024);
		while_check = strncmp("Y", client_message, 1);
	}
	
	// Complement the user on their wins, free the memory we alloced 
	snprintf(server_message_prep, sizeof server_message_prep, "Goodbye \"%s\", you won %d games. Wow!\n", player_name, games);
	server_message = server_message_prep;
	server_message_length = strlen(server_message);
    if (send_everything(client_socket, server_message, &server_message_length) == -1)
    {
    	printf("Send everything error\n");
    }
	free(player_name);
	
	// Close server socket
	// shutdown()
	close(server_socket);
	
	printf("Normal Server Shut!\n");
	return 0;
}

int send_everything(int sock, char *buff, int *len)
{
	int total = 0;
	int bytesleft = *len;
	int number;
	
	while(total < *len)
	{
		number = send(sock, buff+total, bytesleft, 0);
		if (number == -1)
		{
			break;
		}
		total += number;
		bytesleft -= number;
	}
	*len = total;
	
	return number==-1?-1:0;
}

int recv_everything(int sock, char** buff, size_t len)
{
	int number = 0;
	
	if (len == 0)
	{
		len = 12;
	}
	if (*buff == NULL)
	{
		*buff = calloc(len, sizeof(char));
	}
	else
	{
		*buff = realloc(*buff, len);
	}
	number = recv(sock, *buff, len, MSG_PEEK);

	while (number < 0 || (*buff)[(number - 1)] != '\n')
	{
		if (number == -1)
		{
			printf("Error at recv_everything");
			break;
		}
		len += number;

		*buff = realloc(*buff, len);
		number = recv(sock, *buff, len, MSG_PEEK);
	}
	
	char *dump = malloc(number * sizeof(char));
	recv(sock, dump, number, 0); // Dump the rest if the data into a charr aray
	free(dump);
	
	(*buff)[number - 1] = 0; // Remove the '\n' at the end of the data
	
	if (number > 500) // Size requirment testing
		{
			return -2;
		}
		
	return number;
}
