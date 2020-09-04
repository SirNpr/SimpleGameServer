#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#define PORT 8080

int main()
{
    // Creating server socket 
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
 
    // Forcefully attaching socket to the port (8080) cause it hangs otherwise when spamming
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    
    // Set the server address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET; 
    server_address.sin_addr.s_addr = INADDR_ANY; 
    server_address.sin_port = htons( PORT ); 
       
    // Bind the socket to our IP and port (8080)
    bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address));
 
    // Listen for client connections and queue up to 5, but can only handle 1 at a time
    listen(server_socket, 5);

    // Set the client socket to talk back too.
	int client_socket;
	int addrlen = sizeof(server_address);
    client_socket = accept(server_socket, (struct sockaddr *) &server_address, (socklen_t*) &addrlen);
    
    // Send server hello to NC
    char *server_message = "Hello from the Server\n"; 
	send(client_socket , server_message , strlen(server_message) , 0 );
	printf("Hello message sent\n");
	
	// Declare vars for the number guessing game
	int number, guess, attempts = 0; // Declare variables were using
	srand(time(0)); // Set the rand() seed
	number = rand() % 100 + 1; // Seting the number to be guessed based off of what rand() returns
	char client_message[1024] = {0}; // Set buffer for client messages
	
	
	server_message = "Guess My Number\n";
	send(client_socket , server_message , strlen(server_message) , 0 );
	do
	{
		server_message = "Enter a guess bewtween 1 and 100: ";
		send(client_socket , server_message , strlen(server_message) , 0 );
		int value_read = read( client_socket , client_message, 1024); // Read the clients message in bytes and store it in client_message
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
			char str[80];
			sprintf(str, "Correct the number was %d, you got it in %d guesses!\n", number, attempts);
			server_message = str;
			send(client_socket , server_message , strlen(server_message) , 0 );
		}
		
	}while (guess != number);
	
	// Close server socket
	// shutdown()
	close(server_socket);
	
	return 0;
}
