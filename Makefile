# cpfile: cpfile.o csapp.o
# 	gcc -o cpfile cpfile.o csapp.o
# cpfile.o : cpfile.c
# 	gcc -c cpfile.c
# csapp.o : csapp.c
# 	gcc -c csapp.c

# clean:
# 	rm -f *.o
# 	rm -f cpfile

# TARGET = echoclient
# OBJECTS = echoclient.o csapp.o

TARGET = echoserveri
OBJECTS = echoserveri.o csapp.o

# TARGET = tiny
# OBJECTS = tiny.o csapp.o

%.o: %.c
	gcc -c $<
$(TARGET): $(OBJECTS)
	gcc -o $(TARGET) $(OBJECTS)

clean:
	rm -f *.o
	rm -f $(TARGET)


