
CXXFLAGS = -W -Wall -O3
LDLIBS = -lv4l2

LinuxVideo : LinuxVideo.cc

clean :
	$(RM) *~ LinuxVideo
