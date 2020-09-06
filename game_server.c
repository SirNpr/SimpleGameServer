#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

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
    char *server_message = "Hello welcome to the Server!\n"; 
	send(client_socket , server_message , strlen(server_message) , 0 );
	printf("Hello message sent\n");
	
	// Declare vars for the number guessing game
	int value_read, games = 0; // Declare variables were using
	char client_message[1024] = {0}; // Set buffer for client messages
	char server_message_prep[1024] = {0};
	
	char *player_name;
	player_name = malloc(sizeof(char) * 100);
	
	server_message = "Enter your name: ";
	send(client_socket , server_message , strlen(server_message) , 0 );
	value_read = read(client_socket , client_message, 1024);
	
	snprintf(player_name, 100, client_message);
	int player_name_len = strlen(player_name);
	if (player_name[player_name_len-1] == '\n')
	{
    	player_name[player_name_len-1] = 0;
	}
	
	snprintf(server_message_prep, sizeof server_message_prep, "Well \"%s\", do you want to play a game?\nEnter Y/N [case sensative]: ", player_name);
	server_message = server_message_prep;
	send(client_socket , server_message , strlen(server_message) , 0 );
	value_read = read(client_socket , client_message, 1024);
	int while_check = strncmp("Y", client_message, 1);
	
	while (while_check == 0)
	{
		int attempts, number, guess = 0;
		srand(time(0)); // Set the rand() seed
		number = rand() % 100 + 1; // Seting the number to be guessed based off of what rand() returns
		
		snprintf(server_message_prep, sizeof server_message_prep, "Guess My Number \"%s\".\n", player_name);
		server_message = server_message_prep;
		send(client_socket , server_message , strlen(server_message) , 0 );
		
		do
		{
			server_message = "Enter a guess bewtween 1 and 100: ";
			send(client_socket , server_message , strlen(server_message) , 0 );
			value_read = read(client_socket , client_message, 1024); // Read the clients message in bytes and store it in client_message
			guess = atoi(client_message); // Convernt the clients input into an int
			attempts++; // Incrementing attempts by 1
			
			if (guess > number)
			{
				server_message = "Too high!\n";
				send(client_socket , server_message , strlen(server_message) , 0 );
			}
			else if (guess < number)
			{
				server_message = "Too Low!\n";
				send(client_socket , server_message , strlen(server_message) , 0 );
			}
			else
			{
				snprintf(server_message_prep, sizeof server_message_prep, "Correct the number was %d, you got it in %d guesses!\n", number, attempts);
				server_message = server_message_prep;
				send(client_socket , server_message , strlen(server_message) , 0 );
				games++;
			}
		}while (guess != number);
		
		server_message = "Do you want to play again?\nEnter Y/N: ";
		send(client_socket , server_message , strlen(server_message) , 0 );
		value_read = read(client_socket , client_message, 1024);
		while_check = strncmp("Y", client_message, 1);
	}
	
	snprintf(server_message_prep, sizeof server_message_prep, "Goodbye \"%s\", you won %d games. Wow!\n", player_name, games);
	server_message = server_message_prep;
	send(client_socket , server_message , strlen(server_message) , 0 );
	free(player_name);
	
	// Close server socket
	// shutdown()
	close(server_socket);
	
	printf("Normal Server Shut!\n");
	return 0;
}
