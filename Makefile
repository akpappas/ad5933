all: ad5933

ad5933: 
	g++ -g -O0 main.cpp -std=c++17 -lcyusb -o ad5933

