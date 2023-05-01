Name: Adil Aman Mohammed
CWID: A20395630
Description: the following code is a tcp server side code which is capable to handle multiple clients. 
But in the present code it is limited to 3 clients. server.c receives a message from client.c which is stored in a shared file.
In the end the server sends the contains of shared file to client.


Steps to run the server.c and client.c:

Step 1: log in to csx2 server using terminal or cmd 

	server : username@csx2.cs.okstate.edu
	
Step 2: Transfer server.c and client.c files to csx2 server using Winscp
	
Step 3: Use the below command to complie the server.c
	
	gcc server.c -o server -pthread
	
	Use the below command to run server
	
	./server

	Use the below command to complie the client.c

	gcc client.c -o client -w

	Use the below command to run client 

	./client


Important details :
 
Port used : 1051

Server used to check the code : csx2.cs.okstate.edu

Note for client.c :



Before running client.c the IP Address need to be changed to the csx2 IP Address :

IP Address for csx2 : 10.221.247.30

for example IP Address is changed in the following line in client.c code :

if(inet_pton(AF_INET, "10.221.247.30", &serv_addr.sin_addr)<=0)












; 
		 
	