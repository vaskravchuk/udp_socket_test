# udp_socket_test

## Install
### Linux
In CMD:\
git clone https://github.com/vaskravchuk/udp_socket_test.git \
cd udp_socket_test\
gcc main.c -o udp_socket_test
### Windows
Download repository from github\
Run 'Developer Command Prompt for VS'\
Go to directory with sources: 'cd path_to_sources'\
In CMD:\
cl main.c /link /out:udp_socket_test.exe

## Usage
### Send
udp_socket_test[.exe] send -ip XXXX.XXXX.XXXX.XXXX -p XXXXX -s 1707 -c 1000 -d 0.01\
Where:\
-ip - IP address to send packets\
-p - port to send packets\
-s - size of payloads\
-c - amount of packets\
-d - delay in seconds between packets
### Listen
udp_socket_test[.exe] listen -p XXXXX\
Where:\
-p - port to listen
