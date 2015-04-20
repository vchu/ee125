#EE125 Topology Log

# Introduction #

Here is the log for our progress on the project

## 10-30-2008 - first setup ##
Have two cameras in lab (Trung has another one at home)
Replaced the switch with a wireless router
Installed OpenCV - has to be 1.0 because 1.1 is missing cvcam
Installed Visual Studio 2005 - Careful about it not being backwards compatible with 2008 files
Setup wireless router to be able to remote into computers
Opened up project and was able to compile and build on all computers


Note: If cannot remote into computer need to ping the computer first


Password for all computers

Check Google Doc

Password for Router

Check Google Doc

Password for Wireless

Check Google Doc

Computers from the left to right (physically labeled as well)

192.168.1.3 <br>
192.168.1.2 <br>
192.168.1.4 <br>
192.168.1.5 <br>

<h2>10-31-2008</h2>
We installed SVN onto all the local computers and Trung set up google server to hold code<br>
<br>
<h2>11-6-2008 : INTERNETS</h2>
Have one laptop connect wirelessly to the internet<br>
Have that laptop share it's internet<br>
<ol><li>Go to Network Connections<br>
</li></ol><blockquote>2. Go to Properties for wireless<br>
3. Advanced - Allow other netowrk users to connect<br>
If you cannot get both wireless and lan connection check for combatibility mode<br>
on your wireless card by using command prompt<br>
</blockquote><ol><li>Type "netsh bridge show" and then type:<br>
</li></ol><blockquote>2. "netsh bridge set a # e" where number is the number that showed up as wireless<br>
<blockquote>from previous command</blockquote></blockquote>

Then connect your computer to the WAN port of the router and make sure it grabs an IP<br>
TADA internets!<br>
<br>
- We then downloaded all the code onto the computers<br>
<br>
<h2>11-7-2008 : TCP/IP</h2>
We were able to run the base code - proj.exe<br>
<br>
We started to play around with TCP/IP code that we found online.<br>
<br>
<b>Note we should make sure we don't delete the important copyright items before it</b>

<h2>11-9-2008: Send</h2>
We were able to create a Network class with function send:<br>
<ul><li>This function input included the (ID, Data, Size);</li></ul>

Testing:<br>
<ul><li>We were also able to run the server on one central computer and send it data from another computer as a "client" with actual data from the original camera input in Xmain</li></ul>

<h2>11-12-2008: SendBytes and Recieive</h2>
We decided that it would be safer and more effective to send bytes instead of dealing with a string.  Sadly it took us 2-3 hours to figure out how to use the recv API function correctly.  We ended up passing in a pointer incorrectly which caused an infinite loop.<br>
<ul><li>No longer have the same format to send<br>
</li><li></li></ul>

We got SendBytes to work quickly and the rest of the time for receiveBytes.<br>
<ul><li>ReceiveBytes takes in a variable len and r.</li></ul>

<h2>11-14-2008: Queues/Meeting</h2>
Started to work on Queues and implementing on Server side.<br>
<br>
Had meeting with Edgar.  He gave us the code/program for matlab that compiles data from the server into a pretty map interface<br>
<br>
<h2>11-17-2008: More Queues</h2>
<ul><li>Got a working Queue<br>
</li><li>Implemented System calls to make directories and writing to files<br>
</li><li>Still working on making the proper header to make directories work properly<br>
</li><li>Edgar was there and helped mount cameras and make an "encoder" for the robot<br>
</li><li>Trung has a working PWM for the robot</li></ul>


<h2>11-19-2008: EntranceDetection test</h2>
<ul><li>Got EntranceDetection code to work with server end. Temporarily commented out line 141 on <code>\HSN-Demo2\C-Server\_socket.cpp</code> because it wasn't allowing the rest of the code to compile.<br>
</li><li>Edgar's sample data was processed as he showed us in his demo.<br>
<ul><li>When you run the code, click on one of the 3 windows that open up and press any button on the keyboard.</li></ul></li></ul>

<h2>11-19-2008: Server Revamp</h2>
<ul><li>Transferred test server and Socket to a new project and able to compile with Edgar's server project.  Included all mote functions including mote_comm (Need to think about how to properly remove mote_comm and replace with our sending/receive functions)<br>
</li><li>Fixed socket.cpp issue (just forgot to cast ><")<br>
</li><li>Got proper header and folder issues resolved.  Need to still think about adding pkgType to header</li></ul>

<h2>11-20-2008: Server integration</h2>
<ul><li>Split up the test server functions that were created earlier and put them in the new server.  One thread to receive threads and the other to parse.<br>
</li><li>Ran into issues with the thread suddenly stopping?  Seems to hang instead of repeatedly looping.</li></ul>

<h2>11-21-2008: Thread investigation</h2>
<ul><li>Still no luck with multi-threading</li></ul>

<h2>11-24-2008: Team Meeting</h2>
<ul><li>Met to discuss goals for after Thanksgiving<br>
</li><li>Got multi-threading to work<br>
</li><li>Also worked on getting the queue to parse into Edgar's code<br>
</li><li>Ran into a problem with the queue and how it stored the values.</li></ul>

<h2>11-25-2008: Parse Packets</h2>
<ul><li>Figured out that the Queue stores the pointer.  Not a good idea to store pointers unless you don't delete them until after it is parsed.<br>
</li><li>Got a observation packet to parse through Edgar's code<br>
</li><li>Rebuilt the proper code repositories to have project and code without going over quota limit from Google</li></ul>

<h2>11-28-2008: Send Errors</h2>
<ul><li>Realized that Send was dropping three values because it was passing the incorrect length created by header tags added to packet- fixed by size+3<br>
</li><li>Made changes to NewServer - still trying to integrate Edgar's code.  Semi-successful</li></ul>

<h2>12-4-2008: Live feed</h2>

<ul><li>Changed xmain.cpp in Client to take live feed, as opposed to reading from a file code<br>
</li><li>Should pop up with a camera selection when it runs.Pick any camera not currently being used. Another window should pop up with camera option. Make sure auto white balance and auto gain are both disabled<br>
</li></ul><ul><li>Note that the line drawing is not fully tested</li></ul>

<h2>12-5-2008: Line drawing tests, packet sending errors, Sonar prep</h2>

<ul><li>Line drawing has been fully tested and tweaked to meet our specifications. The client is able to effectively detect up to 2 bisecting lines (3 separate regions for a given camera)<br>
</li><li>Server was modified to take packets from client. However an error in bit counting/shifting caused the server to receive the packets improperly, clipping packet information and registering the wrong length.<br>
</li><li>Sonar modules were prepared for integration into the robot. Wires were stripped and attached to female pin connectors to be attached to the robot's breadboard assembly.