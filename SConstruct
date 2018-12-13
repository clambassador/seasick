import os

tools = dict()
tools['csv_transpose.cc'] = 'csv_transpose'
tools['csv_join.cc'] = 'csv_join'
tools['csv_grep.cc'] = 'csv_grep'
tools['csv_uniq.cc'] = 'csv_uniq'
tools['csv_stretch.cc'] = 'csv_stretch'
tools['csv_stretchuniq.cc'] = 'csv_stretchuniq'
tools['csv_filter_len.cc'] = 'csv_filter_len'
tools['csv_filter.cc'] = 'csv_filter'
tools['test_data_frame.cc'] = 'test_data_frame'
tools['seasick.cc'] = 'seasick'
tools['csick.cc'] = 'csick'
tools['nsick.cc'] = 'nsick'

PATH_TO_IB=".."
common = Split(PATH_TO_IB + """/ib/libib.a
	       """)
for i in range(0, 5):
    print ""
print "libib.a set to: " + PATH_TO_IB + "/ib/libib.a"
for i in range(0, 5):
    print ""

libs = Split("""pthread
	        ncurses
		microhttpd
	     """)
env = Environment(CXX="ccache clang++ -pthread -I"+ PATH_TO_IB, CPPFLAGS="-D_FILE_OFFSET_BITS=64 -Wall -g --std=c++11 -pthread", LIBS=libs, CPPPATH=PATH_TO_IB)
env['ENV']['TERM'] = 'xterm'

Decider('MD5')
for i in tools:
	env.Program(source = [i] + common, target = tools[i])

