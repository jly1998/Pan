CC = g++
sources = 	server_connection.cpp http_func.cpp server_other.cpp user_mysql.cpp test_mysql.cpp file_mysql.cpp MD5.cpp 
targets = 	test
headfiles = server.h mysql_select.h MD5.h

LIB=/usr/lib/mysql
HEAD_PATH=/usr/include/mysql

all : $(targets)

test : $(sources) $(headfiles)
	$(CC) -I./ $^ -o $@ `mysql_config --cflags --libs`

	
clean:
	-rm -f $(targets)
