import socket

def start_server():
    host = '127.0.0.1'
    port = 8080
    
    # Create a socket object
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    # Bind the socket to the address and port
    server_socket.bind((host, port))
    
    # Listen for incoming connections
    server_socket.listen(1)
    print(f"Listening for connections on {host}:{port}...")
    
    # Wait for a connection from a client
    client_socket, client_address = server_socket.accept()
    print(f"Accepted connection from {client_address}")
    
    # Receive data from the client
    data = client_socket.recv(1024).decode('utf-8')
    print(f"Received message from client: {data}")
    
    # Close the sockets
    client_socket.close()
    server_socket.close()

if __name__ == "__main__":
    start_server()