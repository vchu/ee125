# EE125 Project proposal: #
# Topological path planning using network cameras. #
### Team: Topology ###
**Members**:  _Vivian_ _Chu_ , _Jason_ _Cuenco_ , _Shervin_ _Javdani_, _Trung_ _Tran_.

## Abstract: ##
> Route and non-metric path navigation like “going down the hall, turn left at dead end, stop and the second intersect “is more natural for behavior-based robot. Those are called topological path planning which has been out there for several decades. The difficulty of this technique is defining these positions which are hall, dead end, intersects, etc… One of the techniques has been using for a while is artificial land mark. In this technique one single camera on robot will do job. This requires high computation power for this camera. Our approach for this project is using multiple stationary cameras which don’t need much computation power and calibration to construct the topology land marks of the environments in which we navigate the robot through it.
## Implementation detail: ##
> Since this is the class project, the demonstration for single working platform is crucial for our team. In the time limit of 8 weeks of work, we won’t commit very hard or impossible implementation. Therefore, our platform won’t use wireless sensor node as multiple ad-hoc cameras. Instead, we will use computer based camera (since it is easier to debug).
> Platform includes 4+ PC-based cameras (USB webcams type), one central computer, and wheeled robot which has Bluetooth communication unit (using SPP). We will create artificial terrain such as a maze.
Multiple PC-based cameras will synchronizedly update the image of what they see and send it to central computer where all the data will be process to either build the map or navigating Robot through it.
The central computer has two main mode: one is constructing map , and second is navigating robot.
In  “Constructing Map” mode, the central computer will send out command and trigger multiple PC-based cameras to gather enough data (we will defined what is ”enough” mean when we start to work on implementing) for constructing the map.  This map will then save at central computer for next stage of navigating robot.
In “Navigating” mode, we will set up the goal for robot , the central computer will then navigate it through. Now the data coming from multiple cameras will just tell where the robot is. These data will be process and send out command for robot whether going straight turn left or back up.
> Robot has its own local object avoidance scheme. We are thinking of using simple sonar to detect whether Robot hit wall or not .But we also have ambitious of using CMU cam to detect how big object for better planning.  The communication unit will be Bluetooth over Serial Port Protocol. For this we need use of BlueSmirf which is excellent device for close range.
## Job break down: ##
> Since this is class project,  the our  purpose  is learning. The break down won’t be necessary as it is since we will work as group. However, this breakdown consider each of us will in charge as  the subject leader.
  * Robot Motion: Vivian Chu, Jason Cuenco.
  * Networking communication (include Bluetooth):  Trung Tran, Vivian Chu
  * Sonar and local object avoidance: Shervin Javdani, Jason Cuenco
  * Object recognition on multiple Camera:  Shervin Javdani, Trung Tran
  * Map Construction:  Team
  * Planning: Team