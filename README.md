# Aaron Steiner
# NetSysPA3
#Language used: C

In this assignment you can run the webserver.c program and then open up a internet browser, and type in
get commands to access certain files within the directory or to load the default webpage. In order to do
this use the following steps:

1. TO RUN webserver.c:
	-open the terminal in the www directory
	-type make
	-type ./webserver 8888

2. Open up a internet browser such as Google Chrome
3. In the top search bar type in:
	localhost:8888

4. This will pull up the default welcome webpage. From here you can click on the various links on the
webpage and it will load those pages. You can also type in a specific path of a file such as
	localhost:8888/images/apple_ex.png and it will load that file. If you type in an incorrect file
name or path you will get the HTTP/1.1 500 Internal Server Error.

5. When you are finished hit ctrl+c and the server will exit gracefully.


In this project I would get the incoming get request and then break it down into segments. I identified
which file the get request wanted, then I would use a function to get the size of the file, and then get
the content type so that I could write the information to the header. Then in a while loop I read the 
requested file and write that file back to the requester's webpage. Then as long as the webpage is
open and there are no new requests the connectection will not timeout.