PATH=$PATH:/lib/modules/$(uname -r)/build/include

export PATH=$PATH:/lib/modules/$(uname -r)/build/include

how to build:
$ sudo make
$ sudo make -f makefile.user

how to load/run:
$ sudo insmod driver.ko
$ sudo ./test

how to unload:
$ sudo rmmod driver.ko


control flow link: https://drive.google.com/file/d/1wPPpvaKvycSonVBvsU9kK3fp0g1Q7KFT/view?usp=sharing
	the flow for test skips over the driver logic since it is already displayed through the normal user
space flow but normally test would lead to additional nodes and branches.


Design:
	For the user space, I kept the main method as simple as possible. All it contains is
the switch statement for the menu inputs, the file open function, and some initialization of values.
Furthermore, the switch statement contains the code that calls ioctl as well as the other methods
in userspace.

	In the kernel space, logic is separated by file operation, ioctl, driver initialization, and
exiting the driver. I simply used the code from a previous assignment for the file operations as well
as the driver init/exit functions except for a few additions such as some more if statements that
verify that the driver was sucessfully added to /dev/. For the ioctl function, I separated it by
read and write. If it is reading, it is copying from user space and it takes the new runner and
initializes start_time, total_time, and cur_lap_time.
	Once the first runner is added, the race begins; thus each runner has an advantage over the
next runners and a disadvantage over the previous runners. All runners run at the same speed so
once their jiffy value hits 9999, they have completed a lap and total_lap_time increases by 9999,
cur_lap_time gets set to 0, the star_time variable is set to the current jiffy value, and the lap
variable increments. My logic here was that I don't necesarily need to do anything to the jiffy value
(ie. convert to seconds) if the runners go at the exact same speed and if they do go at the same
speed then I can choose for how long I want them running for since it would take the same amount
of time for each runner to run a specified distance.

	The system uses stamp coupling which is a form of low coupling where modules share a data
structure. This is done when the runner struct is passed to and from kernelspace through ioctl.
It is a fairly low type of coupling because the only thing connecting the modules is a single
data structure but it sometimes does not get all of its fields used.
	Userspace and kernelspace both use stamp coupling as well (when looked at individually)
since it passes all of the entries to create_runner and print_entries. Kernel space uses the
file_operation struct when the driver gets initialized (in cdev_init).

	As far as cohesion, it is mostly communicational cohesion which is a form of moderate
cohesion where parts of a program are grouped together because they share the same inputs and
outputs. In userspace, main and create_runner could certainly be separated even further to
create a more cohesive system but because create_runner and add_runner (which I had from a
previous hw assignment) had similar inputs, I decided to combine them here. In kernelspace,
ioctl could be separated into read and write functions to improve cohesion, but similar to 
user space, they would have the same exact input and output so I decided to put them together.



TEST:
	As I said in test.c, it was hard to test my functions without just rewriting the code again
in the test function since user input was required. So I hardcoded some runners and tried adding
them to the list. While I was really only able to test print_entries, I did show that a) it sucessfully
prints nothing if there are no runners, b) a runner can be added to an empty track, and c) a runner
can be added to a track with a runner already on it.
	Additionally, the tests also verify that ioctl is working since it initializes memory for 2
linked lists: one for the linked list being written through ioctl and one to have data written into it
through ioctl.
