
CXXFLAGS = -W -Wall -O3 -std=c++11
LDLIBS = -lv4l2

LinuxVideo : LinuxVideo.cc Targa.cc Targa.h

clean :
	$(RM) *~ LinuxVideo
