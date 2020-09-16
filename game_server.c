#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

int send_everything(int sock, char *buff, int *len);
int recv_everything(int sock, char** buff);

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
	
    // Send server hello to NC
    char *server_message = calloc(100, sizeof(char));
    //char server_message_prep[1024] = {0};
    
    //server_message_prep = "Hello welcome to the Server!\n";
    strcpy(server_message, "Hello welcome to the Server!\n");
    int server_message_length = strlen(server_message);
    if (send_everything(client_socket, server_message, &server_message_length) == -1)
    {
    	printf("Send everything error");
    }
    
	printf("Hello message sent\n");
	
	// Propmt the client for a name up too 100 characters
	strcpy(server_message, "Enter your name: ");
	server_message_length = strlen(server_message);
    if (send_everything(client_socket, server_message, &server_message_length) == -1)
    {
    	printf("Send everything error");
    }
	
	// Allocate memory for the player name
	char *player_name;
	player_name = NULL;
	int size_of_name = 0;
	
	while ((size_of_name = recv_everything(client_socket, &player_name)) == -2)
	{
		printf("Error on size\n");
		strcpy(server_message, "That name is too long try again: ");
		server_message_length = strlen(server_message);
	    if (send_everything(client_socket, server_message, &server_message_length) == -1)
	    {
	    	printf("Send everything error");
	    	return -1;
	    }
	}
	if (size_of_name == 0)
	{
		printf("Normal Server Shut!\n");
		return 0;
	}
	
	printf("The player is: %s\n", player_name);
	
	// Ask the players if they want to play a game, then compare what they say to Y, return 0 if all they hit is Y\n
	size_of_name += 100;
	int *size_of_buff;
	size_of_buff = &size_of_name;

	server_message = realloc(server_message, size_of_name * sizeof(char));

	snprintf(server_message, *size_of_buff, "Well \"%s\", do you want to play a game?\nEnter Y/N [case sensative]: ", player_name);

	server_message_length = strlen(server_message);
    if (send_everything(client_socket, server_message, &server_message_length) == -1)
    {
    	printf("Send everything error\n");
    }
    
    char *client_message; // Set buffer for client messages
    client_message = NULL;
    int size_of_client_message = 0;
    
    while ((size_of_client_message = recv_everything(client_socket, &client_message)) == -2)
    {
    	printf("Error on size\n");
    	server_message = "That message is too long try again: ";
    	server_message_length = strlen(server_message);
    	if (send_everything(client_socket, server_message, &server_message_length) == -1)
	    {
	    	printf("Send everything error");
	    }
    }
   
	int while_check = strncmp("Y", client_message, 1);
	
	int games = 0;
	
	// While loop that starts the gameflow
	while (while_check == 0)
	{
		// Delcare stuff
		int attempts, rand_number, guess = 0;
		srand(time(0)); // Set the rand() seed
		rand_number = rand() % 100 + 1; // Seting the number to be guessed based off of what rand() returns
		printf("Rand number is: %d\n", rand_number);
		
		snprintf(server_message, *size_of_buff, "Guess My Number \"%s\".\n", player_name);
		
		server_message_length = strlen(server_message);
	    if (send_everything(client_socket, server_message, &server_message_length) == -1)
	    {
	    	printf("Send everything error\n");
	    }
		
		do
		{
			strcpy(server_message, "Enter a guess bewtween 1 and 100: ");
			
			server_message_length = strlen(server_message);
		    if (send_everything(client_socket, server_message, &server_message_length) == -1)
		    {
		    	printf("Send everything error\n");
		    }

			while ((size_of_client_message = recv_everything(client_socket, &client_message)) == -2)
			{
		    	printf("Error on size\n");
		    	strcpy(server_message, "That message is too long try again: ");
		    	server_message_length = strlen(server_message);
		    	if (send_everything(client_socket, server_message, &server_message_length) == -1)
			    {
			    	printf("Send everything error");
			    }				
			}
			if (size_of_client_message == 0)
			{
				while_check = 1;
				break;
			}
			guess = atoi(client_message); // Convernt the clients input into an int
			printf("Guess is: %d\n", guess);
			attempts++; // Incrementing attempts by 1
			
			if (guess > rand_number)
			{
				strcpy(server_message, "Too high!\n");
				server_message_length = strlen(server_message);
			    if (send_everything(client_socket, server_message, &server_message_length) == -1)
			    {
			    	printf("Send everything error\n");
			    }
			}
			else if (guess < rand_number)
			{
				strcpy(server_message, "Too Low!\n");
				server_message_length = strlen(server_message);
			    if (send_everything(client_socket, server_message, &server_message_length) == -1)
			    {
			    	printf("Send everything error\n");
			    }
			}
			else
			{
				snprintf(server_message, *size_of_buff, "Correct the number was %d, you got it in %d guesses!\n", rand_number, attempts);
				server_message_length = strlen(server_message);
			    if (send_everything(client_socket, server_message, &server_message_length) == -1)
			    {
			    	printf("Send everything error\n");
			    }
				games++;
			}
		}while (guess != rand_number);
		
		if (while_check == 1)
		{
			break;
		}
		// Prompt the user if they want to play again, same check
		attempts = 0; // Reset the attempts number
		strcpy(server_message, "Do you want to play again?\nEnter Y/N: ");
		server_message_length = strlen(server_message);
	    if (send_everything(client_socket, server_message, &server_message_length) == -1)
	    {
	    	printf("Send everything error\n");
	    }

		while ((size_of_client_message = recv_everything(client_socket, &client_message)) == -2)
    	{
	    	printf("Error on size\n");
	    	strcpy(server_message, "That message is too long try again: ");
	    	server_message_length = strlen(server_message);
	    	if (send_everything(client_socket, server_message, &server_message_length) == -1)
		    {
		    	printf("Send everything error");
		    }
    	}
    	if (size_of_client_message == 0)
    	{
    		break;
    	}
    	
		while_check = strncmp("Y", client_message, 1);
	}
	
	// Complement the user on their wins, free the memory we alloced 
	snprintf(server_message, *size_of_buff, "Goodbye \"%s\", you won %d game(s). Wow!\n", player_name, games);
	server_message_length = strlen(server_message);
    if (send_everything(client_socket, server_message, &server_message_length) == -1)
    {
    	printf("Send everything error\n");
    }
	free(player_name);
	free(server_message);
	
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

int recv_everything(int sock, char** buff)
{
	int number = 0;
	char temp_buff[1500] = {0};
	int len = 500;
	
	*buff = realloc(*buff, len * sizeof(char));
	memset(*buff, 0, len);

	number = recv(sock, temp_buff, 1500, 0);

	while (number <= 0 || temp_buff[(number - 1)] != '\n')
	{
		if (number == -1)
		{
			printf("Error at recv_everything\n");
			free(*buff);
			return number;
		}
		if (number == 0)
		{
			
			printf("recv: returning 0\n");
			free(*buff);
			return number;
		}
		if (number < -1)
		{
			printf("Unexpeted Error in recv\n");
			free(*buff);
			return number;
		}
		
		len += number;
		if (len > 3500)
		{
			printf("Size of packet excedes 3499\n");
			//free(*buff);
			return -2;
		}
		
		*buff = realloc(*buff, len);
		strcat(*buff, temp_buff);
		number = recv(sock, temp_buff, 1500, 0);
	}
	
	if (number > len)
	{
		len += number;
		*buff = realloc(*buff, len);
	}
	
	strcat(*buff, temp_buff);
	
	(*buff)[strlen(*buff) - 1] = 0; // Remove the '\n' at the end of the data
		
	return strlen(*buff);
}
