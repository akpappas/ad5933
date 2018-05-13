all: 
	g++ -g -O0 main.cpp -std=c++1z -o ad5933 -lusb-1.0   

clean:
	rm ad5933
