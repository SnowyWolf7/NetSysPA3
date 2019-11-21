# Aaron Steiner
# NetSysPA3
#Language used: C

In this assignment you can run the webproxy.c program and then open up a internet browser that's 
configured to run locally on a specified port and the proxy is set to the local host. Once webproxy.c is 
running with the specified port number and timeout value, you can enter in a HTTP website address and
the proxy will forward your request to the server and return to you the website content. It will store 
recently stored Hostnames and corresponding IP addresses for the duration of the set timeout value for
faster information retrieval.

1. TO RUN webproxy.c:
	-open the terminal in the NetSysPA3 directory
	-type make
	-type ./webproxy 10001 60&

2. Open up a internet browser such as FireFox that is configured to use a proxy on 127.0.0.1 on port 10001

3. In the top search bar type in:
	http://www.cs.cmu.edu/afs/cs/academic/class/15213-s18/www/schedule.html
	http://netsys.cs.colorado.edu
	or any HTTP site you wish to test

4. This will pull up the requested website as forwarded by the proxy server.
If you type in an a blacklisted website you will get a 403 Forbidden error
If you type in an HTTPS website it will return a 400 Bad Request error
If you type in a website that doesn't exist then it will return a 404 Not Found error
The cache file values will be deleted every 60 seconds since that was specified in the 2nd command argument


5. When you are finished hit ctrl+c and the server will exit gracefully.


In this project I get the request from the browser that the user typed in. I parse get the requested 
host name, see if it exists in the cache, and if it does pull the desired information from there. If the
requested host information is not in the cache I do a gethostbyname() call to get the server's address
and then create a TCP socket to write to the server asking for the requested information. While the server
is writing the information back to the proxy the proxy writes that information back to the client, as well
as writing it to a cache for future use. The program is multi-threaded so you can open multiple tabs and
request from various different websites at the same time. If a requested website is blacklisted the program
will write a 403 forbidden error back to the user. 